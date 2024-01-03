#include"gp.h"
// # define DEBUG_GP
using namespace std;
double *DataPtr[1024][6];
ConstantSP ConstantData[1024][6];
extern int dataIndex;
//data rows
int dataSize[1024];
//data cols
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
int resolution[2];

extern "C" int setRowData(char* str, int column){
    string origin = ConstantData[blockIndex][column]->getString(dataIndex);
    // std::cout<<"blockIndex"<<blockIndex<<"column"<<column<<"dataIndex"<<dataIndex<<std::endl;
    // std::cout<<origin<<std::endl;
    strncpy(str, origin.c_str(), strlen(origin.c_str()) + 1);
    return strlen(str);
}

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
}

extern "C" int gpInit(int argc, char **argv);

extern "C" char *gpgetCommand(char *buffer, size_t len) {
    strncpy(buffer, commandBuffer, strlen(commandBuffer) + 1);
    buffer[strlen(commandBuffer)] = '\0';
    return buffer;
}

extern "C" int com_line();

extern "C" void gpSetCommand(std::string commad) {
    if(commad.size() > 1023)
        throw RuntimeException("Too many lines were drawn and the command transferred to gnuplot was too long. ");
    const char *origin = commad.c_str();
    strncpy(commandBuffer, origin, strlen(origin));
    commandBuffer[strlen(origin)] = '\0';
    #ifdef DEBUG_GP
    cout<<commad<<endl;
    #endif
    try {
        com_line();
    }
    catch (char *e) {
        throw RuntimeException(e);
    }
}

void gpSetPath(ConstantSP &path) {
    string pathStr = path->getString();
    if(Util::endWith(pathStr, ".png")){
        string tmp = "size 640, 480";
        if(resolution[0] != 0 && resolution[1] != 0){
            tmp = "size " + std::to_string(resolution[0]) + ", " + std::to_string(resolution[1]);
        }
        gpSetCommand("set terminal png " + tmp);
        gpSetCommand("set output \"" + std::string(pathStr) + "\"");
    }else if(Util::endWith(pathStr, ".jpeg")){
        string tmp = "size 640, 480";
        if(resolution[0] != 0 && resolution[1] != 0){
            tmp = "size " + std::to_string(resolution[0]) + ", " + std::to_string(resolution[1]);
        }
        gpSetCommand("set terminal jpeg " + tmp);
        gpSetCommand("set output \"" + std::string(pathStr) + "\"");
    }else{
        gpSetCommand("set terminal postscript eps color solid ");
        gpSetCommand("set output \"" + std::string(pathStr) + "\"");
        gpSetCommand("set style fill transparent solid 0.4");
    }
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
            if(value->getType() != DT_STRING)
                throw RuntimeException("lineColor must be a string scalar or a string vector");
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
            if(!value->isNumber())
                throw RuntimeException("lineWidth must be a number scalar or a number vector");
            if (value->getForm() == DF_SCALAR) {
                lineWidthVec.push_back(value->getDouble());
            } else if (value->getForm() == DF_VECTOR) {
                int size = value->size();
                double buffer[size];
                VectorSP valueVector = value;
                if(!valueVector->getDouble(0, size, buffer));
                lineWidthVec = vector<double>(buffer, buffer + size);
            } else {
                throw RuntimeException("lineWidth must be a double scalar or a double vector");
            }
        } else if (strKey == "pointType") {
            if(value->getType() != DT_INT)
                throw RuntimeException("pointType must be a int scalar or a int vector");
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
            if(!value->isNumber())
                throw RuntimeException("pointSize must be a number scalar or a number vector");
            if (value->getForm() == DF_SCALAR) {
                double pointSize = value->getDouble();
                if(pointSize < 0)
                    throw RuntimeException("pointSize must be greater than 0");
                pointSizeVec.push_back(value->getDouble());
            } else if (value->getForm() == DF_VECTOR) {
                int size = value->size();
                double buffer[size];
                VectorSP valueVector = value;
                valueVector->getDouble(0, size, buffer);
                pointSizeVec = vector<double>(buffer, buffer + size);
                for(int i = 0; i < size; ++i){
                    if(pointSizeVec[i] < 0)
                        throw RuntimeException("pointSize must be greater than 0");
                }
            } else {
                throw RuntimeException("pointSize must be a double scalar or a double vector");
            }
        } else if (strKey == "smooth") {
            int size = 0;
            if(value->getType() != DT_STRING)
                throw RuntimeException("smooth must be a string scalar or a string vector");
            if (value->getForm() == DF_SCALAR) {
                size = 1;
            } else if (value->getForm() == DF_VECTOR) {
                size = value->size();
            } else {
                throw RuntimeException("smooth must be a string scalar or a string vector");
            }
            char *buffer[size];
            VectorSP valueVector = value;
            char **p = valueVector->getStringConst(0, size, buffer);
            for (int i = 0; i < size; ++i) {
                string tmp = string(p[i]);
                if (tmp != "csplines" && tmp != "bezier") 
                    throw RuntimeException("smooth must be bezier or csplines");
                smoothVec.push_back(tmp);
            }
        } else if (strKey == "resolution"){
            if(value->getType() != DT_INT || value->getForm() != DF_VECTOR){
                throw RuntimeException("resolution must be a int vector.");
            }
            VectorSP data = value;
            if(data->rows() != 2)
                throw RuntimeException("the size of resolution must be equal than 2. ");
            resolution[0] = data->getInt(0);
            resolution[1] = data->getInt(1);
            if(resolution[0] <= 0 || resolution[1] <= 0){
                throw RuntimeException("resolution must be greater than 0. ");
            }
        }
        else {
            throw RuntimeException("the porp " + strKey + " doesn't exist");
        }
    }
}

