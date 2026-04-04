#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>
#include <string>
#include <algorithm>
#include <cstdlib>
#include "pagedArray.h"

using namespace std;

// =====================================================
// Utilidades
// =====================================================

struct SorterConfig {
    const char* input;
    const char* output;
    const char* algorithm;
    int pageSize;
    int pageCount;
};

bool isSupportedAlgorithm(const char* alg) {
    return strcmp(alg, "quick") == 0 || strcmp(alg, "heap") == 0 ||
           strcmp(alg, "merge") == 0 || strcmp(alg, "shell") == 0 ||
           strcmp(alg, "tim") == 0;
}

void printUsage() {
    cout << "Uso: ./sorter -input <INPUT FILE PATH> -output <OUTPUT FILE PATH> ";
    cout << "-alg <ALGORITMO> -pageSize <PAGE-SIZE> -pageCount <PAGE-COUNT>\n";
    cout << "Algoritmos: quick, heap, merge, shell, tim\n";
}

bool parseArguments(int argc, char* argv[], SorterConfig& cfg) {
    cfg.input = nullptr;
    cfg.output = nullptr;
    cfg.algorithm = nullptr;
    cfg.pageSize = 0;
    cfg.pageCount = 0;

    if (argc != 11) {
        return false;
    }

    for (int i = 1; i < argc; i += 2) {
        if (i + 1 >= argc) {
            return false;
        }

        if (strcmp(argv[i], "-input") == 0) {
            cfg.input = argv[i + 1];
        } else if (strcmp(argv[i], "-output") == 0) {
            cfg.output = argv[i + 1];
        } else if (strcmp(argv[i], "-alg") == 0) {
            cfg.algorithm = argv[i + 1];
        } else if (strcmp(argv[i], "-pageSize") == 0) {
            cfg.pageSize = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-pageCount") == 0) {
            cfg.pageCount = atoi(argv[i + 1]);
        } else {
            return false;
        }
    }

    if (cfg.input == nullptr || cfg.output == nullptr || cfg.algorithm == nullptr) {
        return false;
    }

    if (cfg.pageSize <= 0 || cfg.pageCount <= 0) {
        cout << "pageSize y pageCount deben ser mayores que 0\n";
        return false;
    }

    if (!isSupportedAlgorithm(cfg.algorithm)) {
        cout << "Algoritmo no soportado\n";
        cout << "Use: quick, heap, merge, shell, tim\n";
        return false;
    }

    return true;
}

bool copyFile(const char* in, const char* out) {
    const int BUFFER_SIZE = 64 * 1024 * 1024;
    char* buffer = new char[BUFFER_SIZE];

    ifstream src(in, ios::binary);
    ofstream dst(out, ios::binary | ios::trunc);

    if (!src) {
        cout << "No se pudo abrir el archivo de entrada\n";
        delete[] buffer;
        return false;
    }

    if (!dst) {
        cout << "No se pudo crear el archivo de salida\n";
        delete[] buffer;
        return false;
    }

    while (src) {
        src.read(buffer, BUFFER_SIZE);
        streamsize bytesRead = src.gcount();
        if (bytesRead > 0) {
            dst.write(buffer, bytesRead);
            if (!dst) {
                cout << "Error al copiar el archivo de salida\n";
                delete[] buffer;
                return false;
            }
        }
    }

    delete[] buffer;
    return true;
}

string buildReadableOutputPath(const char* binaryOutputPath) {
    string path(binaryOutputPath);
    return path + ".txt";
}

void generateReadableFile(PagedArray& arr, const char* textFile) {
    ofstream out(textFile);
    if (!out) {
        cout << "Error al crear archivo legible\n";
        return;
    }

    long long n = arr.size();
    cout << "Generando archivo legible (" << n << " enteros)..." << endl;

    const size_t FLUSH_THRESHOLD = 1 << 20;
    string lineBuffer;
    lineBuffer.reserve(FLUSH_THRESHOLD + 1024);

    for (long long i = 0; i < n; i++) {
        if (i > 0) lineBuffer += ",";
        lineBuffer += to_string(arr.get(i));

        if (lineBuffer.size() >= FLUSH_THRESHOLD) {
            out << lineBuffer;
            lineBuffer.clear();
        }

        if (n > 0 && i % (n / 100 + 1) == 0) {
            cout << "Progreso: " << (i * 100 / n) << "%\r" << flush;
        }
    }

    if (!lineBuffer.empty()) {
        out << lineBuffer;
    }

    cout << "\nArchivo legible generado: " << textFile << endl;
}

void swapPaged(PagedArray& arr, long long i, long long j) {
    if (i == j) return;
    int temp = arr.get(i);
    arr.set(i, arr.get(j));
    arr.set(j, temp);
}

