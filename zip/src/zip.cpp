#include "zip.h"
#include "Exceptions.h"
#include "Types.h"
#include "Util.h"
#include "zipper.h"

ConstantSP zip(Heap* heap, vector<ConstantSP>& args){
    const string usage = "Usage: zip(zipFilePath, fileOrFolderPath, [compressionLevel], [password]) ";

    if(args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, usage + "zipFilePath must be a string scalar");
    string zipFileName = args[0]->getString();

    if(args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, usage + "fileOrFolderPath must be a string scalar");
    string fileOrFolderPath = args[1]->getString();

    zipper::Zipper::zipFlags compressionLevel = static_cast<zipper::Zipper::zipFlags>(zipper::Zipper::zipFlags::Better | zipper::Zipper::zipFlags::Overwrite);
    if(args.size() > 2 && !args[2]->isNothing()){
        if(args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, usage + "compressionLevel must be a string scalar");
        string str = args[2]->getString();
        if(str != "faster" && str != "better")
            throw IllegalArgumentException(__FUNCTION__, usage + "compressionLevel must be faster or better");
        if(str == "faster")
            compressionLevel =  static_cast<zipper::Zipper::zipFlags>(zipper::Zipper::zipFlags::Faster | zipper::Zipper::zipFlags::Overwrite);
    }

    string password;
    if(args.size() > 3 && !args[3]->isNothing()){
        if(args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, usage + "password must be a string scalar");
        password = args[3]->getString();
        if(password.size() == 0)
            throw IllegalArgumentException(__FUNCTION__, usage + "password must not be empty. ");
    }
    if(password.size() > 128){
        throw IllegalArgumentException(__FUNCTION__, usage + "the length for password must be not greater than 128. ");
    }
    if(Util::exists(zipFileName)){
            throw IllegalArgumentException(__FUNCTION__, usage + zipFileName + " file already exists.");
    }
    bool isDir;
    if(!Util::exists(fileOrFolderPath, isDir)){
            throw IllegalArgumentException(__FUNCTION__, usage + fileOrFolderPath + " file does not exists.");
    }
    fileOrFolderPath = Util::replace(fileOrFolderPath, "\\", "/");
    int separatorCount = 0;
    for(int index = fileOrFolderPath.size() - 1; index >= 0; ++index){
        if(fileOrFolderPath[index] != '/')
            break;
        separatorCount++;
    }
    fileOrFolderPath = fileOrFolderPath.substr(0, fileOrFolderPath.size() - separatorCount);
    SmartPointer<zipper::Zipper> zipHandle;
    try{
        if(password != "")
            zipHandle = new zipper::Zipper(zipFileName, password);
        else
            zipHandle = new zipper::Zipper(zipFileName);
        if(!zipHandle->add(fileOrFolderPath, compressionLevel))
            throw RuntimeException("failed to zip " + fileOrFolderPath + " to " + zipFileName);
        zipHandle->close();
    }catch(exception& e){
        throw RuntimeException(ZIP_PREFIX + e.what());
    }
    return new Bool(true);
}


static ConstantSP unzipFile(const string& zipFilename, const string& outputDir, Heap* heap, const FunctionDefSP& function, zipEncode encode, const string& password);

ConstantSP unzip(Heap* heap, vector<ConstantSP>& args) {
    const string usage = "Usage: unzip(zipFilePath, [outputDir], [callback], [zipEncoding], [password])";
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "zipFilePath must be a string scalar");
    }
    string zipFilename = args[0]->getString();
    if (zipFilename.empty()) {
        throw IllegalArgumentException(__FUNCTION__, usage + "zipFilePath must not be an empty string");
    }
    if (!Util::isAbosultePath(zipFilename)) {
        throw IllegalArgumentException(__FUNCTION__, usage + "zipFilePath must be an absolute path");
    }

#ifdef _WIN32
	zipFilename = Util::replace(zipFilename, '/', '\\');
#endif
    // Get the output path. If no output path is specified, it will default to the same path as the compressed file.
    bool isDir = false;
    bool existFile = Util::exists(zipFilename, isDir);
    if(isDir || !existFile){
        throw IllegalArgumentException(__FUNCTION__, usage + "zipFilePath must be a path to an existing file");
    }
    string shortName = Util::getShortFilename(zipFilename);
    size_t pos = zipFilename.find(shortName);
    if(pos == string::npos){
        throw IllegalArgumentException(__FUNCTION__, usage + "failed to get folder Path in zipFilePath " + zipFilename);
    }
    string outputDir = zipFilename.substr(0, pos);
    if (args.size() > 1 && !args[1]->isNothing() && !args[1]->isNull()) {
        if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, usage + "outputDir must be string scalar");
        }
        outputDir = args[1]->getString();
        if (!Util::isAbosultePath(outputDir)) {
            throw IllegalArgumentException(__FUNCTION__, usage + "outputDir must be string scalar and absolute path");
        }
    }
    string winSeparator = "\\";
    string linuxSeparator = "/";
