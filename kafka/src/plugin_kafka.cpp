#include "plugin_kafka.h"
#include "Concurrent.h"
#include "Exceptions.h"
#include "Types.h"
#include "Util.h"
#include <cppkafka/producer.h>
#include <cppkafka/logging.h>
#include <cppkafka/error.h>
#include <exception>
#include <memory>
#include <mutex>
#include <string>
#include "ddbplugin/CommonInterface.h"

using namespace cppkafka;
using namespace std;

static long long int BUFFER_SIZE = 900000;
static long long int MESSAGE_SIZE = 10000;
const static double FACTOR = 0.95;

Mutex HANDLE_MUTEX;
set<long long> HANDLE_SET;
// use atomic timestamp to indicate the last assign time.
// ddb would crash if the internal between assign() & unassign() is too short
static atomic<int64_t> ASSIGN_TIMEOUT;
// concurrently using assign related function would cause crash, use a mutex to avoid crash
// alert! It's a little inefficient, but assign operation would not be called frequently
static Mutex ASSIGN_LATCH;
static Mutex DICT_LATCH;
const static int TYPE_SIZE[17] = {0,1,1,2,4,8,4,4,4,4,4,8,8,8,8,4,8};
static DictionarySP STATUS_DICT = Util::createDictionary(DT_STRING,nullptr,DT_ANY,nullptr);

static Defer destruct = Defer([](){
    STATUS_DICT->clear();
});

const static string PRODUCER_DESC = "kafka producer connection";
const static string CONSUMER_DESC = "kafka consumer connection";
const static string QUEUE_DESC = "kafka queue connection";
const static string EVENT_DESC = "kafka event connection";

static void kafkaProducerOnClose(Heap *heap, vector<ConstantSP> &args) {
    try {
        LockGuard<Mutex> _(&HANDLE_MUTEX);
        if(HANDLE_SET.find(args[0]->getLong()) == HANDLE_SET.end()) {
            throw IllegalArgumentException(__FUNCTION__, "Invalid producer connection object.");
        }
        KafkaWrapper<Producer> * wrapper = (KafkaWrapper<Producer>*)(args[0]->getLong());
        Producer* producer;
        if(wrapper!= nullptr) {
            producer = wrapper->getDataPtr();
            if(producer != nullptr) {
                producer->flush();
            }
            if(wrapper != nullptr) {
                long long handleLong = args[0]->getLong();
                args[0]->setLong(0);
                delete wrapper;
                HANDLE_SET.erase(handleLong);
            }
        } else {
            throw IllegalArgumentException(__FUNCTION__, "Invalid producer object.");
        }
        // auto producer = (Producer *) args[0]->getLong();
    } catch(std::exception& exception) {
        LOG_ERR(string("[PLUGIN::KAFKA] Producer destruction failed: ") + exception.what());
    }
}

void drain(Consumer * consumer) {
    auto start_time = Util::getEpochTime();
    cppkafka::Error last_error(RD_KAFKA_RESP_ERR_NO_ERROR);

    while (true) {
        auto msg = consumer->poll(std::chrono:: milliseconds(100));
        if (!msg)
            break;

        auto error = msg.get_error();

        if (error) {
            if (msg.is_eof() || error == last_error){
                break;
            } else {
                LOG_ERR("[PLUGIN::KAFKA] Timeout during draining.");
            }
        }

        // i don't stop draining on first error,
        // only if it repeats once again sequentially
        last_error = error;

        auto ts = Util::getEpochTime();
        if (ts-start_time > 5000) {
            LOG_ERR("[PLUGIN::KAFKA] Timeout during draining.");
            break;
        }
    }
}

static void kafkaConsumerOnClose(Heap *heap, vector<ConstantSP> &args) {
    try {
        LockGuard<Mutex> _(&HANDLE_MUTEX);
        if(HANDLE_SET.find(args[0]->getLong()) == HANDLE_SET.end()) {
            throw IllegalArgumentException(__FUNCTION__, "Invalid consumer connection object.");
        }
        KafkaWrapper<Consumer> * wrapper = (KafkaWrapper<Consumer>*)(args[0]->getLong());
        Consumer* consumer;
        if(wrapper!= nullptr) {
            consumer = wrapper->getDataPtr();

            LockGuard<Mutex> _(&DICT_LATCH);
            long long consumerValue = args[0]->getLong();
            ConstantSP keys = STATUS_DICT->keys();
            for(int i = 0; i < keys->size(); i++){
                string key = keys->getString(i);
                ConstantSP conn = STATUS_DICT->getMember(key);
                if(!conn->isNull() && conn->getLong() != 0) {
                    SubConnection *sc = (SubConnection *)(conn->getLong());
                    if (sc->getConsumerLong() == consumerValue) {
                        throw RuntimeException("The consumer [" + std::to_string(consumerValue) +
                            "] is still used in subJob [" + std::to_string(conn->getLong()) + "].");
                    }
                }
            }
            if(consumer!= nullptr) {
                consumer->pause();
                // consumer->set_destroy_flags(RD_KAFKA_DESTROY_F_NO_CONSUMER_CLOSE);
                consumer->unsubscribe();
                consumer->unassign();
                drain(consumer);
            }
            long long handleLong = args[0]->getLong();
            args[0]->setLong(0);
            delete wrapper;
            HANDLE_SET.erase(handleLong);
        } else {
            throw IllegalArgumentException(__FUNCTION__, "Invalid consumer object.");
        }

    } catch(std::exception& exception) {
        LOG_ERR(string("[PLUGIN::KAFKA] Consumer destruction failed: ") + exception.what());
    }
}

ConstantSP kafkaProducer(Heap *heap, vector<ConstantSP> &args) {
    auto &dict = args[0];
    const auto usage = string("Usage: producer(dict[string, any]).\n");
    if (dict->getForm() != DF_DICTIONARY) {
        throw IllegalArgumentException(__FUNCTION__, usage + "Not a dict config.");
    }
    auto conf = createConf(dict);

    try {
        auto producer = new KafkaWrapper<Producer>(new Producer(conf));
        FunctionDefSP onClose(Util::createSystemProcedure("kafka producer onClose()", kafkaProducerOnClose, 1, 1));
        LockGuard<Mutex> _(&HANDLE_MUTEX);
        HANDLE_SET.insert((long long) producer);
        return Util::createResource(
                (long long) producer,
                PRODUCER_DESC,
                onClose,
                heap->currentSession()
        );
    } catch(std::exception &exception) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + exception.what());
    }
}

ConstantSP kafkaProducerFlush(Heap *heap, vector<ConstantSP> &args) {
    const auto usage = string("Usage: produceFlush(producer).\n");
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != PRODUCER_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "producer should be a producer handle.");
    try {
        RWLockGuard<RWLock> lockGuard(NULL, false);
        getWrapperLockGuard<Producer>(args[0], lockGuard);
        getConnection<Producer>(args[0])->flush();
    } catch(std::exception &exception) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + exception.what());
    }

    return new Void();
}

ConstantSP kafkaProduce(Heap *heap, vector<ConstantSP> &args) {
    try {
        if(args.size() == 5){
            ConstantSP temp = Util::createNullConstant(DT_ANY);
            produceMessage(args[0], args[1], args[2], args[3], args[4], temp);
        }
        else if(args.size() == 6) {
            produceMessage(args[0], args[1], args[2], args[3], args[4], args[5]);
        }
    } catch(std::exception &exception) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + exception.what());
    }

    return new Void();
}

ConstantSP kafkaConsumer(Heap *heap, vector<ConstantSP> &args) {
    const auto usage = string("Usage: consumer(dict[string, any]).\n");
    auto &dict = args[0];

    if (dict->getForm() != DF_DICTIONARY) {
        throw IllegalArgumentException(__FUNCTION__, usage + "Not a dict config.");
    }

    auto conf = createConf(args[0], true);
    try {
        auto consumer = new KafkaWrapper<Consumer>(new Consumer(conf));
        FunctionDefSP onClose(Util::createSystemProcedure("kafka consumer onClose()", kafkaConsumerOnClose, 1, 1));
        LockGuard<Mutex> _(&HANDLE_MUTEX);
        HANDLE_SET.insert((long long) consumer);
        return Util::createResource(
                (long long) consumer,
                CONSUMER_DESC,
                onClose,
                heap->currentSession()
        );
    } catch(std::exception &exception) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + exception.what());
    }
}

