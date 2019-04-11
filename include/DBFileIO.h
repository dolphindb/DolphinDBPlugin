/*
 * DBFileIO.h
 *
 *  Created on: Dec 4, 2012
 *  Author: dzhou
 *
 */

#ifndef DBFILEIO_H_
#define DBFILEIO_H_

#include <string>
#include <istream>
#include <stdio.h>

#include "SysIOEx.h"

class SystemHandle;

class TablePersistenceMeta;
class Command;
class CmdNewFileOrDir;
class CmdReplaceFile;
class CmdUpdateHeaderAndAppend;
class CmdAppendFile;
class CmdUpdateFileHeader;
class CmdInMemoryChange;
class IoTransaction;


typedef SmartPointer<TablePersistenceMeta> TablePersistenceMetaSP;
typedef SmartPointer<Command> CommandSP;
typedef SmartPointer<CmdAppendFile> CmdAppendFileSP;
typedef SmartPointer<CmdUpdateFileHeader> CmdUpdateFileHeaderSP;
typedef SmartPointer<CmdNewFileOrDir> CmdNewFileOrDirSP;
typedef SmartPointer<CmdUpdateHeaderAndAppend> CmdUpdateHeaderAndAppendSP;
typedef SmartPointer<CmdReplaceFile> CmdReplaceFileSP;
typedef SmartPointer<CmdInMemoryChange> CmdInMemoryChangeSP;
typedef SmartPointer<IoTransaction> IoTransactionSP;

struct TablePersistenceIndex {
	TablePersistenceIndex(long long rowOffset, int fileIndex, long long fileOffset);

	long long rowOffset_;
	long long fileOffset_;
	int fileIndex_;
};

struct TablePersistenceMeta {
	~TablePersistenceMeta();
	TablePersistenceIndex asof(int startBucket, long long offset) const;
	bool write(const ConstantSP& obj, string& errMsg);

	bool asyns_;
	char compress_;
	int tableIndex_;
	int logFileIndex_;
	int lastCheckPoint_;
	int sizeInFile_;
	long long size_;
	string tableName_;
	string tableDir_;
	DataOutputStreamSP logStream_;
	DataOutputStreamSP indexStream_;
	DecoderSP coder_;
	vector<TablePersistenceIndex> indices_;

private:
	mutable Mutex mutex_;
};

/**
 *  Column File Header Format
 *  0~3 	Integer Data type
 *  4~7 	Integer Symbol base ID
 *  8~8 	Boolean Fast mode
 *  9~12	Integer Capacity
 *  13~16	Integer Number of elements
 *  17~18   Short   Unit Length
 *  19~19	Byte	Character code (always 0)
 */
struct ColumnHeader{
	ColumnHeader(const char* header);
	ColumnHeader();
	static int getHeaderSize(){ return 20;}
	void serialize(ByteArrayCodeBuffer& buf);
	void deserialize(const char* header);
	inline bool isLittleEndian() const { return flag & 1;}
	inline bool containChecksum() const {return flag & 2;}
	inline void setLittleEndian(bool val){ if(val) flag |= 1; else flag &= ~1;}
	inline void setChecksumOption(bool val){ if(val) flag |= 2; else flag &= ~2;}

	char version;
	char flag; //bit0: littleEndian bit1: containChecksum
	char charCode;
	char compression;
	char dataType;
	char unitLength;
	short reserved;
	int extra;
	int count;
	int checksum;
};

