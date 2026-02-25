#include "fftw3.h"
#include <omp.h>

#include <array>
#include <atomic>
#include <cstring>
#include <map>
#include <mutex>
#include <new>

constexpr size_t complex_size { sizeof(double) * 2 };
constexpr size_t OMP_THRESHOLD { 65536 }; // 1MB L2 cache / 16 byte

template <typename T>
class fftw_memory {
public:
    explicit fftw_memory(size_t size)
        : cols_(size)
    {
        alloc(size);
    }
    explicit fftw_memory(size_t rows, size_t cols) // NOLINT(bugprone-easily-swappable-parameters)
        : rows_(rows), cols_(cols)
    {
        constexpr size_t avx_align {64};
        constexpr size_t n_pad = avx_align / sizeof(T);
        stride_ = (cols_ + n_pad - 1) / n_pad * n_pad;
        alloc(rows_ * stride_);
    }
    fftw_memory(const fftw_memory&) = delete;
    fftw_memory& operator=(const fftw_memory&) = delete;
    fftw_memory(fftw_memory&&) = delete;
    fftw_memory& operator=(fftw_memory&&) = delete;
    ~fftw_memory() {
        if (ptr_) {
            fftw_free(ptr_);
        }
    }
    T* get() const { return ptr_; }
    T* operator[](size_t row) const { return ptr_ + (row * stride_); }
    size_t size() const { return size_; }
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }
    size_t stride() const { return stride_; }
private:
    void alloc(size_t size)
    {
        size_ = size;
        ptr_ = static_cast<T*>(fftw_malloc(sizeof(T) * size_));
        if (ptr_ == nullptr) {
            throw std::bad_alloc();
        }
        memset(ptr_, 0, sizeof(T) * size_);
    }
    T* ptr_{nullptr};
    size_t size_{0};
    size_t rows_{1};
    size_t cols_{1};
    size_t stride_{1};
};

enum class fftw_op {
                // plan with
    R2R,        // fftw_plan_r2r_1d and FFTW_REDFT10 (DCT-II)
    DFT_R2C,    // fftw_plan_dft_r2c_1d
    DFT_C2R,    // fftw_plan_dft_c2r_1d
    DFT,        // fftw_plan_dft_1d
    DFT_2D,     // fftw_plan_many_dft
    DFT_R2C_2D, // fftw_plan_many_dft_r2c
};

class fftw_plan_manager {
public:
    fftw_plan_manager() = default;
    ~fftw_plan_manager()
    {
        clear_plans();
    }
    fftw_plan_manager(const fftw_plan_manager&) = delete;
    fftw_plan_manager& operator=(const fftw_plan_manager&) = delete;
    fftw_plan_manager(fftw_plan_manager&&) = delete;
    fftw_plan_manager& operator=(fftw_plan_manager&&) = delete;

    void set_plan_flag(int flag) { plan_flag_ = flag; }

    void clear_plans()
    {
        {
            std::unique_lock<std::mutex> lock(cache_1d_mutex_);
            for (auto &it : cache_1d_) {
                fftw_destroy_plan(it.second);
            }
            cache_1d_.clear();
        }
        {
            std::unique_lock<std::mutex> lock(cache_2d_mutex_);
            for (auto &it : cache_2d_) {
                fftw_destroy_plan(it.second);
            }
            cache_2d_.clear();
        }
        {
            std::unique_lock<std::mutex> lock(cache_2d_aligned_mutex_);
            for (auto &it : cache_2d_aligned_) {
                fftw_destroy_plan(it.second);
            }
            cache_2d_aligned_.clear();
        }
        fftw_forget_wisdom();
    }

    void execute(int n, double* in_out, fftw_r2r_kind kind) {
        fftw_plan plan = get_plan(n, fftw_op::R2R, static_cast<int>(kind));
        fftw_execute_r2r(plan, in_out, in_out);
    }

    void execute(int n, double* in, fftw_complex* out) {
        fftw_plan plan = get_plan(n, fftw_op::DFT_R2C, FFTW_FORWARD);
        fftw_execute_dft_r2c(plan, in, out);
    }

    void execute(const fftw_memory<double> &in, const fftw_memory<fftw_complex> &out) {
        fftw_plan plan = get_plan(in.size(), fftw_op::DFT_R2C, FFTW_FORWARD, true);
        fftw_execute_dft_r2c(plan, in.get(), out.get());
    }

    void execute(int n, fftw_complex* in, double* out) {
        fftw_plan plan = get_plan(n, fftw_op::DFT_C2R, FFTW_BACKWARD);
        fftw_execute_dft_c2r(plan, in, out);
    }

