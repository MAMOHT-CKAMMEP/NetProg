#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <system_error>
#include <fcntl.h>

// Сетевые заголовки
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

class EchoTCPClient {
private:
    int sockfd;
    std::string server_ip;
    int port;
    struct sockaddr_in server_addr;
    
public:
    // Конструктор
    EchoTCPClient(const std::string& ip = "172.16.40.1", int p = 7) 
        : sockfd(-1), server_ip(ip), port(p) {
        std::memset(&server_addr, 0, sizeof(server_addr));
    }
    
    // Деструктор
    ~EchoTCPClient() {
        closeSocket();
    }
    
    // Закрытие сокета
    void closeSocket() {
        if (sockfd != -1) {
            close(sockfd);
            sockfd = -1;
        }
    }
    
    // Инициализация клиента
    bool initialize() {
        // Создание TCP сокета
        sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sockfd < 0) {
            std::cerr << "Ошибка создания сокета: " << std::strerror(errno) << std::endl;
            return false;
        }
        
        // Настройка адреса сервера
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        
        if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
            std::cerr << "Неверный IP-адрес: " << server_ip << std::endl;
            closeSocket();
            return false;
        }
        
        return true;
    }
    
    // Подключение к серверу с таймаутом
    bool connectToServer() {
        if (sockfd < 0) {
            std::cerr << "Сокет не инициализирован" << std::endl;
            return false;
        }
        
        // Установка неблокирующего режима для таймаута
        int flags = fcntl(sockfd, F_GETFL, 0);
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
        
        // Попытка подключения
        int result = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
        
        if (result < 0) {
            if (errno == EINPROGRESS) {
                // Подключение в процессе, ждем с таймаутом
                fd_set writefds;
                struct timeval timeout;
                
                FD_ZERO(&writefds);
                FD_SET(sockfd, &writefds);
                
                timeout.tv_sec = 5;
                timeout.tv_usec = 0;
                
                result = select(sockfd + 1, NULL, &writefds, NULL, &timeout);
                
                if (result <= 0) {
                    std::cerr << "Таймаут подключения: сервер не отвечает" << std::endl;
                    fcntl(sockfd, F_SETFL, flags);
                    return false;
                }
                
                // Проверяем, что подключение успешно
                int error = 0;
                socklen_t len = sizeof(error);
                if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0) {
                    std::cerr << "Ошибка подключения: " << (error ? std::strerror(error) : "unknown error") << std::endl;
                    fcntl(sockfd, F_SETFL, flags);
                    return false;
                }
            } else {
                std::cerr << "Ошибка подключения: " << std::strerror(errno) << std::endl;
                fcntl(sockfd, F_SETFL, flags);
                return false;
            }
        }
        
        // Восстанавливаем блокирующий режим
        fcntl(sockfd, F_SETFL, flags);
        return true;
    }
    
    // Отправка сообщения и получение эха
    bool sendAndReceive(const std::string& message) {
        if (sockfd < 0) {
            std::cerr << "Сокет не инициализирован" << std::endl;
            return false;
        }
        
        // Таймаут на прием данных (5 секунд)
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            std::cerr << "Ошибка установки таймаута: " << std::strerror(errno) << std::endl;
        }
        
        // Отправка сообщения
        ssize_t sent_bytes = send(sockfd, message.c_str(), message.length(), 0);
        
        if (sent_bytes < 0) {
            std::cerr << "Ошибка отправки сообщения: " << std::strerror(errno) << std::endl;
            return false;
        }
        
        // Буфер для приема данных
        char buffer[1024];
        std::memset(buffer, 0, sizeof(buffer));
        
        // Прием ответа от сервера
        ssize_t recv_bytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        
        if (recv_bytes < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cerr << "Таймаут: сервер не ответил" << std::endl;
            } else {
                std::cerr << "Ошибка приема данных: " << std::strerror(errno) << std::endl;
            }
            return false;
        }
        
        if (recv_bytes == 0) {
            std::cout << "Сервер закрыл соединение" << std::endl;
            return false;
        }
        
        // Преобразование полученных данных в строку
        buffer[recv_bytes] = '\0';
        std::string echo_response(buffer);
        
        // Вывод результата
        std::cout << "Эхо-ответ: " << echo_response << std::endl;
        
        return true;
    }
    
    // Интерактивный режим
    void interactiveMode() {
        std::string input;
        
        while (true) {
            std::cout << "Введите сообщение (quit - выход): ";
            std::getline(std::cin, input);
            
            if (input == "quit" || input == "exit") {
                std::cout << "Завершение работы..." << std::endl;
                break;
            }
            
            if (input.empty()) {
                std::cout << "Сообщение не может быть пустым" << std::endl;
                continue;
            }
            
            if (!sendAndReceive(input)) {
                std::cout << "Ошибка при обмене данными" << std::endl;
                break;
            }
        }
    }
    
    // Основной метод запуска
    bool run() {
        if (!initialize()) {
            return false;
        }
        
        if (!connectToServer()) {
            closeSocket();
            return false;
        }
        
        interactiveMode();
        closeSocket();
        return true;
    }
};

int main(int argc, char* argv[]) {
    std::string server_ip = "172.16.40.1";
    int port = 7;
    
    // Обработка аргументов командной строки
    if (argc == 2) {
        server_ip = argv[1];
    } else if (argc == 3) {
        server_ip = argv[1];
        port = std::atoi(argv[2]);
        if (port <= 0 || port > 65535) {
            std::cerr << "Ошибка: неверный номер порта!" << std::endl;
            return 1;
        }
    } else if (argc > 3) {
        std::cerr << "Слишком много аргументов!" << std::endl;
        return 1;
    }
    
    // Вывод информации о подключении
    std::cout << "Сервер: " << server_ip << std::endl;
    std::cout << "Порт: " << port << std::endl;
    std::cout << "Протокол: TCP" << std::endl;
    
    try {
        EchoTCPClient client(server_ip, port);
        client.run();
    } catch (const std::exception& e) {
        std::cerr << "Исключение: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}