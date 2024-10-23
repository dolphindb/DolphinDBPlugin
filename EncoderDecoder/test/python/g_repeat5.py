import test_repeat_5_pb2

def setInfo(inst):
    inst.double.append(29493.12)
    inst.double.append(0)
    inst.double.append(-29493.12)


    r11 = inst.r1.add()
    r11.double = -23.42
    r11.float = -0.000001
    r12 = inst.r1.add()
    r12.double = 0
    r12.float = 0
    r13 = inst.r1.add()
    r13.double = -23.42
    r13.float = -0.000001
    r14 = inst.r1.add()
    r14.double = 43523.42
    r14.float = 2543.000001

    r21 = inst.r2.add()
    r21.long.append(23112324424)
    r21.long.append(0)
    r21.long.append(-2345)
    r21.long.append(1)

    r22 = inst.r2.add()
    r22.long.append(23132424)
    return inst

msg = test_repeat_5_pb2.test_repeat_5()
msg_cmpl = setInfo(msg)
proto_info = msg_cmpl.SerializeToString()

with open("./data_test_repeat_5.txt", "wb") as file:
    file.write(proto_info)

