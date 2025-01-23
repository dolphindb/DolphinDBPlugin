import test_bad_oneof_pb2

def setInfo(inst):
    inst.float = 1.24
    inst.sint32 = 23424
    inst.sint64 = 23424
    inst.fixed32 = 23423
    inst.fixed64 = 23424
    return inst

msg = test_bad_oneof_pb2.test_bad_oneof()
msg_cmpl = setInfo(msg)
proto_info = msg_cmpl.SerializeToString()

with open("./data_test_bad_oneof.txt", "wb") as file:
    file.write(proto_info)
