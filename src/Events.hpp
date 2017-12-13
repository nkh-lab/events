#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <functional>
#include <map>
#include <list>
#include <algorithm>
#include <mutex>

namespace Events {

/**
 * @brief The VoidClass class is used in UnsafeSyncPolicy like stub for Locker_t.
 */
class VoidClass
{
    friend class UnsafeSyncPolicy;
    template <typename TSyncPolicy, typename... TEvtArgs>
    friend class Event;

private:
    VoidClass(){}
};

/**
 * @brief The UnsafeSyncPolicy class is used to determine strategy for Event class.
 * It doesn't include synchronization mechanism and can be used when event provider and consumer are in the single thread.
 */
class UnsafeSyncPolicy
{
    template <typename TSyncPolicy, typename... TEvtArgs>
    friend class Event;

private:
    typedef VoidClass Locker_t;

    UnsafeSyncPolicy(Locker_t& locker) {}
};

/**
 * @brief The SafeSyncPolicy class is used to determine strategy for Event class.
 * Includs synchronization mechanism based on Locker_t type.
 */
class SafeSyncPolicy
{
    template <typename TSyncPolicy, typename... TEvtArgs>
    friend class Event;

private:
    typedef std::mutex Locker_t;

    SafeSyncPolicy(Locker_t& locker): mLocker(locker)
    {
        mLocker.lock();
    }

    ~SafeSyncPolicy()
    {
        mLocker.unlock();
    }

    Locker_t&  mLocker;
};

template <typename TSyncPolicy, typename... TEvtArgs>
/**
 * @brief Event template class based on templates:
 * TSyncPolicy - describes synchronisation starategy.
 * TEvtArgs    - describes variadic arguments for event.
 *
 * @warning subscribe() and unsubscribe() mast not invoke from evnt handler on consumer side.
 * It leads to deadlock (if SafeSyncPolicy is used).
 * To unsubsribe from event handler 'oneShot = true' in subscribe() can be used.
 */
class Event
{
    typedef std::function<void(const TEvtArgs&...)> Handler_t;
    typedef std::map<int /*handlerID*/, Handler_t>  Handlers_t;

    Handlers_t mHandlers;
    std::list<int> mOneShotHandlerIDs;

    typename TSyncPolicy::Locker_t mLocker;

public:
    /**
     * @brief Subscribes to event.
     * @param handler - binded event handler.
     * @param oneShot - true  - handler automatically is usubscribed after fist event rising,
     *                  false (default) - envents are rised until unsubscribe() invoking.
     * @return Handler ID.
     */
    int subscribe(const Handler_t& handler, bool oneShot = false)
    {
        TSyncPolicy guard(mLocker);

        int handlerID = generateHandlerID();

        mHandlers.insert( std::pair<int /*handlerID*/, Handler_t>(handlerID, handler));

        if (oneShot) mOneShotHandlerIDs.push_back(handlerID);

        return handlerID;
    }
    /**
     * @brief Unsubscribes from event.
     * @param handlerID - ID of the handler which should be unsubscribed.
     */
    void unsubscribe(const int& handlerID)
    {
        TSyncPolicy guard(mLocker);

        mHandlers.erase(handlerID);
    }

    /**
     * @brief Rises the event CBs.
     * @param val - event variadic arguments values depends on event type.
     */
    void rise(const TEvtArgs&... val)
    {
        TSyncPolicy guard(mLocker);

        for(auto it = mHandlers.begin(); it != mHandlers.end(); ++it)
        {
            it->second(val...);
        }

        // remove One Shot events
        for(auto it = mOneShotHandlerIDs.begin(); it != mOneShotHandlerIDs.end(); ++it)
        {
           mHandlers.erase(*it);
        }

        mOneShotHandlerIDs.clear();
    }

private:
    int generateHandlerID()
    {
        int ret;

        do
        {
            ret = rand();

        }while(ret == 0 || mHandlers.find(ret) != mHandlers.end());

        return ret;
    }
};

}

#endif
