import test_enum_repeat_1_pb2

def setInfo(inst):
    inst.category = test_enum_repeat_1_pb2.test_enum_repeat.Category.WITHDRAW
    inst.direction.append(1)
    inst.direction.append(0)
    inst.direction.append(0)
    inst.direction.append(1)
    inst.direction.append(0)
    inst.channel = 12
    inst.price = 19000
    return inst

msg = test_enum_repeat_1_pb2.test_enum_repeat()
msg_cmpl = setInfo(msg)
proto_info = msg_cmpl.SerializeToString()

with open("./data_test_enum_repeat_1.txt", "wb") as file:
    file.write(proto_info)

