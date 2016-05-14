#ifndef __SPARTA_EXCEPTION_HXX__
#define __SPARTA_EXCEPTION_HXX__

class sparta_exception : public std::exception{
private:
  std::string _message;

public:
  explicit sparta_exception(const std::string& message) throw()
    : _message(message){
  }

  sparta_exception(const sparta_exception& exception) throw()
    : std::exception(exception), _message(exception._message){
  }

  sparta_exception& operator= (const sparta_exception& exception) throw(){
    _message = exception._message;
    std::exception::operator=(exception);
    return *this;
  }

  virtual const char* what() const throw(){
    return _message.c_str();
  }
};
#endif
