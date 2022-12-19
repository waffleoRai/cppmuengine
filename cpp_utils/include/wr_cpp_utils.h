/***************************************************************************//**
@file wr_cpp_utils.h
@brief C++ general use macros, functions, and types.

WaffleoRaiCUtils (WRCU):
A set of macros and functions for common use. Includes some handling for Unicode
strings, based upon the [ICU] library. This header represents the C++ extension.

Includes:
+ stdint
+ string
+ uchar
+ stdlib
+ exception
+ [ICU] unicode/ucnv.h
+ [ICU] unicode/ustr.h

[ICU]: https://unicode-org.github.io/icu/ "ICU"

@author Blythe Hospelhorn
@version 1.0.0
@since December 4, 2022

 ******************************************************************************/

#ifndef WR_CPP_UTILS_H_INCLUDED
#define WR_CPP_UTILS_H_INCLUDED

#include "wr_c_utils.h"
#include <exception>
#include <string>

#include "unicode/unistr.h"

using std::string;
using std::exception;

/***************************************************************************//**
@namespace waffleoRai_Utils
A set of general-use C/C++ utility types, macros, methods, and functions.
 ******************************************************************************/
namespace waffleoRai_Utils{

//Unicode

//Exceptions

/***************************************************************************//**
An `exception` subclass to throw when an attempt to access data using a NULL
pointer is caught.
 ******************************************************************************/
class WRCU_DLL_API NullPointerException:public exception
{
private:
	const char* sSource;
	const char* sReason;

public:

	/**
	Construct a `NullPointerException` with information regarding the source of the throw and
	reason for throwing.
	@param source String representing the source of the throw. For example, the full name of a method (eg. `mynamespace::myclass::mymethod`).
	May be left `NULL`.
	@param reason String describing the reason for the exception. Is returned by superclass function `what()`. May be left `NULL`.
	*/
	NullPointerException(const char* source, const char* reason):sSource(source),sReason(reason){};

	/**
	Get a string describing the source of the exception. This may be the full name of a method or function, or a file/line number.
	@return Source description string. May be `NULL`.
	*/
	const char* getSourceDescription() const { return sSource; }

	/**
	* `exception` superclass **virtual override**

	From `std::exception` documentation at [cplusplus.com] :

	Returns a null terminated character sequence that may be used to identify the exception. 
	The particular representation pointed by the returned value is implementation-defined. 
	As a virtual function, derived classes may redefine this function so that specific values are returned.
	@return Reason description string. May be `NULL`.
	[cplusplus.com]: https://cplusplus.com/reference/exception/exception/what/ "cplusplus.com"
	*/
	const char* what() const throw() override{return sReason;}
};

/***************************************************************************//**
An `exception` subclass to throw when an attempt to access data outside the bounds
of a known array or indexed data structure is caught.
 ******************************************************************************/
class WRCU_DLL_API IndexOutOfBoundsException :public exception
{
private:
	const char* sSource;
	const char* sReason;

public:
	/**
	Construct a `IndexOutOfBoundsException` with information regarding the source of the throw and
	reason for throwing.
	@param source String representing the source of the throw. For example, the full name of a method (eg. `mynamespace::myclass::mymethod`).
	May be left `NULL`.
	@param reason String describing the reason for the exception. Is returned by superclass function `what()`. May be left `NULL`.
	*/
	IndexOutOfBoundsException(const char* source, const char* reason) :sSource(source), sReason(reason) {};

	/**
	Get a string describing the source of the exception. This may be the full name of a method or function, or a file/line number.
	@return Source description string. May be `NULL`.
	*/
	const char* getSourceDescription() const { return sSource; }

	/**
	* `exception` superclass **virtual override**

	From `std::exception` documentation at [cplusplus.com] :

	Returns a null terminated character sequence that may be used to identify the exception.
	The particular representation pointed by the returned value is implementation-defined.
	As a virtual function, derived classes may redefine this function so that specific values are returned.
	@return Reason description string. May be `NULL`.
	[cplusplus.com]: https://cplusplus.com/reference/exception/exception/what/ "cplusplus.com"
	*/
	const char* what() const throw() override { return sReason; }
};

/***************************************************************************//**
An `exception` subclass to throw when an ICU Unicode conversion fails.
 ******************************************************************************/
class WRCU_DLL_API UnicodeConversionException :public exception
{
private:
	const char* sSource;
	const char* sReason;
	const UErrorCode uerr;

public:
	/**
	Construct a `UnicodeConversionException` with information regarding the source of the throw, reason for throwing,
	and specific ICU library error.
	@param source String representing the source of the throw. For example, the full name of a method (eg. `mynamespace::myclass::mymethod`).
	May be left `NULL`.
	@param reason String describing the reason for the exception. Is returned by superclass function `what()`. May be left `NULL`.
	@param errcode The `UErrorCode` returned by the failed operation.
	*/
	UnicodeConversionException(const char* source, const char* reason, UErrorCode errcode) :sSource(source), sReason(reason),uerr(errcode) {};

	/**
	Get a string describing the source of the exception. This may be the full name of a method or function, or a file/line number.
	@return Source description string. May be `NULL`.
	*/
	const char* getSourceDescription() const { return sSource; }

	/**
	* `exception` superclass **virtual override**

	From `std::exception` documentation at [cplusplus.com] :

	Returns a null terminated character sequence that may be used to identify the exception.
	The particular representation pointed by the returned value is implementation-defined.
	As a virtual function, derived classes may redefine this function so that specific values are returned.
	@return Reason description string. May be `NULL`.
	[cplusplus.com]: https://cplusplus.com/reference/exception/exception/what/ "cplusplus.com"
	*/
	const char* what() const throw() override { return sReason; }

	/**
	Get the `UErrorCode` associated with this exception, if present.
	@return `UErrorCode` returned by failed operation. Defaults to `U_ZERO_ERROR`, but that does not necessarily mean there was no other error.
	*/
	const UErrorCode getError() const { return uerr; }
};

}



#endif // WR_CPP_UTILS_H_INCLUDED
