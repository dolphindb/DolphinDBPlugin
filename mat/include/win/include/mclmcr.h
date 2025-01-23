/*
 * PUBLISHed header for libmclmcr, the mclmcr library.
 *
 * Copyright 1984-2015 The MathWorks, Inc.
 * All Rights Reserved.
 */

#if defined(_MSC_VER)
# pragma once
#endif
#if defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ > 3))
# pragma once
#endif

#ifndef mclmcr_published_api_hpp
#define mclmcr_published_api_hpp


#ifndef LIBMWMCLMCR_API_EXTERN_C
#  ifdef __cplusplus
#    define LIBMWMCLMCR_API_EXTERN_C extern "C"
#  else
#    define LIBMWMCLMCR_API_EXTERN_C extern
#  endif
#endif

#ifndef LIBMWMCLMCR_API
#  define LIBMWMCLMCR_API 
#endif

#ifndef LIBMWMCLMCR_API_LIBMWMCLMCR_API_EXTERN_C
#  define LIBMWMCLMCR_API_LIBMWMCLMCR_API_EXTERN_C LIBMWMCLMCR_API_EXTERN_C LIBMWMCLMCR_API
#endif

#ifdef __cplusplus
extern "C" {
#endif
typedef int (*mclReadCtfStreamFcn)(char* s, int n);
#ifdef __cplusplus
}
#endif




/* All the types of components we can generate code for. These values 
 * determine how the component initializes itself.
 */
typedef enum
{
    NoObjectType,
    COMObject,
    JavaObject,
    DotNetObject
} mccComponentType;

typedef enum {
  ExeTarget,
  LibTarget,
  MexTarget,
  SfunTarget,
  AnyTarget
} mccTargetType;


typedef struct mclCtfStream_tag {
    /*This field is only here to prevent an 
      empty struct warning for some compilers. */
    char unused;
} mclCtfStream_tag;

typedef mclCtfStream_tag* mclCtfStream; 
#include <stddef.h>

 
LIBMWMCLMCR_API_EXTERN_C mclCtfStream mclGetStreamFromArraySrc(char *buffer, int bufferSize);


LIBMWMCLMCR_API_EXTERN_C void mclDestroyStream(mclCtfStream pStream);


LIBMWMCLMCR_API_EXTERN_C mclCtfStream mclGetEmbeddedCtfStream(void* handle);


#include "mclbase.h" /*for mclOutputHandler MCRInstanceProxy*/


LIBMWMCLMCR_API_EXTERN_C bool mclInitializeComponentInstanceNonEmbeddedStandalone(   HMCRINSTANCE* inst,
                                                            const char* path_to_component,   
                                                            const char* component_name,
                                                            mccTargetType ttype,
                                                            mclOutputHandlerFcn error_handler,
                                                            mclOutputHandlerFcn print_handler);


LIBMWMCLMCR_API_EXTERN_C bool mclInitializeInstanceWithoutComponent( HMCRINSTANCE* inst,
                                            const char** options, 
                                            size_t count,
                                            mclOutputHandlerFcn error_handler,
                                            mclOutputHandlerFcn print_handler);


LIBMWMCLMCR_API_EXTERN_C bool mclInitializeComponentInstanceCtfFileToCache(  HMCRINSTANCE* inst,
                                                    mclOutputHandlerFcn error_handler,
                                                    mclOutputHandlerFcn print_handler,
                                                    const char* ctfFileLocation);


LIBMWMCLMCR_API_EXTERN_C bool mclInitializeComponentInstanceEmbedded(HMCRINSTANCE* inst,
                                            mclOutputHandlerFcn error_handler,
                                            mclOutputHandlerFcn print_handler,
                                            mclCtfStream ctfStream);


LIBMWMCLMCR_API_EXTERN_C bool mclInitializeComponentInstanceWithCallbk(HMCRINSTANCE* inst,
                                            mclOutputHandlerFcn error_handler,
                                            mclOutputHandlerFcn print_handler,
                                            mclReadCtfStreamFcn readCtfStream_handler,
                                            size_t ctfStreamSize);


LIBMWMCLMCR_API_EXTERN_C bool mclInitializeComponentInstanceFromExtractedComponent( HMCRINSTANCE* inst,
                                                    mclOutputHandlerFcn error_handler,
                                                    mclOutputHandlerFcn print_handler,
                                                    const char* component_id);


LIBMWMCLMCR_API_EXTERN_C bool mclInitializeComponentInstanceFromExtractedLocation( HMCRINSTANCE* inst,
                                                    mclOutputHandlerFcn error_handler,
                                                    mclOutputHandlerFcn print_handler,
                                                    const char* extractedLoc);

 
LIBMWMCLMCR_API_EXTERN_C int mclGetDotNetComponentType(void);

 
LIBMWMCLMCR_API_EXTERN_C int mclGetMCCTargetType(bool isLibrary);


LIBMWMCLMCR_API_EXTERN_C const char * getStandaloneFileName(const char* path_to_ctf, const char* component_name);


LIBMWMCLMCR_API_EXTERN_C bool mclStandaloneGenericMain( size_t argc, 
                               const char ** argv, 
                               const char* ctfFileName,
                               bool ExtractToComponentCache,
                               void* ctfSettings);


LIBMWMCLMCR_API_EXTERN_C bool mclStandaloneCtfxMain( size_t argc,
                            const char** argv);


LIBMWMCLMCR_API_EXTERN_C void mclWaitForFiguresToDie(HMCRINSTANCE inst);



#include <stdarg.h>

typedef real64_T mxDouble;
typedef real32_T mxSingle;
typedef int8_T mxInt8;
typedef uint8_T mxUint8;
typedef int16_T mxInt16;
typedef uint16_T mxUint16;
typedef int32_T mxInt32;
typedef uint32_T mxUint32;
#if !defined(__MW_STDINT_H__)
#  if defined( linux ) || defined( __linux ) || defined( __linux__ )
#    include <stdint.h>
     typedef int64_t mxInt64;
     typedef uint64_t mxUint64;
#  elif defined( macintosh ) || defined( __APPLE__ ) || defined( __APPLE_CC__ )
#    if defined( __GNUC__ )
#      include <stdint.h>
       typedef int64_t mxInt64;
       typedef uint64_t mxUint64;
