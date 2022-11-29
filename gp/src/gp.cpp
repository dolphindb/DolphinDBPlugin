#include"gp.h"

using namespace std;
double *DataPtr[1024][6];
extern int dataIndex;
int dataSize[1024];
int dataLen[1024];
int blockIndex;
int blockSize;
char commandBuffer[1024];

int mIndex = 1024;
std::unordered_set<std::string> styleSet;

string style;
vector<string> titleVector;
vector<string> lineColorVec;
vector<double> lineWidthVec;
vector<int> pointTypeVec;
vector<double> pointSizeVec;
vector<string> smoothVec;
extern double pointsize;
std::string lineColor;
std::string smooth;

//std::pair<double,double> size;
void styleSetInit() {
    styleSet.insert("line");
    styleSet.insert("point");
    styleSet.insert("linespoint");
    styleSet.insert("impulses");
    styleSet.insert("dots");
    styleSet.insert("step");
    styleSet.insert("errorbars");
    styleSet.insert("histogram");
    styleSet.insert("boxes");
    styleSet.insert("boxerrorbars");
    styleSet.insert("ellipses");
    styleSet.insert("circles");
}

extern "C" int gpInit(int argc, char **argv);

extern "C" char *gpgetCommand(char *buffer, size_t len) {
    strncpy(buffer, commandBuffer, strlen(commandBuffer) + 1);
    buffer[strlen(commandBuffer)] = '\0';
    return buffer;
}

extern "C" int com_line();

extern "C" void gpSetCommand(std::string commad) {
    const char *origin = commad.c_str();
    strncpy(commandBuffer, origin, strlen(origin));
    commandBuffer[strlen(origin)] = '\0';
    try {
        com_line();
    }
    catch (char *e) {
        throw RuntimeException(e);
    }
}

void gpSetPath(ConstantSP &path) {
    gpSetCommand("set terminal postscript eps color solid ");
    gpSetCommand("set output \"" + std::string(path->getString()) + "\"");
    gpSetCommand("set style fill transparent solid 0.4");
}

void gpSetStyle(std::string args) {
    if (styleSet.count(args) == 0) {
        std::string tmp;
        for (std::unordered_set<std::string>::iterator it = styleSet.begin(); it != styleSet.end(); ++it) {
            tmp += ", " + (*it);
        }
        throw RuntimeException("Style must be " + tmp.substr(2));
    }
}

std::string convertScalarDataToString(ConstantSP &data, std::string &varName) {
    if (data->getForm() != DF_SCALAR) {
        throw RuntimeException(varName + "must be a digital scalar");
    }
    std::string ret;
    if (data->getType() == DT_CHAR) {
        char tmp = data->getChar();
        ret = std::to_string(tmp);
    } else if (data->getType() == DT_SHORT) {
        short tmp = data->getShort();
        ret = std::to_string(tmp);
    } else if (data->getType() == DT_INT) {
        int tmp = data->getInt();
        ret = std::to_string(tmp);
    } else if (data->getType() == DT_LONG) {
        long long tmp = data->getLong();
        ret = std::to_string(tmp);
    } else if (data->getType() == DT_FLOAT) {
        float tmp = data->getFloat();
        ret = std::to_string(tmp);
    } else if (data->getType() == DT_DOUBLE) {
        double tmp = data->getDouble();
        ret = std::to_string(tmp);
    } else
        throw RuntimeException(varName + " must be a digital scalar");
    return ret;
}

