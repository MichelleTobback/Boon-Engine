#pragma once
#include <atomic>
#include <thread>
#include <vector>
#include <memory>

namespace Boon {

    struct HazardRecord 
    {
        std::atomic<void*> ptr{ nullptr };
    };

    class HazardPointer 
    {
    public:
        static constexpr int MAX_THREADS = 128;

        static HazardRecord* Acquire() 
        {
            for (int i = 0; i < MAX_THREADS; ++i) 
            {
                void* expected = nullptr;
                if (s_Records[i].ptr.compare_exchange_strong(expected, reinterpret_cast<void*>(1))) {
                    return &s_Records[i];
                }
            }
            return nullptr;
        }

        static void Release(HazardRecord* record) 
        {
            record->ptr.store(nullptr);
        }

        static bool IsHazard(void* ptr) 
        {
            for (int i = 0; i < MAX_THREADS; ++i)
                if (s_Records[i].ptr.load() == ptr)
                    return true;
            return false;
        }

    private:
        static inline HazardRecord s_Records[MAX_THREADS];
    };

}