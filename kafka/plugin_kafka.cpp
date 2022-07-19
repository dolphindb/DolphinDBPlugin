#include "plugin_kafka.h"

using namespace cppkafka;
using namespace std;


ConstantSP kafkaProducer(Heap *heap, vector<ConstantSP> &args) {
    auto &dict = args[0];
    const auto usage = string("Usage: producer(dict[string, any]).\n");
    if (dict->getForm() != DF_DICTIONARY) {
        throw IllegalArgumentException(__FUNCTION__, usage + "Not a dict config.");
    }
    auto conf = createConf(dict);

    unique_ptr<Producer> producer(new Producer(conf));
    FunctionDefSP onClose(Util::createSystemProcedure("kafka producer onClose()", kafkaOnClose<Producer>, 1, 1));
    return Util::createResource(
            (long long) producer.release(),
            producer_desc,
            onClose,
            heap->currentSession()
    );
}

ConstantSP kafkaProducerFlush(Heap *heap, vector<ConstantSP> &args) {
    const auto usage = string("Usage: produceFlush(producer).\n");
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != producer_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "producer should be a producer handle.");
    getConnection<Producer>(args[0])->flush();
    return new Void();
}

ConstantSP kafkaProduce(Heap *heap, vector<ConstantSP> &args) {
    if(args.size() == 5){
        ConstantSP temp = Util::createNullConstant(DT_ANY);
        produceMessage(args[0], args[1], args[2], args[3], args[4], temp);
    }
    else
        produceMessage(args[0], args[1], args[2], args[3], args[4], args[5]);
    return new Void();
}

ConstantSP kafkaConsumer(Heap *heap, vector<ConstantSP> &args) {
    const auto usage = string("Usage: consumer(dict[string, any]).\n");
    auto &dict = args[0];

    if (dict->getForm() != DF_DICTIONARY) {
        throw IllegalArgumentException(__FUNCTION__, usage + "Not a dict config.");
    }

    auto conf = createConf(args[0], true);
    auto consumer = new Consumer(conf);

    FunctionDefSP onClose(Util::createSystemProcedure("kafka consumer onClose()", kafkaConsumerOnClose, 1, 1));

    return Util::createResource(
            (long long) consumer,
            consumer_desc,
            onClose,
            heap->currentSession()
    );
}

ConstantSP kafkaSubscribe(Heap *heap, vector<ConstantSP> &args) {
    const auto usage = string("Usage: subscribe(consumer, topics).\n");
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    auto conn = getConnection<Consumer>(args[0]);
    if (args[1]->getForm() != DF_VECTOR || args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "Not a topic vector.");
    }
    auto &vec = args[1];
    auto topics = vector<string>{};
    for (auto i = 0; i < vec->size(); i++) {
        topics.push_back(vec->get(i)->getString());
    }

    conn->subscribe(topics);
    return new Void();
}


static ConstantSP extractMessage(Message &msg) {
    auto topic = Util::createConstant(DT_STRING);
    auto partition = Util::createConstant(DT_INT);
    auto key = kafkaDeserialize(msg.get_key());
    auto value = kafkaDeserialize(msg.get_payload());
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
            "Usage: consumerPoll(consumer[,timeout:int]).\n"
            "return: [err, msg]\n"
            "err: empty if no error else error info string.\n"
            "msg: [topic, partition, key, value, timestamp].\n"
    );

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    auto consumer = getConnection<Consumer>(args[0]);

    // Try to consume a message
    if(args.size() == 2){
        if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
            throw IllegalArgumentException(__FUNCTION__, + "time need positive integer");
        }
        auto time = args[1]->getInt();
        auto msg = consumer->poll(std::chrono:: milliseconds(time));
        return getMsg(msg);
    }else{
        auto msg = consumer->poll();
        return getMsg(msg);
    }
}

ConstantSP kafkaPollByteStream(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: pollByteStream(consumer[,timeout:int]).\n"
            "return: [err/byte_stream]\n"
    );

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    auto consumer = getConnection<Consumer>(args[0]);
    Message msg;

    // Try to consume a message
    if(args.size() == 2){
        if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
            throw IllegalArgumentException(__FUNCTION__, + "time need positive integer");
        }
        auto time = args[1]->getInt();
        msg = consumer->poll(std::chrono:: milliseconds(time));
    }else{
        msg = consumer->poll();
    }

    auto result = Util::createConstant(DT_STRING);
    if (msg)
        if (msg.get_error())
            if (msg.is_eof())
                result->setString("Broker: No more messages");
            else
                result->setString(msg.get_error().to_string());
        else
            result->setString(string(msg.get_payload()));
    else
        result->setString("No more message");

    return result;
}

