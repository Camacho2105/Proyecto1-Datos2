#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>
#include "PagedArray.h"

using namespace std;

// =====================================================
// Utilidades Optimizadas
// =====================================================

void copyFile(const char* in, const char* out) {
    const int BUFFER_SIZE = 64 * 1024 * 1024; // 64MB buffer
    char* buffer = new char[BUFFER_SIZE];
    
    ifstream src(in, ios::binary);
    ofstream dst(out, ios::binary);
    
    while (src.read(buffer, BUFFER_SIZE)) {
        dst.write(buffer, src.gcount());
    }
    dst.write(buffer, src.gcount());
    
    delete[] buffer;
}

void generateReadableFile(PagedArray& arr, const char* textFile) {
    ofstream out(textFile);
    if (!out) {
        cout << "Error al crear archivo legible\n";
        return;
    }
    
    long long n = arr.size();
    cout << "Generando archivo legible (" << n << " enteros)..." << endl;
    
    // Buffer para reducir llamadas a write
    const int LINE_BUFFER = 10000;
    string lineBuffer;
    lineBuffer.reserve(LINE_BUFFER * 12);
    
    for (long long i = 0; i < n; i++) {
        if (i > 0) lineBuffer += ",";
        lineBuffer += to_string((int)arr[i]);
        
        if (lineBuffer.size() > LINE_BUFFER) {
            out << lineBuffer;
            lineBuffer.clear();
        }
        
        if (i % (n / 100 + 1) == 0) {
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
        int temp = arr.get(low);
        arr.set(low, arr.get(mid));
        arr.set(mid, temp);
    }
    if (arr.get(low) > arr.get(high)) {
        int temp = arr.get(low);
        arr.set(low, arr.get(high));
        arr.set(high, temp);
    }
    if (arr.get(mid) > arr.get(high)) {
        int temp = arr.get(mid);
        arr.set(mid, arr.get(high));
        arr.set(high, temp);
    }
    
    return mid;
}

long long partitionQuick(PagedArray& arr, long long low, long long high) {
    long long pivotIndex = medianOfThree(arr, low, high);
    
    // Swap pivotIndex con high
    int tempPivot = arr.get(pivotIndex);
    arr.set(pivotIndex, arr.get(high));
    arr.set(high, tempPivot);
    
    int pivot = arr.get(high);
    long long i = low - 1;
    
    for (long long j = low; j < high; j++) {
        if (arr.get(j) < pivot) {
            i++;
            // Swap arr[i] y arr[j]
            int temp = arr.get(i);
            arr.set(i, arr.get(j));
            arr.set(j, temp);
        }
    }
    
    // Swap arr[i+1] y arr[high]
    int temp = arr.get(i + 1);
    arr.set(i + 1, arr.get(high));
    arr.set(high, temp);
    
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
    long long largest = i;
    long long left = 2 * i + 1;
    long long right = 2 * i + 2;
    
    if (left < n && arr.get(left) > arr.get(largest)) {
        largest = left;
    }
    
    if (right < n && arr.get(right) > arr.get(largest)) {
        largest = right;
    }
    
    if (largest != i) {
        int temp = arr.get(i);
        arr.set(i, arr.get(largest));
        arr.set(largest, temp);
        heapify(arr, n, largest);
    }
}

void heapSort(PagedArray& arr, long long n) {
    for (long long i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i);
    }
    
    for (long long i = n - 1; i > 0; i--) {
        int temp = arr.get(0);
        arr.set(0, arr.get(i));
        arr.set(i, temp);
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
    
    for (long long i = 0; i < n1; i++)
        L[i] = arr.get(left + i);
    for (long long j = 0; j < n2; j++)
        R[j] = arr.get(mid + 1 + j);
    
    long long i = 0, j = 0, k = left;
    
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr.set(k, L[i]);
            i++;
        } else {
            arr.set(k, R[j]);
            j++;
        }
        k++;
    }
    
    while (i < n1) {
        arr.set(k, L[i]);
        i++;
        k++;
    }
    
    while (j < n2) {
        arr.set(k, R[j]);
        j++;
        k++;
    }
    
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
    
    for (long long i = 0; i < len1; i++)
        left[i] = arr.get(l + i);
    for (long long i = 0; i < len2; i++)
        right[i] = arr.get(m + 1 + i);
    
    long long i = 0, j = 0, k = l;
    
    while (i < len1 && j < len2) {
        if (left[i] <= right[j]) {
            arr.set(k, left[i]);
            i++;
        } else {
            arr.set(k, right[j]);
            j++;
        }
        k++;
    }
    
    while (i < len1) {
        arr.set(k, left[i]);
        i++;
        k++;
    }
    
    while (j < len2) {
        arr.set(k, right[j]);
        j++;
        k++;
    }
    
    delete[] left;
    delete[] right;
}

