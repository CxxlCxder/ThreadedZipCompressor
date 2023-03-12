/*
* Threaded Zip Compressor
* Version 1.0
* Creator : Relic ( CooL-CodeR )
* Initial Release : Feb 2023
* status : V 1.0 Complete
*/

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <pthread.h>

#define CHUNK_SIZE 16384

// Struct to hold information about the thread
typedef struct {
    gzFile in_file;
    gzFile out_file;
    int thread_id;
} thread_data;

// Function to compress a chunk of data in a separate thread
void* compress_chunk(void* data)
{
    thread_data* thread_data_ptr = (thread_data*) data;
    gzFile in_file = thread_data_ptr->in_file;
    gzFile out_file = thread_data_ptr->out_file;
    int thread_id = thread_data_ptr->thread_id;

    // Allocate memory for input and output buffers
    unsigned char* in_buf = malloc(CHUNK_SIZE);
    unsigned char* out_buf = malloc(CHUNK_SIZE);

    // Compress each chunk until end of file is reached
    while (1) {
        // Read data from input file
        int num_read = gzread(in_file, in_buf, CHUNK_SIZE);

        if (num_read < 0) {
            fprintf(stderr, "Error reading from input file in thread %d\n", thread_id);
            break;
        }

        if (num_read == 0) {
            // End of file reached, exit thread
            break;
        }

        // Compress data
        z_stream stream;
        stream.zalloc = Z_NULL;
        stream.zfree = Z_NULL;
        stream.opaque = Z_NULL;
        stream.avail_in = num_read;
        stream.next_in = in_buf;
        stream.avail_out = CHUNK_SIZE;
        stream.next_out = out_buf;
        deflateInit(&stream, Z_DEFAULT_COMPRESSION);
        deflate(&stream, Z_FINISH);
        deflateEnd(&stream);

        // Write compressed data to output file
        gzwrite(out_file, out_buf, CHUNK_SIZE - stream.avail_out);
    }

    // Free memory and exit thread
    free(in_buf);
    free(out_buf);
    pthread_exit(NULL);
}

int main(int argc, char** argv)
{
    // Open input and output files
    gzFile in_file = gzopen(argv[1], "rb");
    gzFile out_file = gzopen(argv[2], "wb");

    // Determine number of threads to use
    int num_threads = atoi(argv[3]);

    // Create an array of thread data structs
    thread_data* thread_data_array = malloc(num_threads * sizeof(thread_data));

    // Create threads to compress each chunk of data
    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    int i;
    for (i = 0; i < num_threads; i++) {
        thread_data_array[i].in_file = in_file;
        thread_data_array[i].out_file = out_file;
        thread_data_array[i].thread_id = i;
        pthread_create(&threads[i], NULL, compress_chunk, (void*) &thread_data_array[i]);
    }

    // Wait for threads to finish
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Close files and free memory
    gzclose(in_file);
    gzclose(out_file);
    free(thread_data_array);
    free(threads);
    
    printf("Completed\n");
    return 0;
}
