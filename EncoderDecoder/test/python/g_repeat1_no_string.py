import test_repeat_1_no_string_pb2

def setInfo(inst):
    inst.double = 1.13242244
    # inst.float = 1.11738
    inst.int32.append(1231)
    inst.int32.append(-4252)
    inst.int32.append(0)
    return inst

msg = test_repeat_1_no_string_pb2.test_repeat_1()
msg_cmpl = setInfo(msg)
proto_info = msg_cmpl.SerializeToString()

with open("./data_test_repeat_1_no_string.txt", "wb") as file:
    file.write(proto_info)
    
