#ifndef SYSTEM_EXCEPTION_HPP
#define SYSTEM_EXCEPTION_HPP

namespace System
{
    // Forward declaration
    class String;

    /// @brief Base class for all exceptions in the System namespace.
    ///
    /// Provides a standard exception hierarchy similar to .NET Framework exceptions.
    /// All exceptions store a message describing the error condition.
    class Exception
    {
    protected:
        char *_message;
        int _messageLength;

        /// @brief Copies a message string into the internal buffer.
        /// @param msg The message string to copy. Can be nullptr.
        void _copyMessage(const char *msg);

        /// @brief Frees the internal message buffer if allocated.
        void _freeMessage();

    public:
        /// @brief Constructs an Exception with a default message.
        Exception();

        /// @brief Constructs an Exception with the specified message.
        /// @param message A description of the error condition.
        explicit Exception(const char *message);

        /// @brief Copy constructor.
        /// @param other The Exception to copy from.
        Exception(const Exception &other);

        /// @brief Copy assignment operator.
        /// @param other The Exception to assign from.
        /// @return A reference to this Exception.
        Exception &operator=(const Exception &other);

        /// @brief Virtual destructor to ensure proper cleanup of derived classes.
        virtual ~Exception();

        /// @brief Gets the message that describes the current exception.
        /// @return A pointer to the error message string.
        [[nodiscard]] virtual const char *Message() const;

        /// @brief Gets the message that describes the current exception (C++ standard compatibility).
        /// @return A pointer to the error message string.
        [[nodiscard]] virtual const char *what() const noexcept;
    };

    /// @brief The exception that is thrown when one of the arguments provided to a method is not valid.
    ///
    /// This exception includes the name of the parameter that caused the exception.
    class ArgumentException : public Exception
    {
    protected:
        char *_paramName;
        int _paramNameLength;

        /// @brief Copies a parameter name string into the internal buffer.
        /// @param name The parameter name to copy. Can be nullptr.
        void _copyParamName(const char *name);

        /// @brief Frees the internal parameter name buffer if allocated.
        void _freeParamName();

    public:
        /// @brief Constructs an ArgumentException with a message and optional parameter name.
        /// @param message A description of the error condition.
        /// @param paramName The name of the parameter that caused the exception. Can be nullptr.
        explicit ArgumentException(const char *message, const char *paramName = nullptr);

        /// @brief Copy constructor.
        /// @param other The ArgumentException to copy from.
        ArgumentException(const ArgumentException &other);

        /// @brief Copy assignment operator.
        /// @param other The ArgumentException to assign from.
        /// @return A reference to this ArgumentException.
        ArgumentException &operator=(const ArgumentException &other);

        /// @brief Destructor that frees the parameter name buffer.
        ~ArgumentException() override;

        /// @brief Gets the name of the parameter that caused the exception.
        /// @return A pointer to the parameter name string, or nullptr if not set.
        [[nodiscard]] const char *ParamName() const;
    };

    /// @brief The exception that is thrown when a null reference is passed to a method that does not accept it as a valid argument.
    class ArgumentNullException : public ArgumentException
    {
    public:
        /// @brief Constructs an ArgumentNullException with the name of the null parameter.
        /// @param paramName The name of the parameter that was null.
        explicit ArgumentNullException(const char *paramName);
    };

    /// @brief The exception that is thrown when the value of an argument is outside the allowable range.
    class ArgumentOutOfRangeException : public ArgumentException
    {
    public:
        /// @brief Constructs an ArgumentOutOfRangeException with parameter name and optional message.
        /// @param paramName The name of the parameter that caused the exception.
        /// @param message A description of the error condition. Can be nullptr for default message.
        explicit ArgumentOutOfRangeException(const char *paramName, const char *message = nullptr);
    };

    /// @brief The exception that is thrown when a method call is invalid for the object's current state.
    class InvalidOperationException : public Exception
    {
    public:
        /// @brief Constructs an InvalidOperationException with the specified message.
        /// @param message A description of why the operation is invalid.
        explicit InvalidOperationException(const char *message);
    };

    /// @brief The exception that is thrown when an attempt is made to access an element with an index that is outside the bounds of the array.
    class IndexOutOfRangeException : public Exception
    {
    public:
        /// @brief Constructs an IndexOutOfRangeException with a default message.
        IndexOutOfRangeException();

        /// @brief Constructs an IndexOutOfRangeException with the specified message.
        /// @param message A description of the error condition.
        explicit IndexOutOfRangeException(const char *message);
    };

    /// @brief The exception that is thrown when there is an attempt to dereference a null object reference.
    class NullReferenceException : public Exception
    {
    public:
        /// @brief Constructs a NullReferenceException with a default message.
        NullReferenceException();

        /// @brief Constructs a NullReferenceException with the specified message.
        /// @param message A description of the error condition.
        explicit NullReferenceException(const char *message);
    };

    /// @brief The exception that is thrown when the format of an argument is invalid.
    class FormatException : public Exception
    {
    public:
        /// @brief Constructs a FormatException with a default message.
        FormatException();

        /// @brief Constructs a FormatException with the specified message.
        /// @param message A description of the format error.
        explicit FormatException(const char *message);
    };

    /// @brief The exception that is thrown when an arithmetic operation results in an overflow.
    class OverflowException : public Exception
    {
    public:
        /// @brief Constructs an OverflowException with a default message.
        OverflowException();

        /// @brief Constructs an OverflowException with the specified message.
        /// @param message A description of the overflow condition.
        explicit OverflowException(const char *message);
    };

    /// @brief The exception that is thrown when an I/O error occurs.
    ///
    /// This is the base class for I/O-related exceptions such as FileNotFoundException.
    class IOException : public Exception
    {
    public:
        /// @brief Constructs an IOException with a default message.
        IOException();

        /// @brief Constructs an IOException with the specified message.
        /// @param message A description of the I/O error.
        explicit IOException(const char *message);
    };

    /// @brief The exception that is thrown when an attempt to access a file that does not exist fails.
    class FileNotFoundException : public IOException
    {
    public:
        /// @brief Constructs a FileNotFoundException with a default message.
        FileNotFoundException();

        /// @brief Constructs a FileNotFoundException for the specified file path.
        /// @param path The path of the file that was not found.
        explicit FileNotFoundException(const char *path);
    };

    /// @brief The exception that is thrown when a data stream is in an invalid format.
    ///
    /// This exception is typically thrown when reading files with corrupted or unexpected data.
    class InvalidDataException : public Exception
    {
    public:
        /// @brief Constructs an InvalidDataException with a default message.
        InvalidDataException();

        /// @brief Constructs an InvalidDataException with the specified message.
        /// @param message A description of the data format error.
        explicit InvalidDataException(const char *message);
    };
} // namespace System

#endif // SYSTEM_EXCEPTION_HPP
