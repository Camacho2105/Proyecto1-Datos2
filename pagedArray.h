#ifndef PAGEDARRAY_H
#define PAGEDARRAY_H

#include <cstdio>
#include <unordered_map>

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
    std::unordered_map<long long, int> loadedPages;

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

    long long size() const;
    long long getHits() const;
    long long getFaults() const;

    void flushAll();
    void prefetch(long long index);

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