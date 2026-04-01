#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <chrono>

using namespace std;

long long getSizeBytes(string size) {
    if (size == "SMALL") return 128LL * 1024 * 1024;      // 128 MB
    if (size == "MEDIUM") return 256LL * 1024 * 1024;    // 256 MB
    if (size == "LARGE") return 512LL * 1024 * 1024;     // 512 MB
    return -1;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        cout << "Uso: ./generator -size <SIZE> -output <FILE>\n";
        cout << "SIZE: SMALL (128MB), MEDIUM (256MB), LARGE (512MB)\n";
        return 1;
    }

    string sizeArg = argv[2];
    string output = argv[4];

    long long bytes = getSizeBytes(sizeArg);

    if (bytes == -1) {
        cout << "Size invalido. Use: SMALL, MEDIUM, LARGE\n";
        return 1;
    }

    // Abrir archivo con buffer grande para mejor rendimiento
    ofstream file(output, ios::binary);
    if (!file) {
        cout << "No se pudo crear el archivo\n";
        return 1;
    }

    srand(time(0));
    long long totalInts = bytes / sizeof(int);
    
    // Buffer de 4MB para escritura más rápida
    const int BUFFER_SIZE = 1024 * 1024; // 1M enteros = 4MB
    int* buffer = new int[BUFFER_SIZE];
    
    cout << "Generando " << totalInts << " enteros (" << bytes / (1024*1024) << " MB)..." << endl;
    
    auto start = chrono::high_resolution_clock::now();
    
    long long remaining = totalInts;
    long long totalWritten = 0;
    
    while (remaining > 0) {
        int toWrite = (remaining > BUFFER_SIZE) ? BUFFER_SIZE : (int)remaining;
        
        // Generar números aleatorios
        for (int i = 0; i < toWrite; i++) {
            buffer[i] = rand();
        }
        
        // Escribir al archivo
        file.write((char*)buffer, toWrite * sizeof(int));
        
        remaining -= toWrite;
        totalWritten += toWrite;
        
        // Mostrar progreso
        int percent = (totalWritten * 100) / totalInts;
        cout << "Progreso: " << percent << "%\r" << flush;
    }
    
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;
    
    delete[] buffer;
    file.close();
    
    cout << "\nArchivo generado correctamente: " << output << endl;
    cout << "Tiempo de generacion: " << elapsed.count() << " segundos\n";
    
    return 0;
}