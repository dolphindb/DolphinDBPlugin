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
#include "msrcode.h"
using namespace std;
using namespace ddb;

string
msr3_code_mseed2 (MS3Record *msr, uint32_t flags, int8_t verbose)
{
    string ret;
    char *rawrec = NULL;
    char *encoded = NULL;  /* Separate encoded data buffer for alignment */
    int8_t swapflag;
    int dataoffset = 0;
    int headerlen;

    int samplesize;
    int maxdatabytes;
    int maxsamples;
    int recordcnt = 0;
    int packsamples;
    int packoffset;
    int64_t totalpackedsamples;

    uint16_t datalength;
    nstime_t nextstarttime;
    uint16_t year;
    uint16_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint32_t nsec;

    if (!msr)
    {
        throw RuntimeException("Required argument not defined: 'msr'");
    }

    if (msr->reclen < 128)
    {
        throw RuntimeException("Record length is not large enough, must be >= 128 bytes");
    }

    /* Check that record length is a power of 2.
     * Power of two if (X & (X - 1)) == 0 */
    if ((msr->reclen & (msr->reclen - 1)) != 0)
    {
        throw RuntimeException("Cannot create miniSEED 2, record length is not a power of 2");
    }

    /* Check to see if byte swapping is needed, miniSEED 2 is written big endian */
    swapflag = (ms_bigendianhost ()) ? 0 : 1;

    /* Allocate space for data record */
    rawrec = (char *)libmseed_memory.malloc (msr->reclen);

    if (rawrec == NULL)
    {
        throw RuntimeException("Cannot allocate memory");
    }

    /* Pack fixed header and extra headers, returned size is data offset */
    headerlen = msr3_pack_header2 (msr, rawrec, msr->reclen, verbose);

    if (headerlen < 0){
        libmseed_memory.free (rawrec);
        throw RuntimeException("Cannot parse SEED identifier codes from full identifier");
    }

    /* Short cut: if there are no samples, record packing is complete */
    if (msr->numsamples <= 0)
    {
        /* Set encoding to ASCII for consistency and to reduce expectations */
        *pMS2B1000_ENCODING (rawrec + 48) = DE_ASCII;

        /* Set empty part of record to zeros */
        memset (rawrec + headerlen, 0, msr->reclen - headerlen);

        if (verbose >= 1)
            ms_log (0, "%s: Packed %d byte record with no payload\n", msr->sid, msr->reclen);

        /* Send record to handler */
        //record_handler (rawrec, msr->reclen, handlerdata);
        ret.append(rawrec, msr->reclen);

        libmseed_memory.free (rawrec);

        return ret;
    }

    samplesize = ms_samplesize (msr->sampletype);

    if (!samplesize)
    {
        throw RuntimeException ("Unknown sample type");
    }

    /* Determine offset to encoded data */
    if (msr->encoding == DE_STEIM1 || msr->encoding == DE_STEIM2)
    {
        dataoffset = 64;
        while (dataoffset < headerlen)
            dataoffset += 64;

        /* Zero memory between blockettes and data if any */
        memset (rawrec + headerlen, 0, dataoffset - headerlen);
    }
    else
    {
        dataoffset = headerlen;
    }

    /* Set data offset in header */
    *pMS2FSDH_DATAOFFSET(rawrec) = HO2u (dataoffset, swapflag);

    /* Determine the max data bytes and sample count */
    maxdatabytes = msr->reclen - dataoffset;

    if (msr->encoding == DE_STEIM1)
    {
        maxsamples = (int)(maxdatabytes / 64) * STEIM1_FRAME_MAX_SAMPLES;
    }
    else if (msr->encoding == DE_STEIM2)
    {
        maxsamples = (int)(maxdatabytes / 64) * STEIM2_FRAME_MAX_SAMPLES;
    }
    else
    {
        maxsamples = maxdatabytes / samplesize;
    }

    /* Allocate space for encoded data separately for alignment */
    if (msr->numsamples > 0)
    {
        encoded = (char *)libmseed_memory.malloc (maxdatabytes);

        if (encoded == NULL)
        {
            libmseed_memory.free (rawrec);
            throw RuntimeException ("Cannot allocate memory");
        }
    }

    /* Pack samples into records */
    totalpackedsamples = 0;
    packoffset = 0;

    while ((msr->numsamples - totalpackedsamples) > maxsamples || flags & MSF_FLUSHDATA)
    {
        packsamples = msr_pack_data (encoded,
                                     (char *)msr->datasamples + packoffset,
                                     (int)(msr->numsamples - totalpackedsamples), maxdatabytes,
                                     msr->sampletype, msr->encoding, swapflag,
                                     &datalength, msr->sid, verbose);

        if (packsamples < 0)
        {
            libmseed_memory.free (encoded);
            libmseed_memory.free (rawrec);
            throw RuntimeException ("Error packing data samples");
        }

        packoffset += packsamples * samplesize;

        /* Copy encoded data into record */
        memcpy (rawrec + dataoffset, encoded, datalength);

        /* Update number of samples */
        *pMS2FSDH_NUMSAMPLES(rawrec) = HO2u (packsamples, swapflag);

        if (verbose >= 1)
            ms_log (0, "%s: Packed %d samples into %d byte record\n", msr->sid, packsamples, msr->reclen);

        /* Send record to handler */
        //record_handler (rawrec, msr->reclen, handlerdata);
        ret.append(rawrec, msr->reclen);

        totalpackedsamples += packsamples;

        recordcnt++;

        if (totalpackedsamples >= msr->numsamples)
            break;

        /* Update record start time for next record */
        nextstarttime = ms_sampletime (msr->starttime, totalpackedsamples, msr->samprate);

        if (ms_nstime2time (nextstarttime, &year, &day, &hour, &min, &sec, &nsec))
        {
            libmseed_memory.free (rawrec);
            throw RuntimeException ("Cannot convert next record starttime");
        }

        *pMS2FSDH_YEAR (rawrec) = HO2u (year, swapflag);
        *pMS2FSDH_DAY (rawrec)  = HO2u (day, swapflag);
        *pMS2FSDH_HOUR (rawrec) = hour;
        *pMS2FSDH_MIN (rawrec)  = min;
        *pMS2FSDH_SEC (rawrec)  = sec;
        *pMS2FSDH_FSEC (rawrec) = HO2u ((nsec / 100000), swapflag);
    }

    if (encoded)
        libmseed_memory.free (encoded);

    libmseed_memory.free (rawrec);

    return ret;
} /* End of msr3_pack_mseed2() */