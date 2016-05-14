#ifndef __SPARTA_HTML_TMPL__
#define __SPARTA_HTML_TMPL__

#define bool_true "true"
#define bool_false "false"
#define end_of_collection "eoc";
#define null_str "nil"
#define number_max_length 100

class tmpl_object{
public:
	tmpl_object(){}

	virtual std::string value() const = 0;
	virtual std::string property(const std::string& name) const = 0;
	virtual bool is_collection() const = 0;
	virtual bool next() = 0;
	virtual void reset() = 0;
	virtual std::shared_ptr<tmpl_object> current() = 0;

	virtual ~tmpl_object(){
	}

private:
	tmpl_object(const tmpl_object&);
	tmpl_object& operator = (const tmpl_object&);
};

template<typename T>
class tmpl_value_object : public tmpl_object{
public:
	explicit tmpl_value_object(const T& value)
		: value_(value){}

private:
	T value_;

public:
	virtual std::string value() const;

	virtual std::string property(const std::string& name) const;

	virtual bool is_collection() const{
		return false;
	}

	virtual bool next(){
		return false;
	}

	virtual std::shared_ptr<tmpl_object> current(){
		return std::shared_ptr<tmpl_object>(nullptr);
	}

	virtual void reset(){}
};

template<typename T>
class tmpl_collection_object : public tmpl_object{
public:
	explicit tmpl_collection_object(const T& value)
		: value_(value){
		this->iterator_ = this->value_.end();
	}

public:
	typedef typename T::const_iterator iterator;

private:
	T value_;
	iterator iterator_;

public:
	virtual std::string value() const{
		if (this->iterator_ == this->value_.end()){
			return end_of_collection;
		}

		return tmpl_value_object<typename T::value_type>(*(this->iterator_)).value();
	}

	virtual std::string property(const std::string& name) const{
		if (this->iterator_ == this->value_.end()){
			return end_of_collection;
		}

		return tmpl_value_object<typename T::value_type>(*(this->iterator_)).property(name);
	}

	virtual std::shared_ptr<tmpl_object> current(){
		if (this->iterator_ == this->value_.end()){
			return std::shared_ptr<tmpl_object>(nullptr);
		}

		return std::move(std::shared_ptr<tmpl_object>(new tmpl_value_object<typename T::value_type>(*(this->iterator_))));
	}

	virtual bool is_collection() const{
		return true;
	}

	virtual void reset(){
		this->iterator_ = this->value_.begin();
	}

	virtual bool next(){
		if (this->iterator_ == this->value_.end()){
			return false;
		}

		++this->iterator_;
		return this->iterator_ != this->value_.end();
	}
};

typedef std::unordered_map<std::string, std::shared_ptr<tmpl_object> > tmpl_data;

class tmpl_context{
public:
	tmpl_context(http_request& request, http_response& response, tmpl_data& data)
		: request_(request), response_(response), data_(data){

	}

public:
	http_request& request(){
		return this->request_;
	}

	http_response& response(){
		return this->response_;
	}

	std::string data(const std::string& name);

	std::shared_ptr<tmpl_object> data_object(const std::string& name);

	void set_data(const std::string& name, std::shared_ptr<tmpl_object>& obj);

  template<typename T>
  void set_value(const std::string& name, const T& value){
    std::shared_ptr<tmpl_object> value_object(new tmpl_value_object<T>(value));
    this->set_data(name, value_object);
  }
private:
	http_request& request_;
	http_response& response_;
	tmpl_data& data_;
};

class tmpl_statement{
public:
	tmpl_statement(){

	}

private:
	tmpl_statement(const tmpl_statement&);
	tmpl_statement& operator = (const tmpl_statement&);

public:
	virtual void execute(tmpl_context& context) = 0;
};

class tmpl_literal_statement: public tmpl_statement{
public:
	explicit tmpl_literal_statement(const std::string& out_string)
		: out_string_(out_string){

	}

public:
	virtual void execute(tmpl_context& context){
		context.response().write(this->out_string_);
	}

private:
	std::string out_string_;
};

class tmpl_out_statement : public tmpl_statement{
public:
	explicit tmpl_out_statement(const std::string& variable)
		: variable_(variable){

	}

public:
	virtual void execute(tmpl_context& context);

private:
	std::string variable_;
};

class tmpl_composite_statement : public tmpl_statement{
public:
	void add_statement(const std::shared_ptr<tmpl_statement>& statement){
		this->statements_.push_back(statement);
	}

	virtual void execute(tmpl_context& context);

private:
	std::list<std::shared_ptr<tmpl_statement> > statements_;
};

class tmpl_if_statement : public tmpl_statement{
public:
	enum comp_op{
		eq,
		gt,
		lt,
		ge,
		le,
		neq
	};

public:
	tmpl_if_statement(const std::string& variable, tmpl_if_statement::comp_op op, const std::string& value)
		: variable_(variable), op_(op), value_(value), statement_at_true_(nullptr), statement_at_false_(nullptr){

	}

public:
	void set_statement_when_true(std::shared_ptr<tmpl_composite_statement>& statement){
		this->statement_at_true_ = statement;
	}

	void set_statement_when_false(std::shared_ptr<tmpl_composite_statement>& statement){
		this->statement_at_false_ = statement;
	}

	virtual void execute(tmpl_context& context);

private:
	std::string variable_;
	comp_op op_;
        std::string value_;
        std::shared_ptr<tmpl_composite_statement> statement_at_true_;
	std::shared_ptr<tmpl_composite_statement> statement_at_false_;

};

class tmpl_foreach_statement : public tmpl_statement{
public:
	tmpl_foreach_statement(const std::string& variable, const std::string& item_name)
		: variable_(variable), item_name_(item_name), iteration_body_(nullptr){

	}

public:
	void set_iteration_body(const std::shared_ptr<tmpl_composite_statement>& iteration_body){
		this->iteration_body_ = iteration_body;
	}

	virtual void execute(tmpl_context& context);

private:
	std::string variable_;
	std::string item_name_;
	std::shared_ptr<tmpl_composite_statement> iteration_body_;
};

std::shared_ptr<tmpl_composite_statement> compile_template(const std::string& tmpl_file);
#endif
