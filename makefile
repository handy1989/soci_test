INCLUDE = -I/usr/include/mysql \
	-I/opt/third_party/soci/include

LIB = -L/opt/third_party/soci/lib \
	  -lsoci_core -lsoci_mysql

RPATH = /opt/third_party/soci/lib
test2:test2.cpp
	g++ $^ -o $@ $(INCLUDE) $(LIB) -Wl,-rpath,$(RPATH) -g -O0
test:test.cpp
	g++ $^ -o $@ $(INCLUDE) $(LIB) -Wl,-rpath,$(RPATH) -g -O0
soci_test:soci_test.cpp
	g++ $^ -o $@ $(INCLUDE) $(LIB) -Wl,-rpath,$(RPATH) -g -O0

clean:
	rm -f soci_test
