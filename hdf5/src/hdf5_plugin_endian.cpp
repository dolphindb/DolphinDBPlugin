#include <hdf5_plugin_util.h>
#include <hdf5_plugin_imp.h>
#include "Types.h"

uint32_t swapEndian32(uint32_t n)
{
    return ((n & 0x000000FFu) << 24) |
           ((n & 0x0000FF00u) << 8) |
           ((n & 0x00FF0000u) >> 8) |
           ((n & 0xFF000000u) >> 24);
}

uint64_t swapEndian64(uint64_t n)
{
    return ((n & 0x00000000000000ffULL) << 56) |
           ((n & 0x000000000000ff00ULL) << 40) |
           ((n & 0x0000000000ff0000ULL) << 24) |
           ((n & 0x00000000ff000000ULL) << 8) |
           ((n & 0x000000ff00000000ULL) >> 8) |
           ((n & 0x0000ff0000000000ULL) >> 24) |
           ((n & 0x00ff000000000000ULL) >> 40) |
           ((n & 0xff00000000000000ULL) >> 56);
}

void registerUnixTimeConvert()
{
    static bool done = false;
    if (done)
        return;

    H5T_conv_t v = [](hid_t src_id,
                      hid_t dst_id,
                      H5T_cdata_t *cdata,
                      size_t nelmts,
                      size_t buf_stride,
                      size_t bkg_stride,
                      void *buf,
                      void *bkg,
                      hid_t dset_xfer_plist) -> herr_t {
        if (cdata->command == H5T_CONV_INIT)
        {
            cdata->need_bkg = H5T_BKG_NO;
            if (H5Tequal(src_id, H5T_UNIX_D32BE) && H5Tequal(dst_id, H5T_UNIX_D32LE))
                return 0;
            if (H5Tequal(src_id, H5T_UNIX_D32LE) && H5Tequal(dst_id, H5T_UNIX_D32BE))
                return 0;
            return -1;
        }
        else if (cdata->command == H5T_CONV_CONV)
        {
            buf_stride = buf_stride ? buf_stride : H5Tget_size(dst_id);
            for (size_t i = 0; i != nelmts; i++)
            {
                char *b = (char *)buf + buf_stride * i;
                uint32_t *n = (uint32_t *)b;
                *n = swapEndian32(*n);
            }
        }

        return 0;
    };

    H5Tregister(H5T_PERS_SOFT, "a", H5T_UNIX_D32BE, H5T_UNIX_D32LE, v);
    H5Tregister(H5T_PERS_SOFT, "b", H5T_UNIX_D32LE, H5T_UNIX_D32BE, v);

    H5T_conv_t p = [](hid_t src_id,
                      hid_t dst_id,
                      H5T_cdata_t *cdata,
                      size_t nelmts,
                      size_t buf_stride,
                      size_t bkg_stride,
                      void *buf,
                      void *bkg,
                      hid_t dset_xfer_plist) -> herr_t {
        if (cdata->command == H5T_CONV_INIT)
        {
            cdata->need_bkg = H5T_BKG_NO;
            if (H5Tequal(src_id, H5T_UNIX_D64BE) && H5Tequal(dst_id, H5T_UNIX_D64LE))
                return 0;
            if (H5Tequal(src_id, H5T_UNIX_D64LE) && H5Tequal(dst_id, H5T_UNIX_D64BE))
                return 0;
            return -1;
        }
        else if (cdata->command == H5T_CONV_CONV)
        {
            buf_stride = buf_stride ? buf_stride : H5Tget_size(dst_id);
            for (size_t i = 0; i != nelmts; i++)
            {
                char *b = (char *)buf + buf_stride * i;
                uint64_t *n = (uint64_t *)b;
                *n = swapEndian64(*n);
            }
        }

        return 0;
    };

    H5Tregister(H5T_PERS_SOFT, "c", H5T_UNIX_D64LE, H5T_UNIX_D64BE, p);
    H5Tregister(H5T_PERS_SOFT, "d", H5T_UNIX_D64BE, H5T_UNIX_D64LE, p);

    H5T_conv_t kk = [](hid_t src_id,
                       hid_t dst_id,
                       H5T_cdata_t *cdata,
                       size_t nelmts,
                       size_t buf_stride,
                       size_t bkg_stride,
                       void *buf,
                       void *bkg,
                       hid_t dset_xfer_plist) -> herr_t {
        if (cdata->command == H5T_CONV_INIT)
        {
            cdata->need_bkg = H5T_BKG_NO;
            bool src_ok = H5Tequal(src_id, H5T_UNIX_D32LE) || H5Tequal(src_id, H5T_UNIX_D32BE);
            bool dst_ok = H5Tequal(dst_id, H5T_UNIX_D64LE) || H5Tequal(dst_id, H5T_UNIX_D64BE);

            return src_ok && dst_ok ? 0 : -1;
        }
        else if (cdata->command == H5T_CONV_CONV)
        {
            H5T_order_t st = H5Tget_order(src_id);
            H5T_order_t dt = H5Tget_order(dst_id);

            bool is_little_endian = Util::isLittleEndian();

            size_t src_stride = buf_stride ? buf_stride : H5Tget_size(src_id);
            size_t dest_stride = buf_stride ? buf_stride : H5Tget_size(dst_id);

            for (int i = int(nelmts - 1); i >= 0; i--)
            {
                char *b = (char *)buf + src_stride * i;
                uint32_t n = *(uint32_t *)b;
                if (is_little_endian && st == H5T_ORDER_BE)
                    n = swapEndian32(n);
                else if (!is_little_endian && st == H5T_ORDER_LE)
                    n = swapEndian32(n);

                uint64_t m = (uint64_t)n * 1000; //seconds to millseconds;

                if (is_little_endian && dt == H5T_ORDER_BE)
                    m = swapEndian64(m);
                else if (!is_little_endian && dt == H5T_ORDER_LE)
                    m = swapEndian64(m);

                b = (char *)buf + dest_stride * i;
                *(uint64_t *)b = m;
            }
        }

        return 0;
    };

    H5Tregister(H5T_PERS_HARD, "e", H5T_UNIX_D32LE, H5T_UNIX_D64LE, kk);
    H5Tregister(H5T_PERS_HARD, "f", H5T_UNIX_D32LE, H5T_UNIX_D64BE, kk);
    H5Tregister(H5T_PERS_HARD, "g", H5T_UNIX_D32BE, H5T_UNIX_D64LE, kk);
    H5Tregister(H5T_PERS_HARD, "h", H5T_UNIX_D32BE, H5T_UNIX_D64BE, kk);

    done = true;
}