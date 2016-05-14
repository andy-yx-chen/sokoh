#include "sokoh.hxx"

typedef struct tm tm;
struct date_time_impl{
	tm _sys_time;
};

date_time::date_time()
	: _impl(new date_time_impl()){
        time_t t = time(nullptr);
        gmtime_r(&t, &(this->_impl->_sys_time));
        this->_impl->_sys_time.tm_year += 1900;
}

date_time::~date_time(){
	if (this->_impl != nullptr){
		delete this->_impl;
	}
}

date_time::date_time(unsigned int year, unsigned int month, unsigned int day, unsigned int hour, unsigned int minute, unsigned int seconds, unsigned int milliseconds)
	: _impl(new date_time_impl()){
	this->_impl->_sys_time.tm_year = (int)year;
	this->_impl->_sys_time.tm_mon = (int)month - 1;
	this->_impl->_sys_time.tm_mday = (int)day;
	this->_impl->_sys_time.tm_hour = (int)hour;
	this->_impl->_sys_time.tm_min = (int)minute;
	this->_impl->_sys_time.tm_sec = (int)seconds;
}

date_time::date_time(const std::string& text)
	: _impl(new date_time_impl()){
	size_t pos = text.find(' ');
	if (pos != std::string::npos && pos < text.length() - 1 && pos > 0){
		std::string date(text.begin(), text.begin() + pos);
		std::string time(text.begin() + pos + 1, text.end());
		string_tokenizer date_tokenizer(date, "/");
		std::string year, month, day, hours, minutes, seconds, milliseconds;
		size_t size;
		const char* str;
		if (date_tokenizer.has_more()){
			str = date_tokenizer.next(size);
			month.append(str, str + size);
		}

		if (date_tokenizer.has_more()){
			str = date_tokenizer.next(size);
			day.append(str, str + size);
		}

		if (date_tokenizer.has_more()){
			str = date_tokenizer.next(size);
			year.append(str, str + size);
		}

		string_tokenizer time_tokenizer(time, ":");
		if (time_tokenizer.has_more()){
			str = time_tokenizer.next(size);
			hours.append(str, str + size);
		}

		if (time_tokenizer.has_more()){
			str = time_tokenizer.next(size);
			minutes.append(str, str + size);
		}

		if (time_tokenizer.has_more()){
			str = time_tokenizer.next(size);
			std::string s_and_ms(str, str + size);
			pos = s_and_ms.find('.');
			if (pos != 0 && pos != std::string::npos && pos != s_and_ms.length() - 1){
				seconds.append(s_and_ms.begin(), s_and_ms.begin() + pos);
				milliseconds.append(s_and_ms.begin() + pos + 1, s_and_ms.end());
			}
			else{
				seconds.append(s_and_ms);
				milliseconds.append("0");
			}
		}

                sscanf(year.c_str(), "%d", &(this->_impl->_sys_time.tm_year));
                sscanf(month.c_str(), "%d", &(this->_impl->_sys_time.tm_mon));
                sscanf(day.c_str(), "%d", &(this->_impl->_sys_time.tm_mday));
                sscanf(hours.c_str(), "%d", &(this->_impl->_sys_time.tm_hour));
                sscanf(minutes.c_str(), "%d", &(this->_impl->_sys_time.tm_min));
                sscanf(seconds.c_str(), "%d", &(this->_impl->_sys_time.tm_sec));
                this->_impl->_sys_time.tm_mon -= 1;
	}
}

date_time::date_time(const date_time& other)
	: _impl(new date_time_impl()){
	this->_impl->_sys_time.tm_year = other._impl->_sys_time.tm_year;
	this->_impl->_sys_time.tm_mon = other._impl->_sys_time.tm_mon;
	this->_impl->_sys_time.tm_mday = other._impl->_sys_time.tm_mday;
	this->_impl->_sys_time.tm_hour = other._impl->_sys_time.tm_hour;
	this->_impl->_sys_time.tm_min = other._impl->_sys_time.tm_min;
	this->_impl->_sys_time.tm_sec = other._impl->_sys_time.tm_sec;
}

date_time::date_time(date_time&& other)
	: _impl(other._impl){
	other._impl = nullptr;
}

date_time& date_time::operator = (const date_time& other){
	if (this == &other){
		return *this;
	}

	if (other._impl != nullptr){
		if (this->_impl == nullptr){
			this->_impl = new date_time_impl();
		}

		date_time tmp(other);
		this->swap(tmp);
	}

	return *this;
}

void inline date_time::swap(date_time& other) throw(){
	std::swap(this->_impl, other._impl);
}

unsigned int date_time::get_year() const{
	if (this->_impl == nullptr){
		return 0;
	}

	return this->_impl->_sys_time.tm_year;
}

unsigned int date_time::get_month() const{
	if (this->_impl == nullptr){
		return 0;
	}

	return this->_impl->_sys_time.tm_mon + 1;
}

unsigned int date_time::get_day() const{
	if (this->_impl == nullptr){
		return 0;
	}

	return this->_impl->_sys_time.tm_mday;
}

unsigned int date_time::get_day_of_week() const{
	if (this->_impl == nullptr){
		return 0;
	}

	return this->_impl->_sys_time.tm_wday;
}

unsigned int date_time::get_hour() const{
	if (this->_impl == nullptr){
		return 0;
	}

	return this->_impl->_sys_time.tm_hour;
}

unsigned int date_time::get_minute() const{
	if (this->_impl == nullptr){
		return 0;
	}

	return this->_impl->_sys_time.tm_min;
}

unsigned int date_time::get_second() const{
	if (this->_impl == nullptr){
		return 0;
	}

	return this->_impl->_sys_time.tm_sec;
}

unsigned int date_time::get_millisecond() const{
        return 0;
}

std::string date_time::text() const{
	char buffer[200] = { '\0' };
	sprintf(buffer,
		"%02d/%02d/%04d %02d:%02d:%02d.%d",
		get_month(),
		get_day(),
		get_year(),
		get_hour(),
		get_minute(),
		get_second(),
		get_millisecond());

	return std::move(std::string(buffer));
}

template<>
std::string tmpl_value_object<date_time>::value() const{
	return this->value_.text();
}

template<>
std::string tmpl_value_object<date_time>::property(const std::string& name) const{
	char buffer[10] = { '\0' };
	int value = -1;
	if (name == "year"){
		value = this->value_.get_year();
	}
	else if (name == "month"){
		value = this->value_.get_month();
	}
	else if (name == "day"){
		value = this->value_.get_day();
	}
	else if (name == "hour"){
		value = this->value_.get_hour();
	}
	else if (name == "minute"){
		value = this->value_.get_minute();
	}
	else if (name == "second"){
		value = this->value_.get_second();
	}
	else if (name == "millisecond"){
		value = this->value_.get_millisecond();
	}
	else{
		return null_str;
	}

        sprintf(buffer, "%d", value);
	return std::move(std::string(buffer));
}
