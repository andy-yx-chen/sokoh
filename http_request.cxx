#include "sokoh.hxx"

const std::string http_request::empty_string("");

std::string http_request::physical_path(const std::string& file) const{
	if (file.length() > 0){
	  std::string result(this->find_variable("DOCUMENT_ROOT"));
		if (file[0] != '/'){
          		result.push_back('/');
		}

		result.append(file);
		return result;
	}

	return empty_string;
}

std::pair<std::string, std::string> http_request::split_string(const char* str, size_t len, const char splitter){
	size_t index_of_splitter = std::string::npos;
	for (size_t i = 0; i < len; ++i){
		if (str[i] == splitter){
			index_of_splitter = i;
			break;
		}
	}

	if (index_of_splitter != std::string::npos){
		return std::make_pair(std::string(str, str + index_of_splitter), std::string(str + index_of_splitter + 1, str + len));
	}
	else{
		return std::make_pair(std::string(), std::string(str));
	}
}

http_request::http_request(FCGX_Request* request) 
  : _request(request), _upload_files(), _request_data(), _request_attribues(), _server_variables(), _request_method(http_get), _has_error(false){
	for (char** envp = request->envp; *envp; ++envp){
		this->parse_env_item(*envp);
	}

	const std::string& method = this->find_variable(REQUEST_METHOD);
	if (strcasecmp(method.c_str(), "GET") == 0){
		this->_request_method = http_get;
	}
	else if (strcasecmp(method.c_str(), "POST") == 0){
		this->_request_method = http_post;
	}
	else if (strcasecmp(method.c_str(), "PUT") == 0){
		this->_request_method = http_put;
	}
	else if (strcasecmp(method.c_str(), "DELETE") == 0){
		this->_request_method = http_delete;
	}
	else if (strcasecmp(method.c_str(), "HEAD") == 0){
		this->_request_method = http_head;
	}
	else{
		this->_request_method = http_other;
	}

	this->parse_query_string(this->find_variable(QUERY_STRING).c_str());
	int content_len = atoi(this->find_variable(CONTENT_LENGTH).c_str());

	if (content_len > 0){
		std::string post_data(content_len, '\0');
		this->_has_error = FCGX_GetStr((char*)post_data.c_str(), post_data.size(), request->in) < 0;
		if (!this->_has_error){
			this->parse_post_data(post_data.data(), post_data.size());
		}
	}
}

const std::string& http_request::get_attribute(const std::string& name) const{
	key_value_iterator result = this->_request_attribues.find(name);
	if (result != this->_request_attribues.end()){
		return result->second;
	}

	return empty_string;
}

void http_request::set_attribute(const std::string& name, const std::string& value){
	key_value_iterator result = this->_request_attribues.find(name);
	if (result == this->_request_attribues.end()){
		this->_request_attribues.insert(std::make_pair(name, value));
	}
}

const std::string& http_request::header(const std::string& name) const{
	std::string header_name("HTTP_");
	for (size_t i = 0; i < name.length(); ++i){
		if (name[i] == '-'){
			header_name.push_back('_');
		}
		else{
			header_name.push_back(toupper(name[i]));
		}
	}

	return this->find_variable(header_name.c_str());
}

http_request::~http_request(){
	if (this->_request != nullptr){
		FCGX_Finish_r(this->_request);
	}
}

void http_request::parse_env_item(const char* env_item){
	std::pair<std::string, std::string> item(move(this->split_string(env_item, strlen(env_item), '=')));
	
	if (item.first.length() > 0){
		this->_server_variables.insert(item);
	}
}

const std::string& http_request::request_data(const std::string& name) const{
	key_value_iterator results = this->_request_data.find(name);
	if (results != this->_request_data.end()){
		return results->second;
	}

	return empty_string;
}

void http_request::parse_query_string(const char* query_string){
	std::string clone_string(query_string);
	string_tokenizer tokenizer(clone_string, "&");
	while (tokenizer.has_more()){
		size_t item_size(0);
		const char* next_item = tokenizer.next(item_size);
		std::pair<std::string, std::string> item(move(this->split_string(next_item, item_size, '=')));

		if (item.first.length() > 0){
			std::string decoded_key(std::move(uri_decode(item.first)));
			std::string decoded_value(std::move(uri_decode(item.second)));
			this->_request_data.insert(std::move(std::make_pair(decoded_key, decoded_value)));
		}
	}
}