// =====================================================
// QUICK SORT (Optimizado con mediana de tres)
// =====================================================

inline long long medianOfThree(PagedArray& arr, long long low, long long high) {
    long long mid = low + (high - low) / 2;

    if (arr.get(low) > arr.get(mid)) {
        swapPaged(arr, low, mid);
    }
    if (arr.get(low) > arr.get(high)) {
        swapPaged(arr, low, high);
    }
    if (arr.get(mid) > arr.get(high)) {
        swapPaged(arr, mid, high);
    }

    return mid;
}

long long partitionQuick(PagedArray& arr, long long low, long long high) {
    long long pivotIndex = medianOfThree(arr, low, high);
    swapPaged(arr, pivotIndex, high);

    int pivot = arr.get(high);
    long long i = low - 1;

    for (long long j = low; j < high; j++) {
        if (arr.get(j) < pivot) {
            i++;
            swapPaged(arr, i, j);
        }
    }

    swapPaged(arr, i + 1, high);
    return i + 1;
}

void quickSort(PagedArray& arr, long long low, long long high) {
    while (low < high) {
        if (high - low < 16) {
            for (long long i = low + 1; i <= high; i++) {
                int key = arr.get(i);
                long long j = i - 1;
                while (j >= low && arr.get(j) > key) {
                    arr.set(j + 1, arr.get(j));
                    j--;
                }
                arr.set(j + 1, key);
            }
            break;
        }

        long long pi = partitionQuick(arr, low, high);

        if (pi - low < high - pi) {
            quickSort(arr, low, pi - 1);
            low = pi + 1;
        } else {
            quickSort(arr, pi + 1, high);
            high = pi - 1;
        }
    }
}

// =====================================================
// HEAP SORT
// =====================================================

void heapify(PagedArray& arr, long long n, long long i) {
    while (true) {
        long long largest = i;
        long long left = 2 * i + 1;
        long long right = 2 * i + 2;

        if (left < n && arr.get(left) > arr.get(largest)) {
            largest = left;
        }

        if (right < n && arr.get(right) > arr.get(largest)) {
            largest = right;
        }

        if (largest == i) {
            break;
        }

        swapPaged(arr, i, largest);
        i = largest;
    }
}

void heapSort(PagedArray& arr, long long n) {
    if (n <= 1) return;

    for (long long i = n / 2; i > 0; --i) {
        heapify(arr, n, i - 1);
    }

    for (long long i = n - 1; i > 0; i--) {
        swapPaged(arr, 0, i);
        heapify(arr, i, 0);
    }
}

// =====================================================
// MERGE SORT (Bottom-Up Iterativo)
// =====================================================

void merge(PagedArray& arr, long long left, long long mid, long long right) {
    long long n1 = mid - left + 1;
    long long n2 = right - mid;

    int* L = new int[n1];
    int* R = new int[n2];

    for (long long i = 0; i < n1; i++) L[i] = arr.get(left + i);
    for (long long j = 0; j < n2; j++) R[j] = arr.get(mid + 1 + j);

    long long i = 0, j = 0, k = left;

    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr.set(k++, L[i++]);
        } else {
            arr.set(k++, R[j++]);
        }
    }

    while (i < n1) arr.set(k++, L[i++]);
    while (j < n2) arr.set(k++, R[j++]);

    delete[] L;
    delete[] R;
}

void mergeSort(PagedArray& arr, long long n) {
    for (long long currSize = 1; currSize < n; currSize *= 2) {
        for (long long leftStart = 0; leftStart < n - 1; leftStart += 2 * currSize) {
            long long mid = leftStart + currSize - 1;
            long long rightEnd = min(leftStart + 2 * currSize - 1, n - 1);

            if (mid < rightEnd) {
                merge(arr, leftStart, mid, rightEnd);
            }
        }
    }
}

// =====================================================
// SHELL SORT (Secuencia de gaps optimizada de Ciura)
// =====================================================

void shellSort(PagedArray& arr, long long n) {
    long long gaps[] = {701, 301, 132, 57, 23, 10, 4, 1};
    int numGaps = sizeof(gaps) / sizeof(gaps[0]);

    for (int g = 0; g < numGaps; g++) {
        long long gap = gaps[g];
        if (gap >= n) continue;

        for (long long i = gap; i < n; i++) {
            int temp = arr.get(i);
            long long j = i;

            while (j >= gap && arr.get(j - gap) > temp) {
                arr.set(j, arr.get(j - gap));
                j -= gap;
            }

            arr.set(j, temp);
        }
    }
}

// =====================================================
// TIM SORT (Híbrido Merge + Insertion)
// =====================================================

const long long RUN = 32;