class DBFileIO {
public:
	static bool saveBasicTable(Session* session, const string& directory, Table* table, const string& tableName, IoTransaction* tran,  bool append = false, int compressionMode = 0, bool saveSymbolBase = true);
	static bool saveBasicTable(Session* session, SystemHandle* db, Table* table, const string& tableName, IoTransaction* tran, bool append = false, int compressionMode = 0, bool saveSymbolBase = true);
	static bool saveBasicTable(Session* session, const string& directory, const string& tableDir, Table* table, const string& tableName, const vector<ColumnDesc>& cols, SymbolBaseSP& symbase, IoTransaction* tran, bool chunkMode, bool append, int compressionMode, bool saveSymbolBase);
	static bool saveBasicTable(const string& directory, const string& tableName, Table* table, const SymbolBaseSP& symBase, IoTransaction* tran, int compressionMode);
	static bool saveBasicTable(Session* session, const string& tableDir, INDEX existingTblSize, Table* table, const vector<ColumnDesc>& cols, const SymbolBaseSP& symBase, IoTransaction* tran, int compressionMode, bool saveSymbolBase);
	static bool savePartitionedTable(Session* session, const DomainSP& domain, TableSP table, const string& tableName, IoTransaction* tran, int compressionMode = 0, bool saveSymbolBase = true );
	static bool saveDualPartitionedTable(Session* session, SystemHandle* db, const DomainSP& secDomain, TableSP table, const string& tableName,
			const string& partitionColName, vector<string>& secPartitionColNames, IoTransaction* tran, int compressionMode = 0);
	static Table* loadTable(Session* session, const string& directory, const string& tableName, const SymbolBaseManagerSP& symbaseManager, const DomainSP& domain, const ConstantSP& partitions, TABLE_TYPE tableType, bool memoryMode);
	static Table* loadTable(Session* session, SystemHandle* db, const string& tableName, const ConstantSP& partitions, TABLE_TYPE tableType, bool memoryMode);
	static void removeTable(SystemHandle* db, const string& tableName);
	static SystemHandle* openDatabase(const string& directory, const DomainSP& localDomain);
	static bool saveDatabase(SystemHandle* db);
	static bool removeDatabase(const string& dbDir);

	static ColumnHeader loadColumnHeader(const string& colFile);
	static VectorSP loadColumn(const string& colFile, int devId, const SymbolBaseManagerSP& symbaseManager);
	static VectorSP loadColumn(const string& colFile, int devId, const SymbolBaseSP& symbase);
	static VectorSP loadColumn(const string& colFile, int devId, const SymbolBaseSP& symbase, int rows, long long& postFileOffset, bool& isLittleEndian, char& compressType);
	static long long loadColumn(const string& colFile, long long fileOffset, bool isLittleEndian, char compressType, int devId, const SymbolBaseSP& symbase, INDEX rows, const VectorSP& col);
	static Vector* loadTextVector(bool includeHeader, DATA_TYPE type, const string& path);
	static bool saveColumn(const VectorSP& col, const string& colFile, int devId, INDEX existingTableSize, bool chunkNode, bool append, int compressionMode, IoTransaction* tran = NULL);
	static bool saveTableHeader(const string& owner, const vector<ColumnDesc>& cols, vector<int>& partitionColumnIndices, long long rows, const string& tableFile, IoTransaction* tran);
	static bool loadTableHeader(const DataInputStreamSP& in, string& owner, vector<ColumnDesc>& cols, vector<int>& partitionColumnIndices);
	static void removeBasicTable(const string& directory, const string& tableName);
	static TableSP createEmptyTableFromSchema(const TableSP& schema);

	static void checkTypeCompatibility(Table* table, vector<string>& partitionColumns, vector<ColumnDesc>& cols, vector<int>& partitionColumnIndices);
	static bool checkTypeCompatibility(DATA_TYPE type1, DATA_TYPE type2);
	static bool checkPartitionColumnCompatibility(DATA_TYPE partitionSchemeType, DATA_TYPE partitionDataType);
	static void saveSymbolBases(const SymbolBaseSP& symbase, IoTransaction* tran);
	static void collectColumnDesc(Table* table, vector<ColumnDesc>& cols);
	static ConstantSP convertColumn(const ConstantSP& col, const ColumnDesc& desiredType, SymbolBaseSP& symbase);
	static ConstantSP convertColumn(const ConstantSP& col, const ColumnDesc& desiredType, const SymbolBaseSP& symbase);
	static VectorSP decompress(const VectorSP& col);
	static VectorSP decompress(const VectorSP& col, const DecoderSP decoder);
	static VectorSP compress(const VectorSP& col);
	static TableSP compressTable(const TableSP& table);
	static ConstantSP appendDataToFile(Heap* heap, vector<ConstantSP>& arguments);
	static int getMappedDeviceId(const string& path);
	static void setVolumeMapper(const VolumeMapperSP& volumeMapper) { volumeMapper_ = volumeMapper;}

private:
	static VectorSP loadColumn(const string& colFile, int devId, const SymbolBaseManagerSP& symbaseManager,	const SymbolBaseSP& symbase,
			int rows, long long& postFileOffset, bool& isLittleEndian, char& compressType);
	static inline DATA_TYPE getCompressedDataType(const VectorSP& vec){return (DATA_TYPE)vec->getChar(4);}
	static VolumeMapperSP volumeMapper_;
};

