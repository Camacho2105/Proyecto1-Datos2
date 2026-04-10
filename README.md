# Proyecto 1 - Arreglos Paginados

## Compilación

```bash
g++ generador.cpp -o generador
g++ sorter.cpp pagedArray.cpp -o sorter
 Generar archivo de prueba
.\generador.exe -size small -output test.bin
Parámetros
-size: tamaño del archivo (small, medium, large)
-output: nombre del archivo de salida
 Ejecutar el ordenador
.\sorter.exe -input test.bin -output out.bin -alg merge -pageSize 4096 -pageCount 128
Parámetros
-input: archivo de entrada
-output: archivo de salida
-alg: algoritmo (merge, quick, tim, heap, shell)
-pageSize: tamaño de página
-pageCount: cantidad de páginas en memoria