ConstantSP kafkaSubscribe(Heap *heap, vector<ConstantSP> &args) {
    const auto usage = string("Usage: subscribe(consumer, topics).\n");
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");

    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto conn = getConnection<Consumer>(args[0]);
    if (args[1]->getForm() != DF_VECTOR || args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "Not a topic vector.");
    }
    auto &vec = args[1];
    auto topics = vector<string>{};
    for (auto i = 0; i < vec->size(); i++) {
        topics.push_back(vec->get(i)->getString());
    }
    try  {
        conn->subscribe(topics);
    } catch(std::exception &exception) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + exception.what());
    }
    return new Void();
}


ConstantSP extractMessage(Message &msg) {
    auto topic = Util::createConstant(DT_STRING);
    auto partition = Util::createConstant(DT_INT);
    ConstantSP key, value;
    try {
        // key = kafkaDeserialize(msg.get_key());
        key = new String(string(msg.get_key()));
        value = kafkaDeserialize(msg.get_payload());
    } catch(std::exception& exception) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + exception.what());
    }
    auto ts = Util::createConstant(DT_TIMESTAMP);

    topic->setString(msg.get_topic());
    partition->setInt(msg.get_partition());
    if (msg.get_timestamp()) {
        ts->setLong(msg.get_timestamp()->get_timestamp().count());
    }

    auto res = Util::createVector(DT_ANY, 5);
    res->set(0, topic);
    res->set(1, key);
    res->set(2, value);
    res->set(3, partition);
    res->set(4, ts);
    return res;
}

ConstantSP kafkaConsumerPoll(Heap *heap, vector<ConstantSP> &args) {
    const auto usage = string(
            "Usage: consumerPoll(consumer, [timeout]).\n"
            "return: [err, msg]\n"
            "err: empty if no error else error info string.\n"
            "msg: [topic, partition, key, value, timestamp].\n"
    );

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto consumer = getConnection<Consumer>(args[0]);

    try{
        // Try to consume a message
        if(args.size() == 2){
            if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
                throw IllegalArgumentException(__FUNCTION__, + "time need positive integer");
            }
            auto time = args[1]->getInt();
            auto msg = consumer->poll(std::chrono:: milliseconds(time));

            // DPLG-275
            for(int i = 0; i < 10; ++i) {
                if(msg && msg.get_error() && msg.is_eof()) {
                    msg = consumer->poll(std::chrono:: milliseconds(time));
                } else {
                    break;
                }
            }
            return getMsg(msg);
        }else{
            auto msg = consumer->poll();
            // DPLG-275
            for(int i = 0; i < 10; ++i) {
                if(msg && msg.get_error() && msg.is_eof()) {
                    msg = consumer->poll();
                } else {
                    break;
                }
            }
            return getMsg(msg);
        }
    } catch(std::exception &exception) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + exception.what());
    }
}

ConstantSP kafkaPollByteStream(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: pollByteStream(consumer, [timeout]).\n"
            "return: [err/byte_stream]\n"
    );

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto consumer = getConnection<Consumer>(args[0]);
    Message msg;

    // Try to consume a message
    if(args.size() == 2){
        if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
            throw IllegalArgumentException(__FUNCTION__, + "time need positive integer");
        }
        auto time = args[1]->getInt();
        msg = consumer->poll(std::chrono:: milliseconds(time));
        // DPLG-275
        for(int i = 0; i < 10; ++i) {
            if(msg && msg.get_error() && msg.is_eof()) {
                msg = consumer->poll(std::chrono:: milliseconds(time));
            } else {
                break;
            }
        }
    }else{
        msg = consumer->poll();
        // DPLG-275
        for(int i = 0; i < 10; ++i) {
            if(msg && msg.get_error() && msg.is_eof()) {
                msg = consumer->poll();
            } else {
                break;
            }
        }
    }

    auto result = Util::createConstant(DT_STRING);
    if (msg) {
        if (msg.get_error()) {
            if (msg.is_eof())
                result->setString("Broker: No more messages");
            else
                result->setString(msg.get_error().to_string());
        }
        else {
            string dataInit = string(msg.get_payload());
            try {
                if(dataInit.size() == 0) {
                    result->setString(dataInit);
                    return result;
                }
                short flag;
                IO_ERR ret;
                auto str = dataInit.c_str();
                if(!(str[0] == '{' || str[0] == '[')){
                    DataInputStreamSP in = new DataInputStream(str, dataInit.length());
                    ret = in->readShort(flag);
                    auto data_form = static_cast<DATA_FORM>(flag >> 8);
                    ConstantUnmarshalFactory factory(in, nullptr);
                    ConstantUnmarshal* unmarshall = factory.getConstantUnmarshal(data_form);
                    if(unmarshall == nullptr){
                        throw RuntimeException("");
                    }
                    if (!unmarshall->start(flag, true, ret)) {
                        unmarshall->reset();
                        throw IOException("Failed to parse the incoming object with IO error type " + std::to_string(ret));
                    }
                    ConstantSP result = unmarshall->getConstant();
                    return result;
                } else {
                    throw RuntimeException("");
                }
            } catch (RuntimeException &exception) {
                result->setString(dataInit);
                return result;
            } catch (IOException &exception) {
                throw IOException(string("[PLUGIN::KAFKA] ") + exception.what());
            }
            throw IOException(string("[PLUGIN::KAFKA] ") + "Failed to parse marshalled object. Please poll the stream by kafka::consumerPoll.");
        }
    }
    else {
        result->setString("No more message");
    }

    return result;
}

ConstantSP kafkaConsumerPollBatch(Heap *heap, vector<ConstantSP> &args) {
    const auto usage = string(
            "Usage: consumerPollBatch(consumer, batch_size, [timeout]).\n"
            "err: empty if no error else some other error string.\n"
            "msgs: vector [topic partition key value timestamp].\n"
    );

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto consumer = getConnection<Consumer>(args[0]);
    if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
        throw IllegalArgumentException(__FUNCTION__, usage + "batch size need positive integer");
    }
    auto batch_size = args[1]->getInt();
    VectorSP result = Util::createVector(DT_ANY, 0);

    vector<Message> msgs;
    try {
        if(args.size() == 3){
            if (args[2]->getType() < DT_SHORT || args[2]->getType() > DT_LONG || args[2]->getInt() < 0) {
                throw IllegalArgumentException(__FUNCTION__, + "time need positive integer");
            }
            auto time = args[2]->getInt();

            msgs = consumer->poll_batch(batch_size, std::chrono:: milliseconds(time));

            // remove RD_KAFKA_RESP_ERR__PARTITION_EOF case manually
            int length = 0;
            while(length < batch_size) {
                if(msgs.size() == 0) {
                    break;
                }
                for (auto &msg: msgs){
                    if(msg && msg.get_error() && msg.is_eof()) {
                        continue;
                    }
                    result->append(getMsg(msg));
                }
                length = result->size();
                msgs = consumer->poll_batch(batch_size-length, std::chrono:: milliseconds(time));
            }
        }else{
            // remove RD_KAFKA_RESP_ERR__PARTITION_EOF case manually
            msgs = consumer->poll_batch(batch_size);
            int length = 0;
            while(length < batch_size) {
                if(msgs.size() == 0) {
                    break;
                }
                for (auto &msg: msgs){
                    if(msg && msg.get_error() && msg.is_eof()) {
                        continue;
                    }
                    result->append(getMsg(msg));
                }
                length = result->size();
                msgs = consumer->poll_batch(batch_size-length);
            }
        }
    } catch(std::exception &exception) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + exception.what());
    }

    if(result->size() == 0){
        auto err_arg = Util::createConstant(DT_STRING);
        auto msg_arg = Util::createNullConstant(DT_ANY);
        err_arg->setString("No more message");
        auto res = Util::createVector(DT_ANY, 2);
        res->set(0, err_arg);
        res->set(1, msg_arg);
        return res;
    }

    return result;
}

static void kafkaSubConnectionOnClose(Heap *heap, vector<ConstantSP> &args) {
    LockGuard<Mutex> _(&DICT_LATCH);
    long long ptrValue = args[0]->getLong();
    args[0]->setLong(0);
    SubConnection *pObject = (SubConnection*)(ptrValue);
    if(pObject != nullptr) {
        delete pObject;
    }
}