    void execute(int n, fftw_complex* in_out, int direction) {
        fftw_plan plan = get_plan(n, fftw_op::DFT, direction);
        fftw_execute_dft(plan, in_out, in_out);
    }

    void execute(const fftw_memory<fftw_complex> &in_out, int direction) {
        fftw_plan plan = get_plan(in_out.size(), fftw_op::DFT, direction, true);
        fftw_execute_dft(plan, in_out.get(), in_out.get());
    }

    void execute_2d(const fftw_memory<fftw_complex> &in_out, int direction) {
        auto stride = in_out.stride();
        fftw_plan plan = get_plan(in_out.rows(), in_out.cols(), stride, stride, fftw_op::DFT_2D, direction);
        fftw_execute_dft(plan, in_out.get(), in_out.get());
    }

    void execute_2d(int rows, int cols, fftw_complex *in_out, int direction) {
        fftw_plan plan = get_plan(rows, cols, fftw_op::DFT_2D, direction);
        fftw_execute_dft(plan, in_out, in_out);
    }

    void execute_2d(const fftw_memory<double> &in, const fftw_memory<fftw_complex> &out) {
        fftw_plan plan = get_plan(in.rows(), in.cols(), in.stride(), out.stride(), fftw_op::DFT_R2C_2D, FFTW_FORWARD);
        fftw_execute_dft_r2c(plan, in.get(), out.get());
    }

private:
    using plan_key_1d = std::tuple<size_t, fftw_op, int>;
    using plan_key_2d = std::tuple<size_t, size_t, fftw_op, int>;
    fftw_plan get_plan(size_t rows, size_t cols, fftw_op op, int type)
    {
        auto key = std::make_tuple(rows, cols, op, type);
        std::unique_lock<std::mutex> lock(cache_2d_aligned_mutex_);
        auto cached = cache_2d_aligned_.find(key);
        if (cached != cache_2d_aligned_.end()) {
            return cached->second;
        }

        fftw_plan newPlan = nullptr;
        int flag = check_size(rows * cols, plan_flag_);
        flag |= FFTW_UNALIGNED; // NOLINT(hicpp-signed-bitwise)
        switch (op) {
            case fftw_op::DFT_2D: {
                fftw_memory<fftw_complex> tmp(rows * cols);
                auto *buf = tmp.get();
                newPlan = fftw_plan_dft_2d(rows, cols, buf, buf, type, flag);
                break;
            }
            case fftw_op::DFT_R2C_2D: {
                fftw_memory<double> in(rows, cols);
                fftw_memory<fftw_complex> out(rows, (cols / 2) + 1);
                newPlan = fftw_plan_dft_r2c_2d(rows, cols, in.get(), out.get(), flag);
                break;
            }
            default: break;
        }
        cache_2d_aligned_[key] = newPlan;
        return newPlan;
    }
    fftw_plan get_plan(size_t rows, size_t cols, size_t in_stride, size_t out_stride, // NOLINT(bugprone-easily-swappable-parameters)
            fftw_op op, int type)
    {
        auto key = std::make_tuple(rows, cols, op, type);
        std::unique_lock<std::mutex> lock(cache_2d_aligned_mutex_);
        auto cached = cache_2d_aligned_.find(key);
        if (cached != cache_2d_aligned_.end()) {
            return cached->second;
        }

        fftw_plan newPlan = nullptr;
        int flag = check_size(rows * cols, plan_flag_);
        std::array<int,2> n {static_cast<int>(rows), static_cast<int>(cols)};
        std::array<int,2> inembed {static_cast<int>(rows), static_cast<int>(in_stride)};
        std::array<int,2> onembed {static_cast<int>(rows), static_cast<int>(out_stride)};
        switch (op) {
            case fftw_op::DFT_2D: {
                fftw_memory<fftw_complex> tmp(rows, cols);
                newPlan = fftw_plan_many_dft(2, n.data(), 1,
                    tmp.get(), inembed.data(), 1, 0,
                    tmp.get(), inembed.data(), 1, 0,
                type, flag);
                break;
            }
            case fftw_op::DFT_R2C_2D: {
                fftw_memory<double> in(rows, cols);
                fftw_memory<fftw_complex> out(rows, cols);
                newPlan = fftw_plan_many_dft_r2c(2, n.data(), 1,
                    in.get(), inembed.data(), 1, 0,
                    out.get(), onembed.data(), 1, 0,
                flag);
                break;
            }
            default: break;
        }
        cache_2d_aligned_[key] = newPlan;
        return newPlan;
    }
    fftw_plan get_plan(size_t n, fftw_op op, int type, bool aligned = false)
    {
        auto key = std::make_tuple(n, op, type);
        std::unique_lock<std::mutex> lock(cache_1d_mutex_);
        auto cached = cache_1d_.find(key);
        if (cached != cache_1d_.end()) {
            return cached->second;
        }

        fftw_plan newPlan = nullptr;
        int flag = check_size(n, plan_flag_);
        if (!aligned) {
            flag |= FFTW_UNALIGNED; // NOLINT(hicpp-signed-bitwise)
        }
        switch (op) {
            case fftw_op::R2R: {
                fftw_memory<double> temp(n);
                newPlan = fftw_plan_r2r_1d(n, temp.get(), temp.get(), (fftw_r2r_kind)type, flag);
                break;
            }
            case fftw_op::DFT_R2C: {
                fftw_memory<double> temp_in(n);
                fftw_memory<fftw_complex> temp_out((n / 2) + 1);
                newPlan = fftw_plan_dft_r2c_1d(n, temp_in.get(), temp_out.get(), flag);
                break;
            }
            case fftw_op::DFT_C2R: {
                fftw_memory<fftw_complex> temp_in(n);
                fftw_memory<double> temp_out(n);
                newPlan = fftw_plan_dft_c2r_1d(n, temp_in.get(), temp_out.get(), flag);
                break;
            }
            case fftw_op::DFT: {
                fftw_memory<fftw_complex> temp(n);
                newPlan = fftw_plan_dft_1d(n, temp.get(), temp.get(), type, flag);
                break;
            }
            default: break;
        }
        cache_1d_[key] = newPlan;
        return newPlan;
    }

