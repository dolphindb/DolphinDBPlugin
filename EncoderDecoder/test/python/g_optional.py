# -*- coding: utf-8 -*-
import test_optional_pb2

def setInfo(inst):
    inst.record = "record_value"
    inst.type1.field1 = 123
    inst.type1.field2 = "type1_field2_value"
    inst.type1.statuses.add(code=200, message=0.1, blob=b'\x00\x01\x02')
    inst.type1.statuses.add(code=0, message=0.0, blob=b'')
    inst.type1.statuses.add(code=0, message=0.0, blob=b'')
    inst.type1.statuses.add(code=200, message=0.1, blob=b'\x17\x11')
    return inst

def setInfo3(inst):
    inst.record = "record_value"
    inst.type1.field1 = 123
    inst.type1.field2 = "type1_field2_value"
    inst.type1.statuses.add(code=0, message=0.0, blob=b'')
    inst.type1.statuses.add(code=0, message=0.0, blob=b'')
    return inst

def setInfo2(inst):
    inst.record = "record_value"
    inst.type1.field1 = 123
    inst.type1.field2 = "type1_field2_value"
    return inst

def setInfo1(inst):
    inst.record = "record_value"
    inst.type1.field1 = 123
    inst.type1.field2 = "type1_field2_value"
    inst.type1.statuses.add(code=200, message=0.1, blob=b'\x17\x01')
    inst.type2.field1 = 456
    inst.type2.field2 = "type2_field2_value"
    inst.type2.statuses.add(code=404, message=0.2, blob=b'\x08\x09')
    return inst

msg = test_optional_pb2.TestOptional()
msg_cmpl = setInfo3(msg)
proto_info = msg_cmpl.SerializeToString()

with open("./data_test_optional.txt", "wb") as file:
    file.write(proto_info)
