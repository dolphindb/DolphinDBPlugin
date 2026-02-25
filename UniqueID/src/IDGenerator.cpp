#include "IDGenerator.h"
#include "DolphinDBEverything.h"
#include "CoreConcept.h"

namespace ddb {

long long IDGeneratorManager::createNewGenerator(string name, long long lastId) {
    std::unique_lock<SpinLock> lck(spinlock_);
    if (nameGenerators_.count(name)) {
        throw RuntimeException("generator already exists");
    }
    auto generator = std::make_shared<IDGenerator>(name, lastId);
    long long address = reinterpret_cast<long long>(generator.get());
    
    resourceGenerators_.insert(address, generator);
    nameGenerators_[name] = address;

    return address;
}

long long IDGeneratorManager::getGenerator(string name) {
    std::unique_lock<SpinLock> lck(spinlock_);
    if (!nameGenerators_.count(name)) {
        throw RuntimeException("generator not found");
    }

    long long res = nameGenerators_[name];

    return res;
}

ConstantSP IDGeneratorManager::listGenerator() {
    std::unordered_map<std::string, long long> nameGeneratorsCopy;
    {
        std::unique_lock<SpinLock> lck(spinlock_);
        nameGeneratorsCopy = nameGenerators_;
    }

    VectorSP nameCol = Util::createVector(DATA_TYPE::DT_STRING, 0, nameGeneratorsCopy.size());
    VectorSP idCol = Util::createVector(DATA_TYPE::DT_LONG, 0, nameGeneratorsCopy.size());

    vector<string> names;
    vector<long long> ids;

    for (auto it = nameGeneratorsCopy.begin(); it != nameGeneratorsCopy.end(); ++it) {
        const std::string& name = it->first;
        long long address = it->second;

        std::shared_ptr<IDGenerator> recv;
        if (resourceGenerators_.find(address, recv)) {
            names.push_back(name);
            ids.push_back(recv->getLastId());
        }
    }
    
    nameCol->appendString(names.data(), names.size());
    idCol->appendLong(ids.data(), ids.size());

    return Util::createTable(vector<string>{"name", "id"}, vector<ConstantSP>{nameCol, idCol});
}

void IDGeneratorManager::destroyGenerator(long long address) {
    std::unique_lock<SpinLock> lck(spinlock_);
    std::shared_ptr<IDGenerator> recv;
    if (!resourceGenerators_.find(address, recv)) {
        throw RuntimeException("generator not found");
    }
    string name = recv->getName();
    resourceGenerators_.erase(address);
    nameGenerators_.erase(name);
}

void IDGeneratorManager::destroyGenerator(string name) {
    std::unique_lock<SpinLock> lck(spinlock_);
    if (!nameGenerators_.count(name)) {
        throw RuntimeException("generator not found");
    }
    long long address = nameGenerators_[name];
    resourceGenerators_.erase(address);
    nameGenerators_.erase(name);
}

long long IDGeneratorManager::newId(long long address, int size) {
    std::shared_ptr<IDGenerator> generator;
    if (!resourceGenerators_.find(address, generator)) {
        throw RuntimeException("generator not found");
    }
    return generator->newId(size);
}

} // namespace ddb;
