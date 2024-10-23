import test_repeat_1_pb2

def setInfo(inst):
    inst.double = 1.13242244
    # inst.float = 1.11738
    inst.string.append('1231')
    inst.string.append('-4252')
    inst.string.append('0')
    # inst.string.append('ddb1')
    # inst.string.append('ddb2')
    # inst.string.append('ddb3')
    return inst

msg = test_repeat_1_pb2.test_repeat_1()
msg_cmpl = setInfo(msg)
proto_info = msg_cmpl.SerializeToString()

with open("./data_test_repeat_1.txt", "wb") as file:
    file.write(proto_info)
    
