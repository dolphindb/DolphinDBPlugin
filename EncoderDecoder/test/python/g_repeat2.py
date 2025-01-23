import test_repeat_2_pb2

def setInfo(inst):
    inst.double = 1245.024013249
    r1 = inst.r1.add()
    r2 = inst.r1.add()
    r3 = inst.r1.add()

    r1.double = 232.2424
    r1.float = 3.13342
    r1.int32 = 234
    r1.int64 = 249287573
    r1.bool = True
    r1.string = 'ddb1'

    r2.double = 232.2424
    r2.float = 0
    r2.int32 = 234
    r2.int64 = 0
    r2.bool = True
    r2.string = 'ddb2'

    # r2.double = 232
    # r2.float = 3
    # r2.int32 = -345353
    # r2.int64 = -902908807573
    # r2.bool = False
    # r2.string = 'ddb3'
    return inst

msg = test_repeat_2_pb2.test_repeat_2()
msg_cmpl = setInfo(msg)
proto_info = msg_cmpl.SerializeToString()

with open("./data_test_repeat_2.txt", "wb") as file:
    file.write(proto_info)
