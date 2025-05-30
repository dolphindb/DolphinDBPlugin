// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: ESecurityIDSource.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "ESecurityIDSource.pb.h"

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
namespace model {

namespace {

const ::google::protobuf::EnumDescriptor* ESecurityIDSource_descriptor_ = NULL;

}  // namespace


void protobuf_AssignDesc_ESecurityIDSource_2eproto() GOOGLE_ATTRIBUTE_COLD;
void protobuf_AssignDesc_ESecurityIDSource_2eproto() {
  protobuf_AddDesc_ESecurityIDSource_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "ESecurityIDSource.proto");
  GOOGLE_CHECK(file != NULL);
  ESecurityIDSource_descriptor_ = file->enum_type(0);
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_ESecurityIDSource_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) GOOGLE_ATTRIBUTE_COLD;
void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
}

}  // namespace

void protobuf_ShutdownFile_ESecurityIDSource_2eproto() {
}

void protobuf_InitDefaults_ESecurityIDSource_2eproto_impl() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

}

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_InitDefaults_ESecurityIDSource_2eproto_once_);
void protobuf_InitDefaults_ESecurityIDSource_2eproto() {
  ::google::protobuf::GoogleOnceInit(&protobuf_InitDefaults_ESecurityIDSource_2eproto_once_,
                 &protobuf_InitDefaults_ESecurityIDSource_2eproto_impl);
}
void protobuf_AddDesc_ESecurityIDSource_2eproto_impl() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  protobuf_InitDefaults_ESecurityIDSource_2eproto();
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\027ESecurityIDSource.proto\022\022com.htsc.mdc."
    "model*\316\007\n\021ESecurityIDSource\022\033\n\027DefaultSe"
    "curityIDSource\020\000\022\010\n\004XSHG\020e\022\010\n\004XSHE\020f\022\010\n\004"
    "NEEQ\020g\022\n\n\006XSHGFI\020h\022\n\n\006XSHECA\020i\022\010\n\004XBSE\020j"
    "\022\n\n\006XSHGFC\020k\022\n\n\006XSHEFC\020l\022\t\n\004XHKG\020\313\001\022\t\n\004H"
    "KSC\020\314\001\022\t\n\004HGHQ\020\315\001\022\t\n\004CCFX\020\255\002\022\t\n\004XSGE\020\256\002\022"
    "\010\n\003INE\020\257\002\022\t\n\004SGEX\020\221\003\022\t\n\004XCFE\020\365\003\022\t\n\004CCDC\020"
    "\366\003\022\t\n\004CNEX\020\367\003\022\t\n\004XDCE\020\331\004\022\t\n\004XZCE\020\332\004\022\t\n\004X"
    "GFE\020\333\004\022\010\n\003SWS\020\275\005\022\010\n\003CNI\020\276\005\022\010\n\003CSI\020\277\005\022\t\n\004"
    "HTIS\020\241\006\022\t\n\004MORN\020\242\006\022\007\n\002QB\020\243\006\022\t\n\004SPDB\020\244\006\022\t"
    "\n\004HTSM\020\245\006\022\010\n\003SCB\020\246\006\022\t\n\004CUBE\020\247\006\022\010\n\003LSE\020\205\007"
    "\022\010\n\003LME\020\206\007\022\n\n\005LIFFE\020\207\007\022\t\n\004ICEU\020\210\007\022\010\n\003BSE"
    "\020\211\007\022\010\n\003NSE\020\212\007\022\010\n\003NEX\020\213\007\022\t\n\004APEX\020\214\007\022\013\n\006IC"
    "E_SG\020\215\007\022\010\n\003SGX\020\216\007\022\010\n\003TSE\020\217\007\022\n\n\005TOCOM\020\220\007\022"
    "\010\n\003OSE\020\221\007\022\n\n\005EUREX\020\222\007\022\010\n\003ICE\020\223\007\022\010\n\003CME\020\224"
    "\007\022\t\n\004CBOT\020\225\007\022\t\n\004CBOE\020\226\007\022\t\n\004AMEX\020\227\007\022\007\n\002US"
    "\020\230\007\022\t\n\004NYSE\020\231\007\022\n\n\005NYMEX\020\232\007\022\n\n\005COMEX\020\233\007\022\t"
    "\n\004ICUS\020\234\007\022\013\n\006NASDAQ\020\235\007\022\010\n\003BBG\020\236\007\022\010\n\003BMD\020"
    "\237\007\022\n\n\005LUXSE\020\240\007\022\010\n\003KRX\020\241\007\022\n\n\005MICEX\020\242\007\022\010\n\003"
    "ASE\020\243\007\022\010\n\003ISE\020\244\007\022\010\n\003DME\020\245\007\022\010\n\003IHK\020\246\007\022\n\n\005"
    "STOXX\020\247\007\022\010\n\003SPI\020\250\007\022\013\n\006NIKKEI\020\251\007\022\010\n\003DJI\020\252"
    "\007\022\t\n\004BATS\020\253\007\022\010\n\003IEX\020\254\007\022\t\n\004OPRA\020\255\007\022\016\n\tREF"
    "INITIV\020\256\007\022\t\n\004OTCM\020\257\007\022\r\n\010EURONEXT\020\260\007\022\010\n\003F"
    "SI\020\261\007\022\t\n\004DBDX\020\262\007\022\010\n\003SAO\020\263\007\022\t\n\004XASX\020\264\007\022\t\n"
    "\004XCBO\020\265\007\022\t\n\004XMIL\020\266\007\022\t\n\004XMOD\020\267\007\022\t\n\004XMEF\020\270"
    "\007\022\t\n\004XOME\020\271\007\022\010\n\003UST\020\272\007B2\n\022com.htsc.mdc.m"
    "odelB\027ESecurityIDSourceProtosH\001\240\001\001b\006prot"
    "o3", 1082);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "ESecurityIDSource.proto", &protobuf_RegisterTypes);
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_ESecurityIDSource_2eproto);
}

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AddDesc_ESecurityIDSource_2eproto_once_);
void protobuf_AddDesc_ESecurityIDSource_2eproto() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AddDesc_ESecurityIDSource_2eproto_once_,
                 &protobuf_AddDesc_ESecurityIDSource_2eproto_impl);
}
// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_ESecurityIDSource_2eproto {
  StaticDescriptorInitializer_ESecurityIDSource_2eproto() {
    protobuf_AddDesc_ESecurityIDSource_2eproto();
  }
} static_descriptor_initializer_ESecurityIDSource_2eproto_;
const ::google::protobuf::EnumDescriptor* ESecurityIDSource_descriptor() {
  protobuf_AssignDescriptorsOnce();
  return ESecurityIDSource_descriptor_;
}
bool ESecurityIDSource_IsValid(int value) {
  switch (value) {
    case 0:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 203:
    case 204:
    case 205:
    case 301:
    case 302:
    case 303:
    case 401:
    case 501:
    case 502:
    case 503:
    case 601:
    case 602:
    case 603:
    case 701:
    case 702:
    case 703:
    case 801:
    case 802:
    case 803:
    case 804:
    case 805:
    case 806:
    case 807:
    case 901:
    case 902:
    case 903:
    case 904:
    case 905:
    case 906:
    case 907:
    case 908:
    case 909:
    case 910:
    case 911:
    case 912:
    case 913:
    case 914:
    case 915:
    case 916:
    case 917:
    case 918:
    case 919:
    case 920:
    case 921:
    case 922:
    case 923:
    case 924:
    case 925:
    case 926:
    case 927:
    case 928:
    case 929:
    case 930:
    case 931:
    case 932:
    case 933:
    case 934:
    case 935:
    case 936:
    case 937:
    case 938:
    case 939:
    case 940:
    case 941:
    case 942:
    case 943:
    case 944:
    case 945:
    case 946:
    case 947:
    case 948:
    case 949:
    case 950:
    case 951:
    case 952:
    case 953:
    case 954:
      return true;
    default:
      return false;
  }
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace model
}  // namespace mdc
}  // namespace htsc
}  // namespace com

// @@protoc_insertion_point(global_scope)
