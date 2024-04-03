import test_repeat_7_pb2

def setInfo(inst):
    inst.double.append(29493.12)
    inst.double.append(0)
    inst.double.append(-29493.12)

    r1 = inst.r1.add()
    r1.double = 234
    r1.float = 0.010209
    r1.n1.int = 0
    r1.n1.long.append(8726348265345)
    r1.n1.long.append(-3453)
    r1.n1.long.append(0)
    r1.n1.bool.append(False)
    r1.n1.bool.append(True)

    r2 = inst.r1.add()
    r2.double = 234
    r2.float =- 0.010209
    r2.n1.int = 2245
    r2.n1.long.append(0)
    r2.n1.bool.append(False)
    r2.n1.bool.append(True)
    r2.n1.bool.append(False)
    r2.n1.bool.append(True)
    r2.n1.bool.append(False)
    r2.n1.bool.append(True)

    r3 = inst.r1.add()
    r3.double = 3223455.23452345

    r4 = inst.r1.add()


    return inst

msg = test_repeat_7_pb2.test_repeat_7()
msg_cmpl = setInfo(msg)
proto_info = msg_cmpl.SerializeToString()

with open("./data_test_repeat_7.txt", "wb") as file:
    file.write(proto_info)

