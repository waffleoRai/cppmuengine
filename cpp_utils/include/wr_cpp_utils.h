#ifndef WR_CPP_UTILS_H_INCLUDED
#define WR_CPP_UTILS_H_INCLUDED

#include "wr_c_utils.h"
#include <exception>
#include <string>

#include "unicode/unistr.h"

using std::string;
using std::exception;

namespace waffleoRai_Utils{

//Unicode



//Exceptions

class WRCU_DLL_API NullPointerException:public exception
{
private:
	const char* sSource;
	const char* sReason;

public:
	NullPointerException(const char* source, const char* reason):sSource(source),sReason(reason){};
	const char* what() const throw(){return sReason;}
};

class WRCU_DLL_API IndexOutOfBoundsException :public exception
{
private:
	const char* sSource;
	const char* sReason;

public:
	IndexOutOfBoundsException(const char* source, const char* reason) :sSource(source), sReason(reason) {};
	const char* what() const throw() { return sReason; }
};

class WRCU_DLL_API UnicodeConversionException :public exception
{
private:
	const char* sSource;
	const char* sReason;
	const UErrorCode uerr;

public:
	UnicodeConversionException(const char* source, const char* reason, UErrorCode errcode) :sSource(source), sReason(reason),uerr(errcode) {};
	const char* what() const throw() { return sReason; }
	const UErrorCode getError() { return uerr; }
};

}



#endif // WR_CPP_UTILS_H_INCLUDED
