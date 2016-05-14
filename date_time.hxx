#ifndef __SPARTA_DATE_TIME__
#define __SPARTA_DATE_TIME__

struct date_time_impl;

class date_time{
public:
	date_time();
	date_time(unsigned int year, unsigned int month, unsigned int day, unsigned int hour = 0, unsigned int minute = 0, unsigned int seconds = 0, unsigned int milliseconds = 0);
	date_time(const date_time& other);
	date_time(date_time&& other);
	date_time(const std::string& text);
	~date_time();

public:
	date_time& operator = (const date_time& source);
	void swap(date_time& other) throw();
	unsigned int get_year() const;
	unsigned int get_month() const;
	unsigned int get_day() const;
	unsigned int get_day_of_week() const;
	unsigned int get_hour() const;
	unsigned int get_minute() const;
	unsigned int get_second() const;
	unsigned int get_millisecond() const;
	std::string text() const;

private:
	date_time_impl* _impl;
};
#endif