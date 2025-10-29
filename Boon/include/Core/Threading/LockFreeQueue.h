#pragma once
#include <atomic>
#include <memory>
#include "HazardPointer.h"

namespace Boon {

    template<typename T>
    class LockFreeQueue 
    {
    private:
        struct Node 
        {
            std::shared_ptr<T> data;
            std::atomic<Node*> next{ nullptr };
            Node(T const& value) : data(std::make_shared<T>(value)) {}
        };

        std::atomic<Node*> m_Head;
        std::atomic<Node*> m_Tail;

    public:
        LockFreeQueue() 
        {
            Node* dummy = new Node(T{});
            m_Head.store(dummy);
            m_Tail.store(dummy);
        }

        ~LockFreeQueue() 
        {
            while (Node* old = m_Head.load()) 
            {
                m_Head.store(old->next);
                delete old;
            }
        }

        void Enqueue(T const& value) 
        {
            Node* node = new Node(value);
            Node* oldTail = nullptr;

            while (true) 
            {
                oldTail = m_Tail.load();
                Node* next = oldTail->next.load();
                if (oldTail == m_Tail.load()) 
                {
                    if (next == nullptr) 
                    {
                        if (oldTail->next.compare_exchange_weak(next, node)) 
                        {
                            m_Tail.compare_exchange_weak(oldTail, node);
                            return;
                        }
                    }
                    else 
                    {
                        m_Tail.compare_exchange_weak(oldTail, next);
                    }
                }
            }
        }

        std::shared_ptr<T> Dequeue() 
        {
            while (true) 
            {
                Node* oldHead = m_Head.load();
                Node* oldTail = m_Tail.load();
                Node* next = oldHead->next.load();

                // Acquire a hazard record
                HazardRecord* rec = HazardPointer::Acquire();
                rec->ptr.store(next);

                if (oldHead == m_Head.load()) 
                {
                    if (oldHead == oldTail) 
                    {
                        if (next == nullptr) 
                        {
                            HazardPointer::Release(rec);
                            return nullptr; // queue is empty
                        }
                        m_Tail.compare_exchange_weak(oldTail, next);
                    }
                    else 
                    {
                        std::shared_ptr<T> result = next->data;
                        if (m_Head.compare_exchange_weak(oldHead, next)) 
                        {
                            rec->ptr.store(nullptr);
                            HazardPointer::Release(rec);
                            delete oldHead; // safe to delete
                            return result;
                        }
                    }
                }
                HazardPointer::Release(rec);
            }
        }
    };

}