#    endif
#  elif defined(_MSC_VER)
     typedef __int64 mxInt64;
     typedef unsigned __int64 mxUint64;
#  elif defined(__BORLANDC__)
     typedef __int64 mxInt64;
     typedef unsigned __int64 mxUint64;
#  elif defined(__WATCOMC__)
     typedef __int64 mxInt64;
     typedef unsigned __int64 mxUint64;
#  elif defined(__LCC__)
     typedef __int64 mxInt64;
     typedef unsigned __int64 mxUint64;
#  endif
#else
   typedef int64_T mxInt64;
   typedef uint64_T mxUint64;
#endif



#include "matrix.h"


#ifdef __cplusplus

/* Forward declarations */
class ref_count_obj;
class char_buffer;
class array_ref;
class array_buffer;
class error_info;

/* Class declarations */

class ref_count_obj
{
public:
    /* Virtual destructor required to avoid compiler warnings. */
    virtual ~ref_count_obj() {}
    virtual int addref() = 0;
    virtual int release() = 0;
};

class char_buffer: public ref_count_obj
{
public:
    /* Virtual destructor required to avoid compiler warnings. */
    virtual ~char_buffer() {}
    virtual size_t size() = 0;
    virtual const char* get_buffer() = 0;
    virtual int set_buffer(const char* str) = 0;
    virtual int compare_to(char_buffer* p) = 0;
};

class array_ref: public ref_count_obj
{
public:
    /* Virtual destructor required to avoid compiler warnings. */
    virtual ~array_ref() {}
    virtual mxClassID classID() = 0;
    virtual array_ref* deep_copy() = 0;
    virtual void detach() = 0;
    virtual array_ref* shared_copy() = 0;
    virtual array_ref* serialize() = 0;
    virtual size_t element_size() = 0;
    virtual mwSize number_of_elements() = 0;
    virtual mwSize number_of_nonzeros() = 0;
    virtual mwSize maximum_nonzeros() = 0;
    virtual mwSize number_of_dimensions() = 0;
    virtual array_ref* get_dimensions() = 0;
    virtual int number_of_fields() = 0;
    virtual char_buffer* get_field_name(int i) = 0;
    virtual bool is_empty() = 0;
    virtual bool is_sparse() = 0;
    virtual bool is_numeric() = 0;
    virtual bool is_complex() = 0;
    virtual int make_complex() = 0;
    virtual bool equals(array_ref* p) = 0;
    virtual int compare_to(array_ref* p) = 0;
    virtual int hash_code() = 0;
    virtual char_buffer* to_string() = 0;
    virtual array_ref* row_index() = 0;
    virtual array_ref* column_index() = 0;
    virtual array_ref* get(mwSize num_indices, const mwIndex* index) = 0;
    virtual array_ref* get(const char* name, mwSize num_indices, const mwIndex* index) = 0;
    virtual array_ref* getV(mwSize num_indices, va_list vargs) = 0;
    virtual array_ref* getV(const char* name, mwSize num_indices, va_list vargs) = 0;
    virtual int set(array_ref* p) = 0;
    virtual array_ref* real() = 0;
    virtual array_ref* imag() = 0;
    virtual int get_numeric(mxDouble* x, mwSize len) = 0;
    virtual int get_numeric(mxSingle* x, mwSize len) = 0;
    virtual int get_numeric(mxInt8* x, mwSize len) = 0;
    virtual int get_numeric(mxUint8* x, mwSize len) = 0;
    virtual int get_numeric(mxInt16* x, mwSize len) = 0;
    virtual int get_numeric(mxUint16* x, mwSize len) = 0;
    virtual int get_numeric(mxInt32* x, mwSize len) = 0;
    virtual int get_numeric(mxUint32* x, mwSize len) = 0;
    virtual int get_numeric(mxInt64* x, mwSize len) = 0;
    virtual int get_numeric(mxUint64* x, mwSize len) = 0;
    virtual int get_char(mxChar* x, mwSize len) = 0;
    virtual int get_logical(mxLogical* x, mwSize len) = 0;
    virtual int set_numeric(const mxDouble* x, mwSize len) = 0;
    virtual int set_numeric(const mxSingle* x, mwSize len) = 0;
    virtual int set_numeric(const mxInt8* x, mwSize len) = 0;
    virtual int set_numeric(const mxUint8* x, mwSize len) = 0;
    virtual int set_numeric(const mxInt16* x, mwSize len) = 0;
    virtual int set_numeric(const mxUint16* x, mwSize len) = 0;
    virtual int set_numeric(const mxInt32* x, mwSize len) = 0;
    virtual int set_numeric(const mxUint32* x, mwSize len) = 0;
    virtual int set_numeric(const mxInt64* x, mwSize len) = 0;
    virtual int set_numeric(const mxUint64* x, mwSize len) = 0;
    virtual int set_char(const mxChar* x, mwSize len) = 0;
    virtual int set_logical(const mxLogical* x, mwSize len) = 0;
};

class array_buffer: public ref_count_obj
{
public:
    /* Virtual destructor required to avoid compiler warnings. */
    virtual ~array_buffer() {}
    virtual mwSize size() = 0;
    virtual array_ref* get(mwIndex offset) = 0;
    virtual int set(mwIndex offset, array_ref* p) = 0;
    virtual int add(array_ref* pa) = 0;
    virtual int remove(mwIndex offset) = 0;
    virtual int clear() = 0;
    virtual array_ref* to_cell(mwIndex offset, mwSize len) = 0;
};

class error_info: public ref_count_obj
{
public:
    /* Virtual destructor required to avoid compiler warnings. */
    virtual ~error_info() {}
    virtual const char* get_message() = 0;
    virtual size_t get_stack_trace(char*** stack) = 0;
};

#endif /* #ifdef __cplusplus */


#include "matrix.h"


#ifdef __cplusplus
/* This is an extern "C" API exclusively for use by C++ programs. It
 * exists solely to work around binary incompatibilities between different
 * C++ compilers. For example, a Borland C++ program cannot invoke a method
 * on an array_ref object created by a library compiled with Microsoft Visual 
 * C++. The mwArray API, therefore, invokes array_ref methods indirectly, by
 * passing the array_ref object to one of these extern "C" routines, which
 * are implemented in the MCLMCR module. Since this module is part of the
 * MCR, it can invoke array_ref methods with impunity.
 *
 * This API needs to be protected by #ifdef __cplusplus because some of the
 * input arguments to the functions it contains are pointers to objects.
 * Translating all of these to void *'s is overly complex and unnecessary,
 * as this API is always going to be called from a C++ program.
 */




