#ifndef __SPARTA_DATA__
#define __SPARTA_DATA__

class topic{
public:
	static const std::string object_name;
	static const std::string fields;
	static const std::string order_fields;

public:
	topic(const std::string& title, const std::string description, const date_time& launched_time)
	  : id_(0), launched_time_(launched_time), title_(title), description_(description){
	}

	topic(const topic& other) :
		id_(other.id_), launched_time_(other.launched_time_), title_(other.title_), description_(other.description_){
	}

	topic& operator = (const topic& other){
		if (this == &other){
			return *this;
		}

		topic temp(other);
		temp.swap(*this);
		return *this;
	}

	void swap(topic& other){
		std::swap(this->id_, other.id_);
		this->launched_time_.swap(other.launched_time_);
		std::swap(this->title_, other.title_);
		std::swap(this->description_, other.description_);
	}

public:
	static std::shared_ptr<topic> create_instance(sql::ResultSet& reader);

public:
	int get_id() const{
		return this->id_;
	}

	const date_time& get_launched_time() const{
		return this->launched_time_;
	}

	const std::string& get_title() const{
		return this->title_;
	}

	const std::string& get_description() const{
		return this->description_;
	}

	void set_id(int id){
		this->id_ = id;
	}

private:
	int id_;
	date_time launched_time_;
	std::string title_;
	std::string description_;
};

class article{
public:
	static const std::string object_name;
	static const std::string fields;
	static const std::string order_fields;

public:
	article(int topic_id, const date_time& published_time, const std::string author, const std::string title, const std::string content)
		: id_(0), topic_id_(topic_id), published_time_(published_time), title_(title), author_(author), content_(content){
	}

	article(const article& other)
		: id_(other.id_), topic_id_(other.topic_id_), published_time_(other.published_time_), title_(other.title_), author_(other.author_), content_(other.content_){
	}

	article& operator = (const article& other){
		if (this == &other){
			return *this;
		}

		article temp(other);
		temp.swap(*this);
		return *this;
	}

	void swap(article& other){
		std::swap(this->id_, other.id_);
		std::swap(this->topic_id_, other.topic_id_);
		this->published_time_.swap(other.published_time_);
		std::swap(this->author_, other.author_);
		std::swap(this->title_, other.title_);
		std::swap(this->content_, other.content_);
	}

public:
	static std::shared_ptr<article> create_instance(sql::ResultSet& reader);

public:
	int get_id() const{
		return this->id_;
	}

	int get_topic_id() const{
		return this->topic_id_;
	}

	const date_time& get_published_time() const{
		return this->published_time_;
	}

	const std::string& get_title() const{
		return this->title_;
	}

	const std::string& get_author() const{
		return this->author_;
	}

	const std::string& get_content() const{
		return this->content_;
	}

	void set_id(int id){
		this->id_ = id;
	}

private:
	int id_;
	int topic_id_;
	date_time published_time_;
	std::string title_;
	std::string author_;
	std::string content_;
};

class comment{
public:
	static const std::string object_name;
	static const std::string fields;
	static const std::string order_fields;

public:
	comment(int article_id, const date_time& time, const std::string author, const std::string title, const std::string content)
		: id_(0), article_id_(article_id), time_(time), title_(title), author_(author), content_(content){
	}

	comment(const comment& other)
		: id_(other.id_), article_id_(other.article_id_), time_(other.time_), title_(other.title_), author_(other.author_), content_(other.content_){
	}

	comment& operator = (const comment& other){
		if (this == &other){
			return *this;
		}

		comment temp(other);
		temp.swap(*this);
		return *this;
	}

	void swap(comment& other){
		std::swap(this->id_, other.id_);
		std::swap(this->article_id_, other.article_id_);
		this->time_.swap(other.time_);
		std::swap(this->author_, other.author_);
		std::swap(this->title_, other.title_);
		std::swap(this->content_, other.content_);
	}

public:
	static std::shared_ptr<comment> create_instance(sql::ResultSet& reader);

public:
	int get_id() const{
		return this->id_;
	}

	int get_article_id() const{
		return this->article_id_;
	}

	const date_time& get_time() const{
		return this->time_;
	}

	const std::string& get_title() const{
		return this->title_;
	}

	const std::string& get_author() const{
		return this->author_;
	}

	const std::string& get_content() const{
		return this->content_;
	}

	void set_id(int id){
		this->id_ = id;
	}

private:
	int id_;
	int article_id_;
	date_time time_;
	std::string title_;
	std::string author_;
	std::string content_;
};

class user{
public:
	static const std::string object_name;
	static const std::string fields;
	static const std::string order_fields;

public:
	user()
		: id_(0), alias_(), email_(), first_name_(), last_name_(){
	}

	user(const std::string& alias, const std::string& email, const std::string& first_name, const std::string& last_name )
		: id_(0), alias_(alias), email_(email), first_name_(first_name), last_name_(last_name){}

	user(const user& other)
		: id_(other.id_), alias_(other.alias_), email_(other.email_), first_name_(other.first_name_), last_name_(other.last_name_){}

	user& operator = (const user& other){
		user tmp(other);
		this->swap(tmp);
		return *this;
	}

public:
	void swap(user& other){
		std::swap(this->id_, other.id_);
		std::swap(this->alias_, other.alias_);
		std::swap(this->email_, other.email_);
		std::swap(this->first_name_, other.first_name_);
		std::swap(this->last_name_, other.last_name_);
	}

public:
	void set_id(int id){
		this->id_ = id;
	}

	const std::string& get_alias() const{
		return this->alias_;
	}

	const std::string& get_email() const{
		return this->email_;
	}

	const std::string& get_first_name() const{
		return this->first_name_;
	}

	const std::string& get_last_name() const{
		return this->last_name_;
	}

	int get_id() const{
		return this->id_;
	}

public:
	static std::shared_ptr<user> create_instance(sql::ResultSet& reader);

private:
	int id_;
	std::string alias_;
	std::string email_;
	std::string first_name_;
	std::string last_name_;
};

void init_db_connection(const std::string& server, const std::string& db_name, const std::string& user, const std::string& password);

sql::Connection* get_db_connection();

void release_db_connection();
#endif