ConstantSP kafkaCreateSubJob(Heap *heap, vector<ConstantSP> args){
    const auto usage = string(
            "Usage: createSubJob(consumer, handler, parser, description, [timeout]).\n"
    );

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto consumer = getConnection<Consumer>(args[0]);
    auto timeout = static_cast<int>(consumer->get_timeout().count());

    if (!(args[1]->isTable() || args[1]->getType()==DT_FUNCTIONDEF || args[1]->isNothing())) {
        throw IllegalArgumentException(__FUNCTION__, usage + "handler should be a table or a function. If the third argument is a coder instance, handler must be null.");
    }
    if(args[1]->getType() == DT_FUNCTIONDEF){
        FunctionDefSP handle = args[1];
        if(handle->getParamCount() != 1){
            throw IllegalArgumentException(__FUNCTION__, usage + "handler function must accept only one param.");
        }
    }

    ConstantSP parser;
    if (args[2]->getType() == DT_FUNCTIONDEF) {
        if (args[1]->isNothing()) {
            throw IllegalArgumentException(__FUNCTION__, usage + "If parser is a function, the second argument must not be empty.");
        }
        parser = args[2];
    } else if(args[2]->getForm() == DF_SYSOBJ && args[2]->getString() == "coder instance") {
        if(!args[1]->isNothing()) {
            throw IllegalArgumentException(__FUNCTION__, usage + "If parser is a decoder, the second argument must be empty.");
        }
        parser = args[2];
    } else {
        throw IllegalArgumentException(__FUNCTION__, usage + "parser must be an function or a decoder.");
    }

    if(parser->getType() == DT_FUNCTIONDEF && (((FunctionDefSP)parser)->getParamCount() < 1 || ((FunctionDefSP)parser)->getParamCount() > 3)){
        throw IllegalArgumentException(__FUNCTION__, usage + "parser function must accept only 1 to 3 param.");
    };

    if (args[3]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "description must be an string.");
    }
    if(args.size() == 5) {
        if (args[4]->getType() < DT_SHORT || args[4]->getType() > DT_LONG || args[4]->getInt() < 0) {
            throw IllegalArgumentException(__FUNCTION__, +"time need positive integer");
        }
        timeout = args[4]->getInt();
    }

    unique_ptr<SubConnection> cup(new SubConnection(heap, args[3]->getString(), args[2], args[1], args[0], timeout));
    FunctionDefSP onClose(Util::createSystemProcedure("kafka sub connection onClose()", kafkaSubConnectionOnClose, 1, 1));
    ConstantSP conn = Util::createResource(
            (long long)(cup.release()),
            "kafka subscribe connection",
            onClose,
            heap->currentSession()
    );
    LockGuard<Mutex> _(&DICT_LATCH);
    STATUS_DICT->set(std::to_string(conn->getLong()),conn);

    return conn;
}

ConstantSP kafkaGetJobStat(Heap *heap, vector<ConstantSP> &args){
    LockGuard<Mutex> _(&DICT_LATCH);
    int size = STATUS_DICT->size();
    ConstantSP connectionIdVec = Util::createVector(DT_STRING, size);
    ConstantSP userVec = Util::createVector(DT_STRING, size);
    ConstantSP desVec = Util::createVector(DT_STRING, size);
    ConstantSP timestampVec = Util::createVector(DT_TIMESTAMP, size);
    VectorSP keys = STATUS_DICT->keys();
    for(int i = 0; i < keys->size();i++){
        string key = keys->getString(i);
        connectionIdVec->setString(i,key);
        ConstantSP conn = STATUS_DICT->getMember(key);
        auto *sc = (SubConnection *)(conn->getLong());
        desVec->setString(i,sc->getDescription());
        timestampVec->setLong(i,sc->getCreateTime());
        userVec->setString(i,sc->getSession()->getUser()->getUserId());
    }

    vector<string> colNames = {"subscriptionId","user","description","createTimestamp"};
    vector<ConstantSP> cols = {connectionIdVec,userVec,desVec,timestampVec};
    return Util::createTable(colNames,cols);
}

void getSubJobConn(ConstantSP handle, ConstantSP &conn, SubConnection *&sc, string &key) {
    switch (handle->getType()){
    case DT_RESOURCE:
        sc = (SubConnection *)(handle->getLong());
        key = std::to_string(handle->getLong());
        conn = STATUS_DICT->getMember(key);
        if(conn->isNothing())
            throw IllegalArgumentException(__FUNCTION__, "Invalid connection resource.");
        break;
    case DT_STRING:
        key = handle->getString();
        conn = STATUS_DICT->getMember(key);
        if(conn->isNothing())
            throw IllegalArgumentException(__FUNCTION__, "Invalid connection string.");
        else
            sc = (SubConnection *)(conn->getLong());
        break;
    case DT_LONG:
    case DT_INT:
        key = std::to_string(handle->getLong());
        conn = STATUS_DICT->getMember(key);
        if(conn->isNothing())
            throw IllegalArgumentException(__FUNCTION__, "Invalid connection long.");
        else
            sc = (SubConnection *)(conn->getLong());
        break;
    default:
        throw IllegalArgumentException(__FUNCTION__, "Invalid connection handle.");
}
}

ConstantSP kafkaCancelSubJob(Heap *heap, vector<ConstantSP> args){
    // parse args first
    std::string usage = "Usage: cancelSubJob(connection or connection ID). ";
    SubConnection *sc = nullptr;
    string key;
    ConstantSP conn = nullptr;
    auto handle = args[0];
    LockGuard<Mutex> _(&DICT_LATCH);

    getSubJobConn(handle, conn, sc, key);

    string ret;
    if (sc != nullptr) {
        sc->cancelThread();
        ret = "subscription: " + to_string(conn->getLong()) + " : " + sc->getDescription() + " is stopped.";
    }
    bool bRemoved=STATUS_DICT->remove(new String(key));
    if(!bRemoved) {
        throw RuntimeException("[PLUGIN::KAFKA] remove subJob status failed.");
    }

    return new String(ret);
}

ConstantSP kafkaPollDict(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: pollDict(consumer, batch_size, [timeout]).\n"
            "err: empty if no error else some other error string.\n"
            "msgs: vector [topic partition key value timestamp].\n"
    );

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto consumer = getConnection<Consumer>(args[0]);
    if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
        throw IllegalArgumentException(__FUNCTION__, usage + "batch size need positive integer");
    }
    auto batch_size = args[1]->getInt();
    auto result = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);


    vector<Message> msgs;
    try {
        if(args.size() == 3){
            if (args[2]->getType() < DT_SHORT || args[2]->getType() > DT_LONG || args[2]->getInt() < 0) {
                throw IllegalArgumentException(__FUNCTION__, + "time need positive integer");
            }
            auto time = args[2]->getInt();
            msgs = consumer->poll_batch(batch_size, std::chrono:: milliseconds(time));

            int length = 0;
            while(length < batch_size)  {
                if(msgs.size() == 0){
                    break;
                }
                for (auto &msg: msgs){
                    if(msg && msg.get_error() && msg.is_eof()) {
                        continue;
                    }
                    auto message = getMsg(msg);
                    if(message->get(1)->get(1)->getType()!=DT_STRING){
                        throw RuntimeException("can only get string as key");
                    }
                    result->set(message->get(1)->get(1), message->get(1)->get(2));
                }
                length = result->size();
                msgs = consumer->poll_batch(batch_size-length, std::chrono:: milliseconds(time));
            }
        }else{
            msgs = consumer->poll_batch(batch_size);
            int length = 0;
            while(length < batch_size)  {
                if(msgs.size() == 0){
                    break;
                }
                for (auto &msg: msgs){
                    if(msg && msg.get_error() && msg.is_eof()) {
                        continue;
                    }
                    auto message = getMsg(msg);
                    if(message->get(1)->get(1)->getType()!=DT_STRING){
                        throw RuntimeException("can only get string as key");
                    }
                    result->set(message->get(1)->get(1), message->get(1)->get(2));
                }
                length = result->size();
                msgs = consumer->poll_batch(batch_size-length);
            }
        }
    } catch(IllegalArgumentException& e) {
        throw e;
    } catch(std::exception &exception) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + exception.what());
    }

    if(result->size() == 0){
        return new Void();
    }

    return result;
}

ConstantSP kafkaCommit(Heap *heap, vector<ConstantSP> &args) {
    const auto usage = string(
            "Usage: commit(consumer).\n"
    );
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto conn = getConnection<Consumer>(args[0]);
    try{
        conn->commit();
    } catch(std::exception &exception) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + exception.what());
    }
    return new Void();
}

ConstantSP kafkaCommitTopic(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: commitTopic(consumer, topic, partition, offset).\n"
    );

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto consumer = getConnection<Consumer>(args[0]);
    Conversion convert(usage,args);
    try{
        consumer->commit(convert.topic_partitions);
    } catch(std::exception &exception) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + exception.what());
    }
    return new Void();
}

ConstantSP kafkaAsyncCommit(Heap *heap, vector<ConstantSP> &args) {
    const auto usage = string(
            "Usage: asyncCommit(consumer).\n"
    );
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto conn = getConnection<Consumer>(args[0]);
    try{
        conn->async_commit();
    } catch(std::exception &exception) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + exception.what());
    }
    return new Void();
}