std::vector<std::string> convertVectorDataToString(ConstantSP &data, int requiredDataSize, std::string &varName) {
    if (data->getForm() != DF_VECTOR) {
        throw RuntimeException(varName + "must be a digital vector");
    }
    if (data->size() < requiredDataSize) {
        throw RuntimeException(varName + "require " + std::to_string(requiredDataSize) + " elements");
    }
    std::vector<std::string> ret;
    if (data->getType() == DT_CHAR) {
        char buffer[2];
        data->getChar(0, requiredDataSize, buffer);
        ret.push_back(std::to_string(buffer[0]));
        ret.push_back(std::to_string(buffer[1]));
    } else if (data->getType() == DT_SHORT) {
        short buffer[2];
        data->getShort(0, requiredDataSize, buffer);
        ret.push_back(std::to_string(buffer[0]));
        ret.push_back(std::to_string(buffer[1]));
    } else if (data->getType() == DT_INT) {
        int buffer[2];
        data->getInt(0, requiredDataSize, buffer);
        ret.push_back(std::to_string(buffer[0]));
        ret.push_back(std::to_string(buffer[1]));
    } else if (data->getType() == DT_LONG) {
        long long buffer[2];
        data->getLong(0, requiredDataSize, buffer);
        ret.push_back(std::to_string(buffer[0]));
        ret.push_back(std::to_string(buffer[1]));
    } else if (data->getType() == DT_FLOAT) {
        float buffer[2];
        data->getFloat(0, requiredDataSize, buffer);
        ret.push_back(std::to_string(buffer[0]));
        ret.push_back(std::to_string(buffer[1]));
    } else if (data->getType() == DT_DOUBLE) {
        double buffer[2];
        data->getDouble(0, requiredDataSize, buffer);
        ret.push_back(std::to_string(buffer[0]));
        ret.push_back(std::to_string(buffer[1]));
    } else
        throw RuntimeException(varName + "must be a digital vector");
    return ret;
}

int convertToInt(ConstantSP &data, std::string &varName) {
    if (data->getForm() != DF_SCALAR) {
        throw RuntimeException(varName + "must be a digital scalar");
    }
    int ret;
    if (data->getType() == DT_CHAR) {
        char tmp = data->getChar();
        ret = tmp;
    } else if (data->getType() == DT_SHORT) {
        short tmp = data->getShort();
        ret = tmp;
    } else if (data->getType() == DT_INT) {
        int tmp = data->getInt();
        ret = tmp;
    } else
        throw RuntimeException(varName + " must be a scalar with type is char, short, int");
    return ret;
}

double convertToDouble(ConstantSP &data, std::string &varName) {
    if (data->getForm() != DF_SCALAR) {
        throw RuntimeException(varName + "must be a digital scalar");
    }
    double ret;
    if (data->getType() == DT_CHAR) {
        char tmp = data->getChar();
        ret = tmp;
    } else if (data->getType() == DT_SHORT) {
        short tmp = data->getShort();
        ret = tmp;
    } else if (data->getType() == DT_INT) {
        int tmp = data->getInt();
        ret = tmp;
    } else if (data->getType() == DT_LONG) {
        long long tmp = data->getLong();
        ret = tmp;
    } else if (data->getType() == DT_FLOAT) {
        float tmp = data->getFloat();
        ret = tmp;
    } else if (data->getType() == DT_DOUBLE) {
        double tmp = data->getDouble();
        ret = tmp;
    } else
        throw RuntimeException(varName + " must be a digital scalar");
    return ret;
}

