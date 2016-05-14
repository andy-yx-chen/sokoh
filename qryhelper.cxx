#include "sokoh.hxx"

std::string NotExpression::GetExpression() const{
	std::string sqlClause;
	sqlClause.append(" NOT (");
	sqlClause.append(innerExp_->GetExpression());
	sqlClause.append(") ");
	return sqlClause;
}

void NotExpression::Bind(sql::PreparedStatement& cmd, int& index){
	innerExp_->Bind(cmd, index);
}

std::string BinaryExpression::GetExpression() const{
	std::string sqlClause(" (");
	sqlClause.append(left_->GetExpression());
	if (isAnd_){
		sqlClause.append(" AND ");
	}
	else{
		sqlClause.append(" OR ");
	}
	sqlClause.append(right_->GetExpression());
	sqlClause.append(") ");
	return sqlClause;
}

void BinaryExpression::Bind(sql::PreparedStatement& cmd, int& index){
	//bind left first and then right, order cannot be changed
	left_->Bind(cmd, index);
	right_->Bind(cmd, index);
}

template<>
std::shared_ptr<QueryExpression> operator %(const Property& prop, const char* val){
	return std::shared_ptr<QueryExpression>(new QueryObject<const char*>(prop.GetName(), QueryOp::LIKE, val));
}

template<>
std::shared_ptr<QueryExpression> operator %(const Property& prop, std::string& val){
	return std::shared_ptr<QueryExpression>(new QueryObject<std::string>(prop.GetName(), QueryOp::LIKE, val));
}

std::shared_ptr<QueryExpression> operator &&(std::shared_ptr<QueryExpression> left, std::shared_ptr<QueryExpression> right){
	return std::shared_ptr<QueryExpression>(new BinaryExpression(left, true, right));
}

std::shared_ptr<QueryExpression> operator ||(std::shared_ptr<QueryExpression> left, std::shared_ptr<QueryExpression> right){
	return std::shared_ptr<QueryExpression>(new BinaryExpression(left, false, right));
}

std::shared_ptr<QueryExpression> operator !(std::shared_ptr<QueryExpression> op){
	return std::shared_ptr<QueryExpression>(new NotExpression(op));
}

template<>
void  QueryObject<int>::Bind(sql::PreparedStatement& cmd, int& index){
	cmd.setInt(index++, value_);
}

template<>
void QueryObject<std::string>::Bind(sql::PreparedStatement& cmd, int& index){
	cmd.setString(index++, value_);
}

template<>
void QueryObject<const char*>::Bind(sql::PreparedStatement& cmd, int& index){
	cmd.setString(index++, value_);
}

template<>
void QueryObject<bool>::Bind(sql::PreparedStatement& cmd, int& index){
	cmd.setBoolean(index++, value_);
}

template<>
void QueryObject<int64_t>::Bind(sql::PreparedStatement& cmd, int& index){
	cmd.setInt64(index++, value_);
}
