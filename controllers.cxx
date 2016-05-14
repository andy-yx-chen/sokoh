#include "sokoh.hxx"


typedef std::shared_ptr<std::list<std::shared_ptr<topic> > > topic_list;
typedef std::shared_ptr<std::list<std::shared_ptr<article> > > article_list;
typedef std::shared_ptr<std::list<std::shared_ptr<comment> > > comment_list;

bool file_exists(const std::string& path);

controller::tmpl_cache_type controller::cache_templates_;

void controller::execute(http_request& request, http_response& response){
	tmpl_data data;
	tmpl_context context(request, response, data);
	http_request::request_method method = request.method();
	std::string view;
	switch (method)
	{
	case http_request::http_delete:
		view = std::move(this->del(context));
		break;
	case http_request::http_get:
		view = std::move(this->get(context));
		break;
	case http_request::http_post:
		view = std::move(this->post(context));
		break;
	case http_request::http_put:
		view = std::move(this->put(context));
	default:
		break;
	}

	if (view.length() > 0){
		std::string path(std::move(request.physical_path(view)));
		tmpl_iterator itor = cache_templates_.find(view);
		if (itor != cache_templates_.end()){
			itor->second->execute(context);
		}
		else{
			if (!file_exists(path)){
				context.response().write("<b>template " + view + " not found.</b>");
			}
			else{
				std::shared_ptr<tmpl_composite_statement> tmpl = compile_template(path);
				cache_templates_.insert(std::make_pair(view, tmpl));
				tmpl->execute(context);
			}
		}
	}
}

std::shared_ptr<user> controller::authenticate(http_request& request){
	std::string auth_info = request.header("Authorization");
	if (auth_info.length() > 0){
		trim(auth_info, " ");
		size_t pos = auth_info.find_first_of(' ');
		if (pos != std::string::npos){
			std::string cred = base64_decode(auth_info.substr(pos + 1));
			if (cred.length() > 0){
				pos = cred.find_first_of(':');
				if (pos != std::string::npos)
				{
					std::shared_ptr<std::list<std::shared_ptr<user> > > users = query_objects<user>(Property("alias") == cred.substr(0, pos) && Property("code") == cred.substr(pos + 1));
					if (users->size() > 0){
						return users->front();
					}
				}
			}
		}
	}

	return nullptr;
}

std::string controller::get(tmpl_context& context){
	context.response().set_status(501, "Not Implemented");
	return std::move(std::string(""));
}

std::string controller::post(tmpl_context& context){
	context.response().set_status(501, "Not Implemented");
	return std::move(std::string(""));
}

std::string controller::del(tmpl_context& context){
	context.response().set_status(501, "Not Implemented");
	return std::move(std::string(""));
}

std::string controller::put(tmpl_context& context){
	context.response().set_status(501, "Not Implemented");
	return std::move(std::string(""));
}

std::string home_controller::get(tmpl_context& context){
	topic_list list = query_objects<topic>(nullptr);
        std::shared_ptr<tmpl_object> list_obj(new tmpl_collection_object<std::list<std::shared_ptr<topic> > >(*list));
	context.set_data("list", list_obj);
	context.response().set_status(200, "OK");
	context.response().set_header("Content-Type", "text/html;charset=utf-8");
	return "tmpl/index.html";
}

std::string home_controller::post(tmpl_context& context){
	std::shared_ptr<user> user = this->authenticate(context.request());
	if (user == nullptr){
		context.response().set_status(401, "Unauthorized");
		context.response().set_header("WWW-Authenticate", "Basic realm=\"AndyTech\"");
	}
	else{
		std::string title = context.request().request_data("title");
		std::string description = context.request().request_data("description");
		if (title.length() > 0 && description.length() > 0){
			try{
				date_time time;
				std::shared_ptr<sql::PreparedStatement> stmt(get_db_connection()->prepareStatement("INSERT INTO topic(title, launched_time, description) values(?, ?, ?)"));
				stmt->setString(1, title);
				stmt->setString(2, time.text());
				stmt->setString(3, description);
				stmt->execute();
				context.response().set_status(201, "Created");
				context.response().set_header("Content-Type", "text/html;charset=utf-8");
				context.response().write("topic is created");
			}
			catch (sql::SQLException& exception){
				context.response().set_status(500, "Internal Server Error");
				context.response().write(exception.what());
			}
		}
		else{
			context.response().set_status(400, "Bad Request");
			context.response().write("neither title nor description can be null.");
		}
	}

	return "";
}

