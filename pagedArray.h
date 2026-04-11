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
        bool referenced;
    };

    FILE* file;
    long long total;
    long long totalPages;
    int pageSize;
    int pageCount;

    Page* pages;
    int* pageToFrame;

    long long hits;
    long long faults;

    int freeFrameCount;
    int* freeFrames;

    long long lastPageNumber;
    int lastFrame;
    bool hasLastPage;

    int clockHand;

    int findLoadedPage(long long pageNumber);
    int loadPage(long long pageNumber);
    int selectVictimFrame();
    void flushPage(int frame);

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
        Proxy& operator=(const Proxy& other);
        operator int() const;
    };

    Proxy operator[](long long index);
};

#endif