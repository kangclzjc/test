#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <cctype>
#include <iostream> 
#include <experimental/filesystem>
#include <fcntl.h>
using namespace std;

namespace fs = std::experimental::filesystem;

#define BUFFER_SIZE 1024
const long long NUM_BYTES = 1ll * 1024 * 1024 * 1024; // 1 MB

int main(int argc, char *argv[]) {
    int opt;
    char *filename = NULL;
    int type = 0;
    int rw = 0;
    int io_type = 0;
    unsigned long long bs = 0;
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
                min_bs = atoll(optarg);
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
            if (io_type == 0) {
                // seq read
                if (bs != 0) {
                    // read file with fixed length
                    // open the file for reading
                    printf("fread seq read with fixed length %d\n", bs);
                    FILE *fp_read;
                    char buffer[bs];
                    fp_read = fopen(filename, "r+b");
                    if (fp_read == NULL) {
                        perror("Error opening input file");
                        exit(EXIT_FAILURE);
                    }

                    long long tot = NUM_BYTES * size;
                    long long num_bytes_read = 0;
                    size_t num_read;
                    while ((num_read = fread(buffer, sizeof(char), bs, fp_read)) > 0 && num_bytes_read < tot) {
                        num_bytes_read += num_read;
                    }
                    printf("fread %lld bytes\n", num_bytes_read);
                    fclose(fp_read);
                } else {
                    // read file with random length
                    printf("fread seq read with random length %d\n", bs);
                    FILE *fp_read;
                    char buffer[max_bs + 1];
                    fp_read = fopen(filename, "r+b");
                    if (fp_read == NULL) {
                        perror("Error opening input file");
                        exit(EXIT_FAILURE);
                    }
                    if (max_bs == 0) {
                        perror("max_bs must larger than 0");
                        return -1;
                    }

                    // seed the random number generator
                    srand(time(NULL));
                    long long tot = NUM_BYTES * size;
                    long long num_bytes_read = 0;
                    size_t num_read;
                    while (!feof(fp_read) && num_bytes_read < tot) {
                        int chunk_size = rand() % (max_bs - min_bs + 1) + min_bs;
                        num_read = fread(buffer, sizeof(char), chunk_size, fp_read);
                        
                        if (num_read == 0) {
                            break;
                        }
                        num_bytes_read += num_read;
                    }
                    printf("fread %lld bytes\n", num_bytes_read);
                    fclose(fp_read);
                }
            } else {
                // random read
                if (bs != 0) {
                    printf("fread random read with fixed length %d\n", bs);
                    FILE *fp_read;
                    char buffer[bs];
                    fp_read = fopen(filename, "r+b");
                    if (fp_read == NULL) {
                        perror("Error opening input file");
                        exit(EXIT_FAILURE);
                    }

                    srand(time(NULL));
                    // Get the file size
                    std::uintmax_t file_size = fs::file_size(filename); 
                    printf("file_size %ld\n",file_size);

                    long long tot = NUM_BYTES * size;
                    long long num_bytes_read = 0;
                    size_t num_read;
                    while (!feof(fp_read) && num_bytes_read < tot) {
                        // Generate a random position within the file
                        int position = rand() % file_size;
                        // Seek to the random position within the file
                        fseek(fp_read, position, SEEK_SET);
                        num_read = fread(buffer, sizeof(char), bs, fp_read);
                        num_bytes_read += num_read;
                    }
                    printf("fread %lld bytes\n", num_bytes_read);
                    fclose(fp_read);
                } else {
                    printf("fread random read with random length %d\n", bs);
                    FILE *fp_read;
                    char buffer[max_bs + 1];
                    fp_read = fopen(filename, "r+b");
                    if (fp_read == NULL) {
                        perror("Error opening input file");
                        exit(EXIT_FAILURE);
                    }

                    // Get the file size
                    std::uintmax_t file_size = fs::file_size(filename); 

                    long long tot = NUM_BYTES * size;
                    long long num_bytes_read = 0;
                    size_t num_read;
                    srand(time(NULL));
                    while (!feof(fp_read) && num_bytes_read < tot) {
                        // Generate a random position within the file
                        int position = rand() % file_size;
                        // Seek to the random position within the file
                        fseek(fp_read, position, SEEK_SET);
                        int chunk_size = rand() % (max_bs - min_bs + 1) + min_bs;
                        chunk_size = min(1ull* chunk_size, max(1ull, file_size - num_bytes_read - chunk_size));
                        num_read = fread(buffer, sizeof(char), chunk_size, fp_read);
                        num_bytes_read += num_read;
                    }
                    printf("fread %lld bytes\n", num_bytes_read);
                    fclose(fp_read);
                }
            }
        } else {
            // write
            if (io_type == 0) {
                // seq write
                if (bs != 0) {
                    // write file with fixed length
                    printf("fwrite seq write with fixed length %d\n", bs);
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
                        for (int i = 0; i < bs; i++) {
                            buffer[i] = rand() % 256;
                        }

                        // write the buffer to the file
                        int num_bytes_to_write = bs;
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
                    printf("fwrite seq write with random length from %d to %d\n", min_bs, max_bs);

                    srand(time(NULL));
                    if (max_bs == 0) {
                        perror("max_bs must larger than 0");
                        return -1;
                    }

                    FILE *fp;
                    // open the file for writing
                    fp = fopen(filename, "w");
                    if (fp == NULL) {
                        perror("Error opening file");
                        return -1;
                    }
                    long long num_bytes_written = 0;
                    long long tot = NUM_BYTES * size;
                    while (num_bytes_written < tot) {
                        int STRING_LENGTH = rand() % (max_bs - min_bs + 1) + min_bs;
                        char random_string[STRING_LENGTH+1];
                        const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
                        for (int i = 0; i < STRING_LENGTH - 1; i++) {
                            int index = rand() % (sizeof(charset) - 1);
                            random_string[i] = charset[index];
                        }
                        random_string[STRING_LENGTH - 1] = '\n';
                        random_string[STRING_LENGTH] = '\0';
                        int num_bytes_to_write = STRING_LENGTH;
                        if (num_bytes_written + num_bytes_to_write > tot) {
                            num_bytes_to_write = tot - num_bytes_written;
                        }
                        fwrite(random_string, sizeof(char), STRING_LENGTH, fp);
                        num_bytes_written += num_bytes_to_write;
                    }
                    fclose(fp);
                }
            } else {
                // random write
                if (bs != 0) {
                    // random write file with fixed length
                    printf("fwrite random write with fixed length %d\n", bs);
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
                    // Get the file size
                    // std::uintmax_t file_size = fs::file_size(filename); 
                    // printf("file_size %ld\n",file_size);

                    long long tot = NUM_BYTES * size;
                    // generate random data and write it to the file
                    while (num_bytes_written < tot) {
                        // fill the buffer with random data
                        for (int i = 0; i < bs; i++) {
                            buffer[i] = rand() % 256;
                        }

                        // write the buffer to the file
                        int num_bytes_to_write = bs;
                        if (num_bytes_written + num_bytes_to_write > tot) {
                            num_bytes_to_write = tot - num_bytes_written;
                        }
                        int position = rand() % (num_bytes_written + 1);
                        // Seek to the random position within the file
                        fseek(fp, position, SEEK_SET);
                        fwrite(buffer, sizeof(char), num_bytes_to_write, fp);
                        num_bytes_written += num_bytes_to_write;
                    }

                    // close the file
                    fclose(fp);
                } else {
                    // random write file with random length
                    printf("fwrite random write with random length\n");
                    srand(time(NULL));
                    if (max_bs == 0 || min_bs == 0) {
                        perror("max_bs or min_bs must larger than 0");
                        return -1;
                    }

                    FILE *fp;
                    // open the file for writing
                    fp = fopen(filename, "w");
                    if (fp == NULL) {
                        perror("Error opening file");
                        return -1;
                    }
                    long long num_bytes_written = 0;
                    long long tot = NUM_BYTES * size;
                    while (num_bytes_written < tot) {
                        int STRING_LENGTH = rand() % (max_bs - min_bs + 1) + min_bs;
                        char random_string[STRING_LENGTH+1];
                        const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
                        for (int i = 0; i < STRING_LENGTH - 1; i++) {
                            int index = rand() % (sizeof(charset) - 1);
                            random_string[i] = charset[index];
                        }
                        random_string[STRING_LENGTH - 1] = '\n';
                        random_string[STRING_LENGTH] = '\0';
                        int num_bytes_to_write = STRING_LENGTH;
                        if (num_bytes_written + num_bytes_to_write > tot) {
                            num_bytes_to_write = tot - num_bytes_written;
                        }
                        int position = rand() % (num_bytes_written + 1);
                        // Seek to the random position within the file
                        fseek(fp, position, SEEK_SET);
                        fwrite(random_string, sizeof(char), STRING_LENGTH, fp);
                        num_bytes_written += num_bytes_to_write;
                    }
                    fclose(fp);
                }
            }
        }     
    } else {
        //read or write
        if (rw == 0) {
            // read
            if (io_type == 0) {
                // seq read
                if (bs != 0) {
                    // read file with fixed length
                    // open the file for reading
                    printf("read seq read with fixed length %d\n", bs);
                    
                    char buffer[bs];
                    int fp_read = open(filename, O_RDWR);
                    if (fp_read == -1) {
                        perror("Error opening input file");
                        exit(EXIT_FAILURE);
                    }

                    long long tot = NUM_BYTES * size;
                    long long num_bytes_read = 0;
                    size_t num_read;
                    while ((num_read = read(fp_read, buffer, bs)) > 0 && num_bytes_read < tot) {
                        num_bytes_read += num_read;
                    }
                    printf("fread %lld bytes\n", num_bytes_read);
                    close(fp_read);
                } else {
                    // read file with random length
                    printf("read seq read with random length %d\n", bs);
                    char buffer[max_bs + 1];
                    int fp_read = open(filename, O_RDWR);
                    if (fp_read == -1) {
                        perror("Error opening input file");
                        exit(EXIT_FAILURE);
                    }
                    if (max_bs == 0 || min_bs == 0) {
                        perror("max_bs or min_bs must larger than 0");
                        return -1;
                    }

                    // seed the random number generator
                    srand(time(NULL));
                    long long tot = NUM_BYTES * size;
                    long long num_bytes_read = 0;
                    size_t num_read;
                    unsigned int seed = (unsigned int)time(NULL);
                    while ((num_read = read(fp_read, buffer, rand_r(&seed) % (max_bs - min_bs + 1) + min_bs)) > 0 && num_bytes_read < tot) {
                        num_bytes_read += num_read;
                    }
                    printf("fread %lld bytes\n", num_bytes_read);
                    close(fp_read);
                }
            } else {
                // random read
                if (bs != 0) {
                    printf("read random read with fixed length %d\n", bs);
                    
                    char buffer[bs];
                    int fp_read = open(filename, O_RDWR);
                    if (fp_read == -1) {
                        perror("Error opening input file");
                        exit(EXIT_FAILURE);
                    }

                    srand(time(NULL));
                    // Get the file size
                    std::uintmax_t file_size = fs::file_size(filename); 
                    printf("file_size %ld\n",file_size);

                    long long tot = NUM_BYTES * size;
                    long long num_bytes_read = 0;
                    size_t num_read;
                    while (num_bytes_read < tot) {
                        // Generate a random position within the file
                        off_t position = rand() % file_size;
                        // Seek to the random position within the file
                        lseek(fp_read, position, SEEK_SET);
                        bs = min(1ull * bs, max(1ull, file_size - num_bytes_read - position));
                        num_read = read(fp_read, buffer, bs);
                        num_bytes_read += num_read;
                    }
                    printf("read %lld bytes\n", num_bytes_read);
                    close(fp_read);
                } else {
                    printf("read random read with random length %d\n", bs);
                    char buffer[max_bs + 1];
                    int fp_read = open(filename, O_RDWR);
                    if (fp_read == -1) {
                        perror("Error opening input file");
                        exit(EXIT_FAILURE);
                    }

                    // Get the file size
                    std::uintmax_t file_size = fs::file_size(filename); 

                    long long tot = NUM_BYTES * size;
                    long long num_bytes_read = 0;
                    size_t num_read;
                    srand(time(NULL));
                    while (num_bytes_read < tot) {
                        // Generate a random position within the file
                        int position = rand() % file_size;
                        // Seek to the random position within the file
                        lseek(fp_read, position, SEEK_SET);
                        int chunk_size = rand() % (max_bs - min_bs + 1) + min_bs;
                        chunk_size = min(1ull* chunk_size, max(1ull, file_size - num_bytes_read - chunk_size));
                        num_read = read(fp_read, buffer, chunk_size);
                        if (num_read == -1) {
                            perror("Error reading input file");
                            exit(EXIT_FAILURE);
                        }
                        num_bytes_read += num_read;
                    }
                    printf("fread %lld bytes\n", num_bytes_read);
                    close(fp_read);
                }
            }
        } else {
            // write
            if (io_type == 0) {
                // seq write
                if (bs != 0) {
                    // write file with fixed length
                    printf("write seq write with fixed length %ull\n", bs);
                    char buffer[bs];
                    long long num_bytes_written = 0;

                    // open the file for writing
                    int fp = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                    if (fp == -1) {
                        perror("Error opening file");
                        return -1;
                    }
                    // seed the random number generator
                    srand(time(NULL));
                    long long tot = NUM_BYTES * size;
                    // generate random data and write it to the file
                    while (num_bytes_written < tot) {
                        // fill the buffer with random data
                        for (int i = 0; i < bs; i++) {
                            buffer[i] = rand() % 256;
                        }

                        // write the buffer to the file
                        int num_bytes_to_write = bs;
                        if (num_bytes_written + num_bytes_to_write > tot) {
                            num_bytes_to_write = tot - num_bytes_written;
                        }
                        write(fp, buffer, num_bytes_to_write);
                        num_bytes_written += num_bytes_to_write;
                    }

                    // close the file
                    close(fp);
                } else {
                    // write file with random length
                    printf("write seq write with random length from %d to %d\n", min_bs, max_bs);

                    srand(time(NULL));
                    if (max_bs == 0 || min_bs == 0) {
                        perror("max_bs or min_bs must larger than 0");
                        return -1;
                    }

                    // open the file for writing
                    int fp = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fp == -1) {
                        perror("Error opening file");
                        return -1;
                    }
                    long long num_bytes_written = 0;
                    long long tot = NUM_BYTES * size;
                    while (num_bytes_written < tot) {
                        int STRING_LENGTH = rand() % (max_bs - min_bs + 1) + min_bs;
                        char random_string[STRING_LENGTH+1];
                        const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
                        for (int i = 0; i < STRING_LENGTH - 1; i++) {
                            int index = rand() % (sizeof(charset) - 1);
                            random_string[i] = charset[index];
                        }
                        random_string[STRING_LENGTH - 1] = '\n';
                        random_string[STRING_LENGTH] = '\0';

                        int num_bytes_to_write = STRING_LENGTH;
                        if (num_bytes_written + num_bytes_to_write > tot) {
                            num_bytes_to_write = tot - num_bytes_written;
                        }

                        int bytes_written = write(fp, random_string, num_bytes_to_write);
                        if (bytes_written == -1) {
                            perror("Error writing to file");
                            close(fp);
                            return -1;
                        }
                        num_bytes_written += bytes_written;
                    }
                    close(fp);
                }
            } else {
                // random write
                if (bs != 0) {
                    // random write file with fixed length
                    printf("write random write with fixed length %d\n", bs);
                    char buffer[bs];
                    long long num_bytes_written = 0;

                    // open the file for writing
                    int fp = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                    if (fp == -1) {
                        perror("Error opening file");
                        return -1;
                    }

                    // seed the random number generator
                    srand(time(NULL));
                    // Get the file size
                    // std::uintmax_t file_size = fs::file_size(filename); 
                    // printf("file_size %ld\n",file_size);

                    long long tot = NUM_BYTES * size;
                    // generate random data and write it to the file
                    while (num_bytes_written < tot) {
                        // fill the buffer with random data
                        for (int i = 0; i < bs; i++) {
                            buffer[i] = rand() % 256;
                        }

                        // write the buffer to the file
                        int num_bytes_to_write = bs;
                        if (num_bytes_written + num_bytes_to_write > tot) {
                            num_bytes_to_write = tot - num_bytes_written;
                        }
                        int position = rand() % (num_bytes_written + 1);
                        // Seek to the random position within the file
                        lseek(fp, position, SEEK_SET);
                        write(fp, buffer, num_bytes_to_write);
                        num_bytes_written += num_bytes_to_write;
                    }

                    // close the file
                    close(fp);
                } else {
                    // random write file with random length
                    printf("write random write with random length\n");
                    srand(time(NULL));
                    if (max_bs == 0 || min_bs == 0) {
                        perror("max_bs or min_bs must larger than 0");
                        return -1;
                    }

                    // open the file for writing
                    int fp = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                    if (fp == -1) {
                        perror("Error opening file");
                        return -1;
                    }
                    long long num_bytes_written = 0;
                    long long tot = NUM_BYTES * size;
                    while (num_bytes_written < tot) {
                        int STRING_LENGTH = rand() % (max_bs - min_bs + 1) + min_bs;
                        char random_string[STRING_LENGTH+1];
                        const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
                        for (int i = 0; i < STRING_LENGTH - 1; i++) {
                            int index = rand() % (sizeof(charset) - 1);
                            random_string[i] = charset[index];
                        }
                        random_string[STRING_LENGTH - 1] = '\n';
                        random_string[STRING_LENGTH] = '\0';
                        int num_bytes_to_write = STRING_LENGTH;
                        if (num_bytes_written + num_bytes_to_write > tot) {
                            num_bytes_to_write = tot - num_bytes_written;
                        }
                        int position = rand() % (num_bytes_written + 1);
                        // Seek to the random position within the file
                        if (lseek(fp, position, SEEK_SET) == -1) {
                            perror("Error seeking in file");
                            return -1;
                        }
                        if (write(fp, random_string, num_bytes_to_write) != num_bytes_to_write) {
                            perror("Error writing to file");
                            return -1;
                        }
                        num_bytes_written += num_bytes_to_write;
                    }
                    close(fp);
                }
            }
        }    
    }

    
    return 0;
}
