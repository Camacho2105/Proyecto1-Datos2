#include <iostream>
#include <fstream>
#include <chrono>
#include <cstring>

using namespace std;

// =====================================================
// Leer archivo binario a memoria
// =====================================================

int* loadFile(const char* path, long long& n) {
    ifstream file(path, ios::binary);

    if (!file) {
        cout << "Error al abrir archivo\n";
        return nullptr;
    }

    file.seekg(0, ios::end);
    long long bytes = static_cast<long long>(file.tellg());
    file.seekg(0, ios::beg);

    if (bytes < 0 || bytes % sizeof(int) != 0) {
        cout << "Archivo binario invalido\n";
        return nullptr;
    }

    n = bytes / static_cast<long long>(sizeof(int));

    int* arr = new int[n];

    file.read(reinterpret_cast<char*>(arr), bytes);

    if (!file) {
        cout << "Error al leer archivo\n";
        delete[] arr;
        return nullptr;
    }

    return arr;
}

// =====================================================
// Verificar orden
// =====================================================

bool isSorted(int* arr, long long n) {
    for (long long i = 1; i < n; i++) {
        if (arr[i - 1] > arr[i]) {
            return false;
        }
    }
    return true;
}

long long minLong(long long a, long long b) {
    return (a < b) ? a : b;
}

// =====================================================
// QUICK SORT
// =====================================================

void quickSort(int* arr, long long low, long long high) {
    long long i, j;
    int temp, pivot;

    if (low < high) {
        i = low;
        j = high;
        pivot = arr[low];

        while (i < j) {
            while (i <= high && arr[i] <= pivot) {
                i++;
            }

            while (arr[j] > pivot) {
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

void heapify(int* arr, long long n, long long i) {
    long long largest = i;
    long long left = 2 * i + 1;
    long long right = 2 * i + 2;
    int temp;

    if (left < n && arr[left] > arr[largest]) {
        largest = left;
    }

    if (right < n && arr[right] > arr[largest]) {
        largest = right;
    }

    if (largest != i) {
        temp = arr[i];
        arr[i] = arr[largest];
        arr[largest] = temp;

        heapify(arr, n, largest);
    }
}

void heapSort(int* arr, long long n) {
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

void merge(int* arr, long long left, long long mid, long long right) {
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

void mergeSort(int* arr, long long left, long long right) {
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

void shellSort(int* arr, long long n) {
    for (long long gap = n / 2; gap > 0; gap /= 2) {
        for (long long i = gap; i < n; i++) {
            int temp = arr[i];
            long long j = i;

            while (j >= gap && arr[j - gap] > temp) {
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

void insertionSortTim(int* arr, long long left, long long right) {
    for (long long i = left + 1; i <= right; i++) {
        int temp = arr[i];
        long long j = i - 1;

        while (j >= left && arr[j] > temp) {
            arr[j + 1] = arr[j];
            j--;
        }

        arr[j + 1] = temp;
    }
}

void mergeTim(int* arr, long long l, long long m, long long r) {
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

void timSort(int* arr, long long n) {
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
    if (argc != 3) {
        cout << "Uso: ./baseline <archivo> <algoritmo>\n";
        cout << "Algoritmos: quick, heap, merge, shell, tim\n";
        return 1;
    }

    const char* filePath = argv[1];
    const char* alg = argv[2];

    long long n = 0;
    int* arr = loadFile(filePath, n);

    if (!arr) {
        return 1;
    }

    cout << "Elementos: " << n << endl;
    cout << "Iniciando baseline (" << alg << ")...\n";

    auto start = chrono::high_resolution_clock::now();

    if (strcmp(alg, "quick") == 0) {
        quickSort(arr, 0, n - 1);
    } else if (strcmp(alg, "heap") == 0) {
        heapSort(arr, n);
    } else if (strcmp(alg, "merge") == 0) {
        mergeSort(arr, 0, n - 1);
    } else if (strcmp(alg, "shell") == 0) {
        shellSort(arr, n);
    } else if (strcmp(alg, "tim") == 0) {
        timSort(arr, n);
    } else {
        cout << "Algoritmo no valido\n";
        delete[] arr;
        return 1;
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;

    cout << "Tiempo: " << elapsed.count() << " segundos\n";
    cout << "Ordenado: " << (isSorted(arr, n) ? "SI" : "NO") << endl;

    delete[] arr;
    return 0;
}