ConstantSP kafkaAsyncCommitTopic(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: asyncCommitTopic(consumer, topic, partition, offset).\n"
    );

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto consumer = getConnection<Consumer>(args[0]);
    Conversion convert(usage,args);
    try{
        consumer->async_commit(convert.topic_partitions);
    } catch(std::exception &exception) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + exception.what());
    }

    return new Void();
}

ConstantSP kafkaUnsubscribe(Heap *heap, vector<ConstantSP> &args) {
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__,"consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto conn = getConnection<Consumer>(args[0]);
    try{
        conn->unsubscribe();
    } catch(std::exception &exception) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + exception.what());
    }
    return new Void();
}

void kafkaErrorCallback(KafkaHandleBase& handle, int error, const std::string& reason) {
    LOG_ERR("[PLUGIN::KAFKA] error: ", Error((rd_kafka_resp_err_t)error).to_string(), ", reason: ", reason);
}

void kafkaLogCallback(KafkaHandleBase& handle, int level, const std::string& facility, const std::string& message) {
    switch((LogLevel)level) {
        case LogLevel::LogEmerg:
        case LogLevel::LogAlert:
        case LogLevel::LogCrit:
        case LogLevel::LogErr:
            LOG_ERR("[PLUGIN::KAFKA] facility: ", facility, ", message: ", message);
            break;
        case LogLevel::LogWarning:
            LOG_WARN("[PLUGIN::KAFKA] facility: ", facility, ", message: ", message);
            break;
        case LogLevel::LogNotice:
        case LogLevel::LogInfo:
            LOG_INFO("[PLUGIN::KAFKA] facility: ", facility, ", message: ", message);
            break;
        case LogLevel::LogDebug:
        default:
            LOG("[PLUGIN::KAFKA] facility: ", facility, ", message: ", message);
    }
}

Configuration createConf(ConstantSP & dict, bool consumer) {
    Configuration configuration;
    configuration.set_error_callback(kafkaErrorCallback);
    configuration.set_log_callback(kafkaLogCallback);
    bool group_id = false;
    bool server = false;

    auto keys = dict->keys();
    for (auto i = 0; i < keys->size(); i++) {
        auto key = keys->get(i);
        auto value = dict->getMember(key);
        if (value->getType() == DT_STRING) {
            configuration.set(key->getString(), value->getString());
        } else if (value->getType() == DT_BOOL) {
            configuration.set(key->getString(), (bool) value->getBool() ? "true" : "false");
        } else {
            throw IllegalArgumentException(__FUNCTION__, "some configurations are illegal");
        }
        if (key->getString() == "group.id") {
            group_id = true;
        } else if (key->getString() == "metadata.broker.list" || key->getString() == "bootstrap.servers") {
            server = true;
        }
    }
    if(consumer && !group_id) {
        throw IllegalArgumentException(__FUNCTION__, "consumer need setting group.id");
    }
    if(!server) {
        throw IllegalArgumentException(__FUNCTION__, "must setting metadata.broker.list or bootstrap.servers");
    }

    return configuration;
}

ConstantSP kafkaSetConsumerTimeout(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: setConsumerTime(consumer, timeout).\n"
    );
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto consumer = getConnection<Consumer>(args[0]);
    if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
        throw IllegalArgumentException(__FUNCTION__, usage + "time need positive integer");
    }
    auto time = args[1]->getInt();
    consumer->set_timeout(std::chrono::milliseconds(time));

    return new Void();
}

ConstantSP kafkaSetProducerTimeout(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: setProducerTime(producer, timeout).\n"
    );
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != PRODUCER_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "producer should be a producer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Producer>(args[0], lockGuard);
    auto producer = getConnection<Producer>(args[0]);
    if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
        throw IllegalArgumentException(__FUNCTION__, usage + "time need positive integer");
    }
    auto time = args[1]->getInt();
    producer->set_timeout(std::chrono:: milliseconds(time));

    return new Void();
}

ConstantSP kafkaGetConsumerTimeout(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: getConsumerTime(consumer).\n"
    );
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto consumer = getConnection<Consumer>(args[0]);
    auto res = Util::createConstant(DT_INT);
    res->setInt(static_cast<int>(consumer->get_timeout().count()));
    return res;
}

ConstantSP kafkaGetProducerTimeout(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: getProducerTime(producer).\n"
    );
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != PRODUCER_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "producer should be a producer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Producer>(args[0], lockGuard);
    auto producer = getConnection<Producer>(args[0]);
    auto res = Util::createConstant(DT_INT);
    res->setInt(static_cast<int>(producer->get_timeout().count()));
    return res;
}

ConstantSP kafkaConsumerAssign(Heap *heap, vector<ConstantSP> &args){
    LockGuard<Mutex> _(&ASSIGN_LATCH);
    const auto usage = string(
            "Usage: assign(consumer, topic, partition, offset).\n"
    );

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto consumer = getConnection<Consumer>(args[0]);
    Conversion convert(usage,args);
    try {
        consumer->assign(convert.topic_partitions);
    } catch(std::exception& e) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
    }
    ASSIGN_TIMEOUT.store(Util::getEpochTime());
    return new Void();
}

ConstantSP kafkaConsumerUnassign(Heap *heap, vector<ConstantSP> &args){
    LockGuard<Mutex> _(&ASSIGN_LATCH);
    atomic<int64_t> current;
    current.store(Util::getEpochTime());
    if(current - ASSIGN_TIMEOUT < 2) {
        Util::sleep(1);
    }
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto consumer = getConnection<Consumer>(args[0]);
    try {
        consumer->unassign();
    } catch(std::exception& e) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
    }
    return new Void();
}

ConstantSP kafkaConsumerGetAssignment(Heap *heap, vector<ConstantSP> &args){
    LockGuard<Mutex> _(&ASSIGN_LATCH);
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto consumer = getConnection<Consumer>(args[0]);
    auto result = consumer->get_assignment();

    int size = result.size();
    ConstantSP topicVec = Util::createVector(DT_STRING, size);
    ConstantSP partitionVec = Util::createVector(DT_INT, size);
    ConstantSP offsetVec = Util::createVector(DT_LONG, size);
    for(int i = 0; i < size;i++){
        topicVec->setString(i,result[i].get_topic());
        partitionVec->setInt(i,result[i].get_partition());
        offsetVec->setLong(i,result[i].get_offset());
    }
    vector<string> colNames = {"topic","partition","offset"};
    vector<ConstantSP> cols = {topicVec,partitionVec,offsetVec};

    return Util::createTable(colNames,cols);
}

ConstantSP kafkaConsumerPause(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto consumer = getConnection<Consumer>(args[0]);
    try {
        consumer->pause();
    } catch(std::exception& e) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
    }
    return new Void();
}

ConstantSP kafkaConsumerResume(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto consumer = getConnection<Consumer>(args[0]);
    try {
        consumer->resume();
    } catch(std::exception& e) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
    }
    return new Void();
}

ConstantSP kafkaGetOffset(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: getOffset(consumer, topic, partition).\n"
    );
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto consumer = getConnection<Consumer>(args[0]);
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "Topic must be string scalar.");
    }
    auto topic = args[1]->getString();
    if (args[2]->getType() != DT_INT || args[2]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "partition must be a integer scalar");
    }
    if(args[2]->getInt() < 0) {
        throw IllegalArgumentException(__FUNCTION__, usage + "partition must be a positive integer scalar");
    }
    auto partition = args[2]->getInt();
    auto back = consumer->get_offsets(TopicPartition(topic, partition));
    auto result = Util::createVector(DT_ANY, 2);
    auto low = Util::createConstant(DT_INT);
    auto high = Util::createConstant(DT_INT);
    low->setInt(get<0>(back));
    high->setInt(get<1>(back));
    result->set(0, low);
    result->set(1, high);

    return result;
}

ConstantSP kafkaGetOffsetsCommitted(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: getOffsetCommitted(consumer, topic, partition, offset, [timeout]).\n"
    );

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto consumer = getConnection<Consumer>(args[0]);
    Conversion convert(usage,args);

    vector<TopicPartition> result;
    try {
        if(args.size() == 5){
            if (args[4]->getType() < DT_SHORT || args[4]->getType() > DT_LONG || args[4]->getInt() < 0) {
                throw IllegalArgumentException(__FUNCTION__, + "time need positive integer");
            }
            auto time = args[4]->getInt();
            result = consumer->get_offsets_committed(convert.topic_partitions, std::chrono::milliseconds(time));
        }else{
            result = consumer->get_offsets_committed(convert.topic_partitions);
        }
    } catch(std::exception& e) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
    }

    vector<string> colNames{"topic", "partition", "offset"};
    vector<ConstantSP> cols;
    vector<string> topics;
    vector<int> partitions;
    vector<int> offsets;

    for(auto & r : result) {
        topics.emplace_back(r.get_topic());
        partitions.emplace_back(r.get_partition());
        offsets.emplace_back(r.get_offset());
    }
    VectorSP topic = Util::createVector(DT_STRING, 0, topics.size());
    VectorSP partition = Util::createVector(DT_INT, 0, partitions.size());
    VectorSP offset = Util::createVector(DT_INT, 0, offsets.size());

    topic->appendString(topics.data(), topics.size());
    partition->appendInt(partitions.data(), partitions.size());
    offset->appendInt(offsets.data(), offsets.size());
    cols.push_back(topic);
    cols.push_back(partition);
    cols.push_back(offset);

    ConstantSP ret = Util::createTable(colNames, cols);

    return ret;
}

