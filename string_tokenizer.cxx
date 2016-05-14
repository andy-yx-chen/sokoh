#include "sokoh.hxx"

bool string_tokenizer::has_more() const{
	return this->_current_pos < this->_source.length();
}

const char* string_tokenizer::next(size_t& size){

	if (!this->has_more()){
		size = 0;
		return nullptr;
	}

	size_t pos = this->_source.find(this->_delimiter, this->_current_pos);
	if (pos == std::string::npos){
		pos = this->_source.length();
	}

	const char* token = this->_source.c_str() + this->_current_pos;
	size = pos - this->_current_pos;
	this->_current_pos = pos + strlen(this->_delimiter);
	return token;
}
