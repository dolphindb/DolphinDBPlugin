/*
 * Table.h
 *
 *  Created on: Nov 3, 2012
 *      Author: dzhou
 */

#ifndef TABLE_H_
#define TABLE_H_

#include <atomic>
#include "CoreConcept.h"

class BasicTable;

typedef SmartPointer<BasicTable> BasicTableSP;

class AbstractTable : public Table {
public:
	AbstractTable(const SmartPointer<vector<string>>& colNames);
	AbstractTable(const SmartPointer<vector<string>>& colNames, SmartPointer<unordered_map<string,int>> colMap);
	AbstractTable(const SmartPointer<vector<string>>& colNames, SmartPointer<unordered_map<string,int>> colMap, const TableSP& emptyTbl);
	virtual ~AbstractTable();
	virtual string getScript() const;
	virtual ConstantSP getColumn(const string& name) const;
	virtual ConstantSP getColumn(const string& qualifier, const string& name) const;
	virtual ConstantSP getColumn(const string& name, const ConstantSP& rowFilter) const;
	virtual ConstantSP getColumn(const string& qualifier, const string& name, const ConstantSP& rowFilter) const;
	virtual ConstantSP getColumn(INDEX index, const ConstantSP& rowFilter) const;
	virtual ConstantSP getColumn(INDEX index) const = 0;
	virtual ConstantSP get(INDEX col, INDEX row) const = 0;
	virtual INDEX columns() const;
	virtual const string& getColumnName(int index) const;
	virtual const string& getColumnQualifier(int index) const {return name_;}
	virtual void setColumnName(int index, const string& name);
	virtual int getColumnIndex(const string& name) const;
	virtual bool contain(const string& name) const;
	virtual bool contain(const string& qualifier, const string& name) const;
	virtual bool contain(const ColumnRef* col) const;
	virtual bool contain(const ColumnRefSP& col) const;
	virtual bool containAll(const vector<ColumnRefSP>& cols) const;
	virtual ConstantSP getColumnLabel() const;
	virtual ConstantSP values() const;
	virtual ConstantSP keys() const { return getColumnLabel();}
	virtual void setName(const string& name){name_=name;}
	virtual const string& getName() const { return name_;}
	virtual bool isTemporary() const {return false;}
	virtual void setTemporary(bool temp){}
	virtual bool sizeable() const {return false;}
	virtual string getString(INDEX index) const;
	virtual string getString() const;
	virtual ConstantSP get(INDEX index) const { return getInternal(index);}
	virtual bool set(INDEX index, const ConstantSP& value);
	virtual ConstantSP get(const ConstantSP& index) const { return getInternal(index);}
	virtual ConstantSP getWindow(int colStart, int colLength, int rowStart, int rowLength) const {return getWindowInternal(colStart, colLength, rowStart, rowLength);}
	virtual ConstantSP getSlice(const ConstantSP& rowIndex, const ConstantSP& colIndex) const { return getSliceInternal(rowIndex, colIndex);}
	virtual ConstantSP getMember(const ConstantSP& key) const { return getMemberInternal(key);}
	virtual ConstantSP getInstance() const {return getInstance(0);}
	virtual ConstantSP getInstance(int size) const;
	virtual ConstantSP getValue() const;
	virtual ConstantSP getValue(INDEX capacity) const;
	virtual ConstantSP getValue(Heap* pHeap) {return getValue();}
	virtual ConstantSP getReference(Heap* pHeap){return getValue();}
	virtual bool append(vector<ConstantSP>& values, INDEX& insertedRows, string& errMsg);
	virtual bool update(vector<ConstantSP>& values, const ConstantSP& indexSP, vector<string>& colNames, string& errMsg);
	virtual bool remove(const ConstantSP& indexSP, string& errMsg);
	virtual bool readPermitted(const AuthenticatedUserSP& user) const override;
	virtual bool writePermitted(const AuthenticatedUserSP& user) const override;

protected:
	ConstantSP getInternal(INDEX index) const;
	ConstantSP getInternal(const ConstantSP& index) const;
	ConstantSP getWindowInternal(int colStart, int colLength, int rowStart, int rowLength) const;
	ConstantSP getSliceInternal(const ConstantSP& rowIndex, const ConstantSP& colIndex) const;
	ConstantSP getMemberInternal(const ConstantSP& key) const;
	inline SmartPointer<vector<string>> getColNamesSnapshot() const {
		if(versionMutex_ == nullptr)
			return colNames_;
		else{
			LockGuard<Mutex> guard(versionMutex_);
			return colNames_;
		}
	}
	inline SmartPointer<unordered_map<string,int>> getColMapSnapshot() const {
		if(versionMutex_ == nullptr)
			return colMap_;
		else{
			LockGuard<Mutex> guard(versionMutex_);
			return colMap_;
		}
	}

private:
	string getTableClassName() const;
	string getTableTypeName() const;

protected:
	SmartPointer<vector<string>> colNames_;
	SmartPointer<unordered_map<string,int>> colMap_;
	string name_;
	const TableSP emptyTbl_;
	mutable Mutex* versionMutex_;
};