void gpSetProps(ConstantSP &props) {
    ConstantSP keys = props->keys();
    for (int i = 0; i < keys->size(); i++) {
        ConstantSP key = keys->get(i);
        std::string strKey = key->getString();
        ConstantSP value = props->getMember(key);
        if (strKey == "title") {
            if (value->getForm() == DF_VECTOR && value->getType() == DT_STRING) {
                int size = value->size();
                char *buffer[size];
                bool flag = value->getString(0, size, buffer);
                if (flag) {
                    for (int i = 0; i < size; ++i) {
                        titleVector.push_back(buffer[i]);
                    }
                }
            } else if (value->getForm() == DF_SCALAR && value->getType() == DT_STRING) {
                titleVector.push_back(value->getString());
            } else {
                throw RuntimeException("title must be a string vector or a string scalar");
            }
        } else if (strKey == "xRange") {
            if (value->getForm() == DF_VECTOR) {
                std::vector<std::string> ret = convertVectorDataToString(value, 2, strKey);
                std::string tmp = "set xrange [" + ret[0] + ":" + ret[1] + "]";
                gpSetCommand(tmp);
            } else {
                throw RuntimeException("xRange must be a vector");
            }
        } else if (strKey == "yRange") {
            if (value->getForm() == DF_VECTOR) {
                std::vector<std::string> ret = convertVectorDataToString(value, 2, strKey);
                std::string tmp = "set yrange [" + ret[0] + ":" + ret[1] + "]";
                gpSetCommand(tmp);
            } else
                throw RuntimeException("yRange must be a vector");
        } else if (strKey == "xLabel") {
            if (value->getForm() == DF_SCALAR && value->getType() == DT_STRING) {
                std::string tmp = "set xlabel \'" + value->getString() + "\'";
                gpSetCommand(tmp);
            } else
                throw RuntimeException("xlabel must be a string scalar");
        } else if (strKey == "yLabel") {
            if (value->getForm() == DF_SCALAR && value->getType() == DT_STRING) {
                std::string tmp = "set ylabel \'" + value->getString() + "\'";
                gpSetCommand(tmp);
            } else
                throw RuntimeException("yLabel must be a string scalar");
        } else if (strKey == "size") {
            if (value->getForm() == DF_VECTOR) {
                if (value->size() < 2)
                    throw RuntimeException("size must be a digital vector, has two elements");
                std::vector<std::string> ret = convertVectorDataToString(value, 2, strKey);
                std::string tmp = "set size " + ret[0] + "," + ret[1];
                gpSetCommand(tmp);
            } else
                throw RuntimeException("size must be a digital vector, has two elements");
        } else if (strKey == "xTics") {
            std::string tmp = "set xtics ";
            std::string ret = convertScalarDataToString(value, strKey);
            tmp += ret;
            gpSetCommand(tmp);
        } else if (strKey == "yTics") {
            std::string tmp = "set ytics ";
            std::string ret = convertScalarDataToString(value, strKey);
            tmp += ret;
            gpSetCommand(tmp);
        } else if (strKey == "lineColor") {
            if (value->getForm() == DF_SCALAR) {
                lineColorVec.push_back(value->getString());
            } else if (value->getForm() == DF_VECTOR) {
                int size = value->size();
                char *buffer[size];
                VectorSP valueVector = value;
                char **p = valueVector->getStringConst(0, size, buffer);
                for (int i = 0; i < size; ++i) {
                    lineColorVec.push_back(string(p[i]));
                }
            } else {
                throw RuntimeException("lineColor must be a string scalar or a string vector");
            }
        } else if (strKey == "lineWidth") {
            if (value->getForm() == DF_SCALAR) {
                lineWidthVec.push_back(value->getDouble());
            } else if (value->getForm() == DF_VECTOR) {
                int size = value->size();
                double buffer[size];
                VectorSP valueVector = value;
                double *p = valueVector->getDoubleBuffer(0, size, buffer);
                lineWidthVec = vector<double>(p, p + size);
            } else {
                throw RuntimeException("lineWidth must be a double scalar or a double vector");
            }
        } else if (strKey == "pointType") {
            if (value->getForm() == DF_SCALAR) {
                pointTypeVec.push_back(value->getInt());
            } else if (value->getForm() == DF_VECTOR) {
                int size = value->size();
                int buffer[size];
                VectorSP valueVector = value;
                int *p = valueVector->getIntBuffer(0, size, buffer);
                pointTypeVec = vector<int>(p, p + size);
            } else {
                throw RuntimeException("pointType must be a int scalar or a int vector");
            }
        } else if (strKey == "pointSize") {
            if (value->getForm() == DF_SCALAR) {
                pointSizeVec.push_back(value->getDouble());
            } else if (value->getForm() == DF_VECTOR) {
                int size = value->size();
                double buffer[size];
                VectorSP valueVector = value;
                double *p = valueVector->getDoubleBuffer(0, size, buffer);
                pointTypeVec = vector<int>(p, p + size);
            } else {
                throw RuntimeException("pointSize must be a double scalar or a double vector");
            }
        } else if (strKey == "smooth") {
            if (value->getForm() == DF_SCALAR) {
                smoothVec.push_back(value->getString());
            } else if (value->getForm() == DF_VECTOR) {
                int size = value->size();
                char *buffer[size];
                VectorSP valueVector = value;
                char **p = valueVector->getStringConst(0, size, buffer);
                for (int i = 0; i < size; ++i) {
                    string tmp = string(p[i]);
                    if (tmp == "csplines")
                        smooth = tmp;
                    else if (tmp == "bezier") {
                        smooth = tmp;
                    } else
                        throw RuntimeException("smooth must be bezier or csplines");
                    lineColorVec.push_back(string(p[i]));
                }
            } else {
                throw RuntimeException("smooth must be a string scalar or a string vector");
            }
        } else {
            throw RuntimeException("the porp " + strKey + " doesn't exist");
        }
    }
}

