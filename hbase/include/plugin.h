// SPDX-License-Identifier: Apache-2.0
// Copyright © 2026-2026 DolphinDB, Inc.
#pragma once

#include "DolphinDBEverything.h"

extern "C" {

ddb::ConstantSP hbase_connect(ddb::Heap *heap, argsT &args);
ddb::ConstantSP hbase_show_tables(ddb::Heap *heap, argsT &args);
ddb::ConstantSP hbase_load(ddb::Heap *heap, argsT &args);
ddb::ConstantSP hbase_delete_table(ddb::Heap *heap, argsT &args);
ddb::ConstantSP hbase_get_row(ddb::Heap *heap, argsT &args);

}
