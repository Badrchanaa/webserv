#ifndef __RingBuffer_HPP__
#define __RingBuffer_HPP__

#include <cstddef>
#include <vector>
#define MIN_BUFFER_SIZE 1024

template <typename T>
class RingBuffer
{
  public:
    explicit RingBuffer(size_t capacity):
        m_Capacity(capacity),
        m_Size(0),
        m_Start(0),
        m_End(0)
    {
        if (m_Capacity >= MIN_BUFFER_SIZE)
            m_Buffer.resize(MIN_BUFFER_SIZE);
    }

    RingBuffer(const RingBuffer<T> &other):
        m_Capacity(other.m_Capacity),
        m_Start(0)
    {
        for (size_t i = 0; i < other.m_Size; i++)
        {
            size_t otherIdx = (other.m_Start + i) % other.m_Capacity;
            m_Buffer.push_back(other.m_Buffer[otherIdx]);
        }
        m_Size = other.m_Size;
        m_End = other.m_Size;
    }

    RingBuffer<T>&  operator=(RingBuffer other)
    {
        std::swap(m_Buffer, other.m_Buffer);
        std::swap(m_Capacity, other.m_Capacity);
        std::swap(m_Size, other.m_Size);
        std::swap(m_Start, other.m_Start);
        std::swap(m_End, other.m_End);
        return *this;
    }

    size_t write(const T *srcBuff, size_t size)
    {
        size_t wbytes;
        wbytes = std::min(m_Capacity - m_Size, size);
        if (m_Buffer.size() < m_Size + wbytes)
            m_Buffer.resize(std::min(m_Capacity, m_Size + wbytes));

        for (size_t i = 0; i < wbytes; i++)
        {
            size_t idx = (m_End + i) % m_Capacity;
            m_Buffer[idx] = srcBuff[i];
        }
        m_End = (m_End + wbytes) % m_Capacity;
        m_Size += wbytes;
        return wbytes;
    }

    size_t read(T *destBuff, size_t size)
    {
        size_t rbytes;
        rbytes = std::min(m_Size, size);
        for (size_t i = 0; i < rbytes; i++)
        {
            size_t idx = (m_Start + i) % m_Capacity;
            destBuff[i] = m_Buffer[idx];
        }
        m_Start = (m_Start + rbytes) % m_Capacity;
        m_Size -= rbytes;
        return rbytes;
    }

    size_t  capacity() const
    {
        return m_Capacity;
    }

    bool    empty() const
    {
        return m_Size == 0;
    }
    
    bool    full() const
    {
        return m_Size == m_Capacity;
    }
    
    size_t  size() const
    {
        return m_Size;
    }

    ~RingBuffer()
    {
    }

  private:
    std::vector<T>  m_Buffer;
    size_t          m_Capacity;
    size_t  m_Size;
    size_t  m_Start;
    size_t  m_End;
};

#endif