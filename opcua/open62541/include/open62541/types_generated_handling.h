/* Generated from Opc.Ua.Types.bsd with script /home/travis/build/open62541/open62541/tools/generate_datatypes.py
 * on host travis-job-7c4d9096-818b-4fc6-8e72-ec7f0cf41f8e by user travis at 2020-01-30 01:03:09 */

#ifndef TYPES_GENERATED_HANDLING_H_
#define TYPES_GENERATED_HANDLING_H_

#include "types_generated.h"

_UA_BEGIN_DECLS

#if defined(__GNUC__) && __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wmissing-field-initializers"
# pragma GCC diagnostic ignored "-Wmissing-braces"
#endif


/* Boolean */
static UA_INLINE void
UA_Boolean_init(UA_Boolean *p) {
    memset(p, 0, sizeof(UA_Boolean));
}

static UA_INLINE UA_Boolean *
UA_Boolean_new(void) {
    return (UA_Boolean*)UA_new(&UA_TYPES[UA_TYPES_BOOLEAN]);
}

static UA_INLINE UA_StatusCode
UA_Boolean_copy(const UA_Boolean *src, UA_Boolean *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_Boolean_deleteMembers(UA_Boolean *p) {
    memset(p, 0, sizeof(UA_Boolean));
}

static UA_INLINE void
UA_Boolean_clear(UA_Boolean *p) {
    memset(p, 0, sizeof(UA_Boolean));
}

static UA_INLINE void
UA_Boolean_delete(UA_Boolean *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_BOOLEAN]);
}

/* SByte */
static UA_INLINE void
UA_SByte_init(UA_SByte *p) {
    memset(p, 0, sizeof(UA_SByte));
}

static UA_INLINE UA_SByte *
UA_SByte_new(void) {
    return (UA_SByte*)UA_new(&UA_TYPES[UA_TYPES_SBYTE]);
}

static UA_INLINE UA_StatusCode
UA_SByte_copy(const UA_SByte *src, UA_SByte *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_SByte_deleteMembers(UA_SByte *p) {
    memset(p, 0, sizeof(UA_SByte));
}

static UA_INLINE void
UA_SByte_clear(UA_SByte *p) {
    memset(p, 0, sizeof(UA_SByte));
}

static UA_INLINE void
UA_SByte_delete(UA_SByte *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_SBYTE]);
}

/* Byte */
static UA_INLINE void
UA_Byte_init(UA_Byte *p) {
    memset(p, 0, sizeof(UA_Byte));
}

static UA_INLINE UA_Byte *
UA_Byte_new(void) {
    return (UA_Byte*)UA_new(&UA_TYPES[UA_TYPES_BYTE]);
}

static UA_INLINE UA_StatusCode
UA_Byte_copy(const UA_Byte *src, UA_Byte *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_Byte_deleteMembers(UA_Byte *p) {
    memset(p, 0, sizeof(UA_Byte));
}

static UA_INLINE void
UA_Byte_clear(UA_Byte *p) {
    memset(p, 0, sizeof(UA_Byte));
}

static UA_INLINE void
UA_Byte_delete(UA_Byte *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_BYTE]);
}

/* Int16 */
static UA_INLINE void
UA_Int16_init(UA_Int16 *p) {
    memset(p, 0, sizeof(UA_Int16));
}

static UA_INLINE UA_Int16 *
UA_Int16_new(void) {
    return (UA_Int16*)UA_new(&UA_TYPES[UA_TYPES_INT16]);
}

static UA_INLINE UA_StatusCode
UA_Int16_copy(const UA_Int16 *src, UA_Int16 *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_Int16_deleteMembers(UA_Int16 *p) {
    memset(p, 0, sizeof(UA_Int16));
}

static UA_INLINE void
UA_Int16_clear(UA_Int16 *p) {
    memset(p, 0, sizeof(UA_Int16));
}

static UA_INLINE void
UA_Int16_delete(UA_Int16 *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_INT16]);
}

/* UInt16 */
static UA_INLINE void
UA_UInt16_init(UA_UInt16 *p) {
    memset(p, 0, sizeof(UA_UInt16));
}

static UA_INLINE UA_UInt16 *
UA_UInt16_new(void) {
    return (UA_UInt16*)UA_new(&UA_TYPES[UA_TYPES_UINT16]);
}

static UA_INLINE UA_StatusCode
UA_UInt16_copy(const UA_UInt16 *src, UA_UInt16 *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_UInt16_deleteMembers(UA_UInt16 *p) {
    memset(p, 0, sizeof(UA_UInt16));
}

static UA_INLINE void
UA_UInt16_clear(UA_UInt16 *p) {
    memset(p, 0, sizeof(UA_UInt16));
}

static UA_INLINE void
UA_UInt16_delete(UA_UInt16 *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_UINT16]);
}

/* Int32 */
static UA_INLINE void
UA_Int32_init(UA_Int32 *p) {
    memset(p, 0, sizeof(UA_Int32));
}

static UA_INLINE UA_Int32 *
UA_Int32_new(void) {
    return (UA_Int32*)UA_new(&UA_TYPES[UA_TYPES_INT32]);
}

static UA_INLINE UA_StatusCode
UA_Int32_copy(const UA_Int32 *src, UA_Int32 *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_Int32_deleteMembers(UA_Int32 *p) {
    memset(p, 0, sizeof(UA_Int32));
}

static UA_INLINE void
UA_Int32_clear(UA_Int32 *p) {
    memset(p, 0, sizeof(UA_Int32));
}

static UA_INLINE void
UA_Int32_delete(UA_Int32 *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_INT32]);
}

/* UInt32 */
static UA_INLINE void
UA_UInt32_init(UA_UInt32 *p) {
    memset(p, 0, sizeof(UA_UInt32));
}

static UA_INLINE UA_UInt32 *
UA_UInt32_new(void) {
    return (UA_UInt32*)UA_new(&UA_TYPES[UA_TYPES_UINT32]);
}

static UA_INLINE UA_StatusCode
UA_UInt32_copy(const UA_UInt32 *src, UA_UInt32 *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_UInt32_deleteMembers(UA_UInt32 *p) {
    memset(p, 0, sizeof(UA_UInt32));
}

static UA_INLINE void
UA_UInt32_clear(UA_UInt32 *p) {
    memset(p, 0, sizeof(UA_UInt32));
}

static UA_INLINE void
UA_UInt32_delete(UA_UInt32 *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_UINT32]);
}

/* Int64 */
static UA_INLINE void
UA_Int64_init(UA_Int64 *p) {
    memset(p, 0, sizeof(UA_Int64));
}

static UA_INLINE UA_Int64 *
UA_Int64_new(void) {
    return (UA_Int64*)UA_new(&UA_TYPES[UA_TYPES_INT64]);
}

static UA_INLINE UA_StatusCode
UA_Int64_copy(const UA_Int64 *src, UA_Int64 *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_Int64_deleteMembers(UA_Int64 *p) {
    memset(p, 0, sizeof(UA_Int64));
}

static UA_INLINE void
UA_Int64_clear(UA_Int64 *p) {
    memset(p, 0, sizeof(UA_Int64));
}

static UA_INLINE void
UA_Int64_delete(UA_Int64 *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_INT64]);
}

/* UInt64 */
static UA_INLINE void
UA_UInt64_init(UA_UInt64 *p) {
    memset(p, 0, sizeof(UA_UInt64));
}

static UA_INLINE UA_UInt64 *
UA_UInt64_new(void) {
    return (UA_UInt64*)UA_new(&UA_TYPES[UA_TYPES_UINT64]);
}

static UA_INLINE UA_StatusCode
UA_UInt64_copy(const UA_UInt64 *src, UA_UInt64 *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_UInt64_deleteMembers(UA_UInt64 *p) {
    memset(p, 0, sizeof(UA_UInt64));
}

static UA_INLINE void
UA_UInt64_clear(UA_UInt64 *p) {
    memset(p, 0, sizeof(UA_UInt64));
}

static UA_INLINE void
UA_UInt64_delete(UA_UInt64 *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_UINT64]);
}

/* Float */
static UA_INLINE void
UA_Float_init(UA_Float *p) {
    memset(p, 0, sizeof(UA_Float));
}

static UA_INLINE UA_Float *
UA_Float_new(void) {
    return (UA_Float*)UA_new(&UA_TYPES[UA_TYPES_FLOAT]);
}

static UA_INLINE UA_StatusCode
UA_Float_copy(const UA_Float *src, UA_Float *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_Float_deleteMembers(UA_Float *p) {
    memset(p, 0, sizeof(UA_Float));
}

static UA_INLINE void
UA_Float_clear(UA_Float *p) {
    memset(p, 0, sizeof(UA_Float));
}

static UA_INLINE void
UA_Float_delete(UA_Float *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_FLOAT]);
}

/* Double */
static UA_INLINE void
UA_Double_init(UA_Double *p) {
    memset(p, 0, sizeof(UA_Double));
}

static UA_INLINE UA_Double *
UA_Double_new(void) {
    return (UA_Double*)UA_new(&UA_TYPES[UA_TYPES_DOUBLE]);
}

static UA_INLINE UA_StatusCode
UA_Double_copy(const UA_Double *src, UA_Double *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_Double_deleteMembers(UA_Double *p) {
    memset(p, 0, sizeof(UA_Double));
}

static UA_INLINE void
UA_Double_clear(UA_Double *p) {
    memset(p, 0, sizeof(UA_Double));
}

static UA_INLINE void
UA_Double_delete(UA_Double *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_DOUBLE]);
}

/* String */
static UA_INLINE void
UA_String_init(UA_String *p) {
    memset(p, 0, sizeof(UA_String));
}

static UA_INLINE UA_String *
UA_String_new(void) {
    return (UA_String*)UA_new(&UA_TYPES[UA_TYPES_STRING]);
}

static UA_INLINE UA_StatusCode
UA_String_copy(const UA_String *src, UA_String *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_STRING]);
}

static UA_INLINE void
UA_String_deleteMembers(UA_String *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_STRING]);
}

static UA_INLINE void
UA_String_clear(UA_String *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_STRING]);
}

static UA_INLINE void
UA_String_delete(UA_String *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_STRING]);
}

/* DateTime */
static UA_INLINE void
UA_DateTime_init(UA_DateTime *p) {
    memset(p, 0, sizeof(UA_DateTime));
}

static UA_INLINE UA_DateTime *
UA_DateTime_new(void) {
    return (UA_DateTime*)UA_new(&UA_TYPES[UA_TYPES_DATETIME]);
}

static UA_INLINE UA_StatusCode
UA_DateTime_copy(const UA_DateTime *src, UA_DateTime *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_DateTime_deleteMembers(UA_DateTime *p) {
    memset(p, 0, sizeof(UA_DateTime));
}

static UA_INLINE void
UA_DateTime_clear(UA_DateTime *p) {
    memset(p, 0, sizeof(UA_DateTime));
}

static UA_INLINE void
UA_DateTime_delete(UA_DateTime *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_DATETIME]);
}

/* Guid */
static UA_INLINE void
UA_Guid_init(UA_Guid *p) {
    memset(p, 0, sizeof(UA_Guid));
}

static UA_INLINE UA_Guid *
UA_Guid_new(void) {
    return (UA_Guid*)UA_new(&UA_TYPES[UA_TYPES_GUID]);
}

static UA_INLINE UA_StatusCode
UA_Guid_copy(const UA_Guid *src, UA_Guid *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_Guid_deleteMembers(UA_Guid *p) {
    memset(p, 0, sizeof(UA_Guid));
}

static UA_INLINE void
UA_Guid_clear(UA_Guid *p) {
    memset(p, 0, sizeof(UA_Guid));
}

static UA_INLINE void
UA_Guid_delete(UA_Guid *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_GUID]);
}

/* ByteString */
static UA_INLINE void
UA_ByteString_init(UA_ByteString *p) {
    memset(p, 0, sizeof(UA_ByteString));
}

static UA_INLINE UA_ByteString *
UA_ByteString_new(void) {
    return (UA_ByteString*)UA_new(&UA_TYPES[UA_TYPES_BYTESTRING]);
}

static UA_INLINE UA_StatusCode
UA_ByteString_copy(const UA_ByteString *src, UA_ByteString *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_BYTESTRING]);
}

static UA_INLINE void
UA_ByteString_deleteMembers(UA_ByteString *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_BYTESTRING]);
}

static UA_INLINE void
UA_ByteString_clear(UA_ByteString *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_BYTESTRING]);
}

static UA_INLINE void
UA_ByteString_delete(UA_ByteString *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_BYTESTRING]);
}

/* XmlElement */
static UA_INLINE void
UA_XmlElement_init(UA_XmlElement *p) {
    memset(p, 0, sizeof(UA_XmlElement));
}

static UA_INLINE UA_XmlElement *
UA_XmlElement_new(void) {
    return (UA_XmlElement*)UA_new(&UA_TYPES[UA_TYPES_XMLELEMENT]);
}

static UA_INLINE UA_StatusCode
UA_XmlElement_copy(const UA_XmlElement *src, UA_XmlElement *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_XMLELEMENT]);
}

static UA_INLINE void
UA_XmlElement_deleteMembers(UA_XmlElement *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_XMLELEMENT]);
}

static UA_INLINE void
UA_XmlElement_clear(UA_XmlElement *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_XMLELEMENT]);
}

static UA_INLINE void
UA_XmlElement_delete(UA_XmlElement *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_XMLELEMENT]);
}

/* NodeId */
static UA_INLINE void
UA_NodeId_init(UA_NodeId *p) {
    memset(p, 0, sizeof(UA_NodeId));
}

static UA_INLINE UA_NodeId *
UA_NodeId_new(void) {
    return (UA_NodeId*)UA_new(&UA_TYPES[UA_TYPES_NODEID]);
}

static UA_INLINE UA_StatusCode
UA_NodeId_copy(const UA_NodeId *src, UA_NodeId *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_NODEID]);
}

static UA_INLINE void
UA_NodeId_deleteMembers(UA_NodeId *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_NODEID]);
}

static UA_INLINE void
UA_NodeId_clear(UA_NodeId *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_NODEID]);
}

static UA_INLINE void
UA_NodeId_delete(UA_NodeId *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_NODEID]);
}

/* ExpandedNodeId */
static UA_INLINE void
UA_ExpandedNodeId_init(UA_ExpandedNodeId *p) {
    memset(p, 0, sizeof(UA_ExpandedNodeId));
}

static UA_INLINE UA_ExpandedNodeId *
UA_ExpandedNodeId_new(void) {
    return (UA_ExpandedNodeId*)UA_new(&UA_TYPES[UA_TYPES_EXPANDEDNODEID]);
}

static UA_INLINE UA_StatusCode
UA_ExpandedNodeId_copy(const UA_ExpandedNodeId *src, UA_ExpandedNodeId *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_EXPANDEDNODEID]);
}

static UA_INLINE void
UA_ExpandedNodeId_deleteMembers(UA_ExpandedNodeId *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_EXPANDEDNODEID]);
}

static UA_INLINE void
UA_ExpandedNodeId_clear(UA_ExpandedNodeId *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_EXPANDEDNODEID]);
}

static UA_INLINE void
UA_ExpandedNodeId_delete(UA_ExpandedNodeId *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_EXPANDEDNODEID]);
}

/* StatusCode */
static UA_INLINE void
UA_StatusCode_init(UA_StatusCode *p) {
    memset(p, 0, sizeof(UA_StatusCode));
}

static UA_INLINE UA_StatusCode *
UA_StatusCode_new(void) {
    return (UA_StatusCode*)UA_new(&UA_TYPES[UA_TYPES_STATUSCODE]);
}

static UA_INLINE UA_StatusCode
UA_StatusCode_copy(const UA_StatusCode *src, UA_StatusCode *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_StatusCode_deleteMembers(UA_StatusCode *p) {
    memset(p, 0, sizeof(UA_StatusCode));
}

static UA_INLINE void
UA_StatusCode_clear(UA_StatusCode *p) {
    memset(p, 0, sizeof(UA_StatusCode));
}

static UA_INLINE void
UA_StatusCode_delete(UA_StatusCode *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_STATUSCODE]);
}

/* QualifiedName */
static UA_INLINE void
UA_QualifiedName_init(UA_QualifiedName *p) {
    memset(p, 0, sizeof(UA_QualifiedName));
}

static UA_INLINE UA_QualifiedName *
UA_QualifiedName_new(void) {
    return (UA_QualifiedName*)UA_new(&UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
}

static UA_INLINE UA_StatusCode
UA_QualifiedName_copy(const UA_QualifiedName *src, UA_QualifiedName *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
}

static UA_INLINE void
UA_QualifiedName_deleteMembers(UA_QualifiedName *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
}

static UA_INLINE void
UA_QualifiedName_clear(UA_QualifiedName *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
}

static UA_INLINE void
UA_QualifiedName_delete(UA_QualifiedName *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
}

/* LocalizedText */
static UA_INLINE void
UA_LocalizedText_init(UA_LocalizedText *p) {
    memset(p, 0, sizeof(UA_LocalizedText));
}

static UA_INLINE UA_LocalizedText *
UA_LocalizedText_new(void) {
    return (UA_LocalizedText*)UA_new(&UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
}

static UA_INLINE UA_StatusCode
UA_LocalizedText_copy(const UA_LocalizedText *src, UA_LocalizedText *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
}

static UA_INLINE void
UA_LocalizedText_deleteMembers(UA_LocalizedText *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
}

static UA_INLINE void
UA_LocalizedText_clear(UA_LocalizedText *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
}

static UA_INLINE void
UA_LocalizedText_delete(UA_LocalizedText *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
}

/* ExtensionObject */
static UA_INLINE void
UA_ExtensionObject_init(UA_ExtensionObject *p) {
    memset(p, 0, sizeof(UA_ExtensionObject));
}

static UA_INLINE UA_ExtensionObject *
UA_ExtensionObject_new(void) {
    return (UA_ExtensionObject*)UA_new(&UA_TYPES[UA_TYPES_EXTENSIONOBJECT]);
}

static UA_INLINE UA_StatusCode
UA_ExtensionObject_copy(const UA_ExtensionObject *src, UA_ExtensionObject *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_EXTENSIONOBJECT]);
}

static UA_INLINE void
UA_ExtensionObject_deleteMembers(UA_ExtensionObject *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_EXTENSIONOBJECT]);
}

static UA_INLINE void
UA_ExtensionObject_clear(UA_ExtensionObject *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_EXTENSIONOBJECT]);
}

static UA_INLINE void
UA_ExtensionObject_delete(UA_ExtensionObject *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_EXTENSIONOBJECT]);
}

/* DataValue */
static UA_INLINE void
UA_DataValue_init(UA_DataValue *p) {
    memset(p, 0, sizeof(UA_DataValue));
}

static UA_INLINE UA_DataValue *
UA_DataValue_new(void) {
    return (UA_DataValue*)UA_new(&UA_TYPES[UA_TYPES_DATAVALUE]);
}

static UA_INLINE UA_StatusCode
UA_DataValue_copy(const UA_DataValue *src, UA_DataValue *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DATAVALUE]);
}

static UA_INLINE void
UA_DataValue_deleteMembers(UA_DataValue *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DATAVALUE]);
}

static UA_INLINE void
UA_DataValue_clear(UA_DataValue *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DATAVALUE]);
}

static UA_INLINE void
UA_DataValue_delete(UA_DataValue *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_DATAVALUE]);
}

/* Variant */
static UA_INLINE void
UA_Variant_init(UA_Variant *p) {
    memset(p, 0, sizeof(UA_Variant));
}

static UA_INLINE UA_Variant *
UA_Variant_new(void) {
    return (UA_Variant*)UA_new(&UA_TYPES[UA_TYPES_VARIANT]);
}

static UA_INLINE UA_StatusCode
UA_Variant_copy(const UA_Variant *src, UA_Variant *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_VARIANT]);
}

static UA_INLINE void
UA_Variant_deleteMembers(UA_Variant *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_VARIANT]);
}

static UA_INLINE void
UA_Variant_clear(UA_Variant *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_VARIANT]);
}

static UA_INLINE void
UA_Variant_delete(UA_Variant *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_VARIANT]);
}

/* DiagnosticInfo */
static UA_INLINE void
UA_DiagnosticInfo_init(UA_DiagnosticInfo *p) {
    memset(p, 0, sizeof(UA_DiagnosticInfo));
}

static UA_INLINE UA_DiagnosticInfo *
UA_DiagnosticInfo_new(void) {
    return (UA_DiagnosticInfo*)UA_new(&UA_TYPES[UA_TYPES_DIAGNOSTICINFO]);
}

static UA_INLINE UA_StatusCode
UA_DiagnosticInfo_copy(const UA_DiagnosticInfo *src, UA_DiagnosticInfo *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DIAGNOSTICINFO]);
}

static UA_INLINE void
UA_DiagnosticInfo_deleteMembers(UA_DiagnosticInfo *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DIAGNOSTICINFO]);
}

static UA_INLINE void
UA_DiagnosticInfo_clear(UA_DiagnosticInfo *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DIAGNOSTICINFO]);
}

static UA_INLINE void
UA_DiagnosticInfo_delete(UA_DiagnosticInfo *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_DIAGNOSTICINFO]);
}

/* ViewAttributes */
static UA_INLINE void
UA_ViewAttributes_init(UA_ViewAttributes *p) {
    memset(p, 0, sizeof(UA_ViewAttributes));
}

static UA_INLINE UA_ViewAttributes *
UA_ViewAttributes_new(void) {
    return (UA_ViewAttributes*)UA_new(&UA_TYPES[UA_TYPES_VIEWATTRIBUTES]);
}

static UA_INLINE UA_StatusCode
UA_ViewAttributes_copy(const UA_ViewAttributes *src, UA_ViewAttributes *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_VIEWATTRIBUTES]);
}

static UA_INLINE void
UA_ViewAttributes_deleteMembers(UA_ViewAttributes *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_VIEWATTRIBUTES]);
}

static UA_INLINE void
UA_ViewAttributes_clear(UA_ViewAttributes *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_VIEWATTRIBUTES]);
}

static UA_INLINE void
UA_ViewAttributes_delete(UA_ViewAttributes *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_VIEWATTRIBUTES]);
}

/* ElementOperand */
static UA_INLINE void
UA_ElementOperand_init(UA_ElementOperand *p) {
    memset(p, 0, sizeof(UA_ElementOperand));
}

static UA_INLINE UA_ElementOperand *
UA_ElementOperand_new(void) {
    return (UA_ElementOperand*)UA_new(&UA_TYPES[UA_TYPES_ELEMENTOPERAND]);
}

static UA_INLINE UA_StatusCode
UA_ElementOperand_copy(const UA_ElementOperand *src, UA_ElementOperand *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_ElementOperand_deleteMembers(UA_ElementOperand *p) {
    memset(p, 0, sizeof(UA_ElementOperand));
}

static UA_INLINE void
UA_ElementOperand_clear(UA_ElementOperand *p) {
    memset(p, 0, sizeof(UA_ElementOperand));
}

static UA_INLINE void
UA_ElementOperand_delete(UA_ElementOperand *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_ELEMENTOPERAND]);
}

/* VariableAttributes */
static UA_INLINE void
UA_VariableAttributes_init(UA_VariableAttributes *p) {
    memset(p, 0, sizeof(UA_VariableAttributes));
}

static UA_INLINE UA_VariableAttributes *
UA_VariableAttributes_new(void) {
    return (UA_VariableAttributes*)UA_new(&UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES]);
}

