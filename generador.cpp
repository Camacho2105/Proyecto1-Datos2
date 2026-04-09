#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <chrono>

using namespace std;

long long getSizeBytes(const string& size) {
    if (size == "SMALL") return 256LL * 1024 * 1024;
    if (size == "MEDIUM") return 512LL * 1024 * 1024;
    if (size == "LARGE") return 1024LL * 1024 * 1024;
    return -1;
}

void printUsage() {
    cout << "Uso: ./generator -size <SIZE> -output <FILE>\n";
    cout << "SIZE: SMALL (256MB), MEDIUM (512MB), LARGE (1024MB)\n";
}

bool parseArguments(int argc, char* argv[], string& sizeArg, string& output) {
    if (argc != 5) {
        return false;
    }

    for (int i = 1; i < argc; i += 2) {
        if (i + 1 >= argc) {
            return false;
        }

        if (string(argv[i]) == "-size") {
            sizeArg = argv[i + 1];
        } else if (string(argv[i]) == "-output") {
            output = argv[i + 1];
        } else {
            return false;
        }
    }

    return !sizeArg.empty() && !output.empty();
}

int main(int argc, char* argv[]) {
    string sizeArg;
    string output;

    if (!parseArguments(argc, argv, sizeArg, output)) {
        printUsage();
        return 1;
    }

    long long bytes = getSizeBytes(sizeArg);

    if (bytes == -1) {
        cout << "Size invalido. Use: SMALL, MEDIUM, LARGE\n";
        return 1;
    }

    ofstream file(output, ios::binary | ios::trunc);
    if (!file) {
        cout << "No se pudo crear el archivo\n";
        return 1;
    }

    srand(static_cast<unsigned int>(time(0)));
    long long totalInts = bytes / static_cast<long long>(sizeof(int));

    const int BUFFER_SIZE = 1024 * 1024;
    int* buffer = new int[BUFFER_SIZE];

    cout << "Generando " << totalInts << " enteros (" << bytes / (1024 * 1024) << " MB)..." << endl;

    auto start = chrono::high_resolution_clock::now();

    long long remaining = totalInts;
    long long totalWritten = 0;

    while (remaining > 0) {
        int toWrite = (remaining > BUFFER_SIZE) ? BUFFER_SIZE : static_cast<int>(remaining);

        for (int i = 0; i < toWrite; i++) {
            buffer[i] = rand();
        }

        file.write(reinterpret_cast<char*>(buffer), toWrite * static_cast<int>(sizeof(int)));
        if (!file) {
            cout << "Error al escribir en el archivo\n";
            delete[] buffer;
            return 1;
        }

        remaining -= toWrite;
        totalWritten += toWrite;

        int percent = static_cast<int>((totalWritten * 100) / totalInts);
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