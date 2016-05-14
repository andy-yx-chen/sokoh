#ifndef __SPARTA_HTTP_FILE__
#define __SPARTA_HTTP_FILE__

class http_file{
private:
	std::string _file_name;
	std::string _tmp_file;
	std::string _content_type;

public:
	http_file(const std::string& file_name, const std::string& tmp_file, const std::string& content_type)
		: _file_name(file_name), _tmp_file(tmp_file), _content_type(content_type){

	}

	~http_file();

private:
	http_file(const http_file&);
	http_file& operator = (const http_file&);

public:
	const std::string& name() const{
		return this->_file_name;
	}

	const std::string& tmp_path() const{
		return this->_tmp_file;
	}

	const std::string& content_type() const{
		return this->_content_type;
	}

	std::string content() const;

	void move_to(const std::string& target_path) const;
};

#endif