static UA_INLINE UA_StatusCode
UA_VariableAttributes_copy(const UA_VariableAttributes *src, UA_VariableAttributes *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES]);
}

static UA_INLINE void
UA_VariableAttributes_deleteMembers(UA_VariableAttributes *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES]);
}

static UA_INLINE void
UA_VariableAttributes_clear(UA_VariableAttributes *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES]);
}

static UA_INLINE void
UA_VariableAttributes_delete(UA_VariableAttributes *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES]);
}

/* EnumValueType */
static UA_INLINE void
UA_EnumValueType_init(UA_EnumValueType *p) {
    memset(p, 0, sizeof(UA_EnumValueType));
}

static UA_INLINE UA_EnumValueType *
UA_EnumValueType_new(void) {
    return (UA_EnumValueType*)UA_new(&UA_TYPES[UA_TYPES_ENUMVALUETYPE]);
}

static UA_INLINE UA_StatusCode
UA_EnumValueType_copy(const UA_EnumValueType *src, UA_EnumValueType *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ENUMVALUETYPE]);
}

static UA_INLINE void
UA_EnumValueType_deleteMembers(UA_EnumValueType *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ENUMVALUETYPE]);
}

static UA_INLINE void
UA_EnumValueType_clear(UA_EnumValueType *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ENUMVALUETYPE]);
}

static UA_INLINE void
UA_EnumValueType_delete(UA_EnumValueType *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_ENUMVALUETYPE]);
}

/* EventFieldList */
static UA_INLINE void
UA_EventFieldList_init(UA_EventFieldList *p) {
    memset(p, 0, sizeof(UA_EventFieldList));
}

static UA_INLINE UA_EventFieldList *
UA_EventFieldList_new(void) {
    return (UA_EventFieldList*)UA_new(&UA_TYPES[UA_TYPES_EVENTFIELDLIST]);
}

static UA_INLINE UA_StatusCode
UA_EventFieldList_copy(const UA_EventFieldList *src, UA_EventFieldList *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_EVENTFIELDLIST]);
}

static UA_INLINE void
UA_EventFieldList_deleteMembers(UA_EventFieldList *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_EVENTFIELDLIST]);
}

static UA_INLINE void
UA_EventFieldList_clear(UA_EventFieldList *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_EVENTFIELDLIST]);
}

static UA_INLINE void
UA_EventFieldList_delete(UA_EventFieldList *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_EVENTFIELDLIST]);
}

/* MonitoredItemCreateResult */
static UA_INLINE void
UA_MonitoredItemCreateResult_init(UA_MonitoredItemCreateResult *p) {
    memset(p, 0, sizeof(UA_MonitoredItemCreateResult));
}

static UA_INLINE UA_MonitoredItemCreateResult *
UA_MonitoredItemCreateResult_new(void) {
    return (UA_MonitoredItemCreateResult*)UA_new(&UA_TYPES[UA_TYPES_MONITOREDITEMCREATERESULT]);
}

static UA_INLINE UA_StatusCode
UA_MonitoredItemCreateResult_copy(const UA_MonitoredItemCreateResult *src, UA_MonitoredItemCreateResult *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_MONITOREDITEMCREATERESULT]);
}

static UA_INLINE void
UA_MonitoredItemCreateResult_deleteMembers(UA_MonitoredItemCreateResult *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_MONITOREDITEMCREATERESULT]);
}

static UA_INLINE void
UA_MonitoredItemCreateResult_clear(UA_MonitoredItemCreateResult *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_MONITOREDITEMCREATERESULT]);
}

static UA_INLINE void
UA_MonitoredItemCreateResult_delete(UA_MonitoredItemCreateResult *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_MONITOREDITEMCREATERESULT]);
}

/* ServerDiagnosticsSummaryDataType */
static UA_INLINE void
UA_ServerDiagnosticsSummaryDataType_init(UA_ServerDiagnosticsSummaryDataType *p) {
    memset(p, 0, sizeof(UA_ServerDiagnosticsSummaryDataType));
}

static UA_INLINE UA_ServerDiagnosticsSummaryDataType *
UA_ServerDiagnosticsSummaryDataType_new(void) {
    return (UA_ServerDiagnosticsSummaryDataType*)UA_new(&UA_TYPES[UA_TYPES_SERVERDIAGNOSTICSSUMMARYDATATYPE]);
}

static UA_INLINE UA_StatusCode
UA_ServerDiagnosticsSummaryDataType_copy(const UA_ServerDiagnosticsSummaryDataType *src, UA_ServerDiagnosticsSummaryDataType *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_ServerDiagnosticsSummaryDataType_deleteMembers(UA_ServerDiagnosticsSummaryDataType *p) {
    memset(p, 0, sizeof(UA_ServerDiagnosticsSummaryDataType));
}

static UA_INLINE void
UA_ServerDiagnosticsSummaryDataType_clear(UA_ServerDiagnosticsSummaryDataType *p) {
    memset(p, 0, sizeof(UA_ServerDiagnosticsSummaryDataType));
}

static UA_INLINE void
UA_ServerDiagnosticsSummaryDataType_delete(UA_ServerDiagnosticsSummaryDataType *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_SERVERDIAGNOSTICSSUMMARYDATATYPE]);
}

/* ContentFilterElementResult */
static UA_INLINE void
UA_ContentFilterElementResult_init(UA_ContentFilterElementResult *p) {
    memset(p, 0, sizeof(UA_ContentFilterElementResult));
}

static UA_INLINE UA_ContentFilterElementResult *
UA_ContentFilterElementResult_new(void) {
    return (UA_ContentFilterElementResult*)UA_new(&UA_TYPES[UA_TYPES_CONTENTFILTERELEMENTRESULT]);
}

static UA_INLINE UA_StatusCode
UA_ContentFilterElementResult_copy(const UA_ContentFilterElementResult *src, UA_ContentFilterElementResult *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CONTENTFILTERELEMENTRESULT]);
}

static UA_INLINE void
UA_ContentFilterElementResult_deleteMembers(UA_ContentFilterElementResult *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CONTENTFILTERELEMENTRESULT]);
}

static UA_INLINE void
UA_ContentFilterElementResult_clear(UA_ContentFilterElementResult *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CONTENTFILTERELEMENTRESULT]);
}

static UA_INLINE void
UA_ContentFilterElementResult_delete(UA_ContentFilterElementResult *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_CONTENTFILTERELEMENTRESULT]);
}

/* LiteralOperand */
static UA_INLINE void
UA_LiteralOperand_init(UA_LiteralOperand *p) {
    memset(p, 0, sizeof(UA_LiteralOperand));
}

static UA_INLINE UA_LiteralOperand *
UA_LiteralOperand_new(void) {
    return (UA_LiteralOperand*)UA_new(&UA_TYPES[UA_TYPES_LITERALOPERAND]);
}

static UA_INLINE UA_StatusCode
UA_LiteralOperand_copy(const UA_LiteralOperand *src, UA_LiteralOperand *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_LITERALOPERAND]);
}

static UA_INLINE void
UA_LiteralOperand_deleteMembers(UA_LiteralOperand *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_LITERALOPERAND]);
}

static UA_INLINE void
UA_LiteralOperand_clear(UA_LiteralOperand *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_LITERALOPERAND]);
}

static UA_INLINE void
UA_LiteralOperand_delete(UA_LiteralOperand *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_LITERALOPERAND]);
}

/* MessageSecurityMode */
static UA_INLINE void
UA_MessageSecurityMode_init(UA_MessageSecurityMode *p) {
    memset(p, 0, sizeof(UA_MessageSecurityMode));
}

static UA_INLINE UA_MessageSecurityMode *
UA_MessageSecurityMode_new(void) {
    return (UA_MessageSecurityMode*)UA_new(&UA_TYPES[UA_TYPES_MESSAGESECURITYMODE]);
}

static UA_INLINE UA_StatusCode
UA_MessageSecurityMode_copy(const UA_MessageSecurityMode *src, UA_MessageSecurityMode *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_MessageSecurityMode_deleteMembers(UA_MessageSecurityMode *p) {
    memset(p, 0, sizeof(UA_MessageSecurityMode));
}

static UA_INLINE void
UA_MessageSecurityMode_clear(UA_MessageSecurityMode *p) {
    memset(p, 0, sizeof(UA_MessageSecurityMode));
}

static UA_INLINE void
UA_MessageSecurityMode_delete(UA_MessageSecurityMode *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_MESSAGESECURITYMODE]);
}

/* UtcTime */
static UA_INLINE void
UA_UtcTime_init(UA_UtcTime *p) {
    memset(p, 0, sizeof(UA_UtcTime));
}

static UA_INLINE UA_UtcTime *
UA_UtcTime_new(void) {
    return (UA_UtcTime*)UA_new(&UA_TYPES[UA_TYPES_UTCTIME]);
}

static UA_INLINE UA_StatusCode
UA_UtcTime_copy(const UA_UtcTime *src, UA_UtcTime *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_UTCTIME]);
}

static UA_INLINE void
UA_UtcTime_deleteMembers(UA_UtcTime *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_UTCTIME]);
}

static UA_INLINE void
UA_UtcTime_clear(UA_UtcTime *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_UTCTIME]);
}

static UA_INLINE void
UA_UtcTime_delete(UA_UtcTime *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_UTCTIME]);
}

/* UserIdentityToken */
static UA_INLINE void
UA_UserIdentityToken_init(UA_UserIdentityToken *p) {
    memset(p, 0, sizeof(UA_UserIdentityToken));
}

static UA_INLINE UA_UserIdentityToken *
UA_UserIdentityToken_new(void) {
    return (UA_UserIdentityToken*)UA_new(&UA_TYPES[UA_TYPES_USERIDENTITYTOKEN]);
}

static UA_INLINE UA_StatusCode
UA_UserIdentityToken_copy(const UA_UserIdentityToken *src, UA_UserIdentityToken *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_USERIDENTITYTOKEN]);
}

static UA_INLINE void
UA_UserIdentityToken_deleteMembers(UA_UserIdentityToken *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_USERIDENTITYTOKEN]);
}

static UA_INLINE void
UA_UserIdentityToken_clear(UA_UserIdentityToken *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_USERIDENTITYTOKEN]);
}

static UA_INLINE void
UA_UserIdentityToken_delete(UA_UserIdentityToken *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_USERIDENTITYTOKEN]);
}

/* X509IdentityToken */
static UA_INLINE void
UA_X509IdentityToken_init(UA_X509IdentityToken *p) {
    memset(p, 0, sizeof(UA_X509IdentityToken));
}

static UA_INLINE UA_X509IdentityToken *
UA_X509IdentityToken_new(void) {
    return (UA_X509IdentityToken*)UA_new(&UA_TYPES[UA_TYPES_X509IDENTITYTOKEN]);
}

static UA_INLINE UA_StatusCode
UA_X509IdentityToken_copy(const UA_X509IdentityToken *src, UA_X509IdentityToken *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_X509IDENTITYTOKEN]);
}

static UA_INLINE void
UA_X509IdentityToken_deleteMembers(UA_X509IdentityToken *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_X509IDENTITYTOKEN]);
}

static UA_INLINE void
UA_X509IdentityToken_clear(UA_X509IdentityToken *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_X509IDENTITYTOKEN]);
}

static UA_INLINE void
UA_X509IdentityToken_delete(UA_X509IdentityToken *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_X509IDENTITYTOKEN]);
}

/* MonitoredItemNotification */
static UA_INLINE void
UA_MonitoredItemNotification_init(UA_MonitoredItemNotification *p) {
    memset(p, 0, sizeof(UA_MonitoredItemNotification));
}

static UA_INLINE UA_MonitoredItemNotification *
UA_MonitoredItemNotification_new(void) {
    return (UA_MonitoredItemNotification*)UA_new(&UA_TYPES[UA_TYPES_MONITOREDITEMNOTIFICATION]);
}

static UA_INLINE UA_StatusCode
UA_MonitoredItemNotification_copy(const UA_MonitoredItemNotification *src, UA_MonitoredItemNotification *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_MONITOREDITEMNOTIFICATION]);
}

static UA_INLINE void
UA_MonitoredItemNotification_deleteMembers(UA_MonitoredItemNotification *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_MONITOREDITEMNOTIFICATION]);
}

static UA_INLINE void
UA_MonitoredItemNotification_clear(UA_MonitoredItemNotification *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_MONITOREDITEMNOTIFICATION]);
}

static UA_INLINE void
UA_MonitoredItemNotification_delete(UA_MonitoredItemNotification *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_MONITOREDITEMNOTIFICATION]);
}

/* ResponseHeader */
static UA_INLINE void
UA_ResponseHeader_init(UA_ResponseHeader *p) {
    memset(p, 0, sizeof(UA_ResponseHeader));
}

static UA_INLINE UA_ResponseHeader *
UA_ResponseHeader_new(void) {
    return (UA_ResponseHeader*)UA_new(&UA_TYPES[UA_TYPES_RESPONSEHEADER]);
}

static UA_INLINE UA_StatusCode
UA_ResponseHeader_copy(const UA_ResponseHeader *src, UA_ResponseHeader *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_RESPONSEHEADER]);
}

static UA_INLINE void
UA_ResponseHeader_deleteMembers(UA_ResponseHeader *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_RESPONSEHEADER]);
}

static UA_INLINE void
UA_ResponseHeader_clear(UA_ResponseHeader *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_RESPONSEHEADER]);
}

static UA_INLINE void
UA_ResponseHeader_delete(UA_ResponseHeader *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_RESPONSEHEADER]);
}

/* SignatureData */
static UA_INLINE void
UA_SignatureData_init(UA_SignatureData *p) {
    memset(p, 0, sizeof(UA_SignatureData));
}

static UA_INLINE UA_SignatureData *
UA_SignatureData_new(void) {
    return (UA_SignatureData*)UA_new(&UA_TYPES[UA_TYPES_SIGNATUREDATA]);
}

static UA_INLINE UA_StatusCode
UA_SignatureData_copy(const UA_SignatureData *src, UA_SignatureData *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_SIGNATUREDATA]);
}

static UA_INLINE void
UA_SignatureData_deleteMembers(UA_SignatureData *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SIGNATUREDATA]);
}

static UA_INLINE void
UA_SignatureData_clear(UA_SignatureData *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SIGNATUREDATA]);
}

static UA_INLINE void
UA_SignatureData_delete(UA_SignatureData *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_SIGNATUREDATA]);
}

/* ModifySubscriptionResponse */
static UA_INLINE void
UA_ModifySubscriptionResponse_init(UA_ModifySubscriptionResponse *p) {
    memset(p, 0, sizeof(UA_ModifySubscriptionResponse));
}

static UA_INLINE UA_ModifySubscriptionResponse *
UA_ModifySubscriptionResponse_new(void) {
    return (UA_ModifySubscriptionResponse*)UA_new(&UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_ModifySubscriptionResponse_copy(const UA_ModifySubscriptionResponse *src, UA_ModifySubscriptionResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONRESPONSE]);
}

static UA_INLINE void
UA_ModifySubscriptionResponse_deleteMembers(UA_ModifySubscriptionResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONRESPONSE]);
}

static UA_INLINE void
UA_ModifySubscriptionResponse_clear(UA_ModifySubscriptionResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONRESPONSE]);
}

static UA_INLINE void
UA_ModifySubscriptionResponse_delete(UA_ModifySubscriptionResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONRESPONSE]);
}

/* NodeAttributes */
static UA_INLINE void
UA_NodeAttributes_init(UA_NodeAttributes *p) {
    memset(p, 0, sizeof(UA_NodeAttributes));
}

static UA_INLINE UA_NodeAttributes *
UA_NodeAttributes_new(void) {
    return (UA_NodeAttributes*)UA_new(&UA_TYPES[UA_TYPES_NODEATTRIBUTES]);
}

static UA_INLINE UA_StatusCode
UA_NodeAttributes_copy(const UA_NodeAttributes *src, UA_NodeAttributes *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_NODEATTRIBUTES]);
}

static UA_INLINE void
UA_NodeAttributes_deleteMembers(UA_NodeAttributes *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_NODEATTRIBUTES]);
}

static UA_INLINE void
UA_NodeAttributes_clear(UA_NodeAttributes *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_NODEATTRIBUTES]);
}

static UA_INLINE void
UA_NodeAttributes_delete(UA_NodeAttributes *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_NODEATTRIBUTES]);
}

/* ActivateSessionResponse */
static UA_INLINE void
UA_ActivateSessionResponse_init(UA_ActivateSessionResponse *p) {
    memset(p, 0, sizeof(UA_ActivateSessionResponse));
}

static UA_INLINE UA_ActivateSessionResponse *
UA_ActivateSessionResponse_new(void) {
    return (UA_ActivateSessionResponse*)UA_new(&UA_TYPES[UA_TYPES_ACTIVATESESSIONRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_ActivateSessionResponse_copy(const UA_ActivateSessionResponse *src, UA_ActivateSessionResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ACTIVATESESSIONRESPONSE]);
}

static UA_INLINE void
UA_ActivateSessionResponse_deleteMembers(UA_ActivateSessionResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ACTIVATESESSIONRESPONSE]);
}

static UA_INLINE void
UA_ActivateSessionResponse_clear(UA_ActivateSessionResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ACTIVATESESSIONRESPONSE]);
}

static UA_INLINE void
UA_ActivateSessionResponse_delete(UA_ActivateSessionResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_ACTIVATESESSIONRESPONSE]);
}

/* VariableTypeAttributes */
static UA_INLINE void
UA_VariableTypeAttributes_init(UA_VariableTypeAttributes *p) {
    memset(p, 0, sizeof(UA_VariableTypeAttributes));
}

static UA_INLINE UA_VariableTypeAttributes *
UA_VariableTypeAttributes_new(void) {
    return (UA_VariableTypeAttributes*)UA_new(&UA_TYPES[UA_TYPES_VARIABLETYPEATTRIBUTES]);
}

static UA_INLINE UA_StatusCode
UA_VariableTypeAttributes_copy(const UA_VariableTypeAttributes *src, UA_VariableTypeAttributes *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_VARIABLETYPEATTRIBUTES]);
}

static UA_INLINE void
UA_VariableTypeAttributes_deleteMembers(UA_VariableTypeAttributes *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_VARIABLETYPEATTRIBUTES]);
}

static UA_INLINE void
UA_VariableTypeAttributes_clear(UA_VariableTypeAttributes *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_VARIABLETYPEATTRIBUTES]);
}

static UA_INLINE void
UA_VariableTypeAttributes_delete(UA_VariableTypeAttributes *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_VARIABLETYPEATTRIBUTES]);
}

/* CallMethodResult */
static UA_INLINE void
UA_CallMethodResult_init(UA_CallMethodResult *p) {
    memset(p, 0, sizeof(UA_CallMethodResult));
}

static UA_INLINE UA_CallMethodResult *
UA_CallMethodResult_new(void) {
    return (UA_CallMethodResult*)UA_new(&UA_TYPES[UA_TYPES_CALLMETHODRESULT]);
}

static UA_INLINE UA_StatusCode
UA_CallMethodResult_copy(const UA_CallMethodResult *src, UA_CallMethodResult *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CALLMETHODRESULT]);
}

static UA_INLINE void
UA_CallMethodResult_deleteMembers(UA_CallMethodResult *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CALLMETHODRESULT]);
}

static UA_INLINE void
UA_CallMethodResult_clear(UA_CallMethodResult *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CALLMETHODRESULT]);
}

static UA_INLINE void
UA_CallMethodResult_delete(UA_CallMethodResult *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_CALLMETHODRESULT]);
}

/* MonitoringMode */
static UA_INLINE void
UA_MonitoringMode_init(UA_MonitoringMode *p) {
    memset(p, 0, sizeof(UA_MonitoringMode));
}

static UA_INLINE UA_MonitoringMode *
UA_MonitoringMode_new(void) {
    return (UA_MonitoringMode*)UA_new(&UA_TYPES[UA_TYPES_MONITORINGMODE]);
}

static UA_INLINE UA_StatusCode
UA_MonitoringMode_copy(const UA_MonitoringMode *src, UA_MonitoringMode *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_MonitoringMode_deleteMembers(UA_MonitoringMode *p) {
    memset(p, 0, sizeof(UA_MonitoringMode));
}

static UA_INLINE void
UA_MonitoringMode_clear(UA_MonitoringMode *p) {
    memset(p, 0, sizeof(UA_MonitoringMode));
}

static UA_INLINE void
UA_MonitoringMode_delete(UA_MonitoringMode *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_MONITORINGMODE]);
}

/* SetMonitoringModeResponse */
static UA_INLINE void
UA_SetMonitoringModeResponse_init(UA_SetMonitoringModeResponse *p) {
    memset(p, 0, sizeof(UA_SetMonitoringModeResponse));
}

static UA_INLINE UA_SetMonitoringModeResponse *
UA_SetMonitoringModeResponse_new(void) {
    return (UA_SetMonitoringModeResponse*)UA_new(&UA_TYPES[UA_TYPES_SETMONITORINGMODERESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_SetMonitoringModeResponse_copy(const UA_SetMonitoringModeResponse *src, UA_SetMonitoringModeResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_SETMONITORINGMODERESPONSE]);
}

static UA_INLINE void
UA_SetMonitoringModeResponse_deleteMembers(UA_SetMonitoringModeResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SETMONITORINGMODERESPONSE]);
}

static UA_INLINE void
UA_SetMonitoringModeResponse_clear(UA_SetMonitoringModeResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SETMONITORINGMODERESPONSE]);
}

static UA_INLINE void
UA_SetMonitoringModeResponse_delete(UA_SetMonitoringModeResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_SETMONITORINGMODERESPONSE]);
}

/* BrowseResultMask */
static UA_INLINE void
UA_BrowseResultMask_init(UA_BrowseResultMask *p) {
    memset(p, 0, sizeof(UA_BrowseResultMask));
}

static UA_INLINE UA_BrowseResultMask *
UA_BrowseResultMask_new(void) {
    return (UA_BrowseResultMask*)UA_new(&UA_TYPES[UA_TYPES_BROWSERESULTMASK]);
}

static UA_INLINE UA_StatusCode
UA_BrowseResultMask_copy(const UA_BrowseResultMask *src, UA_BrowseResultMask *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_BrowseResultMask_deleteMembers(UA_BrowseResultMask *p) {
    memset(p, 0, sizeof(UA_BrowseResultMask));
}

static UA_INLINE void
UA_BrowseResultMask_clear(UA_BrowseResultMask *p) {
    memset(p, 0, sizeof(UA_BrowseResultMask));
}

static UA_INLINE void
UA_BrowseResultMask_delete(UA_BrowseResultMask *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_BROWSERESULTMASK]);
}

/* RequestHeader */
static UA_INLINE void
UA_RequestHeader_init(UA_RequestHeader *p) {
    memset(p, 0, sizeof(UA_RequestHeader));
}

static UA_INLINE UA_RequestHeader *
UA_RequestHeader_new(void) {
    return (UA_RequestHeader*)UA_new(&UA_TYPES[UA_TYPES_REQUESTHEADER]);
}

static UA_INLINE UA_StatusCode
UA_RequestHeader_copy(const UA_RequestHeader *src, UA_RequestHeader *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_REQUESTHEADER]);
}

static UA_INLINE void
UA_RequestHeader_deleteMembers(UA_RequestHeader *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REQUESTHEADER]);
}

