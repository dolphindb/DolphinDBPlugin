import test_bad_unrelated_repeat_1_pb2

def setInfo(inst):
    inst.int.append(1)
    inst.int.append(231)
    inst.int.append(13241)
    inst.string.append('ddb1')
    inst.string.append('ddb2')
    inst.bool.append(True)
    return inst

msg = test_bad_unrelated_repeat_1_pb2.test_bad_unrelated_repeat_1()
msg_cmpl = setInfo(msg)
proto_info = msg_cmpl.SerializeToString()

with open("./data_test_bad_unrelated_repeat_1.txt", "wb") as file:
    file.write(proto_info)
