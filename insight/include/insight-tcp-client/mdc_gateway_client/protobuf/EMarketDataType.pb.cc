// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: EMarketDataType.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "EMarketDataType.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/port.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace com {
namespace htsc {
namespace mdc {
namespace insight {
namespace model {

namespace {

const ::google::protobuf::EnumDescriptor* EMarketDataType_descriptor_ = NULL;

}  // namespace


void protobuf_AssignDesc_EMarketDataType_2eproto() GOOGLE_ATTRIBUTE_COLD;
void protobuf_AssignDesc_EMarketDataType_2eproto() {
  protobuf_AddDesc_EMarketDataType_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "EMarketDataType.proto");
  GOOGLE_CHECK(file != NULL);
  EMarketDataType_descriptor_ = file->enum_type(0);
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_EMarketDataType_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) GOOGLE_ATTRIBUTE_COLD;
void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
}

}  // namespace

void protobuf_ShutdownFile_EMarketDataType_2eproto() {
}

void protobuf_InitDefaults_EMarketDataType_2eproto_impl() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

}

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_InitDefaults_EMarketDataType_2eproto_once_);
void protobuf_InitDefaults_EMarketDataType_2eproto() {
  ::google::protobuf::GoogleOnceInit(&protobuf_InitDefaults_EMarketDataType_2eproto_once_,
                 &protobuf_InitDefaults_EMarketDataType_2eproto_impl);
}
void protobuf_AddDesc_EMarketDataType_2eproto_impl() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  protobuf_InitDefaults_EMarketDataType_2eproto();
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\025EMarketDataType.proto\022\032com.htsc.mdc.in"
    "sight.model*\340\014\n\017EMarketDataType\022\025\n\021UNKNO"
    "WN_DATA_TYPE\020\000\022\013\n\007MD_TICK\020\001\022\022\n\016MD_TRANSA"
    "CTION\020\002\022\014\n\010MD_ORDER\020\003\022\017\n\013MD_CONSTANT\020\004\022\022"
    "\n\016DYNAMIC_PACKET\020\005\022\024\n\020MD_ETF_BASICINFO\020\006"
    "\022\024\n\020MD_IOPV_SNAPSHOT\020\007\022\021\n\rMD_KLINE_1MIN\020"
    "\024\022\021\n\rMD_KLINE_5MIN\020\025\022\022\n\016MD_KLINE_15MIN\020\026"
    "\022\022\n\016MD_KLINE_30MIN\020\027\022\022\n\016MD_KLINE_60MIN\020\030"
    "\022\017\n\013MD_KLINE_1D\020\031\022\020\n\014MD_KLINE_15S\020\032\022\020\n\014M"
    "D_TWAP_1MIN\020\036\022\016\n\nMD_TWAP_1S\020\037\022\020\n\014MD_VWAP"
    "_1MIN\020(\022\016\n\nMD_VWAP_1S\020)\022\022\n\016MD_SIMPLE_TIC"
    "K\0202\022\030\n\024AD_UPSDOWNS_ANALYSIS\0203\022\031\n\025AD_INDI"
    "CATORS_RANKING\0204\022\025\n\021AD_VOLUME_BYPRICE\0205\022"
    "\031\n\025AD_FUND_FLOW_ANALYSIS\0206\022\031\n\025AD_ORDERBO"
    "OK_SNAPSHOT\0207\022#\n\037AD_ORDERBOOK_SNAPSHOT_W"
    "ITH_TICK\0208\022\030\n\024AD_CHIP_DISTRIBUTION\0209\022\016\n\n"
    "MD_WARRANT\020:\022\027\n\023MD_SECURITY_LENDING\020;\022\013\n"
    "\007AD_NEWS\020<\022\025\n\021AD_STARING_RESULT\020=\022\027\n\023AD_"
    "DERIVED_ANALYSIS\020>\022\017\n\013MD_FI_QUOTE\020F\022\014\n\010M"
    "D_QUOTE\020G\022\017\n\013MD_QB_QUOTE\020H\022\025\n\021MD_QB_TRAN"
    "SACTION\020I\022\017\n\013MD_SL_ORDER\020J\022\025\n\021MD_SL_TRAN"
    "SACTION\020K\022\020\n\014MD_USA_ORDER\020L\022\026\n\022MD_USA_TR"
    "ANSACTION\020M\022\025\n\021MD_HK_GREY_MARKET\020N\022\032\n\026MD"
    "_SL_INDICATIVE_QUOTE\020O\022\024\n\020MD_SL_STATISTI"
    "CS\020P\022\020\n\014MD_USA_QUOTE\020Q\022\024\n\020MD_SL_ESTIMATI"
    "ON\020R\022\021\n\rMD_CNEX_QUOTE\020S\022\020\n\014MD_CNEX_DEAL\020"
    "T\022\025\n\021MD_DELAY_SNAPSHOT\020U\022\031\n\025MD_HIGH_ACCU"
    "RACY_TICK\020V\022\022\n\016MD_CFETS_FOREX\020W\022\033\n\027MD_CF"
    "ETS_FOREX_SNAPSHOT\020X\022\030\n\024MD_CFETS_FOREX_Q"
    "UOTE\020Y\022\036\n\032MD_CFETS_CNY_CURRENCY_DEAL\020Z\022\""
    "\n\036MD_CFETS_CNY_CURRENCY_SNAPSHOT\020[\022\032\n\026MD"
    "_CFETS_CNY_BOND_DEAL\020\\\022\036\n\032MD_CFETS_CNY_B"
    "OND_SNAPSHOT\020]\022\032\n\026MD_CFETS_CNY_RATE_DEAL"
    "\020^\022\036\n\032MD_CFETS_CNY_RATE_SNAPSHOT\020_\022\026\n\022MD"
    "_CFETS_BENCHMARK\020`\022\026\n\022MD_CFETS_QDM_QUOTE"
    "\020a\022\031\n\025MD_CFETS_ODM_SNAPSHOT\020b\022#\n\037MD_CFET"
    "S_FOREX_CNY_MIDDLE_PRICE\020c\022#\n\037REPLAY_MD_"
    "TICK_WITH_TRANSACTION\020e\022\035\n\031REPLAY_MD_TIC"
    "K_WITH_ORDER\020f\022-\n)REPLAY_MD_TICK_WITH_TR"
    "ANSACTION_AND_ORDER\020g\022\022\n\016REPLAY_MD_TICK\020"
    "h\022\031\n\025REPLAY_MD_TRANSACTION\020i\022\023\n\017REPLAY_M"
    "D_ORDER\020j\022#\n\037REPLAY_MD_TRANSACTION_AND_O"
    "RDER\020kB7\n\032com.htsc.mdc.insight.modelB\024EM"
    "arketDataTypeProtoH\001\240\001\001b\006proto3", 1751);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "EMarketDataType.proto", &protobuf_RegisterTypes);
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_EMarketDataType_2eproto);
}

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AddDesc_EMarketDataType_2eproto_once_);
void protobuf_AddDesc_EMarketDataType_2eproto() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AddDesc_EMarketDataType_2eproto_once_,
                 &protobuf_AddDesc_EMarketDataType_2eproto_impl);
}
// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_EMarketDataType_2eproto {
  StaticDescriptorInitializer_EMarketDataType_2eproto() {
    protobuf_AddDesc_EMarketDataType_2eproto();
  }
} static_descriptor_initializer_EMarketDataType_2eproto_;
const ::google::protobuf::EnumDescriptor* EMarketDataType_descriptor() {
  protobuf_AssignDescriptorsOnce();
  return EMarketDataType_descriptor_;
}
bool EMarketDataType_IsValid(int value) {
  switch (value) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 30:
    case 31:
    case 40:
    case 41:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 58:
    case 59:
    case 60:
    case 61:
    case 62:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 91:
    case 92:
    case 93:
    case 94:
    case 95:
    case 96:
    case 97:
    case 98:
    case 99:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
      return true;
    default:
      return false;
  }
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace model
}  // namespace insight
}  // namespace mdc
}  // namespace htsc
}  // namespace com

// @@protoc_insertion_point(global_scope)