class Command{
public:
	Command(const string& fileName, int priority) : fileName_(fileName), priority_(priority){}
	virtual ~Command(){}
	virtual void execute() = 0;
    virtual void undo() = 0;
    virtual void print() const = 0;
    virtual TranCmdType getType() const = 0;
    virtual IO_ERR serialize(const DataStreamSP& out) const= 0;
    virtual void commit(){};
    virtual void close(){};
    const string& getFileName() const {return fileName_;}
    int getPriority() const {return priority_;}

protected:
    string fileName_;
    int priority_;
};

class CmdAppendFile : public Command{
public:
	CmdAppendFile(const string& fileName, long long oldLength) : Command(fileName, 3), oldLength_(oldLength){}
	CmdAppendFile(const DataInputStreamSP& in);
	virtual ~CmdAppendFile(){};
	virtual void execute(){}
	virtual void undo();
    virtual void print() const;
    virtual TranCmdType getType() const {return APPEND_FILE;}
    virtual IO_ERR serialize(const DataStreamSP& out) const;

private:
    long long oldLength_;
};

class CmdUpdateFileHeader : public Command{
public:
	CmdUpdateFileHeader(const string& fileName, const char* oldValue, int length);
	CmdUpdateFileHeader(const DataInputStreamSP& in);
	virtual ~CmdUpdateFileHeader();
	virtual void execute(){}
	virtual void undo();
    virtual void print() const;
    virtual TranCmdType getType() const {return UPDATE_FILE_HEADER;}
    virtual IO_ERR serialize(const DataStreamSP& out) const;

private:
    char* oldValue_;
    int length_;
};

class CmdNewFileOrDir : public Command{
public:
	CmdNewFileOrDir(const string& name): Command(name, 0){}
	CmdNewFileOrDir(const DataInputStreamSP& in);
	virtual ~CmdNewFileOrDir(){}
	virtual void execute(){}
	virtual void undo();
	virtual void print() const;
	virtual TranCmdType getType() const {return NEW_FILE;}
	virtual IO_ERR serialize(const DataStreamSP& out) const;
};

class CmdRenameFileOrDir : public Command{
public:
	CmdRenameFileOrDir(const string& oldName,const string& newName);
	CmdRenameFileOrDir(const DataInputStreamSP& in);
	virtual ~CmdRenameFileOrDir(){}
	virtual void execute();
	virtual void undo();
	virtual void print() const;
	virtual TranCmdType getType() const {return RENAME_FILEDIR;}
	virtual IO_ERR serialize(const DataStreamSP& out) const;

private:
    string oldName_;
    string newName_;
};

class CmdReplaceFile : public Command{
public:
	CmdReplaceFile(const string& name,const string& oldFilePath);
	CmdReplaceFile(const DataInputStreamSP& in);
	virtual ~CmdReplaceFile(){}
	virtual void execute();
	virtual void undo();
	virtual void print() const;
	virtual TranCmdType getType() const {return REPLACE_FILE;}
	virtual IO_ERR serialize(const DataStreamSP& out) const;

private:
    string oldFilePath_;
    string uniqueName_;
};

