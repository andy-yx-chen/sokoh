#ifndef __QRY_HELPER_HXX__

enum QueryOp{
    EQ,
    NEQ,
    GE,
    GT,
    LE,
    LT,
	LIKE
};

class QueryExpression{
public:
    QueryExpression(){};
    virtual ~QueryExpression(){}//do nothing, but we really need this.
    virtual std::string GetExpression() const = 0;
	virtual void Bind(sql::PreparedStatement& cmd , int& index) = 0;

private:
    //disable copy and assignment operator
    QueryExpression(const QueryExpression&);
    QueryExpression& operator = (const QueryExpression&);
};

class NotExpression : public QueryExpression{
public:
  explicit NotExpression(std::shared_ptr<QueryExpression> inner) : QueryExpression(), innerExp_(inner){}
    virtual ~NotExpression(){}//nothing need to be freed explicitly

    virtual std::string GetExpression() const;
	virtual void Bind(sql::PreparedStatement& cmd, int& index);
private:
    std::shared_ptr<QueryExpression> innerExp_;
};

class BinaryExpression : public QueryExpression{
public:
  explicit BinaryExpression(std::shared_ptr<QueryExpression> left, bool isAnd, std::shared_ptr<QueryExpression> right) :QueryExpression(), left_(left), isAnd_(isAnd), right_(right){}
    virtual ~BinaryExpression(){}//nothing need to be freed explicitly

    virtual std::string GetExpression() const;
	virtual void Bind(sql::PreparedStatement& cmd, int& index);
private:
    std::shared_ptr<QueryExpression> left_;
    bool isAnd_;
    std::shared_ptr<QueryExpression> right_;
};

template<typename ValueType>
class QueryObject : public QueryExpression{
public:
  QueryObject(const std::string& name, QueryOp op, ValueType& value) :QueryExpression(), propName_(name), value_(value), op_(op){};
    ~QueryObject(){};//nothing to do
	virtual std::string GetExpression() const{
		std::string sql(propName_);
		sql.append(GetOperator());
		sql.append("?");
		return sql;
	}
	virtual void Bind(sql::PreparedStatement& cmd, int& index);
private:
    const char* GetOperator() const{
        switch(op_){
        case EQ:
            return "=";
        case NEQ:
            return "<>";
        case LE:
            return "<=";
        case GE:
            return ">=";
        case LT:
            return "<";
        case GT:
            return ">";
		case LIKE:
			return " LIKE ";
        default:
            throw new std::string("invalid operator");
        }
    }

private:
    std::string propName_;
    ValueType value_;
    QueryOp op_;
};

class Property {
public:
    explicit Property(const std::string& name): name_(name){};
    std::string GetName() const{
        return name_;
    }
private:
    std::string name_;
};

template<typename T>
std::shared_ptr<QueryExpression> operator==(const Property& prop, T val){
	return std::shared_ptr<QueryExpression>(new QueryObject<T>(prop.GetName(), QueryOp::EQ, val));
}

template<typename T>
std::shared_ptr<QueryExpression> operator!=(const Property& prop, T val){
	return std::shared_ptr<QueryExpression>(new QueryObject<T>(prop.GetName(), QueryOp::NEQ, val));
}

template<typename T>
std::shared_ptr<QueryExpression> operator>=(const Property& prop, T val){
	return std::shared_ptr<QueryExpression>(new QueryObject<T>(prop.GetName(), QueryOp::GE, val));
}

template<typename T>
std::shared_ptr<QueryExpression> operator<=(const Property& prop, T val){
	return std::shared_ptr<QueryExpression>(new QueryObject<T>(prop.GetName(), QueryOp::LE, val));
}

template<typename T>
std::shared_ptr<QueryExpression> operator>(const Property& prop, T val){
	return std::shared_ptr<QueryExpression>(new QueryObject<T>(prop.GetName(), QueryOp::GT, val));
}

template<typename T>
std::shared_ptr<QueryExpression> operator<(const Property& prop, T val){
	return std::shared_ptr<QueryExpression>(new QueryObject<T>(prop.GetName(), QueryOp::LT, val));
}


template<typename T>
std::shared_ptr<QueryExpression> operator %(const Property& prop, T val);

std::shared_ptr<QueryExpression> operator &&(std::shared_ptr<QueryExpression> left, std::shared_ptr<QueryExpression> right);

std::shared_ptr<QueryExpression> operator ||(std::shared_ptr<QueryExpression> left, std::shared_ptr<QueryExpression> right);

std::shared_ptr<QueryExpression> operator !(std::shared_ptr<QueryExpression> op);

template<typename T>
std::shared_ptr<std::list<std::shared_ptr<T> > > query_objects(std::shared_ptr<QueryExpression> exp){
	std::string sql = "SELECT ";
	sql += T::fields;
	sql += " FROM ";
	sql += T::object_name;
	if (exp != nullptr){
		sql += " WHERE ";
		sql += exp->GetExpression();
	}

	sql += " ORDER BY ";
	sql += T::order_fields;
	std::shared_ptr<sql::PreparedStatement> cmd(get_db_connection()->prepareStatement(sql));
	if (exp != nullptr){
		int index = 1;
		exp->Bind(*cmd, index);
	}

	std::shared_ptr<sql::ResultSet> reader(cmd->executeQuery());
	std::shared_ptr<std::list<std::shared_ptr<T> > > results(new std::list<std::shared_ptr<T> >());
	while (reader->next()){
		results->push_back(std::move(T::create_instance(*reader)));
	}

	return results;
}

#endif 