bool file_exists(const std::string& path){
        FILE* file = fopen(path.c_str(), "r");
        if(file == nullptr){
                return errno != ENOENT;
        }
      
        fclose(file);
        return true;
}

std::string topic_controller::get(tmpl_context& context){
	int topic_id = atoi(context.request().get_attribute("topic_id").c_str());
	if (topic_id > 0){
		topic_list topics = query_objects<topic>(Property("id") == topic_id);
		if (topics->size() > 0){
                        std::shared_ptr<tmpl_object> topic_obj(new tmpl_value_object<std::shared_ptr<topic>>(topics->front()));
			context.set_data("topic", topic_obj);
			article_list articles = query_objects<article>(Property("topic_id") == topic_id);
                        std::shared_ptr<tmpl_object> articles_obj(new tmpl_collection_object<std::list<std::shared_ptr<article> > >(*articles));
			context.set_data("articles", articles_obj);
			context.response().set_status(200, "OK");
			context.response().set_header("Content-Type", "text/html;charset=utf-8");
			return "tmpl/topic.html";
		}
	}

	context.response().set_status(404, "Not Found");
	return "";
}

std::string topic_controller::post(tmpl_context& context){
	std::shared_ptr<user> author = this->authenticate(context.request());
	if (author == nullptr){
		context.response().set_status(401, "Unauthorized");
		context.response().set_header("WWW-Authenticate", "Basic realm=\"AndyTech\"");
	}
	else{
		std::string topic_id = context.request().get_attribute("topic_id");
		std::string title = context.request().request_data("title");
		std::string content = context.request().request_data("content");
		if (topic_id.length() > 0 && title.length() > 0 && content.length() > 0){
			date_time now;
			std::shared_ptr<sql::PreparedStatement> stmt(get_db_connection()->prepareStatement("INSERT INTO article(topic_id, title, author, published_time, content) VALUES(?, ?, ?, ?, ?)"));
			stmt->setInt(1, atoi(topic_id.c_str()));
			stmt->setString(2, title);
			stmt->setString(3, author->get_first_name() + " " + author->get_last_name());
			stmt->setString(4, now.text());
			stmt->setString(5, content);
			stmt->execute();
			context.response().set_status(201, "Created");
			context.response().set_header("Content-Type", "text/html;charset=utf-8");
			context.response().write("article is created");
		}
		else{
			context.response().set_status(400, "Bad Request");
			context.response().write("none of topic_id, title or description can be null.");
		}
	}

	return "";
}

std::string topic_controller::del(tmpl_context& context){
	std::shared_ptr<user> author = this->authenticate(context.request());
	if (author == nullptr){
		context.response().set_status(401, "Unauthorized");
		context.response().set_header("WWW-Authenticate", "Basic realm=\"AndyTech\"");
	}
	else{
		int topic_id = atoi(context.request().get_attribute("topic_id").c_str());
		std::shared_ptr<sql::PreparedStatement> stmt(get_db_connection()->prepareStatement("DELETE FROM topic WHERE id=?"));
		stmt->setInt(1, topic_id);
		stmt->execute();
		context.response().set_status(200, "OK");
		context.response().set_header("Content-Type", "text/html;charset=utf-8");
		context.response().write("topic is deleted");
	}

	return "";
}

