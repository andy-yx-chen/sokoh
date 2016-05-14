#ifndef __SPARTA_HTTP_REQUEST__
#define __SPARTA_HTTP_REQUEST__

class http_request
{
public:
	enum request_method
	{
		http_get, http_post, http_delete, http_head, http_put, http_other
	};

public:
	static const std::string empty_string;

private:
	typedef std::unordered_map<std::string, std::shared_ptr<http_file> >::const_iterator file_iterator;
	typedef std::unordered_map<std::string, std::string>::const_iterator key_value_iterator;

private:
	FCGX_Request* _request;
	std::unordered_map<std::string, std::shared_ptr<http_file> > _upload_files;
	std::unordered_map<std::string, std::string> _request_data;
	std::unordered_map<std::string, std::string> _request_attribues;
	std::unordered_map<std::string, std::string> _server_variables;
	request_method _request_method;
	bool _has_error;
	
public:
	explicit http_request(FCGX_Request* request);
	~http_request();

private:
	http_request(const http_request&);
	http_request& operator = (const http_request&);

	const std::string& find_variable(const char* var_name) const{
		key_value_iterator result = this->_server_variables.find(var_name);
		if (result != this->_server_variables.end()){
			return result->second;
		}

		return empty_string;
	}

	std::pair<std::string, std::string> split_string(const char* str, size_t len, char splitter);

	void parse_post_data(const char* data, size_t size);
	void parse_query_string(const char* query_string);
	void parse_env_item(const char* env_string);
	void parse_form_item(const char* form_data, size_t size);
	void save_uploaded_file(const std::string& name, const std::string& file_name, const char* data , size_t size, const std::string& content_type);

public:
	request_method method() const {
		return this->_request_method;
	}

	const std::string& query_string() const{
		return this->find_variable(QUERY_STRING);
	}

	const std::string& path_info() const{
		return this->find_variable(PATH_INFO);
	}

	const std::string& user_agent() const{
		return this->find_variable(HTTP_USER_AGENT);
	}

	const std::string& remote_addr() const{
		return this->find_variable(REMOTE_ADDR);
	}

	const std::string& accept_lang() const{
		return this->find_variable(HTTP_ACCEPT_LANGUAGE);
	}

	const std::string& host() const{
		return this->find_variable(HTTP_HOST);
	}

	const std::string& get_attribute(const std::string& name) const;

	void set_attribute(const std::string& name, const std::string& value);

	const std::string& header(const std::string& name) const;

	std::shared_ptr<http_response> response(){
		return std::shared_ptr<http_response>(new http_response(this->_request->out));
	}

	std::shared_ptr<http_file> get_file(const std::string& name) const{
		file_iterator results = this->_upload_files.find(name);
		if (results != this->_upload_files.end()){
			return results->second;
		}

		return std::shared_ptr<http_file>(nullptr);
	}

	const std::string& request_data(const std::string& name) const;

	std::string physical_path(const std::string& file) const;

	void dump_all(http_response* response) const;
};

#endif
