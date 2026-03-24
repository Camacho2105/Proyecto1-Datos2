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
        return 1;
    }

    string sizeArg = argv[2];
    string output = argv[4];

    long long bytes = getSizeBytes(sizeArg);

    if (bytes == -1) {
        cout << "Size invalido\n";
        return 1;
    }

    ofstream file(output, ios::binary);
    if (!file) {
        cout << "No se pudo crear el archivo\n";
        return 1;
    }

    srand(time(0));

    long long totalInts = bytes / sizeof(int);

    for (long long i = 0; i < totalInts; i++) {
        int num = rand();
        file.write((char*)&num, sizeof(int));
    }

    file.close();
    cout << "Archivo generado correctamente\n";
    return 0;
}