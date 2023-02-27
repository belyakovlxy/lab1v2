#include "server.h"


int main()
{
	HttpServer httpServer;

	httpServer.init("127.0.0.1", "8000");
	httpServer.listen();
	while (true)
	{

		Socket client = httpServer.accept();

		
		const int maxSize = 1024;
		char buffer[maxSize];

		for (int i = 0; i < maxSize; i++)
		{
			buffer[i] = '\0';
		}

		int result = httpServer.recieve(client, buffer, maxSize, 0);
		httpServer.handleRequest(client, buffer, result);
	}

	return 0;
}