ConstantSP kafkaConsumerPollBatch(Heap *heap, vector<ConstantSP> &args) {
    const auto usage = string(
            "Usage: consumerPollBatch(consumer, batch_size[,timeout:int]).\n"
            "err: empty if no error else some other error string.\n"
            "msgs: vector [topic partition key value timestamp].\n"
    );

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    auto consumer = getConnection<Consumer>(args[0]);
    if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
        throw IllegalArgumentException(__FUNCTION__, usage + "batch size need positive integer");
    }
    auto batch_size = args[1]->getInt();
    auto result = Util::createVector(DT_ANY, 0);

    vector<Message> msgs;
    if(args.size() == 3){
        if (args[2]->getType() < DT_SHORT || args[2]->getType() > DT_LONG || args[2]->getInt() < 0) {
            throw IllegalArgumentException(__FUNCTION__, + "time need positive integer");
        }
        auto time = args[2]->getInt();
        msgs = consumer->poll_batch(batch_size, std::chrono:: milliseconds(time));
    }else{
        msgs = consumer->poll_batch(batch_size);
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

ConstantSP kafkaCreateSubJob(Heap *heap, vector<ConstantSP> args){
    const auto usage = string(
            "Usage: createSubJob(consumer,table,parser,description[,timeout:int]).\n"
    );

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    auto consumer = getConnection<Consumer>(args[0]);
    auto timeout = static_cast<int>(consumer->get_timeout().count());

    if (!(args[1]->isTable() || args[1]->getType()==DT_FUNCTIONDEF)) {
        throw IllegalArgumentException(__FUNCTION__, usage + "the second argument must be a table or a function.");
    }
    if(args[1]->getType() == DT_FUNCTIONDEF){
        FunctionDefSP handle = args[1];
        if(handle->getParamCount() != 1){
            throw IllegalArgumentException(__FUNCTION__, usage + "handle function must accept only one param.");
        }
    }
    if (args[2]->getType() != DT_FUNCTIONDEF) {
        throw IllegalArgumentException(__FUNCTION__, usage + "parser must be an function.");
    }
    FunctionDefSP parser = args[2];
    if(parser->getParamCount() != 1){
        throw IllegalArgumentException(__FUNCTION__, usage + "parser function must accept only one param.");
    }
    if (args[3]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "description must be an string.");
    }
    if(args.size() == 5) {
        if (args[4]->getType() < DT_SHORT || args[4]->getType() > DT_LONG || args[4]->getInt() < 0) {
            throw IllegalArgumentException(__FUNCTION__, +"time need positive integer");
        }
        timeout = args[4]->getInt();
    }

    std::unique_ptr<SubConnection> cup(new SubConnection(heap, args[3]->getString(), args[2], args[1], args[0], timeout));
    FunctionDefSP onClose(Util::createSystemProcedure("kafka sub connection onClose()", kafkaOnClose<SubConnection>, 1, 1));
    ConstantSP conn = Util::createResource(
            (long long)cup.release(),
            "kafka subscribe connection",
            onClose,
            heap->currentSession()
    );
    status_dict->set(std::to_string(conn->getLong()),conn);

    return conn;
}

ConstantSP kafkaGetJobStat(Heap *heap, vector<ConstantSP> &args){
    int size = status_dict->size();
    ConstantSP connectionIdVec = Util::createVector(DT_STRING, size);
    ConstantSP userVec = Util::createVector(DT_STRING, size);
    ConstantSP desVec = Util::createVector(DT_STRING, size);
    ConstantSP timestampVec = Util::createVector(DT_TIMESTAMP, size);
    VectorSP keys = status_dict->keys();
    for(int i = 0; i < keys->size();i++){
        string key = keys->getString(i);
        connectionIdVec->setString(i,key);
        ConstantSP conn = status_dict->getMember(key);
        auto *sc = (SubConnection *)(conn->getLong());
        desVec->setString(i,sc->getDescription());
        timestampVec->setLong(i,sc->getCreateTime());
        userVec->setString(i,sc->getSession()->getUser()->getUserId());
    }

    vector<string> colNames = {"subscriptionId","user","description","createTimestamp"};
    vector<ConstantSP> cols = {connectionIdVec,userVec,desVec,timestampVec};
    return Util::createTable(colNames,cols);
}

