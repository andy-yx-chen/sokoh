#include "../sokoh.hxx"
#include <assert.h>

using namespace std;

stringstream* out;

string request_data_literal = "request data from http request";

class blog{
public:
	blog(const string& title, const string& author, const string& content)
		: title_(title), author_(author), content_(content){
	}

	blog(const blog& other)
		: title_(other.title_), author_(other.author_), content_(other.content_){

	}

public:
	string get_title() const{
		return this->title_;
	}

	string get_author() const{
		return this->author_;
	}

	string get_content() const{
		return this->content_;
	}

private:
	string title_;
	string author_;
	string content_;
};

template<>
string tmpl_value_object<blog>::value() const{
	return "[object of Blog]";
}

template<> 
string tmpl_value_object<blog>::property(const string& name) const{
	if (name == "title"){
		return this->value_.get_title();
	}
	else if (name == "author"){
		return this->value_.get_author();
	}
	else if (name == "content"){
		return this->value_.get_content();
	}

	return "[property not found]";
}

void test_tmpl_objects();
void test_stmts();
void test_compiler();
void test_date_time();
void test_strings();
void test_mysql();
void test_image_codec();

int main(){
	test_tmpl_objects();
	test_stmts();
	test_compiler();
	test_date_time();
	test_strings();
	test_mysql();
	test_image_codec();
	cout << "test done." << endl;
	return 0;
}

void test_image_codec(){
  std::shared_ptr<jpeg_image_codec> codec(new jpeg_image_codec(100));
  std::shared_ptr<buffered_image> image = codec->decode("test.jpg");
  assert(image != nullptr);
  assert(image->width() == 441);
  assert(image->height() == 394);
  codec->encode(image, "test-result.jpg");
  image = codec->decode("test-result.jpg");
  assert(image != nullptr);
  assert(image->width() == 441);
  assert(image->height() == 394);
  std::shared_ptr<buffered_image> new_img = image->resize(110, 98);
  codec->encode(new_img, "test-resize-result.jpg");
}

void test_mysql(){
	string url("localhost");
	const string user("root");
	const string pass("test-pass");
	const string database("test");

	cout << "testing mysql with data objects" << endl;
	try {

		init_db_connection(url, database, user, pass);
		shared_ptr<sql::PreparedStatement> command(get_db_connection()->prepareStatement("INSERT INTO topic(title, launched_time, description) values (?,?,?)"));
		command->setString(1, "test topic");
		command->setString(2, date_time(2014, 7, 1).text().c_str());
		command->setString(3, "description for test topic");
		command->execute();
		shared_ptr<sql::Statement> stmt(get_db_connection()->createStatement());
		shared_ptr<sql::ResultSet> rs(stmt->executeQuery("SELECT LAST_INSERT_ID();"));
		rs->next();
		int topic_id = rs->getInt(1);
		assert(topic_id != 0);
		shared_ptr<list<shared_ptr<topic> > > topics = query_objects<topic>(Property("id") == topic_id);
		assert(topics->size() == 1);
		shared_ptr<topic> topic = *(topics->begin());
		assert(topic->get_title() == "test topic");
		assert(topic->get_launched_time().get_year() == 2014);
		assert(topic->get_launched_time().get_month() == 7);
		assert(topic->get_launched_time().get_day() == 1);
		assert(topic->get_description() == "description for test topic");
		assert(topic->get_id() == topic_id);

		command.reset(get_db_connection()->prepareStatement("INSERT INTO article(topic_id, title, author, published_time, content) values (?,?,?,?,?)"));
		command->setInt(1, topic->get_id());
		command->setString(2, "article title");
		command->setString(3, "andy");
		command->setString(4, date_time(2014, 7, 1).text().c_str());
		command->setString(5, "test content");
		command->execute();
		stmt.reset(get_db_connection()->createStatement());
		rs.reset(stmt->executeQuery("SELECT LAST_INSERT_ID();"));
		rs->next();
		int article_id = rs->getInt(1);
		shared_ptr<list<shared_ptr<article> > > articles = query_objects<article>(Property("id") == article_id);
		assert(articles->size() == 1);
		shared_ptr<article> article = *(articles->begin());
		assert(article->get_id() == article_id);
		assert(article->get_title() == "article title");
		assert(article->get_author() == "andy");
		assert(article->get_content() == "test content");
		assert(article->get_topic_id() == topic_id);
		assert(article->get_published_time().get_year() == 2014);
		assert(article->get_published_time().get_month() == 7);
		assert(article->get_published_time().get_day() == 1);

		command.reset(get_db_connection()->prepareStatement("INSERT INTO comment(article_id, time_, title, author, content) values (?,?,?,?,?)"));
		command->setInt(1, article->get_id());
		command->setString(2, date_time(2014, 7, 1).text().c_str());
		command->setString(3, "comment title");
		command->setString(4, "andy");
		command->setString(5, to_string(L"test content内容"));
		command->execute();
		stmt.reset(get_db_connection()->createStatement());
		rs.reset(stmt->executeQuery("SELECT LAST_INSERT_ID();"));
		rs->next();
		int comment_id = rs->getInt(1);
		shared_ptr<list<shared_ptr<comment> > > comments = query_objects<comment>(Property("id") == comment_id);
		assert(comments->size() == 1);
		shared_ptr<comment> c = *(comments->begin());
		assert(c->get_id() == comment_id);
		assert(c->get_title() == "comment title");
		assert(c->get_author() == "andy");
		assert(c->get_content() == to_string(L"test content内容"));
		assert(c->get_article_id() == article_id);
		assert(c->get_time().get_year() == 2014);
		assert(c->get_time().get_month() == 7);
		assert(c->get_time().get_day() == 1);

		comments = query_objects<comment>(Property("content") % "%test%");
		assert(comments->size() > 0);
		release_db_connection();
	}
	catch (sql::SQLException &e) {
		/*
		MySQL Connector/C++ throws three different exceptions:

		- sql::MethodNotImplementedException (derived from sql::SQLException)
		- sql::InvalidArgumentException (derived from sql::SQLException)
		- sql::SQLException (derived from std::runtime_error)
		*/
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		/* what() (derived from std::runtime_error) fetches error message */
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}
}

