#pragma once

#include <memory>
#include <chrono>
#include <atomic>
#include <iostream>
#include <stdexcept>

#include "pot/tasks/impl/big_shared_state.h"

namespace pot::tasks
{
    template <typename T>
    class big_task
    {
    public:
        big_task() noexcept {}
        explicit big_task(details::big_shared_state<T> *state)
            : m_state(state) {}

        big_task(big_task &&rhs) noexcept
            : m_state(rhs.m_state)
        {
            rhs.m_state = nullptr;
        }

        big_task &operator=(big_task &&rhs) noexcept
        {
            if (this != &rhs)
            {
                m_state = rhs.m_state;
                rhs.m_state = nullptr;
            }
            return *this;
        }

        big_task(const big_task &) = delete;
        big_task &operator=(const big_task &) = delete;

        explicit operator bool() const noexcept
        {
            return static_cast<bool>(m_state);
        }

        T get()
        {
            if (!m_state)
            {
                throw std::runtime_error("pot::big_task::get() - result is empty.");
            }
            return m_state->get();
        }

        void wait()
        {
            if (!m_state)
            {
                throw std::runtime_error("pot::big_task::wait() - result is empty.");
            }
            m_state->wait();
        }

        template <typename Rep, typename Period>
        bool wait_for(const std::chrono::duration<Rep, Period> &timeout_duration)
        {
            if (!m_state)
            {
                throw std::runtime_error("pot::big_task::wait_for() - result is empty.");
            }
            return m_state->wait_for(timeout_duration);
        }

        template <typename Clock, typename Duration>
        bool wait_until(const std::chrono::time_point<Clock, Duration> &timeout_time)
        {
            if (!m_state)
            {
                throw std::runtime_error("pot::big_task::wait_until() - result is empty.");
            }
            return m_state->wait_until(timeout_time);
        }

        void interrupt() noexcept
        {
            if (!m_state)
            {
                throw std::runtime_error("pot::big_task::interrupt() - result is empty.");
            }
            m_state->interrupt();
        }

        bool is_interrupted() const
        {
            if (!m_state)
            {
                throw std::runtime_error("pot::big_task::is_interrupted() - result is empty.");
            }
            return m_state->is_interrupted();
        }

        void set_progress(int progress)
        {
            if (!m_state)
            {
                throw std::runtime_error("pot::big_task::set_progress() - result is empty.");
            }
            if (progress >= 0 && progress <= 100)
            {
                m_state->set_progress(progress);
            }
            else
            {
                std::cerr << "Wrong progress value: " << progress << std::endl;
            }
        }

        int get_progress() const
        {
            if (!m_state)
            {
                throw std::runtime_error("pot::big_task::get_progress() - result is empty.");
            }
            return m_state->get_progress();
        }

        void pause()
        {
            if (!m_state)
            {
                throw std::runtime_error("pot::big_task::pause() - result is empty.");
            }
            m_state->pause();
        }

        void resume()
        {
            if (!m_state)
            {
                throw std::runtime_error("pot::big_task::resume() - result is empty.");
            }
            m_state->resume();
        }

    private:
        details::big_shared_state<T> *m_state = nullptr;
    };

    template <typename T>
    class big_task_promise
    {
    public:
        big_task_promise() {}

        big_task<T> get_task() noexcept
        {
            return big_task<T>(m_state.get());
        }

        void set_value(T value)
        {
            if (m_state->is_ready())
            {
                throw std::runtime_error("pot::big_task_promise::set_value() - promise already satisfied.");
            }
            m_state->set_value(std::move(value));
        }

        void set_exception(std::exception_ptr e)
        {
            if (m_state->is_ready())
            {
                throw std::runtime_error("pot::big_task_promise::set_exception() - promise already satisfied.");
            }
            m_state->set_exception(std::move(e));
        }

        void set_progress(int progress)
        {
            if (m_state->is_ready())
            {
                throw std::runtime_error("Cannot set progress. Task is already completed.");
            }
            m_state->set_progress(progress);
        }

        int get_progress() const noexcept
        {
            return m_state->get_progress();
        }

        void interrupt()
        {
            m_state->interrupt();
        }

        bool is_interrupted() const noexcept
        {
            return m_state->is_interrupted();
        }

        big_task_promise(const big_task_promise &) = delete;
        big_task_promise &operator=(const big_task_promise &) = delete;

    private:
        std::shared_ptr<details::big_shared_state<T>> m_state{std::make_shared<details::big_shared_state<T>>()};
    };
}
