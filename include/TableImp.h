#pragma once

#include "CoreConcept.h"

namespace ddb {

class SubTable;
class BasicTable;
typedef ObjectPtr<SubTable> SubTableSP;
typedef ObjectPtr<BasicTable> BasicTableSP;

class SWORDFISH_API SubTable: public Table {
public:
	SubTable(const TableSP& source, INDEX offset, INDEX length);
	SubTable(const TableSP& source, const ConstantSP& indices);

	const TableSP &getSource() const {
		return source_;
	}

	const ConstantSP &getIndices() const {
		return indices_;
	}

	void reset(INDEX offset, INDEX length);
	void reset(const ConstantSP& indices);

	~SubTable() override{}
	ConstantSP getColumn(const string& name) const override;
	ConstantSP getColumn(const string& qualifier, const string& name) const override;
	ConstantSP getColumn(INDEX index) const override;
	ConstantSP getColumn(const string& name, const ConstantSP& rowFilter) const override;
	ConstantSP getColumn(const string& qualifier, const string& name, const ConstantSP& rowFilter) const override;
	ConstantSP getColumn(INDEX index, const ConstantSP& rowFilter) const override;
	INDEX columns() const override {return source_->columns();}
	const string& getColumnName(int index) const override { return source_->getColumnName(index);}
	const string& getColumnQualifier(int index) const override { return source_->getColumnQualifier(index);}
	void setColumnName(int index, const string& name) override { throw RuntimeException("SubTable::setColumnName not supported.");}
	int getColumnIndex(const string& name) const override { return source_->getColumnIndex(name);}
	DATA_TYPE getColumnType(int index) const override { return source_->getColumnType(index);}
	int getColumnExtraParam(int index) const override { return source_->getColumnExtraParam(index);}
	bool contain(const string& name) const override { return source_->contain(name);}
	bool contain(const string& qualifier, const string& name) const override {return source_->contain(qualifier, name);}
	bool contain(const ColumnRef* col) const override { return source_->contain(col);}
	bool contain(const ColumnRefSP& col) const override { return source_->contain(col);}
	bool containAll(const vector<ColumnRefSP>& cols) const override { return source_->containAll(cols);}
	void setName(const string& name) override { name_ = name; unsetTableUsingInternalName();}
	const string& getName() const override { return name_;}
	ConstantSP get(INDEX index) const override;
	ConstantSP get(const ConstantSP& index) const override;
	ConstantSP getValue(INDEX capacity) const override;
	ConstantSP getValue() const override;
	ConstantSP getInstance(INDEX size) const override { return source_->getInstance(size);}
	ConstantSP getInstance() const override { return source_->getInstance(0);}
	INDEX size() const override {return size_;}
	bool sizeable() const override {return false;}
	string getString(INDEX index) const override;
	string getString() const override;
	ConstantSP getWindow(INDEX colStart, int colLength, INDEX rowStart, int rowLength) const override;
	ConstantSP getSlice(const ConstantSP& rowIndex, const ConstantSP& colIndex) const override;
	ConstantSP getMember(const ConstantSP& key) const override;
	ConstantSP values() const override;
	ConstantSP keys() const override { return source_->keys();}
	TABLE_TYPE getTableType() const override;
	bool append(vector<ConstantSP>& values, INDEX& insertedRows, string& errMsg) override { return false;}
	INDEX update(vector<ConstantSP>& values, const ConstantSP& indexSP, vector<string>& colNames, string& errMsg) override{ errMsg = "Updating a SubTable is not supported."; return 0;}
	INDEX remove(const ConstantSP& indexSP, string& errMsg) override{ errMsg = "Removing rows from a SubTable is not supported."; return 0;}
	bool isDistributedTable() const override {return source_->isDistributedTable();}
	bool isSegmentedTable() const override {return source_->isSegmentedTable();}
	bool isDimensionalTable() const override {return source_->isDimensionalTable();}
	bool isBasicTable() const override {return source_->isBasicTable();}
	bool isDFSTable() const override {return source_->isDFSTable();}
	DomainSP getGlobalDomain() const override {return source_->getGlobalDomain();}
	DomainSP getLocalDomain() const override {return source_->getLocalDomain();}
	int getGlobalPartitionColumnIndex() const override {return source_->getGlobalPartitionColumnIndex();}
	int getLocalPartitionColumnIndex(int dim) const override {return source_->getLocalPartitionColumnIndex(dim);}
	void setGlobalPartition(const DomainSP& domain, const string& partitionColumn) override{ source_->setGlobalPartition(domain, partitionColumn);}
	bool isLargeConstant() const override {return true;}
	long long getAllocatedMemory() const override {return source_->getAllocatedMemory();}
	void release() const override { source_->release();}
	void checkout() const override { source_->checkout();}
	TableSP getSegment(Heap* heap, const DomainPartitionSP& partition, PartitionGuard* guard = 0) override { return source_->getSegment(heap, partition, guard);}
	const TableSP& getEmptySegment() const override { return source_->getEmptySegment();}
	bool segmentExists(const DomainPartitionSP& partition) const override { return source_->segmentExists(partition);}
	bool snapshotIsolate() const override { return false;}
	bool drop(vector<int>& columns) override { return false;}
	bool join(vector<ConstantSP>& columns) override { return false;}
	bool clear() override { return false;}
	bool reorderColumns(const vector<int>& newOrders) override { return false;}
	bool replaceColumn(int index, const ConstantSP& col) override {return false;}
	void sortBy(Heap* heap, const ObjectSP& sortExpr, const ConstantSP& sortOrder) override {throw RuntimeException("SubTable::sortBy not supported.");}
	INDEX update(Heap* heap, const SQLContextSP& context, const ConstantSP& updateColNames, const ObjectSP& updateExpr, const ConstantSP& filterExprs) override {throw RuntimeException("SubTable::update not supported.");}
	INDEX remove(Heap* heap, const SQLContextSP& context, const ConstantSP& filterExprs) override {throw RuntimeException("SubTable::remove not supported.");}
	bool isEditable() const override {return false;}
	bool isSchemaEditable() const override {return false;}
	bool isAppendable() const override {return false;}
	bool isExpired() const override { return source_->isExpired();}
	int getKeyColumnCount() const override { return source_->getKeyColumnCount();}
	int getKeyColumnIndex(int index) const override { return source_->getKeyColumnIndex(index);}
	int getSortKeyCount() const override { return source_->getSortKeyCount();}
	int getSortKeyColumnIndex(int index) override{return source_->getSortKeyColumnIndex(index);}
	void share() override{}
	string getChunkPath() const override { return source_->getChunkPath();}\
    vector<FunctionDefSP> getPartitionFunction() const override { return source_->getPartitionFunction();}

private:
	TableSP source_;
	ConstantSP indices_;
	string name_;
	INDEX offset_;
	INDEX length_;
	INDEX size_;
};

class SWORDFISH_API AbstractTable : public Table {
public:
	AbstractTable(const SmartPointer<vector<string>>& colNames);
	AbstractTable(const SmartPointer<vector<string>>& colNames, SmartPointer<unordered_map<string,int>> colMap);
	AbstractTable(const SmartPointer<vector<string>>& colNames, SmartPointer<unordered_map<string,int>> colMap, const TableSP& emptyTbl);
	~AbstractTable() override;
	string getScript() const override;
	ConstantSP getColumn(const string& name) const override;
	ConstantSP getColumn(const string& qualifier, const string& name) const override;
	ConstantSP getColumn(const string& name, const ConstantSP& rowFilter) const override;
	ConstantSP getColumn(const string& qualifier, const string& name, const ConstantSP& rowFilter) const override;
	ConstantSP getColumn(INDEX index, const ConstantSP& rowFilter) const override;
	ConstantSP getColumn(INDEX index) const override = 0;
	ConstantSP get(INDEX col, INDEX row) const override = 0;
	INDEX columns() const override;
	const string& getColumnName(int index) const override;
	const string& getColumnQualifier(int index) const override {return name_;}
	void setColumnName(int index, const string& name) override;
	int getColumnIndex(const string& name) const override;
	bool contain(const string& name) const override;
	bool contain(const string& qualifier, const string& name) const override;
	bool contain(const ColumnRef* col) const override;
	bool contain(const ColumnRefSP& col) const override;
	bool containAll(const vector<ColumnRefSP>& cols) const override;
	ConstantSP getColumnLabel() const override;
	ConstantSP values() const override;
	ConstantSP keys() const override { return getColumnLabel();}
	void setName(const string& name) override{name_=name;unsetTableUsingInternalName();}
	const string& getName() const override { return name_;}
	virtual bool isTemporary() const {return false;}
	virtual void setTemporary(bool temp){}
	bool sizeable() const override {return false;}
	string getString(INDEX index) const override;
	string getString() const override;
	ConstantSP get(INDEX index) const override { return getInternal(index);}
	bool set(INDEX index, const ConstantSP& value) override;
	ConstantSP get(const ConstantSP& index) const override { return getInternal(index);}
	ConstantSP getWindow(int colStart, int colLength, int rowStart, int rowLength) const override {return getWindowInternal(colStart, colLength, rowStart, rowLength);}
	ConstantSP getSlice(const ConstantSP& rowIndex, const ConstantSP& colIndex) const override { return getSliceInternal(rowIndex, colIndex);}
	ConstantSP getMember(const ConstantSP& key) const override { return getMemberInternal(key);}
	ConstantSP getInstance() const override {return getInstance(0);}
	ConstantSP getInstance(int size) const override;
	ConstantSP getValue() const override;
	ConstantSP getValue(INDEX capacity) const override;
	ConstantSP getValue(Heap* pHeap) override {return getValue();}
	ConstantSP getReference(Heap* pHeap) override{return getValue();}
	bool append(vector<ConstantSP>& values, INDEX& insertedRows, string& errMsg) override;
	INDEX update(vector<ConstantSP>& values, const ConstantSP& indexSP, vector<string>& colNames, string& errMsg) override;
	INDEX remove(const ConstantSP& indexSP, string& errMsg) override;
	bool readPermitted(const AuthenticatedUserSP& user) const override;
	bool writePermitted(const AuthenticatedUserSP& user) const override;

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

class SWORDFISH_API BasicTable: public AbstractTable{
public:
	BasicTable(const vector<ConstantSP>& cols, const vector<string>& colNames, const vector<int>& keys, bool ordered = false, int timeColIdx = -1);
	BasicTable(const vector<ConstantSP>& cols, const vector<string>& colNames);
	BasicTable(const vector<VectorSP> &cols, const SmartPointer<vector<string>> &colNames,
				const SmartPointer<unordered_map<string, int>> &colMap);
	// FIXME: BasicTable is not a "moveable" object in fact.
	// You should not use the moved BasicTable because its data all moved.
	// It's not a legal state of a ddb::Table. (Contains at least on column)
	BasicTable(BasicTable &&) = default;
	~BasicTable() override = default;
	bool isBasicTable() const override {return true;}
	virtual bool isSpecialBasicTable() const {return false;}
	ConstantSP getColumn(INDEX index) const override;
    const ConstantSP& getColumnRef(INDEX index) override { return cols_[index]; }
    ConstantSP get(INDEX col, INDEX row) const override {return cols_[col]->get(row);}
	DATA_TYPE getColumnType(const int index) const override { return cols_[index]->getType();}
	int getColumnExtraParam(const int index) const override { return cols_[index]->getExtraParamForType(); }
	void setColumnName(int index, const string& name) override;
	INDEX size() const override {return size_;}
	bool sizeable() const override {return !readOnly_;}
	bool set(INDEX index, const ConstantSP& value) override;
	string getString(INDEX index) const override;
	string getString() const override;
	ConstantSP values() const override;
	ConstantSP get(INDEX index) const override;
	ConstantSP get(const ConstantSP& index) const override;
	ConstantSP getWindow(INDEX colStart, int colLength, INDEX rowStart, int rowLength) const override;
	const TableSP& getEmptySegment() const override;
	ConstantSP getSlice(const ConstantSP& rowIndex, const ConstantSP& colIndex) const override;
	ConstantSP getMember(const ConstantSP& key) const override;
	ConstantSP getInstance(int size) const override;
	ConstantSP getValue() const override;
	ConstantSP getValue(INDEX capacity) const override;
	bool upsert(vector<ConstantSP>& values, bool ignoreNull, INDEX& insertedRows, string& errMsg) override;
	bool upsert(vector<ConstantSP>& values, bool ignoreNull, INDEX& insertedRows, INDEX& updatedRows,
						string& errMsg) override;
	bool append(vector<ConstantSP>& values, INDEX& insertedRows, string& errMsg) override;
	INDEX update(vector<ConstantSP>& values, const ConstantSP& indexSP, vector<string>& colNames, string& errMsg) override;
	INDEX remove(const ConstantSP& indexSP, string& errMsg) override;
	long long getAllocatedMemory() const override;
	TABLE_TYPE getTableType() const override {return BASICTBL;}
	bool isDistributedTable() const override {return !domain_.isNull();}
	DomainSP getGlobalDomain() const override {return domain_;}
	int getGlobalPartitionColumnIndex() const override {return partitionColumnIndex_;}
	void setGlobalPartition(const DomainSP& domain, const string& partitionColumn) override;
	ConstantSP retrieveMessage(long long offset, int length, bool msgAsTable, const ObjectSP& filter, long long& messageId) override;
	INDEX getFilterColumnIndex() const override { return filterColumnIndex_; }
	bool snapshotIsolate() const override { return versionMutex_ != NULL;}
	void getSnapshot(TableSP& copy) const override;
	void sortBy(Heap* heap, const ObjectSP& sortExpr, const ConstantSP& sortOrder) override;
	INDEX update(Heap* heap, const SQLContextSP& context, const ConstantSP& updateColNames, const ObjectSP& updateExpr, const ConstantSP& filterExprs) override;
	INDEX remove(Heap* heap, const SQLContextSP& context, const ConstantSP& filterExprs) override;
	bool drop(vector<int>& columns) override;
	bool join(vector<ConstantSP>& columns) override;
	bool clear() override;
	bool reorderColumns(const vector<int>& newOrders) override;
	bool replaceColumn(int index, const ConstantSP& col) override;
	bool isEditable() const override;
	bool isSchemaEditable() const override;
	bool isAppendable() const override;
	void transferAsString(bool option) override;
	int getKeyColumnCount() const override;
	int getKeyColumnIndex(int index) const override;
	int getKeyTimeColumnIndex() const override;
	void share() override;
	string getChunkPath() const override { return chunkPath_;}
	bool segmentExists(const DomainPartitionSP &partition) const override { return false; }
	int getPartitionCount() const override { return 0; }