void test_strings(){
	string str("hello");
	assert(L"hello" == to_wstring(str));

	wstring str1(L"world");
	assert("world" == to_string(str1));

	assert("YW55IGNhcm5hbCBwbGVhc3VyZS4=" == base64_encode("any carnal pleasure."));
	assert("YW55IGNhcm5hbCBwbGVhc3VyZQ==" == base64_encode("any carnal pleasure"));
	assert("YW55IGNhcm5hbCBwbGVhc3Vy" == base64_encode("any carnal pleasur"));
	assert("YW55IGNhcm5hbCBwbGVhc3U=" == base64_encode("any carnal pleasu"));
	assert("YW55IGNhcm5hbCBwbGVhcw==" == base64_encode("any carnal pleas"));
	assert("any carnal pleasure." == base64_decode("YW55IGNhcm5hbCBwbGVhc3VyZS4="));
	assert("any carnal pleasure" == base64_decode("YW55IGNhcm5hbCBwbGVhc3VyZQ=="));
	assert("any carnal pleasur" == base64_decode("YW55IGNhcm5hbCBwbGVhc3Vy"));
	assert("any carnal pleasu" == base64_decode("YW55IGNhcm5hbCBwbGVhc3U="));
	assert("any carnal pleas" == base64_decode("YW55IGNhcm5hbCBwbGVhcw=="));
}

void test_date_time(){
	date_time dt;
	assert(dt.get_year() != 0);
	cout << tmpl_value_object<date_time>(dt).value() << endl;

    	date_time dt1(2014, 7, 1);
	assert("2014" == tmpl_value_object<date_time>(dt1).property("year"));

	dt = dt1;
	assert(dt.get_year() == dt1.get_year());
	assert(dt.get_month() == dt1.get_month());
	assert(dt.get_day() == dt1.get_day());
	assert(dt.get_hour() == 0);

	date_time dt3(move(dt));

	assert(dt.get_year() == 0);
	assert(dt3.get_year() == dt1.get_year());
	assert(dt3.get_month() == dt1.get_month());
	assert(dt3.get_day() == dt1.get_day());
	assert(dt3.get_hour() == 0);

	date_time dt4;
	dt3.swap(dt4);

	assert(dt4.get_year() == dt1.get_year());
	assert(dt4.get_month() == dt1.get_month());
	assert(dt4.get_day() == dt1.get_day());
	assert(dt4.get_hour() == 0);

	date_time dt5("3/4/2014 15:20:34.633");
	assert(dt5.get_year() == 2014);
	assert(dt5.get_month() == 3);
	assert(dt5.get_day() == 4);
	assert(dt5.get_hour() == 15);
	assert(dt5.get_minute() == 20);
	assert(dt5.get_second() == 34);

}

void test_compiler(){
	shared_ptr<tmpl_composite_statement> program = compile_template("test_tmpl.txt");
	tmpl_data data;
	http_request request(nullptr);
	http_response response(nullptr);
	tmpl_context context(request, response, data);
	blog blg("blog-title", "andy", "blog-content");
	out = new stringstream();
	shared_ptr<tmpl_object> number_obj(new tmpl_value_object<int>(123));
	context.set_data("number", number_obj);
	shared_ptr<tmpl_object> title_obj(new tmpl_value_object<string>("hello, template"));
	context.set_data("title", title_obj);
	shared_ptr<tmpl_object> blog_obj(new tmpl_value_object<blog>(blg));
	context.set_data("blog", blog_obj);
	list<int> values;
	values.push_back(50);
	values.push_back(100);
	values.push_back(150);

	std::shared_ptr<tmpl_object> items_obj(new tmpl_collection_object<list<int> >(values));
	context.set_data("items", items_obj);
	program->execute(context);
	assert(out->str() == "{}leadinghello, templatetrue50100150aending.blog-title+andy+blog-content");
	delete out;

	out = new stringstream();
	shared_ptr<tmpl_object> number_obj1(new tmpl_value_object<int>(99));
	context.set_data("number", number_obj1);
	program->execute(context);
	assert(out->str() == "{}leadinghello, templatefalse50100150aending.blog-title+andy+blog-content");
	delete out;

}