void insertionSortTim(PagedArray& arr, long long left, long long right) {
    for (long long i = left + 1; i <= right; i++) {
        int temp = arr.get(i);
        long long j = i - 1;

        while (j >= left && arr.get(j) > temp) {
            arr.set(j + 1, arr.get(j));
            j--;
        }

        arr.set(j + 1, temp);
    }
}

void mergeTim(PagedArray& arr, long long l, long long m, long long r) {
    long long len1 = m - l + 1;
    long long len2 = r - m;

    int* left = new int[len1];
    int* right = new int[len2];

    for (long long i = 0; i < len1; i++) left[i] = arr.get(l + i);
    for (long long i = 0; i < len2; i++) right[i] = arr.get(m + 1 + i);

    long long i = 0, j = 0, k = l;

    while (i < len1 && j < len2) {
        if (left[i] <= right[j]) {
            arr.set(k++, left[i++]);
        } else {
            arr.set(k++, right[j++]);
        }
    }

    while (i < len1) arr.set(k++, left[i++]);
    while (j < len2) arr.set(k++, right[j++]);

    delete[] left;
    delete[] right;
}

void timSort(PagedArray& arr, long long n) {
    for (long long i = 0; i < n; i += RUN) {
        insertionSortTim(arr, i, min(i + RUN - 1, n - 1));
    }

    for (long long size = RUN; size < n; size *= 2) {
        for (long long left = 0; left < n; left += 2 * size) {
            long long mid = left + size - 1;
            long long right = min(left + 2 * size - 1, n - 1);

            if (mid < right) {
                mergeTim(arr, left, mid, right);
            }
        }
    }
}

// =====================================================
// MAIN
// =====================================================

int main(int argc, char* argv[]) {
    SorterConfig cfg;
    if (!parseArguments(argc, argv, cfg)) {
        printUsage();
        return 1;
    }

    cout << "========================================\n";
    cout << "Ordenador de Archivos con Paginación\n";
    cout << "========================================\n";
    cout << "Archivo entrada: " << cfg.input << endl;
    cout << "Archivo salida: " << cfg.output << endl;
    cout << "Algoritmo: " << cfg.algorithm << endl;
    cout << "Tamaño página: " << cfg.pageSize << " enteros\n";
    cout << "Cantidad páginas: " << cfg.pageCount << endl;
    cout << "Memoria total: "
         << (static_cast<long long>(cfg.pageSize) * cfg.pageCount * sizeof(int)) / (1024 * 1024)
         << " MB\n";
    cout << "----------------------------------------\n";

    cout << "Copiando archivo..." << endl;
    if (!copyFile(cfg.input, cfg.output)) {
        return 1;
    }

    cout << "Creando PagedArray..." << endl;
    PagedArray arr(cfg.output, cfg.pageSize, cfg.pageCount);
    long long n = arr.size();
    cout << "Total elementos: " << n << " (" << (n * sizeof(int)) / (1024 * 1024) << " MB)\n";

    if (n == 0) {
        cout << "El archivo está vacío. No hay nada que ordenar.\n";
        return 0;
    }

    cout << "Iniciando " << cfg.algorithm << " sort..." << endl;

    auto start = chrono::high_resolution_clock::now();

    if (strcmp(cfg.algorithm, "quick") == 0) {
        quickSort(arr, 0, n - 1);
    } else if (strcmp(cfg.algorithm, "heap") == 0) {
        heapSort(arr, n);
    } else if (strcmp(cfg.algorithm, "merge") == 0) {
        mergeSort(arr, n);
    } else if (strcmp(cfg.algorithm, "shell") == 0) {
        shellSort(arr, n);
    } else if (strcmp(cfg.algorithm, "tim") == 0) {
        timSort(arr, n);
    }

    cout << "Escribiendo cambios al disco..." << endl;
    arr.flushAll();

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;

    string readablePath = buildReadableOutputPath(cfg.output);
    cout << "Generando archivo legible..." << endl;
    generateReadableFile(arr, readablePath.c_str());

    cout << "========================================\n";
    cout << "RESULTADOS\n";
    cout << "========================================\n";
    cout << "Algoritmo: " << cfg.algorithm << endl;
    cout << "Tiempo total: " << elapsed.count() << " segundos\n";
    cout << "Page Hits: " << arr.getHits() << endl;
    cout << "Page Faults: " << arr.getFaults() << endl;

    long long totalAccesses = arr.getHits() + arr.getFaults();
    double hitRate = (totalAccesses == 0)
        ? 0.0
        : (static_cast<double>(arr.getHits()) / totalAccesses) * 100.0;

    cout << "Hit Rate: " << hitRate << "%\n";
    cout << "Archivo legible: " << readablePath << endl;
    cout << "========================================\n";

    return 0;
}