#ifdef _WIN32
	outputDir = Util::replace(outputDir, '/', '\\');
    if (!outputDir.empty() && !Util::endWith(outputDir, winSeparator) && !Util::endWith(outputDir, linuxSeparator)) {
        outputDir.push_back('\\');
    }
#else
    if (!outputDir.empty() && outputDir.back() != '/') {
        outputDir.push_back('/');
    }
#endif

    FunctionDefSP function;
    if (args.size() > 2 && !args[2]->isNothing() && !args[2]->isNull()) {
        if (args[2]->getType() != DT_FUNCTIONDEF/* || args[2]->getForm() != DF_SCALAR*/) {
            throw IllegalArgumentException(__FUNCTION__, usage + "callback must be a function");
        }
        function = args[2];
    }
    zipEncode encode = zipEncode::DEFAULT;
    if(args.size() > 3 && !args[3]->isNothing()) {
        if(args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, usage + "zipEncoding must be a string scalar");
        }
        string code = Util::lower(args[3]->getString());
        if(code == "utf-8") {
            encode = zipEncode::UTF8;
        } else if(code == "gbk") {
            encode = zipEncode::GBK;
        } else {
            throw IllegalArgumentException(__FUNCTION__, usage + "zipEncoding type must be utf-8 or gbk");
        }
    }
    string password;
    if(args.size() > 4 && !args[4]->isNothing()) {
        if(args[4]->getType() != DT_STRING || args[4]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, usage + "password must be a string scalar");
        }
        password = args[4]->getString();
    }
    if(password.size() > 128){
        throw IllegalArgumentException(__FUNCTION__, usage + "the length for password must be not greater than 128. ");
    }
    return unzipFile(zipFilename, outputDir, heap, function, encode, password);
 }
namespace unzHelper{
    class unzFileWrapper{
        public:
        unzFile uf;
        unzFileWrapper(): uf(nullptr){}
        ~unzFileWrapper(){
            if(uf != nullptr){
                unzClose(uf);
            }
        }
    };
}
ConstantSP unzipFile(const string& zipFilename, const string& outputDir, Heap* heap, const FunctionDefSP& function, zipEncode encode, const string& password) {
    const char *zipFile = zipFilename.c_str();
    unzHelper::unzFileWrapper wrapper;
    wrapper.uf = unzOpen64(zipFile);
    if (wrapper.uf == NULL)
    {
        throw RuntimeException("Cannot open \"" + zipFilename + "\" as zip file.");
    }

    if(!Util::existsDir(outputDir)) {
        string errMsg;
        Util::createDirectoryRecursive(outputDir, errMsg);
        if(!errMsg.empty()) {
            throw RuntimeException(ZIP_PREFIX + errMsg);
        }
    }

    // if(access(outputDir.c_str(), 2) != 0){
    //     for(int i = 0; i < outputDir.size(); ++i) {
    //         std::cout << (int)(unsigned char)(outputDir.c_str()[i]) << " ";
    //     }
    //     std::cout << "\n";
    //     throw RuntimeException("Cannot open outputDir " + outputDir + " in write mode");
    // }

    std::vector<string> filenames;
    //do_extract(wrapper.uf, 0, password == "" ? nullptr : password.c_str(), outputDir, heap, function, encode);
    do_extract(wrapper.uf, 0, password.c_str(), outputDir, heap, function, encode);
    unzGoToFirstFile(wrapper.uf);
    getFilenames(wrapper.uf, filenames);
    size_t size = filenames.size();
    ConstantSP ret = Util::createVector(DT_STRING, size, size);
    for (size_t i = 0; i < size; i++) {
        string fileTmp = filenames[i];
    #ifdef WIN32
        fileTmp = Util::replace(fileTmp, '/', '\\');
        if(encode == zipEncode::DEFAULT || encode == zipEncode::GBK) {
            fileTmp = gbkToUtf8(heap, fileTmp);
        }
    #else
        if(encode == zipEncode::GBK) {
            fileTmp = gbkToUtf8(heap, fileTmp);
        }
    #endif
        ret->setString(i, outputDir + fileTmp);
    }
    return ret;
}