#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <regex>
#include <fstream>

#define _WIN32_WINNT 0x501
#include <WinSock2.h>
#include <WS2tcpip.h>

#include <thread>
#include <vector>
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

class Logger
{
public:
	static void clearLogs(const std::string& filename)
	{
		std::ofstream ofs;
		ofs.open(filename, std::ofstream::out | std::ofstream::trunc);
		ofs.close();
	}

	static void addToLogs(const std::string& filename, const std::string& data)
	{
		std::ofstream outfile;
		outfile.open(filename, std::ios_base::app); // append instead of overwrite
		outfile << data;
		outfile.close();
	}

};

class ThreadHandler
{
private:
	static const int maxThreads = 10;
	std::thread threadArr[maxThreads];
	std::vector<bool> freeThreads;
public:
	ThreadHandler()
	{
		for (int i = 0; i < maxThreads; ++i)
		{
			freeThreads.push_back(true);
		}
	}

	void setThreadStatus(int index, bool status)
	{
		freeThreads[index] = status;
	}

	int getFreeThread()
	{
		for (int i = 0; i < maxThreads; ++i)
		{
			if (freeThreads[i] == true)
			{
				return i;
			}
		}
		return -1;
	}

	~ThreadHandler()
	{
		for (int i = 0; i < maxThreads; ++i)
		{
			freeThreads.pop_back();
		}
	}
};

class HttpServer
{
private:

	struct addrinfo* m_addr;
	Socket m_serverSocket;
	ThreadHandler * m_threadHandler;

public:
	HttpServer() : m_addr(0), m_serverSocket() 
	{
		m_threadHandler = new ThreadHandler();
	};

	int init(const char * ipAddress, const char * port);
	int listen();
	Socket accept();

	int recieve(Socket& client, char* buffer, int bufferSize, int flags);
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