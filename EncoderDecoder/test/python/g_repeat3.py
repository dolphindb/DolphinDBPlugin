import test_repeat_3_pb2

def setInfo(inst):
    inst.double = 1.13242244
    inst.int.append(-1)
    inst.int.append(0)
    inst.int.append(23404)
    # inst.float = 1.11738
    r1 = inst.r1.add()
    r1.float.append(29493.12)
    r1.float.append(0)
    r1.float.append(-23.009)
    return inst

msg = test_repeat_3_pb2.test_repeat_3()
msg_cmpl = setInfo(msg)
proto_info = msg_cmpl.SerializeToString()

with open("./data_test_repeat_3.txt", "wb") as file:
    file.write(proto_info)