LIBMWMCLMCR_API_EXTERN_C int ref_count_obj_addref(ref_count_obj *obj);


LIBMWMCLMCR_API_EXTERN_C int ref_count_obj_release(ref_count_obj *obj);


LIBMWMCLMCR_API_EXTERN_C size_t char_buffer_size(char_buffer *obj);


LIBMWMCLMCR_API_EXTERN_C const char* char_buffer_get_buffer(char_buffer *obj);


LIBMWMCLMCR_API_EXTERN_C int char_buffer_set_buffer(char_buffer *obj, const char* str);


LIBMWMCLMCR_API_EXTERN_C int char_buffer_compare_to(char_buffer *obj, char_buffer* p);


LIBMWMCLMCR_API_EXTERN_C mxClassID array_ref_classID(array_ref *obj);


LIBMWMCLMCR_API_EXTERN_C array_ref* array_ref_deep_copy(array_ref *obj);


LIBMWMCLMCR_API_EXTERN_C void array_ref_detach(array_ref *obj);


LIBMWMCLMCR_API_EXTERN_C array_ref* array_ref_shared_copy(array_ref *obj);


LIBMWMCLMCR_API_EXTERN_C array_ref* array_ref_serialize(array_ref *obj);


LIBMWMCLMCR_API_EXTERN_C size_t array_ref_element_size(array_ref *obj);


LIBMWMCLMCR_API_EXTERN_C mwSize array_ref_number_of_elements(array_ref *obj);


LIBMWMCLMCR_API_EXTERN_C mwSize array_ref_number_of_nonzeros(array_ref *obj);


LIBMWMCLMCR_API_EXTERN_C mwSize array_ref_maximum_nonzeros(array_ref *obj);


LIBMWMCLMCR_API_EXTERN_C mwSize array_ref_number_of_dimensions(array_ref *obj);


LIBMWMCLMCR_API_EXTERN_C array_ref* array_ref_get_dimensions(array_ref *obj);


LIBMWMCLMCR_API_EXTERN_C int array_ref_number_of_fields(array_ref *obj);


LIBMWMCLMCR_API_EXTERN_C char_buffer* array_ref_get_field_name(array_ref *obj, int i);


LIBMWMCLMCR_API_EXTERN_C bool array_ref_is_empty(array_ref *obj);


LIBMWMCLMCR_API_EXTERN_C bool array_ref_is_sparse(array_ref *obj);


LIBMWMCLMCR_API_EXTERN_C bool array_ref_is_numeric(array_ref *obj);


LIBMWMCLMCR_API_EXTERN_C bool array_ref_is_complex(array_ref *obj);


LIBMWMCLMCR_API_EXTERN_C int array_ref_make_complex(array_ref *obj);


LIBMWMCLMCR_API_EXTERN_C bool array_ref_equals(array_ref *obj, array_ref* p);


LIBMWMCLMCR_API_EXTERN_C int array_ref_compare_to(array_ref *obj, array_ref* p);


LIBMWMCLMCR_API_EXTERN_C int array_ref_hash_code(array_ref *obj);


LIBMWMCLMCR_API_EXTERN_C char_buffer* array_ref_to_string(array_ref *obj);


LIBMWMCLMCR_API_EXTERN_C array_ref* array_ref_row_index(array_ref *obj);


LIBMWMCLMCR_API_EXTERN_C array_ref* array_ref_column_index(array_ref *obj);


LIBMWMCLMCR_API_EXTERN_C array_ref* array_ref_get_int(array_ref *obj, mwSize num_indices, const mwIndex* index);


LIBMWMCLMCR_API_EXTERN_C array_ref* array_ref_get_const_char(array_ref *obj, const char* name, mwSize num_indices, const mwIndex* index);


LIBMWMCLMCR_API_EXTERN_C array_ref* array_ref_getV_int(array_ref *obj, mwSize num_indices, va_list vargs);


LIBMWMCLMCR_API_EXTERN_C array_ref* array_ref_getV_const_char(array_ref *obj, const char* name, mwSize num_indices, va_list vargs);


LIBMWMCLMCR_API_EXTERN_C int array_ref_set(array_ref *obj, array_ref* p);


LIBMWMCLMCR_API_EXTERN_C array_ref* array_ref_real(array_ref *obj);


LIBMWMCLMCR_API_EXTERN_C array_ref* array_ref_imag(array_ref *obj);


