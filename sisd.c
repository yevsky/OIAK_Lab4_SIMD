#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    float* data;
    int size;
} FloatArray;

FloatArray generate_random_array(int num_numbers) {
    FloatArray array;
    array.data = (float*)malloc(num_numbers * sizeof(float));
    array.size = num_numbers;

    float MIN = -1.0e6f;
    float MAX =  1.0e6f;

    for (int i = 0; i < num_numbers; i++) {
        float normalized = (float)rand() / (float)RAND_MAX;
        array.data[i] = MIN + normalized * (MAX - MIN);
    }
    return array;
}

void free_array(FloatArray array) {
    free(array.data);
}

void measure_sisd_operations(int num_numbers, int iterations, FILE* output_file) {
    FloatArray x = generate_random_array(num_numbers);
    FloatArray y = generate_random_array(num_numbers);
    FloatArray z = generate_random_array(num_numbers); // tablica wyników

    double times_add[iterations];
    double times_sub[iterations];
    double times_mul[iterations];
    double times_div[iterations];

    for (int iter = 0; iter < iterations; iter++) {
        clock_t start = clock();
        for (int i = 0; i < num_numbers; i++) {
            z.data[i] = x.data[i] + y.data[i];
        }
        clock_t end = clock();
        times_add[iter] = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC;

        start = clock();
        for (int i = 0; i < num_numbers; i++) {
            z.data[i] = x.data[i] - y.data[i];
        }
        end = clock();
        times_sub[iter] = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC;

        start = clock();
        for (int i = 0; i < num_numbers; i++) {
            z.data[i] = x.data[i] * y.data[i];
        }
        end = clock();
        times_mul[iter] = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC;

        start = clock();
        for (int i = 0; i < num_numbers; i++) {
            z.data[i] = x.data[i] / y.data[i];
        }
        end = clock();
        times_div[iter] = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC;
    }

    // Średnie czasy
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

    // Zapis wyników
    fprintf(output_file, "Typ obliczen: SISD\n");
    fprintf(output_file, "Liczba liczb: %d\n", num_numbers);
    fprintf(output_file, "Sredni czas [ms]:\n");
    fprintf(output_file, "+ %.6f\n", avg_add);
    fprintf(output_file, "- %.6f\n", avg_sub);
    fprintf(output_file, "* %.6f\n", avg_mul);
    fprintf(output_file, "/ %.6f\n", avg_div);
    fprintf(output_file, "\n");

    free_array(x);
    free_array(y);
    free_array(z);
}

int main() {
    srand(time(NULL));

    FILE* output_file = fopen("sisd_wyniki.txt", "w");
    if (!output_file) {
        perror("Nie udalo sie otworzyc pliku");
        return 1;
    }

    int test_sizes[] = {2048, 4096, 8192};
    int iterations = 10;

    for (int i = 0; i < sizeof(test_sizes)/sizeof(test_sizes[0]); i++) {
        measure_sisd_operations(test_sizes[i], iterations, output_file);
    }

    fclose(output_file);
    return 0;
}
