//
// Copyright (c) 2008, 2009 Boris Schaeling <boris@highscore.de>
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "dir_monitor_impl.hpp"
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <filesystem>
#include <boost/system/error_code.hpp>

#include <memory>
#include <string>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>

namespace boost {
namespace asio {

template <typename DirMonitorImplementation = dir_monitor_impl>
class basic_dir_monitor_service
    : public boost::asio::io_context::service
{
public:
    static boost::asio::io_context::id id;

    explicit basic_dir_monitor_service(boost::asio::io_context &io_context_in)
        : boost::asio::io_context::service(io_context_in),
        async_monitor_work_(boost::asio::make_work_guard(async_monitor_io_context_)),
          async_monitor_thread_([&]() {
#ifndef _WIN32
#ifdef __APPLE__
              pthread_setname_np("dir_mon_svc");
#else
              pthread_setname_np(pthread_self(), "dir_mon_svc");
#endif
#endif
              async_monitor_io_context_.run();
          })
    {
    }

    typedef std::shared_ptr<DirMonitorImplementation> implementation_type;

    void construct(implementation_type &impl)
    {
        impl.reset(new DirMonitorImplementation());
        impl->begin_read();
    }

    void destroy(implementation_type &impl)
    {
        // If an asynchronous call is currently waiting for an event
        // we must interrupt the blocked call to make sure it returns.
        impl->destroy();

        impl.reset();
    }

    void add_directory(implementation_type &impl, const std::string &dirname)
    {
        if (!std::filesystem::is_directory(dirname))
            throw std::invalid_argument("boost::asio::basic_dir_monitor_service::add_directory: " + dirname + " is not a valid directory entry");

        impl->add_directory(dirname);
    }

    void remove_directory(implementation_type &impl, const std::string &dirname)
    {
        impl->remove_directory(dirname);
    }

    dir_monitor_event monitor(implementation_type &impl, boost::system::error_code &ec)
    {
        return impl->popfront_event(ec);
    }

    template <typename Handler>
    class monitor_operation
    {
    public:
        monitor_operation(implementation_type &impl, boost::asio::io_context &io_context_in, Handler handler)
            : impl_(impl),
            io_context_(io_context_in),
            work_(make_work_guard(io_context_in)),
            handler_(handler)
        {
        }

        void operator()() const
        {
            implementation_type impl = impl_.lock();
            boost::system::error_code ec = boost::asio::error::operation_aborted;
            dir_monitor_event ev;
            if (impl)
                ev = impl->popfront_event(ec);
            PostAndWait(ec, ev);
        }

    protected:
        void PostAndWait(const boost::system::error_code ec, const dir_monitor_event& ev) const
        {
            if (ev.type != dir_monitor_event::null) {
                std::mutex post_mutex;
                std::condition_variable post_condition_variable;
                bool post_cancel = false;

            	boost::asio::post(this->io_context_,
                    [&] {
                        handler_(ec, ev);
                        std::lock_guard<std::mutex> lock(post_mutex);
                        post_cancel = true;
                        post_condition_variable.notify_one();
                    }
                );
                std::unique_lock<std::mutex> lock(post_mutex);
                while (!post_cancel)
                    post_condition_variable.wait(lock);
            }
        }

    private:
        std::weak_ptr<DirMonitorImplementation> impl_;
        boost::asio::io_context &io_context_;
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_;
        Handler handler_;
    };

    template <typename Handler>
    void async_monitor(implementation_type &impl, Handler handler)
    {
#if BOOST_VERSION < 106600
        boost::asio::post(this->async_monitor_io_context_, monitor_operation<Handler>(impl, this->get_io_service(), handler));
#else
        boost::asio::post(this->async_monitor_io_context_, monitor_operation<Handler>(impl, this->get_io_context(), handler));
#endif
    }

private:
    virtual void shutdown() override
    {
        // The async_monitor thread will finish when async_monitor_work_ is reset as all asynchronous
        // operations have been aborted and were discarded before (in destroy).
        async_monitor_work_.reset();

        // Event processing is stopped to discard queued operations.
        async_monitor_io_context_.stop();

        // The async_monitor thread is joined to make sure the directory monitor service is
        // destroyed _after_ the thread is finished (not that the thread tries to access
        // instance properties which don't exist anymore).
		if (async_monitor_thread_.joinable()) {
			async_monitor_thread_.join();
		}
    }

    boost::asio::io_context async_monitor_io_context_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> async_monitor_work_;
    std::thread async_monitor_thread_;
};

template <typename DirMonitorImplementation>
boost::asio::io_context::id basic_dir_monitor_service<DirMonitorImplementation>::id;

}
}

