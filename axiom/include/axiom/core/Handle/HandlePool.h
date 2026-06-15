#pragma once

#include "Handle.h"

namespace axiom {
    template<typename T>
    class HandlePool {
    public:
        Handle Create(const T& value) {
            uint32_t index;

            if (!m_FreeList.empty()) {
                index = m_FreeList.back();
                m_FreeList.pop_back();
                m_Data[index] = value;
            }
            else {
                index = (uint32_t)m_Data.size();
                m_Data.push_back(value);
                m_Generation.push_back(0);
            }

            return { index, m_Generation[index] };
        }

        T* Get(Handle h) {
            if (!IsValid(h)) return nullptr;
            return &m_Data[h.index];
        }

        void Remove(Handle h) {
            if (!IsValid(h)) return;

            m_Generation[h.index]++;
            m_FreeList.push_back(h.index);
        }

    private:
        bool IsValid(Handle h) const {
            return h.index < m_Data.size() &&
                m_Generation[h.index] == h.generation;
        }

    private:
        std::vector<T> m_Data;
        std::vector<uint32_t> m_Generation;
        std::vector<uint32_t> m_FreeList;
    };
}