ConstantSP kafkaGetOffsetPosition(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: getOffsetPosition(consumer, topic, partition).\n"
    );
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto consumer = getConnection<Consumer>(args[0]);
    Conversion convert(usage,args);

    vector<TopicPartition> result;
    try {
        result = consumer->get_offsets_position(convert.topic_partitions);
    } catch(std::exception& e) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
    }
    vector<string> colNames{"topic", "partition", "offset"};
    vector<ConstantSP> cols;
    vector<string> topics;
    vector<int> partitions;
    vector<int> offsets;

    for(auto & r : result) {
        topics.emplace_back(r.get_topic());
        partitions.emplace_back(r.get_partition());
        offsets.emplace_back(r.get_offset());
    }
    VectorSP topic = Util::createVector(DT_STRING, 0, topics.size());
    VectorSP partition = Util::createVector(DT_INT, 0, partitions.size());
    VectorSP offset = Util::createVector(DT_INT, 0, offsets.size());

    topic->appendString(topics.data(), topics.size());
    partition->appendInt(partitions.data(), partitions.size());
    offset->appendInt(offsets.data(), offsets.size());
    cols.push_back(topic);
    cols.push_back(partition);
    cols.push_back(offset);

    ConstantSP ret = Util::createTable(colNames, cols);

    return ret;
}

#if (RD_KAFKA_VERSION >= RD_KAFKA_STORE_OFFSETS_SUPPORT_VERSION)
ConstantSP kafkaStoreConsumedOffsets(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto consumer = getConnection<Consumer>(args[0]);
    try {
        consumer->store_consumed_offsets();
    } catch(std::exception& e) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
    }
    return new Void();
}

ConstantSP kafkaStoreOffsets(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: storeOffset(consumer, topic, partition, offset).\n"
    );
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto consumer = getConnection<Consumer>(args[0]);
    Conversion convert(usage,args);
    try {
        consumer->store_offsets(convert.topic_partitions);
    } catch(std::exception& e) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
    }

    return new Void();
}
#endif

ConstantSP kafkaGetMemberId(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto consumer = getConnection<Consumer>(args[0]);
    auto result = Util::createConstant(DT_STRING);
    try {
        auto str = consumer->get_member_id();
        result->setString(0,str);
        return result;
    } catch(std::exception& e) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
    }
}

ConstantSP kafkaQueueLength(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != QUEUE_DESC)
        throw IllegalArgumentException(__FUNCTION__, "queue should be a queue handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Queue>(args[0], lockGuard);
    auto queue = getConnection<Queue>(args[0]);
    auto result = Util::createConstant(DT_INT);
    try {
        result->setInt(queue->get_length());
        return result;
    } catch(std::exception& e) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
    }
}

ConstantSP  kafkaForToQueue(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != QUEUE_DESC)
        throw IllegalArgumentException(__FUNCTION__, "the first queue should be a queue handle.");
    if(args[1]->getType()!=DT_RESOURCE || args[1]->getString() != QUEUE_DESC)
        throw IllegalArgumentException(__FUNCTION__, "the second queue should be a queue handle.");
    if(args[0]->getLong() == args[1]->getLong()) {
        throw RuntimeException("[PLUGIN::KAFKA] Messages cannot be forwarded to the same queue.");
    }
    RWLockGuard<RWLock> lockGuard0(NULL, false);
    getWrapperLockGuard<Queue>(args[0], lockGuard0);
    auto queue = getConnection<Queue>(args[0]);
    RWLockGuard<RWLock> lockGuard1(NULL, false);
    getWrapperLockGuard<Queue>(args[1], lockGuard1);
    auto forward_queue = getConnection<Queue>(args[1]);
    try {
        queue->forward_to_queue(*forward_queue);
    } catch(std::exception& e) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
    }
    return new Void();
}

ConstantSP kafkaDisForToQueue(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != QUEUE_DESC)
        throw IllegalArgumentException(__FUNCTION__, "queue should be a queue handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Queue>(args[0], lockGuard);
    auto queue = getConnection<Queue>(args[0]);
    try {
        queue->disable_queue_forwarding();
    } catch(std::exception& e) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
    }
    return new Void();
}

ConstantSP kafkaSetQueueTime(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != QUEUE_DESC)
        throw IllegalArgumentException(__FUNCTION__, "queue should be a queue handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Queue>(args[0], lockGuard);
    auto queue = getConnection<Queue>(args[0]);
    if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
        throw IllegalArgumentException(__FUNCTION__, + "time need positive integer");
    }
    auto time = args[1]->getInt();
    queue->set_timeout(std::chrono:: milliseconds(time));
    return new Void();
}

ConstantSP kafkaGetQueueTime(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != QUEUE_DESC)
        throw IllegalArgumentException(__FUNCTION__, "queue should be a queue handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Queue>(args[0], lockGuard);
    auto queue = getConnection<Queue>(args[0]);
    auto result = Util::createConstant(DT_INT);
    result->setInt(static_cast<int>(queue->get_timeout().count()));
    return result;
}

ConstantSP kafkaQueueConsume(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: queuePoll(queue, [timeout]).\n"
            "return: [err, msg]\n"
            "err: empty if no error else error info string.\n"
            "msg: [topic, partition, key, value, timestamp].\n"
    );

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != QUEUE_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "queue should be a queue handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Queue>(args[0], lockGuard);
    auto queue = getConnection<Queue>(args[0]);

    try {
        if(args.size() == 2){
            if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
                throw IllegalArgumentException(__FUNCTION__, + "time need positive integer");
            }
            auto time = args[1]->getInt();
            auto msg = queue->consume(std::chrono:: milliseconds(time));
            if (msg && msg.get_error() && msg.is_eof()) {
                msg = queue->consume(std::chrono:: milliseconds(time));
            }
            return getMsg(msg);
        }else{
            auto msg = queue->consume(std::chrono:: milliseconds(1000));
            if (msg && msg.get_error() && msg.is_eof()) {
                msg = queue->consume(std::chrono:: milliseconds(1000));
            }
            return getMsg(msg);
        }
    } catch(std::exception& e) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
    }
}

ConstantSP kafkaQueueConsumeBatch(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: queuePollBatch(queue, batch_size, [timeout]).\n"
            "err: empty if no error else some other error string.\n"
            "msgs: vector [topic partition key value timestamp].\n"
    );

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != QUEUE_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "queue should be a queue handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Queue>(args[0], lockGuard);
    auto queue = getConnection<Queue>(args[0]);
    if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
        throw IllegalArgumentException(__FUNCTION__, usage + "batch size need positive integer");
    }
    auto batch_size = args[1]->getInt();
    auto result = Util::createVector(DT_ANY, 0);

    vector<Message> msgs;
    try {
        if(args.size() == 3){
            if (args[2]->getType() < DT_SHORT || args[2]->getType() > DT_LONG || args[2]->getInt() < 0) {
                throw IllegalArgumentException(__FUNCTION__, + "time need positive integer");
            }
            auto time = args[2]->getInt();
            msgs = queue->consume_batch(batch_size, std::chrono:: milliseconds(time));

            int length = 0;
            while(length < batch_size) {
                if(msgs.size() == 0) {
                    break;
                }
                for (auto &msg: msgs){
                    if(msg && msg.get_error() && msg.is_eof()) {
                        continue;
                    }
                    result->append(getMsg(msg));
                }
                length = result->size();
                msgs = queue->consume_batch(batch_size-length, std::chrono:: milliseconds(time));
            }
        }else{
            msgs = queue->consume_batch(batch_size, std::chrono:: milliseconds(1000));
            int length = 0;
            while(length < batch_size) {
                if(msgs.size() == 0) {
                    break;
                }
                for (auto &msg: msgs){
                    if(msg && msg.get_error() && msg.is_eof()) {
                        continue;
                    }
                    result->append(getMsg(msg));
                }
                length = result->size();
                msgs = queue->consume_batch(batch_size-length, std::chrono:: milliseconds(1000));
            }
        }
    } catch(std::exception& e) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
    }

    if(result->size() == 0){
        auto err_arg = Util::createConstant(DT_STRING);
        auto msg_arg = Util::createNullConstant(DT_ANY);
        err_arg->setString("No more message");
        auto res = Util::createVector(DT_ANY, 2);
        res->set(0, err_arg);
        res->set(1, msg_arg);
        free(result);
        return res;
    }

    return result;
}