static UA_INLINE void
UA_RequestHeader_clear(UA_RequestHeader *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REQUESTHEADER]);
}

static UA_INLINE void
UA_RequestHeader_delete(UA_RequestHeader *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_REQUESTHEADER]);
}

/* MonitoredItemModifyResult */
static UA_INLINE void
UA_MonitoredItemModifyResult_init(UA_MonitoredItemModifyResult *p) {
    memset(p, 0, sizeof(UA_MonitoredItemModifyResult));
}

static UA_INLINE UA_MonitoredItemModifyResult *
UA_MonitoredItemModifyResult_new(void) {
    return (UA_MonitoredItemModifyResult*)UA_new(&UA_TYPES[UA_TYPES_MONITOREDITEMMODIFYRESULT]);
}

static UA_INLINE UA_StatusCode
UA_MonitoredItemModifyResult_copy(const UA_MonitoredItemModifyResult *src, UA_MonitoredItemModifyResult *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_MONITOREDITEMMODIFYRESULT]);
}

static UA_INLINE void
UA_MonitoredItemModifyResult_deleteMembers(UA_MonitoredItemModifyResult *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_MONITOREDITEMMODIFYRESULT]);
}

static UA_INLINE void
UA_MonitoredItemModifyResult_clear(UA_MonitoredItemModifyResult *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_MONITOREDITEMMODIFYRESULT]);
}

static UA_INLINE void
UA_MonitoredItemModifyResult_delete(UA_MonitoredItemModifyResult *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_MONITOREDITEMMODIFYRESULT]);
}

/* CloseSecureChannelRequest */
static UA_INLINE void
UA_CloseSecureChannelRequest_init(UA_CloseSecureChannelRequest *p) {
    memset(p, 0, sizeof(UA_CloseSecureChannelRequest));
}

static UA_INLINE UA_CloseSecureChannelRequest *
UA_CloseSecureChannelRequest_new(void) {
    return (UA_CloseSecureChannelRequest*)UA_new(&UA_TYPES[UA_TYPES_CLOSESECURECHANNELREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_CloseSecureChannelRequest_copy(const UA_CloseSecureChannelRequest *src, UA_CloseSecureChannelRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CLOSESECURECHANNELREQUEST]);
}

static UA_INLINE void
UA_CloseSecureChannelRequest_deleteMembers(UA_CloseSecureChannelRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CLOSESECURECHANNELREQUEST]);
}

static UA_INLINE void
UA_CloseSecureChannelRequest_clear(UA_CloseSecureChannelRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CLOSESECURECHANNELREQUEST]);
}

static UA_INLINE void
UA_CloseSecureChannelRequest_delete(UA_CloseSecureChannelRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_CLOSESECURECHANNELREQUEST]);
}

/* NotificationMessage */
static UA_INLINE void
UA_NotificationMessage_init(UA_NotificationMessage *p) {
    memset(p, 0, sizeof(UA_NotificationMessage));
}

static UA_INLINE UA_NotificationMessage *
UA_NotificationMessage_new(void) {
    return (UA_NotificationMessage*)UA_new(&UA_TYPES[UA_TYPES_NOTIFICATIONMESSAGE]);
}

static UA_INLINE UA_StatusCode
UA_NotificationMessage_copy(const UA_NotificationMessage *src, UA_NotificationMessage *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_NOTIFICATIONMESSAGE]);
}

static UA_INLINE void
UA_NotificationMessage_deleteMembers(UA_NotificationMessage *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_NOTIFICATIONMESSAGE]);
}

static UA_INLINE void
UA_NotificationMessage_clear(UA_NotificationMessage *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_NOTIFICATIONMESSAGE]);
}

static UA_INLINE void
UA_NotificationMessage_delete(UA_NotificationMessage *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_NOTIFICATIONMESSAGE]);
}

/* CreateSubscriptionResponse */
static UA_INLINE void
UA_CreateSubscriptionResponse_init(UA_CreateSubscriptionResponse *p) {
    memset(p, 0, sizeof(UA_CreateSubscriptionResponse));
}

static UA_INLINE UA_CreateSubscriptionResponse *
UA_CreateSubscriptionResponse_new(void) {
    return (UA_CreateSubscriptionResponse*)UA_new(&UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_CreateSubscriptionResponse_copy(const UA_CreateSubscriptionResponse *src, UA_CreateSubscriptionResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONRESPONSE]);
}

static UA_INLINE void
UA_CreateSubscriptionResponse_deleteMembers(UA_CreateSubscriptionResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONRESPONSE]);
}

static UA_INLINE void
UA_CreateSubscriptionResponse_clear(UA_CreateSubscriptionResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONRESPONSE]);
}

static UA_INLINE void
UA_CreateSubscriptionResponse_delete(UA_CreateSubscriptionResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONRESPONSE]);
}

/* MdnsDiscoveryConfiguration */
static UA_INLINE void
UA_MdnsDiscoveryConfiguration_init(UA_MdnsDiscoveryConfiguration *p) {
    memset(p, 0, sizeof(UA_MdnsDiscoveryConfiguration));
}

static UA_INLINE UA_MdnsDiscoveryConfiguration *
UA_MdnsDiscoveryConfiguration_new(void) {
    return (UA_MdnsDiscoveryConfiguration*)UA_new(&UA_TYPES[UA_TYPES_MDNSDISCOVERYCONFIGURATION]);
}

static UA_INLINE UA_StatusCode
UA_MdnsDiscoveryConfiguration_copy(const UA_MdnsDiscoveryConfiguration *src, UA_MdnsDiscoveryConfiguration *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_MDNSDISCOVERYCONFIGURATION]);
}

static UA_INLINE void
UA_MdnsDiscoveryConfiguration_deleteMembers(UA_MdnsDiscoveryConfiguration *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_MDNSDISCOVERYCONFIGURATION]);
}

static UA_INLINE void
UA_MdnsDiscoveryConfiguration_clear(UA_MdnsDiscoveryConfiguration *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_MDNSDISCOVERYCONFIGURATION]);
}

static UA_INLINE void
UA_MdnsDiscoveryConfiguration_delete(UA_MdnsDiscoveryConfiguration *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_MDNSDISCOVERYCONFIGURATION]);
}

/* BrowseDirection */
static UA_INLINE void
UA_BrowseDirection_init(UA_BrowseDirection *p) {
    memset(p, 0, sizeof(UA_BrowseDirection));
}

static UA_INLINE UA_BrowseDirection *
UA_BrowseDirection_new(void) {
    return (UA_BrowseDirection*)UA_new(&UA_TYPES[UA_TYPES_BROWSEDIRECTION]);
}

static UA_INLINE UA_StatusCode
UA_BrowseDirection_copy(const UA_BrowseDirection *src, UA_BrowseDirection *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_BrowseDirection_deleteMembers(UA_BrowseDirection *p) {
    memset(p, 0, sizeof(UA_BrowseDirection));
}

static UA_INLINE void
UA_BrowseDirection_clear(UA_BrowseDirection *p) {
    memset(p, 0, sizeof(UA_BrowseDirection));
}

static UA_INLINE void
UA_BrowseDirection_delete(UA_BrowseDirection *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_BROWSEDIRECTION]);
}

/* CallMethodRequest */
static UA_INLINE void
UA_CallMethodRequest_init(UA_CallMethodRequest *p) {
    memset(p, 0, sizeof(UA_CallMethodRequest));
}

static UA_INLINE UA_CallMethodRequest *
UA_CallMethodRequest_new(void) {
    return (UA_CallMethodRequest*)UA_new(&UA_TYPES[UA_TYPES_CALLMETHODREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_CallMethodRequest_copy(const UA_CallMethodRequest *src, UA_CallMethodRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CALLMETHODREQUEST]);
}

static UA_INLINE void
UA_CallMethodRequest_deleteMembers(UA_CallMethodRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CALLMETHODREQUEST]);
}

static UA_INLINE void
UA_CallMethodRequest_clear(UA_CallMethodRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CALLMETHODREQUEST]);
}

static UA_INLINE void
UA_CallMethodRequest_delete(UA_CallMethodRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_CALLMETHODREQUEST]);
}

/* ReadResponse */
static UA_INLINE void
UA_ReadResponse_init(UA_ReadResponse *p) {
    memset(p, 0, sizeof(UA_ReadResponse));
}

static UA_INLINE UA_ReadResponse *
UA_ReadResponse_new(void) {
    return (UA_ReadResponse*)UA_new(&UA_TYPES[UA_TYPES_READRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_ReadResponse_copy(const UA_ReadResponse *src, UA_ReadResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_READRESPONSE]);
}

static UA_INLINE void
UA_ReadResponse_deleteMembers(UA_ReadResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_READRESPONSE]);
}

static UA_INLINE void
UA_ReadResponse_clear(UA_ReadResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_READRESPONSE]);
}

static UA_INLINE void
UA_ReadResponse_delete(UA_ReadResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_READRESPONSE]);
}

/* TimestampsToReturn */
static UA_INLINE void
UA_TimestampsToReturn_init(UA_TimestampsToReturn *p) {
    memset(p, 0, sizeof(UA_TimestampsToReturn));
}

static UA_INLINE UA_TimestampsToReturn *
UA_TimestampsToReturn_new(void) {
    return (UA_TimestampsToReturn*)UA_new(&UA_TYPES[UA_TYPES_TIMESTAMPSTORETURN]);
}

static UA_INLINE UA_StatusCode
UA_TimestampsToReturn_copy(const UA_TimestampsToReturn *src, UA_TimestampsToReturn *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_TimestampsToReturn_deleteMembers(UA_TimestampsToReturn *p) {
    memset(p, 0, sizeof(UA_TimestampsToReturn));
}

static UA_INLINE void
UA_TimestampsToReturn_clear(UA_TimestampsToReturn *p) {
    memset(p, 0, sizeof(UA_TimestampsToReturn));
}

static UA_INLINE void
UA_TimestampsToReturn_delete(UA_TimestampsToReturn *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_TIMESTAMPSTORETURN]);
}

/* NodeClass */
static UA_INLINE void
UA_NodeClass_init(UA_NodeClass *p) {
    memset(p, 0, sizeof(UA_NodeClass));
}

static UA_INLINE UA_NodeClass *
UA_NodeClass_new(void) {
    return (UA_NodeClass*)UA_new(&UA_TYPES[UA_TYPES_NODECLASS]);
}

static UA_INLINE UA_StatusCode
UA_NodeClass_copy(const UA_NodeClass *src, UA_NodeClass *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_NodeClass_deleteMembers(UA_NodeClass *p) {
    memset(p, 0, sizeof(UA_NodeClass));
}

static UA_INLINE void
UA_NodeClass_clear(UA_NodeClass *p) {
    memset(p, 0, sizeof(UA_NodeClass));
}

static UA_INLINE void
UA_NodeClass_delete(UA_NodeClass *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_NODECLASS]);
}

/* ObjectTypeAttributes */
static UA_INLINE void
UA_ObjectTypeAttributes_init(UA_ObjectTypeAttributes *p) {
    memset(p, 0, sizeof(UA_ObjectTypeAttributes));
}

static UA_INLINE UA_ObjectTypeAttributes *
UA_ObjectTypeAttributes_new(void) {
    return (UA_ObjectTypeAttributes*)UA_new(&UA_TYPES[UA_TYPES_OBJECTTYPEATTRIBUTES]);
}

static UA_INLINE UA_StatusCode
UA_ObjectTypeAttributes_copy(const UA_ObjectTypeAttributes *src, UA_ObjectTypeAttributes *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_OBJECTTYPEATTRIBUTES]);
}

static UA_INLINE void
UA_ObjectTypeAttributes_deleteMembers(UA_ObjectTypeAttributes *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_OBJECTTYPEATTRIBUTES]);
}

static UA_INLINE void
UA_ObjectTypeAttributes_clear(UA_ObjectTypeAttributes *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_OBJECTTYPEATTRIBUTES]);
}

static UA_INLINE void
UA_ObjectTypeAttributes_delete(UA_ObjectTypeAttributes *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_OBJECTTYPEATTRIBUTES]);
}

/* SecurityTokenRequestType */
static UA_INLINE void
UA_SecurityTokenRequestType_init(UA_SecurityTokenRequestType *p) {
    memset(p, 0, sizeof(UA_SecurityTokenRequestType));
}

static UA_INLINE UA_SecurityTokenRequestType *
UA_SecurityTokenRequestType_new(void) {
    return (UA_SecurityTokenRequestType*)UA_new(&UA_TYPES[UA_TYPES_SECURITYTOKENREQUESTTYPE]);
}

static UA_INLINE UA_StatusCode
UA_SecurityTokenRequestType_copy(const UA_SecurityTokenRequestType *src, UA_SecurityTokenRequestType *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_SecurityTokenRequestType_deleteMembers(UA_SecurityTokenRequestType *p) {
    memset(p, 0, sizeof(UA_SecurityTokenRequestType));
}

static UA_INLINE void
UA_SecurityTokenRequestType_clear(UA_SecurityTokenRequestType *p) {
    memset(p, 0, sizeof(UA_SecurityTokenRequestType));
}

static UA_INLINE void
UA_SecurityTokenRequestType_delete(UA_SecurityTokenRequestType *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_SECURITYTOKENREQUESTTYPE]);
}

/* CloseSessionResponse */
static UA_INLINE void
UA_CloseSessionResponse_init(UA_CloseSessionResponse *p) {
    memset(p, 0, sizeof(UA_CloseSessionResponse));
}

static UA_INLINE UA_CloseSessionResponse *
UA_CloseSessionResponse_new(void) {
    return (UA_CloseSessionResponse*)UA_new(&UA_TYPES[UA_TYPES_CLOSESESSIONRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_CloseSessionResponse_copy(const UA_CloseSessionResponse *src, UA_CloseSessionResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CLOSESESSIONRESPONSE]);
}

static UA_INLINE void
UA_CloseSessionResponse_deleteMembers(UA_CloseSessionResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CLOSESESSIONRESPONSE]);
}

static UA_INLINE void
UA_CloseSessionResponse_clear(UA_CloseSessionResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CLOSESESSIONRESPONSE]);
}

static UA_INLINE void
UA_CloseSessionResponse_delete(UA_CloseSessionResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_CLOSESESSIONRESPONSE]);
}

/* SetPublishingModeRequest */
static UA_INLINE void
UA_SetPublishingModeRequest_init(UA_SetPublishingModeRequest *p) {
    memset(p, 0, sizeof(UA_SetPublishingModeRequest));
}

static UA_INLINE UA_SetPublishingModeRequest *
UA_SetPublishingModeRequest_new(void) {
    return (UA_SetPublishingModeRequest*)UA_new(&UA_TYPES[UA_TYPES_SETPUBLISHINGMODEREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_SetPublishingModeRequest_copy(const UA_SetPublishingModeRequest *src, UA_SetPublishingModeRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_SETPUBLISHINGMODEREQUEST]);
}

static UA_INLINE void
UA_SetPublishingModeRequest_deleteMembers(UA_SetPublishingModeRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SETPUBLISHINGMODEREQUEST]);
}

static UA_INLINE void
UA_SetPublishingModeRequest_clear(UA_SetPublishingModeRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SETPUBLISHINGMODEREQUEST]);
}

static UA_INLINE void
UA_SetPublishingModeRequest_delete(UA_SetPublishingModeRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_SETPUBLISHINGMODEREQUEST]);
}

/* IssuedIdentityToken */
static UA_INLINE void
UA_IssuedIdentityToken_init(UA_IssuedIdentityToken *p) {
    memset(p, 0, sizeof(UA_IssuedIdentityToken));
}

static UA_INLINE UA_IssuedIdentityToken *
UA_IssuedIdentityToken_new(void) {
    return (UA_IssuedIdentityToken*)UA_new(&UA_TYPES[UA_TYPES_ISSUEDIDENTITYTOKEN]);
}

static UA_INLINE UA_StatusCode
UA_IssuedIdentityToken_copy(const UA_IssuedIdentityToken *src, UA_IssuedIdentityToken *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ISSUEDIDENTITYTOKEN]);
}

static UA_INLINE void
UA_IssuedIdentityToken_deleteMembers(UA_IssuedIdentityToken *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ISSUEDIDENTITYTOKEN]);
}

static UA_INLINE void
UA_IssuedIdentityToken_clear(UA_IssuedIdentityToken *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ISSUEDIDENTITYTOKEN]);
}

static UA_INLINE void
UA_IssuedIdentityToken_delete(UA_IssuedIdentityToken *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_ISSUEDIDENTITYTOKEN]);
}

/* ServerOnNetwork */
static UA_INLINE void
UA_ServerOnNetwork_init(UA_ServerOnNetwork *p) {
    memset(p, 0, sizeof(UA_ServerOnNetwork));
}

static UA_INLINE UA_ServerOnNetwork *
UA_ServerOnNetwork_new(void) {
    return (UA_ServerOnNetwork*)UA_new(&UA_TYPES[UA_TYPES_SERVERONNETWORK]);
}

static UA_INLINE UA_StatusCode
UA_ServerOnNetwork_copy(const UA_ServerOnNetwork *src, UA_ServerOnNetwork *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_SERVERONNETWORK]);
}

static UA_INLINE void
UA_ServerOnNetwork_deleteMembers(UA_ServerOnNetwork *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SERVERONNETWORK]);
}

static UA_INLINE void
UA_ServerOnNetwork_clear(UA_ServerOnNetwork *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SERVERONNETWORK]);
}

static UA_INLINE void
UA_ServerOnNetwork_delete(UA_ServerOnNetwork *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_SERVERONNETWORK]);
}

/* DeleteMonitoredItemsResponse */
static UA_INLINE void
UA_DeleteMonitoredItemsResponse_init(UA_DeleteMonitoredItemsResponse *p) {
    memset(p, 0, sizeof(UA_DeleteMonitoredItemsResponse));
}

static UA_INLINE UA_DeleteMonitoredItemsResponse *
UA_DeleteMonitoredItemsResponse_new(void) {
    return (UA_DeleteMonitoredItemsResponse*)UA_new(&UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_DeleteMonitoredItemsResponse_copy(const UA_DeleteMonitoredItemsResponse *src, UA_DeleteMonitoredItemsResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSRESPONSE]);
}

static UA_INLINE void
UA_DeleteMonitoredItemsResponse_deleteMembers(UA_DeleteMonitoredItemsResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSRESPONSE]);
}

static UA_INLINE void
UA_DeleteMonitoredItemsResponse_clear(UA_DeleteMonitoredItemsResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSRESPONSE]);
}

static UA_INLINE void
UA_DeleteMonitoredItemsResponse_delete(UA_DeleteMonitoredItemsResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSRESPONSE]);
}

/* ApplicationType */
static UA_INLINE void
UA_ApplicationType_init(UA_ApplicationType *p) {
    memset(p, 0, sizeof(UA_ApplicationType));
}

static UA_INLINE UA_ApplicationType *
UA_ApplicationType_new(void) {
    return (UA_ApplicationType*)UA_new(&UA_TYPES[UA_TYPES_APPLICATIONTYPE]);
}

static UA_INLINE UA_StatusCode
UA_ApplicationType_copy(const UA_ApplicationType *src, UA_ApplicationType *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_ApplicationType_deleteMembers(UA_ApplicationType *p) {
    memset(p, 0, sizeof(UA_ApplicationType));
}

static UA_INLINE void
UA_ApplicationType_clear(UA_ApplicationType *p) {
    memset(p, 0, sizeof(UA_ApplicationType));
}

static UA_INLINE void
UA_ApplicationType_delete(UA_ApplicationType *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_APPLICATIONTYPE]);
}

/* DiscoveryConfiguration */
static UA_INLINE void
UA_DiscoveryConfiguration_init(UA_DiscoveryConfiguration *p) {
    memset(p, 0, sizeof(UA_DiscoveryConfiguration));
}

static UA_INLINE UA_DiscoveryConfiguration *
UA_DiscoveryConfiguration_new(void) {
    return (UA_DiscoveryConfiguration*)UA_new(&UA_TYPES[UA_TYPES_DISCOVERYCONFIGURATION]);
}

static UA_INLINE UA_StatusCode
UA_DiscoveryConfiguration_copy(const UA_DiscoveryConfiguration *src, UA_DiscoveryConfiguration *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_DiscoveryConfiguration_deleteMembers(UA_DiscoveryConfiguration *p) {
    memset(p, 0, sizeof(UA_DiscoveryConfiguration));
}

static UA_INLINE void
UA_DiscoveryConfiguration_clear(UA_DiscoveryConfiguration *p) {
    memset(p, 0, sizeof(UA_DiscoveryConfiguration));
}

static UA_INLINE void
UA_DiscoveryConfiguration_delete(UA_DiscoveryConfiguration *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_DISCOVERYCONFIGURATION]);
}

/* BrowseNextRequest */
static UA_INLINE void
UA_BrowseNextRequest_init(UA_BrowseNextRequest *p) {
    memset(p, 0, sizeof(UA_BrowseNextRequest));
}

static UA_INLINE UA_BrowseNextRequest *
UA_BrowseNextRequest_new(void) {
    return (UA_BrowseNextRequest*)UA_new(&UA_TYPES[UA_TYPES_BROWSENEXTREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_BrowseNextRequest_copy(const UA_BrowseNextRequest *src, UA_BrowseNextRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_BROWSENEXTREQUEST]);
}

static UA_INLINE void
UA_BrowseNextRequest_deleteMembers(UA_BrowseNextRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_BROWSENEXTREQUEST]);
}

static UA_INLINE void
UA_BrowseNextRequest_clear(UA_BrowseNextRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_BROWSENEXTREQUEST]);
}

static UA_INLINE void
UA_BrowseNextRequest_delete(UA_BrowseNextRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_BROWSENEXTREQUEST]);
}

/* ModifySubscriptionRequest */
static UA_INLINE void
UA_ModifySubscriptionRequest_init(UA_ModifySubscriptionRequest *p) {
    memset(p, 0, sizeof(UA_ModifySubscriptionRequest));
}

static UA_INLINE UA_ModifySubscriptionRequest *
UA_ModifySubscriptionRequest_new(void) {
    return (UA_ModifySubscriptionRequest*)UA_new(&UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_ModifySubscriptionRequest_copy(const UA_ModifySubscriptionRequest *src, UA_ModifySubscriptionRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONREQUEST]);
}

static UA_INLINE void
UA_ModifySubscriptionRequest_deleteMembers(UA_ModifySubscriptionRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONREQUEST]);
}

static UA_INLINE void
UA_ModifySubscriptionRequest_clear(UA_ModifySubscriptionRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONREQUEST]);
}

static UA_INLINE void
UA_ModifySubscriptionRequest_delete(UA_ModifySubscriptionRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONREQUEST]);
}

/* BrowseDescription */
static UA_INLINE void
UA_BrowseDescription_init(UA_BrowseDescription *p) {
    memset(p, 0, sizeof(UA_BrowseDescription));
}

static UA_INLINE UA_BrowseDescription *
UA_BrowseDescription_new(void) {
    return (UA_BrowseDescription*)UA_new(&UA_TYPES[UA_TYPES_BROWSEDESCRIPTION]);
}

static UA_INLINE UA_StatusCode
UA_BrowseDescription_copy(const UA_BrowseDescription *src, UA_BrowseDescription *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_BROWSEDESCRIPTION]);
}

static UA_INLINE void
UA_BrowseDescription_deleteMembers(UA_BrowseDescription *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_BROWSEDESCRIPTION]);
}