class CmdUpdateHeaderAndAppend : public Command{
public:
	CmdUpdateHeaderAndAppend(const string& fileName, int headerSize);
	CmdUpdateHeaderAndAppend(const string& fileName, long long oldLength, const char* oldValue, int valueSize );
	CmdUpdateHeaderAndAppend(const DataInputStreamSP& in);
	virtual ~CmdUpdateHeaderAndAppend();
	virtual void execute(){};
	virtual void undo();
	virtual void print() const;
	virtual TranCmdType getType() const {return UPDATE_HEADER_APPEND;}
	virtual IO_ERR serialize(const DataStreamSP& out) const;

private:
	long long oldLength_;
	char* oldValue_;
	int valueSize_;
};

class CmdInMemoryChange : public Command{
public:
	CmdInMemoryChange(const string& changeDesc, const FunctionDefSP& undoFunc) : Command(changeDesc, 4), undoFunc_(undoFunc){}
	CmdInMemoryChange(const string& changeDesc, const FunctionDefSP& commitFunc, const FunctionDefSP& undoFunc) : Command(changeDesc, 4), commitFunc_(commitFunc), undoFunc_(undoFunc){}
	CmdInMemoryChange(const string& changeDesc, const FunctionDefSP& commitFunc, const FunctionDefSP& undoFunc, const FunctionDefSP& closeFunc) : Command(changeDesc, 4), commitFunc_(commitFunc), undoFunc_(undoFunc), closeFunc_(closeFunc){}
	virtual ~CmdInMemoryChange(){}
	virtual void execute(){}
	virtual void undo();
	virtual void commit();
	virtual void close();
	virtual void print() const;
	virtual TranCmdType getType() const {return INMEMORY_CHANGE;}
	virtual IO_ERR serialize(const DataStreamSP& out) const { return OK;}

private:
	FunctionDefSP commitFunc_;
	FunctionDefSP undoFunc_;
	FunctionDefSP closeFunc_;
};

class IoTransaction{
public:
	/**
	 * The IoTransaction constructor for local transaction mode
	 */
	IoTransaction(const string& dataBaseDir,bool restore = false);
	/**
	 * The IoTransaction constructor for distributed transaction mode
	 */
	IoTransaction(const string& dataBaseDir, long long id, int stage = 0,  bool restore = false);
	~IoTransaction();
	void record(const CommandSP& cmd);
	void recordNewDirectory(const string& dir);
	void commit();
	void rollback();
	void close();
	void print();
	long long getId(){return id_;}
	const string& getRollbackDir() const {return rollbackDir_;}
	const string& getRollbackLog() const {return rollbackLog_;}
	int rollbackFromFile();
	static string getRollbackDir(const string& databaseDir, long long id);
	static bool checkIoTransactionLog(const string& databaseDir, long long id);
	string getRollbackDir(const string& dir);
	inline int getStage(){return stage_;}

	/**
	 * This method is only valid for distributed transaction mode. For local transaction mode, the stage is automatically set when
	 * the method record(), rollback(), commit(), close() is called.
	 */
	void setStage(int stage);
	double getElapsedTimeFromLastOperation() const;
	inline bool isClosed() const {return stage_ == 4;}
	inline bool isOpen() const {return stage_ < 2;}
	bool addChunk(const DFSChunkMetaSP& chunk);
	void getAllChunk(vector<Guid>& chunks);
	void rebuildFromLog (const vector<Guid>& chunks);
	Mutex* getLock() { return &mutex_;}

private:
	bool restore_;
	long long id_;
	/*	transaction stage
	 * 	0 : just creating and before prepare
	 * 	1 : after self prepare finish and before commit
	 * 	2 : after self commit , but others are unknown
	 * 	3 : after self roll back
	 * 	4 : after closed
	 */
	int stage_;
	bool localMode_;
    string rollbackDir_;
    string rollbackLog_;
    DataStreamSP rollbackLogStream_;
    FILE* fp_;
    vector<CommandSP> cmdStack_;
    vector<Guid> chunks_;
    time_t lastOperateTime_;
    Mutex mutex_;
};

#endif /* DBFILEIO_H_ */