ConstantSP kafkaGetMainQueue(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto consumer = getConnection<Consumer>(args[0]);
    try {
        auto queue = new KafkaWrapper<Queue>(new Queue(consumer->get_main_queue()));
        FunctionDefSP onClose(Util::createSystemProcedure("kafka queue onClose()", kafkaOnClose<Queue>, 1, 1));
        LockGuard<Mutex> _(&HANDLE_MUTEX);
        HANDLE_SET.insert((long long) queue);
        return Util::createResource(
                (long long) queue,
                QUEUE_DESC,
                onClose,
                heap->currentSession()
        );
    } catch(std::exception& e) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
    }

}

ConstantSP kafkaGetConsumerQueue(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto consumer = getConnection<Consumer>(args[0]);

    try {
        auto queue = new KafkaWrapper<Queue>(new Queue(consumer->get_consumer_queue()));

        FunctionDefSP onClose(Util::createSystemProcedure("kafka queue onClose()", kafkaOnClose<Queue>, 1, 1));
        LockGuard<Mutex> _(&HANDLE_MUTEX);
        HANDLE_SET.insert((long long) queue);
        return Util::createResource(
                (long long) queue,
                QUEUE_DESC,
                onClose,
                heap->currentSession()
        );
    } catch(std::exception& e) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
    }
}

ConstantSP kafkaGetPartitionQueue(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(__FUNCTION__, "consumer should be a consumer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Consumer>(args[0], lockGuard);
    auto consumer = getConnection<Consumer>(args[0]);
    Conversion convert("",args);
    try {
        auto queue = new KafkaWrapper<Queue>(new Queue(consumer->get_partition_queue(convert.topic_partitions[0])));

        FunctionDefSP onClose(Util::createSystemProcedure("kafka queue onClose()", kafkaOnClose<Queue>, 1, 1));
        LockGuard<Mutex> _(&HANDLE_MUTEX);
        HANDLE_SET.insert((long long) queue);
        return Util::createResource(
                (long long) queue,
                QUEUE_DESC,
                onClose,
                heap->currentSession()
        );
    } catch(std::exception& e) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
    }
}

ConstantSP kafkaQueueEvent(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != QUEUE_DESC)
        throw IllegalArgumentException(__FUNCTION__, "queue should be a queue handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Queue>(args[0], lockGuard);
    auto queue = getConnection<Queue>(args[0]);
    try {
        auto event = new KafkaWrapper<Event>(new Event(queue->next_event()));
        FunctionDefSP onClose(Util::createSystemProcedure("kafka event onClose()", kafkaOnClose<Event>, 1, 1));
        LockGuard<Mutex> _(&HANDLE_MUTEX);
        HANDLE_SET.insert((long long) event);
        return Util::createResource(
                (long long) event,
                EVENT_DESC,
                onClose,
                heap->currentSession()
        );
    } catch(std::exception& e) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
    }
}

ConstantSP kafkaGetEventName(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != EVENT_DESC)
        throw IllegalArgumentException(__FUNCTION__, "event should be a event handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Event>(args[0], lockGuard);
    auto event = getConnection<Event>(args[0]);
    auto name = Util::createConstant(DT_STRING);
    name->setString(event->get_name());
    return name;
}

ConstantSP kafkaEventGetMessages(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: eventGetMessage(event).\n"
            "err: empty if no error else some other error string.\n"
            "msgs: vector [topic partition key value timestamp].\n"
    );

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != EVENT_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "event should be a event handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Event>(args[0], lockGuard);
    auto event = getConnection<Event>(args[0]);
    if(!event->operator bool()){
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + "The event is empty!");
    }
    auto result = Util::createVector(DT_ANY, 0);
    vector<Message> msgs;
    try {
        msgs = event->get_messages();
    } catch(std::exception& e) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
    }
    for (auto &msg: msgs){
        result->append(getMsg(msg));
    }
    if(msgs.size() == 0){
        auto err_arg = Util::createConstant(DT_STRING);
        auto msg_arg = Util::createNullConstant(DT_ANY);
        err_arg->setString("No more message");
        auto res = Util::createVector(DT_ANY, 2);
        res->set(0, err_arg);
        res->set(1, msg_arg);
        free(result);
        return res;
    }
    return result;
}

ConstantSP kafkaGetEventMessageCount(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != EVENT_DESC)
        throw IllegalArgumentException(__FUNCTION__, "event should be a event handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Event>(args[0], lockGuard);
    auto event = getConnection<Event>(args[0]);
    if(!event->operator bool()){
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + "The event is empty!");
    }
    auto count = Util::createConstant(DT_INT);
    count->setInt(event->get_message_count());
    return count;
}

ConstantSP kafkaEventGetError(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != EVENT_DESC)
        throw IllegalArgumentException(__FUNCTION__, "event should be a event handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Event>(args[0], lockGuard);
    auto event = getConnection<Event>(args[0]);
    if(!event->operator bool()){
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + "The event is empty!");
    }
    auto error = event->get_error();
    auto string = Util::createConstant(DT_STRING);
    string->setString(error.to_string());
    return string;
}

ConstantSP kafkaEventGetPartition(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != EVENT_DESC)
        throw IllegalArgumentException(__FUNCTION__, "event should be a event handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Event>(args[0], lockGuard);
    auto event = getConnection<Event>(args[0]);
    if(!event->operator bool()){
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + "The event is empty!");
    }

    stringstream ss;
    try {
        ss << event->get_topic_partition() << endl;
    } catch(std::exception& e) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
    }
    auto result = Util::createConstant(DT_STRING);
    result->setString(ss.str());

    return result;
}

ConstantSP kafkaEventGetPartitionList(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != EVENT_DESC)
        throw IllegalArgumentException(__FUNCTION__, "event should be a event handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Event>(args[0], lockGuard);
    auto event = getConnection<Event>(args[0]);
    if(!event->operator bool()){
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + "The event is empty!");
    }

    if(event->get_type() != RD_KAFKA_EVENT_OFFSET_COMMIT && event->get_type() != RD_KAFKA_EVENT_REBALANCE ) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + "wrong event type.");
    }
    vector<TopicPartition> result;
    try {
        result = event->get_topic_partition_list();
    } catch(std::exception& e) {
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
    }

    vector<string> colNames{"topic", "partition", "offset"};
    vector<ConstantSP> cols;
    vector<string> topics;
    vector<int> partitions;
    vector<int> offsets;

    for(auto & r : result) {
        topics.emplace_back(r.get_topic());
        partitions.emplace_back(r.get_partition());
        offsets.emplace_back(r.get_offset());
    }
    VectorSP topic = Util::createVector(DT_STRING, 0, topics.size());
    VectorSP partition = Util::createVector(DT_INT, 0, partitions.size());
    VectorSP offset = Util::createVector(DT_INT, 0, offsets.size());

    topic->appendString(topics.data(), topics.size());
    partition->appendInt(partitions.data(), partitions.size());
    offset->appendInt(offsets.data(), offsets.size());
    cols.push_back(topic);
    cols.push_back(partition);
    cols.push_back(offset);

    ConstantSP ret = Util::createTable(colNames, cols);

    return ret;
}

ConstantSP kafkaEventBool(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != EVENT_DESC)
        throw IllegalArgumentException(__FUNCTION__, "event should be a event handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Event>(args[0], lockGuard);
    auto event = getConnection<Event>(args[0]);
    auto result = Util::createConstant(DT_BOOL);
    result->setBool(1,event->operator bool());
    return result;
}

ConstantSP kafkaGetBufferSize(Heap *heap, vector<ConstantSP> &args){
    auto result = Util::createConstant(DT_LONG);
    result->setLong(BUFFER_SIZE);
    return result;
}

ConstantSP kafkaSetBufferSize(Heap *heap, vector<ConstantSP> &args){
    long long size = args[0]->getLong();
    if(size>=MESSAGE_SIZE)
        BUFFER_SIZE = size;
    else{
        BUFFER_SIZE = size;
        MESSAGE_SIZE = size;
		throw RuntimeException(string("[PLUGIN::KAFKA] ") + "The buffer_size is smaller than message_size. The message_size is set the same as buffer_size.");
    }
    return new String("The buffer size has been successfully set, please make sure the buffer size is no larger than broker size.");
}

