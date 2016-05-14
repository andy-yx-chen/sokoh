#ifndef __SPARTA_STRING_TOKENIZER__
#define __SPARTA_STRING_TOKENIZER__

class string_tokenizer{
public:
	string_tokenizer(const std::string& source, const char* delimiter)
		: _source(source), _delimiter(delimiter), _current_pos(0){

	}

private:
	const std::string& _source;
	const char* _delimiter;
	size_t _current_pos;

public:
	bool has_more() const;

	const char* next(size_t& size);
};
#endif