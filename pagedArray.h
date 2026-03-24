#ifndef PAGEDARRAY_H
#define PAGEDARRAY_H

#include <cstdio>

class PagedArray {
private:
    struct Page {
        int* data;
        long long number;
        bool loaded;
        bool dirty;
        long long lastUsed;
    };

    FILE* file;
    long long total;
    int pageSize;
    int pageCount;
    Page* pages;

    long long hits;
    long long faults;
    long long counter;

    int loadPage(long long pageNumber);
    void flushPage(int i);

public:
    PagedArray(const char* path, int pSize, int pCount);
    ~PagedArray();

    int get(long long index);
    void set(long long index, int value);

    long long size();
    long long getHits();
    long long getFaults();

    void flushAll();

    class Proxy {
    private:
        PagedArray& arr;
        long long index;

    public:
        Proxy(PagedArray& a, long long i);
        Proxy& operator=(int value);
        operator int();
    };

    Proxy operator[](long long index);
};

#endif