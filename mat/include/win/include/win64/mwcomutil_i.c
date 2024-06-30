

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 8.00.0603 */
/* at Mon Dec 28 15:44:01 2015
 */
/* Compiler settings for win64\mwcomutil.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=IA64 8.00.0603 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IMWUtil,0xC47EA90E,0x56D1,0x11d5,0xB1,0x59,0x00,0xD0,0xB7,0xBA,0x75,0x44);


MIDL_DEFINE_GUID(IID, LIBID_MWComUtil,0xF28D13BB,0x9031,0x4236,0x83,0x5E,0x05,0xC1,0x6C,0xD9,0x74,0x7B);


MIDL_DEFINE_GUID(CLSID, CLSID_MWField,0xF640492B,0x481F,0x4803,0x92,0xDE,0x61,0x8C,0xAB,0xEB,0xE0,0xFE);


MIDL_DEFINE_GUID(CLSID, CLSID_MWStruct,0x3C7B8376,0xA81E,0x4829,0xAF,0xBA,0xBB,0xFA,0xBB,0x12,0xA8,0x5A);


MIDL_DEFINE_GUID(CLSID, CLSID_MWComplex,0xCDE58C15,0x0FD0,0x4B92,0x87,0x4B,0x2E,0x48,0xBF,0xEF,0x5E,0xD7);


MIDL_DEFINE_GUID(CLSID, CLSID_MWSparse,0xFFCBF72C,0x7146,0x4274,0xA8,0x26,0xC9,0x04,0x92,0x9E,0x1F,0x85);


MIDL_DEFINE_GUID(CLSID, CLSID_MWArg,0xF8C3A0E9,0x13C2,0x42D7,0xA7,0x5C,0x79,0xCF,0xE3,0x2C,0x4C,0xD8);


MIDL_DEFINE_GUID(CLSID, CLSID_MWArrayFormatFlags,0x77911634,0xED5E,0x48AC,0xBE,0x8E,0x23,0xDF,0xF9,0x92,0x3D,0xBE);


MIDL_DEFINE_GUID(CLSID, CLSID_MWDataConversionFlags,0xC7906A0F,0xE58A,0x4B8D,0xAA,0xC1,0xDE,0x63,0xD1,0x65,0x77,0x04);


MIDL_DEFINE_GUID(CLSID, CLSID_MWUtil,0xC5FE821D,0x95F0,0x4B39,0x87,0x71,0x94,0x2D,0xBE,0xA2,0x02,0x10);


MIDL_DEFINE_GUID(CLSID, CLSID_MWFlags,0xF301BC9D,0xE766,0x47FD,0x9B,0x44,0x99,0xDB,0xDC,0x01,0x28,0xA5);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



