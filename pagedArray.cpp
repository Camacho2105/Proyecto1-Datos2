#include "PagedArray.h"
#include <iostream>
#include <cstdlib>
#include <cstring>

using namespace std;

// =====================================================
// Proxy implementation
// =====================================================

PagedArray::Proxy::Proxy(PagedArray& a, long long i) : arr(a), index(i) {}

PagedArray::Proxy& PagedArray::Proxy::operator=(int value) {
    arr.set(index, value);
    return *this;
}

PagedArray::Proxy::operator int() {
    return arr.get(index);
}

// =====================================================
// PagedArray implementation
// =====================================================

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
    // Verificar si ya está cargada
    auto it = loadedPages.find(pageNumber);
    if (it != loadedPages.end()) {
        hits++;
        pages[it->second].lastUsed = ++counter;
        return it->second;
    }

    // Page fault
    faults++;

    // Buscar víctima: primero página vacía, luego LRU
    int victim = -1;
    
    for (int i = 0; i < pageCount; i++) {
        if (!pages[i].loaded) {
            victim = i;
            break;
        }
    }
    
    if (victim == -1) {
        victim = 0;
        for (int i = 1; i < pageCount; i++) {
            if (pages[i].lastUsed < pages[victim].lastUsed) {
                victim = i;
            }
        }
    }

    // Flush si es necesario
    if (pages[victim].loaded) {
        if (pages[victim].dirty) {
            flushPage(victim);
        }
        loadedPages.erase(pages[victim].number);
    }
    
    // Calcular offset y lectura
    long long offset = pageNumber * (long long)pageSize * sizeof(int);
    fseek(file, offset, SEEK_SET);

    long long remaining = total - pageNumber * (long long)pageSize;
    int toRead = (remaining < pageSize) ? (int)remaining : pageSize;

    // Limpiar página solo si es parcial
    if (toRead < pageSize) {
        memset(pages[victim].data, 0, pageSize * sizeof(int));
    }

    fread(pages[victim].data, sizeof(int), toRead, file);

    pages[victim].number = pageNumber;
    pages[victim].loaded = true;
    pages[victim].dirty = false;
    pages[victim].lastUsed = ++counter;
    
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
    int toWrite = (remaining < pageSize) ? (int)remaining : pageSize;

    fwrite(pages[i].data, sizeof(int), toWrite, file);
    
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

void PagedArray::prefetch(long long index) {
    if (index < 0 || index >= total) return;
    
    long long pageNum = index / pageSize;
    auto it = loadedPages.find(pageNum);
    
    if (it == loadedPages.end()) {
        loadPage(pageNum);
    }
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
    fflush(file);
}