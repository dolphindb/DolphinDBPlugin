import test_bad_unrelated_repeat_2_pb2

def setInfo(inst):
    inst.string = "dolphindb"
    r1 = inst.r1.add()
    r2 = inst.r2.add()
    r1.int32 = 23234
    r2.double = 3.435

    return inst

msg = test_bad_unrelated_repeat_2_pb2.test_bad_unrelated_repeat_2()
msg_cmpl = setInfo(msg)
proto_info = msg_cmpl.SerializeToString()

with open("./data_test_bad_unrelated_repeat_2.txt", "wb") as file:
    file.write(proto_info)