void convertData(ConstantSP &data, int blockIndex, int colIndex, std::shared_ptr<double> &ptr) {
    int subDataSize = data->size();
    double *buffer = new double[subDataSize];
    ptr = std::shared_ptr<double>(buffer);
    if (data->getType() == DT_CHAR) {
        int frep = subDataSize / mIndex;
        int index = 0;
        char tmpBuffer[1024];
        for (int i = 0; i < frep; ++i) {
            char *p = data->getCharBuffer(index, mIndex, tmpBuffer);
            for (int j = 0; j < mIndex; ++j) {
                if (p[j] == CHAR_MIN)
                    throw RuntimeException("The plot data can't be null");
                buffer[index + j] = p[j];
            }
            index += mIndex;
        }
        int legacy = subDataSize % mIndex;
        char *p = data->getCharBuffer(index, legacy, tmpBuffer);
        for (int j = 0; j < legacy; ++j) {
            if (p[j] == CHAR_MIN)
                throw RuntimeException("The plot data can't be null");
            buffer[index + j] = p[j];
        }
        DataPtr[blockIndex][colIndex] = buffer;
    } else if (data->getType() == DT_SHORT) {
        int frep = subDataSize / mIndex;
        int index = 0;
        short tmpBuffer[1024];
        for (int i = 0; i < frep; ++i) {
            short *p = data->getShortBuffer(index, mIndex, tmpBuffer);
            for (int j = 0; j < mIndex; ++j) {
                if (p[j] == SHRT_MIN)
                    throw RuntimeException("The plot data can't be null");
                buffer[index + j] = p[j];
            }
            index += mIndex;
        }
        int legacy = subDataSize % mIndex;
        short *p = data->getShortBuffer(index, legacy, tmpBuffer);
        for (int j = 0; j < legacy; ++j) {
            if (p[j] == SHRT_MIN)
                throw RuntimeException("The plot data can't be null");
            buffer[index + j] = p[j];
        }
        DataPtr[blockIndex][colIndex] = buffer;
    } else if (data->getType() == DT_INT) {
        int frep = subDataSize / mIndex;
        int index = 0;
        int tmpBuffer[1024];
        for (int i = 0; i < frep; ++i) {
            int *p = data->getIntBuffer(index, mIndex, tmpBuffer);
            for (int j = 0; j < mIndex; ++j) {
                if (p[j] == INT_MIN)
                    throw RuntimeException("The plot data can't be null");
                buffer[index + j] = p[j];
            }
            index += mIndex;
        }
        int legacy = subDataSize % mIndex;
        int *p = data->getIntBuffer(index, legacy, tmpBuffer);
        for (int j = 0; j < legacy; ++j) {
            if (p[j] == INT_MIN)
                throw RuntimeException("The plot data can't be null");
            buffer[index + j] = p[j];
        }
        DataPtr[blockIndex][colIndex] = buffer;
    } else if (data->getType() == DT_LONG) {
        int frep = subDataSize / mIndex;
        int index = 0;
        long long tmpBuffer[1024];
        for (int i = 0; i < frep; ++i) {
            long long *p = data->getLongBuffer(index, mIndex, tmpBuffer);
            for (int j = 0; j < mIndex; ++j) {
                if (p[j] == LONG_MIN)
                    throw RuntimeException("The plot data can't be null");
                buffer[index + j] = p[j];
            }
            index += mIndex;
        }
        int legacy = subDataSize % mIndex;
        long long *p = data->getLongBuffer(index, legacy, tmpBuffer);
        for (int j = 0; j < legacy; ++j) {
            if (p[j] == LONG_MIN)
                throw RuntimeException("The plot data can't be null");
            buffer[index + j] = p[j];
        }
        DataPtr[blockIndex][colIndex] = buffer;
    } else if (data->getType() == DT_FLOAT) {
        int frep = subDataSize / mIndex;
        int index = 0;
        float tmpBuffer[1024];
        for (int i = 0; i < frep; ++i) {
            float *p = data->getFloatBuffer(index, mIndex, tmpBuffer);
            for (int j = 0; j < mIndex; ++j) {
                if (p[j] == FLT_NMIN)
                    throw RuntimeException("The plot data can't be null");
                buffer[index + j] = p[j];
            }
            index += mIndex;
        }
        int legacy = subDataSize % mIndex;
        float *p = data->getFloatBuffer(index, legacy, tmpBuffer);
        for (int j = 0; j < legacy; ++j) {
            if (p[j] == FLT_NMIN)
                throw RuntimeException("The plot data can't be null");
            buffer[index + j] = p[j];
        }
        DataPtr[blockIndex][colIndex] = buffer;
    } else if (data->getType() == DT_DOUBLE) {
        double *p = data->getDoubleBuffer(0, subDataSize, buffer);
        for (int i = 0; i < subDataSize; ++i) {
            if (p[i] == DBL_NMIN)
                throw RuntimeException("The plot data can't be null");
        }
        DataPtr[blockIndex][colIndex] = p;
    } else
        throw RuntimeException("the elements vector must be char, short, int, long, float, double");
}

