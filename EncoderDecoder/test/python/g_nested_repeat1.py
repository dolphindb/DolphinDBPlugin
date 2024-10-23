import test_nest_repeat_1_pb2

def setInfo(inst):
    inst.double = 2.001
    inst.float = 3.435
    inst.int32 = 3234
    inst.int64 = 232441342
    inst.bool = True
    inst.string = 'ddb1'
    r1 = inst.r1.add()
    r1.double = 2.001
    r1.float = 3.435
    r1.int32 = 3234
    r1.int64 = 232441342
    r1.bool = True
    r1.string = 'ddb2'
    n1 = r1.n1.add()
    n1.double = 2.001
    n1.float = 3.435
    n1.int32 = 3234
    n1.int64 = 232441342
    n1.bool = True
    n1.string = 'ddb3'
    
    return inst

msg = test_nest_repeat_1_pb2.test_nest_repeat_1()
msg_cmpl = setInfo(msg)
proto_info = msg_cmpl.SerializeToString()

with open("./data_test_nest_repeat_1.txt", "wb") as file:
    file.write(proto_info)
