#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "bench.c"
#include "bloom.h"
#include "tst.h"

#define TableSize 5000000 /* size of bloom filter */

/** constants insert, delete, max word(s) & stack nodes */
enum { INS, DEL, WRDMAX = 256, STKMAX = 512, LMAX = 1024 };

#define REF INS
#define CPY DEL

long poolsize = 2000000 * WRDMAX;

#define IN_FILE "cities.txt"
#define TRAVERSE_DATA "raw_data.txt"
#define COMPRESSED_DATA "compress_data.txt"

int main(int argc, char **argv)
{
    tst_node *root = NULL;
    int rtn = 0, idx = 0;
    int nodes_cnt = 0, cmpr_nodes_cnt = 0;
    char arr[BUFFER_SIZE];
    memset(arr, '\0', BUFFER_SIZE);
    FILE *fp = fopen(IN_FILE, "r");
    double t1, t2;

    if (!fp) { /* prompt, open, validate file for reading */
        fprintf(stderr, "error: file open failed '%s'.\n", argv[1]);
        return 1;
    }
    t1 = tvgetf();

    bloom_t bloom = bloom_create(TableSize);

    /* memory pool */
    char *pool = (char *) malloc(poolsize * sizeof(char));
    char *Top = pool;
    while ((rtn = fscanf(fp, "%s", Top)) != EOF) {
        char *p = Top;
        /* insert reference to each string */
        if (!tst_ins_del(&root, &p, INS, REF)) { /* fail to insert */
            fprintf(stderr, "error: memory exhausted, tst_insert.\n");
            fclose(fp);
            return 1;
        } else { /* update bloom filter */
            bloom_add(bloom, Top);
        }
        idx++;
        Top += (strlen(Top) + 1);
    }
    t2 = tvgetf();

    fclose(fp);
    printf("ternary_tree, loaded %d words in %.6f sec\n", idx, t2 - t1);
    nodes_cnt = tst_size_count(root);
    memset(arr, 0, BUFFER_SIZE);
    tst_traverse_seq(root, arr, 0, fopen(TRAVERSE_DATA, "w"));

    puts("\nCompressing\n");
    t1 = tvgetf();
    tst_compress(&root);
    t2 = tvgetf();

    printf("ternary_tree, compressed tree in %.6f sec\n", t2 - t1);
    cmpr_nodes_cnt = tst_size_count(root);
    memset(arr, 0, BUFFER_SIZE);
    tst_traverse_seq(root, arr, 0, fopen(COMPRESSED_DATA, "w"));


    double eff = 100 - (((double) cmpr_nodes_cnt / (double) nodes_cnt) * 100);
    printf("compress ratio %.6f %%\n", eff);
}