//set the data, plot
//one point one line, lineCount = cols * rows
void gpSetData(ConstantSP &data, std::string style) {
    if (data->getType() == DT_ANY && data->getForm() == DF_VECTOR) {
        std::string tmp =
                "plot '-' with " + style + (titleVector.size() > 0 ? (" title '" + titleVector[0] + "'") : " notitle");
        if (lineColorVec.size() >= 1)
            tmp += " lc rgb \"" + lineColorVec[0] + "\"";
        if (lineWidthVec.size() >= 1)
            tmp += " lw " + std::to_string(lineWidthVec[0]);
        if (pointTypeVec.size() >= 1)
            tmp += " pt " + std::to_string(pointTypeVec[0]);
        if (pointSizeVec.size() >= 1)
            tmp += " ps " + std::to_string(pointSizeVec[0]);
        if (smoothVec.size() >= 1)
            tmp += " smooth " + smoothVec[0];
        size_t cols = data->size();
        for (size_t i = 1; i < cols; ++i) {
            tmp += (", '-' with " + style +
                    ((titleVector.size() >= i) ? (" title '" + titleVector[i] + "'") : " notitle"));
            if (lineColorVec.size() >= i)
                tmp += " lc rgb \"" + lineColorVec[i] + "\"";
            if (lineWidthVec.size() >= i)
                tmp += " lw " + std::to_string(lineWidthVec[i]);
            if (pointTypeVec.size() >= i)
                tmp += " pt " + std::to_string(pointTypeVec[i]);
            if (pointSizeVec.size() >= i)
                tmp += " ps " + std::to_string(pointSizeVec[i]);
            if (smoothVec.size() >= i)
                tmp += " smooth " + smoothVec[i];
        }
        std::shared_ptr<double> ptr[cols * 2];
        for (size_t i = 0; i < cols; ++i) {
            ConstantSP sp = data->get(i);
            if (sp->getForm() == DF_VECTOR) {
                int subDataSize = sp->size();
                dataSize[i] = subDataSize;
                dataLen[i] = 1;
                convertData(sp, i, 0, ptr[i]);
            } else if (sp->getForm() == DF_TABLE) {
                if(cols>=1){
                    ConstantSP data1 = sp->getColumn(0);
                    int size = data1->size();
                    convertData(data1, i, 0, ptr[i]);
                    dataSize[i] = size;
                    dataLen[i] = 1;
                }else if(cols>=2){
                    ConstantSP data2 = sp->getColumn(1);
                    convertData(data2, i, 1, ptr[i + cols]);
                    dataLen[i] = 2;
                }

            } else {
                throw RuntimeException(
                        "The elements of a vector have to be vectors, tables,char scalar, short scalar, int scalar, long scalar, float scalar, double scalar");
            }
        }
        dataIndex = 0;
        blockIndex = 0;
        blockSize = cols;
        gpSetCommand(tmp);
    } else if (data->getForm() == DF_VECTOR) {
        int cols = data->size();
        std::string tmp =
                "plot '-' with " + style + (titleVector.size() > 0 ? (" title '" + titleVector[0] + "'") : " notitle");
        if (lineColorVec.size() >= 1)
            tmp += " lc rgb \"" + lineColorVec[0] + "\"";
        if (lineWidthVec.size() >= 1)
            tmp += " lw " + std::to_string(lineWidthVec[0]);
        if (pointTypeVec.size() >= 1)
            tmp += " pt " + std::to_string(pointTypeVec[0]);
        if (pointSizeVec.size() >= 1)
            tmp += " ps " + std::to_string(pointSizeVec[0]);
        if (smoothVec.size() >= 1)
            tmp += " smooth " + smoothVec[0];
        std::shared_ptr<double> ptr;
        convertData(data, 0, 0, ptr);
        dataIndex = 0;
        dataSize[0] = cols;
        dataLen[0] = 1;
        blockIndex = 0;
        blockSize = 1;
        gpSetCommand(tmp);
    } else if (data->getForm() == DF_TABLE) {
        TableSP sp = (TableSP) data;
        std::string tmp =
                "plot '-' with " + style + (titleVector.size() > 0 ? (" title '" + titleVector[0] + "'") : " notitle");
        if (lineColorVec.size() >= 1)
            tmp += " lc rgb \"" + lineColorVec[0] + "\"";
        if (lineWidthVec.size() >= 1)
            tmp += " lw " + std::to_string(lineWidthVec[0]);
        if (pointTypeVec.size() >= 1)
            tmp += " pt " + std::to_string(pointTypeVec[0]);
        if (pointSizeVec.size() >= 1)
            tmp += " ps " + std::to_string(pointSizeVec[0]);
        if (smoothVec.size() >= 1)
            tmp += " smooth " + smoothVec[0];
        int cows = ((TableSP)sp)->columns();
        std::shared_ptr<double> ptr[2];
        if(cows>=1){
            ConstantSP data1 = sp->getColumn(0);
            convertData(data1, 0, 0, ptr[0]);
            dataSize[0] = data1->size();
            dataLen[0] = 1;
        }
        if(cows>=2){
            ConstantSP data2 = sp->getColumn(1);
            convertData(data2, 0, 1, ptr[1]);
            dataLen[0] = 1;
        }
        dataIndex = 0;
        blockIndex = 0;
        blockSize = 1;
        gpSetCommand(tmp);
    }
}

