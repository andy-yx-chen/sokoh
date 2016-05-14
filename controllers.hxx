#ifndef __SPARTA_CONTROLLER__
#define __SPARTA_CONTROLLER__

class controller{
public:
	controller() {}

private:
	controller(const controller&);
	controller& operator = (const controller&);

public:
	void execute(http_request& request, http_response& response);

protected:
	virtual std::string post(tmpl_context& context);
	virtual std::string get(tmpl_context& context);
	virtual std::string put(tmpl_context& context);
	virtual std::string del(tmpl_context& context);

	std::shared_ptr<user> authenticate(http_request& request);

public:
	typedef std::unordered_map<std::string, std::shared_ptr<tmpl_composite_statement> > tmpl_cache_type;
	typedef tmpl_cache_type::iterator tmpl_iterator;

private:

	static tmpl_cache_type cache_templates_;
};

class home_controller : public controller{
protected:
	virtual std::string get(tmpl_context& context);
	virtual std::string post(tmpl_context& context);

};

class topic_controller : public controller{
protected:
	virtual std::string get(tmpl_context& context);
	virtual std::string post(tmpl_context& context);
	virtual std::string del(tmpl_context& context);
};

class article_controller : public controller{
protected:
	virtual std::string get(tmpl_context& context);
	virtual std::string post(tmpl_context& context);
	virtual std::string del(tmpl_context& context);
	virtual std::string put(tmpl_context& context);
};

class comment_controller : public controller{
protected:
	virtual std::string del(tmpl_context& context);
};

class manage_controller : public controller{
protected:
	virtual std::string get(tmpl_context& context);
};

class file_manager_controller : public controller{
protected:
  virtual std::string get(tmpl_context& context);
  virtual std::string post(tmpl_context& context);
};
#endif
