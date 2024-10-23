/***************************************************************************
 * Generic routines to pack miniSEED records using an MS3Record as a
 * header template and data source.
 *
 * This file is part of the miniSEED Library.
 *
 * Copyright (c) 2020 Chad Trabant, IRIS Data Management Center
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string>

#include "libmseed.h"
#include "mseedformat.h"
#include "packdata.h"
#include "parson.h"
#include "Exceptions.h"
extern "C" {
#include "pack.h"
}
using namespace std;
#ifndef MSRCODE_H
#define MSRCODE_H
extern string
msr3_code_mseed2 (MS3Record *msr, uint32_t flags, int8_t verbose);

#endif //MSRCODE_H
