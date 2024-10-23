import test_nest_repeat_2_pb2

def setInfo(inst):
    r1 = inst.r1.add()
    r1.double = 1.0001
    
    l1 = r1.l1.add()
    l1.float = -124.1

    l2 = l1.l2.add()
    l2.int32 = 3242

    l3 = l2.l3.add()
    l3.int64 = 23446279462

    l4 = l3.l4.add()
    l4.bool = True

    l5 = l4.l5.add()
    l5.string = 'ddb'
    return inst

msg = test_nest_repeat_2_pb2.test_nest_repeat_2()
msg_cmpl = setInfo(msg)
proto_info = msg_cmpl.SerializeToString()

with open("./data_test_nest_repeat_2.txt", "wb") as file:
    file.write(proto_info)
