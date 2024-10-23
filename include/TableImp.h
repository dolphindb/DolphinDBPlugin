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
#include "ConstantMarshal.h"

class SubTable;
class BasicTable;
typedef SmartPointer<SubTable> SubTableSP;
typedef SmartPointer<BasicTable> BasicTableSP;

class SubTable: public Table {
public:
	SubTable(const TableSP& source, INDEX offset, INDEX length);
	SubTable(const TableSP& source, const ConstantSP& indices);
	TableSP getSource() const { return source_;}
	void reset(INDEX offset, INDEX length);
	void reset(const ConstantSP& indices);
	virtual ~SubTable(){}
	virtual ConstantSP getColumn(const string& name) const;
	virtual ConstantSP getColumn(const string& qualifier, const string& name) const;
	virtual ConstantSP getColumn(INDEX index) const;
	virtual ConstantSP getColumn(const string& name, const ConstantSP& rowFilter) const;
	virtual ConstantSP getColumn(const string& qualifier, const string& name, const ConstantSP& rowFilter) const;
	virtual ConstantSP getColumn(INDEX index, const ConstantSP& rowFilter) const;
	virtual INDEX columns() const {return source_->columns();}
	virtual const string& getColumnName(int index) const { return source_->getColumnName(index);}
	virtual const string& getColumnQualifier(int index) const { return source_->getColumnQualifier(index);}
	virtual void setColumnName(int index, const string& name) { throw RuntimeException("SubTable::setColumnName not supported.");}
	virtual int getColumnIndex(const string& name) const { return source_->getColumnIndex(name);}
	virtual DATA_TYPE getColumnType(int index) const { return source_->getColumnType(index);}
	virtual bool contain(const string& name) const { return source_->contain(name);}
	virtual bool contain(const string& qualifier, const string& name) const {return source_->contain(qualifier, name);}
	virtual bool contain(const ColumnRef* col) const { return source_->contain(col);}
	virtual bool contain(const ColumnRefSP& col) const { return source_->contain(col);}
	virtual bool containAll(const vector<ColumnRefSP>& cols) const { return source_->containAll(cols);}
	virtual void setName(const string& name) { name_ = name;}
	virtual const string& getName() const { return name_;}
	virtual ConstantSP get(INDEX index) const;
	virtual ConstantSP get(const ConstantSP& index) const;
	virtual ConstantSP getValue(INDEX capacity) const;
	virtual ConstantSP getValue() const;
	virtual ConstantSP getInstance(INDEX size) const { return source_->getInstance(size);}
	virtual ConstantSP getInstance() const { return source_->getInstance(0);}
	virtual INDEX size() const {return size_;}
	virtual bool sizeable() const {return false;}
	virtual string getString(INDEX index) const;
	virtual string getString() const;
	virtual ConstantSP getWindow(INDEX colStart, int colLength, INDEX rowStart, int rowLength) const;
	virtual ConstantSP getSlice(const ConstantSP& rowIndex, const ConstantSP& colIndex) const;
	virtual ConstantSP getMember(const ConstantSP& key) const;
	virtual ConstantSP values() const;
	virtual ConstantSP keys() const { return source_->keys();}
	virtual TABLE_TYPE getTableType() const;
	virtual bool append(vector<ConstantSP>& values, INDEX& insertedRows, string& errMsg) { return false;}
	virtual bool update(vector<ConstantSP>& values, const ConstantSP& indexSP, vector<string>& colNames, string& errMsg){return false;}
	virtual bool remove(const ConstantSP& indexSP, string& errMsg){return false;}
	virtual bool isDistributedTable() const {return source_->isDistributedTable();}
	virtual bool isSegmentedTable() const {return source_->isSegmentedTable();}
	virtual bool isDimensionalTable() const {return source_->isDimensionalTable();}
	virtual bool isBasicTable() const {return source_->isBasicTable();}
	virtual bool isDFSTable() const {return source_->isDFSTable();}
	virtual DomainSP getGlobalDomain() const {return source_->getGlobalDomain();}
	virtual DomainSP getLocalDomain() const {return source_->getLocalDomain();}
	virtual int getGlobalPartitionColumnIndex() const {return source_->getGlobalPartitionColumnIndex();}
	virtual int getLocalPartitionColumnIndex(int dim) const {return source_->getLocalPartitionColumnIndex(dim);}
	virtual void setGlobalPartition(const DomainSP& domain, const string& partitionColumn){ source_->setGlobalPartition(domain, partitionColumn);}
	virtual bool isLargeConstant() const {return true;}
	virtual long long getAllocatedMemory() const {return source_->getAllocatedMemory();}
	virtual void release() const { source_->release();}
	virtual void checkout() const { source_->checkout();}
	virtual TableSP getSegment(Heap* heap, const DomainPartitionSP& partition, PartitionGuard* guard = 0) { return source_->getSegment(heap, partition, guard);}
	virtual const TableSP& getEmptySegment() const { return source_->getEmptySegment();}
	virtual bool segmentExists(const DomainPartitionSP& partition) const { return source_->segmentExists(partition);}
	virtual bool snapshotIsolate() const { return false;}
	virtual bool drop(vector<int>& columns) { return false;}
	virtual bool join(vector<ConstantSP>& columns) { return false;}
	virtual	bool clear() { return false;}
	virtual bool reorderColumns(const vector<int>& newOrders) { return false;}
	virtual bool replaceColumn(int index, const ConstantSP& col) {return false;}
	virtual void sortBy(Heap* heap, const ObjectSP& sortExpr, const ConstantSP& sortOrder) {throw RuntimeException("SubTable::sortBy not supported.");}
	virtual void update(Heap* heap, const SQLContextSP& context, const ConstantSP& updateColNames, const ObjectSP& updateExpr, const ConstantSP& filterExprs) {throw RuntimeException("SubTable::update not supported.");}
	virtual void remove(Heap* heap, const SQLContextSP& context, const ConstantSP& filterExprs) {throw RuntimeException("SubTable::remove not supported.");}
	virtual bool isEditable() const {return false;}
	virtual bool isSchemaEditable() const {return false;}
	virtual bool isAppendable() const {return false;}
	virtual bool isExpired() const { return source_->isExpired();}
	virtual int getKeyColumnCount() const { return source_->getKeyColumnCount();}
	virtual int getKeyColumnIndex(int index) const { return source_->getKeyColumnIndex(index);}
	virtual int getSortKeyCount() const { return source_->getSortKeyCount();}
	virtual int getSortKeyColumnIndex(int index){return source_->getSortKeyColumnIndex(index);}
	virtual void share(){}
	virtual string getChunkPath() const { return source_->getChunkPath();}

private:
	TableSP source_;
	ConstantSP indices_;
	string name_;
	INDEX offset_;
	INDEX length_;
	INDEX size_;
};


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
	virtual bool isSpecialBasicTable() const {return false;}
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
	virtual bool upsert(vector<ConstantSP>& values, bool ignoreNull, INDEX& insertedRows, INDEX& updatedRows,
						string& errMsg) override;
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
	void addColumn(const vector<string>& colNames, const vector<DATA_TYPE>& colTypes, const vector<int> &colExtras);
	void setVersion(const BasicTableSP& table) {
		curVersion_ = table->getValue();
	}
	void setTable(const BasicTableSP& table);
	void setSize(INDEX size) { size_ = size; }

