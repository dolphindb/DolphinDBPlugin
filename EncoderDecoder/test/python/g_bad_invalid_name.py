import test_bad_invalid_name_pb2

def setInfo(inst):
    inst.double = 1.1
    inst.m1.m2_double = 234.34
    inst.m1.m2.double = 23424
    return inst

msg = test_bad_invalid_name_pb2.test_bad_invalid_name()
msg_cmpl = setInfo(msg)
proto_info = msg_cmpl.SerializeToString()

with open("./data_test_bad_invalid_name.txt", "wb") as file:
    file.write(proto_info)
