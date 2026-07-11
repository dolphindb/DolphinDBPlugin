#pragma once

#include <atomic>
#include <mutex>
#include <thread>

#include "DolphinDBEverything.h"
#include "ScalarImp.h"
#include "LocklessContainer.h"

class SpinLock {
  private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;

  public:
    void lock() {
        while (flag.test_and_set()) {}
    }

    void unlock() { flag.clear(); }
};

namespace ddb {

class IDGenerator {
  public:
    explicit IDGenerator(string name, long lastId) : name_(name), lastId_(lastId) {}
    long newId(int size) {
        long current = lastId_.load();
        while (true) {
            if (current > std::numeric_limits<long>::max() - size) {
                throw RuntimeException("ID overflow!");
            }

            if (lastId_.compare_exchange_weak(current, current + size)) {
                return current;
            }
        }
    }

    long getLastId() { return lastId_.load(); }
    string getName() { return name_; }

  private:
    string name_;
    std::atomic<long> lastId_;
};

class IDGeneratorManager {
  public:
    static IDGeneratorManager &getInstance() {
        static IDGeneratorManager instance;
        return instance;
    }

    long long createNewGenerator(string name, long long lastId);
    ConstantSP listGenerator();
    void destroyGenerator(long long address);
    void destroyGenerator(string name);
    long long getGenerator(string name);
    long long newId(long long address, int size);

  private:
    std::unordered_map<std::string, long long> nameGenerators_;
    LocklessHashmap<long long, std::shared_ptr<IDGenerator>> resourceGenerators_;
    SpinLock spinlock_;

    IDGeneratorManager() = default;
    IDGeneratorManager(const IDGeneratorManager &) = delete;
    IDGeneratorManager &operator=(const IDGeneratorManager &) = delete;
};

} // namespace ddb;
