#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <xmmintrin.h> // __m128

typedef struct {
    __m128* data;
    int size;
} VectorArray;

VectorArray generate_random_vectors(int num_numbers) {
    int num_vectors = num_numbers / 4; //Dzielimy przez 4 bo 1 wektor ma 4 liczby
    VectorArray array;
    array.data = (__m128*)aligned_alloc(16, num_vectors * sizeof(__m128)); // Alokacja pamięci dla tablicy wektorów
    array.size = num_vectors;

    float MIN = -1.0e6f;
    float MAX =  1.0e6f;

    for (int i = 0; i < num_vectors; i++) {
        float vals[4] __attribute__((aligned(16))); //Tabela
        for (int j = 0; j < 4; j++) {
            float normalized = (float)rand() / (float)RAND_MAX; // [0.0, 1.0]
            vals[j] = MIN + normalized * (MAX - MIN);
        }
        // Ładowanie do __m128 przez ASM
        __asm__ __volatile__ (
            "movaps %1, %%xmm0\n\t"
            "movaps %%xmm0, %0\n\t"
            : "=m" (array.data[i])
            : "m" (vals)
            : "%xmm0"
        );
    }
    return array;
}

void free_vector_array(VectorArray array) {
    free(array.data);
}

void measure_simd_operations(int num_numbers, int iterations, FILE* output_file) {
    VectorArray x = generate_random_vectors(num_numbers);
    VectorArray y = generate_random_vectors(num_numbers);
    VectorArray z = generate_random_vectors(num_numbers);

    double times_add[iterations];
    double times_sub[iterations];
    double times_mul[iterations];
    double times_div[iterations];

    for (int iter = 0; iter < iterations; iter++) {
        // Dodawanie
        clock_t start = clock();
        for (int i = 0; i < x.size; i++) {
            __asm__ __volatile__ (
                "movaps %[x], %%xmm0\n\t"
                "movaps %[y], %%xmm1\n\t"
                "addps %%xmm1, %%xmm0\n\t"
                "movaps %%xmm0, %[z]\n\t"
                : [z] "=m" (z.data[i])
                : [x] "m" (x.data[i]), [y] "m" (y.data[i])
                : "%xmm0", "%xmm1"
            );
        }
        clock_t end = clock();
        times_add[iter] = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC;

        // Odejmowanie
        start = clock();
        for (int i = 0; i < x.size; i++) {
            __asm__ __volatile__ (
                "movaps %[x], %%xmm0\n\t"
                "movaps %[y], %%xmm1\n\t"
                "subps %%xmm1, %%xmm0\n\t"
                "movaps %%xmm0, %[z]\n\t"
                : [z] "=m" (z.data[i])
                : [x] "m" (x.data[i]), [y] "m" (y.data[i])
                : "%xmm0", "%xmm1"
            );
        }
        end = clock();
        times_sub[iter] = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC;

        // Mnożenie
        start = clock();
        for (int i = 0; i < x.size; i++) {
            __asm__ __volatile__ (
                "movaps %[x], %%xmm0\n\t"
                "movaps %[y], %%xmm1\n\t"
                "mulps %%xmm1, %%xmm0\n\t"
                "movaps %%xmm0, %[z]\n\t"
                : [z] "=m" (z.data[i])
                : [x] "m" (x.data[i]), [y] "m" (y.data[i])
                : "%xmm0", "%xmm1"
            );
        }
        end = clock();
        times_mul[iter] = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC;

        // Dzielenie
        start = clock();
        for (int i = 0; i < x.size; i++) {
            __asm__ __volatile__ (
                "movaps %[x], %%xmm0\n\t"
                "movaps %[y], %%xmm1\n\t"
                "divps %%xmm1, %%xmm0\n\t"
                "movaps %%xmm0, %[z]\n\t"
                : [z] "=m" (z.data[i])
                : [x] "m" (x.data[i]), [y] "m" (y.data[i])
                : "%xmm0", "%xmm1"
            );
        }
        end = clock();
        times_div[iter] = ((double)(end - start)) * 1000/ CLOCKS_PER_SEC;
    }

    // Obliczenie wartości średnich
    double avg_add = 0, avg_sub = 0, avg_mul = 0, avg_div = 0;
    for (int i = 0; i < iterations; i++) {
        avg_add += times_add[i];
        avg_sub += times_sub[i];
        avg_mul += times_mul[i];
        avg_div += times_div[i];
    }
    avg_add /= iterations;
    avg_sub /= iterations;
    avg_mul /= iterations;
    avg_div /= iterations;

    // Write results
    fprintf(output_file, "Typ obliczen: SIMD\n");
    fprintf(output_file, "Liczba liczb: %d\n", num_numbers);
    fprintf(output_file, "Sredni czas [ms]:\n");
    fprintf(output_file, "+ %.6f\n", avg_add);
    fprintf(output_file, "- %.6f\n", avg_sub);
    fprintf(output_file, "* %.6f\n", avg_mul);
    fprintf(output_file, "/ %.6f\n", avg_div);
    fprintf(output_file, "\n");

    free_vector_array(x);
    free_vector_array(y);
    free_vector_array(z);
}

int main() {
    srand(time(NULL));

    FILE* output_file = fopen("simd_wyniki_asm.txt", "w");
    if (!output_file) {
        perror("Nie udalo sie otworzyc pliku");
        return 1;
    }

    int test_sizes[] = {2048, 4096, 8192};
    int iterations = 10;

    for (int i = 0; i < sizeof(test_sizes)/sizeof(test_sizes[0]); i++) {
        measure_simd_operations(test_sizes[i], iterations, output_file);
    }

    fclose(output_file);

    return 0;
}

