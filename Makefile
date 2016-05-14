LDFLAGS=jpeg-9a/.libs/libjpeg.a freetype-2.6/objs/.libs/libfreetype.a  -L/usr/lib:/usr/local/lib -lmysqlcppconn -lfcgi
CPPFLAGS=-I./freetype-2.6/include -Wall -std=c++0x -c
CXX=g++
objects=cgi_variables.o controllers.o data.o date_time.o html_tmpl.o http_file.o http_request.o http_response.o qryhelper.o sokoh.o string_tokenizer.o string_utilities.o tmpl_compiler.o buffered_image.o image_codec.o colorutil.o font.o graphics.o sha1.o
web=/var/www/html

%.o:%.cxx
	$(CXX) $(CFLAGS) $(CPPFLAGS) $<
%.o:%.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) $<

cgi_variables.o:cgi_variables.cxx cgi_variables.hxx
controllers.o:controllers.hxx controllers.cxx
data.o:data.cxx data.hxx
date_time.o:date_time.hxx date_time.cxx
html_tmpl.o:html_tmpl.hxx html_tmpl.cxx
http_file.o:http_file.hxx http_file.cxx
http_request.o:http_request.hxx http_request.cxx
http_response.o:http_response.hxx http_response.cxx
qryhelper.o:qryhelper.cxx qryhelper.hxx
sokoh.o:sokoh.cpp sokoh.hxx
string_tokenizer.o:string_tokenizer.cxx string_tokenizer.hxx
string_utilities.o:string_utilities.hxx string_utilities.cxx
tmpl_compiler.o:tmpl_compiler.cxx
buffered_image.o:buffered_image.cxx buffered_image.hxx
image_codec.o:image_codec.cxx image_codec.hxx
colorutil.o:colorutil.cxx colorutil.hxx
font.o:font.cxx font.hxx
graphics.o:graphics.cxx graphics.hxx
sha1.o:sha1.cxx sha1.hxx
sokoh.fcgi:$(objects)
	$(CXX) -o $@ $^ $(LDFLAGS)

jpeg-9a/.libs/libjpeg.a:jpegsrc.v9a.tar.gz
	(tar -xf jpegsrc.v9a.tar.gz && cd jpeg-9a && ./configure && $(MAKE) all)
    
freetype-2.6/objs/.libs/libfreetype.a:freetype-2.6.tar.gz
	(tar -xf freetype-2.6.tar.gz && cd freetype-2.6 && ./configure && $(MAKE) all)

all:jpeg-9a/.libs/libjpeg.a freetype-2.6/objs/.libs/libfreetype.a test sokoh.fcgi
test:
	(cd Test && $(MAKE) && ./test)
clean:
	-rm -rf jpeg-9a
	-rm -rf freetype-2.6
	-(cd Test && $(MAKE) clean)
	-rm -f *.o 2>/dev/null
	-rm -f sokoh.fcgi 2>/dev/null

install:all
	@cp -f sokoh.fcgi $(web)/
	@cp -Rf scripts $(web)/
	@cp -Rf style $(web)/
	@cp -Rf tmpl $(web)/
	@cp -Rf images $(web)/
	@cp -f default.php $(web)/
	@-mkdir $(web)/media
	@-mkdir $(web)/media/puzzles
	@-mkdir $(web)/tmp
	@chmod 777 $(web)/media
	@chmod 777 $(web)/media/puzzles
	@chmod 777 $(web)/tmp
	@cp -f IndieFlower.ttf $(web)/
