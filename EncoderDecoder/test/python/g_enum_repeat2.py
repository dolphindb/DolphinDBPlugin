import test_enum_repeat_2_pb2

def setInfo(inst):
    inst.category = test_enum_repeat_2_pb2.test_enum_repeat.Category.WITHDRAW
    r1 = inst.repeat.add()
    r1.direction = 1
    r1.id = "emmm1"
    r2 = inst.repeat.add()
    r2.direction = 0
    r2.id = "emmm2"
    r3 = inst.repeat.add()
    r3.direction = 1
    r3.id = "emmm3"
    inst.channel = 12
    inst.price = 19000
    return inst

msg = test_enum_repeat_2_pb2.test_enum_repeat()
msg_cmpl = setInfo(msg)
proto_info = msg_cmpl.SerializeToString()

with open("./data_test_enum_repeat_2.txt", "wb") as file:
    file.write(proto_info)
