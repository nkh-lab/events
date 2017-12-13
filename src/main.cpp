#include <iostream>
#include "Events.hpp"

using namespace Events;

/**
 * @brief Example of the class which provides events.
 */
class Provider
{
public:
    Event<UnsafeSyncPolicy, int> EventInt;
    Event<UnsafeSyncPolicy, std::string> EventString;
    Event<UnsafeSyncPolicy, bool> EventBool;

    Event<UnsafeSyncPolicy, int, std::string> EventIntString;
    Event<SafeSyncPolicy> EventVoid;
};

/**
 * @brief Example of the class which consumes(handles) events.
 */
class Consumer
{
public:
    Consumer(const std::string& name, Provider& provider):mProvider(provider)
    {
        mName = name;

        using namespace std::placeholders;
        mEventIntHandlerID = mProvider.EventInt.subscribe(std::bind(&Consumer::onEventInt, this, _1));

        mProvider.EventString.subscribe([this](const std::string& val)
        {

            std::cout << __FUNCTION__ << ": " << mName << " string = " << val << std::endl;
        }
        );

        mProvider.EventBool.subscribe([this](const bool& val)
        {
            std::cout << __FUNCTION__ << ": " << mName << " bool = " << val << std::endl;
        }
        );

        mProvider.EventIntString.subscribe([this](const int& valInt, const std::string& valStr)
        {
            std::cout << __FUNCTION__ << ": " << mName << " int = " << valInt << ", string = " << valStr << std::endl;
        }
        );

        mProvider.EventVoid.subscribe([this]()
        {
            std::cout << __FUNCTION__ << ": " << mName << " void " << std::endl;
        }
        );
    }

    void unsubsribeFromEventInt(void)
    {
        mProvider.EventInt.unsubscribe(mEventIntHandlerID);
    }

private:
    std::string mName;
    int mEventIntHandlerID;
    Provider& mProvider;

    void onEventInt(int val)
    {
        std::cout << mName << " Consumer::onEventInt() val = " << val << std::endl;
    }
};

/**
 * @brief Main app function where events usage is tested.
 * @return default - n/a
 */
int main(void)
{
    Provider p;

    Consumer c1("Consumer1", p);
    Consumer c2("Consumer2", p);

    p.EventInt.rise(1);
    c1.unsubsribeFromEventInt();
    p.EventInt.rise(2);
    p.EventInt.rise(3);

    p.EventString.rise("string test");
    p.EventBool.rise(false);


    p.EventIntString.rise(777, "int and string test");
    p.EventVoid.rise();

    /* Output:

    Consumer2 Consumer::onEventInt() val = 1
    Consumer1 Consumer::onEventInt() val = 1
    Consumer2 Consumer::onEventInt() val = 2
    Consumer2 Consumer::onEventInt() val = 3
    operator(): Consumer2 string = string test
    operator(): Consumer1 string = string test
    operator(): Consumer2 bool = 0
    operator(): Consumer1 bool = 0
    operator(): Consumer2 int = 777, string = int and string test
    operator(): Consumer1 int = 777, string = int and string test
    operator(): Consumer2 void
    operator(): Consumer1 void

     */
}
