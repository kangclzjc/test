#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <cctype>
#include <iostream> 

#define BUFFER_SIZE 1024
const long long NUM_BYTES = 1ll * 1024 * 1024 * 1024; // 1 MB

int main(int argc, char *argv[]) {
    int opt;
    char *filename = NULL;
    int type = 0;
    int rw = 0;
    int io_type = 0;
    int bs = 0;
    int min_bs = 0;
    int max_bs = 4096;
    int size = 10;
    int version_flag = 0;


    static struct option long_options[] = {
        {"type", required_argument, 0, 't'},
        {"rw", required_argument, 0, 'r'},
        {"filename", required_argument, 0, 'f'},
        {"io_type", required_argument, 0, 'i'},
        {"bs", required_argument, 0, 'b'},
        {"min_bs", required_argument, 0, 'l'},
        {"max_bs", required_argument, 0, 'h'},
        {"size", required_argument, 0, 's'},
        {"version", no_argument, 0, 'V'},
        {0, 0, 0, 0}
    };

    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "birlhtvf:V", long_options, &option_index)) != -1) {
        switch (opt) {
            // case 0:
            //     printf ("option %s", long_options[option_index].name);
            //     if (optarg)
            //         printf (" with arg %s", optarg);
            //     printf ("\n");
            //     break;
            case 't':
                type = atoi(optarg);
                break;
            case 'i':
                io_type = atoi(optarg);
                break;
            case 'r':
                rw = atoi(optarg);
                break;
            case 'b':
                bs = atoi(optarg);
                break;            
            case 'l':
                min_bs = atoi(optarg);
                break;
            case 'h':
                max_bs = atoi(optarg);
                break;
            case 's':
                size = atoi(optarg);
                break;
            case 'f':
                filename = optarg;
                break;
            case 'V':
                version_flag = 1;
                break;
            case '?':
                if (optopt == 'f') {
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                } else if (isprint(optopt)) {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                } else {
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                }
                return 1;
            default:
                printf ("?? getopt returned character code 0%o ??\n", optopt);
        }
    }

    if (version_flag) {
        printf("Program version 1.0\n");
        return 0;
    }

    printf("filename = %s, type = %d, rw = %d, io_type = %d, bs = %d, min_bs = %d, max_bs = %d, size = %d\n", filename, type, rw, io_type, bs, min_bs, max_bs, size);
    if (type == 0) {
        //fread or fwrite
        if (rw == 0) {
            // read
        } else {
            // write
            if (io_type == 0) {
                // seq write
                if (bs != 0) {
                    // write file with fixed length
                    printf("fwrite seq write with fixed block size %d\n", bs);
                    char buffer[bs];
                    FILE *fp;
                    long long num_bytes_written = 0;

                    // open the file for writing
                    fp = fopen(filename, "w");
                    if (fp == NULL) {
                        perror("Error opening file");
                        return -1;
                    }

                    // seed the random number generator
                    srand(time(NULL));

                    long long tot = NUM_BYTES * size;
                    // generate random data and write it to the file
                    while (num_bytes_written < tot) {
                        // fill the buffer with random data
                        for (int i = 0; i < BUFFER_SIZE; i++) {
                            buffer[i] = rand() % 256;
                        }

                        // write the buffer to the file
                        int num_bytes_to_write = BUFFER_SIZE;
                        if (num_bytes_written + num_bytes_to_write > tot) {
                            num_bytes_to_write = tot - num_bytes_written;
                        }
                        fwrite(buffer, sizeof(char), num_bytes_to_write, fp);
                        num_bytes_written += num_bytes_to_write;
                    }

                    // close the file
                    fclose(fp);
                } else {
                    // write file with random length
                }
            } else {
                // random write
            }
        }
        
    } else {
        //read or write
    }

    
    return 0;
}
