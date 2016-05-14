#include "sokoh.hxx"

using namespace std;

enum token_type{
	literal,
	out_stmt,
	if_stmt,
	else_stmt,
	foreach_stmt
};

struct token{
	token_type token_type_;
	shared_ptr<tmpl_statement> statement_;
};

int line = 1;
int column = 0;

inline bool valid_var_first_char(char ch){
	return ((ch <= 'Z' && ch >= 'A') || (ch <= 'z' && ch >= 'a') || ch == '_');
}

inline bool valid_var_char(char ch){
	return (ch <= '9' && ch >= '0') || ch == '.' || valid_var_first_char(ch) ;
}

class state{
public:
	virtual shared_ptr<state> execute(ifstream& tmpl_file, string& input_buffer, stack<shared_ptr<token> >& token_stack) = 0;
};

class error_state : public state{
public:
	error_state(const string& error)
		: error_(error) {
	}

public:
	virtual shared_ptr<state> execute(ifstream& tmpl_file, string& input_buffer, stack<shared_ptr<token> >& token_stack){
		stringstream error_stream;
		error_stream << "<b>ERROR: " << this->error_ << "@ line: " << line << ", column: " << column << "</b> <br />" << endl;
		error_stream << input_buffer << "_";

		while (!token_stack.empty()){
			token_stack.pop();
		}

		token* error_token = new token();
		error_token->token_type_ = token_type::literal;
		error_token->statement_ = move(shared_ptr<tmpl_statement>(new tmpl_literal_statement(error_stream.str())));

		token_stack.push(move(shared_ptr<token>(error_token)));

		return shared_ptr<state>(nullptr);
	}

private:
	string error_;
};

class literal_state : public state{
public:
	virtual shared_ptr<state> execute(ifstream& tmpl_file, string& input_buffer, stack<shared_ptr<token> >& token_stack);
};

class code_start_state : public state{
public:
	virtual shared_ptr<state> execute(ifstream& tmpl_file, string& input_buffer, stack<shared_ptr<token> >& token_stack);
};

class out_stmt_state : public state{
public:
	virtual shared_ptr<state> execute(ifstream& tmpl_file, string& input_buffer, stack<shared_ptr<token> >& token_stack);
};

class if_stmt_state : public state{
public:
	virtual shared_ptr<state> execute(ifstream& tmpl_file, string& input_buffer, stack<shared_ptr<token> >& token_stack);
};

class else_stmt_state : public state{
public:
	virtual shared_ptr<state> execute(ifstream& tmpl_file, string& input_buffer, stack<shared_ptr<token> >& token_stack);
};

class foreach_stmt_state : public state{
public:
	virtual shared_ptr<state> execute(ifstream& tmpl_file, string& input_buffer, stack<shared_ptr<token> >& token_stack);
};

shared_ptr<state> literal_state::execute(ifstream& tmpl_file, string& input_buffer, stack<shared_ptr<token> >& token_stack){
	bool escaped = false;
	bool close_escaped = false;
	while (tmpl_file.good()){
		char ch = tmpl_file.get();
		char top = '\0';
		if (!input_buffer.empty()){
			top = input_buffer[input_buffer.length() - 1];
		}

		++column;
		switch (ch){
		case -1:
			break;
		case '{':
			if (top != '{'){
				escaped = false;
				input_buffer.push_back(ch);
			}
			else{
				escaped = !escaped;
			}

			break;
		case '}':
			if (top != '}'){
				close_escaped = false;
				input_buffer.push_back(ch);
			}
			else{
				close_escaped = !close_escaped;
			}

			break;
		case '\n':
			++line;
			column = 0;
		default:
			if (top == '{' && !escaped){
				string buffer = input_buffer.substr(0, input_buffer.length() - 1);
				input_buffer.erase(input_buffer.begin(), input_buffer.end());
				token* literal_token = new token;
				literal_token->token_type_ = token_type::literal;
				literal_token->statement_ = move(shared_ptr<tmpl_statement>(new tmpl_literal_statement(buffer)));
				token_stack.push(move(shared_ptr<token>(literal_token)));
				input_buffer.push_back(ch);


				return move(shared_ptr<state>(new code_start_state()));
			}

			if (top == '}' && !close_escaped){
				return move(shared_ptr<state>(new error_state("expecting } symbol after " + input_buffer)));
			}

			input_buffer.push_back(ch);
		}
	}

	if (input_buffer.length() > 0){
		token* literal_token = new token;
		literal_token->token_type_ = token_type::literal;
		literal_token->statement_ = move(shared_ptr<tmpl_statement>(new tmpl_literal_statement(input_buffer)));
		token_stack.push(move(shared_ptr<token>(literal_token)));
	}

	return shared_ptr<state>(nullptr);
}