void convertData(ConstantSP &data, int blockIndex, int colIndex, DATA_TYPE& lastType, const string& axesString) {
    DATA_TYPE currentType = data->getType();
    if(Util::getCategory(currentType) == DATA_CATEGORY::TEMPORAL && lastType != DT_DOUBLE && currentType != currentType){
        throw RuntimeException("Time-type data must be of the same data type.");
    }
    int rows = data->rows();
    for(int i = 0; i < rows; ++i){
        if(data->isNull(i))
            throw RuntimeException("The plot data can't be null");
    }
    string format;
    switch (currentType)
    {
    case DT_CHAR:
    case DT_SHORT:
    case DT_INT:
    case DT_LONG:
    case DT_FLOAT:
    case DT_DOUBLE:
        break;
    case DT_DATE:
        format = "%Y.%m.%d";
        break;
    case DT_MINUTE:
        format = "%H:%Mm";
        break;
    case DT_SECOND:
        format = "%H:%M:%S";
        break;
    case DT_DATETIME:
        format = "%Y.%m.%dT%H:%M:%S";
        break;
    case DT_DATEHOUR:
        format = "%Y.%m.%dT%H";
        break;
    default:
        throw RuntimeException("The data type " + std::to_string(currentType) + " is not supported yet");
    }
    if(!format.empty()){
        lastType = currentType;
        gpSetCommand("set timefmt \"" + format + "\"");
        gpSetCommand("set format " + axesString + " \"" + format + "\"");
        if(!axesString.empty()) gpSetCommand("set " + axesString + "data time");
    }
    ConstantData[blockIndex][colIndex] = data;
}

string getPlotConfig(size_t index){
    string tmp;
    tmp += titleVector.size() > index ? (" title '" + titleVector[index] + "'") : " notitle";
    tmp += lineColorVec.size() > index ? " lc rgb \"" + lineColorVec[index] + "\"" : "";
    tmp += lineWidthVec.size() > index ? " lw " + std::to_string(lineWidthVec[index]) : "";
    tmp += pointTypeVec.size() > index ? " pt " + std::to_string(pointTypeVec[index]) : "";
    tmp += pointSizeVec.size() > index ? " ps " + std::to_string(pointSizeVec[index]) : "";
    tmp += smoothVec.size() > index ? " smooth " + smoothVec[index] : "";
    return tmp;
}

