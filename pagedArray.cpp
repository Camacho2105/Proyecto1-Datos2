#include "pagedArray.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cerrno>

using namespace std;

namespace {
#ifdef _WIN32
    int fileSeek64(FILE* file, long long offset, int origin) {
        return _fseeki64(file, offset, origin);
    }

    long long fileTell64(FILE* file) {
        return _ftelli64(file);
    }
#else
    int fileSeek64(FILE* file, long long offset, int origin) {
        return fseeko(file, static_cast<off_t>(offset), origin);
    }

    long long fileTell64(FILE* file) {
        return static_cast<long long>(ftello(file));
    }
#endif
}

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

    if (pSize <= 0 || pCount <= 0) {
        cout << "pageSize y pageCount deben ser mayores que 0" << endl;
        fclose(file);
        exit(1);
    }

    if (fileSeek64(file, 0, SEEK_END) != 0) {
        cout << "Error al posicionarse al final del archivo" << endl;
        fclose(file);
        exit(1);
    }

    long long fileBytes = fileTell64(file);
    if (fileBytes < 0) {
        cout << "Error al obtener el tamaño del archivo" << endl;
        fclose(file);
        exit(1);
    }

    if (fileBytes % sizeof(int) != 0) {
        cout << "El archivo binario no contiene una cantidad valida de enteros" << endl;
        fclose(file);
        exit(1);
    }

    total = fileBytes / static_cast<long long>(sizeof(int));

    if (fileSeek64(file, 0, SEEK_SET) != 0) {
        cout << "Error al volver al inicio del archivo" << endl;
        fclose(file);
        exit(1);
    }

    pageSize = pSize;
    pageCount = pCount;

    pages = new Page[pageCount];
    loadedPages.reserve(pageCount * 2);

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
    auto it = loadedPages.find(pageNumber);
    if (it != loadedPages.end()) {
        hits++;
        pages[it->second].lastUsed = ++counter;
        return it->second;
    }

    faults++;

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

    if (pages[victim].loaded) {
        if (pages[victim].dirty) {
            flushPage(victim);
        }
        loadedPages.erase(pages[victim].number);
    }

    long long offset = pageNumber * static_cast<long long>(pageSize) * sizeof(int);
    if (fileSeek64(file, offset, SEEK_SET) != 0) {
        cout << "Error al posicionarse en el archivo para cargar una pagina" << endl;
        exit(1);
    }

    long long remaining = total - pageNumber * static_cast<long long>(pageSize);
    int toRead = (remaining < pageSize) ? static_cast<int>(remaining) : pageSize;

    if (toRead < pageSize) {
        memset(pages[victim].data, 0, pageSize * sizeof(int));
    }

    size_t readCount = fread(pages[victim].data, sizeof(int), toRead, file);
    if (readCount != static_cast<size_t>(toRead)) {
        cout << "Error al leer una pagina desde disco" << endl;
        exit(1);
    }

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

    long long offset = pages[i].number * static_cast<long long>(pageSize) * sizeof(int);
    if (fileSeek64(file, offset, SEEK_SET) != 0) {
        cout << "Error al posicionarse en el archivo para escribir una pagina" << endl;
        exit(1);
    }

    long long remaining = total - pages[i].number * static_cast<long long>(pageSize);
    int toWrite = (remaining < pageSize) ? static_cast<int>(remaining) : pageSize;

    size_t writeCount = fwrite(pages[i].data, sizeof(int), toWrite, file);
    if (writeCount != static_cast<size_t>(toWrite)) {
        cout << "Error al escribir una pagina a disco" << endl;
        exit(1);
    }

    pages[i].dirty = false;
}

int PagedArray::get(long long index) {
    if (index < 0 || index >= total) {
        cout << "Indice fuera de rango: " << index << endl;
        exit(1);
    }

    long long p = index / pageSize;
    int offset = static_cast<int>(index % pageSize);

    int frame = loadPage(p);
    return pages[frame].data[offset];
}

void PagedArray::set(long long index, int value) {
    if (index < 0 || index >= total) {
        cout << "Indice fuera de rango: " << index << endl;
        exit(1);
    }

    long long p = index / pageSize;
    int offset = static_cast<int>(index % pageSize);

    int frame = loadPage(p);
    pages[frame].data[offset] = value;
    pages[frame].dirty = true;
}

void PagedArray::prefetch(long long index) {
    if (index < 0 || index >= total) return;

    long long pageNum = index / pageSize;
    if (loadedPages.find(pageNum) == loadedPages.end()) {
        loadPage(pageNum);
    }
}

PagedArray::Proxy PagedArray::operator[](long long index) {
    return Proxy(*this, index);
}

long long PagedArray::size() const {
    return total;
}

long long PagedArray::getHits() const {
    return hits;
}

long long PagedArray::getFaults() const {
    return faults;
}

void PagedArray::flushAll() {
    for (int i = 0; i < pageCount; i++) {
        flushPage(i);
    }

    if (fflush(file) != 0) {
        cout << "Error al vaciar el buffer del archivo" << endl;
        exit(1);
    }
}