static UA_INLINE void
UA_BrowseDescription_clear(UA_BrowseDescription *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_BROWSEDESCRIPTION]);
}

static UA_INLINE void
UA_BrowseDescription_delete(UA_BrowseDescription *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_BROWSEDESCRIPTION]);
}

/* SignedSoftwareCertificate */
static UA_INLINE void
UA_SignedSoftwareCertificate_init(UA_SignedSoftwareCertificate *p) {
    memset(p, 0, sizeof(UA_SignedSoftwareCertificate));
}

static UA_INLINE UA_SignedSoftwareCertificate *
UA_SignedSoftwareCertificate_new(void) {
    return (UA_SignedSoftwareCertificate*)UA_new(&UA_TYPES[UA_TYPES_SIGNEDSOFTWARECERTIFICATE]);
}

static UA_INLINE UA_StatusCode
UA_SignedSoftwareCertificate_copy(const UA_SignedSoftwareCertificate *src, UA_SignedSoftwareCertificate *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_SIGNEDSOFTWARECERTIFICATE]);
}

static UA_INLINE void
UA_SignedSoftwareCertificate_deleteMembers(UA_SignedSoftwareCertificate *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SIGNEDSOFTWARECERTIFICATE]);
}

static UA_INLINE void
UA_SignedSoftwareCertificate_clear(UA_SignedSoftwareCertificate *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SIGNEDSOFTWARECERTIFICATE]);
}

static UA_INLINE void
UA_SignedSoftwareCertificate_delete(UA_SignedSoftwareCertificate *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_SIGNEDSOFTWARECERTIFICATE]);
}

/* BrowsePathTarget */
static UA_INLINE void
UA_BrowsePathTarget_init(UA_BrowsePathTarget *p) {
    memset(p, 0, sizeof(UA_BrowsePathTarget));
}

static UA_INLINE UA_BrowsePathTarget *
UA_BrowsePathTarget_new(void) {
    return (UA_BrowsePathTarget*)UA_new(&UA_TYPES[UA_TYPES_BROWSEPATHTARGET]);
}

static UA_INLINE UA_StatusCode
UA_BrowsePathTarget_copy(const UA_BrowsePathTarget *src, UA_BrowsePathTarget *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_BROWSEPATHTARGET]);
}

static UA_INLINE void
UA_BrowsePathTarget_deleteMembers(UA_BrowsePathTarget *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_BROWSEPATHTARGET]);
}

static UA_INLINE void
UA_BrowsePathTarget_clear(UA_BrowsePathTarget *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_BROWSEPATHTARGET]);
}

static UA_INLINE void
UA_BrowsePathTarget_delete(UA_BrowsePathTarget *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_BROWSEPATHTARGET]);
}

/* WriteResponse */
static UA_INLINE void
UA_WriteResponse_init(UA_WriteResponse *p) {
    memset(p, 0, sizeof(UA_WriteResponse));
}

static UA_INLINE UA_WriteResponse *
UA_WriteResponse_new(void) {
    return (UA_WriteResponse*)UA_new(&UA_TYPES[UA_TYPES_WRITERESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_WriteResponse_copy(const UA_WriteResponse *src, UA_WriteResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_WRITERESPONSE]);
}

static UA_INLINE void
UA_WriteResponse_deleteMembers(UA_WriteResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_WRITERESPONSE]);
}

static UA_INLINE void
UA_WriteResponse_clear(UA_WriteResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_WRITERESPONSE]);
}

static UA_INLINE void
UA_WriteResponse_delete(UA_WriteResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_WRITERESPONSE]);
}

/* AddNodesResult */
static UA_INLINE void
UA_AddNodesResult_init(UA_AddNodesResult *p) {
    memset(p, 0, sizeof(UA_AddNodesResult));
}

static UA_INLINE UA_AddNodesResult *
UA_AddNodesResult_new(void) {
    return (UA_AddNodesResult*)UA_new(&UA_TYPES[UA_TYPES_ADDNODESRESULT]);
}

static UA_INLINE UA_StatusCode
UA_AddNodesResult_copy(const UA_AddNodesResult *src, UA_AddNodesResult *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ADDNODESRESULT]);
}

static UA_INLINE void
UA_AddNodesResult_deleteMembers(UA_AddNodesResult *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ADDNODESRESULT]);
}

static UA_INLINE void
UA_AddNodesResult_clear(UA_AddNodesResult *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ADDNODESRESULT]);
}

static UA_INLINE void
UA_AddNodesResult_delete(UA_AddNodesResult *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_ADDNODESRESULT]);
}

/* RegisterServerResponse */
static UA_INLINE void
UA_RegisterServerResponse_init(UA_RegisterServerResponse *p) {
    memset(p, 0, sizeof(UA_RegisterServerResponse));
}

static UA_INLINE UA_RegisterServerResponse *
UA_RegisterServerResponse_new(void) {
    return (UA_RegisterServerResponse*)UA_new(&UA_TYPES[UA_TYPES_REGISTERSERVERRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_RegisterServerResponse_copy(const UA_RegisterServerResponse *src, UA_RegisterServerResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_REGISTERSERVERRESPONSE]);
}

static UA_INLINE void
UA_RegisterServerResponse_deleteMembers(UA_RegisterServerResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REGISTERSERVERRESPONSE]);
}

static UA_INLINE void
UA_RegisterServerResponse_clear(UA_RegisterServerResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REGISTERSERVERRESPONSE]);
}

static UA_INLINE void
UA_RegisterServerResponse_delete(UA_RegisterServerResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_REGISTERSERVERRESPONSE]);
}

/* AddReferencesItem */
static UA_INLINE void
UA_AddReferencesItem_init(UA_AddReferencesItem *p) {
    memset(p, 0, sizeof(UA_AddReferencesItem));
}

static UA_INLINE UA_AddReferencesItem *
UA_AddReferencesItem_new(void) {
    return (UA_AddReferencesItem*)UA_new(&UA_TYPES[UA_TYPES_ADDREFERENCESITEM]);
}

static UA_INLINE UA_StatusCode
UA_AddReferencesItem_copy(const UA_AddReferencesItem *src, UA_AddReferencesItem *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ADDREFERENCESITEM]);
}

static UA_INLINE void
UA_AddReferencesItem_deleteMembers(UA_AddReferencesItem *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ADDREFERENCESITEM]);
}

static UA_INLINE void
UA_AddReferencesItem_clear(UA_AddReferencesItem *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ADDREFERENCESITEM]);
}

static UA_INLINE void
UA_AddReferencesItem_delete(UA_AddReferencesItem *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_ADDREFERENCESITEM]);
}

/* RegisterServer2Response */
static UA_INLINE void
UA_RegisterServer2Response_init(UA_RegisterServer2Response *p) {
    memset(p, 0, sizeof(UA_RegisterServer2Response));
}

static UA_INLINE UA_RegisterServer2Response *
UA_RegisterServer2Response_new(void) {
    return (UA_RegisterServer2Response*)UA_new(&UA_TYPES[UA_TYPES_REGISTERSERVER2RESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_RegisterServer2Response_copy(const UA_RegisterServer2Response *src, UA_RegisterServer2Response *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_REGISTERSERVER2RESPONSE]);
}

static UA_INLINE void
UA_RegisterServer2Response_deleteMembers(UA_RegisterServer2Response *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REGISTERSERVER2RESPONSE]);
}

static UA_INLINE void
UA_RegisterServer2Response_clear(UA_RegisterServer2Response *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REGISTERSERVER2RESPONSE]);
}

static UA_INLINE void
UA_RegisterServer2Response_delete(UA_RegisterServer2Response *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_REGISTERSERVER2RESPONSE]);
}

/* DeleteReferencesResponse */
static UA_INLINE void
UA_DeleteReferencesResponse_init(UA_DeleteReferencesResponse *p) {
    memset(p, 0, sizeof(UA_DeleteReferencesResponse));
}

static UA_INLINE UA_DeleteReferencesResponse *
UA_DeleteReferencesResponse_new(void) {
    return (UA_DeleteReferencesResponse*)UA_new(&UA_TYPES[UA_TYPES_DELETEREFERENCESRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_DeleteReferencesResponse_copy(const UA_DeleteReferencesResponse *src, UA_DeleteReferencesResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DELETEREFERENCESRESPONSE]);
}

static UA_INLINE void
UA_DeleteReferencesResponse_deleteMembers(UA_DeleteReferencesResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DELETEREFERENCESRESPONSE]);
}

static UA_INLINE void
UA_DeleteReferencesResponse_clear(UA_DeleteReferencesResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DELETEREFERENCESRESPONSE]);
}

static UA_INLINE void
UA_DeleteReferencesResponse_delete(UA_DeleteReferencesResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_DELETEREFERENCESRESPONSE]);
}

/* RelativePathElement */
static UA_INLINE void
UA_RelativePathElement_init(UA_RelativePathElement *p) {
    memset(p, 0, sizeof(UA_RelativePathElement));
}

static UA_INLINE UA_RelativePathElement *
UA_RelativePathElement_new(void) {
    return (UA_RelativePathElement*)UA_new(&UA_TYPES[UA_TYPES_RELATIVEPATHELEMENT]);
}

static UA_INLINE UA_StatusCode
UA_RelativePathElement_copy(const UA_RelativePathElement *src, UA_RelativePathElement *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_RELATIVEPATHELEMENT]);
}

static UA_INLINE void
UA_RelativePathElement_deleteMembers(UA_RelativePathElement *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_RELATIVEPATHELEMENT]);
}

static UA_INLINE void
UA_RelativePathElement_clear(UA_RelativePathElement *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_RELATIVEPATHELEMENT]);
}

static UA_INLINE void
UA_RelativePathElement_delete(UA_RelativePathElement *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_RELATIVEPATHELEMENT]);
}

/* SubscriptionAcknowledgement */
static UA_INLINE void
UA_SubscriptionAcknowledgement_init(UA_SubscriptionAcknowledgement *p) {
    memset(p, 0, sizeof(UA_SubscriptionAcknowledgement));
}

static UA_INLINE UA_SubscriptionAcknowledgement *
UA_SubscriptionAcknowledgement_new(void) {
    return (UA_SubscriptionAcknowledgement*)UA_new(&UA_TYPES[UA_TYPES_SUBSCRIPTIONACKNOWLEDGEMENT]);
}

static UA_INLINE UA_StatusCode
UA_SubscriptionAcknowledgement_copy(const UA_SubscriptionAcknowledgement *src, UA_SubscriptionAcknowledgement *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_SubscriptionAcknowledgement_deleteMembers(UA_SubscriptionAcknowledgement *p) {
    memset(p, 0, sizeof(UA_SubscriptionAcknowledgement));
}

static UA_INLINE void
UA_SubscriptionAcknowledgement_clear(UA_SubscriptionAcknowledgement *p) {
    memset(p, 0, sizeof(UA_SubscriptionAcknowledgement));
}

static UA_INLINE void
UA_SubscriptionAcknowledgement_delete(UA_SubscriptionAcknowledgement *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_SUBSCRIPTIONACKNOWLEDGEMENT]);
}

/* CreateMonitoredItemsResponse */
static UA_INLINE void
UA_CreateMonitoredItemsResponse_init(UA_CreateMonitoredItemsResponse *p) {
    memset(p, 0, sizeof(UA_CreateMonitoredItemsResponse));
}

static UA_INLINE UA_CreateMonitoredItemsResponse *
UA_CreateMonitoredItemsResponse_new(void) {
    return (UA_CreateMonitoredItemsResponse*)UA_new(&UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_CreateMonitoredItemsResponse_copy(const UA_CreateMonitoredItemsResponse *src, UA_CreateMonitoredItemsResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSRESPONSE]);
}

static UA_INLINE void
UA_CreateMonitoredItemsResponse_deleteMembers(UA_CreateMonitoredItemsResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSRESPONSE]);
}

static UA_INLINE void
UA_CreateMonitoredItemsResponse_clear(UA_CreateMonitoredItemsResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSRESPONSE]);
}

static UA_INLINE void
UA_CreateMonitoredItemsResponse_delete(UA_CreateMonitoredItemsResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSRESPONSE]);
}

/* DeleteReferencesItem */
static UA_INLINE void
UA_DeleteReferencesItem_init(UA_DeleteReferencesItem *p) {
    memset(p, 0, sizeof(UA_DeleteReferencesItem));
}

static UA_INLINE UA_DeleteReferencesItem *
UA_DeleteReferencesItem_new(void) {
    return (UA_DeleteReferencesItem*)UA_new(&UA_TYPES[UA_TYPES_DELETEREFERENCESITEM]);
}

static UA_INLINE UA_StatusCode
UA_DeleteReferencesItem_copy(const UA_DeleteReferencesItem *src, UA_DeleteReferencesItem *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DELETEREFERENCESITEM]);
}

static UA_INLINE void
UA_DeleteReferencesItem_deleteMembers(UA_DeleteReferencesItem *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DELETEREFERENCESITEM]);
}

static UA_INLINE void
UA_DeleteReferencesItem_clear(UA_DeleteReferencesItem *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DELETEREFERENCESITEM]);
}

static UA_INLINE void
UA_DeleteReferencesItem_delete(UA_DeleteReferencesItem *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_DELETEREFERENCESITEM]);
}

/* WriteValue */
static UA_INLINE void
UA_WriteValue_init(UA_WriteValue *p) {
    memset(p, 0, sizeof(UA_WriteValue));
}

static UA_INLINE UA_WriteValue *
UA_WriteValue_new(void) {
    return (UA_WriteValue*)UA_new(&UA_TYPES[UA_TYPES_WRITEVALUE]);
}

static UA_INLINE UA_StatusCode
UA_WriteValue_copy(const UA_WriteValue *src, UA_WriteValue *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_WRITEVALUE]);
}

static UA_INLINE void
UA_WriteValue_deleteMembers(UA_WriteValue *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_WRITEVALUE]);
}

static UA_INLINE void
UA_WriteValue_clear(UA_WriteValue *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_WRITEVALUE]);
}

static UA_INLINE void
UA_WriteValue_delete(UA_WriteValue *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_WRITEVALUE]);
}

/* DataTypeAttributes */
static UA_INLINE void
UA_DataTypeAttributes_init(UA_DataTypeAttributes *p) {
    memset(p, 0, sizeof(UA_DataTypeAttributes));
}

static UA_INLINE UA_DataTypeAttributes *
UA_DataTypeAttributes_new(void) {
    return (UA_DataTypeAttributes*)UA_new(&UA_TYPES[UA_TYPES_DATATYPEATTRIBUTES]);
}

static UA_INLINE UA_StatusCode
UA_DataTypeAttributes_copy(const UA_DataTypeAttributes *src, UA_DataTypeAttributes *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DATATYPEATTRIBUTES]);
}

static UA_INLINE void
UA_DataTypeAttributes_deleteMembers(UA_DataTypeAttributes *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DATATYPEATTRIBUTES]);
}

static UA_INLINE void
UA_DataTypeAttributes_clear(UA_DataTypeAttributes *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DATATYPEATTRIBUTES]);
}

static UA_INLINE void
UA_DataTypeAttributes_delete(UA_DataTypeAttributes *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_DATATYPEATTRIBUTES]);
}

/* AddReferencesResponse */
static UA_INLINE void
UA_AddReferencesResponse_init(UA_AddReferencesResponse *p) {
    memset(p, 0, sizeof(UA_AddReferencesResponse));
}

static UA_INLINE UA_AddReferencesResponse *
UA_AddReferencesResponse_new(void) {
    return (UA_AddReferencesResponse*)UA_new(&UA_TYPES[UA_TYPES_ADDREFERENCESRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_AddReferencesResponse_copy(const UA_AddReferencesResponse *src, UA_AddReferencesResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ADDREFERENCESRESPONSE]);
}

static UA_INLINE void
UA_AddReferencesResponse_deleteMembers(UA_AddReferencesResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ADDREFERENCESRESPONSE]);
}

static UA_INLINE void
UA_AddReferencesResponse_clear(UA_AddReferencesResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ADDREFERENCESRESPONSE]);
}

static UA_INLINE void
UA_AddReferencesResponse_delete(UA_AddReferencesResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_ADDREFERENCESRESPONSE]);
}

/* DeadbandType */
static UA_INLINE void
UA_DeadbandType_init(UA_DeadbandType *p) {
    memset(p, 0, sizeof(UA_DeadbandType));
}

static UA_INLINE UA_DeadbandType *
UA_DeadbandType_new(void) {
    return (UA_DeadbandType*)UA_new(&UA_TYPES[UA_TYPES_DEADBANDTYPE]);
}

static UA_INLINE UA_StatusCode
UA_DeadbandType_copy(const UA_DeadbandType *src, UA_DeadbandType *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_DeadbandType_deleteMembers(UA_DeadbandType *p) {
    memset(p, 0, sizeof(UA_DeadbandType));
}

static UA_INLINE void
UA_DeadbandType_clear(UA_DeadbandType *p) {
    memset(p, 0, sizeof(UA_DeadbandType));
}

static UA_INLINE void
UA_DeadbandType_delete(UA_DeadbandType *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_DEADBANDTYPE]);
}

/* DataChangeTrigger */
static UA_INLINE void
UA_DataChangeTrigger_init(UA_DataChangeTrigger *p) {
    memset(p, 0, sizeof(UA_DataChangeTrigger));
}

static UA_INLINE UA_DataChangeTrigger *
UA_DataChangeTrigger_new(void) {
    return (UA_DataChangeTrigger*)UA_new(&UA_TYPES[UA_TYPES_DATACHANGETRIGGER]);
}

static UA_INLINE UA_StatusCode
UA_DataChangeTrigger_copy(const UA_DataChangeTrigger *src, UA_DataChangeTrigger *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_DataChangeTrigger_deleteMembers(UA_DataChangeTrigger *p) {
    memset(p, 0, sizeof(UA_DataChangeTrigger));
}

static UA_INLINE void
UA_DataChangeTrigger_clear(UA_DataChangeTrigger *p) {
    memset(p, 0, sizeof(UA_DataChangeTrigger));
}

static UA_INLINE void
UA_DataChangeTrigger_delete(UA_DataChangeTrigger *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_DATACHANGETRIGGER]);
}

/* BuildInfo */
static UA_INLINE void
UA_BuildInfo_init(UA_BuildInfo *p) {
    memset(p, 0, sizeof(UA_BuildInfo));
}

static UA_INLINE UA_BuildInfo *
UA_BuildInfo_new(void) {
    return (UA_BuildInfo*)UA_new(&UA_TYPES[UA_TYPES_BUILDINFO]);
}

static UA_INLINE UA_StatusCode
UA_BuildInfo_copy(const UA_BuildInfo *src, UA_BuildInfo *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_BUILDINFO]);
}

static UA_INLINE void
UA_BuildInfo_deleteMembers(UA_BuildInfo *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_BUILDINFO]);
}

static UA_INLINE void
UA_BuildInfo_clear(UA_BuildInfo *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_BUILDINFO]);
}

static UA_INLINE void
UA_BuildInfo_delete(UA_BuildInfo *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_BUILDINFO]);
}

/* FilterOperand */
static UA_INLINE void
UA_FilterOperand_init(UA_FilterOperand *p) {
    memset(p, 0, sizeof(UA_FilterOperand));
}

static UA_INLINE UA_FilterOperand *
UA_FilterOperand_new(void) {
    return (UA_FilterOperand*)UA_new(&UA_TYPES[UA_TYPES_FILTEROPERAND]);
}

static UA_INLINE UA_StatusCode
UA_FilterOperand_copy(const UA_FilterOperand *src, UA_FilterOperand *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_FilterOperand_deleteMembers(UA_FilterOperand *p) {
    memset(p, 0, sizeof(UA_FilterOperand));
}

static UA_INLINE void
UA_FilterOperand_clear(UA_FilterOperand *p) {
    memset(p, 0, sizeof(UA_FilterOperand));
}

static UA_INLINE void
UA_FilterOperand_delete(UA_FilterOperand *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_FILTEROPERAND]);
}

/* MonitoringParameters */
static UA_INLINE void
UA_MonitoringParameters_init(UA_MonitoringParameters *p) {
    memset(p, 0, sizeof(UA_MonitoringParameters));
}

static UA_INLINE UA_MonitoringParameters *
UA_MonitoringParameters_new(void) {
    return (UA_MonitoringParameters*)UA_new(&UA_TYPES[UA_TYPES_MONITORINGPARAMETERS]);
}

static UA_INLINE UA_StatusCode
UA_MonitoringParameters_copy(const UA_MonitoringParameters *src, UA_MonitoringParameters *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_MONITORINGPARAMETERS]);
}

static UA_INLINE void
UA_MonitoringParameters_deleteMembers(UA_MonitoringParameters *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_MONITORINGPARAMETERS]);
}

static UA_INLINE void
UA_MonitoringParameters_clear(UA_MonitoringParameters *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_MONITORINGPARAMETERS]);
}

static UA_INLINE void
UA_MonitoringParameters_delete(UA_MonitoringParameters *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_MONITORINGPARAMETERS]);
}

/* DeleteNodesItem */
static UA_INLINE void
UA_DeleteNodesItem_init(UA_DeleteNodesItem *p) {
    memset(p, 0, sizeof(UA_DeleteNodesItem));
}

static UA_INLINE UA_DeleteNodesItem *
UA_DeleteNodesItem_new(void) {
    return (UA_DeleteNodesItem*)UA_new(&UA_TYPES[UA_TYPES_DELETENODESITEM]);
}

static UA_INLINE UA_StatusCode
UA_DeleteNodesItem_copy(const UA_DeleteNodesItem *src, UA_DeleteNodesItem *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DELETENODESITEM]);
}

static UA_INLINE void
UA_DeleteNodesItem_deleteMembers(UA_DeleteNodesItem *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DELETENODESITEM]);
}

static UA_INLINE void
UA_DeleteNodesItem_clear(UA_DeleteNodesItem *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DELETENODESITEM]);
}

static UA_INLINE void
UA_DeleteNodesItem_delete(UA_DeleteNodesItem *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_DELETENODESITEM]);
}

/* ReadValueId */
static UA_INLINE void
UA_ReadValueId_init(UA_ReadValueId *p) {
    memset(p, 0, sizeof(UA_ReadValueId));
}

static UA_INLINE UA_ReadValueId *
UA_ReadValueId_new(void) {
    return (UA_ReadValueId*)UA_new(&UA_TYPES[UA_TYPES_READVALUEID]);
}

static UA_INLINE UA_StatusCode
UA_ReadValueId_copy(const UA_ReadValueId *src, UA_ReadValueId *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_READVALUEID]);
}

static UA_INLINE void
UA_ReadValueId_deleteMembers(UA_ReadValueId *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_READVALUEID]);
}

static UA_INLINE void
UA_ReadValueId_clear(UA_ReadValueId *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_READVALUEID]);
}

static UA_INLINE void
UA_ReadValueId_delete(UA_ReadValueId *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_READVALUEID]);
}

/* CallRequest */
static UA_INLINE void
UA_CallRequest_init(UA_CallRequest *p) {
    memset(p, 0, sizeof(UA_CallRequest));
}

static UA_INLINE UA_CallRequest *
UA_CallRequest_new(void) {
    return (UA_CallRequest*)UA_new(&UA_TYPES[UA_TYPES_CALLREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_CallRequest_copy(const UA_CallRequest *src, UA_CallRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CALLREQUEST]);
}

static UA_INLINE void
UA_CallRequest_deleteMembers(UA_CallRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CALLREQUEST]);
}