void timSort(PagedArray& arr, long long n) {
    for (long long i = 0; i < n; i += RUN) {
        insertionSortTim(arr, i, min((i + RUN - 1), (n - 1)));
    }
    
    for (long long size = RUN; size < n; size *= 2) {
        for (long long left = 0; left < n; left += 2 * size) {
            long long mid = left + size - 1;
            long long right = min((left + 2 * size - 1), (n - 1));
            
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
    if (argc != 11) {
        cout << "Uso: ./sorter -input <INPUT FILE PATH> -output <OUTPUT FILE PATH> ";
        cout << "-alg <ALGORITMO> -pageSize <PAGE-SIZE> -pageCount <PAGE-COUNT>\n";
        cout << "Algoritmos: quick, heap, merge, shell, tim\n";
        return 1;
    }

    const char* input = argv[2];
    const char* output = argv[4];
    const char* alg = argv[6];
    int pageSize = atoi(argv[8]);
    int pageCount = atoi(argv[10]);

    if (pageSize <= 0 || pageCount <= 0) {
        cout << "pageSize y pageCount deben ser mayores que 0\n";
        return 1;
    }

    if (strcmp(alg, "quick") != 0 && strcmp(alg, "heap") != 0 && 
        strcmp(alg, "merge") != 0 && strcmp(alg, "shell") != 0 && 
        strcmp(alg, "tim") != 0) {
        cout << "Algoritmo no soportado\n";
        cout << "Use: quick, heap, merge, shell, tim\n";
        return 1;
    }

    cout << "========================================\n";
    cout << "Ordenador de Archivos con Paginación\n";
    cout << "========================================\n";
    cout << "Archivo entrada: " << input << endl;
    cout << "Archivo salida: " << output << endl;
    cout << "Algoritmo: " << alg << endl;
    cout << "Tamaño página: " << pageSize << " enteros\n";
    cout << "Cantidad páginas: " << pageCount << endl;
    cout << "Memoria total: " << (pageSize * pageCount * sizeof(int)) / (1024*1024) << " MB\n";
    cout << "----------------------------------------\n";

    cout << "Copiando archivo..." << endl;
    copyFile(input, output);

    cout << "Creando PagedArray..." << endl;
    PagedArray arr(output, pageSize, pageCount);
    long long n = arr.size();
    cout << "Total elementos: " << n << " (" << (n * sizeof(int)) / (1024*1024) << " MB)\n";

    cout << "Iniciando " << alg << " sort..." << endl;

    auto start = chrono::high_resolution_clock::now();

    if (strcmp(alg, "quick") == 0) {
        quickSort(arr, 0, n - 1);
    }
    else if (strcmp(alg, "heap") == 0) {
        heapSort(arr, n);
    }
    else if (strcmp(alg, "merge") == 0) {
        mergeSort(arr, n);
    }
    else if (strcmp(alg, "shell") == 0) {
        shellSort(arr, n);
    }
    else if (strcmp(alg, "tim") == 0) {
        timSort(arr, n);
    }

    cout << "Escribiendo cambios al disco..." << endl;
    arr.flushAll();

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;

    cout << "Generando archivo legible..." << endl;
    generateReadableFile(arr, "ordenado.txt");

    cout << "========================================\n";
    cout << "RESULTADOS\n";
    cout << "========================================\n";
    cout << "Algoritmo: " << alg << endl;
    cout << "Tiempo total: " << elapsed.count() << " segundos\n";
    cout << "Page Hits: " << arr.getHits() << endl;
    cout << "Page Faults: " << arr.getFaults() << endl;
    
    double hitRate = (double)arr.getHits() / (arr.getHits() + arr.getFaults()) * 100;
    cout << "Hit Rate: " << hitRate << "%\n";
    cout << "========================================\n";

    return 0;
}