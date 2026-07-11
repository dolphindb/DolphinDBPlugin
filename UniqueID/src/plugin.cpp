#include "plugin.h"

#include "IDGenerator.h"

using namespace ddb; // NOLINT(google-build-using-namespace)

IDGeneratorManager &manager = IDGeneratorManager::getInstance();

void closeUidGen(Heap *, vector<ConstantSP> &) {}

ConstantSP uid_create_generator(Heap *heap, argsT &args)
{
    if (args[0]->getForm() != DF_SCALAR || args[0]->getType() != DT_STRING) {
        throw IllegalArgumentException("createGenerator", "must imput a string as name");
    }
    string name = args[0]->getString();
    long long id = 1;
    if (args.size() == 2) {
        if (args[1]->getForm() == DF_SCALAR && args[1]->getCategory() == DATA_CATEGORY::INTEGRAL) {
            id = args[1]->getLong();
        } else {
            throw IllegalArgumentException("createGenerator", "id must be a INTEGRAL");
        }
    }
    long long address = manager.createNewGenerator(name, id);
    FunctionDefSP onClose(Util::createSystemProcedure("uidGen onClose()", closeUidGen, 1, 1));
    ConstantSP resource = Util::createResource(address, ("uidGen"), onClose, heap);
    return resource;
}

ConstantSP uid_find_generator(Heap *heap, argsT &args)
{
    if (args[0]->getForm() != DF_SCALAR || args[0]->getType() != DT_STRING) {
        throw IllegalArgumentException("getGenerator", "must imput a string as name");
    }
    string name = args[0]->getString();
    long long address = manager.getGenerator(name);

    FunctionDefSP onClose(Util::createSystemProcedure("uidGen onClose()", closeUidGen, 1, 1));
    ConstantSP resource = Util::createResource(address, ("uidGen"), onClose, heap);
    return resource;
}

ConstantSP uid_generate(Heap *heap, argsT &args)
{
    std::ignore = heap;
    if (args[0]->getForm() != DF_SCALAR || args[0]->getType() != DT_RESOURCE) {
        throw IllegalArgumentException("newUid", "need a resource parameter");
    }

    ResourceSP resource = args[0];

    int size = 1;
    if (args.size() == 2) {
        if (args[1]->getForm() == DF_SCALAR && args[1]->getType() == DT_INT) {
            size = args[1]->getInt();
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

ConstantSP uid_list_generators(Heap *heap, argsT &args)
{
    std::ignore = heap;
    std::ignore = args;
    return manager.listGenerator();
}

ConstantSP uid_destroy_generator(Heap *heap, argsT &args)
{
    std::ignore = heap;
    if (args[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException("destroyGenerator", "need a resource or string parameter");
    }
    if (args[0]->getType() == DT_STRING) {
        manager.destroyGenerator(args[0]->getString());
    } else if (args[0]->getType() == DT_RESOURCE) {
        ResourceSP resource = args[0];
        manager.destroyGenerator(resource->getLong());
    } else {
        throw IllegalArgumentException("destroyGenerator", "need a resource or string parameter");
    }
    return new Void();
}
