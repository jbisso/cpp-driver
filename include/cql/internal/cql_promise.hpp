/*
 * File:   cql_promise.hpp
 * Author: mc
 *
 * Created on October 1, 2013, 2:31 PM
 */

#ifndef CQL_PROMISE_HPP_
#define	CQL_PROMISE_HPP_

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/exception_ptr.hpp>

namespace cql {

template<typename TResult>
class cql_promise_t {
public:
    cql_promise_t() :
        _mutex(new boost::mutex()),
        _value_set(new bool(false)),
        _promise(new boost::promise<TResult>()),
        _future(_promise->get_future()),
        _retry_callback(0)
    {}

    // in multithraded environment only
    // one thread will succeedd in setting  promise value.
    inline bool
    set_value(
        const TResult& value)
    {
        boost::mutex::scoped_lock lock(*_mutex);
        if (!*_value_set) {
            *_value_set = true;
            _promise->set_value(value);
            return true;
        }
        return false;
    }

    typedef boost::function<void()>
        _retry_callback_t;

    /** Sets the time after which the `retry_callback' will be called unless 
        promise is fulfilled. The callback should be capable of retrying the operation. */
    void
    start_timeout(
        boost::asio::io_service& io_service,
        const boost::posix_time::time_duration& duration,
        _retry_callback_t retry_callback)
    {
        _retry_callback = retry_callback;
        
        _timer = boost::shared_ptr<boost::asio::deadline_timer>(new boost::asio::deadline_timer(io_service));
        _timer->expires_from_now(duration);
        _timer->async_wait(boost::bind(&cql_promise_t::retry_trigger, this, _1));
    }

    inline bool
    set_exception(
        const boost::exception_ptr& exception)
    {
        boost::mutex::scoped_lock lock(*_mutex);
        if (!*_value_set) {
            *_value_set = true;
            _promise->set_exception(exception);
            return true;
        }

        return false;
    }

    inline boost::shared_future<TResult>
    shared_future() const
    {
        return _future;
    }
    
private:
    
    void
    retry_trigger(const boost::system::error_code& err)
    {
        if (!_value_set && _retry_callback && !err) {
            _retry_callback();
        }
    }
    
    boost::shared_ptr<boost::mutex>              _mutex;
    boost::shared_ptr<bool>                      _value_set;
    boost::shared_ptr<boost::promise<TResult> >  _promise;
    boost::shared_future<TResult>                _future;
    
    boost::shared_ptr<boost::asio::deadline_timer> _timer;
    _retry_callback_t                              _retry_callback;
};

}

#endif	/* CQL_PROMISE_HPP */