std::string article_controller::get(tmpl_context& context){
	int article_id = atoi(context.request().get_attribute("article_id").c_str());
	if (article_id > 0){
		article_list articles = query_objects<article>(Property("id") == article_id);
		if (articles->size() > 0){
                        std::shared_ptr<tmpl_object> article_obj(new tmpl_value_object<std::shared_ptr<article> >(articles->front()));
			context.set_data("article", article_obj);
			comment_list comments = query_objects<comment>(Property("article_id") == article_id);
                        std::shared_ptr<tmpl_object> comments_obj(new tmpl_collection_object<std::list<std::shared_ptr<comment> > >(*comments));
			context.set_data("comments", comments_obj);
            
            // generate a random puzzle
            std::string client_ip = context.request().remote_addr();
            string_tokenizer tokenizer(client_ip, ".");
            unsigned int ip_value = 0;
            while(tokenizer.has_more()){
                size_t len(0);
                const char* str_start = tokenizer.next(len);
                std::string value(str_start, str_start + len);
                ip_value <<= 8;
                ip_value |= atoi(value.c_str());
            }
            
            unsigned int seed = ip_value + (unsigned int)time(NULL);
            srand(seed);
            int v1 = (rand() % 10 + 1) * 1000;
            int v2 = rand() % 500;
            int result = v1 - v2;
            char buf[20];
            sprintf(buf, "%d", result);
            client_ip += "&this-is-a^key";
            client_ip += buf;
            
            unsigned char sha1_bytes[20];
            char sha1_str[50];
            sha1::calc(client_ip.c_str(), client_ip.length(), sha1_bytes);
            sha1::toHexString(sha1_bytes, sha1_str);
            std::string answer(sha1_str);
            context.set_value<std::string>("answer", answer);
            
            std::string image_location("media/puzzles/");
            image_location.append(answer);
            image_location.append(".jpg");
            context.set_value<std::string>("puzzle", image_location);
            
            char ex_buffer[50];
            sprintf(ex_buffer, "%d-%d=", v1, v2);
            std::wstring expression;
            for(size_t i = 0; i < strlen(ex_buffer); ++i){
                expression += wchar_t(ex_buffer[i]);
            }
            
            // generate the image and save
            std::string filename = context.request().physical_path("media/puzzles/");
            filename.append(answer);
            filename.append(".jpg");
            std::string font_file = context.request().physical_path("IndieFlower.ttf");
            std::shared_ptr<buffered_image> image(new buffered_image(expression.length() * 14 + 8, 26));
            graphics g(*image);
            g.paint_background((argb_t)(255 << 16 | 255 << 8 | 255));
            g.load_font("default", font_file);
            g.draw_text(4, 4, expression, "default", 18);
            jpeg_image_codec codec(100);
            codec.encode(image, filename);
			context.response().set_status(200, "OK");
			context.response().set_header("Content-Type", "text/html;charset=utf-8");
			return "tmpl/article.html";
		}
	}

	context.response().set_status(404, "Not Found");
	return "";
}

std::string article_controller::post(tmpl_context& context){
	int article_id = atoi(context.request().get_attribute("article_id").c_str());
	if (article_id > 0){
		std::string title = context.request().request_data("title");
		std::string author = context.request().request_data("visitor");
		std::string content = context.request().request_data("content");
        
        // check the answer for the puzzle
        std::string answer = context.request().request_data("answer");
        std::string puzzle = context.request().request_data("puzzle");
        std::string client_ip = context.request().remote_addr();
        client_ip += "&this-is-a^key";
        client_ip += answer;
        unsigned char sha1_bytes[20];
        char sha1_str[50];
        sha1::calc(client_ip.c_str(), client_ip.length(), sha1_bytes);
        sha1::toHexString(sha1_bytes, sha1_str);
        std::string test_answer(sha1_str);
        
        // remove the image file
        std::string filename = context.request().physical_path("media/");
        filename.append(puzzle);
        filename.append(".jpg");
        unlink(filename.c_str());
        if(test_answer != puzzle){
            context.response().set_status(400, "Bad Request");
			context.response().write("Answer for the puzzle is incorrect");
            return "";
        }
        
		size_t pos = content.find('<');
		if (pos == std::string::npos){
			pos = content.find('>');
		}

		if (pos == std::string::npos){
			pos = title.find('>');
		}

		if (pos == std::string::npos){
			pos = title.find('<');
		}

		if (pos == std::string::npos){
			pos = author.find('>');
		}

		if (pos == std::string::npos){
			pos = author.find('<');
		}

		if (pos == std::string::npos && title.length() > 0 && author.length() > 0 && content.length() > 0){
			date_time now;
			std::shared_ptr<sql::PreparedStatement> stmt(get_db_connection()->prepareStatement("INSERT INTO comment(article_id, time_, author, title, content) VALUES(?, ?, ?, ?, ?)"));
			stmt->setInt(1, article_id);
			stmt->setString(2, now.text());
			stmt->setString(3, author);
			stmt->setString(4, title);
			stmt->setString(5, content);
			stmt->execute();
			context.response().set_header("Content-Type", "text/html;charset=utf-8");
			context.response().set_status(201, "Created");
			return "tmpl/comment_done.html";
		}
		else{
			context.response().set_status(400, "Bad Request");
			context.response().write("none of article_id, title, author or content can be null.");
		}
	}
	else{
		context.response().set_status(404, "Not Found");
	}

	return "";
}

