#ifndef SYSTEM_EXCEPTION_HPP
#define SYSTEM_EXCEPTION_HPP

namespace System {

// Forward declaration
class String;

class Exception {
protected:
    char* _message;
    int _messageLength;

    void _copyMessage(const char* msg);
    void _freeMessage();

public:
    Exception();
    explicit Exception(const char* message);
    Exception(const Exception& other);
    Exception& operator=(const Exception& other);
    virtual ~Exception();

    virtual const char* Message() const;
    virtual const char* what() const noexcept;
};

class ArgumentException : public Exception {
protected:
    char* _paramName;
    int _paramNameLength;

    void _copyParamName(const char* name);
    void _freeParamName();

public:
    ArgumentException(const char* message, const char* paramName = nullptr);
    ArgumentException(const ArgumentException& other);
    ArgumentException& operator=(const ArgumentException& other);
    ~ArgumentException() override;

    const char* ParamName() const;
};

class ArgumentNullException : public ArgumentException {
public:
    explicit ArgumentNullException(const char* paramName);
};

class ArgumentOutOfRangeException : public ArgumentException {
public:
    ArgumentOutOfRangeException(const char* paramName, const char* message = nullptr);
};

class InvalidOperationException : public Exception {
public:
    explicit InvalidOperationException(const char* message);
};

class IndexOutOfRangeException : public Exception {
public:
    IndexOutOfRangeException();
    explicit IndexOutOfRangeException(const char* message);
};

class NullReferenceException : public Exception {
public:
    NullReferenceException();
    explicit NullReferenceException(const char* message);
};

class FormatException : public Exception {
public:
    FormatException();
    explicit FormatException(const char* message);
};

class OverflowException : public Exception {
public:
    OverflowException();
    explicit OverflowException(const char* message);
};

// I/O Exceptions (for file operations)
class IOException : public Exception {
public:
    IOException();
    explicit IOException(const char* message);
};

class FileNotFoundException : public IOException {
public:
    FileNotFoundException();
    explicit FileNotFoundException(const char* path);
};

class InvalidDataException : public Exception {
public:
    InvalidDataException();
    explicit InvalidDataException(const char* message);
};

} // namespace System

#endif // SYSTEM_EXCEPTION_HPP
