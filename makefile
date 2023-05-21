CXX ?= g++

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2

endif

lib_path = -L /usr/local/mysql/lib/
header_path = -I /usr/local/mysql/include/
CFLAGS=$(lib_path) $(header_path)

server: main.cpp  ./timer/lst_timer.cpp ./http/http_conn.cpp ./log/log.cpp ./CGImysql/sql_connection_pool.cpp  webserver.cpp config.cpp
	$(CXX) $(CFLAGS) -o server  $^ $(CXXFLAGS) -lpthread -lmysqlclient

	
clean:
	rm  -r server
