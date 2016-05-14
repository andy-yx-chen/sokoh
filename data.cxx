#include "sokoh.hxx"

const std::string topic::object_name("topic");
const std::string topic::fields("id, title, launched_time, description");
const std::string topic::order_fields("id ASC");

const std::string article::object_name("article");
const std::string article::fields("id, topic_id, title, author, published_time, content");
const std::string article::order_fields("id DESC");

const std::string comment::object_name("comment");
const std::string comment::fields("id, article_id, time_, title, author, content");
const std::string comment::order_fields("id DESC");

const std::string user::object_name("author");
const std::string user::fields("id, alias, email, first_name, last_name");
const std::string user::order_fields("id DESC");

sql::Driver* driver(nullptr);
sql::Connection* connection(nullptr);


void init_db_connection(const std::string& server, const std::string& db_name, const std::string& user, const std::string& password){
	if (driver == nullptr){
		driver = sql::mysql::get_driver_instance();
	}

	release_db_connection();

	connection = driver->connect(server, user, password);
	connection->setSchema(db_name);
	std::shared_ptr<sql::Statement> stmt(connection->createStatement());
	stmt->execute("SET NAMES 'utf8'");
}

sql::Connection* get_db_connection(){
	return connection;
}

void release_db_connection(){
	if (connection != nullptr){
		connection->close();
		delete connection;
		connection = nullptr;
	}
}

std::shared_ptr<topic> topic::create_instance(sql::ResultSet& rs){
	std::shared_ptr<topic> obj(new topic(rs.getString(2), rs.getString(4), date_time(rs.getString(3))));
	obj->set_id(rs.getInt(1));
	return std::move(obj);
}

std::shared_ptr<article> article::create_instance(sql::ResultSet& rs){
	std::shared_ptr<article> obj(new article(rs.getInt(2), date_time(rs.getString(5).c_str()), rs.getString(4), rs.getString(3), rs.getString(6)));
	obj->set_id(rs.getInt(1));
	return std::move(obj);
}

std::shared_ptr<comment> comment::create_instance(sql::ResultSet& rs){
	std::shared_ptr<comment> obj(new comment(rs.getInt(2), date_time(rs.getString(3).c_str()), rs.getString(5), rs.getString(4), rs.getString(6)));
	obj->set_id(rs.getInt(1));
	return std::move(obj);
}

std::shared_ptr<user> user::create_instance(sql::ResultSet& rs){
	std::shared_ptr<user> obj(new user(rs.getString(2), rs.getString(3), rs.getString(4), rs.getString(5)));
	obj->set_id(rs.getInt(1));
	return std::move(obj);
}

template<>
std::string tmpl_value_object<std::shared_ptr<topic> >::value() const{
	return "object [topic]";
}

template<>
std::string tmpl_value_object<std::shared_ptr<topic> >::property(const std::string& name) const{
	if (name == "id"){
		char buffer[20] = { 0 };
                sprintf(buffer, "%d", this->value_->get_id());
		return buffer;
	}
	else if (name == "title"){
		return this->value_->get_title();
	}
	else if (name == "launched_time"){
		return this->value_->get_launched_time().text();
	}
	else if (name == "description"){
		return this->value_->get_description();
	}
	else{
		return null_str;
	}
}

template<>
std::string tmpl_value_object<std::shared_ptr<article> >::value() const{
	return "object [article]";
}

template<>
std::string tmpl_value_object<std::shared_ptr<article> >::property(const std::string& name) const{
	if (name == "id"){
		char buffer[20] = { 0 };
                sprintf(buffer, "%d", this->value_->get_id());
		return buffer;
	}
	else if (name == "topic_id"){
		char buffer[20] = { 0 };
                sprintf(buffer, "%d", this->value_->get_topic_id());
		return buffer;
	}
	else if (name == "title"){
		return this->value_->get_title();
	}
	else if (name == "published_time"){
		return this->value_->get_published_time().text();
	}
	else if (name == "content"){
		return this->value_->get_content();
	}
	else if (name == "author"){
		return this->value_->get_author();
	}
	else{
		return null_str;
	}
}

template<>
std::string tmpl_value_object<std::shared_ptr<comment> >::value() const{
	return "object [comment]";
}

template<>
std::string tmpl_value_object<std::shared_ptr<comment> >::property(const std::string& name) const{
	if (name == "id"){
		char buffer[20] = { 0 };
                sprintf(buffer, "%d", this->value_->get_id());
		return buffer;
	}
	else if (name == "article_id"){
		char buffer[20] = { 0 };
                sprintf(buffer, "%d", this->value_->get_article_id());
		return buffer;
	}
	else if (name == "title"){
		return this->value_->get_title();
	}
	else if (name == "time"){
		return this->value_->get_time().text();
	}
	else if (name == "content"){
		return this->value_->get_content();
	}
	else if (name == "author"){
		return this->value_->get_author();
	}
	else{
		return null_str;
	}
}
