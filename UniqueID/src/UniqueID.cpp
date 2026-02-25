#include "UniqueID.h"

#include "IDGenerator.h"

namespace ddb {

IDGeneratorManager &manager = IDGeneratorManager::getInstance();

void closeUidGen(Heap *, vector<ConstantSP> &) {}

ConstantSP createGenerator(Heap *heap, vector<ConstantSP> &arguments) {
    if (arguments[0]->getForm() != DF_SCALAR || arguments[0]->getType() != DT_STRING) {
        throw IllegalArgumentException("createGenerator", "must imput a string as name");
    }
    string name = arguments[0]->getString();
    long long id = 1;
    if (arguments.size() == 2) {
        if (arguments[1]->getForm() == DF_SCALAR && arguments[1]->getCategory() == DATA_CATEGORY::INTEGRAL) {
            id = arguments[1]->getLong();
        } else {
            throw IllegalArgumentException("createGenerator", "id must be a INTEGRAL");
        }
    }
    long long address = manager.createNewGenerator(name, id);
    FunctionDefSP onClose(Util::createSystemProcedure("uidGen onClose()", closeUidGen, 1, 1));
    ConstantSP resource = Util::createResource(address, ("uidGen"), onClose, heap);
    return resource;
}

ConstantSP getGenerator(Heap *heap, vector<ConstantSP> &arguments) {
    if (arguments[0]->getForm() != DF_SCALAR || arguments[0]->getType() != DT_STRING) {
        throw IllegalArgumentException("getGenerator", "must imput a string as name");
    }
    string name = arguments[0]->getString();
    long long address = manager.getGenerator(name);

    FunctionDefSP onClose(Util::createSystemProcedure("uidGen onClose()", closeUidGen, 1, 1));
    ConstantSP resource = Util::createResource(address, ("uidGen"), onClose, heap);
    return resource;
}

ConstantSP newUid(Heap *, vector<ConstantSP> &arguments) {
    if (arguments[0]->getForm() != DF_SCALAR || arguments[0]->getType() != DT_RESOURCE) {
        throw IllegalArgumentException("newUid", "need a resource parameter");
    }

    ResourceSP resource = arguments[0];

    int size = 1;
    if (arguments.size() == 2) {
        if (arguments[1]->getForm() == DF_SCALAR && arguments[1]->getType() == DT_INT) {
            size = arguments[1]->getInt();
        } else {
            throw IllegalArgumentException("newUid", "size must be a int");
        }
    }

    if (size <= 0) {
        throw IllegalArgumentException("newUid", "size must be a positive integer");
    }

    if (size == 1) {
        return new Long(manager.newId(resource->getLong(), size));
    }

    long long firstId = manager.newId(resource->getLong(), size);
    vector<long long> ids(size);
    for (long long i = 0; i < size; i++) {
        ids[i] = firstId + i;
    }
    VectorSP res = Util::createVector(DATA_TYPE::DT_LONG, 0, size);
    res->appendLong(ids.data(), ids.size());

    return res;
}

ConstantSP listGenerator(Heap *, vector<ConstantSP> &) {
    return manager.listGenerator();
}

ConstantSP destroyGenerator(Heap *, vector<ConstantSP> &arguments) {
    if (arguments[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException("destroyGenerator", "need a resource or string parameter");
    }
    if (arguments[0]->getType() == DT_STRING) {
        manager.destroyGenerator(arguments[0]->getString());
    } else if (arguments[0]->getType() == DT_RESOURCE) {
        ResourceSP resource = arguments[0];
        manager.destroyGenerator(resource->getLong());
    } else {
        throw IllegalArgumentException("destroyGenerator", "need a resource or string parameter");
    }
    return new Void();
}

} // namespace ddb;
