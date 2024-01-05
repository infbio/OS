#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#define VIRTUAL_PAGES 5000

typedef struct {
    int page_number;
    int in_use;
    int load_time;
    int distance_next;
} Frame_Optimal;

typedef struct {
    int page_number;
    int in_use;
} Frame_FIFO;

typedef struct {
    int page_number;
    int in_use;
    int last_used;
    int load_time;
} Frame_LRU;

typedef struct {
    int page_number;
    int in_use;
    int reference_bit;
    int load_time;
} Frame_Second_Chance;

int findNextUse(int page_number, const int *preload_page_number, int start, int numRequests) {
    for (int i = start; i < numRequests; i++) {
        if (preload_page_number[i] == page_number) {
            return i;
        }
    }
    return INT_MAX;
}
int main(void)
{
    int vlength, f_size, psize, pr_algorithm, input_method;
    while(1) {
        printf("A. Simlulation에 사용할 가상주소 길이를 선택하시오 (1. 18bits 2. 19bits 3. 20bits): ");
        scanf("%d", &vlength);
        if(vlength != 1 && vlength != 2 && vlength != 3) {
            printf("가상주소 길이를 잘못 입력하였습니다.\n");
            continue;
        }

        printf("B. Simulation에 사용할 페이지(프레임)의 크기를 선택하시오 (1. 1KB 2. 2KB 3. 4KB): ");
        scanf("%d", &f_size);
        if(f_size != 1 && f_size != 2 && f_size != 3) {
            printf("페이지(프레임)의 크기를 잘못 입력하였습니다.\n");
            continue;
        }

        printf("C. Simulation에 사용할 물리메모리의 크기를 선택하시오 (1. 32KB 2. 64KB): ");
        scanf("%d", &psize);
        if(psize != 1 && psize != 2) {
            printf("물리메모리를 크기를 잘못 입력하였습니다.\n");
            continue;
        }

        printf("D. Simulation에 적용할 Page Replacement 알고리즘을 선택하시오\n");
        printf("(1. Optimal 2. FIFO 3. LRU 4. Second-Chance): ");
        scanf("%d", &pr_algorithm);
        if(pr_algorithm != 1 && pr_algorithm != 2 && pr_algorithm != 3 && pr_algorithm != 4) {
            printf("Page Replacement 알고리즘을 잘못 입력하였습니다.\n");
            continue;
        }

        printf("E. 가상주소 스트링 입력방식을 선택하시오\n");
        printf("(1. input.in 자동 생성 2. 기존 파일 사용): ");
        scanf("%d", &input_method);
        if(input_method != 1 && input_method != 2) {
            printf("입력방식을 잘못 입력하였습니다.\n");
            continue;
        }
        break;
    }
    int bits;
    int max_frame_num;
    int frame_size;
    int frame_size_kb;
    switch (vlength) {
    case 1:
        bits = 18;
        break;
    case 2:
        bits = 19;
        break;
    case 3:
        bits = 20;
        break;
    }
    switch (f_size) {
        case 1:
            frame_size_kb = 1; // 1 KB
            break;
        case 2:
            frame_size_kb = 2; // 2 KB
            break;
        case 3:
            frame_size_kb = 4; // 4 KB
            break;
    }
    switch (psize) {
        case 1:
            psize = 32; // 32 KB
            break;
        case 2:
            psize = 64; // 64 KB
            break;
    }
    frame_size = frame_size_kb * 1024;
    max_frame_num = psize / frame_size_kb; // 8 ~ 64
    
    int pageRequests[VIRTUAL_PAGES];
    if(input_method == 1) {
        FILE* generate_file = fopen("input.in", "w");
        if (generate_file == NULL) {
            perror("파일 여는데 오류");
            exit(EXIT_FAILURE);
        }

        srand((unsigned int)time(NULL));
        unsigned int max_virtual_address = (1U << bits) - 1;

        for (int i = 0; i < VIRTUAL_PAGES; i++) {
            unsigned int rand_part1 = rand();
            unsigned int rand_part2 = rand();

            unsigned int virtual_address = ((rand_part1 << 15) | rand_part2) % (max_virtual_address + 1U);

            fprintf(generate_file, "%d\n", virtual_address);
        }
        fclose(generate_file);

        FILE* generated_file = fopen("input.in", "r");
        for(int i=0; i < VIRTUAL_PAGES; i++){
            if(fscanf(generated_file, "%d", &pageRequests[i]) != 1) {
                fprintf(stderr, "%d번째 줄에서 주소 읽는데 실패했습니다.\n", i+1);
                fclose(generated_file);
                exit(EXIT_FAILURE);
            }
        }
        fclose(generated_file);
    } else {
        char read_filename[100];
        printf("입력 파일 이름을 입력하시오 ");
        scanf("%s", read_filename);
        FILE* read_file = fopen(read_filename, "r");
        if (read_file == NULL) {
            perror("파일 여는데 오류");
            exit(EXIT_FAILURE);
        }

        for(int i=0; i < VIRTUAL_PAGES; i++){
            if(fscanf(read_file, "%d", &pageRequests[i]) != 1) {
                fprintf(stderr, "%d번째 줄에서 주소 읽는데 실패했습니다.\n", i+1);
                fclose(read_file);
                exit(EXIT_FAILURE);
            }
        }
        fclose(read_file);
    }
    int virtual_address, page_number, frame_number, physical_address, page_fault_count, oldest_frame_index = 0;
    FILE* output_file;
    switch (pr_algorithm) {
        case 1:  //optimal
        {
            Frame_Optimal *frame_table_optimal = malloc(max_frame_num * sizeof(Frame_Optimal));
            output_file = fopen("output.opt", "w");
            fprintf(output_file, "No.\tV.A.\tPage No.\tFrame No.\tP.A.\tPage Fault\n");
            if (frame_table_optimal == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }

            for (int i = 0; i < max_frame_num; i++) {
                frame_table_optimal[i].page_number = -1;
                frame_table_optimal[i].in_use = 0;
                frame_table_optimal[i].load_time = -1;
            }

            page_fault_count = 0;
        
            int preload_page_number[VIRTUAL_PAGES];
            for(int i=0; i < VIRTUAL_PAGES; i++) {
                preload_page_number[i] = pageRequests[i] / frame_size;
            }

            for (int i = 0; i < VIRTUAL_PAGES; i++) {
                virtual_address = pageRequests[i];
                page_number = preload_page_number[i];
                int found = 0;

                for (int j = 0; j < max_frame_num; j++) {
                    if (frame_table_optimal[j].page_number == page_number && frame_table_optimal[j].in_use) {
                        found = 1; // hit
                        frame_number = j;
                        break;
                    }
                }

                if (!found) {
                    int furthestUse = -1;
                    int replaceIndex = -1;

                    for (int j = 0; j < max_frame_num; j++) {
                        if (!frame_table_optimal[j].in_use) {
                            replaceIndex = j;
                            break;
                        }

                        int nextUse = findNextUse(frame_table_optimal[j].page_number, preload_page_number, i + 1, VIRTUAL_PAGES);
                        if (nextUse > furthestUse) {
                            furthestUse = nextUse;
                            replaceIndex = j;
                        } else if(nextUse == furthestUse) {
                            if(frame_table_optimal[j].load_time < frame_table_optimal[replaceIndex].load_time) {
                                replaceIndex = j;
                            }
                        }
                    }

                    frame_table_optimal[replaceIndex].page_number = page_number;
                    frame_table_optimal[replaceIndex].in_use = 1;
                    frame_table_optimal[replaceIndex].load_time = i;
                    frame_number = replaceIndex;
                    page_fault_count++;
                }

                physical_address = (frame_number * frame_size) + (virtual_address % frame_size);
                fprintf(output_file, "%d\t%d\t%d\t\t%d\t\t%d\t%s\n",
                    i + 1,
                    virtual_address,
                    page_number,
                    frame_number,
                    physical_address,
                    found ? "H" : "F");
            }
            fprintf(output_file, "Total Number of Page Faults: %d\n", page_fault_count);
            fclose(output_file);
            free(frame_table_optimal);
            break;
        }
        case 2: //fifo
        {
            Frame_FIFO *frame_table_fifo = malloc(max_frame_num * sizeof(Frame_FIFO));
            output_file = fopen("output.fifo", "w");
            fprintf(output_file, "No.\tV.A.\tPage No.\tFrame No.\tP.A.\tPage Fault\n");
            if (frame_table_fifo == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            for(int i=0; i<max_frame_num; i++) {
                frame_table_fifo[i].page_number = -1;
                frame_table_fifo[i].in_use = 0;
            }

            page_fault_count = 0;

            for(int i=0; i < VIRTUAL_PAGES; i++) {
                virtual_address = pageRequests[i];
                page_number = virtual_address / frame_size;
                
                int found = 0;
                for(int j=0; j < max_frame_num; j++) {
                    if(frame_table_fifo[j].page_number == page_number && frame_table_fifo[j].in_use) {
                        found = 1; //hit
                        frame_number = j;
                        break;
                    }
                }

                if(!found) {
                    //page fault
                    page_fault_count++;
                    frame_number = oldest_frame_index;
                    frame_table_fifo[frame_number].page_number = page_number;
                    frame_table_fifo[frame_number].in_use = 1;
                    oldest_frame_index = (oldest_frame_index + 1) % max_frame_num;
                }

                physical_address = (frame_number * frame_size) + (virtual_address % frame_size);
                fprintf(output_file ,"%d\t%d\t%d\t\t%d\t\t%d\t%s\n",
                    i + 1, // No.
                    virtual_address, // V.A.
                    page_number, // Page No.
                    frame_number, // Frame No.
                    physical_address, // P.A.
                    found ? "H" : "F" // Page Fault
                );
            }
            fprintf(output_file, "Total Number of Page Faults: %d\n", page_fault_count);
            free(frame_table_fifo);
            fclose(output_file);
            break;
        }
        case 3: //LRU
        {
            Frame_LRU *frame_table_lru = malloc(max_frame_num * sizeof(Frame_LRU));
            output_file = fopen("output.lru", "w");
            fprintf(output_file, "No.\tV.A.\tPage No.\tFrame No.\tP.A.\tPage Fault\n");
            if (frame_table_lru == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }

            for (int i = 0; i < max_frame_num; i++) {
                frame_table_lru[i].page_number = -1;
                frame_table_lru[i].in_use = 0;
                frame_table_lru[i].last_used = 0;
                frame_table_lru[i].load_time = -1;
            }

            page_fault_count = 0;
            int currentTime = 0;

            for (int i = 0; i < VIRTUAL_PAGES; i++) {
                virtual_address = pageRequests[i];
                page_number = virtual_address / frame_size;
                int found = 0;

                for (int j = 0; j < max_frame_num; j++) {
                    if (frame_table_lru[j].page_number == page_number) {
                        found = 1; // Page hit
                        frame_table_lru[j].last_used = currentTime;
                        frame_number = j;
                        break;
                    }
                }

                if (!found) {
                    int lru_index = -1;
                    int oldestTime = INT_MAX;
                    int oldestLoadTime = INT_MAX;

                    for (int j = 0; j < max_frame_num; j++) {
                        if (!frame_table_lru[j].in_use) {
                            lru_index = j;
                            break;
                        } else if (frame_table_lru[j].last_used < oldestTime) {
                            oldestTime = frame_table_lru[j].last_used;
                            oldestLoadTime = frame_table_lru[j].load_time;
                            lru_index = j;
                        } else if (frame_table_lru[j].last_used == oldestTime && frame_table_lru[j].load_time < oldestLoadTime) {
                            oldestLoadTime = frame_table_lru[j].load_time;
                            lru_index = j;
                        }
                    }

                    frame_table_lru[lru_index].page_number = page_number;
                    frame_table_lru[lru_index].last_used = currentTime;
                    frame_table_lru[lru_index].load_time = currentTime;
                    frame_table_lru[lru_index].in_use = 1;
                    frame_number = lru_index;
                    page_fault_count++;
                }

                physical_address = (frame_number * frame_size) + (virtual_address % frame_size);
                fprintf(output_file, "%d\t%d\t%d\t\t%d\t\t%d\t%s\n",
                    i + 1, // No.
                    virtual_address, // V.A.
                    page_number, // Page No.
                    frame_number, // Frame No.
                    physical_address, // P.A.
                    found ? "H" : "F" // Page Fault (H for hit, F for fault)
                );

                currentTime++;
            }

            fprintf(output_file, "Total Number of Page Faults: %d\n", page_fault_count);
            fclose(output_file);
            free(frame_table_lru);
            break;
        }
        case 4:
        {//second-chance
            Frame_Second_Chance *frame_table_sc = malloc(max_frame_num * sizeof(Frame_Second_Chance));
            output_file = fopen("output.sc", "w");
            fprintf(output_file, "No.\tV.A.\tPage No.\tFrame No.\tP.A.\tPage Fault\n");
            if (frame_table_sc == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            
            for (int i = 0; i < max_frame_num; i++) {
                frame_table_sc[i].page_number = -1;
                frame_table_sc[i].in_use = 0;
                frame_table_sc[i].reference_bit = 0;
                frame_table_sc[i].load_time = -1;
            }

            page_fault_count = 0;
            int hand = 0;

            for (int i=0; i < VIRTUAL_PAGES; i++) {
                virtual_address = pageRequests[i];
                page_number = virtual_address / frame_size;
                int found = 0;

                for (int j=0; j < max_frame_num; j++) {
                    if(frame_table_sc[j].page_number == page_number && frame_table_sc[j].in_use) {
                        found = 1; //hit
                        frame_number = j;
                        frame_table_sc[j].reference_bit = 1;
                        break;
                    }
                }
            
                if(!found) {
                    page_fault_count++;
                    while (frame_table_sc[hand].reference_bit == 1) {
                        frame_table_sc[hand].reference_bit = 0;
                        hand = (hand + 1) % max_frame_num;
                    }
                    frame_number = hand;
                    frame_table_sc[hand].page_number = page_number;
                    frame_table_sc[hand].in_use = 1;
                    frame_table_sc[hand].load_time = i;
                    hand = (hand + 1) % max_frame_num;
                }
                physical_address = (frame_number * frame_size) + (virtual_address % frame_size);
                fprintf(output_file, "%d\t%d\t%d\t\t%d\t\t%d\t%s\n",
                    i + 1, // No.
                    virtual_address, // V.A.
                    page_number, // Page No.
                    frame_number, // Frame No.
                    physical_address, // P.A.
                    found ? "H" : "F" // Page Fault
                );
            }

            fprintf(output_file, "Total Number of Page Faults: %d\n", page_fault_count);
            fclose(output_file);
            free(frame_table_sc);
            break;
        }
    }
    return 0;
}