void test_stmts(){
	tmpl_data data;
	http_request request(nullptr);
	http_response response(nullptr);
	tmpl_context context(request, response, data);

	shared_ptr<tmpl_statement> stmt(new tmpl_literal_statement("literal string"));
	
	out = new stringstream();
	stmt->execute(context);
	assert(out->str() == "literal string");

	delete out;

	out = new stringstream();
	shared_ptr<tmpl_object> hell(new tmpl_value_object<string>("hello,world!"));
	context.set_data("test_obj", hell);
	stmt = std::shared_ptr<tmpl_statement>(new tmpl_out_statement("test_obj"));
	stmt->execute(context);

	assert(out->str() == "hello,world!");
	delete out;

	out = new stringstream();
	shared_ptr<tmpl_composite_statement> block(new tmpl_composite_statement());
	block->add_statement(shared_ptr<tmpl_statement>(new tmpl_out_statement("item")));
	block->add_statement(shared_ptr<tmpl_statement>(new tmpl_literal_statement("literal")));
	tmpl_if_statement* if_stmt = new tmpl_if_statement("cond", tmpl_if_statement::eq, "100");
	if_stmt->set_statement_when_true(block);
	stmt = shared_ptr<tmpl_statement>(if_stmt);
	
	shared_ptr<tmpl_object> cond_obj(new tmpl_value_object<int>(100));
	context.set_data("cond", cond_obj);
	shared_ptr<tmpl_object> item_obj(new tmpl_value_object<int>(300));
	context.set_data("item", item_obj);
	stmt->execute(context);

	assert(out->str() == "300literal");

	delete out;
	out = new stringstream();
	if_stmt->set_statement_when_false(block);
	shared_ptr<tmpl_object> cond_obj1(new tmpl_value_object<int>(200));
	context.set_data("cond", cond_obj1);
	stmt->execute(context);

	assert(out->str() == "300literal");
	delete out;

	out = new stringstream();

	tmpl_foreach_statement* for_stmt = new tmpl_foreach_statement("items", "item");
	for_stmt->set_iteration_body(block);
	list<int> values;
	values.push_back(50);
	values.push_back(100);
	values.push_back(150);
	std::shared_ptr<tmpl_object> values_obj(new tmpl_collection_object<list<int> >(values));
	context.set_data("items", values_obj);
	stmt = shared_ptr<tmpl_statement>(for_stmt);
	stmt->execute(context);
	assert(out->str() == "50literal100literal150literal");
	
	delete out;
}

void test_tmpl_objects(){
	shared_ptr<tmpl_object> obj(new tmpl_value_object<int>(100));
	assert(!obj->is_collection());
	assert(obj->value() == "100");

	obj = shared_ptr<tmpl_object>(new tmpl_value_object<string>("hello"));
	assert(!obj->is_collection());
	assert(obj->value() == "hello");
	assert(obj->property("length") == "5");

	obj = shared_ptr<tmpl_object>(new tmpl_value_object<bool>(false));
	assert(!obj->is_collection());
	assert(obj->value() == bool_false);

	list<int> values;
	values.push_back(100);
	values.push_back(20);
	values.push_back(30);
	values.push_back(18);
	obj = shared_ptr<tmpl_object>(new tmpl_collection_object<list<int> >(values));
	obj->reset();
	assert(obj->value() == "100");
	assert(obj->current()->value() == obj->value());
	assert(obj->next());
	assert(obj->value() == "20");
	assert(obj->current()->value() == obj->value());
	assert(obj->next());
	assert(obj->value() == "30");
	assert(obj->current()->value() == obj->value());
	assert(obj->next());
	assert(obj->value() == "18");
	assert(obj->current()->value() == obj->value());
	assert(!obj->next());
}

http_response::http_response(FCGX_Stream* out, const char* protocol_version){
}

http_request::http_request(FCGX_Request* request){
}

http_request::~http_request(){
}

const std::string& http_request::request_data(const std::string& name) const{
	return request_data_literal;
}

void http_response::write(const string& line){
	*out << line;
}

const std::string& http_request::get_attribute(const std::string& name) const{
        return request_data_literal;
}

const std::string& http_request::header(const std::string& name) const{
        return request_data_literal;
}

std::string http_request::physical_path(const std::string& file) const{
        return request_data_literal;
}
