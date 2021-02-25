
class SocketServer{
	public:
		SocketServer();
		~SocketServer();
		int port;
		int socker_accept;
	private:
		int socket_server_init();

}
