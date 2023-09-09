#include <algorithm>
#include <cstdint>
#include <iostream>
#include <map>
#include <numeric>
#include <ranges>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Trade {
    int mProfit{0};
};

class Order {
public:

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC TYPES
    ///////////////////////////////////////////////////////////////////////////

    enum class Action {
        Buy,
        Sell,
        Bid,
        Offer,  
        None    ///< Error case if the action string not in {BUY, SELL, BID, OFFER} 
    };
    
    ///////////////////////////////////////////////////////////////////////////
    // CONSTRUCTOR
    ///////////////////////////////////////////////////////////////////////////

    Order(Action action, int size, int price): mAction(action), mSize(size), mPrice(price) {
        mCount = sCount;
        ++sCount;
    }

    ///////////////////////////////////////////////////////////////////////////
    // GETTERS
    ///////////////////////////////////////////////////////////////////////////

    Action action() const { 
        return mAction; 
    }

    int size() const { 
        return mSize; 
    }
    
    int price() const { 
        return mPrice; 
    }

    bool isBid() const { 
        return mAction == Action::Bid || mAction == Action::Buy; 
    }

    bool isOffer() const { 
        return mAction == Action::Offer || mAction == Action::Sell; 
    }

    bool empty() const { 
        return mSize == mTraded; 
    }

    int remaining() const { 
        return mSize - mTraded; 
    }

    int exposure() const { 
        return mPrice * remaining(); 
    }

    bool isMine() const {
        return mAction == Action::Buy || mAction == Action::Sell; 
    }

