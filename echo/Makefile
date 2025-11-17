# Компилятор и флаги
CXX = g++
CXXFLAGS = -Wall -std=c++20 -g
LDFLAGS = 

# Имя целевого файла
TARGET = echo_client

# Исходные файлы
SRC = echo_client.cpp

# Сборка напрямую из исходников
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# Очистка
clean:
	rm -f $(TARGET)

# Запуск
run: $(TARGET)
	./$(TARGET)

help:
	@echo "Доступные цели:"
	@echo "  make        - собрать клиента"
	@echo "  run         - запустить c ip: 172.16.40.1 port: 7"
	@echo "  clean       - удалить скомпилированные файлы"
	@echo "  help        - показать эту справку"
	@echo ""
	@echo "Использование клиента:"
	@echo "  ./echo_client              - использовать сервер 172.16.40.1:7"
	@echo "  ./echo_client <IP>         - использовать указанный IP и порт 7"
	@echo "  ./echo_client <IP> <PORT>  - использовать указанные IP и PORT"
	@echo ""
	@echo "Примеры:"
	@echo "  ./echo_client"
	@echo "  ./echo_client 172.16.40.1"
	@echo "  ./echo_client 172.16.40.1 7"

.PHONY: clean run help