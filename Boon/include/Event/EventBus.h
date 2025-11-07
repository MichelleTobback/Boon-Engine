#pragma once
#include "Core/Threading/LockFreeQueue.h"
#include "Event/Event.h"

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <atomic>
#include <thread>
#include <memory>
#include <functional>
#include <condition_variable>
#include <algorithm>

namespace Boon 
{
    struct ListenerBase 
    {
        EventListenerID id;
        virtual ~ListenerBase() = default;
    };

    template<typename EventType>
    struct Listener : ListenerBase 
    {
        std::function<void(const EventType&)> callback;
    };

    class EventBus 
    {
    public:
        EventBus() : m_Stop(false) 
        {
            m_WorkerThread = std::thread([this]() { ProcessQueueThread(); });
        }

        ~EventBus() 
        {
            m_Stop = true;
            m_Condition.notify_all();
            if (m_WorkerThread.joinable())
                m_WorkerThread.join();
        }

        // Subscribe
        template<typename EventType, typename Func>
        EventListenerID Subscribe(Func&& callback) 
        {
            auto listener = std::make_shared<Listener<EventType>>();
            listener->id = m_NextListenerID++;
            listener->callback = std::forward<Func>(callback);
            GetListeners<EventType>().emplace_back(listener);
            return listener->id;
        }

        // Unsubscribe
        template<typename EventType>
        void Unsubscribe(EventListenerID id) 
        {
            auto& listeners = GetListeners<EventType>();
            listeners.erase(
                std::remove_if(listeners.begin(), listeners.end(),
                    [id](auto& l) { return l->id == id; }),
                listeners.end()
            );
        }

        // Dispatch synchronously
        template<typename EventType>
        void Dispatch(const EventType& event) 
        {
            auto& listeners = GetListeners<EventType>();
            for (auto& l : listeners)
                l->callback(event);
        }

        // Post asynchronously
        template<typename EventType>
        void Post(const EventType& event) 
        {
            m_EventQueue.Enqueue([this, event]() { Dispatch(event); });
            m_Condition.notify_one();
        }

    private:
        std::unordered_map<std::type_index, std::vector<std::shared_ptr<ListenerBase>>> m_ListenerMap;

        template<typename EventType>
        std::vector<std::shared_ptr<Listener<EventType>>>& GetListeners() 
        {
            auto type = std::type_index(typeid(EventType));
            auto it = m_ListenerMap.find(type);
            if (it == m_ListenerMap.end()) 
            {
                m_ListenerMap[type] = {};
            }
            
            return *reinterpret_cast<std::vector<std::shared_ptr<Listener<EventType>>>*>(&m_ListenerMap[type]);
        }

        LockFreeQueue<std::function<void()>> m_EventQueue;
        std::mutex m_QueueMutex;
        std::condition_variable m_Condition;
        std::thread m_WorkerThread;
        std::atomic<bool> m_Stop;
        std::atomic<EventListenerID> m_NextListenerID{ 0 };

        void ProcessQueueThread() 
        {
            //return;
            while (!m_Stop) 
            {
                auto job = m_EventQueue.Dequeue();
                if (job) 
                {
                    (*job)();
                }
                else 
                {
                    std::unique_lock<std::mutex> lock(m_QueueMutex);
                    m_Condition.wait_for(lock, std::chrono::milliseconds(1));
                }
            }
        }
    };

} 