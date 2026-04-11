#include "pagedArray.h"
#include <iostream>
#include <cstdlib>
#include <cstring>

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
// Proxy
// =====================================================

PagedArray::Proxy::Proxy(PagedArray& a, long long i) : arr(a), index(i) {}

PagedArray::Proxy& PagedArray::Proxy::operator=(int value) {
    arr.set(index, value);
    return *this;
}

PagedArray::Proxy& PagedArray::Proxy::operator=(const Proxy& other) {
    arr.set(index, other.arr.get(other.index));
    return *this;
}

PagedArray::Proxy::operator int() const {
    return arr.get(index);
}

// =====================================================
// PagedArray
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

    pageSize = pSize;
    pageCount = pCount;

    // Buffer grande para reducir overhead de I/O
    const size_t IO_BUFFER_SIZE = 4 * 1024 * 1024;
    char* ioBuffer = new char[IO_BUFFER_SIZE];
    if (setvbuf(file, ioBuffer, _IOFBF, IO_BUFFER_SIZE) != 0) {
        delete[] ioBuffer;
    }

    if (fileSeek64(file, 0, SEEK_END) != 0) {
        cout << "Error al posicionarse al final del archivo" << endl;
        fclose(file);
        exit(1);
    }

    long long fileBytes = fileTell64(file);
    if (fileBytes < 0) {
        cout << "Error al obtener el tamano del archivo" << endl;
        fclose(file);
        exit(1);
    }

    if (fileBytes % sizeof(int) != 0) {
        cout << "El archivo binario no contiene una cantidad valida de enteros" << endl;
        fclose(file);
        exit(1);
    }

    total = fileBytes / static_cast<long long>(sizeof(int));
    totalPages = (total + static_cast<long long>(pageSize) - 1) / static_cast<long long>(pageSize);

    if (fileSeek64(file, 0, SEEK_SET) != 0) {
        cout << "Error al volver al inicio del archivo" << endl;
        fclose(file);
        exit(1);
    }

    pages = new Page[pageCount];
    for (int i = 0; i < pageCount; i++) {
        pages[i].data = new int[pageSize];
        pages[i].number = -1;
        pages[i].loaded = false;
        pages[i].dirty = false;
        pages[i].referenced = false;
    }

    pageToFrame = new int[totalPages];
    for (long long i = 0; i < totalPages; i++) {
        pageToFrame[i] = -1;
    }

    freeFrames = new int[pageCount];
    freeFrameCount = pageCount;
    for (int i = 0; i < pageCount; i++) {
        freeFrames[i] = pageCount - 1 - i;
    }

    hits = 0;
    faults = 0;

    hasLastPage = false;
    lastPageNumber = -1;
    lastFrame = -1;

    clockHand = 0;
}

PagedArray::~PagedArray() {
    flushAll();

    for (int i = 0; i < pageCount; i++) {
        delete[] pages[i].data;
    }

    delete[] pages;
    delete[] pageToFrame;
    delete[] freeFrames;

    fclose(file);
}

int PagedArray::findLoadedPage(long long pageNumber) {
    if (hasLastPage && lastPageNumber == pageNumber) {
        hits++;
        pages[lastFrame].referenced = true;
        return lastFrame;
    }

    int frame = pageToFrame[pageNumber];
    if (frame != -1) {
        hits++;
        pages[frame].referenced = true;

        hasLastPage = true;
        lastPageNumber = pageNumber;
        lastFrame = frame;

        return frame;
    }

    return -1;
}

int PagedArray::selectVictimFrame() {
    if (freeFrameCount > 0) {
        freeFrameCount--;
        return freeFrames[freeFrameCount];
    }

    while (true) {
        if (!pages[clockHand].referenced) {
            int victim = clockHand;
            clockHand++;
            if (clockHand >= pageCount) {
                clockHand = 0;
            }
            return victim;
        }

        pages[clockHand].referenced = false;
        clockHand++;
        if (clockHand >= pageCount) {
            clockHand = 0;
        }
    }
}

void PagedArray::flushPage(int frame) {
    if (!pages[frame].loaded || !pages[frame].dirty) {
        return;
    }

    long long offset = pages[frame].number * static_cast<long long>(pageSize) * sizeof(int);
    if (fileSeek64(file, offset, SEEK_SET) != 0) {
        cout << "Error al posicionarse en el archivo para escribir una pagina" << endl;
        exit(1);
    }

    long long firstIndex = pages[frame].number * static_cast<long long>(pageSize);
    long long remaining = total - firstIndex;
    int toWrite = (remaining < pageSize) ? static_cast<int>(remaining) : pageSize;

    size_t writeCount = fwrite(pages[frame].data, sizeof(int), toWrite, file);
    if (writeCount != static_cast<size_t>(toWrite)) {
        cout << "Error al escribir una pagina a disco" << endl;
        exit(1);
    }

    pages[frame].dirty = false;
}

int PagedArray::loadPage(long long pageNumber) {
    int frame = findLoadedPage(pageNumber);
    if (frame != -1) {
        return frame;
    }

    faults++;

    int victim = selectVictimFrame();

    if (pages[victim].loaded) {
        if (pages[victim].dirty) {
            flushPage(victim);
        }

        if (pages[victim].number >= 0) {
            pageToFrame[pages[victim].number] = -1;
        }

        if (hasLastPage && lastFrame == victim) {
            hasLastPage = false;
            lastPageNumber = -1;
            lastFrame = -1;
        }
    }

    long long offset = pageNumber * static_cast<long long>(pageSize) * sizeof(int);
    if (fileSeek64(file, offset, SEEK_SET) != 0) {
        cout << "Error al posicionarse en el archivo para cargar una pagina" << endl;
        exit(1);
    }

    long long firstIndex = pageNumber * static_cast<long long>(pageSize);
    long long remaining = total - firstIndex;
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
    pages[victim].referenced = true;

    pageToFrame[pageNumber] = victim;

    hasLastPage = true;
    lastPageNumber = pageNumber;
    lastFrame = victim;

    return victim;
}

int PagedArray::get(long long index) {
    if (index < 0 || index >= total) {
        cout << "Indice fuera de rango: " << index << endl;
        exit(1);
    }

    long long pageNumber = index / pageSize;
    int offset = static_cast<int>(index % pageSize);

    int frame = loadPage(pageNumber);
    return pages[frame].data[offset];
}

void PagedArray::set(long long index, int value) {
    if (index < 0 || index >= total) {
        cout << "Indice fuera de rango: " << index << endl;
        exit(1);
    }

    long long pageNumber = index / pageSize;
    int offset = static_cast<int>(index % pageSize);

    int frame = loadPage(pageNumber);

    if (pages[frame].data[offset] != value) {
        pages[frame].data[offset] = value;
        pages[frame].dirty = true;
    }
}

void PagedArray::prefetch(long long index) {
    if (index < 0 || index >= total) {
        return;
    }

    long long pageNumber = index / pageSize;
    if (pageToFrame[pageNumber] == -1) {
        loadPage(pageNumber);
    } else {
        int frame = pageToFrame[pageNumber];
        pages[frame].referenced = true;
        hasLastPage = true;
        lastPageNumber = pageNumber;
        lastFrame = frame;
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