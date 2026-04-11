# Proyecto #1 - Arreglos Paginados

**Curso:** Algoritmos y Estructuras de Datos II (CE-1103)
**Institución:** Instituto Tecnológico de Costa Rica
**Estudiante:** Kendall Camacho Salas
**Semestre:** I Semestre 2026

---

## Descripción

Este proyecto implementa un sistema de ordenamiento para archivos binarios de gran tamaño utilizando una estructura de **arreglo paginado (PagedArray)** que simula memoria virtual. Los datos se mantienen parcialmente en memoria RAM mientras que el resto permanece en disco, permitiendo procesar archivos que exceden la capacidad de la memoria principal.

### Componentes del proyecto

| Componente | Descripción |
|---|---|
| `generator.exe` | Genera archivos binarios con números enteros aleatorios |
| `sorter.exe` | Ordena archivos binarios usando PagedArray y diferentes algoritmos |
| `baseline.exe` | Versión sin paginación para comparación de rendimiento |
| `pagedArray.cpp/h` | Implementación del arreglo paginado con algoritmo de reemplazo CLOCK |

### Algoritmos de ordenamiento implementados

- Quick Sort
- Heap Sort
- Merge Sort
- Shell Sort
- Tim Sort

---

## Compilación

### Usando MinGW (recomendado)

```batch
:: Compilar generador
g++ generator.exe generador.cpp

:: Compilar ordenador (con paginación)
g++ sorter.exe sorter.cpp pagedArray.cpp

:: Compilar baseline (sin paginación)
g++ baseline.exe baseline.cpp
```


## Uso del Generador

### Sintaxis

```batch
generator.exe -size <SIZE> -output <ARCHIVO_SALIDA>
```

### Parámetros

| Parámetro | Valores | Descripción |
|---|---|---|
| `-size` | `SMALL`, `MEDIUM`, `LARGE` | Tamaño del archivo a generar |
| `-output` | Ruta del archivo | Archivo binario de salida |

### Tamaños de archivo

| Opción | Tamaño | Cantidad de enteros |
|---|---|---|
| `SMALL` | 32 MB | 8,388,608 |
| `MEDIUM` | 64 MB | 16,777,216 |
| `LARGE` | 128 MB | 33,554,432 |

### Ejemplos

```batch
:: Generar archivo SMALL
generator.exe -size SMALL -output datos_small.bin

:: Generar archivo MEDIUM
generator.exe -size MEDIUM -output datos_medium.bin

:: Generar archivo LARGE
generator.exe -size LARGE -output datos_large.bin
```


## 🔢 Uso del Ordenador (con paginación)

### Sintaxis

```batch
sorter.exe -input <ARCHIVO_ENTRADA> -output <ARCHIVO_SALIDA> -alg <ALGORITMO> -pageSize <PAGE_SIZE> -pageCount <PAGE_COUNT>
```

### Parámetros

| Parámetro | Descripción |
|---|---|
| `-input` | Ruta del archivo binario a ordenar |
| `-output` | Ruta del archivo binario ordenado (se genera también un `.txt` legible) |
| `-alg` | Algoritmo de ordenamiento: `quick`, `heap`, `merge`, `shell`, `tim` |
| `-pageSize` | Cantidad de enteros por página (ej: 512, 1024, 2048, 4096, 8192) |
| `-pageCount` | Cantidad de páginas en memoria (ej: 8, 16, 32, 64, 128, 256) |

### Ejemplos

```batch
:: Merge Sort con página de 4096 enteros y 128 páginas en memoria
.\sorter.exe -input datos_small.bin -output ordenado.bin -alg merge -pageSize 4096 -pageCount 128

:: Quick Sort con configuración agresiva
.\sorter.exe -input datos_small.bin -output ordenado.bin -alg quick -pageSize 8192 -pageCount 256

:: Tim Sort con página pequeña
.\sorter.exe -input datos_small.bin -output ordenado.bin -alg tim -pageSize 1024 -pageCount 64
```