string gpSetData(ConstantSP data, const std::string& style, int blockIndex, DATA_TYPE& xDataType, DATA_TYPE yDataType){
    string plotString;
    if (data->getForm() == DF_VECTOR){
        plotString += " '-' using 1 with " + style + getPlotConfig(blockIndex);
        convertData(data, blockIndex, 0, yDataType, "y");
        int rows = data->rows();
        dataSize[blockIndex] = rows;
        dataLen[blockIndex] = 1;
        return plotString;
    }else if (data->getForm() == DF_TABLE){
        int cols = data->columns();
        if(cols == 1){
            return gpSetData(data->getColumn(0), style, blockIndex, xDataType, yDataType);
        }else{
            plotString += " '-' using 1:2 with " + style + getPlotConfig(blockIndex);
            
            ConstantSP data1 = data->getColumn(0);
            ConstantSP data2 = data->getColumn(1);
            convertData(data1, blockIndex, 0, xDataType, "x");
            convertData(data2, blockIndex, 1, xDataType, "y");
            dataSize[blockIndex] = data1->size();
            dataLen[blockIndex] = 2;
            return plotString;
        }
    }else{
        throw RuntimeException("The data for plotting must be a vector or a table. ");
    }
}

//set the data, plot
//one point one line, lineCount = cols * rows
void gpSetData(ConstantSP &data, std::string style) {
    DATA_TYPE xDataType = DT_DOUBLE;
    DATA_TYPE yDataType = DT_DOUBLE;
    string plotString = "plot ";
    if (data->getType() == DT_ANY && data->getForm() == DF_VECTOR) {
        size_t size = data->rows();
        for (size_t i = 0; i < size; ++i) {
            ConstantSP sp = data->get(i);
            if (sp->getForm() == DF_VECTOR || sp->getForm() == DF_TABLE) {
                int subDataSize = sp->size();
                if(i != 0){
                    plotString += ", ";
                }
                plotString += gpSetData(sp, style, i, xDataType, yDataType);
            } else {
                throw RuntimeException(
                        "The elements of a vector have to be vectors, tables,char scalar, short scalar, int scalar, long scalar, float scalar, double scalar");
            }
        }
        dataIndex = 0;
        blockIndex = 0;
        blockSize = size;
        if(size != 0)
            gpSetCommand(plotString);
    } else if (data->getForm() == DF_VECTOR || data->getForm() == DF_TABLE) {
        plotString = "plot " + gpSetData(data, style, 0, xDataType, yDataType);
        dataIndex = 0;
        blockIndex = 0;
        blockSize = 1;
        gpSetCommand(plotString);
    }
}

void onePlot() {
    lineColorVec.clear();
    lineWidthVec.clear();
    pointTypeVec.clear();
    pointSizeVec.clear();
    smoothVec.clear();
    titleVector.clear();
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
    gpSetCommand("set format x \"%f\"");
    gpSetCommand("set format y \"%f\"");
    gpSetCommand("unset xdata");
    gpSetCommand("unset ydata");
    resolution[0] = 0;
    resolution[1] = 0;
}

bool needGpInit = true;
Mutex mLock;

ConstantSP gpPlot(Heap *heap, vector<ConstantSP> &args) {
    LockGuard<Mutex> lock(&mLock);
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
    if (needGpInit) {
        gpInit(0, NULL);
        styleSetInit();
        needGpInit = false;
    }
    onePlot();
    if (args.size() > 3)
        gpSetProps(args[3]);
    try {
        gpSetPath(args[2]);
    }
    catch (char *e) {
        throw RuntimeException(e);
    }
    gpSetStyle(args[1]->getString());
    gpSetData(args[0], args[1]->getString());
    return new Bool(true);
}