	void updateSize();
	void getKeyColumnNameAndType(vector<string>& keyNames, vector<pair<DATA_TYPE, DATA_CATEGORY>>& keyTypes, bool& ordered) const;
	ConstantSP getRowByKey(vector<ConstantSP>& keys, bool excludeNotExist, bool preserveOrder = false) const;
	void containKey(vector<ConstantSP>& keys, const ConstantSP& result) const;
	void setChunkPath(const string& chunkPath){ chunkPath_ = chunkPath;}
	ConstantSP toWideTable();
	void addColumn(const vector<string>& colNames, const vector<DATA_TYPE>& colTypes, const vector<int> &colExtras);
	void addColumn(const vector<string>& colNames, const vector<DATA_TYPE>& colTypes, const vector<ConstantSP>& vecs);
	void setVersion(const BasicTableSP& table) {
		curVersion_ = table->getValue();
	}
	void setTable(const BasicTableSP& table);
	void setSize(INDEX size) { size_ = size; }

    TableSP beginQueryTransaction(Heap* pHeap, const TableSP& originalTable) override;
    TableSP beginUpdateTransaction(Heap* pHeap, const TableSP& originalTable) override;
    TableSP beginDeleteTransaction(Heap* pHeap, const TableSP& originalTable) override;

    bool isKeyTable() const {
        return keyTable_ != nullptr;
    }
    bool isHashKeyTable() const {
    	return keyTable_ != nullptr && !keyTable_->ordered;
    }
    const DictionarySP& getKeyDictionary() const { return keyTable_->dict;}
    TableSP getKeyTableCopy(const vector<ConstantSP>& cols) const;
    inline const ConstantSP& getInternalColumn(int index) const { return cols_[index];}

