/**
 * @file src/gbException.cpp
 *
 * ghulbus Library - The Ghulbus Library
 *
 * @brief gbException Implementation
 *
 * @version 1.1
 * @author Andreas Weis
 *
 */
#include "../include/gbException.hpp"

namespace Ghulbus {
	gbException::gbException() throw() 
		: m_errorcode(gbException::GB_FAILED), m_errorstring(NULL)
	{
		;
	}
	gbException::gbException(GBERROR e) throw()
		: m_errorcode(e), m_errorstring(NULL) 
	{
		;
	}
	gbException::gbException(GBERROR e, const char* str) throw()
		: m_errorcode(e), m_errorstring(str) 
	{
		;
	}
	gbException::gbException(gbException const& e) throw()
		: m_errorcode(e.m_errorcode), m_errorstring(e.m_errorstring)
	{
		;
	}
	gbException::~gbException() throw()
	{
		;
	}
	char const* gbException::GetErrorString() const throw() {
		return (m_errorstring?m_errorstring:""); 
	}
	gbException::GBERROR gbException::GetErrorCode() const throw () {
		return m_errorcode;
	}

	gbException& gbException::operator=(gbException const& e) throw() {
		if(this != &e) {
			this->m_errorcode = e.m_errorcode;
			this->m_errorstring = e.m_errorstring;
		}
		return (*this);
	}

	char const* gbException::what() const throw()
	{
		char const* ret;
		switch(m_errorcode) {
			case gbException::GB_OK:
				ret = "GB_OK - No errors.";
				break;
			case gbException::GB_FAILED:
				ret = "GB_FAILED - The operation failed.";
				break;
			case gbException::GB_OUTOFMEMORY:
				ret = "GB_OUTOFMEMORY - Not enough memory.";
				break;
			case gbException::GB_ILLEGALPARAMETER:
				ret = "GB_ILLEGALPARAMETER - A parameter passed to a function was illegal.";
				break;
			case gbException::GB_INVALIDCONTEXT:
				ret = "GB_INVALIDCONTEXT - A function was called in a wrong context.";
				break;
			case gbException::GB_NOTIMPLEMENTED:
				ret = "GB_NOTIMPLEMENTED - The requested functionality is not yet implemented.";
				break;
			default:
				ret = "Unknown Error.";
				break;
		}
		return ret;
	}

	::std::ostream& operator<<(::std::ostream& os, ::Ghulbus::gbException const& e) {
		return os << e.GetErrorCode() << " " << e.what();
	}
};