shared_ptr<state> code_start_state::execute(ifstream& tmpl_file, string& input_buffer, stack<shared_ptr<token> >& token_stack){
	char top = '\0';
	if (!input_buffer.empty()){
		top = input_buffer[input_buffer.length() - 1];
	}

	if (top == '$'){
		return move(shared_ptr<state>(new out_stmt_state()));
	}

	bool tag_close = top == '/';
	while (tmpl_file.good()){
		char ch = tmpl_file.get();
		++column;
		switch (ch){
		case '\n':
			++line;
			column = 0;
		case ' ':
		case '\t':
		case '\r':
			if (input_buffer == "if"){
				return move(shared_ptr<state>(new if_stmt_state()));
			}
			else if (input_buffer == "foreach"){
				return move(shared_ptr<state>(new foreach_stmt_state()));
			}
			else{
				return move(shared_ptr<state>(new error_state("invalid statement " + input_buffer)));
			}

		case '}':
			if (tag_close){
				stack<shared_ptr<tmpl_statement> > sub_statements;
				if (input_buffer == "/if"){
					shared_ptr<tmpl_composite_statement> comp_stmt(new tmpl_composite_statement());
					while (!token_stack.empty()
						&& (token_stack.top()->token_type_ != token_type::else_stmt
						&& token_stack.top()->token_type_ != token_type::if_stmt)){
						sub_statements.push(token_stack.top()->statement_);
						token_stack.pop();
					}

					while (!sub_statements.empty()){
						if (sub_statements.top() != nullptr){
							comp_stmt->add_statement(sub_statements.top());
						}

						sub_statements.pop();
					}

					if (token_stack.empty()){
						return move(shared_ptr<state>(new error_state("invalid close if tag as there is no open tag")));
					}

					if (token_stack.top()->token_type_ == token_type::else_stmt){
						shared_ptr<tmpl_composite_statement> f_stmt(move(comp_stmt));
						comp_stmt = move(shared_ptr<tmpl_composite_statement>(new tmpl_composite_statement));
						token_stack.pop();
						while (!token_stack.empty()
							&& token_stack.top()->token_type_ != token_type::if_stmt){

							if (token_stack.top()->token_type_ == token_type::else_stmt){
								return move(shared_ptr<state>(new error_state("multiple else tag for an if statement")));
							}

							sub_statements.push(token_stack.top()->statement_);
							token_stack.pop();
						}

						while (!sub_statements.empty()){
							if (sub_statements.top() != nullptr){
								comp_stmt->add_statement(sub_statements.top());
							}

							sub_statements.pop();
						}

						if (token_stack.empty()){
							return move(shared_ptr<state>(new error_state("invalid close if tag as there is no open tag")));
						}

						((tmpl_if_statement*)token_stack.top()->statement_.get())->set_statement_when_false(f_stmt);
					}

					((tmpl_if_statement*)token_stack.top()->statement_.get())->set_statement_when_true(comp_stmt);
					input_buffer.clear();
					return move(shared_ptr<state>(new literal_state()));
				}
				else if (input_buffer == "/foreach"){
					shared_ptr<tmpl_composite_statement> comp_stmt(new tmpl_composite_statement());
					while (!token_stack.empty()
						&& token_stack.top()->token_type_ != token_type::foreach_stmt){
						sub_statements.push(token_stack.top()->statement_);
						token_stack.pop();
					}

					while (!sub_statements.empty()){
						if (sub_statements.top() != nullptr){
							comp_stmt->add_statement(sub_statements.top());
						}

						sub_statements.pop();
					}

					if (token_stack.empty()){
						return move(shared_ptr<state>(new error_state("invalid close foreach tag as there is no open tag")));
					}

					((tmpl_foreach_statement*)token_stack.top()->statement_.get())->set_iteration_body(comp_stmt);
					input_buffer.clear();
					return move(shared_ptr<state>(new literal_state()));
				}

				return move(shared_ptr<state>(new error_state("invalid tag name " + input_buffer)));
			}

			if (input_buffer == "else"){
				token* else_token = new token;
				else_token->statement_ = shared_ptr<tmpl_statement>(nullptr);
				else_token->token_type_ = token_type::else_stmt;
				token_stack.push(move(shared_ptr<token>(else_token)));
				input_buffer.clear();
				return move(shared_ptr<state>(new literal_state()));
			}

			return move(shared_ptr<state>(new error_state("syntax error for " + input_buffer)));
		default:
			input_buffer.push_back(ch);
		}
	}

	return move(shared_ptr<state>(new error_state("invalid code snap " + input_buffer)));
}