ConstantSP kafkaCancelSubJob(Heap *heap, vector<ConstantSP> args){
    // parse args first
    std::string usage = "Usage: cancelSubJob(connection or connection ID). ";
    SubConnection *sc = nullptr;
    string key;
    ConstantSP conn = nullptr;
    auto handle = args[0];
    switch (handle->getType()){
        case DT_RESOURCE:
            sc = (SubConnection *)(handle->getLong());
            key = std::to_string(handle->getLong());
            conn = status_dict->getMember(key);
            if(conn->isNothing())
                throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
            break;
        case DT_STRING:
            key = handle->getString();
            conn = status_dict->getMember(key);
            if(conn->isNothing())
                throw IllegalArgumentException(__FUNCTION__, "Invalid connection string.");
            else
                sc = (SubConnection *)(conn->getLong());
            break;
        case DT_LONG:
            key = std::to_string(handle->getLong());
            conn = status_dict->getMember(key);
            if(conn->isNothing())
                throw IllegalArgumentException(__FUNCTION__, "Invalid connection integer.");
            else
                sc = (SubConnection *)(conn->getLong());
            break;
        case DT_INT:
            key = std::to_string(handle->getInt());
            conn = status_dict->getMember(key);
            if(conn->isNothing())
                throw IllegalArgumentException(__FUNCTION__, "Invalid connection integer.");
            else
                sc = (SubConnection *)(conn->getLong());
            break;
        default:
            throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
    }

    bool bRemoved=status_dict->remove(new String(key));
    if (bRemoved && sc != nullptr) {
        sc->cancelThread();
        cout << "subscription: " << conn->getLong() << " : " << sc->getDescription() << " is stopped" << endl;
    }

    return new Void();
}

ConstantSP kafkaPollDict(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: pollDict(consumer, batch_size[,timeout:int]).\n"
            "err: empty if no error else some other error string.\n"
            "msgs: vector [topic partition key value timestamp].\n"
    );

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    auto consumer = getConnection<Consumer>(args[0]);
    if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
        throw IllegalArgumentException(__FUNCTION__, usage + "batch size need positive integer");
    }
    auto batch_size = args[1]->getInt();
    auto result = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);

    vector<Message> msgs;
    if(args.size() == 3){
        if (args[2]->getType() < DT_SHORT || args[2]->getType() > DT_LONG || args[2]->getInt() < 0) {
            throw IllegalArgumentException(__FUNCTION__, + "time need positive integer");
        }
        auto time = args[2]->getInt();
        msgs = consumer->poll_batch(batch_size, std::chrono:: milliseconds(time));
    }else{
        msgs = consumer->poll_batch(batch_size);
    }

    for (auto &msg: msgs){
        auto message = getMsg(msg);
        if(message->get(1)->get(1)->getType()!=DT_STRING){
            cout << "can only get string as key" << endl;
        }
        result->set(message->get(1)->get(1), message->get(1)->get(2));
    }
    if(msgs.size() == 0 || result->size() == 0){
        return new Void();
    }

    return result;
}

ConstantSP kafkaCommit(Heap *heap, vector<ConstantSP> &args) {
    const auto usage = string(
            "Usage: commit(consumer).\n"
    );
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    auto conn = getConnection<Consumer>(args[0]);
    conn->commit();
    return new Void();
}

ConstantSP kafkaCommitTopic(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: commitTopic(consumer,topic:vector<string>,partition:vector<int>,offset:vector<int>).\n"
    );

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    auto consumer = getConnection<Consumer>(args[0]);
    Convertion convert(usage,args);
    consumer->commit(convert.topic_partitions);

    return new Void();
}

ConstantSP kafkaAsyncCommit(Heap *heap, vector<ConstantSP> &args) {
    const auto usage = string(
            "Usage: asyncCommit(consumer).\n"
    );
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    auto conn = getConnection<Consumer>(args[0]);
    conn->async_commit();
    return new Void();
}

ConstantSP kafkaAsyncCommitTopic(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: asyncCommitTopic(consumer,topic:vector<string>,partition:vector<int>,offset:vector<int>).\n"
    );

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    auto consumer = getConnection<Consumer>(args[0]);
    Convertion convert(usage,args);
    consumer->async_commit(convert.topic_partitions);

    return new Void();
}

ConstantSP kafkaUnsubscribe(Heap *heap, vector<ConstantSP> &args) {
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__,"consumer should be a consumer handle.");
    auto conn = getConnection<Consumer>(args[0]);
    conn->unsubscribe();
    return new Void();
}

Configuration createConf(ConstantSP & dict, bool consumer) {
    Configuration configuration = {
            {"metadata.broker.list", "115.239.209.234:9092"},
    };
    bool group_id = false;

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
        if(key->getString() == "group.id")
            group_id = true;
    }
    if(consumer && !group_id)
        throw IllegalArgumentException(__FUNCTION__, "consumer need setting group.id");

    return configuration;
}