    string toString() const {
        const std::map<Action, string> actionMap = {
            {Action::Buy, "BUY"}, 
            {Action::Sell, "SELL"}, 
            {Action::Bid, "BID"}, 
            {Action::Offer, "OFFER"}
        };
        return actionMap.at(mAction) + " " + std::to_string(mSize) + " " + std::to_string(mPrice) + " " 
            + std::to_string(mCount);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // COMPARISON OPERATOR
    ///////////////////////////////////////////////////////////////////////////

    bool operator<(const Order &rhs) const {
        if (isOffer()) {
            return mPrice < rhs.mPrice || (mPrice == rhs.mPrice && mCount < rhs.mCount);
        } else {
            return mPrice > rhs.mPrice || (mPrice == rhs.mPrice && mCount < rhs.mCount);
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    bool canTrade(const Order &other) const {
        if (isBid() && other.isOffer()) {
            return mPrice >= other.mPrice;
        } else if (isOffer() && other.isBid()) {
            return mPrice <= other.mPrice;
        }
        return false;
    }
    
    // This must be a buy or bid, and other is a sell or offer. The price of the buy must be gte 
    // to price of the sell to process. This is considered the new order.
    // returns true if action was taken, else false. 
    Trade trade(Order &other) {
        if (canTrade(other)) {
            int saleSize = std::min(mSize, other.mSize);
            mTraded += saleSize;
            other.mTraded += saleSize;
            int profit = 0;
            if ((isMine() && !other.isMine()) || (!isMine() && other.isMine())) {
                profit = saleSize * std::abs(mPrice - other.mPrice);
            }
            return Trade{profit};
        }
        return Trade{};
    }
    
    // Converts the parameter `str` to an Action enum. If the str doesn't represent
    // one of the action, Action::None is returned.
    static inline Action parseAction(const string &str) {
        const std::map<string, Action> actionMap = {
            {"BUY", Action::Buy}, 
            {"SELL", Action::Sell}, 
            {"BID", Action::Bid}, 
            {"OFFER", Action::Offer}
        };
        auto it = actionMap.find(str);
        if (it == actionMap.end()) {
            return Action::None;
        }
        return it->second;
    }

private:

    ///////////////////////////////////////////////////////////////////////////
    // PRIVATE VARIABLES
    ///////////////////////////////////////////////////////////////////////////

    static inline uint64_t sCount{0};

    Action mAction;     ///< See enum `Action` documentation.
    int mSize;          ///< The number of shares in the order.
    int mPrice;         ///< Price per share in the order.
    uint64_t mCount;    ///< Kinda like a timestamp.
    int mTraded{0};     ///< Keep track of matched shares.

}; // class Order


// Stores sets of orders & stats associated with a given share.
class Share {
public:

    ///////////////////////////////////////////////////////////////////////////
    // CONSTRUCTORS
    ///////////////////////////////////////////////////////////////////////////

    Share() = default;
    Share(string name): mName(name) {}

    ///////////////////////////////////////////////////////////////////////////
    // GETTERS
    ///////////////////////////////////////////////////////////////////////////

    int profit() const {
        return mProfit;
    }

    int longExposure() const {
        return std::accumulate(mBids.begin(), mBids.end(), int(0), [](const int acc, const auto &bid){
            return acc + (bid.action() == Order::Action::Buy) ? bid.exposure() : 0;
        });
    }

    int shortExposure() const {
        return std::accumulate(mOffers.begin(), mOffers.end(), int(0), [](const int acc, const auto &offer){
            return acc + (offer.action() == Order::Action::Sell) ? offer.exposure() : 0;
        });
    }

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    std::vector<Trade> executeTrade2(Order &newOrder) {
        std::vector<Trade> trades;
        auto &otherOrders = (newOrder.isOffer()) ? mBids : mOffers;
        int totalTradeSize = 0;

        auto matchTrade = [&](const Order &order) -> bool {
            if (newOrder.remaining() > totalTradeSize && newOrder.canTrade(order)) {
                totalTradeSize += order.remaining();
                return true;
            }
            return false;
        };

        auto executeTrade = [&](Order order) -> Order {
            std::cout << "Executing trade" << endl;
            trades.push_back(newOrder.trade(order));
            return order;
        };

        auto orders = otherOrders | std::views::take_while(matchTrade) | std::views::transform(executeTrade);

        for (const Order &order : orders) {
            otherOrders.erase(order);
            if (!order.empty()) {
                otherOrders.insert(order);
            }
        }

        if (!newOrder.empty()) {
            // If new order is not empty, add into list of existing orders.
            auto &orderSet = (newOrder.isOffer()) ? mOffers : mBids;
            orderSet.insert(newOrder);
        }
        return trades;
    }

    std::vector<Trade> executeTrade(Order &newOrder) {
        std::vector<Trade> trades;
        auto &otherOrders = (newOrder.isOffer()) ? mBids : mOffers;
        for (auto it = otherOrders.begin(); it != otherOrders.end();) {
            if (newOrder.canTrade(*it)) {
                std::cout << "Executing trade" << endl;
                Order existingOrder = *it;
                it = otherOrders.erase(it);
                trades.push_back(newOrder.trade(existingOrder));
                if (newOrder.empty()) {
                    // If newOrder is empty, we can stop search for orders to trade with. 
                    if (!existingOrder.empty()) {
                        // If the existing order is not empty after trading, add the existing back into the set.
                        otherOrders.insert(existingOrder);
                    }
                    break;
                }
                continue;
            }
            // If we can't execute a trade, there are no more orders that we can trade with.
            break;
        }
        if (!newOrder.empty()) {
            // If new order is not empty, add into list of existing orders.
            auto &orderSet = (newOrder.isOffer()) ? mOffers : mBids;
            orderSet.insert(newOrder);
        }
        return trades;
    }

    void addNewOrder(Order newOrder) {
        cout << "Adding order to: " << mName << " " << newOrder.toString() << endl;
        auto trades = executeTrade2(newOrder);
        printOrders();
        for (const auto &trade : trades) {
            mProfit += trade.mProfit;
        }
    }

    void printOrders() {
        cout << "-- OFFERS (size, price) " << mOffers.size() << endl;
        for (const auto &order : mOffers) {
            cout << order.toString() << endl;
        }
        cout << "-- BIDS " << mBids.size() << endl;
        for (const auto &order : mBids) {
            cout << order.toString() << endl;
        }
        cout << "-------------------" << endl;
    }
    
private:

    ///////////////////////////////////////////////////////////////////////////
    // PRIVATE VARIABLES
    ///////////////////////////////////////////////////////////////////////////

    int mProfit{0};
    int mLongExposure{0};
    int mShortExposure{0};

    string mName{""};
    std::set<Order> mBids;
    std::set<Order> mOffers;
};

// All the orders stored in the system.
std::map<string, Share> sShares;

// Determines the Share container to place the order.
void processOrder(const string &name, const string& actionStr, int size, int price) {

    // Check if share already exists in the system.
    if (sShares.find(name) == sShares.end()) {
        cout << "Adding new share to system: " << name << endl;
        sShares.emplace(name, Share{name});
    }
    
    // Parse the action string of the order and add to Share object if no issue.
    auto action = Order::parseAction(actionStr);
    if (action != Order::Action::None) {
        sShares.at(name).addNewOrder(Order{action, size, price});
    } else {
        cout << "Invalid order action, order not processed." << endl;
    }
}

// Process the a single record that contains 1 or more orders in string format.
void processRecord(const string &record) {
    cout << "Processing Order: " << record << endl;
    string name, actionStr;
    int size, price;
    stringstream stream{record};
    stream >> name;
    while (!stream.eof()) {
        stream >> actionStr >> size >> price;
        processOrder(name, actionStr, size, price);
    }   
}

// Returns profit, long exposure, short exposure as a function of `records`.
std::tuple<int, int, int> trade(vector<string> const& records) {
    for (auto s : records) {
        processRecord(s);
    }
    return std::accumulate(sShares.begin(), sShares.end(), std::tuple<int, int, int>{0, 0, 0}, 
        [](const auto &acc, const auto &share) {
            return std::make_tuple(
                std::get<0>(acc) + share.second.profit(), 
                std::get<1>(acc) + share.second.longExposure(),
                std::get<2>(acc) + share.second.shortExposure()
            );
        }
    );
}


int main() {
    const std::vector<std::string> records = {
        "AAPL BUY 10 20 SELL 5 25 OFFER 10 18 BID 5 28",
    };

    auto result = trade(records);

    std::cout << "Results" << endl;
    std::cout << "-------" << endl;
    std::cout << std::get<0>(result) << ' ';
    std::cout << std::get<1>(result) << ' ';
    std::cout << std::get<2>(result) << ' ';
    std::cout << "\n";

    return 0;
}