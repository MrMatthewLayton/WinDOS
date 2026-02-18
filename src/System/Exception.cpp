#include "Exception.hpp"
#include <cstring>

namespace System
{
    // ============================================================================
    // Exception
    // ============================================================================

    void Exception::_copyMessage(const char *msg)
    {
        if (msg)
        {
            _messageLength = std::strlen(msg);
            _message = new char[_messageLength + 1];
            std::strcpy(_message, msg);
        }
        else
        {
            _message = nullptr;
            _messageLength = 0;
        }
    }

    void Exception::_freeMessage()
    {
        delete[] _message;
        _message = nullptr;
        _messageLength = 0;
    }

    Exception::Exception()
        : _message(nullptr), _messageLength(0)
    {
        _copyMessage("An exception occurred.");
    }

    Exception::Exception(const char *message)
        : _message(nullptr), _messageLength(0)
    {
        _copyMessage(message);
    }

    Exception::Exception(const Exception &other)
        : _message(nullptr), _messageLength(0)
    {
        _copyMessage(other._message);
    }

    Exception &Exception::operator=(const Exception &other)
    {
        if (this != &other)
        {
            _freeMessage();
            _copyMessage(other._message);
        }
        return *this;
    }

    Exception::~Exception()
    {
        _freeMessage();
    }

    const char *Exception::Message() const
    {
        return _message ? _message : "";
    }

    const char *Exception::what() const noexcept
    {
        return _message ? _message : "";
    }

    // ============================================================================
    // ArgumentException
    // ============================================================================

    void ArgumentException::_copyParamName(const char *name)
    {
        if (name)
        {
            _paramNameLength = std::strlen(name);
            _paramName = new char[_paramNameLength + 1];
            std::strcpy(_paramName, name);
        } else
        {
            _paramName = nullptr;
            _paramNameLength = 0;
        }
    }

    void ArgumentException::_freeParamName()
    {
        delete[] _paramName;
        _paramName = nullptr;
        _paramNameLength = 0;
    }

    ArgumentException::ArgumentException(const char *message, const char *paramName)
        : Exception(message), _paramName(nullptr), _paramNameLength(0)
    {
        _copyParamName(paramName);
    }

    ArgumentException::ArgumentException(const ArgumentException &other)
        : Exception(other), _paramName(nullptr), _paramNameLength(0)
    {
        _copyParamName(other._paramName);
    }

    ArgumentException &ArgumentException::operator=(const ArgumentException &other)
    {
        if (this != &other)
        {
            Exception::operator=(other);
            _freeParamName();
            _copyParamName(other._paramName);
        }
        return *this;
    }

    ArgumentException::~ArgumentException()
    {
        _freeParamName();
    }

    const char *ArgumentException::ParamName() const
    {
        return _paramName ? _paramName : "";
    }

    // ============================================================================
    // ArgumentNullException
    // ============================================================================

    ArgumentNullException::ArgumentNullException(const char *paramName)
        : ArgumentException("Value cannot be null.", paramName)
    {
    }

    // ============================================================================
    // ArgumentOutOfRangeException
    // ============================================================================

    ArgumentOutOfRangeException::ArgumentOutOfRangeException(const char *paramName, const char *message)
        : ArgumentException(message ? message : "Specified argument was out of the range of valid values.", paramName)
    {
    }

    // ============================================================================
    // InvalidOperationException
    // ============================================================================

    InvalidOperationException::InvalidOperationException(const char *message)
        : Exception(message)
    {
    }

    // ============================================================================
    // IndexOutOfRangeException
    // ============================================================================

    IndexOutOfRangeException::IndexOutOfRangeException()
        : Exception("Index was outside the bounds of the array.")
    {
    }

    IndexOutOfRangeException::IndexOutOfRangeException(const char *message)
        : Exception(message)
    {
    }

    // ============================================================================
    // NullReferenceException
    // ============================================================================

    NullReferenceException::NullReferenceException()
        : Exception("Object reference not set to an instance of an object.")
    {
    }

    NullReferenceException::NullReferenceException(const char *message)
        : Exception(message)
    {
    }

    // ============================================================================
    // FormatException
    // ============================================================================

    FormatException::FormatException()
        : Exception("Input string was not in a correct format.")
    {
    }

    FormatException::FormatException(const char *message)
        : Exception(message)
    {
    }

    // ============================================================================
    // OverflowException
    // ============================================================================

    OverflowException::OverflowException()
        : Exception("Arithmetic operation resulted in an overflow.")
    {
    }

    OverflowException::OverflowException(const char *message)
        : Exception(message)
    {
    }

    // ============================================================================
    // IOException
    // ============================================================================

    IOException::IOException()
        : Exception("An I/O error occurred.")
    {
    }

    IOException::IOException(const char *message)
        : Exception(message)
    {
    }

    // ============================================================================
    // FileNotFoundException
    // ============================================================================

    FileNotFoundException::FileNotFoundException()
        : IOException("The specified file was not found.")
    {
    }

    FileNotFoundException::FileNotFoundException(const char *path)
        : IOException(path ? path : "The specified file was not found.")
    {
    }

    // ============================================================================
    // InvalidDataException
    // ============================================================================

    InvalidDataException::InvalidDataException()
        : Exception("The data is invalid.")
    {
    }

    InvalidDataException::InvalidDataException(const char *message)
        : Exception(message)
    {
    }
} // namespace System
