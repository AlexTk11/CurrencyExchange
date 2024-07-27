#ifndef COMMON_HPP
#define COMMON_HPP

#include <string>

static short port = 5555;

namespace Requests {
    static const std::string Registration = "Reg";
    static const std::string Hello = "Hel";
    static const std::string Balance = "Bal";
    static const std::string Order = "Ord";
    static const std::string ViewOrders = "ViewOrders";
    static const std::string CancelOrder = "CancelOrder";
    static const std::string ViewAllOrders = "ViewAllOrders";
}

enum OrderType {
    BUY,
    SELL
};

#endif // COMMON_HPP