static UA_INLINE void
UA_CallRequest_clear(UA_CallRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CALLREQUEST]);
}

static UA_INLINE void
UA_CallRequest_delete(UA_CallRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_CALLREQUEST]);
}

/* RelativePath */
static UA_INLINE void
UA_RelativePath_init(UA_RelativePath *p) {
    memset(p, 0, sizeof(UA_RelativePath));
}

static UA_INLINE UA_RelativePath *
UA_RelativePath_new(void) {
    return (UA_RelativePath*)UA_new(&UA_TYPES[UA_TYPES_RELATIVEPATH]);
}

static UA_INLINE UA_StatusCode
UA_RelativePath_copy(const UA_RelativePath *src, UA_RelativePath *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_RELATIVEPATH]);
}

static UA_INLINE void
UA_RelativePath_deleteMembers(UA_RelativePath *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_RELATIVEPATH]);
}

static UA_INLINE void
UA_RelativePath_clear(UA_RelativePath *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_RELATIVEPATH]);
}

static UA_INLINE void
UA_RelativePath_delete(UA_RelativePath *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_RELATIVEPATH]);
}

/* DeleteNodesRequest */
static UA_INLINE void
UA_DeleteNodesRequest_init(UA_DeleteNodesRequest *p) {
    memset(p, 0, sizeof(UA_DeleteNodesRequest));
}

static UA_INLINE UA_DeleteNodesRequest *
UA_DeleteNodesRequest_new(void) {
    return (UA_DeleteNodesRequest*)UA_new(&UA_TYPES[UA_TYPES_DELETENODESREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_DeleteNodesRequest_copy(const UA_DeleteNodesRequest *src, UA_DeleteNodesRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DELETENODESREQUEST]);
}

static UA_INLINE void
UA_DeleteNodesRequest_deleteMembers(UA_DeleteNodesRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DELETENODESREQUEST]);
}

static UA_INLINE void
UA_DeleteNodesRequest_clear(UA_DeleteNodesRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DELETENODESREQUEST]);
}

static UA_INLINE void
UA_DeleteNodesRequest_delete(UA_DeleteNodesRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_DELETENODESREQUEST]);
}

/* MonitoredItemModifyRequest */
static UA_INLINE void
UA_MonitoredItemModifyRequest_init(UA_MonitoredItemModifyRequest *p) {
    memset(p, 0, sizeof(UA_MonitoredItemModifyRequest));
}

static UA_INLINE UA_MonitoredItemModifyRequest *
UA_MonitoredItemModifyRequest_new(void) {
    return (UA_MonitoredItemModifyRequest*)UA_new(&UA_TYPES[UA_TYPES_MONITOREDITEMMODIFYREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_MonitoredItemModifyRequest_copy(const UA_MonitoredItemModifyRequest *src, UA_MonitoredItemModifyRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_MONITOREDITEMMODIFYREQUEST]);
}

static UA_INLINE void
UA_MonitoredItemModifyRequest_deleteMembers(UA_MonitoredItemModifyRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_MONITOREDITEMMODIFYREQUEST]);
}

static UA_INLINE void
UA_MonitoredItemModifyRequest_clear(UA_MonitoredItemModifyRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_MONITOREDITEMMODIFYREQUEST]);
}

static UA_INLINE void
UA_MonitoredItemModifyRequest_delete(UA_MonitoredItemModifyRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_MONITOREDITEMMODIFYREQUEST]);
}

/* UserTokenType */
static UA_INLINE void
UA_UserTokenType_init(UA_UserTokenType *p) {
    memset(p, 0, sizeof(UA_UserTokenType));
}

static UA_INLINE UA_UserTokenType *
UA_UserTokenType_new(void) {
    return (UA_UserTokenType*)UA_new(&UA_TYPES[UA_TYPES_USERTOKENTYPE]);
}

static UA_INLINE UA_StatusCode
UA_UserTokenType_copy(const UA_UserTokenType *src, UA_UserTokenType *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_UserTokenType_deleteMembers(UA_UserTokenType *p) {
    memset(p, 0, sizeof(UA_UserTokenType));
}

static UA_INLINE void
UA_UserTokenType_clear(UA_UserTokenType *p) {
    memset(p, 0, sizeof(UA_UserTokenType));
}

static UA_INLINE void
UA_UserTokenType_delete(UA_UserTokenType *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_USERTOKENTYPE]);
}

/* AggregateConfiguration */
static UA_INLINE void
UA_AggregateConfiguration_init(UA_AggregateConfiguration *p) {
    memset(p, 0, sizeof(UA_AggregateConfiguration));
}

static UA_INLINE UA_AggregateConfiguration *
UA_AggregateConfiguration_new(void) {
    return (UA_AggregateConfiguration*)UA_new(&UA_TYPES[UA_TYPES_AGGREGATECONFIGURATION]);
}

static UA_INLINE UA_StatusCode
UA_AggregateConfiguration_copy(const UA_AggregateConfiguration *src, UA_AggregateConfiguration *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_AggregateConfiguration_deleteMembers(UA_AggregateConfiguration *p) {
    memset(p, 0, sizeof(UA_AggregateConfiguration));
}

static UA_INLINE void
UA_AggregateConfiguration_clear(UA_AggregateConfiguration *p) {
    memset(p, 0, sizeof(UA_AggregateConfiguration));
}

static UA_INLINE void
UA_AggregateConfiguration_delete(UA_AggregateConfiguration *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_AGGREGATECONFIGURATION]);
}

/* LocaleId */
static UA_INLINE void
UA_LocaleId_init(UA_LocaleId *p) {
    memset(p, 0, sizeof(UA_LocaleId));
}

static UA_INLINE UA_LocaleId *
UA_LocaleId_new(void) {
    return (UA_LocaleId*)UA_new(&UA_TYPES[UA_TYPES_LOCALEID]);
}

static UA_INLINE UA_StatusCode
UA_LocaleId_copy(const UA_LocaleId *src, UA_LocaleId *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_LOCALEID]);
}

static UA_INLINE void
UA_LocaleId_deleteMembers(UA_LocaleId *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_LOCALEID]);
}

static UA_INLINE void
UA_LocaleId_clear(UA_LocaleId *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_LOCALEID]);
}

static UA_INLINE void
UA_LocaleId_delete(UA_LocaleId *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_LOCALEID]);
}

/* UnregisterNodesResponse */
static UA_INLINE void
UA_UnregisterNodesResponse_init(UA_UnregisterNodesResponse *p) {
    memset(p, 0, sizeof(UA_UnregisterNodesResponse));
}

static UA_INLINE UA_UnregisterNodesResponse *
UA_UnregisterNodesResponse_new(void) {
    return (UA_UnregisterNodesResponse*)UA_new(&UA_TYPES[UA_TYPES_UNREGISTERNODESRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_UnregisterNodesResponse_copy(const UA_UnregisterNodesResponse *src, UA_UnregisterNodesResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_UNREGISTERNODESRESPONSE]);
}

static UA_INLINE void
UA_UnregisterNodesResponse_deleteMembers(UA_UnregisterNodesResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_UNREGISTERNODESRESPONSE]);
}

static UA_INLINE void
UA_UnregisterNodesResponse_clear(UA_UnregisterNodesResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_UNREGISTERNODESRESPONSE]);
}

static UA_INLINE void
UA_UnregisterNodesResponse_delete(UA_UnregisterNodesResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_UNREGISTERNODESRESPONSE]);
}

/* ContentFilterResult */
static UA_INLINE void
UA_ContentFilterResult_init(UA_ContentFilterResult *p) {
    memset(p, 0, sizeof(UA_ContentFilterResult));
}

static UA_INLINE UA_ContentFilterResult *
UA_ContentFilterResult_new(void) {
    return (UA_ContentFilterResult*)UA_new(&UA_TYPES[UA_TYPES_CONTENTFILTERRESULT]);
}

static UA_INLINE UA_StatusCode
UA_ContentFilterResult_copy(const UA_ContentFilterResult *src, UA_ContentFilterResult *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CONTENTFILTERRESULT]);
}

static UA_INLINE void
UA_ContentFilterResult_deleteMembers(UA_ContentFilterResult *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CONTENTFILTERRESULT]);
}

static UA_INLINE void
UA_ContentFilterResult_clear(UA_ContentFilterResult *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CONTENTFILTERRESULT]);
}

static UA_INLINE void
UA_ContentFilterResult_delete(UA_ContentFilterResult *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_CONTENTFILTERRESULT]);
}

/* UserTokenPolicy */
static UA_INLINE void
UA_UserTokenPolicy_init(UA_UserTokenPolicy *p) {
    memset(p, 0, sizeof(UA_UserTokenPolicy));
}

static UA_INLINE UA_UserTokenPolicy *
UA_UserTokenPolicy_new(void) {
    return (UA_UserTokenPolicy*)UA_new(&UA_TYPES[UA_TYPES_USERTOKENPOLICY]);
}

static UA_INLINE UA_StatusCode
UA_UserTokenPolicy_copy(const UA_UserTokenPolicy *src, UA_UserTokenPolicy *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_USERTOKENPOLICY]);
}

static UA_INLINE void
UA_UserTokenPolicy_deleteMembers(UA_UserTokenPolicy *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_USERTOKENPOLICY]);
}

static UA_INLINE void
UA_UserTokenPolicy_clear(UA_UserTokenPolicy *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_USERTOKENPOLICY]);
}

static UA_INLINE void
UA_UserTokenPolicy_delete(UA_UserTokenPolicy *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_USERTOKENPOLICY]);
}

/* DeleteMonitoredItemsRequest */
static UA_INLINE void
UA_DeleteMonitoredItemsRequest_init(UA_DeleteMonitoredItemsRequest *p) {
    memset(p, 0, sizeof(UA_DeleteMonitoredItemsRequest));
}

static UA_INLINE UA_DeleteMonitoredItemsRequest *
UA_DeleteMonitoredItemsRequest_new(void) {
    return (UA_DeleteMonitoredItemsRequest*)UA_new(&UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_DeleteMonitoredItemsRequest_copy(const UA_DeleteMonitoredItemsRequest *src, UA_DeleteMonitoredItemsRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSREQUEST]);
}

static UA_INLINE void
UA_DeleteMonitoredItemsRequest_deleteMembers(UA_DeleteMonitoredItemsRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSREQUEST]);
}

static UA_INLINE void
UA_DeleteMonitoredItemsRequest_clear(UA_DeleteMonitoredItemsRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSREQUEST]);
}

static UA_INLINE void
UA_DeleteMonitoredItemsRequest_delete(UA_DeleteMonitoredItemsRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSREQUEST]);
}

/* SetMonitoringModeRequest */
static UA_INLINE void
UA_SetMonitoringModeRequest_init(UA_SetMonitoringModeRequest *p) {
    memset(p, 0, sizeof(UA_SetMonitoringModeRequest));
}

static UA_INLINE UA_SetMonitoringModeRequest *
UA_SetMonitoringModeRequest_new(void) {
    return (UA_SetMonitoringModeRequest*)UA_new(&UA_TYPES[UA_TYPES_SETMONITORINGMODEREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_SetMonitoringModeRequest_copy(const UA_SetMonitoringModeRequest *src, UA_SetMonitoringModeRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_SETMONITORINGMODEREQUEST]);
}

static UA_INLINE void
UA_SetMonitoringModeRequest_deleteMembers(UA_SetMonitoringModeRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SETMONITORINGMODEREQUEST]);
}

static UA_INLINE void
UA_SetMonitoringModeRequest_clear(UA_SetMonitoringModeRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SETMONITORINGMODEREQUEST]);
}

static UA_INLINE void
UA_SetMonitoringModeRequest_delete(UA_SetMonitoringModeRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_SETMONITORINGMODEREQUEST]);
}

/* Duration */
static UA_INLINE void
UA_Duration_init(UA_Duration *p) {
    memset(p, 0, sizeof(UA_Duration));
}

static UA_INLINE UA_Duration *
UA_Duration_new(void) {
    return (UA_Duration*)UA_new(&UA_TYPES[UA_TYPES_DURATION]);
}

static UA_INLINE UA_StatusCode
UA_Duration_copy(const UA_Duration *src, UA_Duration *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DURATION]);
}

static UA_INLINE void
UA_Duration_deleteMembers(UA_Duration *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DURATION]);
}

static UA_INLINE void
UA_Duration_clear(UA_Duration *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DURATION]);
}

static UA_INLINE void
UA_Duration_delete(UA_Duration *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_DURATION]);
}

/* ReferenceTypeAttributes */
static UA_INLINE void
UA_ReferenceTypeAttributes_init(UA_ReferenceTypeAttributes *p) {
    memset(p, 0, sizeof(UA_ReferenceTypeAttributes));
}

static UA_INLINE UA_ReferenceTypeAttributes *
UA_ReferenceTypeAttributes_new(void) {
    return (UA_ReferenceTypeAttributes*)UA_new(&UA_TYPES[UA_TYPES_REFERENCETYPEATTRIBUTES]);
}

static UA_INLINE UA_StatusCode
UA_ReferenceTypeAttributes_copy(const UA_ReferenceTypeAttributes *src, UA_ReferenceTypeAttributes *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_REFERENCETYPEATTRIBUTES]);
}

static UA_INLINE void
UA_ReferenceTypeAttributes_deleteMembers(UA_ReferenceTypeAttributes *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REFERENCETYPEATTRIBUTES]);
}

static UA_INLINE void
UA_ReferenceTypeAttributes_clear(UA_ReferenceTypeAttributes *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REFERENCETYPEATTRIBUTES]);
}

static UA_INLINE void
UA_ReferenceTypeAttributes_delete(UA_ReferenceTypeAttributes *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_REFERENCETYPEATTRIBUTES]);
}

/* GetEndpointsRequest */
static UA_INLINE void
UA_GetEndpointsRequest_init(UA_GetEndpointsRequest *p) {
    memset(p, 0, sizeof(UA_GetEndpointsRequest));
}

static UA_INLINE UA_GetEndpointsRequest *
UA_GetEndpointsRequest_new(void) {
    return (UA_GetEndpointsRequest*)UA_new(&UA_TYPES[UA_TYPES_GETENDPOINTSREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_GetEndpointsRequest_copy(const UA_GetEndpointsRequest *src, UA_GetEndpointsRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_GETENDPOINTSREQUEST]);
}

static UA_INLINE void
UA_GetEndpointsRequest_deleteMembers(UA_GetEndpointsRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_GETENDPOINTSREQUEST]);
}

static UA_INLINE void
UA_GetEndpointsRequest_clear(UA_GetEndpointsRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_GETENDPOINTSREQUEST]);
}

static UA_INLINE void
UA_GetEndpointsRequest_delete(UA_GetEndpointsRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_GETENDPOINTSREQUEST]);
}

/* CloseSecureChannelResponse */
static UA_INLINE void
UA_CloseSecureChannelResponse_init(UA_CloseSecureChannelResponse *p) {
    memset(p, 0, sizeof(UA_CloseSecureChannelResponse));
}

static UA_INLINE UA_CloseSecureChannelResponse *
UA_CloseSecureChannelResponse_new(void) {
    return (UA_CloseSecureChannelResponse*)UA_new(&UA_TYPES[UA_TYPES_CLOSESECURECHANNELRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_CloseSecureChannelResponse_copy(const UA_CloseSecureChannelResponse *src, UA_CloseSecureChannelResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CLOSESECURECHANNELRESPONSE]);
}

static UA_INLINE void
UA_CloseSecureChannelResponse_deleteMembers(UA_CloseSecureChannelResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CLOSESECURECHANNELRESPONSE]);
}

static UA_INLINE void
UA_CloseSecureChannelResponse_clear(UA_CloseSecureChannelResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CLOSESECURECHANNELRESPONSE]);
}

static UA_INLINE void
UA_CloseSecureChannelResponse_delete(UA_CloseSecureChannelResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_CLOSESECURECHANNELRESPONSE]);
}

/* ViewDescription */
static UA_INLINE void
UA_ViewDescription_init(UA_ViewDescription *p) {
    memset(p, 0, sizeof(UA_ViewDescription));
}

static UA_INLINE UA_ViewDescription *
UA_ViewDescription_new(void) {
    return (UA_ViewDescription*)UA_new(&UA_TYPES[UA_TYPES_VIEWDESCRIPTION]);
}

static UA_INLINE UA_StatusCode
UA_ViewDescription_copy(const UA_ViewDescription *src, UA_ViewDescription *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_VIEWDESCRIPTION]);
}

static UA_INLINE void
UA_ViewDescription_deleteMembers(UA_ViewDescription *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_VIEWDESCRIPTION]);
}

static UA_INLINE void
UA_ViewDescription_clear(UA_ViewDescription *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_VIEWDESCRIPTION]);
}

static UA_INLINE void
UA_ViewDescription_delete(UA_ViewDescription *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_VIEWDESCRIPTION]);
}

/* SetPublishingModeResponse */
static UA_INLINE void
UA_SetPublishingModeResponse_init(UA_SetPublishingModeResponse *p) {
    memset(p, 0, sizeof(UA_SetPublishingModeResponse));
}

static UA_INLINE UA_SetPublishingModeResponse *
UA_SetPublishingModeResponse_new(void) {
    return (UA_SetPublishingModeResponse*)UA_new(&UA_TYPES[UA_TYPES_SETPUBLISHINGMODERESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_SetPublishingModeResponse_copy(const UA_SetPublishingModeResponse *src, UA_SetPublishingModeResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_SETPUBLISHINGMODERESPONSE]);
}

static UA_INLINE void
UA_SetPublishingModeResponse_deleteMembers(UA_SetPublishingModeResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SETPUBLISHINGMODERESPONSE]);
}

static UA_INLINE void
UA_SetPublishingModeResponse_clear(UA_SetPublishingModeResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SETPUBLISHINGMODERESPONSE]);
}

static UA_INLINE void
UA_SetPublishingModeResponse_delete(UA_SetPublishingModeResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_SETPUBLISHINGMODERESPONSE]);
}

/* StatusChangeNotification */
static UA_INLINE void
UA_StatusChangeNotification_init(UA_StatusChangeNotification *p) {
    memset(p, 0, sizeof(UA_StatusChangeNotification));
}

static UA_INLINE UA_StatusChangeNotification *
UA_StatusChangeNotification_new(void) {
    return (UA_StatusChangeNotification*)UA_new(&UA_TYPES[UA_TYPES_STATUSCHANGENOTIFICATION]);
}

static UA_INLINE UA_StatusCode
UA_StatusChangeNotification_copy(const UA_StatusChangeNotification *src, UA_StatusChangeNotification *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_STATUSCHANGENOTIFICATION]);
}

static UA_INLINE void
UA_StatusChangeNotification_deleteMembers(UA_StatusChangeNotification *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_STATUSCHANGENOTIFICATION]);
}

static UA_INLINE void
UA_StatusChangeNotification_clear(UA_StatusChangeNotification *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_STATUSCHANGENOTIFICATION]);
}

static UA_INLINE void
UA_StatusChangeNotification_delete(UA_StatusChangeNotification *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_STATUSCHANGENOTIFICATION]);
}

/* NodeAttributesMask */
static UA_INLINE void
UA_NodeAttributesMask_init(UA_NodeAttributesMask *p) {
    memset(p, 0, sizeof(UA_NodeAttributesMask));
}

static UA_INLINE UA_NodeAttributesMask *
UA_NodeAttributesMask_new(void) {
    return (UA_NodeAttributesMask*)UA_new(&UA_TYPES[UA_TYPES_NODEATTRIBUTESMASK]);
}

static UA_INLINE UA_StatusCode
UA_NodeAttributesMask_copy(const UA_NodeAttributesMask *src, UA_NodeAttributesMask *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_NodeAttributesMask_deleteMembers(UA_NodeAttributesMask *p) {
    memset(p, 0, sizeof(UA_NodeAttributesMask));
}

static UA_INLINE void
UA_NodeAttributesMask_clear(UA_NodeAttributesMask *p) {
    memset(p, 0, sizeof(UA_NodeAttributesMask));
}

static UA_INLINE void
UA_NodeAttributesMask_delete(UA_NodeAttributesMask *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_NODEATTRIBUTESMASK]);
}

/* EventFilterResult */
static UA_INLINE void
UA_EventFilterResult_init(UA_EventFilterResult *p) {
    memset(p, 0, sizeof(UA_EventFilterResult));
}

static UA_INLINE UA_EventFilterResult *
UA_EventFilterResult_new(void) {
    return (UA_EventFilterResult*)UA_new(&UA_TYPES[UA_TYPES_EVENTFILTERRESULT]);
}

static UA_INLINE UA_StatusCode
UA_EventFilterResult_copy(const UA_EventFilterResult *src, UA_EventFilterResult *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_EVENTFILTERRESULT]);
}

static UA_INLINE void
UA_EventFilterResult_deleteMembers(UA_EventFilterResult *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_EVENTFILTERRESULT]);
}

static UA_INLINE void
UA_EventFilterResult_clear(UA_EventFilterResult *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_EVENTFILTERRESULT]);
}

static UA_INLINE void
UA_EventFilterResult_delete(UA_EventFilterResult *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_EVENTFILTERRESULT]);
}

/* MonitoredItemCreateRequest */
static UA_INLINE void
UA_MonitoredItemCreateRequest_init(UA_MonitoredItemCreateRequest *p) {
    memset(p, 0, sizeof(UA_MonitoredItemCreateRequest));
}

static UA_INLINE UA_MonitoredItemCreateRequest *
UA_MonitoredItemCreateRequest_new(void) {
    return (UA_MonitoredItemCreateRequest*)UA_new(&UA_TYPES[UA_TYPES_MONITOREDITEMCREATEREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_MonitoredItemCreateRequest_copy(const UA_MonitoredItemCreateRequest *src, UA_MonitoredItemCreateRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_MONITOREDITEMCREATEREQUEST]);
}

static UA_INLINE void
UA_MonitoredItemCreateRequest_deleteMembers(UA_MonitoredItemCreateRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_MONITOREDITEMCREATEREQUEST]);
}

static UA_INLINE void
UA_MonitoredItemCreateRequest_clear(UA_MonitoredItemCreateRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_MONITOREDITEMCREATEREQUEST]);
}

static UA_INLINE void
UA_MonitoredItemCreateRequest_delete(UA_MonitoredItemCreateRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_MONITOREDITEMCREATEREQUEST]);
}

/* Range */
static UA_INLINE void
UA_Range_init(UA_Range *p) {
    memset(p, 0, sizeof(UA_Range));
}

static UA_INLINE UA_Range *
UA_Range_new(void) {
    return (UA_Range*)UA_new(&UA_TYPES[UA_TYPES_RANGE]);
}

static UA_INLINE UA_StatusCode
UA_Range_copy(const UA_Range *src, UA_Range *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_Range_deleteMembers(UA_Range *p) {
    memset(p, 0, sizeof(UA_Range));
}

static UA_INLINE void
UA_Range_clear(UA_Range *p) {
    memset(p, 0, sizeof(UA_Range));
}

static UA_INLINE void
UA_Range_delete(UA_Range *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_RANGE]);
}

/* DataChangeNotification */
static UA_INLINE void
UA_DataChangeNotification_init(UA_DataChangeNotification *p) {
    memset(p, 0, sizeof(UA_DataChangeNotification));
}