ConstantSP kafkaSetConsumerTimeout(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: setConsumerTime(consumer,timeout).\n"
    );
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    auto consumer = getConnection<Consumer>(args[0]);
    if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
        throw IllegalArgumentException(__FUNCTION__, usage + "time need positive integer");
    }
    auto time = args[1]->getInt();
    consumer->set_timeout(std::chrono:: milliseconds(time));

    return new Void();
}

ConstantSP kafkaSetProducerTimeout(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: setProducerTime(producer,timeout).\n"
    );
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != producer_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "producer should be a producer handle.");
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
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    auto consumer = getConnection<Consumer>(args[0]);
    auto res = Util::createConstant(DT_INT);
    res->setInt(static_cast<int>(consumer->get_timeout().count()));
    return res;
}

ConstantSP kafkaGetProducerTimeout(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: getProducerTime(producer).\n"
    );
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != producer_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "producer should be a producer handle.");
    auto producer = getConnection<Producer>(args[0]);
    auto res = Util::createConstant(DT_INT);
    res->setInt(static_cast<int>(producer->get_timeout().count()));
    return res;
}

ConstantSP kafkaConsumerAssign(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: assign(consumer,topic:vector<string>,partition:vector<int>,offset:vector<int>).\n"
    );

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    auto consumer = getConnection<Consumer>(args[0]);
    Convertion convert(usage,args);
    consumer->assign(convert.topic_partitions);

    return new Void();
}

ConstantSP kafkaConsumerUnassign(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, "consumer should be a consumer handle.");
    auto consumer = getConnection<Consumer>(args[0]);
    consumer->unassign();
    return new Void();
}

ConstantSP kafkaConsumerGetAssignment(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, "consumer should be a consumer handle.");
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
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, "consumer should be a consumer handle.");
    auto consumer = getConnection<Consumer>(args[0]);
    consumer->pause();
    return new Void();
}

ConstantSP kafkaConsumerResume(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, "consumer should be a consumer handle.");
    auto consumer = getConnection<Consumer>(args[0]);
    consumer->resume();
    return new Void();
}

ConstantSP kafkaGetOffset(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: getOffset(consumer,topic:string,partition:int).\n"
    );
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    auto consumer = getConnection<Consumer>(args[0]);
    if (args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "Topic must be string.");
    }
    auto topic = args[1]->getString();
    if (args[2]->getType() != DT_INT || args[2]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "partition must be a integer");
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
            "Usage: getOffsetCommitted(consumer,topic:vector<string>,partition:vector<int>,offset:vector<int>[,timeout:int]).\n"
    );

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    auto consumer = getConnection<Consumer>(args[0]);
    Convertion convert(usage,args);

    vector<TopicPartition> result;
    if(args.size() == 5){
        if (args[4]->getType() < DT_SHORT || args[4]->getType() > DT_LONG || args[4]->getInt() < 0) {
            throw IllegalArgumentException(__FUNCTION__, + "time need positive integer");
        }
        auto time = args[4]->getInt();
        result = consumer->get_offsets_committed(convert.topic_partitions, std::chrono::milliseconds(time));
    }else{
        result = consumer->get_offsets_committed(convert.topic_partitions);
    }

    for(int i = 0;i<(int)result.size();i++){
        cout << result[i] << endl;
    }

    return new Void();
}

ConstantSP kafkaGetOffsetPosition(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: getOffsetPosition(consumer,topic:vector<string>,partition:vector<int>).\n"
    );
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    auto consumer = getConnection<Consumer>(args[0]);
    Convertion convert(usage,args);
    auto result = consumer->get_offsets_position(convert.topic_partitions);
    for(auto & i : result){
        cout << i << endl;
    }

    return new Void();
}

#if (RD_KAFKA_VERSION >= RD_KAFKA_STORE_OFFSETS_SUPPORT_VERSION)
ConstantSP kafkaStoreConsumedOffsets(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, "consumer should be a consumer handle.");
    auto consumer = getConnection<Consumer>(args[0]);
    consumer->store_consumed_offsets();
    return new Void();
}

ConstantSP kafkaStoreOffsets(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: storeOffset(consumer,topic:vector<string>,partition:vector<int>,offset:vector<int>).\n"
    );
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a consumer handle.");
    auto consumer = getConnection<Consumer>(args[0]);
    Convertion convert(usage,args);
    consumer->store_offsets(convert.topic_partitions);

    return new Void();
}
#endif

ConstantSP kafkaGetMemberId(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, "consumer should be a consumer handle.");
    auto consumer = getConnection<Consumer>(args[0]);
    auto result = Util::createConstant(DT_STRING);
    auto str = consumer->get_member_id();
    result->setString(0,str);

    return result;
}

