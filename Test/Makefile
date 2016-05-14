LDFLAGS=../jpeg-9a/.libs/libjpeg.a -L/usr/lib:/usr/local/lib -lmysqlcppconn
CPPFLAGS=-Wall -std=c++0x -DUNIT_TEST -c
CXX=g++
objects=cgi_variables.o data.o date_time.o html_tmpl.o http_file.o qryhelper.o string_tokenizer.o string_utilities.o tmpl_compiler.o test.o buffered_image.o image_codec.o

%.o:%.cxx
	$(CXX) $(CFLAGS) $(CPPFLAGS) $<
%.o:%.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) $<
%.o:../%.cxx
	$(CXX) $(CFLAGS) $(CPPFLAGS) $<
all:test
test:$(objects)
	$(CXX) -o $@ $^ $(LDFLAGS)

cgi_variables.o:../cgi_variables.cxx ../cgi_variables.hxx
data.o:../data.cxx ../data.hxx
date_time.o:../date_time.hxx ../date_time.cxx
html_tmpl.o:../html_tmpl.hxx ../html_tmpl.cxx
buffered_image.o: ../buffered_image.cxx ../buffered_image.hxx
image_codec.o: ../image_codec.cxx ../image_codec.hxx
http_file.o:../http_file.hxx ../http_file.cxx
qryhelper.o:../qryhelper.cxx ../qryhelper.hxx
string_tokenizer.o:../string_tokenizer.cxx ../string_tokenizer.hxx
string_utilities.o:../string_utilities.hxx ../string_utilities.cxx
tmpl_compiler.o:../tmpl_compiler.cxx
test.o:test.cxx test_includes.hxx

clean:
	-rm -f *.o
	-rm -f test
	-rm -f test-result.jpg
	-rm -f test-resize-result.jpg