LIBMWMCLMCR_API_EXTERN_C int array_ref_get_numeric_mxDouble(array_ref *obj, mxDouble* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_ref_get_numeric_mxSingle(array_ref *obj, mxSingle* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_ref_get_numeric_mxInt8(array_ref *obj, mxInt8* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_ref_get_numeric_mxUint8(array_ref *obj, mxUint8* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_ref_get_numeric_mxInt16(array_ref *obj, mxInt16* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_ref_get_numeric_mxUint16(array_ref *obj, mxUint16* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_ref_get_numeric_mxInt32(array_ref *obj, mxInt32* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_ref_get_numeric_mxUint32(array_ref *obj, mxUint32* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_ref_get_numeric_mxInt64(array_ref *obj, mxInt64* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_ref_get_numeric_mxUint64(array_ref *obj, mxUint64* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_ref_get_char(array_ref *obj, mxChar* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_ref_get_logical(array_ref *obj, mxLogical* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_ref_set_numeric_mxDouble(array_ref *obj, const mxDouble* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_ref_set_numeric_mxSingle(array_ref *obj, const mxSingle* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_ref_set_numeric_mxInt8(array_ref *obj, const mxInt8* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_ref_set_numeric_mxUint8(array_ref *obj, const mxUint8* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_ref_set_numeric_mxInt16(array_ref *obj, const mxInt16* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_ref_set_numeric_mxUint16(array_ref *obj, const mxUint16* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_ref_set_numeric_mxInt32(array_ref *obj, const mxInt32* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_ref_set_numeric_mxUint32(array_ref *obj, const mxUint32* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_ref_set_numeric_mxInt64(array_ref *obj, const mxInt64* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_ref_set_numeric_mxUint64(array_ref *obj, const mxUint64* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_ref_set_char(array_ref *obj, const mxChar* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_ref_set_logical(array_ref *obj, const mxLogical* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C mwSize array_buffer_size(array_buffer *obj);


LIBMWMCLMCR_API_EXTERN_C array_ref* array_buffer_get(array_buffer *obj, mwIndex offset);


LIBMWMCLMCR_API_EXTERN_C int array_buffer_set(array_buffer *obj, mwIndex offset, array_ref* p);


LIBMWMCLMCR_API_EXTERN_C int array_buffer_add(array_buffer *obj, array_ref* pa);


LIBMWMCLMCR_API_EXTERN_C int array_buffer_remove(array_buffer *obj, mwIndex offset);


LIBMWMCLMCR_API_EXTERN_C int array_buffer_clear(array_buffer *obj);


LIBMWMCLMCR_API_EXTERN_C array_ref* array_buffer_to_cell(array_buffer *obj, mwIndex offset, mwSize len);


LIBMWMCLMCR_API_EXTERN_C const char* error_info_get_message(error_info *obj);


LIBMWMCLMCR_API_EXTERN_C size_t error_info_get_stack_trace(error_info *obj, char*** stack);


#endif /* #ifdef __cplusplus */


#define MCLCPP_OK    0
#define MCLCPP_ERR  -1


LIBMWMCLMCR_API_EXTERN_C int mclcppGetLastError(void** ppv);


LIBMWMCLMCR_API_EXTERN_C int mclcppCreateError(void** ppv, const char* msg);


LIBMWMCLMCR_API_EXTERN_C void mclcppSetLastError(const char* msg);


LIBMWMCLMCR_API_EXTERN_C int mclcppErrorCheck(void);



#ifdef __cplusplus

#include <iostream>
#include <exception>

class mwException : public std::exception
{
public:
    mwException() : std::exception()
    {
        m_err = 0;
        mclcppCreateError((void**)&m_err, "Unspecified error");
    }
    mwException(const char* msg) : std::exception()
    {
        m_err = 0;
        mclcppCreateError((void**)&m_err, (msg ? msg : "Unspecified error"));
    }
    mwException(const mwException& e) : std::exception()
    {
        m_err = e.m_err;
        if (m_err)
            ref_count_obj_addref(m_err);
    }
    mwException(error_info* pe)
    {
        m_err = 0;
        if (pe)
        {
            m_err = pe;
            ref_count_obj_addref(m_err);
        }
        else
        {
            mclcppCreateError((void**)&m_err, "Unspecified error");
        }
    }
    mwException(error_info* pe, bool bAddRef)
    {
        m_err = 0;
        if (pe)
        {
            m_err = pe;
            if (bAddRef)
                ref_count_obj_addref(m_err);
        }
        else
        {
            mclcppCreateError((void**)&m_err, "Unspecified error");
        }
    }
    mwException(const std::exception& e) : std::exception()
    {
        m_err = 0;
        mclcppCreateError((void**)&m_err, e.what());
    }
    virtual ~mwException() throw()
    {
        if (m_err)
            ref_count_obj_release(m_err);
    }
    mwException& operator=(const std::exception& e)
    {
        if (m_err)
            ref_count_obj_release(m_err);
        mclcppCreateError((void**)&m_err, e.what());
        return *this;
    }
    mwException& operator=(const mwException& e)
    {
        if (m_err)
            ref_count_obj_release(m_err);
        m_err = e.m_err;
        if (m_err)
            ref_count_obj_addref(m_err);
        return *this;
    }
    const char *what() const throw()
    {
        return (m_err ? error_info_get_message(m_err) : NULL);
    }
    static void raise_error()
    {
        error_info* pe = 0;
        if (mclcppGetLastError((void**)&pe) == MCLCPP_ERR)
	{
	    throw mwException();
	}
        throw mwException(pe, false);
    }
    void print_stack_trace()
    {
        char** stack;
        size_t stackDepth;
        
        stackDepth = get_stack_trace(&stack);
        if(stackDepth > 0)
        {
            std::cerr << "... Matlab M-code Stack Trace ..." << std::endl;
            for(size_t i=0; i<stackDepth; i++)
            {
                std::cerr << stack[i] << std::endl;
            }
        }
    }
    static void check_raise_error()
    {
        if (!mclcppErrorCheck())
            return;
        mwException::raise_error();
    }
protected:
    error_info* m_err;

private:
    size_t get_stack_trace(char*** stack)
    {
        return (m_err ? error_info_get_stack_trace(m_err, stack) : 0);
    }
};


#endif




#ifdef __cplusplus

template<typename T>
class mw_auto_ptr_t
{
public:
    mw_auto_ptr_t()
    {
        m_p = 0;
    }
    explicit mw_auto_ptr_t(T* p)
    {
        m_p = p;
        addref();
    }
    mw_auto_ptr_t(T*p, bool bAddRef)
    {
        m_p = p;
        if (bAddRef)
            addref();
    }
    virtual ~mw_auto_ptr_t()
    {
        if (m_p)
	    ref_count_obj_release(m_p);
    }
    void addref()
    {
        if (m_p)
	    ref_count_obj_addref(m_p);
    }
    void release()
    {
        if (m_p)
        {
	    ref_count_obj_release(m_p);
            m_p = 0;
        }
    }
    void attach(T* p, bool bAddRef)
    {
        release();
        m_p = p;
        if (bAddRef)
            addref();
    }
    T* detach()
    {
        T* p = m_p;
        m_p = 0;
        return p;
    }
    T* operator->(void) const
    {
        if (!m_p)
            throw mwException("Null pointer");
        return m_p;
    }
    T** operator&(void)
    {
        release();
        return &m_p;
    }
    operator T*() const
    {
        return m_p;
    }
    mw_auto_ptr_t<T>& operator=(T* p)
    {
        release();
        m_p = p;
        addref();
        return *this;
    }
    mw_auto_ptr_t<T>& operator=(const mw_auto_ptr_t<T>& ptr)
    {
        release();
        m_p = ptr.m_p;
        addref();
        return *this;
    }
    bool operator!() const
    {
        return (m_p == 0);
    }
    operator bool() const
    {
        return (m_p != 0);
    }
    bool operator==(const mw_auto_ptr_t<T>& ptr)
    {
        return (m_p == (T*)ptr);
    }
    bool operator!=(const mw_auto_ptr_t<T>& ptr)
    {
        return (m_p != (T*)ptr);
    }
protected:
    T* m_p;
};

#endif /* ifdef  __cplusplus */



LIBMWMCLMCR_API_EXTERN_C int mclCreateCharBuffer(void** ppv, const char* str);



#ifdef __cplusplus

//#include <iostream>


class mwString
{
public:
    mwString()
    {
        if (mclCreateCharBuffer((void**)&m_str, "") == MCLCPP_ERR)
            mwException::raise_error();
    }
    mwString(const char* str)
    {
        if (mclCreateCharBuffer((void**)&m_str, str) == MCLCPP_ERR)
            mwException::raise_error();
    }
    mwString(char_buffer* buff, bool bAddref)
    {
	m_str.attach(buff, bAddref);
    }
    mwString(const mwString& str)
    {
        if (mclCreateCharBuffer((void**)&m_str, str) == MCLCPP_ERR)
            mwException::raise_error();
        if (char_buffer_set_buffer(m_str, (const char*)str) == MCLCPP_ERR)
            mwException::raise_error();
    }
    virtual ~mwString(){}
public:
    mwSize Length() const
    {
        return char_buffer_size(m_str);
    }
    operator const char* () const
    {
        return char_buffer_get_buffer(m_str);
    }
    mwString& operator=(const mwString& str)
    {
        if (&str == this)
            return *this;
        if (char_buffer_set_buffer(m_str, (const char*)str) == MCLCPP_ERR)
            mwException::raise_error();
        return *this;
    }
    mwString& operator=(const char* str)
    {
        if (char_buffer_set_buffer(m_str, str) == MCLCPP_ERR)
            mwException::raise_error();
        return *this;
    }
    bool operator==(const mwString& str) const
    {
        return (char_buffer_compare_to(m_str, str.m_str) == 0);
    }
    bool operator!=(const mwString& str) const
    {
        return (char_buffer_compare_to(m_str, str.m_str) != 0);
    }
    bool operator<(const mwString& str) const
    {
        return (char_buffer_compare_to(m_str, str.m_str) < 0);
    }
    bool operator<=(const mwString& str) const
    {
        return (char_buffer_compare_to(m_str, str.m_str) <= 0);
    }
    bool operator>(const mwString& str) const
    {
        return (char_buffer_compare_to(m_str, str.m_str) > 0);
    }
    bool operator>=(const mwString& str) const
    {
        return (char_buffer_compare_to(m_str, str.m_str) >= 0);
    }
    friend std::ostream& operator<<(std::ostream& os, const mwString& str)
    {
        os << (const char*)str;
        return os;
    }
private:
    mw_auto_ptr_t<char_buffer> m_str;
};

#endif



LIBMWMCLMCR_API_EXTERN_C double mclGetEps(void);


LIBMWMCLMCR_API_EXTERN_C double mclGetInf(void);


LIBMWMCLMCR_API_EXTERN_C double mclGetNaN(void);


LIBMWMCLMCR_API_EXTERN_C bool mclIsFinite(double x);


LIBMWMCLMCR_API_EXTERN_C bool mclIsInf(double x);


LIBMWMCLMCR_API_EXTERN_C bool mclIsNaN(double x);


LIBMWMCLMCR_API_EXTERN_C bool mclIsIdentical(mxArray* pArray1, mxArray* pArray2);


LIBMWMCLMCR_API_EXTERN_C int mclGetEmptyArray(void** ppv, mxClassID classid);


LIBMWMCLMCR_API_EXTERN_C int mclGetMatrix(void** ppv, mwSize num_rows, mwSize num_cols, mxClassID classid, mxComplexity cmplx);


LIBMWMCLMCR_API_EXTERN_C int mclGetArray(void** ppv, mwSize num_dims, const mwSize* dims, mxClassID classid, mxComplexity cmplx);


LIBMWMCLMCR_API_EXTERN_C int mclGetNumericMatrix(void** ppv, mwSize num_rows, mwSize num_cols, mxClassID mxID, mxComplexity cmplx);


LIBMWMCLMCR_API_EXTERN_C int mclGetNumericArray(void** ppv, mwSize num_dims, const mwSize* dims, mxClassID mxID, mxComplexity cmplx);


LIBMWMCLMCR_API_EXTERN_C int mclGetScalarDouble(void** ppv, mxDouble re, mxDouble im, mxComplexity cmplx);


LIBMWMCLMCR_API_EXTERN_C int mclGetScalarSingle(void** ppv, mxSingle re, mxSingle im, mxComplexity cmplx);


LIBMWMCLMCR_API_EXTERN_C int mclGetScalarInt8(void** ppv, mxInt8 re, mxInt8 im, mxComplexity cmplx);


LIBMWMCLMCR_API_EXTERN_C int mclGetScalarUint8(void** ppv, mxUint8 re, mxUint8 im, mxComplexity cmplx);


LIBMWMCLMCR_API_EXTERN_C int mclGetScalarInt16(void** ppv, mxInt16 re, mxInt16 im, mxComplexity cmplx);


LIBMWMCLMCR_API_EXTERN_C int mclGetScalarUint16(void** ppv, mxUint16 re, mxUint16 im, mxComplexity cmplx);


LIBMWMCLMCR_API_EXTERN_C int mclGetScalarInt32(void** ppv, mxInt32 re, mxInt32 im, mxComplexity cmplx);


LIBMWMCLMCR_API_EXTERN_C int mclGetScalarUint32(void** ppv, mxUint32 re, mxUint32 im, mxComplexity cmplx);


LIBMWMCLMCR_API_EXTERN_C int mclGetScalarInt64(void** ppv, mxInt64 re, mxInt64 im, mxComplexity cmplx);


LIBMWMCLMCR_API_EXTERN_C int mclGetScalarUint64(void** ppv, mxUint64 re, mxUint64 im, mxComplexity cmplx);


LIBMWMCLMCR_API_EXTERN_C int mclGetCharMatrix(void** ppv, mwSize num_rows, mwSize num_cols);


LIBMWMCLMCR_API_EXTERN_C int mclGetCharArray(void** ppv, mwSize num_dims, const mwSize* dims);


LIBMWMCLMCR_API_EXTERN_C int mclGetScalarChar(void** ppv, mxChar x);


LIBMWMCLMCR_API_EXTERN_C int mclGetString(void** ppv, const char* str);


LIBMWMCLMCR_API_EXTERN_C int mclGetCharMatrixFromStrings(void** ppv, mwSize m, const char** str);


LIBMWMCLMCR_API_EXTERN_C int mclGetLogicalMatrix(void** ppv, mwSize num_rows, mwSize num_cols);


LIBMWMCLMCR_API_EXTERN_C int mclGetLogicalArray(void** ppv, mwSize num_dims, const mwSize* dims);


LIBMWMCLMCR_API_EXTERN_C int mclGetScalarLogical(void** ppv, mxLogical x);


LIBMWMCLMCR_API_EXTERN_C int mclGetCellMatrix(void** ppv, mwSize num_rows, mwSize num_cols);


LIBMWMCLMCR_API_EXTERN_C int mclGetCellArray(void** ppv, mwSize num_dims, const mwSize* dims);


LIBMWMCLMCR_API_EXTERN_C int mclGetStructMatrix(void** ppv, mwSize num_rows, mwSize num_cols, int nFields, const char** fieldnames);


LIBMWMCLMCR_API_EXTERN_C int mclGetStructArray(void** ppv, mwSize num_dims, const mwSize* dims, int nFields, const char** fieldnames);


LIBMWMCLMCR_API_EXTERN_C int mclGetNumericSparse(
    void** ppv,
    mwSize rowindex_size,
    const mwSize* rowindex,
    mwSize colindex_size,
    const mwSize* colindex,
    mwSize data_size,
    const void* rData,
    const void* iData,
    mwSize num_rows,
    mwSize num_cols,
    mwSize nzmax,
    mxClassID mxType,
    mxComplexity cmplx
    );


LIBMWMCLMCR_API_EXTERN_C int mclGetNumericSparseInferRowsCols(
    void** ppv,
    mwSize rowindex_size,
    const mwSize* rowindex,
    mwSize colindex_size,
    const mwSize* colindex,
    mwSize data_size,
    const void* rData,
    const void* iData,
    mwSize nzmax,
    mxClassID mxType,
    mxComplexity cmplx
    );


LIBMWMCLMCR_API_EXTERN_C int mclGetLogicalSparse(
    void** ppv,
    mwSize rowindex_size,
    const mwIndex* rowindex,
    mwSize colindex_size,
    const mwIndex* colindex,
    mwSize data_size,
    const mxLogical* rData,
    mwSize num_rows,
    mwSize num_cols,
    mwSize nzmax
    );


LIBMWMCLMCR_API_EXTERN_C int mclGetLogicalSparseInferRowsCols(
    void** ppv,
    mwSize rowindex_size,
    const mwIndex* rowindex,
    mwSize colindex_size,
    const mwIndex* colindex,
    mwSize data_size,
    const mxLogical* rData,
    mwSize nzmax
    );


LIBMWMCLMCR_API_EXTERN_C int mclDeserializeArray(void** ppv, void** ppa);


LIBMWMCLMCR_API_EXTERN_C int mclcppGetArrayBuffer(void** ppv, mwSize size);


LIBMWMCLMCR_API_EXTERN_C int mclcppFeval(HMCRINSTANCE inst, const char* name, int nargout, void** lhs, void* rhs);


LIBMWMCLMCR_API_EXTERN_C int mclcppArrayToString(const mxArray* parray, char** ppstr);


LIBMWMCLMCR_API_EXTERN_C void mclcppFreeString(char* pstr);


LIBMWMCLMCR_API_EXTERN_C int mclmxArray2ArrayHandle(void** pphArray, mxArray* pmxArray);


LIBMWMCLMCR_API_EXTERN_C int mclArrayHandle2mxArray(mxArray** ppArrayImpl, void* phArray);


LIBMWMCLMCR_API_EXTERN_C int mclMXArrayGetIndexArrays(mxArray** ppRows, mxArray** ppColumns, mxArray* pSrcArray);


LIBMWMCLMCR_API_EXTERN_C int mclMXArrayGet(mxArray** ppSrcElem, mxArray* pSrcArray, mwSize num_indices, const mwIndex* index);


LIBMWMCLMCR_API_EXTERN_C int mclMXArrayGetReal(mxArray** ppSrcElem, mxArray* pSrcArray, mwSize num_indices, const mwIndex* index);


LIBMWMCLMCR_API_EXTERN_C int mclMXArrayGetImag(mxArray** ppSrcElem, mxArray* pSrcArray, mwSize num_indices, const mwIndex* index);


LIBMWMCLMCR_API_EXTERN_C int mclMXArraySet(mxArray* pTrgArray, mxArray* pSrcElem, mwSize num_indices, const mwIndex* index);


LIBMWMCLMCR_API_EXTERN_C int mclMXArraySetReal(mxArray* pTrgArray, mxArray* pSrcElem, mwSize num_indices, const mwIndex* index);


LIBMWMCLMCR_API_EXTERN_C int mclMXArraySetImag(mxArray* pTrgArray, mxArray* pSrcElem, mwSize num_indices, const mwIndex* index);


LIBMWMCLMCR_API_EXTERN_C int mclMXArraySetLogical(mxArray* pTrgArray, mxArray* pSrcElem, mwSize num_indices, const mwIndex* index);

LIBMWMCLMCR_API_EXTERN_C void mclMxRefDestroyArray(mxArray* pa);

LIBMWMCLMCR_API_EXTERN_C mxArray *mclMxRefSerialize(mxArray* pa);

LIBMWMCLMCR_API_EXTERN_C mxArray *mclMxRefDeserialize(const void* pa,
                                               size_t len, size_t mvmNumber);

LIBMWMCLMCR_API_EXTERN_C size_t mclMxRefMvmId(mxArray *pa);

LIBMWMCLMCR_API_EXTERN_C size_t mclHashNBytes (size_t u, size_t n,
                                               const char * pb);


#include "matrix.h"


#ifdef __cplusplus
/* This public extern "C" API provides functions to create and manipulate
 * arrays. The API represents arrays as oqaque objects (void *'s, to be
 * precise). 
 *
 * The MATLAB Compiler and related Builder products use this API to insulate
 * the generated code from changes in the underlying matrix data structure.
 */



/* Opaque handle type definition. */
typedef void * array_handle;


LIBMWMCLMCR_API_EXTERN_C mxClassID array_handle_classID(array_handle handle);


LIBMWMCLMCR_API_EXTERN_C array_handle array_handle_deep_copy(array_handle handle);


LIBMWMCLMCR_API_EXTERN_C void array_handle_detach(array_handle handle);


LIBMWMCLMCR_API_EXTERN_C array_handle array_handle_shared_copy(array_handle handle);


LIBMWMCLMCR_API_EXTERN_C array_handle array_handle_serialize(array_handle handle);


LIBMWMCLMCR_API_EXTERN_C size_t array_handle_element_size(array_handle handle);


LIBMWMCLMCR_API_EXTERN_C mwSize array_handle_number_of_elements(array_handle handle);


LIBMWMCLMCR_API_EXTERN_C mwSize array_handle_number_of_nonzeros(array_handle handle);


LIBMWMCLMCR_API_EXTERN_C mwSize array_handle_maximum_nonzeros(array_handle handle);


LIBMWMCLMCR_API_EXTERN_C mwSize array_handle_number_of_dimensions(array_handle handle);


LIBMWMCLMCR_API_EXTERN_C array_handle array_handle_get_dimensions(array_handle handle);


LIBMWMCLMCR_API_EXTERN_C int array_handle_number_of_fields(array_handle handle);


LIBMWMCLMCR_API_EXTERN_C char_buffer* array_handle_get_field_name(array_handle handle, int i);


LIBMWMCLMCR_API_EXTERN_C bool array_handle_is_empty(array_handle handle);


LIBMWMCLMCR_API_EXTERN_C bool array_handle_is_sparse(array_handle handle);


LIBMWMCLMCR_API_EXTERN_C bool array_handle_is_numeric(array_handle handle);


LIBMWMCLMCR_API_EXTERN_C bool array_handle_is_complex(array_handle handle);


LIBMWMCLMCR_API_EXTERN_C int array_handle_make_complex(array_handle handle);


LIBMWMCLMCR_API_EXTERN_C bool array_handle_equals(array_handle handle, array_handle p);


LIBMWMCLMCR_API_EXTERN_C int array_handle_compare_to(array_handle handle, array_handle p);


LIBMWMCLMCR_API_EXTERN_C int array_handle_hash_code(array_handle handle);


LIBMWMCLMCR_API_EXTERN_C char_buffer* array_handle_to_string(array_handle handle);


LIBMWMCLMCR_API_EXTERN_C array_handle array_handle_row_index(array_handle handle);


LIBMWMCLMCR_API_EXTERN_C array_handle array_handle_column_index(array_handle handle);


LIBMWMCLMCR_API_EXTERN_C array_handle array_handle_get_int(array_handle handle, mwSize num_indices, const mwIndex* index);


LIBMWMCLMCR_API_EXTERN_C array_handle array_handle_get_const_char(array_handle handle, const char* name, mwSize num_indices, const mwIndex* index);


LIBMWMCLMCR_API_EXTERN_C array_handle array_handle_getV_int(array_handle handle, mwSize num_indices, va_list vargs);


LIBMWMCLMCR_API_EXTERN_C array_handle array_handle_getV_const_char(array_handle handle, const char* name, mwSize num_indices, va_list vargs);


LIBMWMCLMCR_API_EXTERN_C int array_handle_set(array_handle handle, array_handle p);


LIBMWMCLMCR_API_EXTERN_C array_handle array_handle_real(array_handle handle);


LIBMWMCLMCR_API_EXTERN_C array_handle array_handle_imag(array_handle handle);


LIBMWMCLMCR_API_EXTERN_C int array_handle_get_numeric_mxDouble(array_handle handle, mxDouble* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_handle_get_numeric_mxSingle(array_handle handle, mxSingle* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_handle_get_numeric_mxInt8(array_handle handle, mxInt8* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_handle_get_numeric_mxUint8(array_handle handle, mxUint8* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_handle_get_numeric_mxInt16(array_handle handle, mxInt16* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_handle_get_numeric_mxUint16(array_handle handle, mxUint16* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_handle_get_numeric_mxInt32(array_handle handle, mxInt32* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_handle_get_numeric_mxUint32(array_handle handle, mxUint32* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_handle_get_numeric_mxInt64(array_handle handle, mxInt64* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_handle_get_numeric_mxUint64(array_handle handle, mxUint64* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_handle_get_char(array_handle handle, mxChar* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_handle_get_logical(array_handle handle, mxLogical* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_handle_set_numeric_mxDouble(array_handle handle, const mxDouble* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_handle_set_numeric_mxSingle(array_handle handle, const mxSingle* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_handle_set_numeric_mxInt8(array_handle handle, const mxInt8* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_handle_set_numeric_mxUint8(array_handle handle, const mxUint8* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_handle_set_numeric_mxInt16(array_handle handle, const mxInt16* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_handle_set_numeric_mxUint16(array_handle handle, const mxUint16* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_handle_set_numeric_mxInt32(array_handle handle, const mxInt32* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_handle_set_numeric_mxUint32(array_handle handle, const mxUint32* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_handle_set_numeric_mxInt64(array_handle handle, const mxInt64* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_handle_set_numeric_mxUint64(array_handle handle, const mxUint64* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_handle_set_char(array_handle handle, const mxChar* x, mwSize len);


LIBMWMCLMCR_API_EXTERN_C int array_handle_set_logical(array_handle handle, const mxLogical* x, mwSize len);


#endif /* #ifdef __cplusplus */


#include <stdarg.h>
#include <string.h>
#include <wchar.h>


#ifdef __cplusplus
class MclMcr {};
#endif


LIBMWMCLMCR_API_EXTERN_C int mclWrite(int fd, const void *ptr, size_t count);



/* define MW_CALL_CONV to __cdecl if building on windows */
#ifndef MW_CALL_CONV
#if defined( _MSC_VER) || defined(__BORLANDC__) || defined(__WATCOMC__) || defined(__LCC__)
#define MW_CALL_CONV __cdecl
#else
#define MW_CALL_CONV
#endif /* using some PC compiler */
#endif /* MW_CALL_CONV */



#ifdef __cplusplus
extern "C" {
#endif
    typedef void (*mclErrorCallbackFcnPtr)(void);
#ifdef __cplusplus
}
#endif


LIBMWMCLMCR_API_EXTERN_C void mclAddCanonicalPathMacro(const char* macro, const char* expansion);

LIBMWMCLMCR_API_EXTERN_C bool mclFeval(HMCRINSTANCE inst, const char* name, int nlhs, mxArray** plhs, int nrhs, mxArray** prhs);


LIBMWMCLMCR_API_EXTERN_C int mclGetMaxPathLen(void);


LIBMWMCLMCR_API_EXTERN_C bool mclmcrInitialize2(int primaryMode);


LIBMWMCLMCR_API_EXTERN_C bool mclmcrInitialize(void);



/*
 * in LCC, int32_t is defined as a long since it does not
 * contain a definition for __int32, this is MSVC specific.
 */
#ifdef _WIN32
#ifdef __LCC__
typedef long int32_t;
#else
typedef __int32 int32_t;
#endif
#endif

#define MAX_FIELD_NAME_SIZE 1024

typedef struct _wcsStackPointer
{
    CHAR16_T * hPtr;
    CHAR16_T   sPtr[MAX_FIELD_NAME_SIZE];
    int32_t buffLen;
} *pwcsStackPointer;


LIBMWMCLMCR_API_EXTERN_C void deleteWcsStackPointer_hPtr(pwcsStackPointer ptr);


LIBMWMCLMCR_API_EXTERN_C void initializeWcsStackPointer(pwcsStackPointer *ptr);


LIBMWMCLMCR_API_EXTERN_C void deleteWcsStackPointer(pwcsStackPointer ptr);


LIBMWMCLMCR_API_EXTERN_C bool allocWcsStackPointer(pwcsStackPointer *ptr, int newLen);


LIBMWMCLMCR_API_EXTERN_C int mwMbstowcs(pwcsStackPointer sp, const char *sourceString);


LIBMWMCLMCR_API_EXTERN_C void utf16_to_lcp_n_fcn(char * target, int32_t * targetSize,
                        CHAR16_T const * source, int32_t sourceSize);


LIBMWMCLMCR_API_EXTERN_C int32_t utf16_strlen_fcn(CHAR16_T const * s);


LIBMWMCLMCR_API_EXTERN_C CHAR16_T * utf16_strncpy_fcn(CHAR16_T * dst, CHAR16_T const * src, int32_t n);


LIBMWMCLMCR_API_EXTERN_C CHAR16_T * utf16_strdup_fcn(const CHAR16_T * sl);



/* PATH_MAX is the maximum number of characters that can appear in a full
 * path specification. Define it, if it isn't already defined.
 */

#if !defined(PATH_MAX) || (defined(PATH_MAX) && PATH_MAX<1024)
#undef PATH_MAX

/* _WIN32 is always defined, for both Win32 and Win64 platforms */
#ifdef _WIN32
#ifdef _MAX_PATH
#define PATH_MAX _MAX_PATH
#endif
#endif

/* Windows platforms on which _MAX_PATH is not defined, and all UNIX
 * platforms that don't define PATH_MAX. (Most UNIX platforms should
 * define PATH_MAX in <limits.h>)
 */
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#endif

LIBMWMCLMCR_API_EXTERN_C bool mclFeval(HMCRINSTANCE inst, const char* name, int nlhs, mxArray** plhs, int nrhs, mxArray** prhs);

LIBMWMCLMCR_API_EXTERN_C bool mclSetGlobal(HMCRINSTANCE inst, const char* name, mxArray* px);


LIBMWMCLMCR_API_EXTERN_C bool mclIsStandaloneMode();

 
LIBMWMCLMCR_API_EXTERN_C bool mclImpersonationFeval(HMCRINSTANCE inst, const char* name, int nlhs, mxArray** plhs, int nrhs, mxArray** prhs, void* impersonationToken);

 
LIBMWMCLMCR_API_EXTERN_C bool mclFeval(HMCRINSTANCE inst, const char* name, int nlhs, mxArray** plhs, int nrhs, mxArray** prhs);


LIBMWMCLMCR_API_EXTERN_C bool mclGetGlobal(HMCRINSTANCE inst, const char* name, mxArray** ppx);


LIBMWMCLMCR_API_EXTERN_C long mclGetID(HMCRINSTANCE inst);


LIBMWMCLMCR_API_EXTERN_C int mclMain(HMCRINSTANCE inst, int argc, const char* argv[],
            const char* name, int nlhs);

 
LIBMWMCLMCR_API_EXTERN_C bool mclMlfVFeval(HMCRINSTANCE inst, const char* name, int nargout, int fnout, int fnin, va_list ap);


typedef void (*mclEventFunctionPtr) (void * context, int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);


LIBMWMCLMCR_API_EXTERN_C bool mclRegisterEventFunction(HMCRINSTANCE inst, const char* varname, mclEventFunctionPtr fcn);


LIBMWMCLMCR_API_EXTERN_C bool mclRegisterExternalFunction(HMCRINSTANCE inst, const char* varname, mxFunctionPtr fcn);


LIBMWMCLMCR_API_EXTERN_C bool mclSetGlobal(HMCRINSTANCE inst, const char* name, mxArray* px);


LIBMWMCLMCR_API_EXTERN_C bool mclGetMCRVersion(const char **version);


LIBMWMCLMCR_API_EXTERN_C size_t mclGetActiveID(void);


typedef int (*mclOutputFcnCpp)(const char *);


LIBMWMCLMCR_API_EXTERN_C char* mclGetTempFileName(char* tempFileName);

 
LIBMWMCLMCR_API_EXTERN_C bool mclTerminateInstance(HMCRINSTANCE* inst);

LIBMWMCLMCR_API_EXTERN_C  void stopImpersonationOnMCRThread();

  
LIBMWMCLMCR_API_EXTERN_C void stopImpersonationOnMCRThread();

#endif /* mclmcr_published_api_hpp */