static UA_INLINE UA_DataChangeNotification *
UA_DataChangeNotification_new(void) {
    return (UA_DataChangeNotification*)UA_new(&UA_TYPES[UA_TYPES_DATACHANGENOTIFICATION]);
}

static UA_INLINE UA_StatusCode
UA_DataChangeNotification_copy(const UA_DataChangeNotification *src, UA_DataChangeNotification *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DATACHANGENOTIFICATION]);
}

static UA_INLINE void
UA_DataChangeNotification_deleteMembers(UA_DataChangeNotification *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DATACHANGENOTIFICATION]);
}

static UA_INLINE void
UA_DataChangeNotification_clear(UA_DataChangeNotification *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DATACHANGENOTIFICATION]);
}

static UA_INLINE void
UA_DataChangeNotification_delete(UA_DataChangeNotification *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_DATACHANGENOTIFICATION]);
}

/* Argument */
static UA_INLINE void
UA_Argument_init(UA_Argument *p) {
    memset(p, 0, sizeof(UA_Argument));
}

static UA_INLINE UA_Argument *
UA_Argument_new(void) {
    return (UA_Argument*)UA_new(&UA_TYPES[UA_TYPES_ARGUMENT]);
}

static UA_INLINE UA_StatusCode
UA_Argument_copy(const UA_Argument *src, UA_Argument *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ARGUMENT]);
}

static UA_INLINE void
UA_Argument_deleteMembers(UA_Argument *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ARGUMENT]);
}

static UA_INLINE void
UA_Argument_clear(UA_Argument *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ARGUMENT]);
}

static UA_INLINE void
UA_Argument_delete(UA_Argument *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_ARGUMENT]);
}

/* ChannelSecurityToken */
static UA_INLINE void
UA_ChannelSecurityToken_init(UA_ChannelSecurityToken *p) {
    memset(p, 0, sizeof(UA_ChannelSecurityToken));
}

static UA_INLINE UA_ChannelSecurityToken *
UA_ChannelSecurityToken_new(void) {
    return (UA_ChannelSecurityToken*)UA_new(&UA_TYPES[UA_TYPES_CHANNELSECURITYTOKEN]);
}

static UA_INLINE UA_StatusCode
UA_ChannelSecurityToken_copy(const UA_ChannelSecurityToken *src, UA_ChannelSecurityToken *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_ChannelSecurityToken_deleteMembers(UA_ChannelSecurityToken *p) {
    memset(p, 0, sizeof(UA_ChannelSecurityToken));
}

static UA_INLINE void
UA_ChannelSecurityToken_clear(UA_ChannelSecurityToken *p) {
    memset(p, 0, sizeof(UA_ChannelSecurityToken));
}

static UA_INLINE void
UA_ChannelSecurityToken_delete(UA_ChannelSecurityToken *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_CHANNELSECURITYTOKEN]);
}

/* ServerState */
static UA_INLINE void
UA_ServerState_init(UA_ServerState *p) {
    memset(p, 0, sizeof(UA_ServerState));
}

static UA_INLINE UA_ServerState *
UA_ServerState_new(void) {
    return (UA_ServerState*)UA_new(&UA_TYPES[UA_TYPES_SERVERSTATE]);
}

static UA_INLINE UA_StatusCode
UA_ServerState_copy(const UA_ServerState *src, UA_ServerState *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_ServerState_deleteMembers(UA_ServerState *p) {
    memset(p, 0, sizeof(UA_ServerState));
}

static UA_INLINE void
UA_ServerState_clear(UA_ServerState *p) {
    memset(p, 0, sizeof(UA_ServerState));
}

static UA_INLINE void
UA_ServerState_delete(UA_ServerState *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_SERVERSTATE]);
}

/* EventNotificationList */
static UA_INLINE void
UA_EventNotificationList_init(UA_EventNotificationList *p) {
    memset(p, 0, sizeof(UA_EventNotificationList));
}

static UA_INLINE UA_EventNotificationList *
UA_EventNotificationList_new(void) {
    return (UA_EventNotificationList*)UA_new(&UA_TYPES[UA_TYPES_EVENTNOTIFICATIONLIST]);
}

static UA_INLINE UA_StatusCode
UA_EventNotificationList_copy(const UA_EventNotificationList *src, UA_EventNotificationList *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_EVENTNOTIFICATIONLIST]);
}

static UA_INLINE void
UA_EventNotificationList_deleteMembers(UA_EventNotificationList *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_EVENTNOTIFICATIONLIST]);
}

static UA_INLINE void
UA_EventNotificationList_clear(UA_EventNotificationList *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_EVENTNOTIFICATIONLIST]);
}

static UA_INLINE void
UA_EventNotificationList_delete(UA_EventNotificationList *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_EVENTNOTIFICATIONLIST]);
}

/* AnonymousIdentityToken */
static UA_INLINE void
UA_AnonymousIdentityToken_init(UA_AnonymousIdentityToken *p) {
    memset(p, 0, sizeof(UA_AnonymousIdentityToken));
}

static UA_INLINE UA_AnonymousIdentityToken *
UA_AnonymousIdentityToken_new(void) {
    return (UA_AnonymousIdentityToken*)UA_new(&UA_TYPES[UA_TYPES_ANONYMOUSIDENTITYTOKEN]);
}

static UA_INLINE UA_StatusCode
UA_AnonymousIdentityToken_copy(const UA_AnonymousIdentityToken *src, UA_AnonymousIdentityToken *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ANONYMOUSIDENTITYTOKEN]);
}

static UA_INLINE void
UA_AnonymousIdentityToken_deleteMembers(UA_AnonymousIdentityToken *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ANONYMOUSIDENTITYTOKEN]);
}

static UA_INLINE void
UA_AnonymousIdentityToken_clear(UA_AnonymousIdentityToken *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ANONYMOUSIDENTITYTOKEN]);
}

static UA_INLINE void
UA_AnonymousIdentityToken_delete(UA_AnonymousIdentityToken *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_ANONYMOUSIDENTITYTOKEN]);
}

/* FilterOperator */
static UA_INLINE void
UA_FilterOperator_init(UA_FilterOperator *p) {
    memset(p, 0, sizeof(UA_FilterOperator));
}

static UA_INLINE UA_FilterOperator *
UA_FilterOperator_new(void) {
    return (UA_FilterOperator*)UA_new(&UA_TYPES[UA_TYPES_FILTEROPERATOR]);
}

static UA_INLINE UA_StatusCode
UA_FilterOperator_copy(const UA_FilterOperator *src, UA_FilterOperator *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_FilterOperator_deleteMembers(UA_FilterOperator *p) {
    memset(p, 0, sizeof(UA_FilterOperator));
}

static UA_INLINE void
UA_FilterOperator_clear(UA_FilterOperator *p) {
    memset(p, 0, sizeof(UA_FilterOperator));
}

static UA_INLINE void
UA_FilterOperator_delete(UA_FilterOperator *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_FILTEROPERATOR]);
}

/* AggregateFilter */
static UA_INLINE void
UA_AggregateFilter_init(UA_AggregateFilter *p) {
    memset(p, 0, sizeof(UA_AggregateFilter));
}

static UA_INLINE UA_AggregateFilter *
UA_AggregateFilter_new(void) {
    return (UA_AggregateFilter*)UA_new(&UA_TYPES[UA_TYPES_AGGREGATEFILTER]);
}

static UA_INLINE UA_StatusCode
UA_AggregateFilter_copy(const UA_AggregateFilter *src, UA_AggregateFilter *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_AGGREGATEFILTER]);
}

static UA_INLINE void
UA_AggregateFilter_deleteMembers(UA_AggregateFilter *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_AGGREGATEFILTER]);
}

static UA_INLINE void
UA_AggregateFilter_clear(UA_AggregateFilter *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_AGGREGATEFILTER]);
}

static UA_INLINE void
UA_AggregateFilter_delete(UA_AggregateFilter *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_AGGREGATEFILTER]);
}

/* RepublishResponse */
static UA_INLINE void
UA_RepublishResponse_init(UA_RepublishResponse *p) {
    memset(p, 0, sizeof(UA_RepublishResponse));
}

static UA_INLINE UA_RepublishResponse *
UA_RepublishResponse_new(void) {
    return (UA_RepublishResponse*)UA_new(&UA_TYPES[UA_TYPES_REPUBLISHRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_RepublishResponse_copy(const UA_RepublishResponse *src, UA_RepublishResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_REPUBLISHRESPONSE]);
}

static UA_INLINE void
UA_RepublishResponse_deleteMembers(UA_RepublishResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REPUBLISHRESPONSE]);
}

static UA_INLINE void
UA_RepublishResponse_clear(UA_RepublishResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REPUBLISHRESPONSE]);
}

static UA_INLINE void
UA_RepublishResponse_delete(UA_RepublishResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_REPUBLISHRESPONSE]);
}

/* DeleteSubscriptionsResponse */
static UA_INLINE void
UA_DeleteSubscriptionsResponse_init(UA_DeleteSubscriptionsResponse *p) {
    memset(p, 0, sizeof(UA_DeleteSubscriptionsResponse));
}

static UA_INLINE UA_DeleteSubscriptionsResponse *
UA_DeleteSubscriptionsResponse_new(void) {
    return (UA_DeleteSubscriptionsResponse*)UA_new(&UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_DeleteSubscriptionsResponse_copy(const UA_DeleteSubscriptionsResponse *src, UA_DeleteSubscriptionsResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSRESPONSE]);
}

static UA_INLINE void
UA_DeleteSubscriptionsResponse_deleteMembers(UA_DeleteSubscriptionsResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSRESPONSE]);
}

static UA_INLINE void
UA_DeleteSubscriptionsResponse_clear(UA_DeleteSubscriptionsResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSRESPONSE]);
}

static UA_INLINE void
UA_DeleteSubscriptionsResponse_delete(UA_DeleteSubscriptionsResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSRESPONSE]);
}

/* RegisterNodesRequest */
static UA_INLINE void
UA_RegisterNodesRequest_init(UA_RegisterNodesRequest *p) {
    memset(p, 0, sizeof(UA_RegisterNodesRequest));
}

static UA_INLINE UA_RegisterNodesRequest *
UA_RegisterNodesRequest_new(void) {
    return (UA_RegisterNodesRequest*)UA_new(&UA_TYPES[UA_TYPES_REGISTERNODESREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_RegisterNodesRequest_copy(const UA_RegisterNodesRequest *src, UA_RegisterNodesRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_REGISTERNODESREQUEST]);
}

static UA_INLINE void
UA_RegisterNodesRequest_deleteMembers(UA_RegisterNodesRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REGISTERNODESREQUEST]);
}

static UA_INLINE void
UA_RegisterNodesRequest_clear(UA_RegisterNodesRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REGISTERNODESREQUEST]);
}

static UA_INLINE void
UA_RegisterNodesRequest_delete(UA_RegisterNodesRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_REGISTERNODESREQUEST]);
}

/* MethodAttributes */
static UA_INLINE void
UA_MethodAttributes_init(UA_MethodAttributes *p) {
    memset(p, 0, sizeof(UA_MethodAttributes));
}

static UA_INLINE UA_MethodAttributes *
UA_MethodAttributes_new(void) {
    return (UA_MethodAttributes*)UA_new(&UA_TYPES[UA_TYPES_METHODATTRIBUTES]);
}

static UA_INLINE UA_StatusCode
UA_MethodAttributes_copy(const UA_MethodAttributes *src, UA_MethodAttributes *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_METHODATTRIBUTES]);
}

static UA_INLINE void
UA_MethodAttributes_deleteMembers(UA_MethodAttributes *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_METHODATTRIBUTES]);
}

static UA_INLINE void
UA_MethodAttributes_clear(UA_MethodAttributes *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_METHODATTRIBUTES]);
}

static UA_INLINE void
UA_MethodAttributes_delete(UA_MethodAttributes *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_METHODATTRIBUTES]);
}

/* UserNameIdentityToken */
static UA_INLINE void
UA_UserNameIdentityToken_init(UA_UserNameIdentityToken *p) {
    memset(p, 0, sizeof(UA_UserNameIdentityToken));
}

static UA_INLINE UA_UserNameIdentityToken *
UA_UserNameIdentityToken_new(void) {
    return (UA_UserNameIdentityToken*)UA_new(&UA_TYPES[UA_TYPES_USERNAMEIDENTITYTOKEN]);
}

static UA_INLINE UA_StatusCode
UA_UserNameIdentityToken_copy(const UA_UserNameIdentityToken *src, UA_UserNameIdentityToken *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_USERNAMEIDENTITYTOKEN]);
}

static UA_INLINE void
UA_UserNameIdentityToken_deleteMembers(UA_UserNameIdentityToken *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_USERNAMEIDENTITYTOKEN]);
}

static UA_INLINE void
UA_UserNameIdentityToken_clear(UA_UserNameIdentityToken *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_USERNAMEIDENTITYTOKEN]);
}

static UA_INLINE void
UA_UserNameIdentityToken_delete(UA_UserNameIdentityToken *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_USERNAMEIDENTITYTOKEN]);
}

/* UnregisterNodesRequest */
static UA_INLINE void
UA_UnregisterNodesRequest_init(UA_UnregisterNodesRequest *p) {
    memset(p, 0, sizeof(UA_UnregisterNodesRequest));
}

static UA_INLINE UA_UnregisterNodesRequest *
UA_UnregisterNodesRequest_new(void) {
    return (UA_UnregisterNodesRequest*)UA_new(&UA_TYPES[UA_TYPES_UNREGISTERNODESREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_UnregisterNodesRequest_copy(const UA_UnregisterNodesRequest *src, UA_UnregisterNodesRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_UNREGISTERNODESREQUEST]);
}

static UA_INLINE void
UA_UnregisterNodesRequest_deleteMembers(UA_UnregisterNodesRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_UNREGISTERNODESREQUEST]);
}

static UA_INLINE void
UA_UnregisterNodesRequest_clear(UA_UnregisterNodesRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_UNREGISTERNODESREQUEST]);
}

static UA_INLINE void
UA_UnregisterNodesRequest_delete(UA_UnregisterNodesRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_UNREGISTERNODESREQUEST]);
}

/* OpenSecureChannelResponse */
static UA_INLINE void
UA_OpenSecureChannelResponse_init(UA_OpenSecureChannelResponse *p) {
    memset(p, 0, sizeof(UA_OpenSecureChannelResponse));
}

static UA_INLINE UA_OpenSecureChannelResponse *
UA_OpenSecureChannelResponse_new(void) {
    return (UA_OpenSecureChannelResponse*)UA_new(&UA_TYPES[UA_TYPES_OPENSECURECHANNELRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_OpenSecureChannelResponse_copy(const UA_OpenSecureChannelResponse *src, UA_OpenSecureChannelResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_OPENSECURECHANNELRESPONSE]);
}

static UA_INLINE void
UA_OpenSecureChannelResponse_deleteMembers(UA_OpenSecureChannelResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_OPENSECURECHANNELRESPONSE]);
}

static UA_INLINE void
UA_OpenSecureChannelResponse_clear(UA_OpenSecureChannelResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_OPENSECURECHANNELRESPONSE]);
}

static UA_INLINE void
UA_OpenSecureChannelResponse_delete(UA_OpenSecureChannelResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_OPENSECURECHANNELRESPONSE]);
}

/* SetTriggeringResponse */
static UA_INLINE void
UA_SetTriggeringResponse_init(UA_SetTriggeringResponse *p) {
    memset(p, 0, sizeof(UA_SetTriggeringResponse));
}

static UA_INLINE UA_SetTriggeringResponse *
UA_SetTriggeringResponse_new(void) {
    return (UA_SetTriggeringResponse*)UA_new(&UA_TYPES[UA_TYPES_SETTRIGGERINGRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_SetTriggeringResponse_copy(const UA_SetTriggeringResponse *src, UA_SetTriggeringResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_SETTRIGGERINGRESPONSE]);
}

static UA_INLINE void
UA_SetTriggeringResponse_deleteMembers(UA_SetTriggeringResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SETTRIGGERINGRESPONSE]);
}

static UA_INLINE void
UA_SetTriggeringResponse_clear(UA_SetTriggeringResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SETTRIGGERINGRESPONSE]);
}

static UA_INLINE void
UA_SetTriggeringResponse_delete(UA_SetTriggeringResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_SETTRIGGERINGRESPONSE]);
}

/* SimpleAttributeOperand */
static UA_INLINE void
UA_SimpleAttributeOperand_init(UA_SimpleAttributeOperand *p) {
    memset(p, 0, sizeof(UA_SimpleAttributeOperand));
}

static UA_INLINE UA_SimpleAttributeOperand *
UA_SimpleAttributeOperand_new(void) {
    return (UA_SimpleAttributeOperand*)UA_new(&UA_TYPES[UA_TYPES_SIMPLEATTRIBUTEOPERAND]);
}

static UA_INLINE UA_StatusCode
UA_SimpleAttributeOperand_copy(const UA_SimpleAttributeOperand *src, UA_SimpleAttributeOperand *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_SIMPLEATTRIBUTEOPERAND]);
}

static UA_INLINE void
UA_SimpleAttributeOperand_deleteMembers(UA_SimpleAttributeOperand *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SIMPLEATTRIBUTEOPERAND]);
}

static UA_INLINE void
UA_SimpleAttributeOperand_clear(UA_SimpleAttributeOperand *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SIMPLEATTRIBUTEOPERAND]);
}

static UA_INLINE void
UA_SimpleAttributeOperand_delete(UA_SimpleAttributeOperand *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_SIMPLEATTRIBUTEOPERAND]);
}

/* RepublishRequest */
static UA_INLINE void
UA_RepublishRequest_init(UA_RepublishRequest *p) {
    memset(p, 0, sizeof(UA_RepublishRequest));
}

static UA_INLINE UA_RepublishRequest *
UA_RepublishRequest_new(void) {
    return (UA_RepublishRequest*)UA_new(&UA_TYPES[UA_TYPES_REPUBLISHREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_RepublishRequest_copy(const UA_RepublishRequest *src, UA_RepublishRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_REPUBLISHREQUEST]);
}

static UA_INLINE void
UA_RepublishRequest_deleteMembers(UA_RepublishRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REPUBLISHREQUEST]);
}

static UA_INLINE void
UA_RepublishRequest_clear(UA_RepublishRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REPUBLISHREQUEST]);
}

static UA_INLINE void
UA_RepublishRequest_delete(UA_RepublishRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_REPUBLISHREQUEST]);
}

/* RegisterNodesResponse */
static UA_INLINE void
UA_RegisterNodesResponse_init(UA_RegisterNodesResponse *p) {
    memset(p, 0, sizeof(UA_RegisterNodesResponse));
}

static UA_INLINE UA_RegisterNodesResponse *
UA_RegisterNodesResponse_new(void) {
    return (UA_RegisterNodesResponse*)UA_new(&UA_TYPES[UA_TYPES_REGISTERNODESRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_RegisterNodesResponse_copy(const UA_RegisterNodesResponse *src, UA_RegisterNodesResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_REGISTERNODESRESPONSE]);
}

static UA_INLINE void
UA_RegisterNodesResponse_deleteMembers(UA_RegisterNodesResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REGISTERNODESRESPONSE]);
}

static UA_INLINE void
UA_RegisterNodesResponse_clear(UA_RegisterNodesResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REGISTERNODESRESPONSE]);
}

static UA_INLINE void
UA_RegisterNodesResponse_delete(UA_RegisterNodesResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_REGISTERNODESRESPONSE]);
}

/* ModifyMonitoredItemsResponse */
static UA_INLINE void
UA_ModifyMonitoredItemsResponse_init(UA_ModifyMonitoredItemsResponse *p) {
    memset(p, 0, sizeof(UA_ModifyMonitoredItemsResponse));
}

static UA_INLINE UA_ModifyMonitoredItemsResponse *
UA_ModifyMonitoredItemsResponse_new(void) {
    return (UA_ModifyMonitoredItemsResponse*)UA_new(&UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_ModifyMonitoredItemsResponse_copy(const UA_ModifyMonitoredItemsResponse *src, UA_ModifyMonitoredItemsResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSRESPONSE]);
}

static UA_INLINE void
UA_ModifyMonitoredItemsResponse_deleteMembers(UA_ModifyMonitoredItemsResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSRESPONSE]);
}

static UA_INLINE void
UA_ModifyMonitoredItemsResponse_clear(UA_ModifyMonitoredItemsResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSRESPONSE]);
}

static UA_INLINE void
UA_ModifyMonitoredItemsResponse_delete(UA_ModifyMonitoredItemsResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSRESPONSE]);
}

/* DeleteSubscriptionsRequest */
static UA_INLINE void
UA_DeleteSubscriptionsRequest_init(UA_DeleteSubscriptionsRequest *p) {
    memset(p, 0, sizeof(UA_DeleteSubscriptionsRequest));
}

static UA_INLINE UA_DeleteSubscriptionsRequest *
UA_DeleteSubscriptionsRequest_new(void) {
    return (UA_DeleteSubscriptionsRequest*)UA_new(&UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_DeleteSubscriptionsRequest_copy(const UA_DeleteSubscriptionsRequest *src, UA_DeleteSubscriptionsRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSREQUEST]);
}

static UA_INLINE void
UA_DeleteSubscriptionsRequest_deleteMembers(UA_DeleteSubscriptionsRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSREQUEST]);
}

static UA_INLINE void
UA_DeleteSubscriptionsRequest_clear(UA_DeleteSubscriptionsRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSREQUEST]);
}

static UA_INLINE void
UA_DeleteSubscriptionsRequest_delete(UA_DeleteSubscriptionsRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSREQUEST]);
}

/* RedundancySupport */
static UA_INLINE void
UA_RedundancySupport_init(UA_RedundancySupport *p) {
    memset(p, 0, sizeof(UA_RedundancySupport));
}

static UA_INLINE UA_RedundancySupport *
UA_RedundancySupport_new(void) {
    return (UA_RedundancySupport*)UA_new(&UA_TYPES[UA_TYPES_REDUNDANCYSUPPORT]);
}

static UA_INLINE UA_StatusCode
UA_RedundancySupport_copy(const UA_RedundancySupport *src, UA_RedundancySupport *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_RedundancySupport_deleteMembers(UA_RedundancySupport *p) {
    memset(p, 0, sizeof(UA_RedundancySupport));
}

static UA_INLINE void
UA_RedundancySupport_clear(UA_RedundancySupport *p) {
    memset(p, 0, sizeof(UA_RedundancySupport));
}

static UA_INLINE void
UA_RedundancySupport_delete(UA_RedundancySupport *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_REDUNDANCYSUPPORT]);
}

/* BrowsePath */
static UA_INLINE void
UA_BrowsePath_init(UA_BrowsePath *p) {
    memset(p, 0, sizeof(UA_BrowsePath));
}

static UA_INLINE UA_BrowsePath *
UA_BrowsePath_new(void) {
    return (UA_BrowsePath*)UA_new(&UA_TYPES[UA_TYPES_BROWSEPATH]);
}

static UA_INLINE UA_StatusCode
UA_BrowsePath_copy(const UA_BrowsePath *src, UA_BrowsePath *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_BROWSEPATH]);
}

static UA_INLINE void
UA_BrowsePath_deleteMembers(UA_BrowsePath *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_BROWSEPATH]);
}

static UA_INLINE void
UA_BrowsePath_clear(UA_BrowsePath *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_BROWSEPATH]);
}