class BasicTable: public AbstractTable{
public:
	BasicTable(const vector<ConstantSP>& cols, const vector<string>& colNames, const vector<int>& keys, bool ordered = false, int timeColIdx = -1);
	BasicTable(const vector<ConstantSP>& cols, const vector<string>& colNames);
	virtual ~BasicTable();
	virtual bool isBasicTable() const {return true;}
	virtual ConstantSP getColumn(INDEX index) const;
	virtual ConstantSP get(INDEX col, INDEX row) const {return cols_[col]->get(row);}
	virtual DATA_TYPE getColumnType(const int index) const { return cols_[index]->getType();}
	virtual int getColumnExtraParam(const int index) const override { return cols_[index]->getExtraParamForType(); }
	virtual void setColumnName(int index, const string& name);
	virtual INDEX size() const {return size_;}
	virtual bool sizeable() const {return !readOnly_;}
	virtual bool set(INDEX index, const ConstantSP& value);
	virtual string getString(INDEX index) const;
	virtual string getString() const;
	virtual ConstantSP values() const;
	virtual ConstantSP get(INDEX index) const;
	virtual ConstantSP get(const ConstantSP& index) const;
	virtual ConstantSP getWindow(INDEX colStart, int colLength, INDEX rowStart, int rowLength) const;
	virtual ConstantSP getSlice(const ConstantSP& rowIndex, const ConstantSP& colIndex) const;
	virtual ConstantSP getMember(const ConstantSP& key) const;
	virtual ConstantSP getInstance(int size) const;
	virtual ConstantSP getValue() const;
	virtual ConstantSP getValue(INDEX capacity) const;
	virtual bool upsert(vector<ConstantSP>& values, bool ignoreNull, INDEX& insertedRows, string& errMsg);
	virtual bool append(vector<ConstantSP>& values, INDEX& insertedRows, string& errMsg);
	virtual bool update(vector<ConstantSP>& values, const ConstantSP& indexSP, vector<string>& colNames, string& errMsg);
	virtual bool remove(const ConstantSP& indexSP, string& errMsg);
	virtual long long getAllocatedMemory() const;
	virtual TABLE_TYPE getTableType() const {return BASICTBL;}
	virtual bool isDistributedTable() const {return !domain_.isNull();}
	virtual DomainSP getGlobalDomain() const {return domain_;}
	virtual int getGlobalPartitionColumnIndex() const {return partitionColumnIndex_;}
	virtual void setGlobalPartition(const DomainSP& domain, const string& partitionColumn);
	virtual ConstantSP retrieveMessage(long long offset, int length, bool msgAsTable, const ObjectSP& filter, long long& messageId);
	virtual INDEX getFilterColumnIndex() const override { return filterColumnIndex_; }
	virtual bool snapshotIsolate() const { return versionMutex_ != NULL;}
	virtual void getSnapshot(TableSP& copy) const;
	virtual void sortBy(Heap* heap, const ObjectSP& sortExpr, const ConstantSP& sortOrder);
	virtual void update(Heap* heap, const SQLContextSP& context, const ConstantSP& updateColNames, const ObjectSP& updateExpr, const ConstantSP& filterExprs);
	virtual void remove(Heap* heap, const SQLContextSP& context, const ConstantSP& filterExprs);
	virtual bool drop(vector<int>& columns);
	virtual bool join(vector<ConstantSP>& columns);
	virtual bool clear();
	virtual bool reorderColumns(const vector<int>& newOrders);
	virtual bool replaceColumn(int index, const ConstantSP& col);
	virtual bool isEditable() const;
	virtual bool isSchemaEditable() const;
	virtual bool isAppendable() const;
	virtual void transferAsString(bool option);
	virtual int getKeyColumnCount() const;
	virtual int getKeyColumnIndex(int index) const;
	virtual int getKeyTimeColumnIndex() const;
	virtual void share();
	virtual string getChunkPath() const { return chunkPath_;}
	virtual bool segmentExists(const DomainPartitionSP &partition) const override { return false; }
	virtual int getPartitionCount() const override { return 0; }