protected:
	const vector<ConstantSP>& getCols() const { return cols_; }

private:
	BasicTable(const vector<ConstantSP>& cols, const SmartPointer<vector<string>>& colNames, const SmartPointer<unordered_map<string, int>>& colMap,
			const string& tableName, const DomainSP& domain, int partitionColumnIndex, long long offset);
	inline void updateSize(INDEX size) { size_ = size;}
	bool increaseCapacity(long long newCapacity, string& errMsg);
	bool prepareNewVersion(long long newSize, string& errMsg);
	void initData(const vector<ConstantSP>& cols, const vector<string>& colNames);
	bool internalUpsert(vector<ConstantSP>& values, INDEX& insertedRows, string& errMsg);
	bool internalUpsertNonNull(vector<ConstantSP>& values, INDEX& insertedRows, string& errMsg);
	bool internalUpsert(vector<ConstantSP>& values, INDEX& insertedRows, INDEX& updatedRows, string& errMsg);
	/**
	 * @param needUpdatedRows If true, will update `updatedRows`.
	 */
	bool internalUpsertNonNull(vector<ConstantSP>& values, INDEX& insertedRows, INDEX& updatedRows, string& errMsg,
								bool needUpdatedRows);
	bool internalAppend(vector<ConstantSP>& values, INDEX& insertedRows, string& errMsg);
	void internalSortBy(Heap* heap, const ObjectSP& sortExpr, const ConstantSP& sortOrder);
	bool internalUpdate(vector<ConstantSP>& values, const ConstantSP& indexSP, vector<string>& colNames, string& errMsg);
	void internalUpdate(Heap* heap, const SQLContextSP& context, const ConstantSP& updateColNames, const ObjectSP& updateExpr, vector<ObjectSP>& filterExprs);
	void internalRemove(Heap* heap, const SQLContextSP& context, vector<ObjectSP>& filterExprs);
	bool internalRemove(const ConstantSP& indexSP, string& errMsg);
	bool internalDrop(vector<int>& columns);
	ConstantSP prepareHashKey(vector<ConstantSP>& cols) const;
	ConstantSP checkKeyDuplicate(ConstantSP& key, const ConstantSP& timeCol = nullptr);
	void setColumnarTuple(Vector* tuple);

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
	friend class PartitionedPersistentTable;

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
	int rowUnitLength_;
	mutable BasicTableSP curVersion_;
};

#endif /* TABLE_H_ */
