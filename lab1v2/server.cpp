#include "server.h"
#include <fstream>

std::string HttpServer::decodeURIComponent(const std::string & encoded) {

    std::string decoded = encoded;
    std::smatch sm;
    std::string haystack;

    int dynamicLength = decoded.size() - 2;

    if (decoded.size() < 3) return decoded;

    for (int i = 0; i < dynamicLength; i++)
    {

        haystack = decoded.substr(i, 3);

        if (std::regex_match(haystack, sm, std::regex("%[0-9A-F]{2}")))
        {
            haystack = haystack.replace(0, 1, "0x");
            std::string rc = { (char)std::stoi(haystack, nullptr, 16) };
            decoded = decoded.replace(decoded.begin() + i, decoded.begin() + i + 3, rc);
        }

        dynamicLength = decoded.size() - 2;

    }

    return decoded;
}

std::string HttpServer::encodeURIComponent(const std::string & decoded)
{
    std::ostringstream oss;
    std::regex r("[!'\\(\\)*-.0-9A-Za-z_~]");
    std::smatch sm;
    std::string character;
    for (const char& c : decoded)
    {
        character = c;
        if (std::regex_match(character, sm, r))
        {
            oss << c;
        }
        else
        {
            oss << "%" << std::uppercase << std::hex << (0xff & c);
        }
    }
    return oss.str();
}


int HttpServer::init(const char* ipAddress, const char* port)
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (result != 0) {
        std::cout << "WSAStartup failed: " << result << std::endl;
        return result;
    }
    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    m_addr = NULL;
    result = ::getaddrinfo(ipAddress, port, &hints, &m_addr);
    if (result != 0) {
        std::cout << "getaddrinfo failed: " << result << std::endl;
        return result;
    }

    m_serverSocket = std::move(socket(m_addr->ai_family, m_addr->ai_socktype, m_addr->ai_protocol));
    if (m_serverSocket.get() == INVALID_SOCKET) {
        std::cout << "Error at socket: " << WSAGetLastError() << std::endl;
        freeaddrinfo(m_addr);
        return 1;
    }

    result = bind(m_serverSocket.get(), m_addr->ai_addr, (int)m_addr->ai_addrlen);
    if (result == SOCKET_ERROR) {
        std::cout << "bind failed with error: " << WSAGetLastError() << std::endl;
        freeaddrinfo(m_addr);
        return 1;
    }
}

int HttpServer::listen()
{
    int result = ::listen(m_serverSocket.get(), SOMAXCONN);
    if (result == SOCKET_ERROR) {
        std::cout << "listen failed with error: " << WSAGetLastError() << std::endl;
        return 1;
    }
    return 0;
}

Socket HttpServer::accept()
{
    Socket clientSocket(::accept(m_serverSocket.get(), NULL, NULL));
    if (clientSocket.get() == INVALID_SOCKET) {
        std::cout << "accept failed: " << WSAGetLastError() << std::endl;
        return 0;
    }
    return clientSocket;
}

int HttpServer::recieve(Socket & client, Buffer buffer, int flags)
{
    return ::recv(client.get(), buffer.getPointer(), buffer.size(), flags);
}

int HttpServer::recieve(Socket& client, char* buffer, int bufferSize, int flags)
{
    return ::recv(client.get(), buffer, bufferSize, flags);
}

int HttpServer::send(Socket & client, Buffer buffer, int flags)
{
    return ::send(client.get(), buffer.getPointer(), buffer.size(), flags);
}

int HttpServer::send(Socket & client,const char* buffer, int bufferSize, int flags)
{
    return ::send(client.get(), buffer, bufferSize, flags);
}

std::string HttpServer::getRequestUri(const std::string& request)
{
    std::string firstLine = request.substr(0, request.find('\n'));
    return firstLine;
}

std::string HttpServer::getFilePath(const std::string& request)
{
    std::string firstLine = getRequestUri(request);
    std::string filePath = "";
    if (firstLine.find("GET") != std::string::npos)
    {
        filePath = firstLine.substr(5, firstLine.find("HTTP") - 6);
    }
    return filePath;
}

int HttpServer::handleRequest(Socket & client, char* request, int result)
{
    std::string response = ""; // сюда будет записыватьс€ ответ клиенту
    std::string response_body = ""; // тело ответа

    std::cout << request << std::endl;
    request[result] = '\0';
    std::string filePath = getFilePath(std::string(request));
    filePath = decodeURIComponent(filePath);

    

    std::ifstream inFile(filePath, std::ios_base::binary);
    
    
    if (inFile.is_open())
    {
        inFile.seekg(0, std::ios_base::end);
        size_t length = inFile.tellg();
        inFile.seekg(0, std::ios_base::beg);

        std::vector<char> buffer;
        buffer.reserve(length);
        std::copy(std::istreambuf_iterator<char>(inFile),
            std::istreambuf_iterator<char>(),
            std::back_inserter(buffer));
        
        std::cout << length << std::endl;

        std::string strBuffer(buffer.begin(), buffer.end());

        // ‘ормируем весь ответ вместе с заголовками
        response = response + "HTTP/1.1 200 OK\r\n"
            + "Version: HTTP/1.1\r\n"
            + "Content-Type: text/html\r\n"
            + "Content-Length: " + std::to_string(strBuffer.length())
            + "\r\n\r\n"
            + strBuffer;
    }
    else
    {
        std::cout << "can't open the file" << std::endl;

        response = response + "HTTP/1.1 404\r\n"
            + "Version: HTTP/1.1\r\n"
            + "Content-Type: text/html\r\n"
            + "Content-Length: " + std::to_string(response_body.length())
            + "\r\n\r\n"
            + response_body;
    }

    this->send(client, response.c_str(), response.length(), 0);
    
    return 0; 
}