std::string article_controller::del(tmpl_context& context){
	std::shared_ptr<user> user = this->authenticate(context.request());
	if (user == nullptr){
		context.response().set_status(401, "Unauthorized");
		context.response().set_header("WWW-Authenticate", "Basic realm=\"AndyTech\"");
	}
	else{
		int article_id = atoi(context.request().get_attribute("article_id").c_str());
		std::shared_ptr<sql::PreparedStatement> stmt(get_db_connection()->prepareStatement("DELETE FROM article WHERE id=?"));
		stmt->setInt(1, article_id);
		stmt->execute();
		context.response().set_status(200, "OK");
		context.response().set_header("Content-Type", "text/html;charset=utf-8");
		context.response().write("article is deleted");
	}

	return "";
}

std::string article_controller::put(tmpl_context& context){
	std::shared_ptr<user> author = this->authenticate(context.request());
	if (author == nullptr){
		context.response().set_status(401, "Unauthorized");
		context.response().set_header("WWW-Authenticate", "Basic realm=\"AndyTech\"");
	}
	else{
		std::string article_id = context.request().get_attribute("article_id");
		std::string title = context.request().request_data("title");
		std::string content = context.request().request_data("content");
		if (article_id.length() > 0 && title.length() > 0 && content.length() > 0){
			date_time now;
			std::shared_ptr<sql::PreparedStatement> stmt(get_db_connection()->prepareStatement("UPDATE article SET title=?, content=? WHERE id=?"));
			stmt->setString(1, title);
			stmt->setString(2, content);
			stmt->setInt(3, atoi(article_id.c_str()));
			stmt->execute();
			context.response().set_status(200, "OK");
			context.response().set_header("Content-Type", "text/html;charset=utf-8");
			context.response().write("article is updated");
		}
		else{
			context.response().set_status(400, "Bad Request");
			context.response().write("none of article_id, title or description can be null.");
		}
	}

	return "";
}

std::string comment_controller::del(tmpl_context& context){
	std::shared_ptr<user> user = this->authenticate(context.request());
	if (user == nullptr){
		context.response().set_status(401, "Unauthorized");
		context.response().set_header("WWW-Authenticate", "Basic realm=\"AndyTech\"");
	}
	else{
		int comment_id = atoi(context.request().get_attribute("comment_id").c_str());
		std::shared_ptr<sql::PreparedStatement> stmt(get_db_connection()->prepareStatement("DELETE FROM comment WHERE id=?"));
		stmt->setInt(1, comment_id);
		stmt->execute();
		context.response().set_status(200, "OK");
		context.response().set_header("Content-Type", "text/html;charset=utf-8");
		context.response().write("comment is deleted");
	}

	return "";
}

std::string manage_controller::get(tmpl_context& context){
	std::shared_ptr<user> user = this->authenticate(context.request());
	if (user == nullptr){
		context.response().set_status(401, "Unauthorized");
		context.response().set_header("WWW-Authenticate", "Basic realm=\"AndyTech\"");
	}
	else{
		std::string command = context.request().get_attribute("command");
		if (command == "article"){
			topic_list topics = query_objects<topic>(nullptr);
                        std::shared_ptr<tmpl_object> topics_obj(new tmpl_collection_object<std::list<std::shared_ptr<topic> > >(*topics));
			context.set_data("topics", topics_obj);
			context.response().set_status(200, "OK");
			context.response().set_header("Content-Type", "text/html;charset=utf-8");
			return "tmpl/add_article.html";
		}
		else if (command == "topic"){
			context.response().set_status(200, "OK");
			context.response().set_header("Content-Type", "text/html;charset=utf-8");
			return "tmpl/add_topic.html";
		}else if(command == "console"){
		  context.response().set_status(200, "OK");
		  context.response().set_header("Content-Type", "text/html;charset=utf-8");
		  return "tmpl/console.html";
		}

		context.response().set_status(404, "Not Found");
	}

	return "";
}

