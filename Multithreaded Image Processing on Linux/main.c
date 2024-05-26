#include <stdlib.h>
#include <stdio.h>
#include <png.h>
#include <pthread.h>

int width, height;
png_byte color_type;
png_byte bit_depth;
png_bytep *row_pointers = NULL;
int num_threads[] = {1,2,4,8}; 
int rowbytes;

void read_png_file(char *filename) {
    FILE *fp = fopen(filename, "rb");

    if(!fp) {
        perror("File could not be opened for reading");
        abort();
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png) abort();

    png_infop info = png_create_info_struct(png);
    if(!info) abort();

    if(setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    png_read_info(png, info);

    width = png_get_image_width(png, info);
    height = png_get_image_height(png, info);
    color_type = png_get_color_type(png, info);
    bit_depth = png_get_bit_depth(png, info);
    rowbytes = png_get_rowbytes(png, info);

    if(bit_depth == 16)
        png_set_strip_16(png);

    if(color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);

    if(png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    if(color_type == PNG_COLOR_TYPE_RGB ||
       color_type == PNG_COLOR_TYPE_GRAY ||
       color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

    if(color_type == PNG_COLOR_TYPE_GRAY ||
       color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    if (row_pointers) abort();

    row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
    for(int y = 0; y < height; y++) {
        row_pointers[y] = (png_byte*)malloc(rowbytes);
    }

    png_read_image(png, row_pointers);

    fclose(fp);

    png_destroy_read_struct(&png, &info, NULL);
}

void write_png_file(char *filename) {
    FILE *fp = fopen(filename, "wb");
    if(!fp) abort();

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) abort();

    png_infop info = png_create_info_struct(png);
    if (!info) abort();

    if (setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    png_set_IHDR(
        png,
        info,
        width, height,
        8,
        PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);

    if (!row_pointers) abort();

    png_write_image(png, row_pointers);
    png_write_end(png, NULL);

    fclose(fp);

    png_destroy_write_struct(&png, &info);
}

typedef struct {
    int start_row;
    int end_row;
    png_bytep *src_row_pointers;
    png_bytep *dst_row_pointers;
} ThreadData;

void *process_png_file(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int kernel_size = 5;
    int kernel_half = kernel_size / 2;
    float kernel[5][5] = {
        {1 / 256.0, 4 / 256.0, 6 / 256.0, 4 / 256.0, 1 / 256.0},
        {4 / 256.0, 16 / 256.0, 24 / 256.0, 16 / 256.0, 4 / 256.0},
        {6 / 256.0, 24 / 256.0, 36 / 256.0, 24 / 256.0, 6 / 256.0},
        {4 / 256.0, 16 / 256.0, 24 / 256.0, 16 / 256.0, 4 / 256.0},
        {1 / 256.0, 4 / 256.0, 6 / 256.0, 4 / 256.0, 1 / 256.0}
    };

    for (int y = data->start_row; y < data->end_row; y++) {
        for (int x = 0; x < width; x++) {
            float r = 0, g = 0, b = 0, a = 0;

            // Apply Gaussian kernel
            for (int ky = -kernel_half; ky <= kernel_half; ky++) {
                for (int kx = -kernel_half; kx <= kernel_half; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        png_bytep px = &(data->src_row_pointers[ny][nx * 4]);
                        float kernel_value = kernel[ky + kernel_half][kx + kernel_half];
                        r += px[0] * kernel_value;
                        g += px[1] * kernel_value;
                        b += px[2] * kernel_value;
                        a += px[3] * kernel_value;
                        for (int i = 0; i < 1000; i++){
                            ;
                        }
                    }
                }
            }

            // Set the new pixel value
            png_bytep px = &(data->dst_row_pointers[y][x * 4]);
            px[0] = (png_byte)r;
            px[1] = (png_byte)g;
            px[2] = (png_byte)b;
            px[3] = (png_byte)a;
        }
    }
    
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <input_png>>\n", argv[0]);
        return 1;
    }
    read_png_file(argv[1]);

    png_bytep *new_row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
    for(int y = 0; y < height; y++) {
        new_row_pointers[y] = (png_byte*)malloc(rowbytes);
    }

    for (int k = 0; k < 4; k ++){
        pthread_t threads[num_threads[k]];
        ThreadData thread_data[num_threads[k]];
        int rows_per_thread = height / num_threads[k];

        clock_t start_t = clock();
        for(int i = 0; i < num_threads[k]; i++) {
            thread_data[i].start_row = i * rows_per_thread;
            if (i == num_threads[k] - 1) {
                thread_data[i].end_row = height; // Last thread processes the remaining rows
            } else {
                thread_data[i].end_row = (i + 1) * rows_per_thread;
            }
            thread_data[i].src_row_pointers = row_pointers;
            thread_data[i].dst_row_pointers = new_row_pointers;
            pthread_create(&threads[i], NULL, process_png_file, (void *)&thread_data[i]);
        }

        for(int i = 0; i < num_threads[k]; i++) {
            pthread_join(threads[i], NULL);
        }

        row_pointers = new_row_pointers;
        char buff[50];
        sprintf(buff, "output%d.png", k);
        write_png_file(buff);
  
        clock_t end_t = clock();
        double total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;
        printf("Threads[%d] -> Time taken %f seconds\n",num_threads[k], total_t/num_threads[k]);
    }
    

    for(int y = 0; y < height; y++) {
        free(row_pointers[y]);
    }
    free(row_pointers);
    
    return 0;
}
//gcc -o main main.c -lpng -lpthread
//./main input.png 