ConstantSP kafkaGetMessageSize(Heap *heap, vector<ConstantSP> &args){
    auto result = Util::createConstant(DT_LONG);
    result->setLong(MESSAGE_SIZE);
    return result;
}

ConstantSP kafkaSetMessageSize(Heap *heap, vector<ConstantSP> &args){
    long long size = args[0]->getLong();
    if(size<=BUFFER_SIZE)
        MESSAGE_SIZE = size;
    else{
        throw IllegalArgumentException(__FUNCTION__, + "message_size should not be larger than buffer_size");
    }

    return new Void();
}

inline static int getLength(ConstantSP &data){
	DATA_TYPE type = data->getType();
    if(type<=DT_DOUBLE)
        return TYPE_SIZE[type];
    else if(type == DT_SYMBOL || type == DT_STRING || type == DT_BLOB)
        return data->getString().length();
    else if(type == DT_IP || type == DT_INT128)
        return 16;
    else if(type == DT_FUNCTIONDEF)
        return 128;
    else
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + "Unsolved type.");
}

void produceMessage(ConstantSP &produce, ConstantSP &pTopic, ConstantSP &key, ConstantSP &value, ConstantSP &json, ConstantSP &pPartition){
    const auto usage = string("Usage: produce(producer, topic: string, key, value, json, [partition]).\n");
    if(produce->getType()!=DT_RESOURCE || produce->getString() != PRODUCER_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "producer should be a producer handle.");
    RWLockGuard<RWLock> lockGuard(NULL, false);
    getWrapperLockGuard<Producer>(produce, lockGuard);
    auto producer = getConnection<Producer>(produce);
    if (pTopic.isNull() || pTopic->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "Topic must be string.");
    }
    auto topic = pTopic->getString();
    // ConstantSP judge = Util::createConstant(DT_BOOL);
    // judge->setBool(1, false);
    // auto keyStr = kafkaSerialize(key, judge);
    string keyStr = key->getString();
    auto valueStr = kafkaSerialize(value, json);

    if(keyStr.length() + valueStr.length() <= BUFFER_SIZE*(2-FACTOR)){
        try {
            auto &&msg = MessageBuilder(topic).key(keyStr).payload(valueStr);
            if (!pPartition->isNull()) {
                if (pPartition->getType() != DT_INT || pPartition->getForm() != DF_SCALAR) {
                    throw IllegalArgumentException(__FUNCTION__, usage + "partition must be a integer");
                }
                auto partition = pPartition->getInt();
                msg.partition(partition);
            }
            producer->produce(msg);
        } catch(IllegalArgumentException& e){
            throw e;
        } catch(std::exception& e) {
            throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
        }
    }
    else{
        long long start = 0;
        long long size = 0;
        long long len;
        long long length;
        vector<int> strVec;
        int headLen = 0;
        int rowLen = 0;

        if(value->getForm() == DF_TABLE){
            length = value->columns();
            auto keys = value->keys();
            auto values = value->values();
            auto valueSize = value->size();
            if(valueSize == 1) {
                throw IllegalArgumentException(__FUNCTION__, usage + "size of data is too big");
            }
            for(int i = 0;i<value->columns();i++){
                headLen += keys->get(i)->getString().length();
                if(values->get(i)->getType() == DT_STRING || values->get(i)->getType() == DT_SYMBOL)
                    strVec.push_back(i);
                else{
                    ConstantSP temp = value->values()->get(i);
                    rowLen+=getLength(temp);
                    rowLen += 2;    // simulate ", "
                }
            }

            if(strVec.empty()){
                long long step = (MESSAGE_SIZE-headLen)/rowLen * 2 / 3; // To prevent corner cases where MESSAGE_SIZE and BUFFER_SIZE are equal
                if(step < 1){
                    if(rowLen+headLen>BUFFER_SIZE*FACTOR)
                        throw RuntimeException("[PLUGIN::KAFKA] The data is too large.");
                    else
                        step = 1;
                }
                for(long long i = 0;i<valueSize;i+=step){
                    auto pass = value->getWindow(0,length,i,(i+step>=valueSize)?(valueSize-i):step);
                    produceMessage(produce,pTopic,key,pass,json,pPartition);
                }
            }
            else {
                size = headLen;
                vector<ConstantSP> strCol;
                for(int j : strVec) {
                    strCol.push_back(values->get(j));
                }
                for (long long i = 0; i < valueSize; i++) {
                    len = rowLen;
                    for(ConstantSP col: strCol)
                        len += col->get(i)->getString().length();
                    size+=len;
                    if (size > BUFFER_SIZE*FACTOR){
                        if(size - len <= BUFFER_SIZE*FACTOR && size - len > headLen){
                            auto pass = value->getWindow(0,length,start,i-start);
                            produceMessage(produce, pTopic, key, pass, json, pPartition);
                            start = i;
                            size = headLen+len;
                        }
                        else {
                            throw IllegalArgumentException(__FUNCTION__, usage + "size of data is too big");
                        }
                    }
                    else if (size > MESSAGE_SIZE*FACTOR) {
                        auto pass = value->getWindow(0,length,start,i-start+1);
                        produceMessage(produce, pTopic, key, pass, json, pPartition);
                        start = i + 1;
                        size = headLen;
                    }
                }
                if(size>headLen){
                    auto pass = value->getWindow(0,length,start,valueSize-start);
                    produceMessage(produce,pTopic,key,pass,json,pPartition);
                }
            }
        }

        else if(value->getForm() == DF_VECTOR){
            VectorSP vector = value;
            length = vector->size();
            for(long long i = 0;i<length;i++){
                ConstantSP temp = vector->get(i);
                len = getLength(temp);
                if(len>BUFFER_SIZE*FACTOR)
                    throw IllegalArgumentException(__FUNCTION__, usage + "size of data is too big");
                else if(len+size>=BUFFER_SIZE*FACTOR){
                    auto pass = vector->getSubVector(start,i-start);
                    produceMessage(produce,pTopic,key,pass,json,pPartition);
                    size = len;
                    start = i;
                }
                else if(len>MESSAGE_SIZE*FACTOR){
                    if(size!=0){
                        auto pass = vector->getSubVector(start, i-start+1);
                        produceMessage(produce,pTopic,key,pass,json,pPartition);
                        size = 0;
                    }
                    auto pass_temp = vector->getSubVector(i, 1);
                    produceMessage(produce,pTopic,key,pass_temp,json,pPartition);
                    start=i+1;
                }
                else if(len+size>=MESSAGE_SIZE*FACTOR){
                    auto pass = vector->getSubVector(start,i-start+1);
                    produceMessage(produce,pTopic,key,pass,json,pPartition);
                    size = 0;
                    start = i+1;
                }
                else if(len+size<MESSAGE_SIZE*FACTOR){
                    size+=len;
                }
            }
            if(size!=0){
                auto pass = vector->getSubVector(start,length-start);
                produceMessage(produce,pTopic,key,pass,json,pPartition);
            }
        }

        else
            throw IllegalArgumentException(__FUNCTION__, usage + "size of data is too big");
    }
}

Vector* getMsg(Message &msg){
    auto err_arg = Util::createConstant(DT_STRING);

    if (msg) {
        ConstantSP msg_arg;
        try {
            if (msg.get_error()) {
                if (msg.is_eof())
                    err_arg->setString("Broker: No more messages");
                else
                    err_arg->setString(msg.get_error().to_string());
                msg_arg = Util::createConstant(DT_VOID);
            } else {
                err_arg->setString("");
                msg_arg = extractMessage(msg);
            }
        } catch(std::exception& ex) {
            msg_arg = Util::createConstant(DT_VOID);
            err_arg->setString("Parse message failed, " + string(ex.what()));
        } catch(...) {
            msg_arg = Util::createConstant(DT_VOID);
            err_arg->setString("Parse message failed.");
        }

        auto res = Util::createVector(DT_ANY, 2);
        res->set(0, err_arg);
        res->set(1, msg_arg);
        return res;
    } else {
        err_arg->setString("No more message");
        auto msg_arg = Util::createNullConstant(DT_ANY);
        auto res = Util::createVector(DT_ANY, 2);
        res->set(0, err_arg);
        res->set(1, msg_arg);
        return res;
    }
}