ConstantSP kafkaQueueLength(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != queue_desc)
        throw IllegalArgumentException(__FUNCTION__, "queue should be a queue handle.");
    auto queue = getConnection<Queue>(args[0]);
    auto result = Util::createConstant(DT_INT);
    result->setInt(queue->get_length());
    return result;
}

ConstantSP  kafkaForToQueue(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != queue_desc)
        throw IllegalArgumentException(__FUNCTION__, "the first queue should be a queue handle.");
    if(args[1]->getType()!=DT_RESOURCE || args[1]->getString() != queue_desc)
        throw IllegalArgumentException(__FUNCTION__, "the second queue should be a queue handle.");
    auto queue = getConnection<Queue>(args[0]);
    auto forward_queue = getConnection<Queue>(args[1]);
    queue->forward_to_queue(*forward_queue);
    return new Void();
}

ConstantSP kafkaDisForToQueue(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != queue_desc)
        throw IllegalArgumentException(__FUNCTION__, "queue should be a queue handle.");
    auto queue = getConnection<Queue>(args[0]);
    queue->disable_queue_forwarding();
    return new Void();
}

ConstantSP kafkaSetQueueTime(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != queue_desc)
        throw IllegalArgumentException(__FUNCTION__, "queue should be a queue handle.");
    auto queue = getConnection<Queue>(args[0]);
    if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
        throw IllegalArgumentException(__FUNCTION__, + "time need positive integer");
    }
    auto time = args[1]->getInt();
    queue->set_timeout(std::chrono:: milliseconds(time));
    return new Void();
}

ConstantSP kafkaGetQueueTime(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != queue_desc)
        throw IllegalArgumentException(__FUNCTION__, "queue should be a queue handle.");
    auto queue = getConnection<Queue>(args[0]);
    auto result = Util::createConstant(DT_INT);
    result->setInt(static_cast<int>(queue->get_timeout().count()));
    return result;
}

ConstantSP kafkaQueueConsume(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: queuePoll(queue[,timeout]).\n"
            "return: [err, msg]\n"
            "err: empty if no error else error info string.\n"
            "msg: [topic, partition, key, value, timestamp].\n"
    );

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != queue_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "queue should be a queue handle.");
    auto queue = getConnection<Queue>(args[0]);
    if(args.size() == 2){
        if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
            throw IllegalArgumentException(__FUNCTION__, + "time need positive integer");
        }
        auto time = args[1]->getInt();
        auto msg = queue->consume(std::chrono:: milliseconds(time));
        return getMsg(msg);
    }else{
        auto msg = queue->consume();
        return getMsg(msg);
    }
}

ConstantSP kafkaQueueConsumeBatch(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string(
            "Usage: queuePollBatch(queue, batch_size[,timeout]).\n"
            "err: empty if no error else some other error string.\n"
            "msgs: vector [topic partition key value timestamp].\n"
    );

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != queue_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "queue should be a queue handle.");
    auto queue = getConnection<Queue>(args[0]);
    if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
        throw IllegalArgumentException(__FUNCTION__, usage + "batch size need positive integer");
    }
    auto batch_size = args[1]->getInt();
    auto result = Util::createVector(DT_ANY, 0);

    vector<Message> msgs;
    if(args.size() == 3){
        if (args[2]->getType() < DT_SHORT || args[2]->getType() > DT_LONG || args[2]->getInt() < 0) {
            throw IllegalArgumentException(__FUNCTION__, + "time need positive integer");
        }
        auto time = args[2]->getInt();
        msgs = queue->consume_batch(batch_size, std::chrono:: milliseconds(time));
    }else{
        msgs = queue->consume_batch(batch_size);
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

ConstantSP kafkaGetMainQueue(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, "consumer should be a consumer handle.");
    auto consumer = getConnection<Consumer>(args[0]);
    auto queue = new Queue(consumer->get_main_queue());

    FunctionDefSP onClose(Util::createSystemProcedure("kafka queue onClose()", kafkaOnClose<Queue>, 1, 1));
    return Util::createResource(
            (long long) queue,
            queue_desc,
            onClose,
            heap->currentSession()
    );
}

ConstantSP kafkaGetConsumerQueue(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, "consumer should be a consumer handle.");
    auto consumer = getConnection<Consumer>(args[0]);
    auto queue = new Queue(consumer->get_consumer_queue());

    FunctionDefSP onClose(Util::createSystemProcedure("kafka queue onClose()", kafkaOnClose<Queue>, 1, 1));
    return Util::createResource(
            (long long) queue,
            queue_desc,
            onClose,
            heap->currentSession()
    );
}

