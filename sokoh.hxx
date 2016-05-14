// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#ifndef SPARTA_HXX_
#define SPARTA_HXX_

#ifndef UNIT_TEST

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <string.h>
#include <math.h>

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <memory>
#include <unordered_map>
#include <list>
#include <stdexcept>
#include <vector>
#include <stack>
#include <algorithm>
#include <locale>

#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include <fcgio.h>
#include <fcgi_config.h>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "sha1.hxx"
#include "exception.hxx"
#include "cgi_variables.hxx"
#include "date_time.hxx"
#include "buffered_image.hxx"
#include "image_codec.hxx"
#include "colorutil.hxx"
#include "font.hxx"
#include "graphics.hxx"
#include "string_utilities.hxx"
#include "string_tokenizer.hxx"
#include "data.hxx"
#include "qryhelper.hxx"
#include "http_file.hxx"
#include "http_response.hxx"
#include "http_request.hxx"
#include "html_tmpl.hxx"
#include "controllers.hxx"

#else
#include "Test/test_includes.hxx"
#endif

#endif
