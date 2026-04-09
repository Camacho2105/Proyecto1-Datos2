#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>
#include <string>
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
    return strcmp(alg, "quick") == 0 ||
           strcmp(alg, "heap") == 0 ||
           strcmp(alg, "merge") == 0 ||
           strcmp(alg, "shell") == 0 ||
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
        if (i > 0) {
            lineBuffer += ",";
        }

        lineBuffer += to_string((int)arr[i]);

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

long long minLong(long long a, long long b) {
    return (a < b) ? a : b;
}

bool isSorted(PagedArray& arr, long long n) {
    for (long long i = 1; i < n; i++) {
        if ((int)arr[i - 1] > (int)arr[i]) {
            return false;
        }
    }
    return true;
}

// =====================================================
// QUICK SORT
// =====================================================

void quickSort(PagedArray& arr, long long low, long long high) {
    long long i, j;
    int temp, pivotValue;

    if (low < high) {
        i = low;
        j = high;
        pivotValue = arr[low];

        while (i < j) {
            while (i <= high && (int)arr[i] <= pivotValue) {
                i++;
            }

            while ((int)arr[j] > pivotValue) {
                j--;
            }

            if (i < j) {
                temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }

        temp = arr[low];
        arr[low] = arr[j];
        arr[j] = temp;

        quickSort(arr, low, j - 1);
        quickSort(arr, j + 1, high);
    }
}

// =====================================================
// HEAP SORT
// =====================================================

void heapify(PagedArray& arr, long long n, long long i) {
    long long largest = i;
    long long left = 2 * i + 1;
    long long right = 2 * i + 2;
    int temp;

    if (left < n && (int)arr[left] > (int)arr[largest]) {
        largest = left;
    }

    if (right < n && (int)arr[right] > (int)arr[largest]) {
        largest = right;
    }

    if (largest != i) {
        temp = arr[i];
        arr[i] = arr[largest];
        arr[largest] = temp;

        heapify(arr, n, largest);
    }
}

void heapSort(PagedArray& arr, long long n) {
    int temp;

    if (n <= 1) {
        return;
    }

    for (long long i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i);
        if (i == 0) {
            break;
        }
    }

    for (long long i = n - 1; i > 0; i--) {
        temp = arr[0];
        arr[0] = arr[i];
        arr[i] = temp;

        heapify(arr, i, 0);
    }
}

// =====================================================
// MERGE SORT
// =====================================================

void merge(PagedArray& arr, long long left, long long mid, long long right) {
    long long n1 = mid - left + 1;
    long long n2 = right - mid;

    int* L = new int[n1];
    int* R = new int[n2];

    for (long long i = 0; i < n1; i++) {
        L[i] = arr[left + i];
    }

    for (long long j = 0; j < n2; j++) {
        R[j] = arr[mid + 1 + j];
    }

    long long i = 0;
    long long j = 0;
    long long k = left;

    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }

    delete[] L;
    delete[] R;
}

