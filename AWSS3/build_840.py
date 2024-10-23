import os
import subprocess
import build_openssl
import build_curl
import build_aws_sdk
import build_plugin
import shutil
import glob

AWSPLUGIN_BUILD_FOLDER = os.path.dirname(os.path.abspath(__file__))

CURL_SRC_PATH = os.path.join(AWSPLUGIN_BUILD_FOLDER, "curl-7.59.0")
CURL_INSTALL_PATH = os.path.join(AWSPLUGIN_BUILD_FOLDER, "curl_build")

AWS_SDK_SRC_PATH = os.path.join(AWSPLUGIN_BUILD_FOLDER, "aws-sdk-cpp-1.4.41")
AWS_SDK_INSTALL_PATH = os.path.join(AWSPLUGIN_BUILD_FOLDER, "aws-sdk_build")
AWS_SDK_RELATIVE_RPATH_TO_ORIGIN = "./"

AWS_PLUGIG_SRC_PATH = os.path.join(AWSPLUGIN_BUILD_FOLDER, "awsplugin")
AWS_PLUGIG_INSTALL_PATH = os.path.join(
    AWSPLUGIN_BUILD_FOLDER, "awsplugin_build")
AWS_PLUGIN_RELATIVE_RPATH_TO_ORIGIN = "./"

OPENSSL_SRC_PATH = os.path.join(AWSPLUGIN_BUILD_FOLDER, "openssl-1.0.2g")
OPENSSL_INSTALL_PATH = os.path.join(AWSPLUGIN_BUILD_FOLDER, "openssl_build")

PATH_INSTALL_ALL = "AWS_PLUGIN_PKG"

# make sure config  right here

# dolphindb lib name
#DOLPHINDB_LIB = "dfs2_lib_debug"
# path of dolphindb lib
#DOLPHINDB_LIB_PATH = "/usr/local/path"
# the head file used by plugin
DOLPHINDB_INCLUDE_PATH = "/hdd/plugins_jenkins/workspace/plugin_awss3/include/"

# the compiler used here should be same with the one used by dolphindb
C_COMPILER = "/opt/gcc840/bin/gcc"
CXX_COMPILER = "/opt/gcc840/bin/g++"
# the lib used by compiler
STATIC_STDCXX_LIBPATH = "/usr/lib64"

print("openssl src path     :" + OPENSSL_SRC_PATH)
print("openssl install path :" + OPENSSL_INSTALL_PATH)
print("curl src path        :" + CURL_SRC_PATH)
print("curl install path    :" + CURL_INSTALL_PATH)
print("aws_sdk src path     :" + AWS_SDK_SRC_PATH)
print("aws_sdk install path :" + AWS_SDK_INSTALL_PATH)
print("aws_plugin src path  :" + AWS_PLUGIG_SRC_PATH)


print("-----build static openssl-----")
#build_openssl.build(OPENSSL_SRC_PATH, OPENSSL_INSTALL_PATH)

print("----build curl----")
#build_curl.build(CURL_SRC_PATH, CURL_INSTALL_PATH, OPENSSL_INSTALL_PATH)

print("----build aws-sdk----")
#build_aws_sdk.build(AWS_SDK_SRC_PATH, AWS_SDK_INSTALL_PATH,
#                    OPENSSL_INSTALL_PATH, CURL_INSTALL_PATH,
#                    CXX_COMPILER, STATIC_STDCXX_LIBPATH, AWS_SDK_RELATIVE_RPATH_TO_ORIGIN)

print("----build plugin----")
build_plugin.build(AWS_PLUGIG_SRC_PATH, AWS_PLUGIG_INSTALL_PATH, AWS_SDK_INSTALL_PATH,
                   CXX_COMPILER, AWS_PLUGIN_RELATIVE_RPATH_TO_ORIGIN,
                   DOLPHINDB_INCLUDE_PATH)

print("----install all to"+PATH_INSTALL_ALL+"----")
os.chdir(AWSPLUGIN_BUILD_FOLDER)
if os.path.exists(PATH_INSTALL_ALL):
    shutil.rmtree(PATH_INSTALL_ALL)
os.mkdir(PATH_INSTALL_ALL)
shutil.copy(AWS_PLUGIG_INSTALL_PATH+"/libPluginAWSS3.so", PATH_INSTALL_ALL)
shutil.copy(AWS_PLUGIG_INSTALL_PATH+"/PluginAWSS3.txt", PATH_INSTALL_ALL)
shutil.copy(AWS_SDK_INSTALL_PATH +
            "/lib/libaws-cpp-sdk-core.so", PATH_INSTALL_ALL)
shutil.copy(AWS_SDK_INSTALL_PATH+"/lib/libaws-cpp-sdk-s3.so", PATH_INSTALL_ALL)
for curllib in glob.glob(CURL_INSTALL_PATH+"/lib/libcurl.so*"):
    shutil.copy(curllib, PATH_INSTALL_ALL)
