INCLUDE = -I/usr/include/mysql \
	-I/opt/third_party/soci/include

LIB = -L/opt/third_party/soci/lib \
	  -lsoci_core -lsoci_mysql

RPATH = /opt/third_party/soci/lib
all: test_orm test
test_orm:test_orm.cpp
	g++ $^ -o $@ $(INCLUDE) $(LIB) -Wl,-rpath,$(RPATH) -g -O0
test:test.cpp
	g++ $^ -o $@ $(INCLUDE) $(LIB) -Wl,-rpath,$(RPATH) -g -O0

clean:
	rm -f test test_orm