void http_request::parse_post_data(const char* post_data, size_t size){
	std::string content_type = this->find_variable(CONTENT_TYPE);
	if (content_type == empty_string){
		return;
	}

	std::pair<std::string, std::string> results(move(this->split_string(content_type.c_str(), content_type.length(), ';')));
	if (results.first.length() == 0 && strcasecmp(content_type.c_str(), "application/x-www-form-urlencoded") == 0){
		std::string query_string(post_data, post_data + size);
		this->parse_query_string(query_string.c_str());
	}
	else if (results.first.length() != 0 && strcasecmp(results.first.c_str(), "multipart/form-data") == 0){
		// deal with the boundary and the content
		std::pair<std::string, std::string> boundary(move(this->split_string(results.second.c_str(), results.second.length(), '=')));
		if (boundary.first.length() > 0){
			std::string boundary_string("--");
			boundary_string.append(boundary.second);
			std::string post_buffer(post_data, post_data + size);
			string_tokenizer tokenizer(post_buffer, boundary_string.c_str());
			while (tokenizer.has_more()){
				size_t data_size(0);
				const char* data = tokenizer.next(data_size);
				if (data_size > 0){
					this->parse_form_item(data, data_size);
				}
			}
		}
	}
	else{
		this->save_uploaded_file(__REQUEST_FILE__, __REQUEST_FILE__, post_data, size, results.first);
	}
}

void http_request::save_uploaded_file(const std::string& name, const std::string& file_name, const char* data, size_t size, const std::string& content_type){
  std::string temp_path = this->physical_path("/tmp/SPARXXXXXX");
  int temp_file = mkstemp((char*)temp_path.c_str());
	if (temp_file != -1){
		size_t bytes_to_write = size;
		size_t bytes_written(0);
		while (bytes_written < bytes_to_write){
			ssize_t bytes_written_this_round(0);
			if (0 >= (bytes_written_this_round = write(temp_file, (const void*)(data + bytes_written), bytes_to_write - bytes_written))){
				break;
			}

			bytes_written += bytes_written_this_round;
		}

		this->_upload_files.insert(std::move(std::make_pair(name, std::move(std::shared_ptr<http_file>(new http_file(file_name, temp_path, content_type))))));
		close(temp_file);
	}
}

void http_request::parse_form_item(const char* data_buf, size_t size){
	std::string data_buffer(data_buf, data_buf + size);
	string_tokenizer tokenizer(data_buffer, "\r\n\r\n");
	size_t data_len(0);
	const char* data(nullptr);
	if (tokenizer.has_more()){
		data = tokenizer.next(data_len);
		std::string header(data, data + data_len);
		if (tokenizer.has_more()){
			data = tokenizer.next(data_len);
			std::string content(data, data + data_len);
			trim(header, "\r\n");
			trim(content, "\r\n");

			// Deal with the header
			string_tokenizer header_tokenizer(header, "\r\n");
			std::string name, content_type, file_name;
			while (header_tokenizer.has_more()){
				data = header_tokenizer.next(data_len);
				std::pair<std::string, std::string> header_parts(move(this->split_string(data, data_len, ':')));
				trim(header_parts.first, " ");
				trim(header_parts.second, " ");
				if (strcasecmp(header_parts.first.c_str(), "Content-Type") == 0){
					content_type = header_parts.second;
				}
				else{
					string_tokenizer header_info_tokenizer(header_parts.second, ";");
					while (header_info_tokenizer.has_more()){
						data = header_info_tokenizer.next(data_len);
						std::pair<std::string, std::string> tokens(move(this->split_string(data, data_len, '=')));
						trim(tokens.first, " ");
						trim(tokens.second, "\"");
						if (tokens.first.length() > 0){
							if (strcasecmp(tokens.first.c_str(), "name") == 0){
								name = tokens.second;
							}
							else if (strcasecmp(tokens.first.c_str(), "filename") == 0){
								file_name = tokens.second;
							}
						}
					}
				}
			}

			if (file_name.length() == 0){
				// Not a file
				this->_request_data.insert(move(make_pair(name, content)));
			}
			else{
				// A file, need to store the file to a temp file
				this->save_uploaded_file(name, file_name, content.data(), content.size(), content_type);
			}
		}
	}
}

void http_request::dump_all(http_response* response) const{
	response->set_status(200, "OK");
	response->set_header("Content-Type", "text/html; charset=utf-8");

	std::string dump_data("Dump of request object: <br />");
	dump_data += "<b>SERVER VARIABLES: </b><br />";
	for (key_value_iterator it = this->_server_variables.begin(); it != this->_server_variables.end(); ++it){
		dump_data += it->first;
		dump_data += "=";
		dump_data += it->second;
		dump_data += "<br />";
	}

	dump_data += "<br /><b>REQUEST DATA:</b> <br />";
	for (key_value_iterator it = this->_request_data.begin(); it != this->_request_data.end(); ++it){
		dump_data += it->first;
		dump_data += "=";
		dump_data += it->second;
		dump_data += "<br />";
	}

	dump_data += "<br /><b>UPLOADED FILES:</b> <br />";
	for (file_iterator it = this->_upload_files.begin(); it != this->_upload_files.end(); ++it){
		dump_data += it->first;
		dump_data += "={";
		dump_data += it->second->name();
		dump_data += ",";
		dump_data += it->second->tmp_path();
		dump_data += ",";
		dump_data += it->second->content_type();
		dump_data += "}";
		dump_data += "<br />";
	}
	
	response->write(dump_data);
}