ConstantSP kafkaGetPartitionQueue(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != consumer_desc)
        throw IllegalArgumentException(__FUNCTION__, "consumer should be a consumer handle.");
    auto consumer = getConnection<Consumer>(args[0]);
    Convertion convert("",args);
    auto queue = new Queue(consumer->get_partition_queue(convert.topic_partitions[0]));

    FunctionDefSP onClose(Util::createSystemProcedure("kafka queue onClose()", kafkaOnClose<Queue>, 1, 1));
    return Util::createResource(
            (long long) queue,
            queue_desc,
            onClose,
            heap->currentSession()
    );
}

ConstantSP kafkaQueueEvent(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != queue_desc)
        throw IllegalArgumentException(__FUNCTION__, "queue should be a queue handle.");
    auto queue = getConnection<Queue>(args[0]);
    auto event = new Event(queue->next_event());

    FunctionDefSP onClose(Util::createSystemProcedure("kafka event onClose()", kafkaOnClose<Event>, 1, 1));
    return Util::createResource(
            (long long) event,
            event_desc,
            onClose,
            heap->currentSession()
    );
}

ConstantSP kafkaGetEventName(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != event_desc)
        throw IllegalArgumentException(__FUNCTION__, "event should be a event handle.");
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

    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != event_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "event should be a event handle.");
    auto event = getConnection<Event>(args[0]);
    if(!event->operator bool()){
        throw Exception("The event is empty!");
    }
    auto result = Util::createVector(DT_ANY, 0);
    auto msgs = event->get_messages();

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
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != event_desc)
        throw IllegalArgumentException(__FUNCTION__, "event should be a event handle.");
    auto event = getConnection<Event>(args[0]);
    if(!event->operator bool()){
        throw Exception("The event is empty!");
    }
    auto count = Util::createConstant(DT_INT);
    count->setInt(event->get_message_count());
    return count;
}

ConstantSP kafkaEventGetError(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != event_desc)
        throw IllegalArgumentException(__FUNCTION__, "event should be a event handle.");
    auto event = getConnection<Event>(args[0]);
    if(!event->operator bool()){
        throw Exception("The event is empty!");
    }
    auto error = event->get_error();
    auto string = Util::createConstant(DT_STRING);
    string->setString(error.to_string());
    return string;
}

ConstantSP kafkaEventGetPartition(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != event_desc)
        throw IllegalArgumentException(__FUNCTION__, "event should be a event handle.");
    auto event = getConnection<Event>(args[0]);
    if(!event->operator bool()){
        throw Exception("The event is empty!");
    }
    cout <<  event->get_topic_partition() << endl;
    return new Void();
}

ConstantSP kafkaEventGetPartitionList(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != event_desc)
        throw IllegalArgumentException(__FUNCTION__, "event should be a event handle.");
    auto event = getConnection<Event>(args[0]);
    auto parts = event->get_topic_partition_list();
    for(auto &part:parts){
        cout << part << endl;
    }
    return new Void();
}

ConstantSP kafkaEventBool(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType()!=DT_RESOURCE || args[0]->getString() != event_desc)
        throw IllegalArgumentException(__FUNCTION__, "event should be a event handle.");
    auto event = getConnection<Event>(args[0]);
    auto result = Util::createConstant(DT_BOOL);
    result->setBool(1,event->operator bool());
    return result;
}

ConstantSP kafkaGetBufferSize(Heap *heap, vector<ConstantSP> &args){
    auto result = Util::createConstant(DT_LONG);
    result->setLong(buffer_size);
    return result;
}

ConstantSP kafkaSetBufferSize(Heap *heap, vector<ConstantSP> &args){
    long long size = args[0]->getLong();
    if(size>=message_size)
        buffer_size = size;
    else{
        buffer_size = size;
        message_size = size;
        cout << "The buffer_size is smaller than message_size. The message_size is set the same as buffer_size" << endl;
    }
    cout << "The buffer size has been successfully set, please make sure the buffer size is no larger than broker size." << endl;
    return new Void();
}

ConstantSP kafkaGetMessageSize(Heap *heap, vector<ConstantSP> &args){
    auto result = Util::createConstant(DT_LONG);
    result->setLong(message_size);
    return result;
}

ConstantSP kafkaSetMessageSize(Heap *heap, vector<ConstantSP> &args){
    long long size = args[0]->getLong();
    if(size<=buffer_size)
        message_size = size;
    else{
        throw IllegalArgumentException(__FUNCTION__, + "message_size should be larger than buffer_size");
    }

    return new Void();
}

