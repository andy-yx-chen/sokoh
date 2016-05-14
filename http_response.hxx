#ifndef __SPARTA_HTTP_RESPONSE__
#define __SPARTA_HTTP_RESPONSE__

class http_response{
public:
	explicit http_response(FCGX_Stream* out, const char* protocol_version = "HTTP/1.1");

private:
	typedef std::unordered_map<std::string, std::string>::iterator key_value_iterator;
	typedef std::unordered_map<std::string, std::string>::const_iterator key_value_const_iterator;

private:
	FCGX_Stream* _out;
	int _status;
	std::string _protocol_version;
	std::string _reason_phrase;
	std::unordered_map<std::string, std::string> _headers;
	std::list<std::string> _content;

private:
	void crlf();

public:
	void set_header(const std::string& name, const std::string& value){
		key_value_iterator itor = this->_headers.find(name);
		if (itor != this->_headers.end()){
			itor->second = value;
		}
		else{
			this->_headers.insert(std::move(make_pair(name, value)));
		}
	}

	void set_status(int status, const std::string& reason){
		this->_status = status;
		this->_reason_phrase = reason;
	}

	void write(const std::string& line);

	void done();
};
#endif
