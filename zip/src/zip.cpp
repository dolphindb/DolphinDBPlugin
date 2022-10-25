#include "zip.h"
#include "Util.h"

void unzipFile(const string& zipFilename, const string& outputDir, Heap* heap, FunctionDefSP function);

ConstantSP unzip(Heap* heap, vector<ConstantSP>& args) {
    // 获取压缩文件的路径，需要是绝对路径
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw RuntimeException("first argument illegal, should be string");
    }
    string zipFilename = args[0]->getString();
    if (zipFilename.empty()) {
        throw RuntimeException("first argument illegal, should not be empty string");
    }
    if (zipFilename[0] != '/') {
        throw RuntimeException("first argument illegal, should be absolute path");
    }

    // 寻找最后一个'/'出现的位置pos，截取[0, pos + 1]字符串
    int pos = zipFilename.find_last_of('/');

    // 获取输出路径，若未指定输出路径，则默认与压缩包路径相同
    string outputDir = zipFilename.substr(0, pos);
    if (args.size() > 1 && !args[1]->isNull()) {
        if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
            throw RuntimeException("second argument illegal, should be string scalar");
        }
        // if (!args[1]->getString().empty()) {
        outputDir = args[1]->getString();
        // }
        if (/*!args[1]->getString().empty() && */outputDir[0] != '/') {
            throw RuntimeException("second argument illegal, should be string scalar and absolute path");
        }
    }
    if (!outputDir.empty() && outputDir.back() != '/') {
        outputDir.push_back('/');
    }

    // 获取回调函数
    FunctionDefSP function;
    if (args.size() > 2) {
        std::cout << "form" << args[2]->getForm() << std::endl;
        std::cout << "type" << args[2]->getType() << std::endl;
        if (args[2]->getType() != DT_FUNCTIONDEF/* || args[2]->getForm() != DF_SCALAR*/) {
            throw RuntimeException("third argument illegal, should be function");
        }
        function = args[2];
    }
    
    // 实际进行解压的函数
    unzipFile(zipFilename, outputDir, heap, function);

    // 返回所有解压文件的名字
    const char *zipFile = zipFilename.c_str();
    unzFile uf = unzOpen64(zipFile);

    vector<string> filenames;
    (void)getFilenames(uf, filenames);

    int size = (int)filenames.size();
    ConstantSP ret = Util::createVector(DT_STRING, size, size);
    for (int i = 0; i < size; i++) {
        ret->setString(i, outputDir + filenames[i]);
    }

    return ret;
}

void unzipFile(const string& zipFilename, const string& outputDir, Heap* heap, FunctionDefSP function) {
    unzFile uf = NULL;
    int retValue = 0;
    const char *zipFile = zipFilename.c_str();

    uf = unzOpen64(zipFile);
    if (uf == NULL)
    {
        throw RuntimeException("Cannot open file " + zipFilename);
    }

    // 获取当前工作路径。在该函数结束时切换回原来的工作路径
    string currentDirName = get_current_dir_name();
    if (/*!outputDir.empty() && */(retValue = chdir(outputDir.c_str()))) {
        throw RuntimeException("Cannot open file " + outputDir);
    }
    retValue = do_extract(uf, 0, 1, nullptr, outputDir, heap, function);

    // chdir back to original
    retValue = chdir(currentDirName.c_str());

    unzClose(uf);

    (void)retValue;
}