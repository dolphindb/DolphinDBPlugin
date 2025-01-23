

/* this ALWAYS GENERATED file contains the proxy stub code */


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

#if defined(_M_IA64) 


#pragma warning( disable: 4049 )  /* more than 64k source lines */
#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning( disable: 4211 )  /* redefine extern to static */
#pragma warning( disable: 4232 )  /* dllimport identity*/
#pragma warning( disable: 4024 )  /* array to pointer mapping*/
#pragma warning( disable: 4152 )  /* function/data pointer conversion in expression */

#define USE_STUBLESS_PROXY


/* verify that the <rpcproxy.h> version is high enough to compile this file*/
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 475
#endif


#include "rpcproxy.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif /* __RPCPROXY_H_VERSION__ */


#include "mwcomutil.h"

#define TYPE_FORMAT_STRING_SIZE   1029                              
#define PROC_FORMAT_STRING_SIZE   713                               
#define EXPR_FORMAT_STRING_SIZE   1                                 
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   2            

typedef struct _mwcomutil_MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } mwcomutil_MIDL_TYPE_FORMAT_STRING;

typedef struct _mwcomutil_MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } mwcomutil_MIDL_PROC_FORMAT_STRING;

typedef struct _mwcomutil_MIDL_EXPR_FORMAT_STRING
    {
    long          Pad;
    unsigned char  Format[ EXPR_FORMAT_STRING_SIZE ];
    } mwcomutil_MIDL_EXPR_FORMAT_STRING;


static const RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax = 
{{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}};


extern const mwcomutil_MIDL_TYPE_FORMAT_STRING mwcomutil__MIDL_TypeFormatString;
extern const mwcomutil_MIDL_PROC_FORMAT_STRING mwcomutil__MIDL_ProcFormatString;
extern const mwcomutil_MIDL_EXPR_FORMAT_STRING mwcomutil__MIDL_ExprFormatString;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IMWUtil_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IMWUtil_ProxyInfo;


extern const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ];

#if !defined(__RPC_WIN64__)
#error  Invalid build platform for this stub.
#endif

