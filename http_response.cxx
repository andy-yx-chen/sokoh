#include "sokoh.hxx"

http_response::http_response(FCGX_Stream* out, const char* protocol_version)
  : _out(out), _status(200), _protocol_version(protocol_version), _reason_phrase(), _headers(), _content(){
}

void http_response::crlf(){
	FCGX_PutStr("\r\n", 2, this->_out);
}

void http_response::done(){
	// Status line
	size_t status_line_size = this->_protocol_version.length() + 10 + this->_reason_phrase.length();
	std::shared_ptr<char> status_line(new char[status_line_size]);
	sprintf(status_line.get(), "%d %s", this->_status, this->_reason_phrase.c_str());
	FCGX_PutStr("Status:", 7, this->_out);
	FCGX_PutStr(status_line.get(), strlen(status_line.get()), this->_out);
	this->crlf();

	// Headers
	for (key_value_const_iterator it = this->_headers.begin(); it != this->_headers.end(); ++it){
		FCGX_PutStr(it->first.c_str(), it->first.length(), this->_out);
		FCGX_PutStr(": ", 2, this->_out);
		FCGX_PutStr(it->second.c_str(), it->second.length(), this->_out);
		this->crlf();
	}

	this->crlf();

	// Content
	for (std::list<std::string>::const_iterator it = this->_content.begin(); it != this->_content.end(); ++it){
		FCGX_PutStr(it->c_str(), it->length(), this->_out);
	}
}

void http_response::write(const std::string& line){
	this->_content.push_back(line);
}