    void resetKeyCols(const vector<int>& keys, bool ordered = false);

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
	INDEX internalUpdate(vector<ConstantSP>& values, const ConstantSP& indexSP, vector<string>& colNames, string& errMsg);
	INDEX internalUpdate(Heap* heap, const SQLContextSP& context, const ConstantSP& updateColNames, const ObjectSP& updateExpr, vector<ObjectSP>& filterExprs);
	INDEX internalRemove(Heap* heap, const SQLContextSP& context, vector<ObjectSP>& filterExprs);
	INDEX internalRemove(const ConstantSP& indexSP, string& errMsg);
	bool internalDrop(vector<int>& columns);
	ConstantSP prepareHashKey(vector<ConstantSP> &keys) const;
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
	friend class HaMvccTable;

private:
	class KeyTable {
	public:
		KeyTable(const vector<ddb::ConstantSP> &cols, vector<int> keys, bool ordered, int timeColIdx);
		KeyTable(const KeyTable &);

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

		ConstantSP prepareHashKey(const vector<ConstantSP> &cols, vector<ConstantSP> &keys);

	private:
		void initializeKeyDictionary(const vector<ConstantSP> &cols);

		static DictionarySP createKeyDictionary(const ddb::ConstantSP &keys, bool ordered);
	};

	vector<ConstantSP> cols_;
	std::unique_ptr<KeyTable> keyTable_;
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
	mutable TableSP emptyTbl_;
};

} // namespace ddb
