#include "zip.h"
#include "Exceptions.h"
#include "Types.h"
#include "Util.h"


static ConstantSP unzipFile(const string& zipFilename, const string& outputDir, Heap* heap, const FunctionDefSP& function, zipEncode encode);

ConstantSP unzip(Heap* heap, vector<ConstantSP>& args) {
    const string usage = "Usage: unzip(zipFileName, [outputDir], [callback], [zipEncode])";
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "zipFileName must be a string scalar");
    }
    string zipFilename = args[0]->getString();
    if (zipFilename.empty()) {
        throw IllegalArgumentException(__FUNCTION__, usage + "zipFileName must not be an empty string");
    }
    if (!Util::isAbosultePath(zipFilename)) {
        throw IllegalArgumentException(__FUNCTION__, usage + "zipFileName must be an absolute path");
    }

#ifdef WINDOWS
	zipFilename = Util::replace(zipFilename, '/', '\\');
#endif
    // Get the output path. If no output path is specified, it will default to the same path as the compressed file.
    bool isDir = false;
    bool existFile = Util::exists(zipFilename, isDir);
    if(isDir || !existFile){
        throw IllegalArgumentException(__FUNCTION__, usage + "zipFileName must be a path to an existing file");
    }
    string shortName = Util::getShortFilename(zipFilename);
    size_t pos = zipFilename.find(shortName);
    if(pos == string::npos){
        throw IllegalArgumentException(__FUNCTION__, usage + "failed to get folder Path in zipFileName " + zipFilename);
    }
    string outputDir = zipFilename.substr(0, pos);
    if (args.size() > 1 && !args[1]->isNull()) {
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
#ifdef WINDOWS
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
    if (args.size() > 2 && !args[2]->isNull()) {
        if (args[2]->getType() != DT_FUNCTIONDEF/* || args[2]->getForm() != DF_SCALAR*/) {
            throw IllegalArgumentException(__FUNCTION__, usage + "callback must be a function");
        }
        function = args[2];
    }
    zipEncode encode = zipEncode::DEFAULT;
    if(args.size() > 3) {
        if(args[3]->getType() != DT_STRING && args[3]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, usage + "zipEncode must be a string scalar");
        }
        string code = Util::lower(args[3]->getString());
        if(code == "utf-8") {
            encode = zipEncode::UTF8;
        } else if(code == "gbk") {
            encode = zipEncode::GBK;
        } else {
            throw IllegalArgumentException(__FUNCTION__, usage + "zipEncode type must be utf-8 or gbk");
        }
    }

    return unzipFile(zipFilename, outputDir, heap, function, encode);
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
ConstantSP unzipFile(const string& zipFilename, const string& outputDir, Heap* heap, const FunctionDefSP& function, zipEncode encode) {
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

    do_extract(wrapper.uf, 0, 1, nullptr, outputDir, heap, function, encode);
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