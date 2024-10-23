import test_repeat_string_pb2

def setInfo(inst):
    inst.double1 = 29493.12

    r1 = inst.r1.add()
    r1.int0="1"
    r1.long0="100000000"
    r1.float0="5.2334"
    r1.double0="24245.23452452543"

    r2 = inst.r1.add()
    r2.int0="1"
    r2.long0="100000000"
    r2.float0="5.2334"
    r2.double0="24245.23452452543"

    r3 = inst.r1.add()
    r3.int0="wrong"
    r3.long0="wrong"
    r3.float0="wrong"
    r3.double0="wrong"

    return inst

msg = test_repeat_string_pb2.test_repeat_string()
msg_cmpl = setInfo(msg)
proto_info = msg_cmpl.SerializeToString()

with open("./data_test_repeat_string.txt", "wb") as file:
    file.write(proto_info)