shared_ptr<state> out_stmt_state::execute(ifstream& tmpl_file, string& input_buffer, stack<shared_ptr<token> >& token_stack){
	input_buffer.clear();
	bool space = false;
	while (tmpl_file.good()){
		char ch = tmpl_file.get();
		++column;
		switch (ch)
		{
		case '\n':
			++line;
			column = 0;
		case ' ':
		case '\t':
		case '\r':
			space = true;
		case '}':
			if (input_buffer.length() == 0){
				return move(shared_ptr<state>(new error_state("invalid out statement, variable name is expected.")));
			}

			if (ch == '}'){
				if ((input_buffer[0] <= '9' && input_buffer[0] >= '0') || input_buffer[0] == '.'){
					return move(shared_ptr<state>(new error_state("variable name cannot start with a number")));
				}

				token* out = new token();
				out->token_type_ = token_type::out_stmt;
				out->statement_ = move(shared_ptr<tmpl_statement>(new tmpl_out_statement(input_buffer)));
				token_stack.push(move(shared_ptr<token>(out)));

				input_buffer.clear();
				return move(shared_ptr<state>(new literal_state()));
			}

			break;
		default:
			if (!space && valid_var_char(ch)){
				input_buffer.push_back(ch);
			}
			else{
				return move(shared_ptr<state>(new error_state("invalid variable name")));
			}

			break;
		}
	}

	return move(shared_ptr<state>(new error_state("invalid out statement")));
}

bool is_valid_var_name(const string& var_name){
	if (var_name.length() < 1 || !valid_var_first_char(var_name[0])){
		return false;
	}

	for (size_t i = 1; i < var_name.length(); ++i){
		if (!valid_var_char(var_name[i])){
			return false;
		}
	}

	return true;
}

bool read_var_name(ifstream& tmpl_file, string& input_buffer, char* last_char){
	while (tmpl_file.good()){
		char ch = tmpl_file.get();
		++column;
		if (last_char != nullptr){
			*last_char = ch;
		}

		switch (ch){
		case '\n':
			++line;
			column = 0;
		case '\t':
		case ' ':
		case '\r':
		case '}':
			if (input_buffer.length() > 1 && input_buffer[0] == '$'){
				string var_name = input_buffer.substr(1);
				return is_valid_var_name(var_name);
			}
			
			if (ch == '}'){
				return false;
			}

			break;
		default:
			input_buffer.push_back(ch);
			break;
		}
	}

	if (input_buffer.length() > 1 && input_buffer[0] == '$'){
		string var_name = input_buffer.substr(1);
		return is_valid_var_name(var_name);
	}

	return false;
}

bool read_a_key_word(ifstream& tmpl_file, string& input_buffer){
	while (tmpl_file.good()){
		char ch = tmpl_file.get();
		++column;
		switch (ch){
		case '\n':
			++line;
			column = 0;
		case ' ':
		case '\t':
		case '\r':
		case '}':
			if (!input_buffer.empty()){
				return true;
			}

			if (ch == '}'){
				return false;
			}

			break;
		default:
			if (ch < 'a' || ch > 'z'){
				return false;
			}

			input_buffer.push_back(ch);
			break;
		}
	}

	return !input_buffer.empty();
}

bool read_a_string(ifstream& tmpl_file, string& input_buffer){
	bool started = false;
	bool escaping = false;
	while (tmpl_file.good()){
		char ch = tmpl_file.get();
		++column;
		switch (ch){
		case '"':
			if (!started){
				started = true;
			}
			else if (escaping){
				input_buffer.push_back(ch);
				escaping = false;
			}
			else{
				return true;
			}

			break;
		case '\n':
			++line;
			column = 0;
		case '\r':
			if (started){
				return false;
			}
		case '\t':
		case ' ':
			if (!started){
				break;
			}
		default:
			if (!started){
				return false;
			}
			
			if (ch == '\\' && !escaping){
				escaping = true;
				break;
			}

			if (escaping){
				if (ch == 'r'){
					ch = '\r';
				}
				else if (ch == 't'){
					ch = '\t';
				}
				else if (ch == 'n'){
					ch = '\n';
				}

				escaping = false;
			}

			input_buffer.push_back(ch);
			break;
		}
	}

	return started;
}

