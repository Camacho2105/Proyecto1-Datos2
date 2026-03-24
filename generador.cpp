#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <string>

using namespace std;

long long getSizeBytes(string size) {
    if (size == "SMALL") return 128LL * 1024 * 1024;
    if (size == "MEDIUM") return 256LL * 1024 * 1024;
    if (size == "LARGE") return 512LL * 1024 * 1024;
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

    ofstream file(output, ios::binary);
    if (!file) {
        cout << "No se pudo crear el archivo\n";
        return 1;
    }

    srand(time(0));

    long long totalInts = bytes / sizeof(int);
    const int BUFFER_SIZE = 1024 * 1024; // 1M de enteros por buffer
    int* buffer = new int[BUFFER_SIZE];
    
    cout << "Generando " << totalInts << " enteros (" << bytes / (1024*1024) << " MB)..." << endl;
    
    long long remaining = totalInts;
    while (remaining > 0) {
        int toWrite = (remaining > BUFFER_SIZE) ? BUFFER_SIZE : (int)remaining;
        
        for (int i = 0; i < toWrite; i++) {
            buffer[i] = rand();
        }
        
        file.write((char*)buffer, toWrite * sizeof(int));
        remaining -= toWrite;
        
        cout << "Progreso: " << ((totalInts - remaining) * 100 / totalInts) << "%\r" << flush;
    }
    
    delete[] buffer;
    file.close();
    
    cout << "\nArchivo generado correctamente: " << output << endl;
    return 0;
}