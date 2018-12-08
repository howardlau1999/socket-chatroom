server: server_main.cpp ChatRoom.cpp User.cpp Server.cpp Protocol.cpp Socket.cpp
	@mkdir -p bin
	g++ -o bin/server server_main.cpp ChatRoom.cpp User.cpp Server.cpp Protocol.cpp Socket.cpp -lpthread
client: client_main.cpp ChatRoom.cpp User.cpp Server.cpp Protocol.cpp Socket.cpp
	@mkdir -p bin
	g++ -o bin/client client_main.cpp ChatRoom.cpp User.cpp Server.cpp Protocol.cpp Socket.cpp -lpthread 