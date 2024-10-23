import test_repeat_4_pb2

def setInfo(inst):
    inst.double = 1.13242244
    inst.r1.float.append(29493.12)
    inst.r1.float.append(0)
    inst.r1.float.append(-23.009)

    inr1 = inst.r2.inside.add()
    inr1.float.append(29493.0001)
    inr1.float.append(0)
    inr1.float.append(-0.0001)
    inr1.float.append(888)
    inr1.float.append(666)
    inr1.float.append(-0.0001)
    inr1.float.append(777)
    inr1.float.append(-555.01)
    inr1.float.append(-0.0001)

    inr2 = inst.r2.inside.add()

    inr3 = inst.r2.inside.add()

    return inst

msg = test_repeat_4_pb2.test_repeat_4()
msg_cmpl = setInfo(msg)
proto_info = msg_cmpl.SerializeToString()

with open("./data_test_repeat_4.txt", "wb") as file:
    file.write(proto_info)

