#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>
#include "PagedArray.h"

using namespace std;

// =====================================================
// Utilidades
// =====================================================

void copyFile(const char* in, const char* out) {
    ifstream src(in, ios::binary);
    ofstream dst(out, ios::binary);
    dst << src.rdbuf();
}

void generateReadableFile(PagedArray& arr, const char* textFile) {
    ofstream out(textFile);
    if (!out) {
        cout << "Error al crear archivo legible\n";
        return;
    }
    
    long long n = arr.size();
    cout << "Generando archivo legible (" << n << " enteros)..." << endl;
    
    for (long long i = 0; i < n; i++) {
        if (i > 0) out << ",";
        out << (int)arr[i];
        
        // Mostrar progreso cada 1%
        if (i % (n / 100 + 1) == 0) {
            cout << "Progreso: " << (i * 100 / n) << "%\r" << flush;
        }
    }
    
    cout << "\nArchivo legible generado: " << textFile << endl;
}

void swapPaged(PagedArray& arr, long long i, long long j) {
    int temp = (int)arr[i];
    arr[i] = (int)arr[j];
    arr[j] = temp;
}

// =====================================================
// QUICK SORT
// =====================================================

long long partitionQuick(PagedArray& arr, long long low, long long high) {
    int pivot = (int)arr[high];
    long long i = low - 1;

    for (long long j = low; j < high; j++) {
        if ((int)arr[j] < pivot) {
            i++;
            swapPaged(arr, i, j);
        }
    }

    swapPaged(arr, i + 1, high);
    return i + 1;
}

void quickSort(PagedArray& arr, long long low, long long high) {
    if (low < high) {
        long long pi = partitionQuick(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

// =====================================================
// HEAP SORT
// =====================================================

void heapify(PagedArray& arr, long long n, long long i) {
    long long largest = i;
    long long left = 2 * i + 1;
    long long right = 2 * i + 2;

    if (left < n && (int)arr[left] > (int)arr[largest]) {
        largest = left;
    }

    if (right < n && (int)arr[right] > (int)arr[largest]) {
        largest = right;
    }

    if (largest != i) {
        swapPaged(arr, i, largest);
        heapify(arr, n, largest);
    }
}

void heapSort(PagedArray& arr, long long n) {
    // Build heap
    for (long long i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i);
    }

    // Extract elements from heap
    for (long long i = n - 1; i > 0; i--) {
        swapPaged(arr, 0, i);
        heapify(arr, i, 0);
    }
}

// =====================================================
// SHELL SORT
// =====================================================

void shellSort(PagedArray& arr, long long n) {
    for (long long gap = n / 2; gap > 0; gap /= 2) {
        for (long long i = gap; i < n; i++) {
            int temp = (int)arr[i];
            long long j = i;

            while (j >= gap && (int)arr[j - gap] > temp) {
                arr[j] = (int)arr[j - gap];
                j -= gap;
            }

            arr[j] = temp;
        }
    }
}

// =====================================================
// INSERTION SORT
// =====================================================

void insertionSort(PagedArray& arr, long long n) {
    for (long long i = 1; i < n; i++) {
        int key = (int)arr[i];
        long long j = i - 1;

        while (j >= 0 && (int)arr[j] > key) {
            arr[j + 1] = (int)arr[j];
            j--;
        }

        arr[j + 1] = key;
    }
}

// =====================================================
// SELECTION SORT
// =====================================================

void selectionSort(PagedArray& arr, long long n) {
    for (long long i = 0; i < n - 1; i++) {
        long long minIndex = i;

        for (long long j = i + 1; j < n; j++) {
            if ((int)arr[j] < (int)arr[minIndex]) {
                minIndex = j;
            }
        }

        if (minIndex != i) {
            swapPaged(arr, i, minIndex);
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
        cout << "Algoritmos: quick, heap, shell, insertion, selection\n";
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
        strcmp(alg, "shell") != 0 && strcmp(alg, "insertion") != 0 && 
        strcmp(alg, "selection") != 0) {
        cout << "Algoritmo no soportado\n";
        cout << "Use: quick, heap, shell, insertion, selection\n";
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
    else if (strcmp(alg, "shell") == 0) {
        shellSort(arr, n);
    }
    else if (strcmp(alg, "insertion") == 0) {
        insertionSort(arr, n);
    }
    else if (strcmp(alg, "selection") == 0) {
        selectionSort(arr, n);
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