string kafkaGetString(const ConstantSP &data, bool key){
    if(data->getForm() == DF_SCALAR) {
        if(key && data->getType()!=DT_STRING && data->getType()!=DT_CHAR && data->getType() != DT_BOOL){
            return "\"" + data->getString() + "\"";
        }
        else if (data->getType() == DT_BOOL)
            if ((int) data->getBool() == 1)
                return string("true");
            else
                return string("false");
        else if (data->isNull())
            return string("null");
        else if (data->getType() == DT_STRING || data->getType() == DT_CHAR)
            return "\"" + data->getString() + "\"";
        else if(data->getType() == DT_INT || data->getType() == DT_DOUBLE || data->getType() == DT_FLOAT || data->getType() == DT_SHORT || data->getType() == DT_LONG)
            return data->getString();
        else{
            return "\"" + data->getString() + "\"";
        }
    }
    else
        return kafkaJsonSerialize(data);
}

string kafkaJsonSerialize(const ConstantSP &data){
	string result;
    if(data->getForm() == DF_VECTOR){
        if(data->size() == 0){
            return string("[]");
        }
        result+="[";
        result+=kafkaGetString(data->get(0));
        for(int i = 1;i<data->size();i++){
            result+=",";
            result+=kafkaGetString(data->get(i));
        }
        result+="]";
    }
    else if(data->getForm() == DF_DICTIONARY){
        if(data->size() == 0)
            return string("{}");
        int length = data->size();

        ConstantSP dictKeys = data->keys();
        ConstantSP dictValues = data->values();

        result+="{";
        result+=kafkaGetString(dictKeys->get(length-1), true);
        result+=":";
        result+=kafkaGetString(dictValues->get(length-1));
        for(int i = length-2;i>=0;i--){
            result+=",";
            result+=kafkaGetString(dictKeys->get(i), true);
            result+=":";
            result+=kafkaGetString(dictValues->get(i));
        }
        result+="}";
    }
    else if(data->getForm() == DF_TABLE){
        if(data->size() == 0 || data->columns() == 0)
            return string("{}");
        ConstantSP dictKeys = data->keys();
        ConstantSP dictValues = data->values();
        result+="{";
        result+=kafkaGetString(dictKeys->get(0), true);
        result+=":";
        result+=kafkaJsonSerialize(dictValues->get(0));
        for(int i = 1;i<data->columns();i++){
            result+=",";
            result+=kafkaGetString(dictKeys->get(i), true);
            result+=":";
            result+=kafkaJsonSerialize(dictValues->get(i));
        }
        result+="}";
    }
    else if(data->getForm() == DF_SCALAR){
        result+="[";
        result+=kafkaGetString(data);
        result+="]";
    }
    else
        throw IllegalArgumentException(__FUNCTION__, "Only scalar, vector, dictionary and table can be passed as json.");

    return result;
}

string kafkaSerialize(ConstantSP &data, ConstantSP &json){
    if(data->getType() == DT_FUNCTIONDEF){
        throw IllegalArgumentException(__FUNCTION__, "Can't pass function type.");
    }
    if((int)json->getBool() == 1)
        return kafkaJsonSerialize(data);
    auto result = string("");
    IO_ERR ret;

    DataOutputStreamSP outStream = new DataOutputStream();
    ConstantMarshalFactory marshallFactory(outStream);
    ConstantMarshal* marshall = marshallFactory.getConstantMarshal(data->getForm());
    marshall->start(data, true, ret);
    result+=string(outStream->getBuffer(), outStream->size());

    ret = outStream->flush();
    if(ret != IO_ERR::OK) {
        throw IOException(ret);
    }
    return result;
}

ConstantSP kafkaDeserialize(const string &dataInit){
    if(dataInit == ""){
        auto empty = Util::createNullConstant(DT_ANY);
        return empty;
    }
    short flag;
    IO_ERR ret;
    auto str = dataInit.c_str();
    try {
    if(str[0] == '{' || str[0] == '['){
        json data = json::parse(str);
        if(data.is_object()){
            auto result = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);
            for (json::iterator it = data.begin(); it != data.end(); ++it) {
                ConstantSP value;
                if(it.value().is_array() || it.value().is_object()) {
                    value = kafkaDeserialize(it.value().dump());
                }
                else if(it.value().is_boolean()) {
                    value = Util::createConstant(DT_BOOL);
                    value->setBool(0, (bool)*it);
                }
                else if(it.value().is_null()) {
                    value = Util::createNullConstant(DT_ANY);
                }
                else if(it.value().is_number_integer() || it.value().is_number_unsigned()) {
                    if(*it>0x7fffffffffffffffL){
                        LOG_INFO(string("[PLUGIN::KAFKA] ") + "The integer is too large and it will be cast to string.");
                        value = Util::createConstant(DT_STRING);
                        string temp = it.value().dump();
                        value->setString(temp.substr(0,temp.length()));
                    }
                    else if(*it>0x7fffffff){
                        value = Util::createConstant(DT_LONG);
                        value->setLong(*it);
                    }
                    else {
                        value = Util::createConstant(DT_INT);
                        value->setInt(*it);
                    }
                }
                else if(it.value().is_number_float()) {
                    value = Util::createConstant(DT_DOUBLE);
                    value->setDouble(*it);
                }
                else if(it.value().is_string()){
                    value = Util::createConstant(DT_STRING);
                    string temp = it.value().dump();
                    value->setString(temp.substr(1,temp.length()-2));
                }
                else{
                    LOG_INFO(string("[PLUGIN::KAFKA] ") + string(*it) + ":un defined data type.");
                    value = Util::createNullConstant(DT_ANY);
                }
                auto key = Util::createConstant(DT_STRING);
                key->setString(it.key());
                result->set(key,value);
            }
            return result;
        }
        else if(data.is_array()){
            auto result = Util::createVector(DT_ANY,0);
            for (json::iterator it = data.begin(); it != data.end(); ++it) {
                ConstantSP value;
                if(it->is_number_integer() || it->is_number_unsigned()) {
                    if(*it>0x7fffffffffffffffL){
                        LOG_INFO(string("[PLUGIN::KAFKA] ") + "The integer is too large and it will be cast to string.");
                        value = Util::createConstant(DT_STRING);
                        string temp = it.value().dump();
                        value->setString(temp.substr(0,temp.length()));
                    }
                    else if(*it>0x7fffffff){
                        value = Util::createConstant(DT_LONG);
                        value->setLong(*it);
                    }
                    else {
                        value = Util::createConstant(DT_INT);
                        value->setInt(*it);
                    }
                }
                else if(it->is_number_float()) {
                    value = Util::createConstant(DT_DOUBLE);
                    value->setDouble(*it);
                }
                else if(it->is_object() || it->is_array()){
                    value = kafkaDeserialize(it.value().dump());
                }
                else if(it->is_null()) {
                    value = Util::createNullConstant(DT_ANY);
                }
                else if(it->is_boolean()) {
                    value = Util::createConstant(DT_BOOL);
                    value->setBool(0, (bool)*it);
                }
                else if(it->is_string()){
                    value = Util::createConstant(DT_STRING);
                    string temp = it.value().dump();
                    value->setString(temp.substr(1,temp.length()-2));
                }
                else{
                    LOG_INFO(string("[PLUGIN::KAFKA] ") + string(*it) + ":un defined data type.");
                    value = Util::createNullConstant(DT_ANY);
                }
                result->append(value);
            }
            return result;
        }
    }
    } catch(std::exception& e) {
        string errMsg = e.what();
        if(errMsg.find("[PLUGIN::KAFKA]") == errMsg.npos) {
            throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
        } else {
            throw RuntimeException(e.what());
        }
    }
    DataInputStreamSP in = new DataInputStream(str, dataInit.length());
    ret = in->readShort(flag);
    auto data_form = static_cast<DATA_FORM>(flag >> 8);
    ConstantUnmarshalFactory factory(in, nullptr);
    ConstantUnmarshal* unmarshall = factory.getConstantUnmarshal(data_form);
    if(unmarshall == nullptr){
        throw RuntimeException(string("[PLUGIN::KAFKA] ") + "Failed to parse the incoming object: " + dataInit + ". Please poll the stream by kafka::pollByteStream.");
    }
    if (!unmarshall->start(flag, true, ret)) {
        unmarshall->reset();
        throw IOException(string("[PLUGIN::KAFKA] ") + "Failed to parse the incoming object with IO error type " + std::to_string(ret));
    }

    ConstantSP result = unmarshall->getConstant();

    return result;
}

ConstantSP kafkaGetSubJobConsumer(Heap *heap, vector<ConstantSP> &args) {
    std::string usage = "Usage: getSubJobConsumer(connection or connection ID). ";
    SubConnection *sc = nullptr;
    string key;
    ConstantSP conn = nullptr;
    auto handle = args[0];
    LockGuard<Mutex> _(&DICT_LATCH);

    getSubJobConn(handle, conn, sc, key);

    if (sc == nullptr) {
        throw RuntimeException("[PLUGIN::KAFKA] Invalid connection object.");
    }

    return sc->getConsumerHandle();
}