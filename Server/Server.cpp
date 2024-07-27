#include <cstdlib>
#include <iostream>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <vector>
#include <queue>
#include <string>
#include <chrono>
#include "Common.hpp"

using boost::asio::ip::tcp;

class Core {
public:
    struct Order {
        size_t userId;
        double amount;
        double price;
        OrderType type;
        std::chrono::time_point<std::chrono::steady_clock> timestamp;
        bool operator==(const Order& other) const {
            return userId == other.userId && amount == other.amount && price == other.price && type == other.type 
                    && timestamp == other.timestamp;
        }
    };

    // "Регистрирует" нового пользователя и возвращает его ID.
    std::string RegisterNewUser(const std::string& aUserName) {
        size_t newUserId = mUsers.size();
        mUsers[newUserId] = aUserName;
        mBalances[newUserId] = {0, 0}; // Initialize balance with 0 USD and 0 RUB
        return std::to_string(newUserId);
    }

    // Запрос имени клиента по ID
    std::string GetUserName(const std::string& aUserId) {
        const auto userIt = mUsers.find(std::stoi(aUserId));
        if (userIt == mUsers.cend()) {
            return "Error! Unknown User\n";
        } else {
            return userIt->second;
        }
    }

    // Запрос баланса клиента по ID
    std::string GetBalance(const std::string& aUserId) {
        size_t userId = std::stoi(aUserId);
        const auto balanceIt = mBalances.find(userId);
        if (balanceIt == mBalances.cend()) {
            return "Error! Unknown User\n";
        } else {
            return "USD: " + std::to_string(balanceIt->second.first) + ", RUB: " 
                    + std::to_string(balanceIt->second.second) + "\n";
        }
    }

    // Добавление ордера в очередь
    std::string PlaceOrder(const std::string& aUserId, double amount, double price, OrderType type) {
        size_t userId = std::stoi(aUserId);
        Order order = { userId, amount, price, type, std::chrono::steady_clock::now()};

        if (type == OrderType::BUY) {
            buyOrders.push(order);
        } else {
            sellOrders.push(order);
        }

        MatchOrders();

        return "Order placed.\n";
    }

    std::string ViewActiveOrders(const std::string& aUserId) {
        size_t userId = std::stoi(aUserId);
        std::string result = "Active Orders:\n";
        
        std::priority_queue<Order, std::vector<Order>, decltype(buyComp)> buyOrdersCopy = buyOrders;
        std::priority_queue<Order, std::vector<Order>, decltype(sellComp)> sellOrdersCopy = sellOrders;
        
        result += "Buy Orders:\n";
        while (!buyOrdersCopy.empty()) {
            Order order = buyOrdersCopy.top();
            buyOrdersCopy.pop();
            if (order.userId == userId) {
                result += "Amount: " + std::to_string(order.amount) + ", Price: " + std::to_string(order.price) + "\n";
            }
        }
        
        result += "Sell Orders:\n";
        while (!sellOrdersCopy.empty()) {
            Order order = sellOrdersCopy.top();
            sellOrdersCopy.pop();
            if (order.userId == userId) {
                result += "Amount: " + std::to_string(order.amount) + ", Price: " + std::to_string(order.price) + "\n";
            }
        }

        return result;
    }


    std::string ViewAllOrders() {
        
        std::string result = "Active Orders:\n";
        
        std::priority_queue<Order, std::vector<Order>, decltype(buyComp)> buyOrdersCopy = buyOrders;
        std::priority_queue<Order, std::vector<Order>, decltype(sellComp)> sellOrdersCopy = sellOrders;
        
        result += "Buy Orders:\n";
        while (!buyOrdersCopy.empty()) {
            Order order = buyOrdersCopy.top();
            buyOrdersCopy.pop();
            
            result += "Amount: " + std::to_string(order.amount) + ", Price: " + std::to_string(order.price) + "\n";
            
        }
        
        result += "Sell Orders:\n";
        while (!sellOrdersCopy.empty()) {
            Order order = sellOrdersCopy.top();
            sellOrdersCopy.pop();
            
            result += "Amount: " + std::to_string(order.amount) + ", Price: " + std::to_string(order.price) + "\n";
            
        }

        return result;
    }

    std::string CancelOrder(const std::string& aUserId, double amount, double price, OrderType type) {
        size_t userId = std::stoi(aUserId);
        Order order = { userId, amount, price, type };

        bool removed = false;
        if (type == OrderType::BUY) {
            removed = RemoveOrder(buyOrders, order);
        } else {
            removed = RemoveOrder(sellOrders, order);
        }

        if (removed) {
            return "Order cancelled.\n";
        } else {
            return "Order not found.\n";
        }
    }

private:
    std::unordered_map<size_t, std::string> mUsers;
    std::unordered_map<size_t, std::pair<double, double>> mBalances;
    std::function<bool(Order, Order)> buyComp = [](const Order& a, const Order& b) {
        if (a.price == b.price) return a.timestamp > b.timestamp;
        return a.price < b.price;
    };
    std::function<bool(Order, Order)> sellComp = [](const Order& a, const Order& b) {
        if (a.price == b.price) return a.timestamp > b.timestamp;
        return a.price > b.price;
    };

