#include <iostream>
#include <boost/asio.hpp>
#include "Common.hpp"
#include <nlohmann/json.hpp>

using boost::asio::ip::tcp;

//отправка сообщения на сервер
void SendMessage(
    tcp::socket& aSocket,
    const std::string& aId,
    const std::string& aRequestType,
    const std::string& aMessage,
    double amount = 0,
    double price = 0,
    const std::string& orderType = "") {
    nlohmann::json req;
    req["UserId"] = aId;
    req["ReqType"] = aRequestType;
    req["Message"] = aMessage;
    req["Amount"] = amount;
    req["Price"] = price;
    req["OrderType"] = orderType;

    std::string request = req.dump();
    boost::asio::write(aSocket, boost::asio::buffer(request, request.size()));
}


std::string ReadMessage(tcp::socket& aSocket) {
    boost::asio::streambuf b;
    boost::asio::read_until(aSocket, b, "\0");
    std::istream is(&b);
    std::string line(std::istreambuf_iterator<char>(is), {});
    return line;
}


std::string ProcessRegistration(tcp::socket& aSocket) {
    std::string name;
    std::cout << "Hello! Enter your name: ";
    std::cin >> name;
    while(name == ""){
        std::cout << "Incorrect name! Enter your name: ";
        std::cin >> name;
    }
    
    SendMessage(aSocket, "0", Requests::Registration, name);
    return ReadMessage(aSocket);
}

int main() {
    try {
        boost::asio::io_service io_service;

        tcp::resolver resolver(io_service);
        tcp::resolver::query query(tcp::v4(), "127.0.0.1", std::to_string(port));
        tcp::resolver::iterator itr = resolver.resolve(query);

        tcp::socket s(io_service);
        s.connect(*itr);

        std::string my_id = ProcessRegistration(s);

        while (true) {
            std::cout << "Menu:\n"
                         "1) Hello Request\n"
                         "2) Check Balance\n"
                         "3) Place Order\n"
                         "4) View Active Orders\n"
                         "5) View All Orders\n"
                         "6) Cancel Order\n"
                         "7) Exit\n"
                         << std::endl;

            short menu_option_num;
            std::cin >> menu_option_num;
            switch (menu_option_num) {
                case 1: {
                    SendMessage(s, my_id, Requests::Hello, "");
                    std::cout << ReadMessage(s);
                    break;
                }
                case 2: {
                    SendMessage(s, my_id, Requests::Balance, "");
                    std::cout << ReadMessage(s);
                    break;
                }
                case 3: {
                    double amount, price;
                    short orderType;
                    std::cout << "Enter amount: ";
                    std::cin >> amount;
                    std::cout << "Enter price: ";
                    std::cin >> price;
                    std::cout << "Select type of order:\n1) BUY\n2) SELL\n";
                    std::cin >> orderType;
                    switch (orderType) {
                        case 1: {
                            SendMessage(s, my_id, Requests::Order, "", amount, price, "BUY");
                            std::cout << ReadMessage(s);
                            break;
                        }
                        case 2: {
                            SendMessage(s, my_id, Requests::Order, "", amount, price, "SELL");
                            std::cout << ReadMessage(s);
                            break;
                        }
                        default: {
                            std::cout << "Unknown command\n";
                            break;
                        }
                    }
                    break;
                }
                case 4: {
                    SendMessage(s, my_id, Requests::ViewOrders, "");
                    std::cout << ReadMessage(s);
                    break;
                }
                case 5: {
                    SendMessage(s, my_id, Requests::ViewAllOrders, "");
                    std::cout << ReadMessage(s);
                    break;
                }
                case 6: {
                    double amount, price;
                    short orderType;
                    std::cout << "Enter amount: ";
                    std::cin >> amount;
                    std::cout << "Enter price: ";
                    std::cin >> price;
                    std::cout << "Select type of order to cancel:\n1) BUY\n2) SELL\n";
                    std::cin >> orderType;
                    switch (orderType) {
                        case 1: {
                            SendMessage(s, my_id, Requests::CancelOrder, "", amount, price, "BUY");
                            std::cout << ReadMessage(s);
                            break;
                        }
                        case 2: {
                            SendMessage(s, my_id, Requests::CancelOrder, "", amount, price, "SELL");
                            std::cout << ReadMessage(s);
                            break;
                        }
                        default: {
                            std::cout << "Unknown command\n";
                            break;
                        }
                    }
                    break;
                }
                case 7: {
                    exit(0);
                    break;
                }
                default: {
                    std::cout << "Unknown menu option\n" << std::endl;
                }
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}