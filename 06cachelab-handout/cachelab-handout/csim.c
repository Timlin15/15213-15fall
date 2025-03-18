#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "cachelab.h"
#include <unistd.h>

int asso, verbose;
int hits = 0, misses = 0, evictions = 0;

struct caches {
    int valid;
    int timestamp;
    int tags;
};
int load(int currSet, int tag, struct caches **cache);
int store(int currSet, int tag, struct caches **cache);
void updateRow(int currSet, struct caches **cache);
int findLargest(int currSet, struct caches **cache);
void update(int code);

int main(int argc, char *argv[])
{
    int opt, set;
    char filename[64];
    while ((opt = getopt(argc, argv, "vs:E:b:t:")) != -1) {
        switch (opt) {
            case 'v':
                verbose = 1;
                break;
            case 's':
                set = 1<<atoi(optarg);
                break;
            case 'E':
                asso = atoi(optarg);
                break;
            case 'b':
                break;
            case 't':
                strcpy(filename, optarg);
                break;
            default:
        }
    }

    struct caches **cacheline = malloc(set * sizeof(struct caches *));
    for (int i = 0; i < set; i++) {
        cacheline[i] = malloc(asso * sizeof(struct caches));
        for (int j = 0; j < asso; j++) {// initialization
            cacheline[i][j].valid = 0;
            cacheline[i][j].tags = 0;
            cacheline[i][j].timestamp = 0;
        }
    } // allocate a memory belong to caches

    FILE *fp = fopen(filename, "r");  // "r" for reading

    size_t address;
    int offset;
    char instruction;
    while (fscanf(fp, " %c %lx,%d\n", &instruction, &address, &offset) == 3) {
        int currSet = address >> 4 & 0xf;
        int tag = address >> 8 & 0xf;
        if (instruction != 'I') {
            if (verbose) {
                printf("%c %lx,%d", instruction, address, offset);
            }
            switch (instruction) {
                case 'L':
                    int code1 = load(currSet, tag, &cacheline[currSet][tag]);
                    update(code1);
                    printf("\n");
                    break;
                case 'S':
                    int code2 = store(currSet, tag, &cacheline[currSet][tag]);
                    update(code2);
                    printf("\n");
                    break;
                case 'M': {
                    int code3 = load(currSet, tag, &cacheline[currSet][tag]);
                    int code4 = store(currSet, tag, &cacheline[currSet][tag]);
                    update(code3);
                    update(code4);
                    printf("\n");
                    break;
                }
            }
        }
    }
    printf("hits:%d misses:%d evictions:%d", hits, misses, evictions);
    printSummary(0, 0, 0);
    free(cacheline);
    return 0;
}

/** Will return 1 if it is a hit, 0 if it is a cold miss, 2 if it needs an evict */
int load(int currSet, int tag, struct caches **cache) {
    int cold = 0;
    for (int i = 0; i < asso; i++) {
        if (cache[currSet][i].valid && cache[currSet][i].tags==tag) {
            cache[currSet][i].timestamp = 0;
            return 1;
        }
        if (cache[currSet][i].valid == 0) {
            cold = 1;
        }
    }
    if (cold) {
        return 0;
    }
    int largest = findLargest(currSet, cache);
    cache[currSet][largest].valid = 1;
    cache[currSet][largest].tags = tag;
    cache[currSet][largest].timestamp = -1;
    updateRow(currSet, cache);
    return 2;
}

/** update the `valid` value if it is 0, and update every cache in this row if it is a hit.
 *  return 1 if hit, 2 if evicted.
 */
int store(int currSet, int tag, struct caches **cache) {
    int hit = 0, evicted = 0;
    for (int i = 0; i < asso; i++) {
        if (cache[currSet][i].tags == tag) {
            cache[currSet][i].valid = 1;
            cache[currSet][i].timestamp = -1;
            hit = 1;
        }
    }
    if (!hit) {
        int largest = findLargest(currSet, cache);
        cache[currSet][largest].timestamp = -1;
        cache[currSet][largest].tags = tag;
        evicted = 1;
    }
    updateRow(currSet, cache);
    if (evicted) {
        return 2;
    }
    return 1;
}

/** add every timestamp by 1 if store successfully. */
void updateRow(int currSet, struct caches **cache) {
    for (int i = 0; i < asso; i++) {
        if (cache[currSet][i].valid) {
            cache[currSet][i].timestamp += 1;
        }
    }
}

/** return the row index of the cache with the largest timestamp. */
int findLargest(int currSet, struct caches **cache) {
    int max_index = 0;
    for (int i = 1; i < asso; i++) {
        if (cache[currSet][i].valid) {
            if (cache[currSet][i].timestamp > cache[currSet][max_index].timestamp) {
                max_index = i;
            }
        }
    }
}

void update(int code) {
    if (code == 1) {
        hits++;
        if (verbose) {
           printf(" hit");
        }
    } else if (code == 0) {
        misses++;
        if (verbose) {
            printf(" miss");
        }
    } else {
        evictions++;
        if (verbose) {
            printf(" eviction");
        }
    }
}