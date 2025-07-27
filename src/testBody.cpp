#include "RingBuffer.hpp"
#include <string.h>
#include <iostream>

void printbuff(unsigned char *buff, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        std::cout << buff[i];
    }
    std::cout << std::endl;
    std::cout << " ---------------" << std::endl;
}

int main() {
    unsigned char buff[64];
    unsigned char out[64];

    // Helper to fill buffer with a character
    auto fill_buff = [](unsigned char* b, size_t size, unsigned char c) {
        for (size_t i = 0; i < size; i++) b[i] = c;
    };

    // 1. Write zero bytes
    {
        RingBuffer<unsigned char> rb(10);
        size_t written = rb.write(buff, 0);
        std::cout << "Write zero bytes: " << written << " (expected 0)\n";
        std::cout << "size: " << rb.size() << " (expected 0)\n";
    }

    // 2. Read zero bytes
    {
        RingBuffer<unsigned char> rb(10);
        size_t read = rb.read(out, 0);
        std::cout << "Read zero bytes: " << read << " (expected 0)\n";
        std::cout << "size: " << rb.size() << " (expected 0)\n";
    }

    // 3. Write more than capacity
    {
        RingBuffer<unsigned char> rb(10);
        fill_buff(buff, 20, 'X');
        size_t written = rb.write(buff, 20);
        std::cout << "Write more than capacity: " << written << " (expected 10)\n";
        std::cout << "size: " << rb.size() << " (expected 10)\n";
    }

    // 4. Read more than size
    {
        RingBuffer<unsigned char> rb(10);
        fill_buff(buff, 5, 'A');
        rb.write(buff, 5);
        size_t read = rb.read(out, 10);
        std::cout << "Read more than size: " << read << " (expected 5)\n";
        std::cout << "size: " << rb.size() << " (expected 0)\n";
    }

    // 5. Write and read exactly capacity
    {
        RingBuffer<unsigned char> rb(10);
        fill_buff(buff, 10, 'B');
        rb.write(buff, 10);
        size_t read = rb.read(out, 10);
        std::cout << "Write and read exactly capacity: " << read << " (expected 10)\n";
        std::cout << "size: " << rb.size() << " (expected 0)\n";
    }

    // 6. Write-read-write-read with wrap-around
    {
        RingBuffer<unsigned char> rb(10);
        fill_buff(buff, 6, 'C');
        rb.write(buff, 6);
        size_t r1 = rb.read(out, 4); // read 4
        std::cout << "Read 4 bytes: " << r1 << " (expected 4)\n";
        fill_buff(buff, 5, 'D');
        size_t w2 = rb.write(buff, 5);
        std::cout << "Write 5 bytes after read: " << w2 << " (expected 5)\n";
        size_t r2 = rb.read(out, 7);
        std::cout << "Read remaining bytes: " << r2 << " (expected 7)\n";
        std::cout << "size: " << rb.size() << " (expected 0)\n";
    }

    // 7. Copy constructor test
    {
        RingBuffer<unsigned char> rb(10);
        fill_buff(buff, 8, 'E');
        rb.write(buff, 8);
        RingBuffer<unsigned char> copy(rb);
        size_t readOrig = rb.read(out, 8);
        size_t readCopy = copy.read(out, 8);
        std::cout << "Copy constructor original read: " << readOrig << " (expected 8)\n";
        std::cout << "Copy constructor copy read: " << readCopy << " (expected 8)\n";
        std::cout << "size: " << rb.size() << " (expected 0)\n";
    }

    // 8. Assignment operator test
    {
        RingBuffer<unsigned char> rb1(10);
        RingBuffer<unsigned char> rb2(5);
        fill_buff(buff, 5, 'F');
        rb1.write(buff, 5);
        rb2 = rb1;
        size_t read2 = rb2.read(out, 5);
        std::cout << "Assignment operator read: " << read2 << " (expected 5)\n";
    }

    // 9. Empty buffer read
    {
        RingBuffer<unsigned char> rb(10);
        size_t read = rb.read(out, 5);
        std::cout << "Read from empty buffer: " << read << " (expected 0)\n";
    }

    // 10. Full buffer write
    {
        RingBuffer<unsigned char> rb(5);
        fill_buff(buff, 5, 'G');
        rb.write(buff, 5);
        size_t write2 = rb.write(buff, 3);
        std::cout << "Write to full buffer: " << write2 << " (expected 0)\n";
    }

    return 0;
}
