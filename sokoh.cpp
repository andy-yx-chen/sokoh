// sokoh.cpp : Defines the entry point for the application.
//

#include "sokoh.hxx"

#ifdef RELEASE_TO_AZURE
#define DB_SERVER "<mysql-db-server>"
#define DB_USER "<db-user>"
#define DB_PASSWORD "<db-password>"
#else
#define DB_SERVER "<mysql-db-server>"
#define DB_USER "<db-user>"
#define DB_PASSWORD "<db-password>"
#endif

int handle_request(http_request* context);
bool is_number(const std::string& value);

int main(int argc, char* argv[])
{
	FCGX_Init();
	FCGX_Request req = { 0 };
	if (FCGX_InitRequest(&req, 0, 0) < 0){
		return 0;
	}

	while (FCGX_Accept_r(&req) >= 0)
	{
		handle_request(new http_request(&req));
	}

	return 0;
}

int handle_request(http_request* context){
	std::shared_ptr<http_request> request(context);
	std::shared_ptr<http_response> response(std::move(request->response()));
	std::string script_name = request->request_data("c");

	if (script_name == "debug"){
		response->set_header("Content-Type", "text/html;charset=utf-8");
		request->dump_all(response.get());
		response->done();
		return 0;
	}

	try{
		init_db_connection(DB_SERVER, "datatech", DB_USER, DB_PASSWORD);
		std::shared_ptr<controller> ctrlr(nullptr);

		if (is_number(script_name)){
			request->set_attribute("topic_id", script_name);
			ctrlr.reset(new topic_controller());
		}
		else if (script_name == "index" || script_name == http_request::empty_string){
			ctrlr.reset(new home_controller());
		}else if(script_name == "filemgr"){
		  ctrlr.reset(new file_manager_controller());
		}
		else{
			size_t pos = script_name.find_first_of('_');
			if (pos != std::string::npos){
				std::string prefix(script_name.substr(0, pos));
				std::string object_id(script_name.substr(pos + 1));
				if (prefix == "a" && is_number(object_id)){
					request->set_attribute("article_id", object_id);
					ctrlr.reset(new article_controller());
				}
				else if (prefix == "c" && is_number(object_id)){
					request->set_attribute("comment_id", object_id);
					ctrlr.reset(new comment_controller());
				}
			}
			else{
				// call manage controller
				request->set_attribute("command", script_name);
				ctrlr.reset(new manage_controller());
			}
		}

		if (ctrlr == nullptr){
			response->set_status(400, "Not Found");
			response->write("not controller could handle script: " + script_name);
		}
		else{
			ctrlr->execute(*request, *response);
		}

		release_db_connection();
	}
	catch (sql::SQLException& exception){
		response->set_status(500, "Internal Server Error");
		response->write("<b>SQL error: </b>");
		response->write(exception.what());
		release_db_connection();
	}

	response->done();
	return 0;
}

bool is_number(const std::string& value){
	if (value.length() == 0){
		return false;
	}

	for (size_t i = 0; i < value.length(); ++i){
		if (value[i] > '9' || value[i] < '0'){
			return false;
		}
	}

	return true;
}