inline static int getLength(ConstantSP &data){
    int type = (int)data->getType();
    if(type<=16)
        return type_size[type];
    else if(type == 17 || type == 18 || type == 32)
        return data->getString().length();
    else if(type == 30 || type == 31)
        return 16;
    else if(type == 20)
        return 128;
    else
        throw Exception("Unsolved type.");
}

static void produceMessage(ConstantSP &produce, ConstantSP &pTopic, ConstantSP &key, ConstantSP &value, ConstantSP &json, ConstantSP &pPartition){
    const auto usage = string("Usage: produce(producer, topic: string, key, value, json[,partition]).\n");
    if(produce->getType()!=DT_RESOURCE || produce->getString() != producer_desc)
        throw IllegalArgumentException(__FUNCTION__, usage + "producer should be a producer handle.");
    auto producer = getConnection<Producer>(produce);
    if (pTopic->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "Topic must be string.");
    }
    auto topic = pTopic->getString();
    ConstantSP judge = Util::createConstant(DT_BOOL);
    judge->setBool(1, false);
    auto key_str = kafkaSerialize(key, judge);
    auto value_str = kafkaSerialize(value, json);

    if(key_str.length() + value_str.length() <= buffer_size*(2-factor)){
        auto &&msg = MessageBuilder(topic).key(key_str).payload(value_str);
        if (!pPartition->isNull()) {
            if (pPartition->getType() != DT_INT || pPartition->getForm() != DF_SCALAR) {
                throw IllegalArgumentException(__FUNCTION__, usage + "partition must be a integer");
            }
            auto partition = pPartition->getInt();
            msg.partition(partition);
        }
        producer->produce(msg);
    }
    else{
        long long start = 0, end;
        long long size = 0;
        long long len;
        long long length;
        vector<int> str_vec;
        int head_len = 0;
        int row_len = 0;

        if(value->getForm() == DF_TABLE){
            length = value->columns();
            for(int i = 0;i<value->columns();i++){
                head_len+=value->keys()->get(i)->getString().length();
                if(value->values()->get(i)->getType() == DT_STRING || value->values()->get(i)->getType() == DT_SYMBOL)
                    str_vec.push_back(i);
                else{
                    ConstantSP temp = value->values()->get(i);
                    row_len+=getLength(temp);
                }
            }
            if(str_vec.empty()){
                long long step = (message_size-head_len)/row_len;
                if(step == 0){
                    if(row_len+head_len>buffer_size*factor)
                        throw Exception("The data is too largs.");
                    else
                        step = 1;
                }
                for(long long i = 0;i<value->size();i+=step){
                    auto pass = value->getWindow(0,length,i,(i+step>=value->size())?(value->size()-i):step);
                    produceMessage(produce,pTopic,key,pass,json,pPartition);
                }
            }
            else {
                size = head_len;
                for (long long i = 0; i < value->size(); i++) {
                    len = row_len;
                    for(int j : str_vec)
                        len += value->values()->get(j)->get(i)->getString().length();
                    size+=len;
                    if (size > buffer_size*factor){
                        if(size>head_len){
                            auto pass = value->getWindow(0,length,start,i-start);
                            produceMessage(produce, pTopic, key, pass, json, pPartition);
                            start = i;
                            size = head_len+len;
                        }
                        else
                            throw IllegalArgumentException(__FUNCTION__, usage + "size of data is too big");
                    }
                    else if (size > message_size*factor) {
                        auto pass = value->getWindow(0,length,start,i-start+1);
                        produceMessage(produce, pTopic, key, pass, json, pPartition);
                        start = i + 1;
                        size = head_len;
                    }
                }
                if(size>head_len){
                    auto pass = value->getWindow(0,length,start,value->size()-start);
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
                if(len>buffer_size*factor)
                    throw IllegalArgumentException(__FUNCTION__, usage + "size of data is too big");
                else if(len>message_size*factor){
                    if(size!=0){
                        auto pass = vector->getSubVector(start, end-start+1);
                        produceMessage(produce,pTopic,key,pass,json,pPartition);
                        size = 0;
                    }
                    auto pass_temp = vector->getSubVector(i, 1);
                    produceMessage(produce,pTopic,key,pass_temp,json,pPartition);
                    start=i+1;
                }
                else if(len+size<message_size*factor){
                    size+=len;
                    end = i;
                }
                else if(len+size>=buffer_size*factor){
                    end = i-1;
                    auto pass = vector->getSubVector(start,end-start+1);
                    produceMessage(produce,pTopic,key,pass,json,pPartition);
                    size = len;
                    start = i;
                }
                else if(len+size>=message_size*factor){
                    end = i;
                    auto pass = vector->getSubVector(start,end-start+1);
                    produceMessage(produce,pTopic,key,pass,json,pPartition);
                    size = 0;
                    start = i+1;
                }
            }
            if(size!=0){
                end = length-1;
                auto pass = vector->getSubVector(start,end-start+1);
                produceMessage(produce,pTopic,key,pass,json,pPartition);
            }
        }

        else
            throw IllegalArgumentException(__FUNCTION__, usage + "size of data is too big");
    }
}

