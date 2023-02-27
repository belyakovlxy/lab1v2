#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <regex>

#define _WIN32_WINNT 0x501
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

class Socket
{
private:
	SOCKET m_socket;

public:
	Socket() : m_socket(0) {};
	Socket(SOCKET sock) 
	{
		m_socket = sock;
		sock = 0;
	};

	Socket(const Socket&) = delete;
	Socket(Socket&& x) noexcept
	{
		m_socket = x.m_socket;
		x.m_socket = 0;
	}


	int get()
	{
		return m_socket;
	}

	Socket operator=(const Socket& x) = delete;
	Socket & operator=(Socket&& x) noexcept
	{
		if (this != &x)
		{
			m_socket = x.m_socket;
			x.m_socket = 0;
		}
		return *this;
	}

	Socket& operator=(SOCKET&& x) noexcept
	{
		if (m_socket != x)
		{
			m_socket = x;
			x = 0;
		}
		return *this;
	}

	void close()
	{
		::closesocket(m_socket);
		m_socket = 0;
	}

	~Socket()
	{
		if (m_socket != 0)
		{
			::closesocket(m_socket);
			m_socket = 0;
		}
	}
	
	 
};

class Buffer
{
public:
	int m_bufferSize;
	char* m_buffer;

public:
	Buffer() : m_bufferSize(0), m_buffer(0) {};

	Buffer(int maxSize) : m_bufferSize(maxSize)
	{
		init(maxSize);
	}

	void init(int maxSize)
	{
		if (m_bufferSize != 0)
		{
			clear();
		}
		m_bufferSize = maxSize;
		m_buffer = new char[m_bufferSize];
		for (int i = 0; i < maxSize - 1; ++i)
		{
			m_buffer[i] = ' ';
		}
		m_buffer[maxSize - 1] = '\0';
	}

	char* getPointer()
	{
		return m_buffer;
	}

	int size() const
	{
		return m_bufferSize;
	}

	void clear()
	{
		if (m_bufferSize != 0)
		{
			m_bufferSize = 0;
			delete[] m_buffer;
			m_buffer = 0;
		}
	}

	~Buffer()
	{
		if (m_bufferSize != 0)
		{
			m_bufferSize = 0;
			delete[] m_buffer;
			m_buffer = 0;
		}
	}
};

class HttpServer
{
private:

	struct addrinfo* m_addr;
	Socket m_serverSocket;

public:
	HttpServer() : m_addr(0), m_serverSocket() {};

	int init(const char * ipAddress, const char * port);
	int listen();
	Socket accept();

	int recieve(Socket & client, Buffer buffer, int flags);
	int recieve(Socket& client, char* buffer, int bufferSize, int flags);
	int send(Socket & client, Buffer buffer, int flags);
	int send(Socket & client, const char * buffer, int bufferSize, int flags);

	std::string decodeURIComponent(const std::string & encoded);
	std::string encodeURIComponent(const std::string & decoded);

	int handleRequest(Socket & client, char * buffer, int bufferSize);
	std::string getRequestUri(const std::string& request);
	std::string getFilePath(const std::string& request);
	~HttpServer()
	{
		WSACleanup();
	}
};