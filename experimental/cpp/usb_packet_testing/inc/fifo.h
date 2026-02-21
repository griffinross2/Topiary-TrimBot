#pragma once

template <typename T, size_t Capacity>
class FIFO
{
public:
    FIFO() : m_head(0), m_tail(0) {};

    bool is_empty() const { return m_num_items == 0; }
    bool is_full() const { return m_num_items == Capacity; }
    int num_items() const
    {
        return m_num_items;
    }
    int space_left() const
    {
        return Capacity - m_num_items;
    }
    int push(const T *item, size_t count = 1)
    {
        if (count > Capacity - m_num_items)
        {
            // Check if enough space
            return -1;
        }

        for (size_t i = 0; i < count; i++)
        {
            m_buffer[m_tail] = item[i];
            m_tail = (m_tail + 1) % Capacity;
        }
        m_num_items += count;

        return 0;
    }
    int pop(T *item, size_t count = 1)
    {
        if (count > m_num_items)
        {
            // Check if enough items
            return -1;
        }

        for (size_t i = 0; i < count; i++)
        {
            item[i] = m_buffer[m_head];
            m_head = (m_head + 1) % Capacity;
        }
        m_num_items -= count;

        return 0;
    }

private:
    T m_buffer[Capacity];
    size_t m_num_items;
    size_t m_head;
    size_t m_tail;
};