static Vector* getMsg(Message &msg){
    auto err_arg = Util::createConstant(DT_STRING);

    if (msg) {
        ConstantSP msg_arg;
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

static string kafkaGetString(const ConstantSP &data, bool key){
    if(data->getForm() == DF_SCALAR) {
        if(key && data->getType()!=DT_STRING && data->getType()!=DT_CHAR && data->getType() != DT_BOOL){
            cout << "The key of json will be cast to string." << endl;
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
            cout << "The data type will be cast to string." << endl;
            return "\"" + data->getString() + "\"";
        }
    }
    else
        return kafkaJsonSerialize(data);
}

static string kafkaJsonSerialize(const ConstantSP &data){
    auto result = string("");
    if(data->getForm() == DF_VECTOR){
        if(data->size() == 0){
            return string("");
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
        result+="{";
        result+=kafkaGetString(data->keys()->get(length-1), true);
        result+=":";
        result+=kafkaGetString(data->values()->get(length-1));
        for(int i = length-2;i>=0;i--){
            result+=",";
            result+=kafkaGetString(data->keys()->get(i), true);
            result+=":";
            result+=kafkaGetString(data->values()->get(i));
        }
        result+="}";
    }
    else if(data->getForm() == DF_TABLE){
        if(data->size() == 0 || data->columns() == 0)
            return string("{}");
        result+="{";
        result+=kafkaGetString(data->keys()->get(0), true);
        result+=":";
        result+=kafkaJsonSerialize(data->values()->get(0));
        for(int i = 1;i<data->columns();i++){
            result+=",";
            result+=kafkaGetString(data->keys()->get(i), true);
            result+=":";
            result+=kafkaJsonSerialize(data->values()->get(i));
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

static string kafkaSerialize(ConstantSP &data, ConstantSP &json){
    if(data->getType() == DT_FUNCTIONDEF){
        throw IllegalArgumentException(__FUNCTION__, "Can't pass function type.");
    }
    if((int)json->getBool() == 1)
        return kafkaJsonSerialize(data);
    auto result = string("");
    IO_ERR ret;

    DataOutputStreamSP outStream = new DataOutputStream();
    ConstantMarshallFactory marshallFactory(outStream);
    ConstantMarshall* marshall = marshallFactory.getConstantMarshall(data->getForm());
    marshall->start(data, true, ret);
    result+=string(outStream->getBuffer(), outStream->size());

    ret = outStream->flush();
    return result;
}

static ConstantSP kafkaDeserialize(const string &data_init){
    if(data_init == ""){
        auto empty = Util::createNullConstant(DT_ANY);
        return empty;
    }
    short flag;
    IO_ERR ret;
    auto str = data_init.c_str();
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
                    value->setBool(0, *it);
                }
                else if(it.value().is_null()) {
                    value = Util::createNullConstant(DT_ANY);
                }
                else if(it.value().is_number_integer() || it.value().is_number_unsigned()) {
                    if(*it>0x7fffffffffffffffL){
                        cout << "The integer is too large and it will be cast to string." << endl;
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
                    cout << *it << ":un defined data type." << endl;
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
                        cout << "The integer is too large and it will be cast to string." << endl;
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
                    value->setBool(0, *it);
                }
                else if(it->is_string()){
                    value = Util::createConstant(DT_STRING);
                    string temp = it.value().dump();
                    value->setString(temp.substr(1,temp.length()-2));
                }
                else{
                    cout << *it << ":un defined data type." << endl;
                    value = Util::createNullConstant(DT_ANY);
                }
                result->append(value);
            }
            return result;
        }
    }
    DataInputStreamSP in = new DataInputStream(str, data_init.length());
    ret = in->readShort(flag);
    auto data_form = static_cast<DATA_FORM>(flag >> 8);
    ConstantUnmarshallFactory factory(in, nullptr);
    ConstantUnmarshall* unmarshall = factory.getConstantUnmarshall(data_form);
    if(unmarshall == nullptr){
        throw Exception("Failed to parse the incoming object: " + data_init + ". Please poll the stream by kafka::pollByteStream.");
    }
    if (!unmarshall->start(flag, true, ret)) {
        unmarshall->reset();
        throw IOException("Failed to parse the incoming object with IO error type " + std::to_string(ret));
    }

    ConstantSP result = unmarshall->getConstant();

    return result;
}