static const mwcomutil_MIDL_PROC_FORMAT_STRING mwcomutil__MIDL_ProcFormatString =
    {
        0,
        {

	/* Procedure MWPack */

			0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/*  2 */	NdrFcLong( 0x0 ),	/* 0 */
/*  6 */	NdrFcShort( 0x7 ),	/* 7 */
/*  8 */	NdrFcShort( 0x318 ),	/* ia64 Stack size/offset = 792 */
/* 10 */	NdrFcShort( 0x0 ),	/* 0 */
/* 12 */	NdrFcShort( 0x8 ),	/* 8 */
/* 14 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0x22,		/* 34 */
/* 16 */	0xa,		/* 10 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 18 */	NdrFcShort( 0x1 ),	/* 1 */
/* 20 */	NdrFcShort( 0x1 ),	/* 1 */
/* 22 */	NdrFcShort( 0x0 ),	/* 0 */
/* 24 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter pVarArg */

/* 26 */	NdrFcShort( 0x6113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=24 */
/* 28 */	NdrFcShort( 0x8 ),	/* ia64 Stack size/offset = 8 */
/* 30 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter Var0 */

/* 32 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 34 */	NdrFcShort( 0x10 ),	/* ia64 Stack size/offset = 16 */
/* 36 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var1 */

/* 38 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 40 */	NdrFcShort( 0x28 ),	/* ia64 Stack size/offset = 40 */
/* 42 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var2 */

/* 44 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 46 */	NdrFcShort( 0x40 ),	/* ia64 Stack size/offset = 64 */
/* 48 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var3 */

/* 50 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 52 */	NdrFcShort( 0x58 ),	/* ia64 Stack size/offset = 88 */
/* 54 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var4 */

/* 56 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 58 */	NdrFcShort( 0x70 ),	/* ia64 Stack size/offset = 112 */
/* 60 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var5 */

/* 62 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 64 */	NdrFcShort( 0x88 ),	/* ia64 Stack size/offset = 136 */
/* 66 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var6 */

/* 68 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 70 */	NdrFcShort( 0xa0 ),	/* ia64 Stack size/offset = 160 */
/* 72 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var7 */

/* 74 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 76 */	NdrFcShort( 0xb8 ),	/* ia64 Stack size/offset = 184 */
/* 78 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var8 */

/* 80 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 82 */	NdrFcShort( 0xd0 ),	/* ia64 Stack size/offset = 208 */
/* 84 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var9 */

/* 86 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 88 */	NdrFcShort( 0xe8 ),	/* ia64 Stack size/offset = 232 */
/* 90 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var10 */

/* 92 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 94 */	NdrFcShort( 0x100 ),	/* ia64 Stack size/offset = 256 */
/* 96 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var11 */

/* 98 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 100 */	NdrFcShort( 0x118 ),	/* ia64 Stack size/offset = 280 */
/* 102 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var12 */

/* 104 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 106 */	NdrFcShort( 0x130 ),	/* ia64 Stack size/offset = 304 */
/* 108 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var13 */

/* 110 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 112 */	NdrFcShort( 0x148 ),	/* ia64 Stack size/offset = 328 */
/* 114 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var14 */

/* 116 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 118 */	NdrFcShort( 0x160 ),	/* ia64 Stack size/offset = 352 */
/* 120 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var15 */

/* 122 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 124 */	NdrFcShort( 0x178 ),	/* ia64 Stack size/offset = 376 */
/* 126 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var16 */

/* 128 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 130 */	NdrFcShort( 0x190 ),	/* ia64 Stack size/offset = 400 */
/* 132 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var17 */

/* 134 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 136 */	NdrFcShort( 0x1a8 ),	/* ia64 Stack size/offset = 424 */
/* 138 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var18 */

/* 140 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 142 */	NdrFcShort( 0x1c0 ),	/* ia64 Stack size/offset = 448 */
/* 144 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var19 */

/* 146 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 148 */	NdrFcShort( 0x1d8 ),	/* ia64 Stack size/offset = 472 */
/* 150 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var20 */

/* 152 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 154 */	NdrFcShort( 0x1f0 ),	/* ia64 Stack size/offset = 496 */
/* 156 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var21 */

/* 158 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 160 */	NdrFcShort( 0x208 ),	/* ia64 Stack size/offset = 520 */
/* 162 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var22 */

/* 164 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 166 */	NdrFcShort( 0x220 ),	/* ia64 Stack size/offset = 544 */
/* 168 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var23 */

/* 170 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 172 */	NdrFcShort( 0x238 ),	/* ia64 Stack size/offset = 568 */
/* 174 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var24 */

/* 176 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 178 */	NdrFcShort( 0x250 ),	/* ia64 Stack size/offset = 592 */
/* 180 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var25 */

/* 182 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 184 */	NdrFcShort( 0x268 ),	/* ia64 Stack size/offset = 616 */
/* 186 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var26 */

/* 188 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 190 */	NdrFcShort( 0x280 ),	/* ia64 Stack size/offset = 640 */
/* 192 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var27 */

/* 194 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 196 */	NdrFcShort( 0x298 ),	/* ia64 Stack size/offset = 664 */
/* 198 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var28 */

/* 200 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 202 */	NdrFcShort( 0x2b0 ),	/* ia64 Stack size/offset = 688 */
/* 204 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var29 */

/* 206 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 208 */	NdrFcShort( 0x2c8 ),	/* ia64 Stack size/offset = 712 */
/* 210 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var30 */

/* 212 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 214 */	NdrFcShort( 0x2e0 ),	/* ia64 Stack size/offset = 736 */
/* 216 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter Var31 */

/* 218 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 220 */	NdrFcShort( 0x2f8 ),	/* ia64 Stack size/offset = 760 */
/* 222 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Return value */

/* 224 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 226 */	NdrFcShort( 0x310 ),	/* ia64 Stack size/offset = 784 */
/* 228 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure MWUnpack */

/* 230 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 232 */	NdrFcLong( 0x0 ),	/* 0 */
/* 236 */	NdrFcShort( 0x8 ),	/* 8 */
/* 238 */	NdrFcShort( 0x138 ),	/* ia64 Stack size/offset = 312 */
/* 240 */	NdrFcShort( 0xe ),	/* 14 */
/* 242 */	NdrFcShort( 0x8 ),	/* 8 */
/* 244 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0x24,		/* 36 */
/* 246 */	0xa,		/* 10 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 248 */	NdrFcShort( 0x1 ),	/* 1 */
/* 250 */	NdrFcShort( 0x1 ),	/* 1 */
/* 252 */	NdrFcShort( 0x0 ),	/* 0 */
/* 254 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter VarArg */

/* 256 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 258 */	NdrFcShort( 0x8 ),	/* ia64 Stack size/offset = 8 */
/* 260 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Parameter nStartAt */

/* 262 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 264 */	NdrFcShort( 0x20 ),	/* ia64 Stack size/offset = 32 */
/* 266 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter bAutoResize */

/* 268 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 270 */	NdrFcShort( 0x28 ),	/* ia64 Stack size/offset = 40 */
/* 272 */	0x6,		/* FC_SHORT */
			0x0,		/* 0 */

	/* Parameter pVar0 */

/* 274 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 276 */	NdrFcShort( 0x30 ),	/* ia64 Stack size/offset = 48 */
/* 278 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar1 */

/* 280 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 282 */	NdrFcShort( 0x38 ),	/* ia64 Stack size/offset = 56 */
/* 284 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar2 */

/* 286 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 288 */	NdrFcShort( 0x40 ),	/* ia64 Stack size/offset = 64 */
/* 290 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar3 */

/* 292 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 294 */	NdrFcShort( 0x48 ),	/* ia64 Stack size/offset = 72 */
/* 296 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar4 */

/* 298 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 300 */	NdrFcShort( 0x50 ),	/* ia64 Stack size/offset = 80 */
/* 302 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar5 */

/* 304 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 306 */	NdrFcShort( 0x58 ),	/* ia64 Stack size/offset = 88 */
/* 308 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar6 */

/* 310 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 312 */	NdrFcShort( 0x60 ),	/* ia64 Stack size/offset = 96 */
/* 314 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar7 */

/* 316 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 318 */	NdrFcShort( 0x68 ),	/* ia64 Stack size/offset = 104 */
/* 320 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar8 */

/* 322 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 324 */	NdrFcShort( 0x70 ),	/* ia64 Stack size/offset = 112 */
/* 326 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar9 */

/* 328 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 330 */	NdrFcShort( 0x78 ),	/* ia64 Stack size/offset = 120 */
/* 332 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar10 */

/* 334 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 336 */	NdrFcShort( 0x80 ),	/* ia64 Stack size/offset = 128 */
/* 338 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar11 */

/* 340 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 342 */	NdrFcShort( 0x88 ),	/* ia64 Stack size/offset = 136 */
/* 344 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar12 */

/* 346 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 348 */	NdrFcShort( 0x90 ),	/* ia64 Stack size/offset = 144 */
/* 350 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar13 */

/* 352 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 354 */	NdrFcShort( 0x98 ),	/* ia64 Stack size/offset = 152 */
/* 356 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar14 */

/* 358 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 360 */	NdrFcShort( 0xa0 ),	/* ia64 Stack size/offset = 160 */
/* 362 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar15 */

/* 364 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 366 */	NdrFcShort( 0xa8 ),	/* ia64 Stack size/offset = 168 */
/* 368 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar16 */

/* 370 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 372 */	NdrFcShort( 0xb0 ),	/* ia64 Stack size/offset = 176 */
/* 374 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar17 */

/* 376 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 378 */	NdrFcShort( 0xb8 ),	/* ia64 Stack size/offset = 184 */
/* 380 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar18 */

/* 382 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 384 */	NdrFcShort( 0xc0 ),	/* ia64 Stack size/offset = 192 */
/* 386 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar19 */

/* 388 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 390 */	NdrFcShort( 0xc8 ),	/* ia64 Stack size/offset = 200 */
/* 392 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar20 */

/* 394 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 396 */	NdrFcShort( 0xd0 ),	/* ia64 Stack size/offset = 208 */
/* 398 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar21 */

/* 400 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 402 */	NdrFcShort( 0xd8 ),	/* ia64 Stack size/offset = 216 */
/* 404 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar22 */

/* 406 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 408 */	NdrFcShort( 0xe0 ),	/* ia64 Stack size/offset = 224 */
/* 410 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar23 */

/* 412 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 414 */	NdrFcShort( 0xe8 ),	/* ia64 Stack size/offset = 232 */
/* 416 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar24 */

/* 418 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 420 */	NdrFcShort( 0xf0 ),	/* ia64 Stack size/offset = 240 */
/* 422 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar25 */

/* 424 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 426 */	NdrFcShort( 0xf8 ),	/* ia64 Stack size/offset = 248 */
/* 428 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar26 */

/* 430 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 432 */	NdrFcShort( 0x100 ),	/* ia64 Stack size/offset = 256 */
/* 434 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar27 */

/* 436 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 438 */	NdrFcShort( 0x108 ),	/* ia64 Stack size/offset = 264 */
/* 440 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar28 */

/* 442 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 444 */	NdrFcShort( 0x110 ),	/* ia64 Stack size/offset = 272 */
/* 446 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar29 */

/* 448 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 450 */	NdrFcShort( 0x118 ),	/* ia64 Stack size/offset = 280 */
/* 452 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar30 */

/* 454 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 456 */	NdrFcShort( 0x120 ),	/* ia64 Stack size/offset = 288 */
/* 458 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Parameter pVar31 */

/* 460 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 462 */	NdrFcShort( 0x128 ),	/* ia64 Stack size/offset = 296 */
/* 464 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Return value */

/* 466 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 468 */	NdrFcShort( 0x130 ),	/* ia64 Stack size/offset = 304 */
/* 470 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure MWDate2VariantDate */

/* 472 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 474 */	NdrFcLong( 0x0 ),	/* 0 */
/* 478 */	NdrFcShort( 0x9 ),	/* 9 */
/* 480 */	NdrFcShort( 0x18 ),	/* ia64 Stack size/offset = 24 */
/* 482 */	NdrFcShort( 0x0 ),	/* 0 */
/* 484 */	NdrFcShort( 0x8 ),	/* 8 */
/* 486 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0x2,		/* 2 */
/* 488 */	0xa,		/* 10 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 490 */	NdrFcShort( 0x1 ),	/* 1 */
/* 492 */	NdrFcShort( 0x1 ),	/* 1 */
/* 494 */	NdrFcShort( 0x0 ),	/* 0 */
/* 496 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter pVar */

/* 498 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 500 */	NdrFcShort( 0x8 ),	/* ia64 Stack size/offset = 8 */
/* 502 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */

	/* Return value */

/* 504 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 506 */	NdrFcShort( 0x10 ),	/* ia64 Stack size/offset = 16 */
/* 508 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure MWInitApplication */

/* 510 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 512 */	NdrFcLong( 0x0 ),	/* 0 */
/* 516 */	NdrFcShort( 0xa ),	/* 10 */
/* 518 */	NdrFcShort( 0x18 ),	/* ia64 Stack size/offset = 24 */
/* 520 */	NdrFcShort( 0x0 ),	/* 0 */
/* 522 */	NdrFcShort( 0x8 ),	/* 8 */
/* 524 */	0x46,		/* Oi2 Flags:  clt must size, has return, has ext, */
			0x2,		/* 2 */
/* 526 */	0xa,		/* 10 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 528 */	NdrFcShort( 0x0 ),	/* 0 */
/* 530 */	NdrFcShort( 0x0 ),	/* 0 */
/* 532 */	NdrFcShort( 0x0 ),	/* 0 */
/* 534 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter pApp */

/* 536 */	NdrFcShort( 0xb ),	/* Flags:  must size, must free, in, */
/* 538 */	NdrFcShort( 0x8 ),	/* ia64 Stack size/offset = 8 */
/* 540 */	NdrFcShort( 0x166 ),	/* Type Offset=358 */

	/* Return value */

/* 542 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 544 */	NdrFcShort( 0x10 ),	/* ia64 Stack size/offset = 16 */
/* 546 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure MWInitApplicationWithMCROptions */

/* 548 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 550 */	NdrFcLong( 0x0 ),	/* 0 */
/* 554 */	NdrFcShort( 0xb ),	/* 11 */
/* 556 */	NdrFcShort( 0x30 ),	/* ia64 Stack size/offset = 48 */
/* 558 */	NdrFcShort( 0x0 ),	/* 0 */
/* 560 */	NdrFcShort( 0x8 ),	/* 8 */
/* 562 */	0x46,		/* Oi2 Flags:  clt must size, has return, has ext, */
			0x3,		/* 3 */
/* 564 */	0xa,		/* 10 */
			0x5,		/* Ext Flags:  new corr desc, srv corr check, */
/* 566 */	NdrFcShort( 0x0 ),	/* 0 */
/* 568 */	NdrFcShort( 0x1 ),	/* 1 */
/* 570 */	NdrFcShort( 0x0 ),	/* 0 */
/* 572 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter pApp */

/* 574 */	NdrFcShort( 0xb ),	/* Flags:  must size, must free, in, */
/* 576 */	NdrFcShort( 0x8 ),	/* ia64 Stack size/offset = 8 */
/* 578 */	NdrFcShort( 0x166 ),	/* Type Offset=358 */

	/* Parameter mcrOptionList */

/* 580 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 582 */	NdrFcShort( 0x10 ),	/* ia64 Stack size/offset = 16 */
/* 584 */	NdrFcShort( 0x3e0 ),	/* Type Offset=992 */

	/* Return value */

/* 586 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 588 */	NdrFcShort( 0x28 ),	/* ia64 Stack size/offset = 40 */
/* 590 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure CreateRemoteObject */

/* 592 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 594 */	NdrFcLong( 0x0 ),	/* 0 */
/* 598 */	NdrFcShort( 0xc ),	/* 12 */
/* 600 */	NdrFcShort( 0x20 ),	/* ia64 Stack size/offset = 32 */
/* 602 */	NdrFcShort( 0x0 ),	/* 0 */
/* 604 */	NdrFcShort( 0x8 ),	/* 8 */
/* 606 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0x3,		/* 3 */
/* 608 */	0xa,		/* 10 */
			0x5,		/* Ext Flags:  new corr desc, srv corr check, */
/* 610 */	NdrFcShort( 0x0 ),	/* 0 */
/* 612 */	NdrFcShort( 0x1 ),	/* 1 */
/* 614 */	NdrFcShort( 0x0 ),	/* 0 */
/* 616 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter progId */

/* 618 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 620 */	NdrFcShort( 0x8 ),	/* ia64 Stack size/offset = 8 */
/* 622 */	NdrFcShort( 0x3f2 ),	/* Type Offset=1010 */

	/* Parameter pObject */

/* 624 */	NdrFcShort( 0x13 ),	/* Flags:  must size, must free, out, */
/* 626 */	NdrFcShort( 0x10 ),	/* ia64 Stack size/offset = 16 */
/* 628 */	NdrFcShort( 0x3fc ),	/* Type Offset=1020 */

	/* Return value */

/* 630 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 632 */	NdrFcShort( 0x18 ),	/* ia64 Stack size/offset = 24 */
/* 634 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure IsMCRJVMEnabled */

/* 636 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 638 */	NdrFcLong( 0x0 ),	/* 0 */
/* 642 */	NdrFcShort( 0xd ),	/* 13 */
/* 644 */	NdrFcShort( 0x18 ),	/* ia64 Stack size/offset = 24 */
/* 646 */	NdrFcShort( 0x0 ),	/* 0 */
/* 648 */	NdrFcShort( 0x22 ),	/* 34 */
/* 650 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x2,		/* 2 */
/* 652 */	0xa,		/* 10 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 654 */	NdrFcShort( 0x0 ),	/* 0 */
/* 656 */	NdrFcShort( 0x0 ),	/* 0 */
/* 658 */	NdrFcShort( 0x0 ),	/* 0 */
/* 660 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter enabled */

/* 662 */	NdrFcShort( 0x2150 ),	/* Flags:  out, base type, simple ref, srv alloc size=8 */
/* 664 */	NdrFcShort( 0x8 ),	/* ia64 Stack size/offset = 8 */
/* 666 */	0x6,		/* FC_SHORT */
			0x0,		/* 0 */

	/* Return value */

/* 668 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 670 */	NdrFcShort( 0x10 ),	/* ia64 Stack size/offset = 16 */
/* 672 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure IsMCRInitialized */

/* 674 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 676 */	NdrFcLong( 0x0 ),	/* 0 */
/* 680 */	NdrFcShort( 0xe ),	/* 14 */
/* 682 */	NdrFcShort( 0x18 ),	/* ia64 Stack size/offset = 24 */
/* 684 */	NdrFcShort( 0x0 ),	/* 0 */
/* 686 */	NdrFcShort( 0x22 ),	/* 34 */
/* 688 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x2,		/* 2 */
/* 690 */	0xa,		/* 10 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 692 */	NdrFcShort( 0x0 ),	/* 0 */
/* 694 */	NdrFcShort( 0x0 ),	/* 0 */
/* 696 */	NdrFcShort( 0x0 ),	/* 0 */
/* 698 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter initialized */

/* 700 */	NdrFcShort( 0x2150 ),	/* Flags:  out, base type, simple ref, srv alloc size=8 */
/* 702 */	NdrFcShort( 0x8 ),	/* ia64 Stack size/offset = 8 */
/* 704 */	0x6,		/* FC_SHORT */
			0x0,		/* 0 */

	/* Return value */

/* 706 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 708 */	NdrFcShort( 0x10 ),	/* ia64 Stack size/offset = 16 */
/* 710 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

			0x0
        }
    };

static const mwcomutil_MIDL_TYPE_FORMAT_STRING mwcomutil__MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	/* 0 */
/*  2 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/*  4 */	NdrFcShort( 0x3ce ),	/* Offset= 974 (978) */
/*  6 */	
			0x13, 0x0,	/* FC_OP */
/*  8 */	NdrFcShort( 0x3b6 ),	/* Offset= 950 (958) */
/* 10 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 12 */	0x7,		/* Corr desc: FC_USHORT */
			0x0,		/*  */
/* 14 */	NdrFcShort( 0xfff8 ),	/* -8 */
/* 16 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 18 */	NdrFcShort( 0x2 ),	/* Offset= 2 (20) */
/* 20 */	NdrFcShort( 0x10 ),	/* 16 */
/* 22 */	NdrFcShort( 0x2f ),	/* 47 */
/* 24 */	NdrFcLong( 0x14 ),	/* 20 */
/* 28 */	NdrFcShort( 0x800b ),	/* Simple arm type: FC_HYPER */
/* 30 */	NdrFcLong( 0x3 ),	/* 3 */
/* 34 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 36 */	NdrFcLong( 0x11 ),	/* 17 */
/* 40 */	NdrFcShort( 0x8001 ),	/* Simple arm type: FC_BYTE */
/* 42 */	NdrFcLong( 0x2 ),	/* 2 */
/* 46 */	NdrFcShort( 0x8006 ),	/* Simple arm type: FC_SHORT */
/* 48 */	NdrFcLong( 0x4 ),	/* 4 */
/* 52 */	NdrFcShort( 0x800a ),	/* Simple arm type: FC_FLOAT */
/* 54 */	NdrFcLong( 0x5 ),	/* 5 */
/* 58 */	NdrFcShort( 0x800c ),	/* Simple arm type: FC_DOUBLE */
/* 60 */	NdrFcLong( 0xb ),	/* 11 */
/* 64 */	NdrFcShort( 0x8006 ),	/* Simple arm type: FC_SHORT */
/* 66 */	NdrFcLong( 0xa ),	/* 10 */
/* 70 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 72 */	NdrFcLong( 0x6 ),	/* 6 */
/* 76 */	NdrFcShort( 0xe8 ),	/* Offset= 232 (308) */
/* 78 */	NdrFcLong( 0x7 ),	/* 7 */
/* 82 */	NdrFcShort( 0x800c ),	/* Simple arm type: FC_DOUBLE */
/* 84 */	NdrFcLong( 0x8 ),	/* 8 */
/* 88 */	NdrFcShort( 0xe2 ),	/* Offset= 226 (314) */
/* 90 */	NdrFcLong( 0xd ),	/* 13 */
/* 94 */	NdrFcShort( 0xf6 ),	/* Offset= 246 (340) */
/* 96 */	NdrFcLong( 0x9 ),	/* 9 */
/* 100 */	NdrFcShort( 0x102 ),	/* Offset= 258 (358) */
/* 102 */	NdrFcLong( 0x2000 ),	/* 8192 */
/* 106 */	NdrFcShort( 0x10e ),	/* Offset= 270 (376) */
/* 108 */	NdrFcLong( 0x24 ),	/* 36 */
/* 112 */	NdrFcShort( 0x304 ),	/* Offset= 772 (884) */
/* 114 */	NdrFcLong( 0x4024 ),	/* 16420 */
/* 118 */	NdrFcShort( 0x2fe ),	/* Offset= 766 (884) */
/* 120 */	NdrFcLong( 0x4011 ),	/* 16401 */
/* 124 */	NdrFcShort( 0x2fc ),	/* Offset= 764 (888) */
/* 126 */	NdrFcLong( 0x4002 ),	/* 16386 */
/* 130 */	NdrFcShort( 0x2fa ),	/* Offset= 762 (892) */
/* 132 */	NdrFcLong( 0x4003 ),	/* 16387 */
/* 136 */	NdrFcShort( 0x2f8 ),	/* Offset= 760 (896) */
/* 138 */	NdrFcLong( 0x4014 ),	/* 16404 */
/* 142 */	NdrFcShort( 0x2f6 ),	/* Offset= 758 (900) */
/* 144 */	NdrFcLong( 0x4004 ),	/* 16388 */
/* 148 */	NdrFcShort( 0x2f4 ),	/* Offset= 756 (904) */
/* 150 */	NdrFcLong( 0x4005 ),	/* 16389 */
/* 154 */	NdrFcShort( 0x2f2 ),	/* Offset= 754 (908) */
/* 156 */	NdrFcLong( 0x400b ),	/* 16395 */
/* 160 */	NdrFcShort( 0x2dc ),	/* Offset= 732 (892) */
/* 162 */	NdrFcLong( 0x400a ),	/* 16394 */
/* 166 */	NdrFcShort( 0x2da ),	/* Offset= 730 (896) */
/* 168 */	NdrFcLong( 0x4006 ),	/* 16390 */
/* 172 */	NdrFcShort( 0x2e4 ),	/* Offset= 740 (912) */
/* 174 */	NdrFcLong( 0x4007 ),	/* 16391 */
/* 178 */	NdrFcShort( 0x2da ),	/* Offset= 730 (908) */
/* 180 */	NdrFcLong( 0x4008 ),	/* 16392 */
/* 184 */	NdrFcShort( 0x2dc ),	/* Offset= 732 (916) */
/* 186 */	NdrFcLong( 0x400d ),	/* 16397 */
/* 190 */	NdrFcShort( 0x2da ),	/* Offset= 730 (920) */
/* 192 */	NdrFcLong( 0x4009 ),	/* 16393 */
/* 196 */	NdrFcShort( 0x2d8 ),	/* Offset= 728 (924) */
/* 198 */	NdrFcLong( 0x6000 ),	/* 24576 */
/* 202 */	NdrFcShort( 0x2d6 ),	/* Offset= 726 (928) */
/* 204 */	NdrFcLong( 0x400c ),	/* 16396 */
/* 208 */	NdrFcShort( 0x2d4 ),	/* Offset= 724 (932) */
/* 210 */	NdrFcLong( 0x10 ),	/* 16 */
/* 214 */	NdrFcShort( 0x8002 ),	/* Simple arm type: FC_CHAR */
/* 216 */	NdrFcLong( 0x12 ),	/* 18 */
/* 220 */	NdrFcShort( 0x8006 ),	/* Simple arm type: FC_SHORT */
/* 222 */	NdrFcLong( 0x13 ),	/* 19 */
/* 226 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 228 */	NdrFcLong( 0x15 ),	/* 21 */
/* 232 */	NdrFcShort( 0x800b ),	/* Simple arm type: FC_HYPER */
/* 234 */	NdrFcLong( 0x16 ),	/* 22 */
/* 238 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 240 */	NdrFcLong( 0x17 ),	/* 23 */
/* 244 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 246 */	NdrFcLong( 0xe ),	/* 14 */
/* 250 */	NdrFcShort( 0x2b2 ),	/* Offset= 690 (940) */
/* 252 */	NdrFcLong( 0x400e ),	/* 16398 */
/* 256 */	NdrFcShort( 0x2b6 ),	/* Offset= 694 (950) */
/* 258 */	NdrFcLong( 0x4010 ),	/* 16400 */
/* 262 */	NdrFcShort( 0x2b4 ),	/* Offset= 692 (954) */
/* 264 */	NdrFcLong( 0x4012 ),	/* 16402 */
/* 268 */	NdrFcShort( 0x270 ),	/* Offset= 624 (892) */
/* 270 */	NdrFcLong( 0x4013 ),	/* 16403 */
/* 274 */	NdrFcShort( 0x26e ),	/* Offset= 622 (896) */
/* 276 */	NdrFcLong( 0x4015 ),	/* 16405 */
/* 280 */	NdrFcShort( 0x26c ),	/* Offset= 620 (900) */
/* 282 */	NdrFcLong( 0x4016 ),	/* 16406 */
/* 286 */	NdrFcShort( 0x262 ),	/* Offset= 610 (896) */
/* 288 */	NdrFcLong( 0x4017 ),	/* 16407 */
/* 292 */	NdrFcShort( 0x25c ),	/* Offset= 604 (896) */
/* 294 */	NdrFcLong( 0x0 ),	/* 0 */
/* 298 */	NdrFcShort( 0x0 ),	/* Offset= 0 (298) */
/* 300 */	NdrFcLong( 0x1 ),	/* 1 */
/* 304 */	NdrFcShort( 0x0 ),	/* Offset= 0 (304) */
/* 306 */	NdrFcShort( 0xffff ),	/* Offset= -1 (305) */
/* 308 */	
			0x15,		/* FC_STRUCT */
			0x7,		/* 7 */
/* 310 */	NdrFcShort( 0x8 ),	/* 8 */
/* 312 */	0xb,		/* FC_HYPER */
			0x5b,		/* FC_END */
/* 314 */	
			0x13, 0x0,	/* FC_OP */
/* 316 */	NdrFcShort( 0xe ),	/* Offset= 14 (330) */
/* 318 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 320 */	NdrFcShort( 0x2 ),	/* 2 */
/* 322 */	0x9,		/* Corr desc: FC_ULONG */
			0x0,		/*  */
/* 324 */	NdrFcShort( 0xfffc ),	/* -4 */
/* 326 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 328 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 330 */	
			0x17,		/* FC_CSTRUCT */
			0x3,		/* 3 */
/* 332 */	NdrFcShort( 0x8 ),	/* 8 */
/* 334 */	NdrFcShort( 0xfff0 ),	/* Offset= -16 (318) */
/* 336 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 338 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 340 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 342 */	NdrFcLong( 0x0 ),	/* 0 */
/* 346 */	NdrFcShort( 0x0 ),	/* 0 */
/* 348 */	NdrFcShort( 0x0 ),	/* 0 */
/* 350 */	0xc0,		/* 192 */
			0x0,		/* 0 */
/* 352 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 354 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 356 */	0x0,		/* 0 */
			0x46,		/* 70 */
/* 358 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 360 */	NdrFcLong( 0x20400 ),	/* 132096 */
/* 364 */	NdrFcShort( 0x0 ),	/* 0 */
/* 366 */	NdrFcShort( 0x0 ),	/* 0 */
/* 368 */	0xc0,		/* 192 */
			0x0,		/* 0 */
/* 370 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 372 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 374 */	0x0,		/* 0 */
			0x46,		/* 70 */
/* 376 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 378 */	NdrFcShort( 0x2 ),	/* Offset= 2 (380) */
/* 380 */	
			0x13, 0x0,	/* FC_OP */
/* 382 */	NdrFcShort( 0x1e4 ),	/* Offset= 484 (866) */
/* 384 */	
			0x2a,		/* FC_ENCAPSULATED_UNION */
			0x89,		/* 137 */
/* 386 */	NdrFcShort( 0x20 ),	/* 32 */
/* 388 */	NdrFcShort( 0xa ),	/* 10 */
/* 390 */	NdrFcLong( 0x8 ),	/* 8 */
/* 394 */	NdrFcShort( 0x50 ),	/* Offset= 80 (474) */
/* 396 */	NdrFcLong( 0xd ),	/* 13 */
/* 400 */	NdrFcShort( 0x70 ),	/* Offset= 112 (512) */
/* 402 */	NdrFcLong( 0x9 ),	/* 9 */
/* 406 */	NdrFcShort( 0x90 ),	/* Offset= 144 (550) */
/* 408 */	NdrFcLong( 0xc ),	/* 12 */
/* 412 */	NdrFcShort( 0xb0 ),	/* Offset= 176 (588) */
/* 414 */	NdrFcLong( 0x24 ),	/* 36 */
/* 418 */	NdrFcShort( 0x102 ),	/* Offset= 258 (676) */
/* 420 */	NdrFcLong( 0x800d ),	/* 32781 */
/* 424 */	NdrFcShort( 0x11e ),	/* Offset= 286 (710) */
/* 426 */	NdrFcLong( 0x10 ),	/* 16 */
/* 430 */	NdrFcShort( 0x138 ),	/* Offset= 312 (742) */
/* 432 */	NdrFcLong( 0x2 ),	/* 2 */
/* 436 */	NdrFcShort( 0x14e ),	/* Offset= 334 (770) */
/* 438 */	NdrFcLong( 0x3 ),	/* 3 */
/* 442 */	NdrFcShort( 0x164 ),	/* Offset= 356 (798) */
/* 444 */	NdrFcLong( 0x14 ),	/* 20 */
/* 448 */	NdrFcShort( 0x17a ),	/* Offset= 378 (826) */
/* 450 */	NdrFcShort( 0xffff ),	/* Offset= -1 (449) */
/* 452 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 454 */	NdrFcShort( 0x0 ),	/* 0 */
/* 456 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 458 */	NdrFcShort( 0x0 ),	/* 0 */
/* 460 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 462 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 466 */	NdrFcShort( 0x0 ),	/* Corr flags:  */
/* 468 */	
			0x13, 0x0,	/* FC_OP */
/* 470 */	NdrFcShort( 0xff74 ),	/* Offset= -140 (330) */
/* 472 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 474 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 476 */	NdrFcShort( 0x10 ),	/* 16 */
/* 478 */	NdrFcShort( 0x0 ),	/* 0 */
/* 480 */	NdrFcShort( 0x6 ),	/* Offset= 6 (486) */
/* 482 */	0x8,		/* FC_LONG */
			0x40,		/* FC_STRUCTPAD4 */
/* 484 */	0x36,		/* FC_POINTER */
			0x5b,		/* FC_END */
/* 486 */	
			0x11, 0x0,	/* FC_RP */
/* 488 */	NdrFcShort( 0xffdc ),	/* Offset= -36 (452) */
/* 490 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 492 */	NdrFcShort( 0x0 ),	/* 0 */
/* 494 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 496 */	NdrFcShort( 0x0 ),	/* 0 */
/* 498 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 500 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 504 */	NdrFcShort( 0x0 ),	/* Corr flags:  */
/* 506 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 508 */	NdrFcShort( 0xff58 ),	/* Offset= -168 (340) */
/* 510 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 512 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 514 */	NdrFcShort( 0x10 ),	/* 16 */
/* 516 */	NdrFcShort( 0x0 ),	/* 0 */
/* 518 */	NdrFcShort( 0x6 ),	/* Offset= 6 (524) */
/* 520 */	0x8,		/* FC_LONG */
			0x40,		/* FC_STRUCTPAD4 */
/* 522 */	0x36,		/* FC_POINTER */
			0x5b,		/* FC_END */
/* 524 */	
			0x11, 0x0,	/* FC_RP */
/* 526 */	NdrFcShort( 0xffdc ),	/* Offset= -36 (490) */
/* 528 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 530 */	NdrFcShort( 0x0 ),	/* 0 */
/* 532 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 534 */	NdrFcShort( 0x0 ),	/* 0 */
/* 536 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 538 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 542 */	NdrFcShort( 0x0 ),	/* Corr flags:  */
/* 544 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 546 */	NdrFcShort( 0xff44 ),	/* Offset= -188 (358) */
/* 548 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 550 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 552 */	NdrFcShort( 0x10 ),	/* 16 */
/* 554 */	NdrFcShort( 0x0 ),	/* 0 */
/* 556 */	NdrFcShort( 0x6 ),	/* Offset= 6 (562) */
/* 558 */	0x8,		/* FC_LONG */
			0x40,		/* FC_STRUCTPAD4 */
/* 560 */	0x36,		/* FC_POINTER */
			0x5b,		/* FC_END */
/* 562 */	
			0x11, 0x0,	/* FC_RP */
/* 564 */	NdrFcShort( 0xffdc ),	/* Offset= -36 (528) */
/* 566 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 568 */	NdrFcShort( 0x0 ),	/* 0 */
/* 570 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 572 */	NdrFcShort( 0x0 ),	/* 0 */
/* 574 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 576 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 580 */	NdrFcShort( 0x0 ),	/* Corr flags:  */
/* 582 */	
			0x13, 0x0,	/* FC_OP */
/* 584 */	NdrFcShort( 0x176 ),	/* Offset= 374 (958) */
/* 586 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 588 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 590 */	NdrFcShort( 0x10 ),	/* 16 */
/* 592 */	NdrFcShort( 0x0 ),	/* 0 */
/* 594 */	NdrFcShort( 0x6 ),	/* Offset= 6 (600) */
/* 596 */	0x8,		/* FC_LONG */
			0x40,		/* FC_STRUCTPAD4 */
/* 598 */	0x36,		/* FC_POINTER */
			0x5b,		/* FC_END */
/* 600 */	
			0x11, 0x0,	/* FC_RP */
/* 602 */	NdrFcShort( 0xffdc ),	/* Offset= -36 (566) */
/* 604 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 606 */	NdrFcLong( 0x2f ),	/* 47 */
/* 610 */	NdrFcShort( 0x0 ),	/* 0 */
/* 612 */	NdrFcShort( 0x0 ),	/* 0 */
/* 614 */	0xc0,		/* 192 */
			0x0,		/* 0 */
/* 616 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 618 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 620 */	0x0,		/* 0 */
			0x46,		/* 70 */
/* 622 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 624 */	NdrFcShort( 0x1 ),	/* 1 */
/* 626 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 628 */	NdrFcShort( 0x4 ),	/* 4 */
/* 630 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 632 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 634 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 636 */	NdrFcShort( 0x18 ),	/* 24 */
/* 638 */	NdrFcShort( 0x0 ),	/* 0 */
/* 640 */	NdrFcShort( 0xa ),	/* Offset= 10 (650) */
/* 642 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 644 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 646 */	NdrFcShort( 0xffd6 ),	/* Offset= -42 (604) */
/* 648 */	0x36,		/* FC_POINTER */
			0x5b,		/* FC_END */
/* 650 */	
			0x13, 0x0,	/* FC_OP */
/* 652 */	NdrFcShort( 0xffe2 ),	/* Offset= -30 (622) */
/* 654 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 656 */	NdrFcShort( 0x0 ),	/* 0 */
/* 658 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 660 */	NdrFcShort( 0x0 ),	/* 0 */
/* 662 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 664 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 668 */	NdrFcShort( 0x0 ),	/* Corr flags:  */
/* 670 */	
			0x13, 0x0,	/* FC_OP */
/* 672 */	NdrFcShort( 0xffda ),	/* Offset= -38 (634) */
/* 674 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 676 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 678 */	NdrFcShort( 0x10 ),	/* 16 */
/* 680 */	NdrFcShort( 0x0 ),	/* 0 */
/* 682 */	NdrFcShort( 0x6 ),	/* Offset= 6 (688) */
/* 684 */	0x8,		/* FC_LONG */
			0x40,		/* FC_STRUCTPAD4 */
/* 686 */	0x36,		/* FC_POINTER */
			0x5b,		/* FC_END */
/* 688 */	
			0x11, 0x0,	/* FC_RP */
/* 690 */	NdrFcShort( 0xffdc ),	/* Offset= -36 (654) */
/* 692 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 694 */	NdrFcShort( 0x8 ),	/* 8 */
/* 696 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 698 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 700 */	NdrFcShort( 0x10 ),	/* 16 */
/* 702 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 704 */	0x6,		/* FC_SHORT */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 706 */	0x0,		/* 0 */
			NdrFcShort( 0xfff1 ),	/* Offset= -15 (692) */
			0x5b,		/* FC_END */
/* 710 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 712 */	NdrFcShort( 0x20 ),	/* 32 */
/* 714 */	NdrFcShort( 0x0 ),	/* 0 */
/* 716 */	NdrFcShort( 0xa ),	/* Offset= 10 (726) */
/* 718 */	0x8,		/* FC_LONG */
			0x40,		/* FC_STRUCTPAD4 */
/* 720 */	0x36,		/* FC_POINTER */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 722 */	0x0,		/* 0 */
			NdrFcShort( 0xffe7 ),	/* Offset= -25 (698) */
			0x5b,		/* FC_END */
/* 726 */	
			0x11, 0x0,	/* FC_RP */
/* 728 */	NdrFcShort( 0xff12 ),	/* Offset= -238 (490) */
/* 730 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 732 */	NdrFcShort( 0x1 ),	/* 1 */
/* 734 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 736 */	NdrFcShort( 0x0 ),	/* 0 */
/* 738 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 740 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 742 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 744 */	NdrFcShort( 0x10 ),	/* 16 */
/* 746 */	NdrFcShort( 0x0 ),	/* 0 */
/* 748 */	NdrFcShort( 0x6 ),	/* Offset= 6 (754) */
/* 750 */	0x8,		/* FC_LONG */
			0x40,		/* FC_STRUCTPAD4 */
/* 752 */	0x36,		/* FC_POINTER */
			0x5b,		/* FC_END */
/* 754 */	
			0x13, 0x0,	/* FC_OP */
/* 756 */	NdrFcShort( 0xffe6 ),	/* Offset= -26 (730) */
/* 758 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 760 */	NdrFcShort( 0x2 ),	/* 2 */
/* 762 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 764 */	NdrFcShort( 0x0 ),	/* 0 */
/* 766 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 768 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 770 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 772 */	NdrFcShort( 0x10 ),	/* 16 */
/* 774 */	NdrFcShort( 0x0 ),	/* 0 */
/* 776 */	NdrFcShort( 0x6 ),	/* Offset= 6 (782) */
/* 778 */	0x8,		/* FC_LONG */
			0x40,		/* FC_STRUCTPAD4 */
/* 780 */	0x36,		/* FC_POINTER */
			0x5b,		/* FC_END */
/* 782 */	
			0x13, 0x0,	/* FC_OP */
/* 784 */	NdrFcShort( 0xffe6 ),	/* Offset= -26 (758) */
/* 786 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 788 */	NdrFcShort( 0x4 ),	/* 4 */
/* 790 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 792 */	NdrFcShort( 0x0 ),	/* 0 */
/* 794 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 796 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 798 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 800 */	NdrFcShort( 0x10 ),	/* 16 */
/* 802 */	NdrFcShort( 0x0 ),	/* 0 */
/* 804 */	NdrFcShort( 0x6 ),	/* Offset= 6 (810) */
/* 806 */	0x8,		/* FC_LONG */
			0x40,		/* FC_STRUCTPAD4 */
/* 808 */	0x36,		/* FC_POINTER */
			0x5b,		/* FC_END */
/* 810 */	
			0x13, 0x0,	/* FC_OP */
/* 812 */	NdrFcShort( 0xffe6 ),	/* Offset= -26 (786) */
/* 814 */	
			0x1b,		/* FC_CARRAY */
			0x7,		/* 7 */
/* 816 */	NdrFcShort( 0x8 ),	/* 8 */
/* 818 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 820 */	NdrFcShort( 0x0 ),	/* 0 */
/* 822 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 824 */	0xb,		/* FC_HYPER */
			0x5b,		/* FC_END */
/* 826 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 828 */	NdrFcShort( 0x10 ),	/* 16 */
/* 830 */	NdrFcShort( 0x0 ),	/* 0 */
/* 832 */	NdrFcShort( 0x6 ),	/* Offset= 6 (838) */
/* 834 */	0x8,		/* FC_LONG */
			0x40,		/* FC_STRUCTPAD4 */
/* 836 */	0x36,		/* FC_POINTER */
			0x5b,		/* FC_END */
/* 838 */	
			0x13, 0x0,	/* FC_OP */
/* 840 */	NdrFcShort( 0xffe6 ),	/* Offset= -26 (814) */
/* 842 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 844 */	NdrFcShort( 0x8 ),	/* 8 */
/* 846 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 848 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 850 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 852 */	NdrFcShort( 0x8 ),	/* 8 */
/* 854 */	0x7,		/* Corr desc: FC_USHORT */
			0x0,		/*  */
/* 856 */	NdrFcShort( 0xffc8 ),	/* -56 */
/* 858 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 860 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 862 */	NdrFcShort( 0xffec ),	/* Offset= -20 (842) */
/* 864 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 866 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 868 */	NdrFcShort( 0x38 ),	/* 56 */
/* 870 */	NdrFcShort( 0xffec ),	/* Offset= -20 (850) */
/* 872 */	NdrFcShort( 0x0 ),	/* Offset= 0 (872) */
/* 874 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 876 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 878 */	0x40,		/* FC_STRUCTPAD4 */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 880 */	0x0,		/* 0 */
			NdrFcShort( 0xfe0f ),	/* Offset= -497 (384) */
			0x5b,		/* FC_END */
/* 884 */	
			0x13, 0x0,	/* FC_OP */
/* 886 */	NdrFcShort( 0xff04 ),	/* Offset= -252 (634) */
/* 888 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 890 */	0x1,		/* FC_BYTE */
			0x5c,		/* FC_PAD */
/* 892 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 894 */	0x6,		/* FC_SHORT */
			0x5c,		/* FC_PAD */
/* 896 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 898 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 900 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 902 */	0xb,		/* FC_HYPER */
			0x5c,		/* FC_PAD */
/* 904 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 906 */	0xa,		/* FC_FLOAT */
			0x5c,		/* FC_PAD */
/* 908 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 910 */	0xc,		/* FC_DOUBLE */
			0x5c,		/* FC_PAD */
/* 912 */	
			0x13, 0x0,	/* FC_OP */
/* 914 */	NdrFcShort( 0xfda2 ),	/* Offset= -606 (308) */
/* 916 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 918 */	NdrFcShort( 0xfda4 ),	/* Offset= -604 (314) */
/* 920 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 922 */	NdrFcShort( 0xfdba ),	/* Offset= -582 (340) */
/* 924 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 926 */	NdrFcShort( 0xfdc8 ),	/* Offset= -568 (358) */
/* 928 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 930 */	NdrFcShort( 0xfdd6 ),	/* Offset= -554 (376) */
/* 932 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 934 */	NdrFcShort( 0x2 ),	/* Offset= 2 (936) */
/* 936 */	
			0x13, 0x0,	/* FC_OP */
/* 938 */	NdrFcShort( 0x14 ),	/* Offset= 20 (958) */
/* 940 */	
			0x15,		/* FC_STRUCT */
			0x7,		/* 7 */
/* 942 */	NdrFcShort( 0x10 ),	/* 16 */
/* 944 */	0x6,		/* FC_SHORT */
			0x1,		/* FC_BYTE */
/* 946 */	0x1,		/* FC_BYTE */
			0x8,		/* FC_LONG */
/* 948 */	0xb,		/* FC_HYPER */
			0x5b,		/* FC_END */
/* 950 */	
			0x13, 0x0,	/* FC_OP */
/* 952 */	NdrFcShort( 0xfff4 ),	/* Offset= -12 (940) */
/* 954 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 956 */	0x2,		/* FC_CHAR */
			0x5c,		/* FC_PAD */
/* 958 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x7,		/* 7 */
/* 960 */	NdrFcShort( 0x20 ),	/* 32 */
/* 962 */	NdrFcShort( 0x0 ),	/* 0 */
/* 964 */	NdrFcShort( 0x0 ),	/* Offset= 0 (964) */
/* 966 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 968 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 970 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 972 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 974 */	NdrFcShort( 0xfc3c ),	/* Offset= -964 (10) */
/* 976 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 978 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 980 */	NdrFcShort( 0x0 ),	/* 0 */
/* 982 */	NdrFcShort( 0x18 ),	/* 24 */
/* 984 */	NdrFcShort( 0x0 ),	/* 0 */
/* 986 */	NdrFcShort( 0xfc2c ),	/* Offset= -980 (6) */
/* 988 */	
			0x12, 0x0,	/* FC_UP */
/* 990 */	NdrFcShort( 0xffe0 ),	/* Offset= -32 (958) */
/* 992 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 994 */	NdrFcShort( 0x0 ),	/* 0 */
/* 996 */	NdrFcShort( 0x18 ),	/* 24 */
/* 998 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1000 */	NdrFcShort( 0xfff4 ),	/* Offset= -12 (988) */
/* 1002 */	
			0x11, 0x0,	/* FC_RP */
/* 1004 */	NdrFcShort( 0xffe6 ),	/* Offset= -26 (978) */
/* 1006 */	
			0x12, 0x0,	/* FC_UP */
/* 1008 */	NdrFcShort( 0xfd5a ),	/* Offset= -678 (330) */
/* 1010 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 1012 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1014 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1016 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1018 */	NdrFcShort( 0xfff4 ),	/* Offset= -12 (1006) */
/* 1020 */	
			0x11, 0x10,	/* FC_RP [pointer_deref] */
/* 1022 */	NdrFcShort( 0xfd68 ),	/* Offset= -664 (358) */
/* 1024 */	
			0x11, 0xc,	/* FC_RP [alloced_on_stack] [simple_pointer] */
/* 1026 */	0x6,		/* FC_SHORT */
			0x5c,		/* FC_PAD */

			0x0
        }
    };

static const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ] = 
        {
            
            {
            VARIANT_UserSize
            ,VARIANT_UserMarshal
            ,VARIANT_UserUnmarshal
            ,VARIANT_UserFree
            },
            {
            BSTR_UserSize
            ,BSTR_UserMarshal
            ,BSTR_UserUnmarshal
            ,BSTR_UserFree
            }

        };



