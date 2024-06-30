/* Copyright 2004-2007 The MathWorks, Inc.
 *
 * ATTENTION! ATTENTION! ATTENTION! 
 *
 *   1. mwArray MAY NOT directly call any method (including public methods)
 *      of the array_ref class, despite the C++ compiler's willingness to
 *      allow this. Generally speaking, this is only a problem when a
 *      Compiler-generated application is compiled into a binary by a C++
 *      compiler different than the one used to build the MCR (i.e., when
 *      mbuild -setup has been used to choose a compiler other than 
 *      MSVC on the PC).
 *
 *   2. If you add a method to the array_ref interface, make sure that you
 *      provide an extern "C" version of it, and that mwArray calls the
 *      extern "C" version rather than the method itself.
 *
 *   3. Before making any changes here, please read (and understand!) the
 *      "MathWorks Deployment Team: Products and Processes" document, esp.
 *      the mclmcrrt section.
 *
 */

#ifndef _MCLCPPCLASS_H_
#define _MCLCPPCLASS_H_

#ifdef __cplusplus

#include <string>
#include <exception>
#include <iostream>

#include "mclmcr.h"

class mwArray;

template<class T>
class mwArraySharedCopy : public T
{
private:
    mwArraySharedCopy(array_ref* p) : T(p) {
    }

    friend class mwArray;
};

