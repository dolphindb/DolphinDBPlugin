import test_types_pb2

def setInfo(inst):
    inst.double =  0.0000001
    inst.float = 900234.1
    inst.int32 = -435
    inst.int64 = 230742945245
    inst.uint32 = 4294967295
    inst.uint64 = 9223372036854775806
    inst.sint32 = -23424
    inst.sint64 = 24262562463342
    inst.fixed32 = 234249
    inst.fixed64 = 17446743073709551615
    inst.sfixed32 = 11
    inst.sfixed64 = 12
    inst.bool= False
    inst.string = "dolphindb"
    inst.bytes = b'\x13bytepart1\"\xfc\x21\nbytepart2'
    return inst

msg = test_types_pb2.test_types()
msg_cmpl = setInfo(msg)
proto_info = msg_cmpl.SerializeToString()

with open("./data_test_types1.txt", "wb") as file:
    file.write(proto_info)
