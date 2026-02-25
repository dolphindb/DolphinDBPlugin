#include "Helper.h"
#include "TypeHelper.h"
#include "ScalarImp.h"
#include <cstdio>


namespace helper {

void writeFile(const char *pfilepath, const void *pbytes, int bytelen){
	if(bytelen < 1)
		return;
	FILE *pf = fopen(pfilepath,"ab");
	if(pf == NULL)
		return;
	fwrite(pbytes, bytelen, 1, pf);
	fclose(pf);
}

unsigned long getCurThreadId() {
#ifdef WINDOWS
	return GetCurrentThreadId();
#elif defined MAC
	return syscall(SYS_thread_selfid);
#else
	return pthread_self();
#endif
}


std::string strJoin(const std::string& delimiter, const std::vector<std::string>& elements) {
    if (elements.size() == 0) {
        return "";
    }
    std::string result;
    size_t total_size = 0;
    for (const auto& elem : elements) {
        total_size += elem.size();
    }
    total_size += delimiter.size() * (elements.size() - 1);

    result.reserve(total_size);

    for (size_t i = 0; i < elements.size(); ++i) {
        result += elements[i];
        if (i < elements.size() - 1) {
            result += delimiter;
        }
    }
    return result;
}


std::string str2UTF8(const std::string& input) {
    std::string output;
    size_t i = 0;
    while (i < input.size()) {
        unsigned char c = static_cast<unsigned char>(input[i]);
        if (c < 0x80) {
            output += c;
            i++;
        } 
        else if ((c & 0xE0) == 0xC0 && i + 1 < input.size() && (input[i + 1] & 0xC0) == 0x80) {
            output += input.substr(i, 2);
            i += 2;
        } 
        else if ((c & 0xF0) == 0xE0 && i + 2 < input.size() && 
                 (input[i + 1] & 0xC0) == 0x80 && (input[i + 2] & 0xC0) == 0x80) {
            output += input.substr(i, 3);
            i += 3;
        } 
        else if ((c & 0xF8) == 0xF0 && i + 3 < input.size() && 
                 (input[i + 1] & 0xC0) == 0x80 && (input[i + 2] & 0xC0) == 0x80 &&
                 (input[i + 3] & 0xC0) == 0x80) {
            output += input.substr(i, 4);
            i += 4;
        } 
        else {
            output += "\\x";
            const char hex_chars[] = "0123456789ABCDEF";
            output += hex_chars[c >> 4];
            output += hex_chars[c & 0x0F];
            i++;
        }
    }
    return output;
}


ConstantSP getRowFromTableWhere(const TableSP &t, const std::string &column, const std::string &value) {
    VectorSP name = t->getColumn(column);
    ConstantSP result = new Int(-1);
    name->find(new String(value), result);
    if (result->getIndex() == -1) {
        return converter::getConstantSP_VOID();
    }
    return t->getRow(result->getIndex());
}



} /* namespace helper */