void mergeSort(PagedArray& arr, long long left, long long right) {
    if (left < right) {
        long long mid = left + (right - left) / 2;

        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

// =====================================================
// SHELL SORT
// =====================================================

void shellSort(PagedArray& arr, long long n) {
    for (long long gap = n / 2; gap > 0; gap /= 2) {
        for (long long i = gap; i < n; i++) {
            int temp = arr[i];
            long long j = i;

            while (j >= gap && (int)arr[j - gap] > temp) {
                arr[j] = arr[j - gap];
                j -= gap;
            }

            arr[j] = temp;
        }
    }
}

// =====================================================
// TIM SORT
// =====================================================

const long long RUN = 32;

void insertionSortTim(PagedArray& arr, long long left, long long right) {
    for (long long i = left + 1; i <= right; i++) {
        int temp = arr[i];
        long long j = i - 1;

        while (j >= left && (int)arr[j] > temp) {
            arr[j + 1] = arr[j];
            j--;
        }

        arr[j + 1] = temp;
    }
}

void mergeTim(PagedArray& arr, long long l, long long m, long long r) {
    long long len1 = m - l + 1;
    long long len2 = r - m;

    int* left = new int[len1];
    int* right = new int[len2];

    for (long long i = 0; i < len1; i++) {
        left[i] = arr[l + i];
    }

    for (long long i = 0; i < len2; i++) {
        right[i] = arr[m + 1 + i];
    }

    long long i = 0;
    long long j = 0;
    long long k = l;

    while (i < len1 && j < len2) {
        if (left[i] <= right[j]) {
            arr[k] = left[i];
            i++;
        } else {
            arr[k] = right[j];
            j++;
        }
        k++;
    }

    while (i < len1) {
        arr[k] = left[i];
        i++;
        k++;
    }

    while (j < len2) {
        arr[k] = right[j];
        j++;
        k++;
    }

    delete[] left;
    delete[] right;
}

void timSort(PagedArray& arr, long long n) {
    for (long long i = 0; i < n; i += RUN) {
        insertionSortTim(arr, i, minLong(i + RUN - 1, n - 1));
    }

    for (long long size = RUN; size < n; size = 2 * size) {
        for (long long left = 0; left < n; left += 2 * size) {
            long long mid = left + size - 1;
            long long right = minLong(left + 2 * size - 1, n - 1);

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
    cout << "Ordenador de Archivos con Paginacion\n";
    cout << "========================================\n";
    cout << "Archivo entrada: " << cfg.input << endl;
    cout << "Archivo salida: " << cfg.output << endl;
    cout << "Algoritmo: " << cfg.algorithm << endl;
    cout << "Tamano pagina: " << cfg.pageSize << " enteros\n";
    cout << "Cantidad paginas: " << cfg.pageCount << endl;

    double totalMB = (static_cast<double>(cfg.pageSize) * cfg.pageCount * sizeof(int)) / (1024.0 * 1024.0);
    cout << "Memoria total: " << totalMB << " MB\n";
    cout << "----------------------------------------\n";

    cout << "Copiando archivo..." << endl;
    if (!copyFile(cfg.input, cfg.output)) {
        return 1;
    }

    cout << "Creando PagedArray..." << endl;
    PagedArray arr(cfg.output, cfg.pageSize, cfg.pageCount);
    long long n = arr.size();

    double fileMB = (static_cast<double>(n) * sizeof(int)) / (1024.0 * 1024.0);
    cout << "Total elementos: " << n << " (" << fileMB << " MB)\n";

    if (n == 0) {
        cout << "El archivo esta vacio. No hay nada que ordenar.\n";
        return 0;
    }

    cout << "Iniciando " << cfg.algorithm << " sort..." << endl;

    auto start = chrono::high_resolution_clock::now();

    if (strcmp(cfg.algorithm, "quick") == 0) {
        quickSort(arr, 0, n - 1);
    } else if (strcmp(cfg.algorithm, "heap") == 0) {
        heapSort(arr, n);
    } else if (strcmp(cfg.algorithm, "merge") == 0) {
        mergeSort(arr, 0, n - 1);
    } else if (strcmp(cfg.algorithm, "shell") == 0) {
        shellSort(arr, n);
    } else if (strcmp(cfg.algorithm, "tim") == 0) {
        timSort(arr, n);
    }

    cout << "Escribiendo cambios al disco..." << endl;
    arr.flushAll();

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;

    bool sorted = isSorted(arr, n);

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
    double hitRate = 0.0;

    if (totalAccesses > 0) {
        hitRate = (static_cast<double>(arr.getHits()) / totalAccesses) * 100.0;
    }

    cout << "Hit Rate: " << hitRate << "%\n";
    cout << "Ordenado correctamente: " << (sorted ? "SI" : "NO") << endl;
    cout << "Archivo legible: " << readablePath << endl;
    cout << "========================================\n";

    return 0;
}