static UA_INLINE void
UA_BrowsePath_delete(UA_BrowsePath *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_BROWSEPATH]);
}

/* ObjectAttributes */
static UA_INLINE void
UA_ObjectAttributes_init(UA_ObjectAttributes *p) {
    memset(p, 0, sizeof(UA_ObjectAttributes));
}

static UA_INLINE UA_ObjectAttributes *
UA_ObjectAttributes_new(void) {
    return (UA_ObjectAttributes*)UA_new(&UA_TYPES[UA_TYPES_OBJECTATTRIBUTES]);
}

static UA_INLINE UA_StatusCode
UA_ObjectAttributes_copy(const UA_ObjectAttributes *src, UA_ObjectAttributes *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES]);
}

static UA_INLINE void
UA_ObjectAttributes_deleteMembers(UA_ObjectAttributes *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES]);
}

static UA_INLINE void
UA_ObjectAttributes_clear(UA_ObjectAttributes *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES]);
}

static UA_INLINE void
UA_ObjectAttributes_delete(UA_ObjectAttributes *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES]);
}

/* PublishRequest */
static UA_INLINE void
UA_PublishRequest_init(UA_PublishRequest *p) {
    memset(p, 0, sizeof(UA_PublishRequest));
}

static UA_INLINE UA_PublishRequest *
UA_PublishRequest_new(void) {
    return (UA_PublishRequest*)UA_new(&UA_TYPES[UA_TYPES_PUBLISHREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_PublishRequest_copy(const UA_PublishRequest *src, UA_PublishRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_PUBLISHREQUEST]);
}

static UA_INLINE void
UA_PublishRequest_deleteMembers(UA_PublishRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_PUBLISHREQUEST]);
}

static UA_INLINE void
UA_PublishRequest_clear(UA_PublishRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_PUBLISHREQUEST]);
}

static UA_INLINE void
UA_PublishRequest_delete(UA_PublishRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_PUBLISHREQUEST]);
}

/* FindServersRequest */
static UA_INLINE void
UA_FindServersRequest_init(UA_FindServersRequest *p) {
    memset(p, 0, sizeof(UA_FindServersRequest));
}

static UA_INLINE UA_FindServersRequest *
UA_FindServersRequest_new(void) {
    return (UA_FindServersRequest*)UA_new(&UA_TYPES[UA_TYPES_FINDSERVERSREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_FindServersRequest_copy(const UA_FindServersRequest *src, UA_FindServersRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_FINDSERVERSREQUEST]);
}

static UA_INLINE void
UA_FindServersRequest_deleteMembers(UA_FindServersRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_FINDSERVERSREQUEST]);
}

static UA_INLINE void
UA_FindServersRequest_clear(UA_FindServersRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_FINDSERVERSREQUEST]);
}

static UA_INLINE void
UA_FindServersRequest_delete(UA_FindServersRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_FINDSERVERSREQUEST]);
}

/* FindServersOnNetworkResponse */
static UA_INLINE void
UA_FindServersOnNetworkResponse_init(UA_FindServersOnNetworkResponse *p) {
    memset(p, 0, sizeof(UA_FindServersOnNetworkResponse));
}

static UA_INLINE UA_FindServersOnNetworkResponse *
UA_FindServersOnNetworkResponse_new(void) {
    return (UA_FindServersOnNetworkResponse*)UA_new(&UA_TYPES[UA_TYPES_FINDSERVERSONNETWORKRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_FindServersOnNetworkResponse_copy(const UA_FindServersOnNetworkResponse *src, UA_FindServersOnNetworkResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_FINDSERVERSONNETWORKRESPONSE]);
}

static UA_INLINE void
UA_FindServersOnNetworkResponse_deleteMembers(UA_FindServersOnNetworkResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_FINDSERVERSONNETWORKRESPONSE]);
}

static UA_INLINE void
UA_FindServersOnNetworkResponse_clear(UA_FindServersOnNetworkResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_FINDSERVERSONNETWORKRESPONSE]);
}

static UA_INLINE void
UA_FindServersOnNetworkResponse_delete(UA_FindServersOnNetworkResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_FINDSERVERSONNETWORKRESPONSE]);
}

/* ReferenceDescription */
static UA_INLINE void
UA_ReferenceDescription_init(UA_ReferenceDescription *p) {
    memset(p, 0, sizeof(UA_ReferenceDescription));
}

static UA_INLINE UA_ReferenceDescription *
UA_ReferenceDescription_new(void) {
    return (UA_ReferenceDescription*)UA_new(&UA_TYPES[UA_TYPES_REFERENCEDESCRIPTION]);
}

static UA_INLINE UA_StatusCode
UA_ReferenceDescription_copy(const UA_ReferenceDescription *src, UA_ReferenceDescription *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_REFERENCEDESCRIPTION]);
}

static UA_INLINE void
UA_ReferenceDescription_deleteMembers(UA_ReferenceDescription *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REFERENCEDESCRIPTION]);
}

static UA_INLINE void
UA_ReferenceDescription_clear(UA_ReferenceDescription *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REFERENCEDESCRIPTION]);
}

static UA_INLINE void
UA_ReferenceDescription_delete(UA_ReferenceDescription *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_REFERENCEDESCRIPTION]);
}

/* CreateSubscriptionRequest */
static UA_INLINE void
UA_CreateSubscriptionRequest_init(UA_CreateSubscriptionRequest *p) {
    memset(p, 0, sizeof(UA_CreateSubscriptionRequest));
}

static UA_INLINE UA_CreateSubscriptionRequest *
UA_CreateSubscriptionRequest_new(void) {
    return (UA_CreateSubscriptionRequest*)UA_new(&UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_CreateSubscriptionRequest_copy(const UA_CreateSubscriptionRequest *src, UA_CreateSubscriptionRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONREQUEST]);
}

static UA_INLINE void
UA_CreateSubscriptionRequest_deleteMembers(UA_CreateSubscriptionRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONREQUEST]);
}

static UA_INLINE void
UA_CreateSubscriptionRequest_clear(UA_CreateSubscriptionRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONREQUEST]);
}

static UA_INLINE void
UA_CreateSubscriptionRequest_delete(UA_CreateSubscriptionRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONREQUEST]);
}

/* FindServersOnNetworkRequest */
static UA_INLINE void
UA_FindServersOnNetworkRequest_init(UA_FindServersOnNetworkRequest *p) {
    memset(p, 0, sizeof(UA_FindServersOnNetworkRequest));
}

static UA_INLINE UA_FindServersOnNetworkRequest *
UA_FindServersOnNetworkRequest_new(void) {
    return (UA_FindServersOnNetworkRequest*)UA_new(&UA_TYPES[UA_TYPES_FINDSERVERSONNETWORKREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_FindServersOnNetworkRequest_copy(const UA_FindServersOnNetworkRequest *src, UA_FindServersOnNetworkRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_FINDSERVERSONNETWORKREQUEST]);
}

static UA_INLINE void
UA_FindServersOnNetworkRequest_deleteMembers(UA_FindServersOnNetworkRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_FINDSERVERSONNETWORKREQUEST]);
}

static UA_INLINE void
UA_FindServersOnNetworkRequest_clear(UA_FindServersOnNetworkRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_FINDSERVERSONNETWORKREQUEST]);
}

static UA_INLINE void
UA_FindServersOnNetworkRequest_delete(UA_FindServersOnNetworkRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_FINDSERVERSONNETWORKREQUEST]);
}

/* CallResponse */
static UA_INLINE void
UA_CallResponse_init(UA_CallResponse *p) {
    memset(p, 0, sizeof(UA_CallResponse));
}

static UA_INLINE UA_CallResponse *
UA_CallResponse_new(void) {
    return (UA_CallResponse*)UA_new(&UA_TYPES[UA_TYPES_CALLRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_CallResponse_copy(const UA_CallResponse *src, UA_CallResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CALLRESPONSE]);
}

static UA_INLINE void
UA_CallResponse_deleteMembers(UA_CallResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CALLRESPONSE]);
}

static UA_INLINE void
UA_CallResponse_clear(UA_CallResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CALLRESPONSE]);
}

static UA_INLINE void
UA_CallResponse_delete(UA_CallResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_CALLRESPONSE]);
}

/* DeleteNodesResponse */
static UA_INLINE void
UA_DeleteNodesResponse_init(UA_DeleteNodesResponse *p) {
    memset(p, 0, sizeof(UA_DeleteNodesResponse));
}

static UA_INLINE UA_DeleteNodesResponse *
UA_DeleteNodesResponse_new(void) {
    return (UA_DeleteNodesResponse*)UA_new(&UA_TYPES[UA_TYPES_DELETENODESRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_DeleteNodesResponse_copy(const UA_DeleteNodesResponse *src, UA_DeleteNodesResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DELETENODESRESPONSE]);
}

static UA_INLINE void
UA_DeleteNodesResponse_deleteMembers(UA_DeleteNodesResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DELETENODESRESPONSE]);
}

static UA_INLINE void
UA_DeleteNodesResponse_clear(UA_DeleteNodesResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DELETENODESRESPONSE]);
}

static UA_INLINE void
UA_DeleteNodesResponse_delete(UA_DeleteNodesResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_DELETENODESRESPONSE]);
}

/* ModifyMonitoredItemsRequest */
static UA_INLINE void
UA_ModifyMonitoredItemsRequest_init(UA_ModifyMonitoredItemsRequest *p) {
    memset(p, 0, sizeof(UA_ModifyMonitoredItemsRequest));
}

static UA_INLINE UA_ModifyMonitoredItemsRequest *
UA_ModifyMonitoredItemsRequest_new(void) {
    return (UA_ModifyMonitoredItemsRequest*)UA_new(&UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_ModifyMonitoredItemsRequest_copy(const UA_ModifyMonitoredItemsRequest *src, UA_ModifyMonitoredItemsRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSREQUEST]);
}

static UA_INLINE void
UA_ModifyMonitoredItemsRequest_deleteMembers(UA_ModifyMonitoredItemsRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSREQUEST]);
}

static UA_INLINE void
UA_ModifyMonitoredItemsRequest_clear(UA_ModifyMonitoredItemsRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSREQUEST]);
}

static UA_INLINE void
UA_ModifyMonitoredItemsRequest_delete(UA_ModifyMonitoredItemsRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSREQUEST]);
}

/* ServiceFault */
static UA_INLINE void
UA_ServiceFault_init(UA_ServiceFault *p) {
    memset(p, 0, sizeof(UA_ServiceFault));
}

static UA_INLINE UA_ServiceFault *
UA_ServiceFault_new(void) {
    return (UA_ServiceFault*)UA_new(&UA_TYPES[UA_TYPES_SERVICEFAULT]);
}

static UA_INLINE UA_StatusCode
UA_ServiceFault_copy(const UA_ServiceFault *src, UA_ServiceFault *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_SERVICEFAULT]);
}

static UA_INLINE void
UA_ServiceFault_deleteMembers(UA_ServiceFault *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SERVICEFAULT]);
}

static UA_INLINE void
UA_ServiceFault_clear(UA_ServiceFault *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SERVICEFAULT]);
}

static UA_INLINE void
UA_ServiceFault_delete(UA_ServiceFault *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_SERVICEFAULT]);
}

/* PublishResponse */
static UA_INLINE void
UA_PublishResponse_init(UA_PublishResponse *p) {
    memset(p, 0, sizeof(UA_PublishResponse));
}

static UA_INLINE UA_PublishResponse *
UA_PublishResponse_new(void) {
    return (UA_PublishResponse*)UA_new(&UA_TYPES[UA_TYPES_PUBLISHRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_PublishResponse_copy(const UA_PublishResponse *src, UA_PublishResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_PUBLISHRESPONSE]);
}

static UA_INLINE void
UA_PublishResponse_deleteMembers(UA_PublishResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_PUBLISHRESPONSE]);
}

static UA_INLINE void
UA_PublishResponse_clear(UA_PublishResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_PUBLISHRESPONSE]);
}

static UA_INLINE void
UA_PublishResponse_delete(UA_PublishResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_PUBLISHRESPONSE]);
}

/* CreateMonitoredItemsRequest */
static UA_INLINE void
UA_CreateMonitoredItemsRequest_init(UA_CreateMonitoredItemsRequest *p) {
    memset(p, 0, sizeof(UA_CreateMonitoredItemsRequest));
}

static UA_INLINE UA_CreateMonitoredItemsRequest *
UA_CreateMonitoredItemsRequest_new(void) {
    return (UA_CreateMonitoredItemsRequest*)UA_new(&UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_CreateMonitoredItemsRequest_copy(const UA_CreateMonitoredItemsRequest *src, UA_CreateMonitoredItemsRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSREQUEST]);
}

static UA_INLINE void
UA_CreateMonitoredItemsRequest_deleteMembers(UA_CreateMonitoredItemsRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSREQUEST]);
}

static UA_INLINE void
UA_CreateMonitoredItemsRequest_clear(UA_CreateMonitoredItemsRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSREQUEST]);
}

static UA_INLINE void
UA_CreateMonitoredItemsRequest_delete(UA_CreateMonitoredItemsRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSREQUEST]);
}

/* OpenSecureChannelRequest */
static UA_INLINE void
UA_OpenSecureChannelRequest_init(UA_OpenSecureChannelRequest *p) {
    memset(p, 0, sizeof(UA_OpenSecureChannelRequest));
}

static UA_INLINE UA_OpenSecureChannelRequest *
UA_OpenSecureChannelRequest_new(void) {
    return (UA_OpenSecureChannelRequest*)UA_new(&UA_TYPES[UA_TYPES_OPENSECURECHANNELREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_OpenSecureChannelRequest_copy(const UA_OpenSecureChannelRequest *src, UA_OpenSecureChannelRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_OPENSECURECHANNELREQUEST]);
}

static UA_INLINE void
UA_OpenSecureChannelRequest_deleteMembers(UA_OpenSecureChannelRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_OPENSECURECHANNELREQUEST]);
}

static UA_INLINE void
UA_OpenSecureChannelRequest_clear(UA_OpenSecureChannelRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_OPENSECURECHANNELREQUEST]);
}

static UA_INLINE void
UA_OpenSecureChannelRequest_delete(UA_OpenSecureChannelRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_OPENSECURECHANNELREQUEST]);
}

/* CloseSessionRequest */
static UA_INLINE void
UA_CloseSessionRequest_init(UA_CloseSessionRequest *p) {
    memset(p, 0, sizeof(UA_CloseSessionRequest));
}

static UA_INLINE UA_CloseSessionRequest *
UA_CloseSessionRequest_new(void) {
    return (UA_CloseSessionRequest*)UA_new(&UA_TYPES[UA_TYPES_CLOSESESSIONREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_CloseSessionRequest_copy(const UA_CloseSessionRequest *src, UA_CloseSessionRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CLOSESESSIONREQUEST]);
}

static UA_INLINE void
UA_CloseSessionRequest_deleteMembers(UA_CloseSessionRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CLOSESESSIONREQUEST]);
}

static UA_INLINE void
UA_CloseSessionRequest_clear(UA_CloseSessionRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CLOSESESSIONREQUEST]);
}

static UA_INLINE void
UA_CloseSessionRequest_delete(UA_CloseSessionRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_CLOSESESSIONREQUEST]);
}

/* SetTriggeringRequest */
static UA_INLINE void
UA_SetTriggeringRequest_init(UA_SetTriggeringRequest *p) {
    memset(p, 0, sizeof(UA_SetTriggeringRequest));
}

static UA_INLINE UA_SetTriggeringRequest *
UA_SetTriggeringRequest_new(void) {
    return (UA_SetTriggeringRequest*)UA_new(&UA_TYPES[UA_TYPES_SETTRIGGERINGREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_SetTriggeringRequest_copy(const UA_SetTriggeringRequest *src, UA_SetTriggeringRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_SETTRIGGERINGREQUEST]);
}

static UA_INLINE void
UA_SetTriggeringRequest_deleteMembers(UA_SetTriggeringRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SETTRIGGERINGREQUEST]);
}

static UA_INLINE void
UA_SetTriggeringRequest_clear(UA_SetTriggeringRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SETTRIGGERINGREQUEST]);
}

static UA_INLINE void
UA_SetTriggeringRequest_delete(UA_SetTriggeringRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_SETTRIGGERINGREQUEST]);
}

/* BrowseResult */
static UA_INLINE void
UA_BrowseResult_init(UA_BrowseResult *p) {
    memset(p, 0, sizeof(UA_BrowseResult));
}

static UA_INLINE UA_BrowseResult *
UA_BrowseResult_new(void) {
    return (UA_BrowseResult*)UA_new(&UA_TYPES[UA_TYPES_BROWSERESULT]);
}

static UA_INLINE UA_StatusCode
UA_BrowseResult_copy(const UA_BrowseResult *src, UA_BrowseResult *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_BROWSERESULT]);
}

static UA_INLINE void
UA_BrowseResult_deleteMembers(UA_BrowseResult *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_BROWSERESULT]);
}

static UA_INLINE void
UA_BrowseResult_clear(UA_BrowseResult *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_BROWSERESULT]);
}

static UA_INLINE void
UA_BrowseResult_delete(UA_BrowseResult *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_BROWSERESULT]);
}

/* AddReferencesRequest */
static UA_INLINE void
UA_AddReferencesRequest_init(UA_AddReferencesRequest *p) {
    memset(p, 0, sizeof(UA_AddReferencesRequest));
}

static UA_INLINE UA_AddReferencesRequest *
UA_AddReferencesRequest_new(void) {
    return (UA_AddReferencesRequest*)UA_new(&UA_TYPES[UA_TYPES_ADDREFERENCESREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_AddReferencesRequest_copy(const UA_AddReferencesRequest *src, UA_AddReferencesRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ADDREFERENCESREQUEST]);
}

static UA_INLINE void
UA_AddReferencesRequest_deleteMembers(UA_AddReferencesRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ADDREFERENCESREQUEST]);
}

static UA_INLINE void
UA_AddReferencesRequest_clear(UA_AddReferencesRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ADDREFERENCESREQUEST]);
}

static UA_INLINE void
UA_AddReferencesRequest_delete(UA_AddReferencesRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_ADDREFERENCESREQUEST]);
}

/* AddNodesItem */
static UA_INLINE void
UA_AddNodesItem_init(UA_AddNodesItem *p) {
    memset(p, 0, sizeof(UA_AddNodesItem));
}

static UA_INLINE UA_AddNodesItem *
UA_AddNodesItem_new(void) {
    return (UA_AddNodesItem*)UA_new(&UA_TYPES[UA_TYPES_ADDNODESITEM]);
}

static UA_INLINE UA_StatusCode
UA_AddNodesItem_copy(const UA_AddNodesItem *src, UA_AddNodesItem *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ADDNODESITEM]);
}

static UA_INLINE void
UA_AddNodesItem_deleteMembers(UA_AddNodesItem *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ADDNODESITEM]);
}

static UA_INLINE void
UA_AddNodesItem_clear(UA_AddNodesItem *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ADDNODESITEM]);
}

static UA_INLINE void
UA_AddNodesItem_delete(UA_AddNodesItem *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_ADDNODESITEM]);
}

/* ServerStatusDataType */
static UA_INLINE void
UA_ServerStatusDataType_init(UA_ServerStatusDataType *p) {
    memset(p, 0, sizeof(UA_ServerStatusDataType));
}

static UA_INLINE UA_ServerStatusDataType *
UA_ServerStatusDataType_new(void) {
    return (UA_ServerStatusDataType*)UA_new(&UA_TYPES[UA_TYPES_SERVERSTATUSDATATYPE]);
}

static UA_INLINE UA_StatusCode
UA_ServerStatusDataType_copy(const UA_ServerStatusDataType *src, UA_ServerStatusDataType *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_SERVERSTATUSDATATYPE]);
}

static UA_INLINE void
UA_ServerStatusDataType_deleteMembers(UA_ServerStatusDataType *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SERVERSTATUSDATATYPE]);
}

static UA_INLINE void
UA_ServerStatusDataType_clear(UA_ServerStatusDataType *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_SERVERSTATUSDATATYPE]);
}

static UA_INLINE void
UA_ServerStatusDataType_delete(UA_ServerStatusDataType *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_SERVERSTATUSDATATYPE]);
}

/* BrowseNextResponse */
static UA_INLINE void
UA_BrowseNextResponse_init(UA_BrowseNextResponse *p) {
    memset(p, 0, sizeof(UA_BrowseNextResponse));
}

static UA_INLINE UA_BrowseNextResponse *
UA_BrowseNextResponse_new(void) {
    return (UA_BrowseNextResponse*)UA_new(&UA_TYPES[UA_TYPES_BROWSENEXTRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_BrowseNextResponse_copy(const UA_BrowseNextResponse *src, UA_BrowseNextResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_BROWSENEXTRESPONSE]);
}

static UA_INLINE void
UA_BrowseNextResponse_deleteMembers(UA_BrowseNextResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_BROWSENEXTRESPONSE]);
}

static UA_INLINE void
UA_BrowseNextResponse_clear(UA_BrowseNextResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_BROWSENEXTRESPONSE]);
}

static UA_INLINE void
UA_BrowseNextResponse_delete(UA_BrowseNextResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_BROWSENEXTRESPONSE]);
}

/* RegisteredServer */
static UA_INLINE void
UA_RegisteredServer_init(UA_RegisteredServer *p) {
    memset(p, 0, sizeof(UA_RegisteredServer));
}

static UA_INLINE UA_RegisteredServer *
UA_RegisteredServer_new(void) {
    return (UA_RegisteredServer*)UA_new(&UA_TYPES[UA_TYPES_REGISTEREDSERVER]);
}

static UA_INLINE UA_StatusCode
UA_RegisteredServer_copy(const UA_RegisteredServer *src, UA_RegisteredServer *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_REGISTEREDSERVER]);
}

static UA_INLINE void
UA_RegisteredServer_deleteMembers(UA_RegisteredServer *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REGISTEREDSERVER]);
}

static UA_INLINE void
UA_RegisteredServer_clear(UA_RegisteredServer *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REGISTEREDSERVER]);
}

static UA_INLINE void
UA_RegisteredServer_delete(UA_RegisteredServer *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_REGISTEREDSERVER]);
}

/* ApplicationDescription */
static UA_INLINE void
UA_ApplicationDescription_init(UA_ApplicationDescription *p) {
    memset(p, 0, sizeof(UA_ApplicationDescription));
}

static UA_INLINE UA_ApplicationDescription *
UA_ApplicationDescription_new(void) {
    return (UA_ApplicationDescription*)UA_new(&UA_TYPES[UA_TYPES_APPLICATIONDESCRIPTION]);
}

static UA_INLINE UA_StatusCode
UA_ApplicationDescription_copy(const UA_ApplicationDescription *src, UA_ApplicationDescription *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_APPLICATIONDESCRIPTION]);
}

static UA_INLINE void
UA_ApplicationDescription_deleteMembers(UA_ApplicationDescription *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_APPLICATIONDESCRIPTION]);
}

static UA_INLINE void
UA_ApplicationDescription_clear(UA_ApplicationDescription *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_APPLICATIONDESCRIPTION]);
}

static UA_INLINE void
UA_ApplicationDescription_delete(UA_ApplicationDescription *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_APPLICATIONDESCRIPTION]);
}

