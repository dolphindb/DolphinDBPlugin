import test_repeat_8_pb2

def setInfo(inst):
    inst.double1.append(29493.12)
    inst.double1.append(0)
    inst.double1.append(-29493.12)

    r1 = inst.r1.add()
    r1.double2 = 234
    r1.float = 0.010209
    r1.n1.intIn = 0
    r1.n1.long.append(8726348265345)
    r1.n1.long.append(-3453)
    r1.n1.long.append(0)
    r1.n1.bool.append(False)
    r1.n1.bool.append(True)

    r2 = inst.r1.add()
    r2.double2 = 234
    r2.float = -0.010209
    r2.n1.intIn = 2245
    r2.n1.long.append(0)
    r2.n1.bool.append(False)
    r2.n1.bool.append(True)
    r2.n1.bool.append(False)
    r2.n1.bool.append(True)
    r2.n1.bool.append(False)
    r2.n1.bool.append(True)

    r3 = inst.r1.add()
    r3.double2 = 3223455.23452345

    r4 = inst.r1.add()

    inst.intOut = 9999

    return inst

msg = test_repeat_8_pb2.test_repeat_8()
msg_cmpl = setInfo(msg)
proto_info = msg_cmpl.SerializeToString()

with open("./data_test_repeat_8.txt", "wb") as file:
    file.write(proto_info)