	void updateSize();
	void getKeyColumnNameAndType(vector<string>& keyNames, vector<pair<DATA_TYPE, DATA_CATEGORY>>& keyTypes, bool& ordered) const;
	ConstantSP getRowByKey(vector<ConstantSP>& keys, bool excludeNotExist) const;
	void containKey(vector<ConstantSP>& keys, const ConstantSP& result) const;
	void setChunkPath(const string& chunkPath){ chunkPath_ = chunkPath;}
	ConstantSP toWideTable();
	void addColumn(const vector<string>& colNames, const vector<DATA_TYPE>& colTypes);
	void setVersion(const BasicTableSP& table) {
		curVersion_ = table->getValue();
	}
	void setTable(const BasicTableSP& table) {
		vector<ConstantSP> cols(table->columns());
		for (int i = 0; i < table->columns(); ++i) {
			cols[i] = table->getColumn(i)->getValue();
		}
		std::swap(cols_, cols);
	}
	void setSize(INDEX size) { size_ = size; }
	

private:
	BasicTable(const vector<ConstantSP>& cols, const SmartPointer<vector<string>>& colNames, const SmartPointer<unordered_map<string, int>>& colMap,
			const string& tableName, const DomainSP& domain, int partitionColumnIndex, long long offset);
	inline void updateSize(INDEX size) { size_ = size;}
	bool increaseCapacity(long long newCapacity, string& errMsg);
	bool prepareNewVersion(long long newSize, string& errMsg);
	void initData(const vector<ConstantSP>& cols, const vector<string>& colNames);
	bool internalUpsert(vector<ConstantSP>& values, INDEX& insertedRows, string& errMsg);
	bool internalUpsertNonNull(vector<ConstantSP>& values, INDEX& insertedRows, string& errMsg);
	bool internalAppend(vector<ConstantSP>& values, INDEX& insertedRows, string& errMsg);
	void internalSortBy(Heap* heap, const ObjectSP& sortExpr, const ConstantSP& sortOrder);
	bool internalUpdate(vector<ConstantSP>& values, const ConstantSP& indexSP, vector<string>& colNames, string& errMsg);
	void internalUpdate(Heap* heap, const SQLContextSP& context, const ConstantSP& updateColNames, const ObjectSP& updateExpr, vector<ObjectSP>& filterExprs);
	void internalRemove(Heap* heap, const SQLContextSP& context, vector<ObjectSP>& filterExprs);
	bool internalRemove(const ConstantSP& indexSP, string& errMsg);
	bool internalDrop(vector<int>& columns);
	ConstantSP prepareHashKey(vector<ConstantSP>& cols) const;
	ConstantSP checkKeyDuplicate(ConstantSP& key, const ConstantSP& timeCol = nullptr);

	inline BasicTableSP getSnapshot() const{
		versionMutex_->lock();
		BasicTableSP copy(curVersion_);
		versionMutex_->unlock();
		return copy;
	}

	friend class RealtimeTable;
	friend class MvccTable;
	friend class SegmentedTable;
	friend class DimensionalTable;

private:
	struct KeyTable {
		bool ordered;
		vector<int> keys;
		int timeColIdx;
		vector<bool> isKeyCol;
		DictionarySP dict;
		DictionarySP dictCheck;
		VectorSP keyVec;
		ConstantSP keyScalar;
		ConstantSP oldRowIndices;
		ConstantSP oldIndices;
		ConstantSP newRowIndices;
	};
	vector<ConstantSP> cols_;
	KeyTable* keyTable_;
	bool readOnly_;
	INDEX size_;
	long long offset_;
	DomainSP domain_;
	int partitionColumnIndex_;
	int filterColumnIndex_;
	INDEX capacity_;
	string chunkPath_;
	mutable BasicTableSP curVersion_;
};

#endif /* TABLE_H_ */