/* ReadRequest */
static UA_INLINE void
UA_ReadRequest_init(UA_ReadRequest *p) {
    memset(p, 0, sizeof(UA_ReadRequest));
}

static UA_INLINE UA_ReadRequest *
UA_ReadRequest_new(void) {
    return (UA_ReadRequest*)UA_new(&UA_TYPES[UA_TYPES_READREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_ReadRequest_copy(const UA_ReadRequest *src, UA_ReadRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_READREQUEST]);
}

static UA_INLINE void
UA_ReadRequest_deleteMembers(UA_ReadRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_READREQUEST]);
}

static UA_INLINE void
UA_ReadRequest_clear(UA_ReadRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_READREQUEST]);
}

static UA_INLINE void
UA_ReadRequest_delete(UA_ReadRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_READREQUEST]);
}

/* ActivateSessionRequest */
static UA_INLINE void
UA_ActivateSessionRequest_init(UA_ActivateSessionRequest *p) {
    memset(p, 0, sizeof(UA_ActivateSessionRequest));
}

static UA_INLINE UA_ActivateSessionRequest *
UA_ActivateSessionRequest_new(void) {
    return (UA_ActivateSessionRequest*)UA_new(&UA_TYPES[UA_TYPES_ACTIVATESESSIONREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_ActivateSessionRequest_copy(const UA_ActivateSessionRequest *src, UA_ActivateSessionRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ACTIVATESESSIONREQUEST]);
}

static UA_INLINE void
UA_ActivateSessionRequest_deleteMembers(UA_ActivateSessionRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ACTIVATESESSIONREQUEST]);
}

static UA_INLINE void
UA_ActivateSessionRequest_clear(UA_ActivateSessionRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ACTIVATESESSIONREQUEST]);
}

static UA_INLINE void
UA_ActivateSessionRequest_delete(UA_ActivateSessionRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_ACTIVATESESSIONREQUEST]);
}

/* BrowsePathResult */
static UA_INLINE void
UA_BrowsePathResult_init(UA_BrowsePathResult *p) {
    memset(p, 0, sizeof(UA_BrowsePathResult));
}

static UA_INLINE UA_BrowsePathResult *
UA_BrowsePathResult_new(void) {
    return (UA_BrowsePathResult*)UA_new(&UA_TYPES[UA_TYPES_BROWSEPATHRESULT]);
}

static UA_INLINE UA_StatusCode
UA_BrowsePathResult_copy(const UA_BrowsePathResult *src, UA_BrowsePathResult *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_BROWSEPATHRESULT]);
}

static UA_INLINE void
UA_BrowsePathResult_deleteMembers(UA_BrowsePathResult *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_BROWSEPATHRESULT]);
}

static UA_INLINE void
UA_BrowsePathResult_clear(UA_BrowsePathResult *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_BROWSEPATHRESULT]);
}

static UA_INLINE void
UA_BrowsePathResult_delete(UA_BrowsePathResult *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_BROWSEPATHRESULT]);
}

/* AddNodesRequest */
static UA_INLINE void
UA_AddNodesRequest_init(UA_AddNodesRequest *p) {
    memset(p, 0, sizeof(UA_AddNodesRequest));
}

static UA_INLINE UA_AddNodesRequest *
UA_AddNodesRequest_new(void) {
    return (UA_AddNodesRequest*)UA_new(&UA_TYPES[UA_TYPES_ADDNODESREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_AddNodesRequest_copy(const UA_AddNodesRequest *src, UA_AddNodesRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ADDNODESREQUEST]);
}

static UA_INLINE void
UA_AddNodesRequest_deleteMembers(UA_AddNodesRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ADDNODESREQUEST]);
}

static UA_INLINE void
UA_AddNodesRequest_clear(UA_AddNodesRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ADDNODESREQUEST]);
}

static UA_INLINE void
UA_AddNodesRequest_delete(UA_AddNodesRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_ADDNODESREQUEST]);
}

/* BrowseRequest */
static UA_INLINE void
UA_BrowseRequest_init(UA_BrowseRequest *p) {
    memset(p, 0, sizeof(UA_BrowseRequest));
}

static UA_INLINE UA_BrowseRequest *
UA_BrowseRequest_new(void) {
    return (UA_BrowseRequest*)UA_new(&UA_TYPES[UA_TYPES_BROWSEREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_BrowseRequest_copy(const UA_BrowseRequest *src, UA_BrowseRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_BROWSEREQUEST]);
}

static UA_INLINE void
UA_BrowseRequest_deleteMembers(UA_BrowseRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_BROWSEREQUEST]);
}

static UA_INLINE void
UA_BrowseRequest_clear(UA_BrowseRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_BROWSEREQUEST]);
}

static UA_INLINE void
UA_BrowseRequest_delete(UA_BrowseRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_BROWSEREQUEST]);
}

/* WriteRequest */
static UA_INLINE void
UA_WriteRequest_init(UA_WriteRequest *p) {
    memset(p, 0, sizeof(UA_WriteRequest));
}

static UA_INLINE UA_WriteRequest *
UA_WriteRequest_new(void) {
    return (UA_WriteRequest*)UA_new(&UA_TYPES[UA_TYPES_WRITEREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_WriteRequest_copy(const UA_WriteRequest *src, UA_WriteRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_WRITEREQUEST]);
}

static UA_INLINE void
UA_WriteRequest_deleteMembers(UA_WriteRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_WRITEREQUEST]);
}

static UA_INLINE void
UA_WriteRequest_clear(UA_WriteRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_WRITEREQUEST]);
}

static UA_INLINE void
UA_WriteRequest_delete(UA_WriteRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_WRITEREQUEST]);
}

/* AddNodesResponse */
static UA_INLINE void
UA_AddNodesResponse_init(UA_AddNodesResponse *p) {
    memset(p, 0, sizeof(UA_AddNodesResponse));
}

static UA_INLINE UA_AddNodesResponse *
UA_AddNodesResponse_new(void) {
    return (UA_AddNodesResponse*)UA_new(&UA_TYPES[UA_TYPES_ADDNODESRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_AddNodesResponse_copy(const UA_AddNodesResponse *src, UA_AddNodesResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ADDNODESRESPONSE]);
}

static UA_INLINE void
UA_AddNodesResponse_deleteMembers(UA_AddNodesResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ADDNODESRESPONSE]);
}

static UA_INLINE void
UA_AddNodesResponse_clear(UA_AddNodesResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ADDNODESRESPONSE]);
}

static UA_INLINE void
UA_AddNodesResponse_delete(UA_AddNodesResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_ADDNODESRESPONSE]);
}

/* RegisterServer2Request */
static UA_INLINE void
UA_RegisterServer2Request_init(UA_RegisterServer2Request *p) {
    memset(p, 0, sizeof(UA_RegisterServer2Request));
}

static UA_INLINE UA_RegisterServer2Request *
UA_RegisterServer2Request_new(void) {
    return (UA_RegisterServer2Request*)UA_new(&UA_TYPES[UA_TYPES_REGISTERSERVER2REQUEST]);
}

static UA_INLINE UA_StatusCode
UA_RegisterServer2Request_copy(const UA_RegisterServer2Request *src, UA_RegisterServer2Request *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_REGISTERSERVER2REQUEST]);
}

static UA_INLINE void
UA_RegisterServer2Request_deleteMembers(UA_RegisterServer2Request *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REGISTERSERVER2REQUEST]);
}

static UA_INLINE void
UA_RegisterServer2Request_clear(UA_RegisterServer2Request *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REGISTERSERVER2REQUEST]);
}

static UA_INLINE void
UA_RegisterServer2Request_delete(UA_RegisterServer2Request *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_REGISTERSERVER2REQUEST]);
}

/* AttributeOperand */
static UA_INLINE void
UA_AttributeOperand_init(UA_AttributeOperand *p) {
    memset(p, 0, sizeof(UA_AttributeOperand));
}

static UA_INLINE UA_AttributeOperand *
UA_AttributeOperand_new(void) {
    return (UA_AttributeOperand*)UA_new(&UA_TYPES[UA_TYPES_ATTRIBUTEOPERAND]);
}

static UA_INLINE UA_StatusCode
UA_AttributeOperand_copy(const UA_AttributeOperand *src, UA_AttributeOperand *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ATTRIBUTEOPERAND]);
}

static UA_INLINE void
UA_AttributeOperand_deleteMembers(UA_AttributeOperand *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ATTRIBUTEOPERAND]);
}

static UA_INLINE void
UA_AttributeOperand_clear(UA_AttributeOperand *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ATTRIBUTEOPERAND]);
}

static UA_INLINE void
UA_AttributeOperand_delete(UA_AttributeOperand *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_ATTRIBUTEOPERAND]);
}

/* DataChangeFilter */
static UA_INLINE void
UA_DataChangeFilter_init(UA_DataChangeFilter *p) {
    memset(p, 0, sizeof(UA_DataChangeFilter));
}

static UA_INLINE UA_DataChangeFilter *
UA_DataChangeFilter_new(void) {
    return (UA_DataChangeFilter*)UA_new(&UA_TYPES[UA_TYPES_DATACHANGEFILTER]);
}

static UA_INLINE UA_StatusCode
UA_DataChangeFilter_copy(const UA_DataChangeFilter *src, UA_DataChangeFilter *dst) {
    *dst = *src;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE void
UA_DataChangeFilter_deleteMembers(UA_DataChangeFilter *p) {
    memset(p, 0, sizeof(UA_DataChangeFilter));
}

static UA_INLINE void
UA_DataChangeFilter_clear(UA_DataChangeFilter *p) {
    memset(p, 0, sizeof(UA_DataChangeFilter));
}

static UA_INLINE void
UA_DataChangeFilter_delete(UA_DataChangeFilter *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_DATACHANGEFILTER]);
}

/* EndpointDescription */
static UA_INLINE void
UA_EndpointDescription_init(UA_EndpointDescription *p) {
    memset(p, 0, sizeof(UA_EndpointDescription));
}

static UA_INLINE UA_EndpointDescription *
UA_EndpointDescription_new(void) {
    return (UA_EndpointDescription*)UA_new(&UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
}

static UA_INLINE UA_StatusCode
UA_EndpointDescription_copy(const UA_EndpointDescription *src, UA_EndpointDescription *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
}

static UA_INLINE void
UA_EndpointDescription_deleteMembers(UA_EndpointDescription *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
}

static UA_INLINE void
UA_EndpointDescription_clear(UA_EndpointDescription *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
}

static UA_INLINE void
UA_EndpointDescription_delete(UA_EndpointDescription *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
}

/* DeleteReferencesRequest */
static UA_INLINE void
UA_DeleteReferencesRequest_init(UA_DeleteReferencesRequest *p) {
    memset(p, 0, sizeof(UA_DeleteReferencesRequest));
}

static UA_INLINE UA_DeleteReferencesRequest *
UA_DeleteReferencesRequest_new(void) {
    return (UA_DeleteReferencesRequest*)UA_new(&UA_TYPES[UA_TYPES_DELETEREFERENCESREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_DeleteReferencesRequest_copy(const UA_DeleteReferencesRequest *src, UA_DeleteReferencesRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DELETEREFERENCESREQUEST]);
}

static UA_INLINE void
UA_DeleteReferencesRequest_deleteMembers(UA_DeleteReferencesRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DELETEREFERENCESREQUEST]);
}

static UA_INLINE void
UA_DeleteReferencesRequest_clear(UA_DeleteReferencesRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_DELETEREFERENCESREQUEST]);
}

static UA_INLINE void
UA_DeleteReferencesRequest_delete(UA_DeleteReferencesRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_DELETEREFERENCESREQUEST]);
}

/* TranslateBrowsePathsToNodeIdsRequest */
static UA_INLINE void
UA_TranslateBrowsePathsToNodeIdsRequest_init(UA_TranslateBrowsePathsToNodeIdsRequest *p) {
    memset(p, 0, sizeof(UA_TranslateBrowsePathsToNodeIdsRequest));
}

static UA_INLINE UA_TranslateBrowsePathsToNodeIdsRequest *
UA_TranslateBrowsePathsToNodeIdsRequest_new(void) {
    return (UA_TranslateBrowsePathsToNodeIdsRequest*)UA_new(&UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_TranslateBrowsePathsToNodeIdsRequest_copy(const UA_TranslateBrowsePathsToNodeIdsRequest *src, UA_TranslateBrowsePathsToNodeIdsRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSREQUEST]);
}

static UA_INLINE void
UA_TranslateBrowsePathsToNodeIdsRequest_deleteMembers(UA_TranslateBrowsePathsToNodeIdsRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSREQUEST]);
}

static UA_INLINE void
UA_TranslateBrowsePathsToNodeIdsRequest_clear(UA_TranslateBrowsePathsToNodeIdsRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSREQUEST]);
}

static UA_INLINE void
UA_TranslateBrowsePathsToNodeIdsRequest_delete(UA_TranslateBrowsePathsToNodeIdsRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSREQUEST]);
}

/* FindServersResponse */
static UA_INLINE void
UA_FindServersResponse_init(UA_FindServersResponse *p) {
    memset(p, 0, sizeof(UA_FindServersResponse));
}

static UA_INLINE UA_FindServersResponse *
UA_FindServersResponse_new(void) {
    return (UA_FindServersResponse*)UA_new(&UA_TYPES[UA_TYPES_FINDSERVERSRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_FindServersResponse_copy(const UA_FindServersResponse *src, UA_FindServersResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_FINDSERVERSRESPONSE]);
}

static UA_INLINE void
UA_FindServersResponse_deleteMembers(UA_FindServersResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_FINDSERVERSRESPONSE]);
}

static UA_INLINE void
UA_FindServersResponse_clear(UA_FindServersResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_FINDSERVERSRESPONSE]);
}

static UA_INLINE void
UA_FindServersResponse_delete(UA_FindServersResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_FINDSERVERSRESPONSE]);
}

/* CreateSessionRequest */
static UA_INLINE void
UA_CreateSessionRequest_init(UA_CreateSessionRequest *p) {
    memset(p, 0, sizeof(UA_CreateSessionRequest));
}

static UA_INLINE UA_CreateSessionRequest *
UA_CreateSessionRequest_new(void) {
    return (UA_CreateSessionRequest*)UA_new(&UA_TYPES[UA_TYPES_CREATESESSIONREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_CreateSessionRequest_copy(const UA_CreateSessionRequest *src, UA_CreateSessionRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CREATESESSIONREQUEST]);
}

static UA_INLINE void
UA_CreateSessionRequest_deleteMembers(UA_CreateSessionRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CREATESESSIONREQUEST]);
}

static UA_INLINE void
UA_CreateSessionRequest_clear(UA_CreateSessionRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CREATESESSIONREQUEST]);
}

static UA_INLINE void
UA_CreateSessionRequest_delete(UA_CreateSessionRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_CREATESESSIONREQUEST]);
}

/* ContentFilterElement */
static UA_INLINE void
UA_ContentFilterElement_init(UA_ContentFilterElement *p) {
    memset(p, 0, sizeof(UA_ContentFilterElement));
}

static UA_INLINE UA_ContentFilterElement *
UA_ContentFilterElement_new(void) {
    return (UA_ContentFilterElement*)UA_new(&UA_TYPES[UA_TYPES_CONTENTFILTERELEMENT]);
}

static UA_INLINE UA_StatusCode
UA_ContentFilterElement_copy(const UA_ContentFilterElement *src, UA_ContentFilterElement *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CONTENTFILTERELEMENT]);
}

static UA_INLINE void
UA_ContentFilterElement_deleteMembers(UA_ContentFilterElement *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CONTENTFILTERELEMENT]);
}

static UA_INLINE void
UA_ContentFilterElement_clear(UA_ContentFilterElement *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CONTENTFILTERELEMENT]);
}

static UA_INLINE void
UA_ContentFilterElement_delete(UA_ContentFilterElement *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_CONTENTFILTERELEMENT]);
}

/* RegisterServerRequest */
static UA_INLINE void
UA_RegisterServerRequest_init(UA_RegisterServerRequest *p) {
    memset(p, 0, sizeof(UA_RegisterServerRequest));
}

static UA_INLINE UA_RegisterServerRequest *
UA_RegisterServerRequest_new(void) {
    return (UA_RegisterServerRequest*)UA_new(&UA_TYPES[UA_TYPES_REGISTERSERVERREQUEST]);
}

static UA_INLINE UA_StatusCode
UA_RegisterServerRequest_copy(const UA_RegisterServerRequest *src, UA_RegisterServerRequest *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_REGISTERSERVERREQUEST]);
}

static UA_INLINE void
UA_RegisterServerRequest_deleteMembers(UA_RegisterServerRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REGISTERSERVERREQUEST]);
}

static UA_INLINE void
UA_RegisterServerRequest_clear(UA_RegisterServerRequest *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_REGISTERSERVERREQUEST]);
}

static UA_INLINE void
UA_RegisterServerRequest_delete(UA_RegisterServerRequest *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_REGISTERSERVERREQUEST]);
}

/* TranslateBrowsePathsToNodeIdsResponse */
static UA_INLINE void
UA_TranslateBrowsePathsToNodeIdsResponse_init(UA_TranslateBrowsePathsToNodeIdsResponse *p) {
    memset(p, 0, sizeof(UA_TranslateBrowsePathsToNodeIdsResponse));
}

static UA_INLINE UA_TranslateBrowsePathsToNodeIdsResponse *
UA_TranslateBrowsePathsToNodeIdsResponse_new(void) {
    return (UA_TranslateBrowsePathsToNodeIdsResponse*)UA_new(&UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_TranslateBrowsePathsToNodeIdsResponse_copy(const UA_TranslateBrowsePathsToNodeIdsResponse *src, UA_TranslateBrowsePathsToNodeIdsResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSRESPONSE]);
}

static UA_INLINE void
UA_TranslateBrowsePathsToNodeIdsResponse_deleteMembers(UA_TranslateBrowsePathsToNodeIdsResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSRESPONSE]);
}

static UA_INLINE void
UA_TranslateBrowsePathsToNodeIdsResponse_clear(UA_TranslateBrowsePathsToNodeIdsResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSRESPONSE]);
}

static UA_INLINE void
UA_TranslateBrowsePathsToNodeIdsResponse_delete(UA_TranslateBrowsePathsToNodeIdsResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSRESPONSE]);
}

/* BrowseResponse */
static UA_INLINE void
UA_BrowseResponse_init(UA_BrowseResponse *p) {
    memset(p, 0, sizeof(UA_BrowseResponse));
}

static UA_INLINE UA_BrowseResponse *
UA_BrowseResponse_new(void) {
    return (UA_BrowseResponse*)UA_new(&UA_TYPES[UA_TYPES_BROWSERESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_BrowseResponse_copy(const UA_BrowseResponse *src, UA_BrowseResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_BROWSERESPONSE]);
}

static UA_INLINE void
UA_BrowseResponse_deleteMembers(UA_BrowseResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_BROWSERESPONSE]);
}

static UA_INLINE void
UA_BrowseResponse_clear(UA_BrowseResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_BROWSERESPONSE]);
}

static UA_INLINE void
UA_BrowseResponse_delete(UA_BrowseResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_BROWSERESPONSE]);
}

/* CreateSessionResponse */
static UA_INLINE void
UA_CreateSessionResponse_init(UA_CreateSessionResponse *p) {
    memset(p, 0, sizeof(UA_CreateSessionResponse));
}

static UA_INLINE UA_CreateSessionResponse *
UA_CreateSessionResponse_new(void) {
    return (UA_CreateSessionResponse*)UA_new(&UA_TYPES[UA_TYPES_CREATESESSIONRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_CreateSessionResponse_copy(const UA_CreateSessionResponse *src, UA_CreateSessionResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CREATESESSIONRESPONSE]);
}

static UA_INLINE void
UA_CreateSessionResponse_deleteMembers(UA_CreateSessionResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CREATESESSIONRESPONSE]);
}

static UA_INLINE void
UA_CreateSessionResponse_clear(UA_CreateSessionResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CREATESESSIONRESPONSE]);
}

static UA_INLINE void
UA_CreateSessionResponse_delete(UA_CreateSessionResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_CREATESESSIONRESPONSE]);
}

/* ContentFilter */
static UA_INLINE void
UA_ContentFilter_init(UA_ContentFilter *p) {
    memset(p, 0, sizeof(UA_ContentFilter));
}

static UA_INLINE UA_ContentFilter *
UA_ContentFilter_new(void) {
    return (UA_ContentFilter*)UA_new(&UA_TYPES[UA_TYPES_CONTENTFILTER]);
}

static UA_INLINE UA_StatusCode
UA_ContentFilter_copy(const UA_ContentFilter *src, UA_ContentFilter *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CONTENTFILTER]);
}

static UA_INLINE void
UA_ContentFilter_deleteMembers(UA_ContentFilter *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CONTENTFILTER]);
}

static UA_INLINE void
UA_ContentFilter_clear(UA_ContentFilter *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_CONTENTFILTER]);
}

static UA_INLINE void
UA_ContentFilter_delete(UA_ContentFilter *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_CONTENTFILTER]);
}

/* GetEndpointsResponse */
static UA_INLINE void
UA_GetEndpointsResponse_init(UA_GetEndpointsResponse *p) {
    memset(p, 0, sizeof(UA_GetEndpointsResponse));
}

static UA_INLINE UA_GetEndpointsResponse *
UA_GetEndpointsResponse_new(void) {
    return (UA_GetEndpointsResponse*)UA_new(&UA_TYPES[UA_TYPES_GETENDPOINTSRESPONSE]);
}

static UA_INLINE UA_StatusCode
UA_GetEndpointsResponse_copy(const UA_GetEndpointsResponse *src, UA_GetEndpointsResponse *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_GETENDPOINTSRESPONSE]);
}

static UA_INLINE void
UA_GetEndpointsResponse_deleteMembers(UA_GetEndpointsResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_GETENDPOINTSRESPONSE]);
}

static UA_INLINE void
UA_GetEndpointsResponse_clear(UA_GetEndpointsResponse *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_GETENDPOINTSRESPONSE]);
}

static UA_INLINE void
UA_GetEndpointsResponse_delete(UA_GetEndpointsResponse *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_GETENDPOINTSRESPONSE]);
}

/* EventFilter */
static UA_INLINE void
UA_EventFilter_init(UA_EventFilter *p) {
    memset(p, 0, sizeof(UA_EventFilter));
}

static UA_INLINE UA_EventFilter *
UA_EventFilter_new(void) {
    return (UA_EventFilter*)UA_new(&UA_TYPES[UA_TYPES_EVENTFILTER]);
}

static UA_INLINE UA_StatusCode
UA_EventFilter_copy(const UA_EventFilter *src, UA_EventFilter *dst) {
    return UA_copy(src, dst, &UA_TYPES[UA_TYPES_EVENTFILTER]);
}

static UA_INLINE void
UA_EventFilter_deleteMembers(UA_EventFilter *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_EVENTFILTER]);
}

static UA_INLINE void
UA_EventFilter_clear(UA_EventFilter *p) {
    UA_clear(p, &UA_TYPES[UA_TYPES_EVENTFILTER]);
}

static UA_INLINE void
UA_EventFilter_delete(UA_EventFilter *p) {
    UA_delete(p, &UA_TYPES[UA_TYPES_EVENTFILTER]);
}

#if defined(__GNUC__) && __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
# pragma GCC diagnostic pop
#endif

_UA_END_DECLS

#endif /* TYPES_GENERATED_HANDLING_H_ */
