#include "PagedArray.h"
#include <iostream>
#include <cstdlib>

using namespace std;

// Proxy implementation
PagedArray::Proxy::Proxy(PagedArray& a, long long i) : arr(a), index(i) {}

PagedArray::Proxy& PagedArray::Proxy::operator=(int value) {
    arr.set(index, value);
    return *this;
}

PagedArray::Proxy::operator int() {
    return arr.get(index);
}

// PagedArray implementation
PagedArray::PagedArray(const char* path, int pSize, int pCount) {
    file = fopen(path, "r+b");
    if (file == NULL) {
        cout << "Error al abrir el archivo" << endl;
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    total = ftell(file) / sizeof(int);
    rewind(file);

    pageSize = pSize;
    pageCount = pCount;

    if (pageSize <= 0 || pageCount <= 0) {
        cout << "pageSize y pageCount deben ser mayores que 0" << endl;
        fclose(file);
        exit(1);
    }

    pages = new Page[pageCount];

    for (int i = 0; i < pageCount; i++) {
        pages[i].data = new int[pageSize];
        pages[i].loaded = false;
        pages[i].dirty = false;
        pages[i].number = -1;
        pages[i].lastUsed = 0;
    }

    hits = 0;
    faults = 0;
    counter = 0;
}

PagedArray::~PagedArray() {
    flushAll();

    for (int i = 0; i < pageCount; i++) {
        delete[] pages[i].data;
    }

    delete[] pages;
    fclose(file);
}

int PagedArray::loadPage(long long pageNumber) {
    // Check if page is already loaded (O(1) with unordered_map)
    auto it = loadedPages.find(pageNumber);
    if (it != loadedPages.end()) {
        int frame = it->second;
        hits++;
        pages[frame].lastUsed = ++counter;
        return frame;
    }

    // Page fault
    faults++;

    // Find victim frame
    int victim = -1;
    
    // First try to find an empty frame
    for (int i = 0; i < pageCount; i++) {
        if (!pages[i].loaded) {
            victim = i;
            break;
        }
    }
    
    // If no empty frame, use LRU to find victim
    if (victim == -1) {
        victim = 0;
        for (int i = 1; i < pageCount; i++) {
            if (pages[i].lastUsed < pages[victim].lastUsed) {
                victim = i;
            }
        }
    }

    // Flush victim if dirty
    flushPage(victim);
    
    // Remove victim from loadedPages map if it was loaded
    if (pages[victim].loaded) {
        loadedPages.erase(pages[victim].number);
    }

    // Load new page from disk
    long long offset = pageNumber * (long long)pageSize * sizeof(int);
    fseek(file, offset, SEEK_SET);

    long long remaining = total - pageNumber * (long long)pageSize;
    int toRead = pageSize;

    if (remaining < pageSize) {
        toRead = (int)remaining;
    }

    // Clear the page data first
    for (int i = 0; i < pageSize; i++) {
        pages[victim].data[i] = 0;
    }

    fread(pages[victim].data, sizeof(int), toRead, file);

    pages[victim].number = pageNumber;
    pages[victim].loaded = true;
    pages[victim].dirty = false;
    pages[victim].lastUsed = ++counter;
    
    // Add to loaded pages map
    loadedPages[pageNumber] = victim;

    return victim;
}

void PagedArray::flushPage(int i) {
    if (!pages[i].loaded || !pages[i].dirty) {
        return;
    }

    long long offset = pages[i].number * (long long)pageSize * sizeof(int);
    fseek(file, offset, SEEK_SET);

    long long remaining = total - pages[i].number * (long long)pageSize;
    int toWrite = pageSize;

    if (remaining < pageSize) {
        toWrite = (int)remaining;
    }

    fwrite(pages[i].data, sizeof(int), toWrite, file);
    // Removed fflush here for performance - OS will handle buffering
    
    pages[i].dirty = false;
}

int PagedArray::get(long long index) {
    if (index < 0 || index >= total) {
        cout << "Indice fuera de rango: " << index << endl;
        exit(1);
    }

    long long p = index / pageSize;
    int offset = index % pageSize;

    int frame = loadPage(p);
    return pages[frame].data[offset];
}

void PagedArray::set(long long index, int value) {
    if (index < 0 || index >= total) {
        cout << "Indice fuera de rango: " << index << endl;
        exit(1);
    }

    long long p = index / pageSize;
    int offset = index % pageSize;

    int frame = loadPage(p);
    pages[frame].data[offset] = value;
    pages[frame].dirty = true;
}

PagedArray::Proxy PagedArray::operator[](long long index) {
    return Proxy(*this, index);
}

long long PagedArray::size() {
    return total;
}

long long PagedArray::getHits() {
    return hits;
}

long long PagedArray::getFaults() {
    return faults;
}

void PagedArray::flushAll() {
    for (int i = 0; i < pageCount; i++) {
        flushPage(i);
    }
    fflush(file); // Final flush to ensure all data is written
}