/* Object interface: IUnknown, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: IDispatch, ver. 0.0,
   GUID={0x00020400,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: IMWUtil, ver. 0.0,
   GUID={0xC47EA90E,0x56D1,0x11d5,{0xB1,0x59,0x00,0xD0,0xB7,0xBA,0x75,0x44}} */

#pragma code_seg(".orpc")
static const unsigned short IMWUtil_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0,
    230,
    472,
    510,
    548,
    592,
    636,
    674
    };

static const MIDL_STUBLESS_PROXY_INFO IMWUtil_ProxyInfo =
    {
    &Object_StubDesc,
    mwcomutil__MIDL_ProcFormatString.Format,
    &IMWUtil_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IMWUtil_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    mwcomutil__MIDL_ProcFormatString.Format,
    &IMWUtil_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(15) _IMWUtilProxyVtbl = 
{
    &IMWUtil_ProxyInfo,
    &IID_IMWUtil,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* IDispatch::GetTypeInfoCount */ ,
    0 /* IDispatch::GetTypeInfo */ ,
    0 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */ ,
    (void *) (INT_PTR) -1 /* IMWUtil::MWPack */ ,
    (void *) (INT_PTR) -1 /* IMWUtil::MWUnpack */ ,
    (void *) (INT_PTR) -1 /* IMWUtil::MWDate2VariantDate */ ,
    (void *) (INT_PTR) -1 /* IMWUtil::MWInitApplication */ ,
    (void *) (INT_PTR) -1 /* IMWUtil::MWInitApplicationWithMCROptions */ ,
    (void *) (INT_PTR) -1 /* IMWUtil::CreateRemoteObject */ ,
    (void *) (INT_PTR) -1 /* IMWUtil::IsMCRJVMEnabled */ ,
    (void *) (INT_PTR) -1 /* IMWUtil::IsMCRInitialized */
};


static const PRPC_STUB_FUNCTION IMWUtil_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2
};

