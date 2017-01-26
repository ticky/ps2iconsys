/**
 * @file include/gbException.hpp
 *
 * ghulbus Library - The Ghulbus Library
 *
 * @brief gbException Exceptions
 *
 * @version 1.1
 * @author Andreas Weis
 *
 */
#ifndef _GHULBUSEXCEPTION_HPP_INCLUDE_GUARD_
#define _GHULBUSEXCEPTION_HPP_INCLUDE_GUARD_

#include <iostream>
#include <exception>

namespace Ghulbus {
	/** The Exceptions thrown by the objects of the ghulbus library
	 */
	class gbException: public ::std::exception {
	public:
		typedef enum {
			GB_OK=0,						///< No errors
			GB_FAILED,						///< General failure
			GB_OUTOFMEMORY,					///< Not enough memory
			GB_ILLEGALPARAMETER,			///< Illegal function parameter
			GB_INVALIDCONTEXT,				///< Illegal function context
			GB_NOTIMPLEMENTED,				///< Feature not yet implemented
		} GBERROR;
	private:
		GBERROR m_errorcode;				///< Errorcode of the exception
		const char* m_errorstring;			///< User supplied string describing the error (optional)
	public:
		/** Default constructor
		 * Sets errorcode to GB_FAILED and errorstring to NULL
		 */
		gbException() throw();
		/** Constructor
		 * @param[in] e Errorcode
		 */
		gbException(GBERROR e) throw();
		/** Constructor
		 * @param[in] e Errorcode
		 * @param[in] str C-style string describing the error
		 */
		gbException(GBERROR e, const char* str) throw();
		/** Copy constructor
		 */
		gbException(gbException const& e) throw();
		/** Destructor
		 */
		virtual ~gbException() throw();
		/** Get the errorstring if supplied; returns empty string otherwise
		 * @return A reference to m_errorstring
		 */
		char const* GetErrorString() const throw();
		/** Get the errorcode of this exception
		 * @return A copy of m_errorcode
		 */
		GBERROR GetErrorCode() const throw ();
		/** Copy assignment
		 */
		gbException& operator=(gbException const& e) throw();
		/** Get a message describing the nature of the exception
		 * @return A C-style string representation of m_errorcode
		 */
		virtual char const* what() const throw();
	};

	/** Stream output for gbException
	 */
	::std::ostream& operator<<(::std::ostream& os, ::Ghulbus::gbException const& e);
};

#endif