std::string file_manager_controller::get(tmpl_context& context){
  std::shared_ptr<user> user = this->authenticate(context.request());
  if(user == nullptr){
    context.response().set_status(401, "Unauthorized");
    context.response().set_header("WWW-Authenticate", "Basic realm=\"AndyTech\"");
  }else{
    context.response().set_status(200, "OK");
    context.response().set_header("Content-Type", "text/html;charset=utf-8");
    std::list<std::string> dirs;
    std::string container(std::move(context.request().physical_path("media")));
    DIR* dir = opendir(container.c_str());
    if(dir == NULL){
      if(errno == ENOENT && mkdir(container.c_str(), 777) < 0){
		std::shared_ptr<tmpl_object> error(new tmpl_value_object<std::string>("media container cannot be found nor created."));
		context.set_data("error", error);
		return "tmpl/file_manager.html";
      }

      // try it again, if it's created
      dir = opendir(container.c_str());
      if(dir == NULL){
		std::shared_ptr<tmpl_object> error(new tmpl_value_object<std::string>("media container cannot be accessed"));
		context.set_data("error", error);
		return "tmpl/file_manager.html";
      }
    }

    dirent* ent = NULL;
    while((ent = readdir(dir)) != NULL){
      if(strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0){
	continue;
      }

      dirs.push_back(ent->d_name);
    }

    closedir(dir);
    std::shared_ptr<tmpl_object> dirs_obj(new tmpl_collection_object<std::list<std::string> >(dirs));
    context.set_data("dirs", dirs_obj);
    return "tmpl/file_manager.html";
  }

  return "";
}

std::string file_manager_controller::post(tmpl_context& context){
  std::shared_ptr<user> user = this->authenticate(context.request());
  if(user == nullptr){
    context.response().set_status(401, "Unauthorized");
    context.response().set_header("WWW-Authenticate", "Basic realm=\"AndyTech\"");
    return "";
  }

  std::string command = context.request().request_data("cmd");
  std::string container = context.request().physical_path("media");
  if(command == "mkdir"){
    std::string dir_name = context.request().request_data("name");
    std::string full_path = container + "/" + dir_name;
    std::string message = dir_name + " is created successfully";
    if(mkdir(full_path.c_str(), 0777) < 0){
      message = "failed to create directory, please contact your administrator";
    }
    std::shared_ptr<tmpl_object> err (new tmpl_value_object<std::string>(message));
    context.set_data("error", err);
	context.response().set_status(200, "OK");
	context.response().set_header("Content-Type", "text/html;charset=utf-8");
    return "tmpl/file_manager.html";
  }else if(command == "upload"){
    std::shared_ptr<http_file> file = context.request().get_file("file");
    std::string message = "file is  successfully uploaded to ";
    if(file == nullptr){
      message = "no file could be seen";
    }else{
      std::string dir = context.request().request_data("dest");
      std::string dest;
      if(dir.length() > 0){
		dest += "/";
		dest += dir;
      }
      
      dest += "/" + file->name();
      message += "/media" + dest;
      dest = container + dest;
      std::string resize = context.request().request_data("resize");
      if(resize == "yes"){
	int width = atoi(context.request().request_data("width").c_str());
	int height = atoi(context.request().request_data("height").c_str());
	std::shared_ptr<image_codec> codec(new jpeg_image_codec(100));
	std::shared_ptr<buffered_image> src_img = codec->decode(file->tmp_path());
	std::shared_ptr<buffered_image> dest_img = src_img->resize(width, height);
	if(dest_img != nullptr){
	  codec->encode(dest_img, dest);
	}
      }else{
	file->move_to(dest);
      }
    }

    std::shared_ptr<tmpl_object> err(new tmpl_value_object<std::string>(message));
    context.set_data("error", err);
	context.response().set_status(200, "OK");
	context.response().set_header("Content-Type", "text/html;charset=utf-8");
    return "tmpl/file_manager.html";
  }
  
  context.response().set_status(404, "Not found");
  return "";
}
      