CInterfaceStubVtbl _IMWUtilStubVtbl =
{
    &IID_IMWUtil,
    &IMWUtil_ServerInfo,
    15,
    &IMWUtil_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};

static const MIDL_STUB_DESC Object_StubDesc = 
    {
    0,
    NdrOleAllocate,
    NdrOleFree,
    0,
    0,
    0,
    0,
    0,
    mwcomutil__MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x50002, /* Ndr library version */
    0,
    0x800025b, /* MIDL Version 8.0.603 */
    0,
    UserMarshalRoutines,
    0,  /* notify & notify_flag routine table */
    0x1, /* MIDL flag */
    0, /* cs routines */
    0,   /* proxy/server info */
    0
    };

const CInterfaceProxyVtbl * const _mwcomutil_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_IMWUtilProxyVtbl,
    0
};

const CInterfaceStubVtbl * const _mwcomutil_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_IMWUtilStubVtbl,
    0
};

PCInterfaceName const _mwcomutil_InterfaceNamesList[] = 
{
    "IMWUtil",
    0
};

const IID *  const _mwcomutil_BaseIIDList[] = 
{
    &IID_IDispatch,
    0
};


#define _mwcomutil_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _mwcomutil, pIID, n)

int __stdcall _mwcomutil_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!_mwcomutil_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo mwcomutil_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _mwcomutil_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _mwcomutil_StubVtblList,
    (const PCInterfaceName * ) & _mwcomutil_InterfaceNamesList,
    (const IID ** ) & _mwcomutil_BaseIIDList,
    & _mwcomutil_IID_Lookup, 
    1,
    2,
    0, /* table of [async_uuid] interfaces */
    0, /* Filler1 */
    0, /* Filler2 */
    0  /* Filler3 */
};
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif /* defined(_M_IA64) */