class mwArray
{
    friend void mclcppMlfFeval(HMCRINSTANCE inst, const char* name, int nargout, int fnout, int fnin, ...);
public:
    mwArray() : m_pa(0)
    {
        if (mclGetEmptyArray((void**)&m_pa, mxDOUBLE_CLASS) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }

    mwArray(mxClassID mxID) : m_pa(0)
    {
        if (mclGetEmptyArray((void**)&m_pa, mxID) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    mwArray(mwSize num_rows, mwSize num_cols, mxClassID mxID, mxComplexity cmplx = mxREAL) : m_pa(0)
    {
        if (mclGetMatrix((void**)&m_pa, num_rows, num_cols, mxID, cmplx) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    mwArray(mwSize num_dims, const mwSize* dims, mxClassID mxID, mxComplexity cmplx = mxREAL) : m_pa(0)
    {
        if (mclGetArray((void**)&m_pa, num_dims, dims, mxID, cmplx) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    mwArray(const char* str) : m_pa(0)
    {
        if (mclGetString((void**)&m_pa, str) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    mwArray(mwSize num_strings, const char** str) : m_pa(0)
    {
        if (mclGetCharMatrixFromStrings((void**)&m_pa, num_strings, str) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    mwArray(mwSize num_rows, mwSize num_cols, int num_fields, const char** fieldnames) : m_pa(0)
    {
        if (mclGetStructMatrix((void**)&m_pa, num_rows, num_cols, num_fields, fieldnames) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    mwArray(mwSize num_dims, const mwSize* dims, int num_fields, const char** fieldnames) : m_pa(0)
    {
        if (mclGetStructArray((void**)&m_pa, num_dims, dims, num_fields, fieldnames) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    explicit mwArray(mxDouble re) : m_pa(0)
    {
        if (mclGetScalarDouble((void**)&m_pa, re, 0, mxREAL) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    mwArray(mxDouble re, mxDouble im) : m_pa(0)
    {
        if (mclGetScalarDouble((void**)&m_pa, re, im, mxCOMPLEX) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    explicit mwArray(mxSingle re) : m_pa(0)
    {
        if (mclGetScalarSingle((void**)&m_pa, re, 0, mxREAL) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    mwArray(mxSingle re, mxSingle im) : m_pa(0)
    {
        if (mclGetScalarSingle((void**)&m_pa, re, im, mxCOMPLEX) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    explicit mwArray(mxInt8 re) : m_pa(0)
    {
        if (mclGetScalarInt8((void**)&m_pa, re, 0, mxREAL) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    mwArray(mxInt8 re, mxInt8 im) : m_pa(0)
    {
        if (mclGetScalarInt8((void**)&m_pa, re, im, mxCOMPLEX) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    explicit mwArray(mxUint8 re) : m_pa(0)
    {
        if (mclGetScalarUint8((void**)&m_pa, re, 0, mxREAL) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    mwArray(mxUint8 re, mxUint8 im) : m_pa(0)
    {
        if (mclGetScalarUint8((void**)&m_pa, re, im, mxCOMPLEX) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    explicit mwArray(mxInt16 re) : m_pa(0)
    {
        if (mclGetScalarInt16((void**)&m_pa, re, 0, mxREAL) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    mwArray(mxInt16 re, mxInt16 im) : m_pa(0)
    {
        if (mclGetScalarInt16((void**)&m_pa, re, im, mxCOMPLEX) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    explicit mwArray(mxUint16 re) : m_pa(0)
    {
        if (mclGetScalarUint16((void**)&m_pa, re, 0, mxREAL) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    mwArray(mxUint16 re, mxUint16 im) : m_pa(0)
    {
        if (mclGetScalarUint16((void**)&m_pa, re, im, mxCOMPLEX) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    explicit mwArray(mxInt32 re) : m_pa(0)
    {
        if (mclGetScalarInt32((void**)&m_pa, re, 0, mxREAL) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    mwArray(mxInt32 re, mxInt32 im) : m_pa(0)
    {
        if (mclGetScalarInt32((void**)&m_pa, re, im, mxCOMPLEX) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    explicit mwArray(mxUint32 re) : m_pa(0)
    {
        if (mclGetScalarUint32((void**)&m_pa, re, 0, mxREAL) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    mwArray(mxUint32 re, mxUint32 im) : m_pa(0)
    {
        if (mclGetScalarUint32((void**)&m_pa, re, im, mxCOMPLEX) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    explicit mwArray(mxInt64 re) : m_pa(0)
    {
        if (mclGetScalarInt64((void**)&m_pa, re, 0, mxREAL) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    mwArray(mxInt64 re, mxInt64 im) : m_pa(0)
    {
        if (mclGetScalarInt64((void**)&m_pa, re, im, mxCOMPLEX) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    explicit mwArray(mxUint64 re) : m_pa(0)
    {
        if (mclGetScalarUint64((void**)&m_pa, re, 0, mxREAL) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
    mwArray(mxUint64 re, mxUint64 im) : m_pa(0)
    {
        if (mclGetScalarUint64((void**)&m_pa, re, im, mxCOMPLEX) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
#if !defined(__APPLE_CC__)
    explicit mwArray(mxLogical re) : m_pa(0)
    {
        if (mclGetScalarLogical((void**)&m_pa, re) == MCLCPP_ERR)
            mwException::raise_error();
        validate();
    }
#endif
    mwArray(const mwArray& arr) : m_pa(0)
    {
        m_pa = array_ref_deep_copy(arr.m_pa);
        if (!m_pa)
            mwException::raise_error();
    }
    mwArray(const mwArraySharedCopy<mwArray>& sharedCopy)
    {
        m_pa = array_ref_shared_copy(sharedCopy.m_pa);
    }
    virtual ~mwArray()
    {
        ref_count_obj_release(m_pa);
    }
protected:
    mwArray(array_ref* pa, bool incref = false) : m_pa(pa)
    {
        if (!m_pa)
            throw mwException("Internal Error");
        if (incref)
            ref_count_obj_addref(m_pa);
    }
    virtual void validate()
    {
        if (!m_pa)
            throw mwException("Internal Error");
    }
    array_ref* get_ptr() const
    {
        return m_pa;
    }
    void set_ptr(array_ref* pa)
    {
        if (!pa)
            throw mwException("Null pointer");
        ref_count_obj_release(m_pa);
        m_pa = pa;
        ref_count_obj_addref(m_pa);
    }
public:
    mwArray Clone() const
    {
        array_ref* p = array_ref_deep_copy(m_pa);
        if (!p)
            mwException::raise_error();
        return mwArray(p);
    }
    mwArraySharedCopy<mwArray> SharedCopy() const
    {
        array_ref* p = array_ref_shared_copy(m_pa);
        if (!p)
            mwException::raise_error();
        return mwArraySharedCopy<mwArray>(p);
    }
    mwArray Serialize() const
    {
        array_ref* p = array_ref_serialize(m_pa);
        if (!p)
            mwException::raise_error();
        return mwArray(p);
    }
    mxClassID ClassID() const
    {
        return array_ref_classID(m_pa);
    }
    size_t ElementSize() const
    {
        return array_ref_element_size(m_pa);
    }
    mwSize NumberOfElements() const
    {
        return array_ref_number_of_elements(m_pa);
    }
    mwSize NumberOfNonZeros() const
    {
        return array_ref_number_of_nonzeros(m_pa);
    }
    mwSize MaximumNonZeros() const
    {
        return array_ref_maximum_nonzeros(m_pa);
    }
    mwSize NumberOfDimensions() const
    {
        return array_ref_number_of_dimensions(m_pa);
    }
    int NumberOfFields() const
    {
        return array_ref_number_of_fields(m_pa);
    }
    mwString GetFieldName(int i)
    {
        char_buffer* p = array_ref_get_field_name(m_pa, i);
        if (!p)
            mwException::raise_error();
        return mwString(p, false);
    }
    mwArray GetDimensions() const
    {
        array_ref* p = array_ref_get_dimensions(m_pa);
        if (!p)
            mwException::raise_error();
        return mwArray(p);
    }
    bool IsEmpty() const
    {
        return array_ref_is_empty(m_pa);
    }
    bool IsSparse() const
    {
        return array_ref_is_sparse(m_pa);
    }
    bool IsNumeric() const
    {
        return array_ref_is_numeric(m_pa);
    }
    bool IsComplex() const
    {
        return array_ref_is_complex(m_pa);
    }
    bool Equals(const mwArray& arr) const
    {
        return array_ref_equals(m_pa, arr.m_pa);
    }
    int CompareTo(const mwArray& arr) const
    {
        return array_ref_compare_to(m_pa, arr.m_pa);
    }
    int HashCode() const
    {
        return array_ref_hash_code(m_pa);
    }
    mwString ToString() const
    {
        char_buffer* p = array_ref_to_string(m_pa);
        if (!p)
            mwException::raise_error();
        return mwString(p, false);
    }
    mwArray RowIndex() const
    {
        array_ref* p = array_ref_row_index(m_pa);
        if (!p)
            mwException::raise_error();
        return mwArray(p);
    }
    mwArray ColumnIndex() const
    {
        array_ref* p = array_ref_column_index(m_pa);
        if (!p)
            mwException::raise_error();
        return mwArray(p);
    }
    void MakeComplex()
    {
        if (array_ref_make_complex(m_pa) == MCLCPP_ERR)
            mwException::raise_error();
    }

    bool operator==(const mwArray& arr) const
    {
        return Equals(arr);
    }
    bool operator!=(const mwArray& arr) const
    {
        return !Equals(arr);
    }
    bool operator<(const mwArray& arr) const
    {
        return (CompareTo(arr) < 0);
    }
    bool operator>(const mwArray& arr) const
    {
        return (CompareTo(arr) > 0);
    }
    bool operator<=(const mwArray& arr) const
    {
        return (CompareTo(arr) <= 0);
    }
    bool operator>=(const mwArray& arr) const
    {
        return (CompareTo(arr) >= 0);
    }
    friend std::ostream& operator<<(std::ostream& os, const mwArray& arr)
    {
        os << arr.ToString();
        return os;
    }
    mwArray GetA(mwSize num_indices, const mwIndex* index)
    {
        array_ref* p = array_ref_get_int(m_pa, num_indices, index);
        if (!p)
            mwException::raise_error();
        return mwArray(p);
    }
    const mwArray GetA(mwSize num_indices, const mwIndex* index) const
    {
        array_ref* p = array_ref_get_int(m_pa, num_indices, index);
        if (!p)
            mwException::raise_error();
        return mwArray(p);
    }
    mwArray GetA(const char* name, mwIndex index)
    {
        array_ref* p = array_ref_get_const_char(m_pa, name, 1, &index);
        if (!p)
            mwException::raise_error();
        return mwArray(p);
    }
    const mwArray GetA(const char* name, mwIndex index) const
    {
        array_ref* p = array_ref_get_const_char(m_pa, name, 1, &index);
        if (!p)
            mwException::raise_error();
        return mwArray(p);
    }
    mwArray GetA(const char* name, mwSize num_indices, const mwIndex* index)
    {
        array_ref* p = array_ref_get_const_char(m_pa, name, num_indices,
						index);
        if (!p)
            mwException::raise_error();
        return mwArray(p);
    }
    const mwArray GetA(const char* name, mwSize num_indices, const mwIndex* index) const
    {
        array_ref* p = array_ref_get_const_char(m_pa, name, num_indices,
						index);
        if (!p)
            mwException::raise_error();
        return mwArray(p);
    }
    mwArray Get(mwSize num_indices, mwIndex i1)
    {
        return GetPromoted( 1, i1); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2)
    {
        return GetPromoted( 2, i1,  i2); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3)
    {
        return GetPromoted( 3, i1,  i2,  i3); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4)
    {
        return GetPromoted( 4, i1,  i2,  i3,  i4); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5)
    { 
        return GetPromoted( 5, i1,  i2,  i3,  i4,  i5); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6)
    {
        return GetPromoted( 6, i1,  i2,  i3,  i4,  i5,  i6); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7)
    {
        return GetPromoted( 7, i1,  i2,  i3,  i4,  i5,  i6,  i7); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8)
    {
        return GetPromoted( 8, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9)
    {
        return GetPromoted( 9, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10)
    {
        return GetPromoted( 10, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11)
    {
        return GetPromoted( 11, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12)
    {
        return GetPromoted( 12, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13)
    {
        return GetPromoted( 13, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14)
    {
        return GetPromoted( 14, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15)
    {
        return GetPromoted( 15, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16)
    {
        return GetPromoted( 16, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17)
    {
        return GetPromoted( 17, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18)
    {
        return GetPromoted( 18, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19)
    {
        return GetPromoted( 19, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20)
    {
        return GetPromoted( 20, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21)
    {
        return GetPromoted( 21, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22)
    {
        return GetPromoted( 22, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23)
    {
        return GetPromoted( 23, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24)
    {
        return GetPromoted( 24, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25)
    {
        return GetPromoted( 25, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26)
    {
        return GetPromoted( 26, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27)
    {
        return GetPromoted( 27, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28)
    {
        return GetPromoted( 28, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29)
    {
        return GetPromoted( 29, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30)
    {
        return GetPromoted( 30, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30, mwIndex i31)
    {
        return GetPromoted( 31, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30, i31); 
    }
    mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30, mwIndex i31, mwIndex i32)
    {
        return GetPromoted( 32, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30, i31, i32); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1) const
    {
        return GetPromoted( 1, i1); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2) const
    {
        return GetPromoted( 2, i1,  i2); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3) const
    {
        return GetPromoted( 3, i1,  i2,  i3); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4) const
    {
        return GetPromoted( 4, i1,  i2,  i3,  i4); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5) const
    {
        return GetPromoted( 5, i1,  i2,  i3,  i4,  i5); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6) const
    {
        return GetPromoted( 6, i1,  i2,  i3,  i4,  i5,  i6); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7) const
    {
        return GetPromoted( 7, i1,  i2,  i3,  i4,  i5,  i6,  i7); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8) const
    {
        return GetPromoted( 8, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9) const
    {
        return GetPromoted( 9, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10) const
    {
        return GetPromoted( 10, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11) const
    {
        return GetPromoted( 11, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12) const
    {
        return GetPromoted( 12, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13) const
    {
        return GetPromoted( 13, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14) const
    {
        return GetPromoted( 14, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15) const
    {
        return GetPromoted( 15, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16) const
    {
        return GetPromoted( 16, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17) const
    {
        return GetPromoted( 17, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18) const
    {
        return GetPromoted( 18, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19) const
    {
        return GetPromoted( 19, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20) const
    {
        return GetPromoted( 20, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21) const
    {
        return GetPromoted( 21, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22) const
    {
        return GetPromoted( 22, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23) const
    {
        return GetPromoted( 23, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24) const
    {
        return GetPromoted( 24, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25) const
    {
        return GetPromoted( 25, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26) const
    {
        return GetPromoted( 26, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27) const
    {
        return GetPromoted( 27, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28) const
    {
        return GetPromoted( 28, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29) const
    {
        return GetPromoted( 29, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30) const
    {
        return GetPromoted( 30, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30, mwIndex i31) const
    {
        return GetPromoted( 31, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30, i31); 
    }
    const mwArray Get(mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30, mwIndex i31, mwIndex i32) const
    {
        return GetPromoted( 32, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30, i31, i32); 
    }
    mwArray GetPromoted(mwSize num_indices, ...)
    {
        va_list vargs;
        va_start(vargs, num_indices);
        array_ref* p = array_ref_getV_int(m_pa, num_indices, vargs);
        va_end(vargs);
        if (!p)
            mwException::raise_error();
        return mwArray(p);
    }
    const mwArray GetPromoted(mwSize num_indices, ...) const
    {
        va_list vargs;
        va_start(vargs, num_indices);
        array_ref* p = array_ref_getV_int(m_pa, num_indices, vargs);
        va_end(vargs);
        if (!p)
            mwException::raise_error();
        return mwArray(p);
    }
    mwArray Get(const char* name, mwSize num_indices, mwIndex i1)
    {
        return GetPromoted( name, 1, i1); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2)
    {
        return GetPromoted( name, 2, i1,  i2); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3)
    {
        return GetPromoted( name, 3, i1,  i2,  i3); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4)
    {
        return GetPromoted( name, 4, i1,  i2,  i3,  i4); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5)
    { 
        return GetPromoted( name, 5, i1,  i2,  i3,  i4,  i5); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6)
    {
        return GetPromoted( name, 6, i1,  i2,  i3,  i4,  i5,  i6); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7)
    {
        return GetPromoted( name, 7, i1,  i2,  i3,  i4,  i5,  i6,  i7); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8)
    {
        return GetPromoted( name, 8, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9)
    {
        return GetPromoted( name, 9, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10)
    {
        return GetPromoted( name, 10, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11)
    {
        return GetPromoted( name, 11, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12)
    {
        return GetPromoted( name, 12, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13)
    {
        return GetPromoted( name, 13, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14)
    {
        return GetPromoted( name, 14, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15)
    {
        return GetPromoted( name, 15, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16)
    {
        return GetPromoted( name, 16, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17)
    {
        return GetPromoted( name, 17, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18)
    {
        return GetPromoted( name, 18, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19)
    {
        return GetPromoted( name, 19, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20)
    {
        return GetPromoted( name, 20, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21)
    {
        return GetPromoted( name, 21, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22)
    {
        return GetPromoted( name, 22, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23)
    {
        return GetPromoted( name, 23, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24)
    {
        return GetPromoted( name, 24, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25)
    {
        return GetPromoted( name, 25, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26)
    {
        return GetPromoted( name, 26, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27)
    {
        return GetPromoted( name, 27, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28)
    {
        return GetPromoted( name, 28, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29)
    {
        return GetPromoted( name, 29, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30)
    {
        return GetPromoted( name, 30, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30, mwIndex i31)
    {
        return GetPromoted( name, 31, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30, i31); 
    }
    mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30, mwIndex i31, mwIndex i32)
    {
        return GetPromoted( name, 32, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30, i31, i32); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1) const
    {
        return GetPromoted( name, 1, i1); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2) const
    {
        return GetPromoted( name, 2, i1,  i2); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3) const
    {
        return GetPromoted( name, 3, i1,  i2,  i3); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4) const
    {
        return GetPromoted( name, 4, i1,  i2,  i3,  i4); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5) const
    {
        return GetPromoted( name, 5, i1,  i2,  i3,  i4,  i5); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6) const
    {
        return GetPromoted( name, 6, i1,  i2,  i3,  i4,  i5,  i6); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7) const
    {
        return GetPromoted( name, 7, i1,  i2,  i3,  i4,  i5,  i6,  i7); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8) const
    {
        return GetPromoted( name, 8, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9) const
    {
        return GetPromoted( name, 9, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10) const
    {
        return GetPromoted( name, 10, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11) const
    {
        return GetPromoted( name, 11, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12) const
    {
        return GetPromoted( name, 12, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13) const
    {
        return GetPromoted( name, 13, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14) const
    {
        return GetPromoted( name, 14, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15) const
    {
        return GetPromoted( name, 15, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16) const
    {
        return GetPromoted( name, 16, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17) const
    {
        return GetPromoted( name, 17, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18) const
    {
        return GetPromoted( name, 18, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19) const
    {
        return GetPromoted( name, 19, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20) const
    {
        return GetPromoted( name, 20, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21) const
    {
        return GetPromoted( name, 21, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22) const
    {
        return GetPromoted( name, 22, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23) const
    {
        return GetPromoted( name, 23, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24) const
    {
        return GetPromoted( name, 24, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25) const
    {
        return GetPromoted( name, 25, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26) const
    {
        return GetPromoted( name, 26, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27) const
    {
        return GetPromoted( name, 27, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28) const
    {
        return GetPromoted( name, 28, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29) const
    {
        return GetPromoted( name, 29, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30) const
    {
        return GetPromoted( name, 30, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30, mwIndex i31) const
    {
        return GetPromoted( name, 31, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30, i31); 
    }
    const mwArray Get(const char *name, mwSize num_indices, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30, mwIndex i31, mwIndex i32) const
    {
        return GetPromoted( name, 32, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30, i31, i32); 
    }
    mwArray GetPromoted(const char* name, mwSize num_indices, ...)
    {
        va_list vargs;
        va_start(vargs, num_indices);
        array_ref* p = array_ref_getV_const_char(m_pa, name, num_indices,
						 vargs);
        va_end(vargs);
        if (!p)
            mwException::raise_error();
        return mwArray(p);
    }
    const mwArray GetPromoted(const char* name, mwSize num_indices, ...) const
    {
        va_list vargs;
        va_start(vargs, num_indices);
        array_ref* p = array_ref_getV_const_char(m_pa, name, num_indices,
						 vargs);
        va_end(vargs);
        if (!p)
            mwException::raise_error();
        return mwArray(p);
    }
    mwArray Real()
    {
        array_ref* p = array_ref_real(m_pa);
        if (!p)
            mwException::raise_error();
        return mwArray(p);
    }
    const mwArray Real() const
    {
        array_ref* p = array_ref_real(m_pa);
        if (!p)
            mwException::raise_error();
        return mwArray(p);
    }
    mwArray Imag()
    {
        array_ref* p = array_ref_imag(m_pa);
        if (!p)
            mwException::raise_error();
        return mwArray(p);
    }
    const mwArray Imag() const
    {
        array_ref* p = array_ref_imag(m_pa);
        if (!p)
            mwException::raise_error();
        return mwArray(p);
    }
    void Set(const mwArray& arr)
    {
        if (array_ref_set(m_pa, arr.m_pa) == MCLCPP_ERR)
            mwException::raise_error();
    }
    void Set(const mwArraySharedCopy<mwArray>& shared)
    {
        array_ref* pa_sharedCopy = array_ref_shared_copy(shared.m_pa);
        if (NULL == pa_sharedCopy) {
            mwException::raise_error();
        }
        ref_count_obj_release(m_pa);
        m_pa = pa_sharedCopy;
    }
    void GetData(mxDouble* buffer, mwSize len) const
    {
        if (array_ref_get_numeric_mxDouble(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
    void GetData(mxSingle* buffer, mwSize len) const
    {
        if (array_ref_get_numeric_mxSingle(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
    void GetData(mxInt8* buffer, mwSize len) const
    {
        if (array_ref_get_numeric_mxInt8(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
    void GetData(mxUint8* buffer, mwSize len) const
    {
        if (array_ref_get_numeric_mxUint8(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
    void GetData(mxInt16* buffer, mwSize len) const
    {
        if (array_ref_get_numeric_mxInt16(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
    void GetData(mxUint16* buffer, mwSize len) const
    {
        if (array_ref_get_numeric_mxUint16(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
    void GetData(mxInt32* buffer, mwSize len) const
    {
        if (array_ref_get_numeric_mxInt32(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
    void GetData(mxUint32* buffer, mwSize len) const
    {
        if (array_ref_get_numeric_mxUint32(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
    void GetData(mxInt64* buffer, mwSize len) const
    {
        if (array_ref_get_numeric_mxInt64(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
    void GetData(mxUint64* buffer, mwSize len) const
    {
        if (array_ref_get_numeric_mxUint64(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
#ifdef __APPLE__
    void GetData(mwIndex* buffer, mwSize len) const
    {
#ifdef _LP64
        if (array_ref_get_numeric_mxUint64(m_pa, static_cast<mxUint64 *>(static_cast<void *>(buffer)), len) == MCLCPP_ERR)
            mwException::raise_error();
#else
        if (array_ref_get_numeric_mxUint32(m_pa, static_cast<mxUint32 *>(static_cast<void *>(buffer)), len) == MCLCPP_ERR)
            mwException::raise_error();
#endif
    }
#endif
    void GetLogicalData(mxLogical* buffer, mwSize len) const
    {
        if (array_ref_get_logical(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
    void GetCharData(mxChar* buffer, mwSize len) const
    {
        if (array_ref_get_char(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
    void SetData(mxDouble* buffer, mwSize len)
    {
        if (array_ref_set_numeric_mxDouble(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
    void SetData(mxSingle* buffer, mwSize len)
    {
        if (array_ref_set_numeric_mxSingle(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
    void SetData(mxInt8* buffer, mwSize len)
    {
        if (array_ref_set_numeric_mxInt8(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
    void SetData(mxUint8* buffer, mwSize len)
    {
        if (array_ref_set_numeric_mxUint8(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
    void SetData(mxInt16* buffer, mwSize len)
    {
        if (array_ref_set_numeric_mxInt16(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
    void SetData(mxUint16* buffer, mwSize len)
    {
        if (array_ref_set_numeric_mxUint16(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
    void SetData(mxInt32* buffer, mwSize len)
    {
        if (array_ref_set_numeric_mxInt32(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
    void SetData(mxUint32* buffer, mwSize len)
    {
        if (array_ref_set_numeric_mxUint32(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
    void SetData(mxInt64* buffer, mwSize len)
    {
        if (array_ref_set_numeric_mxInt64(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
    void SetData(mxUint64* buffer, mwSize len)
    {
        if (array_ref_set_numeric_mxUint64(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
#ifdef __APPLE
    void SetData(mwIndex* buffer, mwSize len)
    {
#ifdef _LP64
        if (array_ref_set_numeric_mxUint64(m_pa, static_cast<mxUint64 *>(static_cast<void *>(buffer))buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
#else
        if (array_ref_set_numeric_mxUint32(m_pa, static_cast<mxUint32 *>(static_cast<void *>(buffer)), len) == MCLCPP_ERR)
            mwException::raise_error();
#endif
    }
#endif
    void SetLogicalData(mxLogical* buffer, mwSize len)
    {
        if (array_ref_set_logical(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
    void SetCharData(mxChar* buffer, mwSize len)
    {
        if (array_ref_set_char(m_pa, buffer, len) == MCLCPP_ERR)
            mwException::raise_error();
    }
    mwArray& operator=(const mwArray& arr)
    {
        if (&arr == const_cast<const mwArray*>(this)) {
            return *this;
        }
        Set(arr);
        return *this;
    }
    mwArray& operator=(const mwArraySharedCopy<mwArray>& arr)
    {
        if (static_cast<const mwArray*>(&arr) == const_cast<const mwArray*>(this)) {
            return *this;
        }
        Set(arr);
        return *this;
    }
    mwArray operator()(mwIndex i1)
    {
	    return GetPromoted(1, i1);
    }
    const mwArray operator()(mwIndex i1) const
    {
	    return GetPromoted(1, i1);
    }
    mwArray operator()(mwIndex i1, mwIndex i2)
    {
	    return GetPromoted(2, i1, i2);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2) const
    {
        return GetPromoted(2, i1, i2);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3)
    {
	    return GetPromoted(3, i1, i2, i3);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3) const
    {
        return GetPromoted(3, i1, i2, i3);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4)
    {
	    return GetPromoted(4, i1, i2, i3, i4);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4) const
    {
        return GetPromoted(4, i1, i2, i3, i4);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5)
    {
	    return GetPromoted(5, i1, i2, i3, i4, i5);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5) const
    {
        return GetPromoted(5, i1, i2, i3, i4, i5);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6)
    {
	    return GetPromoted(6, i1, i2, i3, i4, i5, i6);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6) const
    {
        return GetPromoted(6, i1, i2, i3, i4, i5, i6);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7)
    {
	    return GetPromoted(7, i1, i2, i3, i4, i5, i6, i7);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7) const
    {
        return GetPromoted(7, i1, i2, i3, i4, i5, i6, i7);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8)
    {
	    return GetPromoted(8, i1, i2, i3, i4, i5, i6, i7, i8);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8) const
    {
        return GetPromoted(8, i1, i2, i3, i4, i5, i6, i7, i8);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9)
    {
	    return GetPromoted(9, i1, i2, i3, i4, i5, i6, i7, i8, i9);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9) const
    {
        return GetPromoted(9, i1, i2, i3, i4, i5, i6, i7, i8, i9);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10)
    {
	    return GetPromoted(10, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10) const
    {
        return GetPromoted(10, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11)
    {
	    return GetPromoted(11, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11) const
    {
        return GetPromoted(11, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12)
    {
	    return GetPromoted(12, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12) const
    {
        return GetPromoted(12, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13)
    {
	    return GetPromoted(13, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13) const
    {
        return GetPromoted(13, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14)
    {
	    return GetPromoted(14, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14) const
    {
        return GetPromoted(14, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15)
    {
	    return GetPromoted(15, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15) const
    {
        return GetPromoted(15, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16)
    {
	    return GetPromoted(16, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15, i16);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16) const
    {
        return GetPromoted(16, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15, i16);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17)
    {
	    return GetPromoted(17, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15, i16, i17);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17) const
    {
        return GetPromoted(17, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15, i16, i17);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18)
    {
	    return GetPromoted(18, i1, i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18) const
    {
        return GetPromoted(18, i1, i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19)
    {
	    return GetPromoted(19, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19) const
    {
        return GetPromoted(19, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20)
    {
	    return GetPromoted(20, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20) const
    {
        return GetPromoted(20, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21)
    {
	    return GetPromoted(21, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21) const
    {
        return GetPromoted(21, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22)
    {
	    return GetPromoted(22, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22) const
    {
        return GetPromoted(22, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23)
    {
	    return GetPromoted(23, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23) const
    {
        return GetPromoted(23, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24)
    {
	    return GetPromoted(24, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24) const
    {
        return GetPromoted(24, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25)
    {
	    return GetPromoted(25, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25) const
    {
	    return GetPromoted(25, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26)
    {
	    return GetPromoted(26, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25, i26);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26) const
    {
        return GetPromoted(26, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25, i26);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27)
    {
	    return GetPromoted(27, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27) const
    {
        return GetPromoted(27, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28)
    {
	    return GetPromoted(28, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28) const
    {
        return GetPromoted(28, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29)
    {
	    return GetPromoted(29, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29) const
    {
        return GetPromoted(29, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30)
    {
	    return GetPromoted(30, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30) const
    {
        return GetPromoted(30, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30, mwIndex i31)
    {
	    return GetPromoted(31, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30, i31);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30, mwIndex i31) const
    {
        return GetPromoted(31, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30, i31);
    }
    mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30, mwIndex i31, mwIndex i32)
    {
	    return GetPromoted(32, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30, i31, i32);
    }
    const mwArray operator()(mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30, mwIndex i31, mwIndex i32) const
    {
	    return GetPromoted(32, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30, i31, i32);
    }
    mwArray operator()(const char* name, mwIndex i1)
    {
	    return GetPromoted(name, 1, i1);
    }
    const mwArray operator()(const char* name, mwIndex i1) const
    {
	    return GetPromoted(name, 1, i1);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2)
    {
	    return GetPromoted(name, 2, i1, i2);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2) const
    {
        return GetPromoted(name, 2, i1, i2);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3)
    {
	    return GetPromoted(name, 3, i1, i2, i3);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3) const
    {
        return GetPromoted(name, 3, i1, i2, i3);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4)
    {
	    return GetPromoted(name, 4, i1, i2, i3, i4);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4) const
    {
        return GetPromoted(name, 4, i1, i2, i3, i4);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5)
    {
	    return GetPromoted(name, 5, i1, i2, i3, i4, i5);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5) const
    {
        return GetPromoted(name, 5, i1, i2, i3, i4, i5);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6)
    {
	    return GetPromoted(name, 6, i1, i2, i3, i4, i5, i6);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6) const
    {
        return GetPromoted(name, 6, i1, i2, i3, i4, i5, i6);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7)
    {
	    return GetPromoted(name, 7, i1, i2, i3, i4, i5, i6, i7);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7) const
    {
        return GetPromoted(name, 7, i1, i2, i3, i4, i5, i6, i7);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8)
    {
	    return GetPromoted(name, 8, i1, i2, i3, i4, i5, i6, i7, i8);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8) const
    {
        return GetPromoted(name, 8, i1, i2, i3, i4, i5, i6, i7, i8);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9)
    {
	    return GetPromoted(name, 9, i1, i2, i3, i4, i5, i6, i7, i8, i9);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9) const
    {
        return GetPromoted(name, 9, i1, i2, i3, i4, i5, i6, i7, i8, i9);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10)
    {
	    return GetPromoted(name, 10, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10) const
    {
        return GetPromoted(name, 10, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11)
    {
	    return GetPromoted(name, 11, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11) const
    {
        return GetPromoted(name, 11, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12)
    {
	    return GetPromoted(name, 12, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12) const
    {
        return GetPromoted(name, 12, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13)
    {
	    return GetPromoted(name, 13, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13) const
    {
        return GetPromoted(name, 13, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14)
    {
	    return GetPromoted(name, 14, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14) const
    {
        return GetPromoted(name, 14, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15)
    {
	    return GetPromoted(name, 15, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15) const
    {
        return GetPromoted(name, 15, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16)
    {
	    return GetPromoted(name, 16, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15, i16);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16) const
    {
        return GetPromoted(name, 16, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15, i16);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17)
    {
	    return GetPromoted(name, 17, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15, i16, i17);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17) const
    {
        return GetPromoted(name, 17, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15, i16, i17);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18)
    {
	    return GetPromoted(name, 18, i1, i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18) const
    {
        return GetPromoted(name, 18, i1, i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19)
    {
	    return GetPromoted(name, 19, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19) const
    {
        return GetPromoted(name, 19, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20)
    {
	    return GetPromoted(name, 20, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20) const
    {
        return GetPromoted(name, 20, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21)
    {
	    return GetPromoted(name, 21, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21) const
    {
        return GetPromoted(name, 21, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22)
    {
	    return GetPromoted(name, 22, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22) const
    {
        return GetPromoted(name, 22, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23)
    {
	    return GetPromoted(name, 23, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23) const
    {
        return GetPromoted(name, 23, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24)
    {
	    return GetPromoted(name, 24, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24) const
    {
        return GetPromoted(name, 24, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25)
    {
	    return GetPromoted(name, 25, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25) const
    {
	    return GetPromoted(name, 25, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26)
    {
	    return GetPromoted(name, 26, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25, i26);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26) const
    {
        return GetPromoted(name, 26, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25, i26);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27)
    {
	    return GetPromoted(name, 27, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27) const
    {
        return GetPromoted(name, 27, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28)
    {
	    return GetPromoted(name, 28, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28) const
    {
        return GetPromoted(name, 28, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29)
    {
	    return GetPromoted(name, 29, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29) const
    {
        return GetPromoted(name, 29, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30)
    {
	    return GetPromoted(name, 30, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30) const
    {
        return GetPromoted(name, 30, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30, mwIndex i31)
    {
	    return GetPromoted(name, 31, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30, i31);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30, mwIndex i31) const
    {
        return GetPromoted(name, 31, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30, i31);
    }
    mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30, mwIndex i31, mwIndex i32)
    {
	    return GetPromoted(name, 32, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30, i31, i32);
    }
    const mwArray operator()(const char* name, mwIndex i1, mwIndex i2, mwIndex i3, mwIndex i4, mwIndex i5, mwIndex i6, mwIndex i7, mwIndex i8, mwIndex i9, mwIndex i10, mwIndex i11, mwIndex i12, mwIndex i13, mwIndex i14, mwIndex i15, mwIndex i16, mwIndex i17, mwIndex i18, mwIndex i19, mwIndex i20, mwIndex i21, mwIndex i22, mwIndex i23, mwIndex i24, mwIndex i25, mwIndex i26, mwIndex i27, mwIndex i28, mwIndex i29, mwIndex i30, mwIndex i31, mwIndex i32) const
    {
	    return GetPromoted(name, 32, i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9,  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22, i23, i24, i25,  i26, i27, i28, i29, i30, i31, i32);
    }
    mwArray& operator=(const mxDouble& x)
    {
        if (array_ref_set_numeric_mxDouble(m_pa, &x, 1) == MCLCPP_ERR)
            mwException::raise_error();
        return *this;
    }
    mwArray& operator=(const mxSingle& x)
    {
        if (array_ref_set_numeric_mxSingle(m_pa, &x, 1) == MCLCPP_ERR)
            mwException::raise_error();
        return *this;
    }
    mwArray& operator=(const mxInt8& x)
    {
        if (array_ref_set_numeric_mxInt8(m_pa, &x, 1) == MCLCPP_ERR)
            mwException::raise_error();
        return *this;
    }
    mwArray& operator=(const mxUint8& x)
    {
        if (array_ref_set_numeric_mxUint8(m_pa, &x, 1) == MCLCPP_ERR)
            mwException::raise_error();
        return *this;
    }
    mwArray& operator=(const mxInt16& x)
    {
        if (array_ref_set_numeric_mxInt16(m_pa, &x, 1) == MCLCPP_ERR)
            mwException::raise_error();
        return *this;
    }
    mwArray& operator=(const mxUint16& x)
    {
        if (array_ref_set_numeric_mxUint16(m_pa, &x, 1) == MCLCPP_ERR)
            mwException::raise_error();
        return *this;
    }
    mwArray& operator=(const mxInt32& x)
    {
        if (array_ref_set_numeric_mxInt32(m_pa, &x, 1) == MCLCPP_ERR)
            mwException::raise_error();
        return *this;
    }
    mwArray& operator=(const mxUint32& x)
    {
        if (array_ref_set_numeric_mxUint32(m_pa, &x, 1) == MCLCPP_ERR)
            mwException::raise_error();
        return *this;
    }
    mwArray& operator=(const mxInt64& x)
    {
        if (array_ref_set_numeric_mxInt64(m_pa, &x, 1) == MCLCPP_ERR)
            mwException::raise_error();
        return *this;
    }
    mwArray& operator=(const mxUint64& x)
    {
        if (array_ref_set_numeric_mxUint64(m_pa, &x, 1) == MCLCPP_ERR)
            mwException::raise_error();
        return *this;
    }
#if !defined(__APPLE_CC__)
    mwArray& operator=(const mxLogical& x)
    {
        if (array_ref_set_logical(m_pa, &x, 1) == MCLCPP_ERR)
            mwException::raise_error();
        return *this;
    }
#endif
    operator mxDouble() const
    {
        mxDouble x;
        if (array_ref_get_numeric_mxDouble(m_pa, &x, 1) == MCLCPP_ERR)
            mwException::raise_error();
        return x;
    }
    operator mxSingle() const
    {
        mxSingle x;
        if (array_ref_get_numeric_mxSingle(m_pa, &x, 1) == MCLCPP_ERR)
            mwException::raise_error();
        return x;
    }
    operator mxInt8() const
    {
        mxInt8 x;
        if (array_ref_get_numeric_mxInt8(m_pa, &x, 1) == MCLCPP_ERR)
            mwException::raise_error();
        return x;
    }
    operator mxUint8() const
    {
        mxUint8 x;
        if (array_ref_get_numeric_mxUint8(m_pa, &x, 1) == MCLCPP_ERR)
            mwException::raise_error();
        return x;
    }
    operator mxInt16() const
    {
        mxInt16 x;
        if (array_ref_get_numeric_mxInt16(m_pa, &x, 1) == MCLCPP_ERR)
            mwException::raise_error();
        return x;
    }
    operator mxUint16() const
    {
        mxUint16 x;
        if (array_ref_get_numeric_mxUint16(m_pa, &x, 1) == MCLCPP_ERR)
            mwException::raise_error();
        return x;
    }
    operator mxInt32() const
    {
        mxInt32 x;
        if (array_ref_get_numeric_mxInt32(m_pa, &x, 1) == MCLCPP_ERR)
            mwException::raise_error();
        return x;
    }
    operator mxUint32() const
    {
        mxUint32 x;
        if (array_ref_get_numeric_mxUint32(m_pa, &x, 1) == MCLCPP_ERR)
            mwException::raise_error();
        return x;
    }
    operator mxInt64() const
    {
        mxInt64 x;
        if (array_ref_get_numeric_mxInt64(m_pa, &x, 1) == MCLCPP_ERR)
            mwException::raise_error();
        return x;
    }
    operator mxUint64() const
    {
        mxUint64 x;
        if (array_ref_get_numeric_mxUint64(m_pa, &x, 1) == MCLCPP_ERR)
            mwException::raise_error();
        return x;
    }
#if !defined(__APPLE_CC__)
    operator mxLogical() const
    {
        mxLogical x;
        if (array_ref_get_logical(m_pa, &x, 1) == MCLCPP_ERR)
            mwException::raise_error();
        return x;
    }
#endif
    static double GetNaN()
    {
	    return mclGetNaN();
    }
    static double GetEps()
    {
        return mclGetEps();
    } 
    static double GetInf()
    {
	    return mclGetInf();
    }
    static bool IsFinite(double x)
    {
        return mclIsFinite(x);
    }
    static bool IsInf(double x)
    {
        return mclIsInf(x);
    }
    static bool IsNaN(double x)
    {
        return mclIsNaN(x);
    }
    static mwArray Deserialize(const mwArray& arr)
    {
        array_ref* p = 0;
        if (mclDeserializeArray((void**)&p, (void**)&(arr.m_pa)) == MCLCPP_ERR)
            mwException::raise_error();
        return mwArray(p);
    }
    static mwArray NewSparse(mwSize num_rows, mwSize num_cols, mwSize nzmax, mxClassID mxID, mxComplexity cmplx = mxREAL)
    {
        array_ref* p = 0;
        int retval = 0;
        if (mxID == mxLOGICAL_CLASS)
            retval = mclGetLogicalSparse((void **)&p, 0, NULL, 0, NULL, 0, NULL, num_rows, num_cols, nzmax);
        else
            retval = mclGetNumericSparse((void**)&p, 0, NULL, 0, NULL, 0, NULL, NULL, num_rows, num_cols, nzmax, mxID, cmplx);
	    if (retval)
            mwException::raise_error();
        return mwArray(p);
    }
    static mwArray NewSparse(mwSize rowindex_size, const mwIndex* rowindex,
			     mwSize colindex_size, const mwIndex* colindex, mwSize data_size, 
                 const mxDouble* rData, mwSize num_rows, mwSize num_cols, mwSize nzmax)
    {
        array_ref* p = 0;
	    if (mclGetNumericSparse((void **)&p, rowindex_size, rowindex, colindex_size, colindex, 
            data_size, rData, NULL, num_rows, num_cols, nzmax, mxDOUBLE_CLASS, mxREAL))
	            mwException::raise_error();
        return mwArray(p);
    }
    static mwArray NewSparse(mwSize rowindex_size, const mwIndex* rowindex,
			     mwSize colindex_size, const mwIndex* colindex, mwSize data_size, 
                 const mxDouble* rData, mwSize nzmax)
    {
        array_ref* p = 0;
	    if (mclGetNumericSparseInferRowsCols((void **)&p, rowindex_size, rowindex, colindex_size, colindex, 
            data_size, rData, NULL, nzmax, mxDOUBLE_CLASS, mxREAL))
	            mwException::raise_error();
        return mwArray(p);
    }
    static mwArray NewSparse(mwSize rowindex_size, const mwIndex* rowindex,
			     mwSize colindex_size, const mwIndex* colindex, mwSize data_size, 
                 const mxDouble* rData, const mxDouble* iData, mwSize num_rows, mwSize num_cols, mwSize nzmax)
    {
        array_ref* p = 0;
	    if (mclGetNumericSparse((void **)&p, rowindex_size, rowindex, colindex_size, colindex, 
            data_size, rData, iData, num_rows, num_cols, nzmax, mxDOUBLE_CLASS, mxCOMPLEX))
	            mwException::raise_error();
        return mwArray(p);
    }
    static mwArray NewSparse(mwSize rowindex_size, const mwIndex* rowindex,
			     mwSize colindex_size, const mwIndex* colindex, mwSize data_size, 
                 const mxDouble* rData, const mxDouble* iData, mwSize nzmax)
    {
        array_ref* p = 0;
	    if (mclGetNumericSparseInferRowsCols((void **)&p, rowindex_size, rowindex, colindex_size, colindex, 
            data_size, rData, iData, nzmax, mxDOUBLE_CLASS, mxCOMPLEX))
	            mwException::raise_error();
        return mwArray(p);
    }
    static mwArray NewSparse(mwSize rowindex_size, const mwIndex* rowindex,
			     mwSize colindex_size, const mwIndex* colindex, mwSize data_size, 
                 const mxLogical* rData, mwSize num_rows, mwSize num_cols, mwSize nzmax)
    {
        array_ref* p = 0;
	    if (mclGetLogicalSparse((void **)&p, rowindex_size, rowindex, colindex_size, colindex, 
            data_size, rData, num_rows, num_cols, nzmax))
	            mwException::raise_error();
        return mwArray(p);
    }
    static mwArray NewSparse(mwSize rowindex_size, const mwIndex* rowindex,
			     mwSize colindex_size, const mwIndex* colindex, mwSize data_size, 
                 const mxLogical* rData, mwSize nzmax)
    {
        array_ref* p = 0;
	    if (mclGetLogicalSparseInferRowsCols((void **)&p, rowindex_size, rowindex, colindex_size, colindex, 
            data_size, rData, nzmax))
	            mwException::raise_error();
        return mwArray(p);
    }
protected:
    array_ref* m_pa;
};

inline void mclcppMlfFeval(HMCRINSTANCE inst, const char* name, int nargout,
			   int fnout, int fnin, ...)
{
    va_list ap;
    mw_auto_ptr_t<array_buffer> rhs;
    mw_auto_ptr_t<array_buffer> lhs;
    bool bVarargout = fnout < 0;
    bool bVarargin = fnin < 0;
    int i = 0;
    mwIndex ixIndx;

    if (bVarargout) 
        fnout = -fnout;
    if (bVarargin) 
        fnin = -fnin;
    int nin = (bVarargin ? fnin-1 : fnin);
    int nout = (bVarargout ? fnout-1 : fnout);

    // Try to allocate an buffer for the right hand side (input) parameters.
    if ((mclcppGetArrayBuffer((void**)&rhs, nin) == MCLCPP_ERR))
        mwException::raise_error();
    va_start(ap, fnin);

    // Skip over the output arguments
    for (i=0; i<fnout; i++)
        va_arg(ap, mwArray*);

    for (i=1; i<=nin; i++)
    {
        const mwArray* arr = va_arg(ap, const mwArray*);
        if (array_buffer_set(rhs, i, arr->get_ptr()) == MCLCPP_ERR)
            mwException::raise_error();
    }
    if (bVarargin)
    {
        const mwArray* varargin = va_arg(ap, const mwArray*);
        if (varargin->ClassID() == mxCELL_CLASS)
        {
            for (ixIndx=1; ixIndx<=varargin->NumberOfElements(); ixIndx++)
            {
                if (array_buffer_add(rhs, varargin->Get(1, ixIndx).get_ptr())
		    == MCLCPP_ERR)
                    mwException::raise_error();
            }
        }
        else
        {
            if (array_buffer_add(rhs, varargin->get_ptr()) == MCLCPP_ERR)
                mwException::raise_error();
        }
    }

    va_end(ap);

    // Execute function
    if (mclcppFeval(inst, name, nargout, (void**)&lhs,
		    (void*)((array_buffer*)rhs)) == MCLCPP_ERR)
        mwException::raise_error();
    // Process outputs

    va_start(ap, fnin);
    for(i=1; i<=nout && i<=nargout; i++) 
    {
        mwArray* arr = va_arg(ap, mwArray*);
        array_ref* p = array_buffer_get(lhs, i);
        if (!p)
            mwException::raise_error();
        arr->set_ptr(p);
        ref_count_obj_release(p);
    }
    if (bVarargout && i<= nargout)
    {
        mwArray* varargout = va_arg(ap, mwArray*);
        array_ref* p = array_buffer_to_cell(lhs, i, nargout-i+1);
        if (!p)
            mwException::raise_error();
        varargout->set_ptr(p);
        ref_count_obj_release(p);
    }
    va_end(ap);
}

#endif /* ifdef __cplusplus */

#endif /* #ifndef _MCLCPPCLASS_H_ */

