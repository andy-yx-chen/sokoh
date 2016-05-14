#include "sokoh.hxx"

http_file::~http_file(){
	if (this->_tmp_file.size() > 0){
		unlink(this->_tmp_file.c_str());
	}
}

void http_file::move_to(const std::string& target_path) const{
  rename(this->_tmp_file.c_str(), target_path.c_str());
}

std::string http_file::content() const {
	std::string data("");
	int file = open(this->_tmp_file.c_str(), O_RDONLY);
	if (file != -1){
		// We don't accept any files that larger than 2GB
                struct stat st;
		if (0 == fstat(file, &st)){
			size_t file_size = st.st_size;
                        size_t total_bytes_read(0);
			ssize_t bytes_read(0);
			data.resize(file_size);
			while (total_bytes_read < file_size
				&& (bytes_read = read(file, (void*)(data.data() + total_bytes_read), file_size - total_bytes_read)) > 0){
				total_bytes_read += bytes_read;
			}
		}

		close(file);
	}

	return data;
}