shared_ptr<state> if_stmt_state::execute(ifstream& tmpl_file, string& input_buffer, stack<shared_ptr<token> >& token_stack){
	input_buffer.clear();
	if (!read_var_name(tmpl_file, input_buffer, nullptr)){
		return move(shared_ptr<state>(new error_state("invalid variable name")));
	}

	string var_name = input_buffer.substr(1);

	input_buffer.clear();
	if (!read_a_key_word(tmpl_file, input_buffer)){
		return move(shared_ptr<state>(new error_state("no valid comparasion key word could be found for if statement")));
	}
	
	tmpl_if_statement::comp_op op;
	if (input_buffer == "eq"){
		op = tmpl_if_statement::comp_op::eq;
	}
	else if (input_buffer == "neq"){
		op = tmpl_if_statement::comp_op::neq;
	}
	else if (input_buffer == "gt"){
		op = tmpl_if_statement::comp_op::gt;
	}
	else if (input_buffer == "lt"){
		op = tmpl_if_statement::comp_op::lt;
	}
	else if (input_buffer == "ge"){
		op = tmpl_if_statement::comp_op::ge;
	}
	else if (input_buffer == "le"){
		op = tmpl_if_statement::comp_op::le;
	}
	else{
		return move(shared_ptr<state>(new error_state("invalid comparasion key word \"" + input_buffer + "\" for if statement")));
	}

	input_buffer.clear();
	if (!read_a_string(tmpl_file, input_buffer)){
		return move(shared_ptr<state>(new error_state("a value string is expected")));
	}

	token* if_token = nullptr;
	while (tmpl_file.good()){
		char ch = tmpl_file.get();
		++column;
		switch (ch){
		case '\n':
			++line;
			column = 0;
		case ' ':
		case '\t':
		case '\r':
			break;
		case '}':
			if_token = new token;
			if_token->statement_ = move(shared_ptr<tmpl_statement>(new tmpl_if_statement(var_name, op, input_buffer)));
			if_token->token_type_ = token_type::if_stmt;
			token_stack.push(move(shared_ptr<token>(if_token)));
			input_buffer.clear();
			return move(shared_ptr<state>(new literal_state()));

		default:
			return move(shared_ptr<state>(new error_state("unexpected token found")));
		}
	}

	return move(shared_ptr<state>(new error_state("incompleted if statement")));
}

shared_ptr<state> foreach_stmt_state::execute(ifstream& tmpl_file, string& input_buffer, stack<shared_ptr<token> >& token_stack){
	input_buffer.clear();
	if (!read_var_name(tmpl_file, input_buffer, nullptr)){
		return move(shared_ptr<state>(new error_state("invalid variable name")));
	}

	string item_name = input_buffer.substr(1);

	input_buffer.clear();
	if (!read_a_key_word(tmpl_file, input_buffer)){
		return move(shared_ptr<state>(new error_state("no valid key word could be found for foreach statement")));
	}

	if (input_buffer != "in"){
		return move(shared_ptr<state>(new error_state("invalid key word \"" + input_buffer + "\" for foreach statement")));
	}

	input_buffer.clear();
	char last_char = '\0';
	if (!read_var_name(tmpl_file, input_buffer, &last_char)){
		return move(shared_ptr<state>(new error_state("invalid variable name")));
	}

	string var_name = input_buffer.substr(1);
	token* for_token = nullptr;
	if (last_char == '}'){
		for_token = new token;
		for_token->statement_ = move(shared_ptr<tmpl_statement>(new tmpl_foreach_statement(var_name, item_name)));
		for_token->token_type_ = token_type::foreach_stmt;
		token_stack.push(move(shared_ptr<token>(for_token)));
		input_buffer.clear();
		return move(shared_ptr<state>(new literal_state()));
	}

	while (tmpl_file.good()){
		char ch = tmpl_file.get();
		++column;
		switch (ch){
		case '\n':
			++line;
			column = 0;
		case ' ':
		case '\t':
		case '\r':
			break;
		case '}':
			for_token = new token;
			for_token->statement_ = move(shared_ptr<tmpl_statement>(new tmpl_foreach_statement(var_name, item_name)));
			for_token->token_type_ = token_type::foreach_stmt;
			token_stack.push(move(shared_ptr<token>(for_token)));
			input_buffer.clear();
			return move(shared_ptr<state>(new literal_state()));

		default:
			return move(shared_ptr<state>(new error_state("unexpected token found")));
		}
	}

	return move(shared_ptr<state>(new error_state("incompleted foreach statement")));
}

shared_ptr<tmpl_composite_statement> compile_template(const string& file_name){
	ifstream tmpl_file(file_name, fstream::in);
	shared_ptr<state> current_state(new literal_state());
	string input_buffer;
	stack<shared_ptr<token> > token_stack;
	while (current_state != nullptr){
		current_state = current_state->execute(tmpl_file, input_buffer, token_stack);
	}

	shared_ptr<tmpl_composite_statement> results(new tmpl_composite_statement());
	list<shared_ptr<tmpl_statement> > stmt_list;
	while (!token_stack.empty()){
		stmt_list.push_back(move(token_stack.top()->statement_));
		token_stack.pop();
	}

	reverse(stmt_list.begin(), stmt_list.end());
	for (list<shared_ptr<tmpl_statement> >::iterator it = stmt_list.begin(); it != stmt_list.end(); ++it){
		results->add_statement(*it);
	}

	return move(results);
}
