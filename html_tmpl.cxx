#include "sokoh.hxx"

using namespace std;

template<>
std::string tmpl_value_object<std::string>::value() const{
	return this->value_;
}

template<>
std::string tmpl_value_object<std::string>::property(const std::string& name) const{
	if (name == "length"){
		int length = (int)this->value_.length();
		char size_value[number_max_length];
                sprintf(size_value, "%d", length);
		return std::move(std::string(size_value));
	}

	return null_str;
}

template<>
std::string tmpl_value_object<bool>::value() const{
	if (this->value_){
		return bool_true;
	}
	else{
		return bool_false;
	}
}

template<>
std::string tmpl_value_object<bool>::property(const std::string& name) const{
	return null_str;
}

template<>
std::string tmpl_value_object<int>::value() const{
	char str_value[number_max_length];
        sprintf(str_value, "%d", this->value_);
	return std::move(std::string(str_value));
}

template<>
std::string tmpl_value_object<int>::property(const std::string& name) const{
	return null_str;
}

template<>
std::string tmpl_value_object<float>::value() const{
	char str_value[number_max_length];
	sprintf(str_value, "%.2f", this->value_);
	return std::move(std::string(str_value));
}

template<>
std::string tmpl_value_object<float>::property(const std::string& name) const{
	return null_str;
}

string tmpl_context::data(const string& name){
	size_t dot_index = name.find_first_of('.');
	if (dot_index == std::string::npos){
		tmpl_data::const_iterator data_itor = this->data_.find(name);
		if (data_itor != this->data_.end()){
			return data_itor->second->value();
		}
		else{
			return this->request_.request_data(name);
		}
	}
	else{
		std::string var_name = name.substr(0, dot_index);
		std::string prop_name = name.substr(dot_index + 1);
		tmpl_data::const_iterator data_itor = this->data_.find(var_name);
		if (data_itor != this->data_.end()){
			return move(data_itor->second->property(prop_name));
		}
		else{
			return null_str;
		}
	}
}

shared_ptr<tmpl_object> tmpl_context::data_object(const string& name){
	tmpl_data::const_iterator data_itor = this->data_.find(name);
	if (data_itor != this->data_.end()){
		return data_itor->second;
	}

	return move(shared_ptr<tmpl_object>(nullptr));
}

void tmpl_context::set_data(const string& name, shared_ptr<tmpl_object>& obj){
	tmpl_data::iterator data_itor = this->data_.find(name);
	if (data_itor == this->data_.end()){
		this->data_.insert(move(make_pair(name, obj)));
	}
	else{
		data_itor->second = obj;
	}
}

void tmpl_out_statement::execute(tmpl_context& context){
	context.response().write(context.data(this->variable_));
}

void tmpl_composite_statement::execute(tmpl_context& context){
	std::for_each(this->statements_.begin(), this->statements_.end(), [&context](std::shared_ptr<tmpl_statement>& stmt) -> void {
		stmt->execute(context);
	});
}

void tmpl_if_statement::execute(tmpl_context& context){
	string value = context.data(this->variable_);
	bool result = false;
        float tmp_value = 0.0f, tmp_value1 = 0.0f;
	switch (this->op_)
	{
	case comp_op::eq:
		result = (strcasecmp(value.c_str(), this->value_.c_str()) == 0);
		break;
	case comp_op::neq:
		result = (strcasecmp(value.c_str(), this->value_.c_str()) != 0);
		break;
	case comp_op::ge:
                sscanf(value.c_str(), "%f", &tmp_value);
                sscanf(this->value_.c_str(), "%f", &tmp_value1);
		result = tmp_value >= tmp_value1;
		break;
	case comp_op::gt:
		sscanf(value.c_str(), "%f", &tmp_value);
                sscanf(this->value_.c_str(), "%f", &tmp_value1);
		result = tmp_value > tmp_value1;
		break;
	case comp_op::le:
		sscanf(value.c_str(), "%f", &tmp_value);
                sscanf(this->value_.c_str(), "%f", &tmp_value1);
		result = tmp_value <= tmp_value1;
		break;
	case comp_op::lt:
		sscanf(value.c_str(), "%f", &tmp_value);
                sscanf(this->value_.c_str(), "%f", &tmp_value1);
		result = tmp_value < tmp_value1;
		break;
	default:
		break;
	}

	if (result){
		if (this->statement_at_true_ != nullptr){
			this->statement_at_true_->execute(context);
		}
	}
	else{
		if (this->statement_at_false_ != nullptr){
			this->statement_at_false_->execute(context);
		}
	}
}

void tmpl_foreach_statement::execute(tmpl_context& context){
	shared_ptr<tmpl_object> obj = context.data_object(this->variable_);
	if (obj != nullptr && obj->is_collection()){
		obj->reset();
		do{
			shared_ptr<tmpl_object> item = obj->current();
			if (item != nullptr){
				context.set_data(this->item_name_, item);
				if (this->iteration_body_ != nullptr){
					this->iteration_body_->execute(context);
				}
			}
		} while (obj->next());
	}
}