    int check_size(size_t size, int flag)
    {
        if (size >= OMP_THRESHOLD) {
            fftw_plan_with_nthreads(omp_get_num_procs());
        } else {
            fftw_plan_with_nthreads(1);
        }
        if (size > MEASURE_THRESHOLD) {
            flag = FFTW_ESTIMATE;
        }
        return flag;
    }

    std::mutex cache_1d_mutex_;
    std::map<plan_key_1d, fftw_plan> cache_1d_;
    std::mutex cache_2d_mutex_;
    std::map<plan_key_2d, fftw_plan> cache_2d_;
    std::mutex cache_2d_aligned_mutex_;
    std::map<plan_key_2d, fftw_plan> cache_2d_aligned_;

    std::atomic_int plan_flag_ { FFTW_ESTIMATE };

    constexpr static int MEASURE_THRESHOLD {1000000};
};

inline void fft_scale(fftw_complex *buf, size_t n, double value)
{
#pragma omp parallel for schedule(static) if(n > OMP_THRESHOLD)
    for (size_t i = 0; i < n; i++) {
        buf[i][0] *= value;
        buf[i][1] *= value;
    }
}

inline void fft_scale_fill(fftw_complex *buf, size_t n, double scale)
{
    buf[0][0] *= scale;
    buf[0][1] *= scale;
    if (n % 2 == 0) {
        buf[n / 2][0] *= scale;
        buf[n / 2][1] *= scale;
    }
    size_t half = (n - 1) / 2;
#pragma omp parallel for schedule(static) if(n > OMP_THRESHOLD)
    for (size_t i = 1; i <= half; i++) {
        buf[i][0] *= scale;
        buf[i][1] *= scale;
        int target = n - i;
        buf[target][0] = buf[i][0];
        buf[target][1] = -buf[i][1];
    }
}

inline void fft_scale_fill(fftw_memory<fftw_complex> &buf, double scale)
{
    auto rows = buf.rows();
    auto cols = buf.cols();
    size_t n = rows * cols;
    size_t half_c = cols / 2;
#pragma omp parallel for schedule(static) if(n > OMP_THRESHOLD)
    for (size_t i = 0; i < rows; i++) {
        fftw_complex *row = buf[i];
        for (size_t j = 0; j <= half_c; j++) {
            row[j][0] *= scale;
            row[j][1] *= scale;
        }
    }
    fftw_complex *row = buf[0];
#pragma omp parallel for schedule(static) if(n > OMP_THRESHOLD)
    for (size_t j = half_c + 1; j < cols; ++j) {
        size_t src_j = cols - j;
        row[j][0] = row[src_j][0];
        row[j][1] = -row[src_j][1];
    }
#pragma omp parallel for schedule(static) if(n > OMP_THRESHOLD)
    for (size_t i = 1; i < rows; i++) {
        fftw_complex *row = buf[i];
        fftw_complex *src_row = buf[rows - i];
        for (size_t j = half_c + 1; j < cols; ++j) {
            size_t src_j = cols - j;
            row[j][0] = src_row[src_j][0];
            row[j][1] = -src_row[src_j][1];
        }
    }
}