    std::priority_queue<Order, std::vector<Order>, decltype(buyComp)> buyOrders{buyComp};
    std::priority_queue<Order, std::vector<Order>, decltype(sellComp)> sellOrders{sellComp};
    

    void MatchOrders() {
    while (!buyOrders.empty() && !sellOrders.empty()) {
        Order buyOrder = buyOrders.top();
        Order sellOrder = sellOrders.top();

        if (buyOrder.price >= sellOrder.price) {
            double dealAmount = std::min(buyOrder.amount, sellOrder.amount);
            double dealPrice = sellOrder.price;

            mBalances[buyOrder.userId].first += dealAmount;
            mBalances[buyOrder.userId].second -= dealAmount * dealPrice;
            mBalances[sellOrder.userId].first -= dealAmount;
            mBalances[sellOrder.userId].second += dealAmount * dealPrice;

            buyOrder.amount -= dealAmount;
            sellOrder.amount -= dealAmount;

            buyOrders.pop();    
            sellOrders.pop();

            // Возвращаем частично исполненные ордера обратно в очереди
            if (buyOrder.amount > 0) buyOrders.push(buyOrder);
            if (sellOrder.amount > 0) sellOrders.push(sellOrder);
        } else {
            break;
        }
    }
}

    

    template<typename T>
    bool RemoveOrder(std::priority_queue<Order, std::vector<Order>, T>& orders, const Order& order) {
        std::vector<Order> tempOrders;
        bool removed = false;

        while (!orders.empty()) {
            Order topOrder = orders.top();
            orders.pop();
            if (!removed && topOrder.userId == order.userId && topOrder.amount == order.amount && topOrder.price == order.price 
                    && topOrder.type == order.type) {
                removed = true;
            } else {
                tempOrders.push_back(topOrder);
            }
        }

        for (const Order& ord : tempOrders) {
            orders.push(ord);
        }

        return removed;
    }

};

Core& GetCore() {
    static Core core;
    return core;
}

class session {
public:
    session(boost::asio::io_service& io_service)
        : socket_(io_service) {
    }

    tcp::socket& socket() {
        return socket_;
    }

    void start() {
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&session::handle_read, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    // Обработка полученного сообщения.
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
        if (!error) {
            data_[bytes_transferred] = '\0';

            auto j = nlohmann::json::parse(data_);
            auto reqType = j["ReqType"];

            std::string reply = "Error! Unknown request type";
            if (reqType == Requests::Registration) {
                reply = GetCore().RegisterNewUser(j["Message"]);
            } else if (reqType == Requests::Hello) {
                reply = "Hello, " + GetCore().GetUserName(j["UserId"]) + "!\n";
            }  else if (reqType == Requests::Balance) {
                reply = GetCore().GetBalance(j["UserId"]);
            } else if (reqType == Requests::Order) {
                double amount = j["Amount"];
                double price = j["Price"];
                OrderType type = j["OrderType"] == "BUY" ? OrderType::BUY : OrderType::SELL;
                reply = GetCore().PlaceOrder(j["UserId"], amount, price, type);
            } else if (reqType == Requests::ViewOrders) {
                reply = GetCore().ViewActiveOrders(j["UserId"]);
            } else if (reqType == Requests::CancelOrder) {
                double amount = j["Amount"];
                double price = j["Price"];
                OrderType type = j["OrderType"] == "BUY" ? OrderType::BUY : OrderType::SELL;
                reply = GetCore().CancelOrder(j["UserId"], amount, price, type);
            } else if (reqType == Requests::ViewAllOrders) {
                reply = GetCore().ViewAllOrders();
            }

            boost::asio::async_write(socket_,
                boost::asio::buffer(reply, reply.size()),
                boost::bind(&session::handle_write, this,
                    boost::asio::placeholders::error));
        } else {
            delete this;
        }
    }

    void handle_write(const boost::system::error_code& error) {
        if (!error) {
            socket_.async_read_some(boost::asio::buffer(data_, max_length),
                boost::bind(&session::handle_read, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
        } else {
            delete this;
        }
    }

private:
    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
};

class server {
public:
    server(boost::asio::io_service& io_service)
        : io_service_(io_service),
          acceptor_(io_service, tcp::endpoint(tcp::v4(), port)) {
        std::cout << "Server started! Listen " << port << " port" << std::endl;

        session* new_session = new session(io_service_);
        acceptor_.async_accept(new_session->socket(),
            boost::bind(&server::handle_accept, this, new_session,
                boost::asio::placeholders::error));
    }

    void handle_accept(session* new_session,
        const boost::system::error_code& error) {
        if (!error) {
            new_session->start();
            new_session = new session(io_service_);
            acceptor_.async_accept(new_session->socket(),
                boost::bind(&server::handle_accept, this, new_session,
                    boost::asio::placeholders::error));
        } else {
            delete new_session;
        }
    }

private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
};

int main() {
    try {
        boost::asio::io_service io_service;
        static Core core;

        server s(io_service);

        io_service.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
