import test_repeat_6_pb2

def setInfo(inst):
    inst.double = 29493.12

    r1 = inst.r1.add()
    r1.double = 0.0002324
    r1.float = 1.0002324
    r1.n1.int = 23442
    r1.n1.long = -23442

    r2 = inst.r1.add()

    r3 = inst.r1.add()
    r3.double = 0.0002324
    r3.float = 0
    r3.n1.int = 0
    r3.n1.long = -23442

    return inst

msg = test_repeat_6_pb2.test_repeat_6()
msg_cmpl = setInfo(msg)
proto_info = msg_cmpl.SerializeToString()

with open("./data_test_repeat_6.txt", "wb") as file:
    file.write(proto_info)