void onePlot() {
    lineColorVec.clear();
    lineWidthVec.clear();
    pointTypeVec.clear();
    pointSizeVec.clear();
    smoothVec.clear();
    gpSetCommand("unset xrange");
    gpSetCommand("set xrange");
    gpSetCommand("unset yrange");
    gpSetCommand("set yrange");
    gpSetCommand("unset size");
    gpSetCommand("unset xtics");
    gpSetCommand("set xtics");
    gpSetCommand("unset ytics");
    gpSetCommand("set ytics");
    gpSetCommand("unset xlabel");
    gpSetCommand("unset ylabel");
    gpSetCommand("unset style");
}

bool isGpInit = true;
Mutex mLock;
Mutex wLock;

class pLock {
public:
    pLock() {
        wLock.lock();
    }

    ~pLock() {
        wLock.unlock();
    }
};

ConstantSP gpPlot(Heap *heap, vector<ConstantSP> &args) {
    pLock t;
    if (args[0]->getForm() != DF_VECTOR && args[0]->getForm() != DF_TABLE) {
        throw RuntimeException("Data must be a  vector or a table");
    }
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw RuntimeException("Style must be a string scalar");
    }
    if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR) {
        throw RuntimeException("Path must be a string scalar");
    }
    if (args.size() > 3) {
        if (args[3]->getType() != DT_ANY || args[3]->getForm() != DF_DICTIONARY) {
            throw RuntimeException("Props must be a dictionary");
        }
    }
    if (isGpInit) {
        mLock.lock();
        if (isGpInit) {
            gpInit(0, NULL);
            styleSetInit();
            isGpInit = false;
        }
        mLock.unlock();
    }
    onePlot();
    titleVector.clear();
    try {
        gpSetPath(args[2]);
    }
    catch (char *e) {
        throw RuntimeException(e);
    }
    gpSetStyle(args[1]->getString());
    if (args.size() > 3)
        gpSetProps(args[3]);
    gpSetData(args[0], args[1]->getString());
    return new Bool(true);

}