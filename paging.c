#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int * mem; /* physical memory */
int mem_size = 0;  /* memory size in pages */

struct entry{

    /* R & M bits used in the aging algorithm (R bit will be used in again too) */
    int R;
    int M;

    /* the counter used in NFU */
    unsigned char counter;
}; //page table entry

struct entry page_table[21]; //The page table
/* You will use the page table above only for making page replacement decisions.
   No virtual to physical translation will take place.
   If you want to update/access R bit, for example, in aging algorithm, for page frame x
   you do it as page_table[x].R
*/


/* **  Start of your code ******************************** */
/* TODO: Your job is to implement the following four functions */


/* ********************** NRU replacement policy ************* */
/* input: none */
/* output: frame of page to be replaced */
int nru() {
    //TODO OPTIMIZE AND REDUCE CODE
    int lowestPageNum = 0;
    int lowestMemIndex = 0;

    //Class 0 excluded as by skeleton design, NRU never called when class 0 exist.
    int class1[21];
    int class2[21];
    int class3[21];

    int memClass1[mem_size];
    int memClass2[mem_size];
    int memClass3[mem_size];

    //Initializing class arrays with -1 as default value
    for (int i = 0; i < 21; i++) {
        class1[i] = -1;
        class2[i] = -1;
        class3[i] = -1;
    }
    for (int i = 0; i < mem_size; i++) {
        memClass1[i] = -1;
        memClass2[i] = -1;
        memClass3[i] = -1;
    }


    //CODE IN LONG FORM FOR READABILITY
    //Placing pages into classes by their page frame number
    for (int i = 1; i < 21; i++) {
        int R = page_table[i].R;
        int M = page_table[i].M;

        if (R == 1 && M == 1) {
            class3[i] = i;
        }
        else if (R == 1 && M == 0) {
            class2[i] = i;
        }
        else if (R == 0 && M == 1) {
            class1[i] = i;
        }
    }
    //loading memClasses: Necessary to have first in memory, not first by page number
    for (int i = 1; i < 21; i++) {
        if (class3[i] != -1) {
            for (int mI = 0; mI < mem_size; mI++) {
                if (class3[i] == mem[mI]) {
                    memClass3[mI] = class3[i];
                }
            }
        }
    }
    for (int i = 1; i < 21; i++) {
        if (class2[i] != -1) {
            for (int mI = 0; mI < mem_size; mI++) {
                if (class2[i] == mem[mI]) {
                    memClass2[mI] = class2[i];
                }
            }
        }
    }
    for (int i = 1; i < 21; i++) {
        if (class1[i] != -1) {
            for (int mI = 0; mI < mem_size; mI++) {
                if (class1[i] == mem[mI]) {
                    memClass1[mI] = class1[i];
                }
            }
        }
    }

    //selecting lowest class by Memory and by first in place
    for (int i = 0; i < mem_size; i++) {
        if (memClass3[i] != -1) {
            lowestMemIndex = i;
            lowestPageNum = memClass3[i];
            break;
        }
    }
    for (int i = 0; i < mem_size; i++) {
        if (memClass2[i] != -1) {
            lowestMemIndex = i;
            lowestPageNum = memClass2[i];
            break;
        }
    }
    for (int i = 0; i < mem_size; i++) {
        if (memClass1[i] != -1) {
            lowestMemIndex = i;
            lowestPageNum = memClass1[i];
            break;
        }
    }

    //resetting replaced value to not be caught by NRU again
    page_table[lowestPageNum].R = 0;
    page_table[lowestPageNum].M = 0;

    return lowestMemIndex;
}

/* ************************************************************* */
/* input: page number referenced
   R and M for that page
*/
/* output: NONE */
/* This function udpates info in the page table to reflect the page reference.
   This information will be used later for page replacement */
void nru_pt_update(int page, int R, int M) {
    page_table[page].R = R;
    page_table[page].M = M;
}
/* ********************** aging replacement policy **************** */
/* input: none */
/* output: frame of page to be replaced */
int aging() {
    int lowestMemIndex = 0;
    int lowestPageNumber = 0;
    int currentLowestVal = 255;
    for (int i = 0; i < mem_size; i++) { //this grabs lowest possible value, but not necessarily unique
        for (int j = 1; j < 21; j++) {
            if (j == mem[i]) {
                if (page_table[j].counter < currentLowestVal) {
                    currentLowestVal = page_table[j].counter;
                    lowestMemIndex = i;
                }
            }
        }
    }

    for (int i = 0; i < mem_size; i++) { //getting first matching low value
        for (int j = 1; j < 21; j++) {
            if (j == mem[i]) {
                if (page_table[j].counter == currentLowestVal) {
                    lowestPageNumber = j;
                    lowestMemIndex = i;
                    break;
                }
            }
        }
    }
    //cleaning up removed Page from pageTable;
    page_table[lowestPageNumber].R = 0;
    page_table[lowestPageNumber].counter = 128;

    return lowestMemIndex;
}
/* ************************************************************* */
/* input: page number */
/* output: NONE */
/* This function udpates info in the page table to reflect the page reference.
   This information will be used later for page replacement */
void aging_pt_update(int page) {
    for (int i = 1; i < 21; i++) {
        for (int j = 0; j < mem_size; j++) {
            if (mem[j] == i) { //CURRENTLY IN MEMORY
                if (i == page) { //CURRENT PAGE
                    page_table[i].R = 1;
                    page_table[i].counter = page_table[i].counter >> 1;
                    page_table[i].counter = page_table[i].counter + 128;
                }
                //REST OF PAGES
                else {
                    page_table[i].counter = page_table[i].counter >> 1;
                    if (page_table[i].R == 1) { //adding R to front, if 0 do nothing
                        page_table[i].counter = page_table[i].counter + 128;
                    }
                    page_table[i].R = 0; //CLEARING R BIT
                }
            }
        }
    }
}
/* **************** Methods for Testing Binary ***************** */
/* ************************************************************* */
void printBinary(unsigned char val) {
    for (int bit = 8; bit; --bit) {  // count from 8 to 1
        putchar(val & (1 << (bit - 1)) ? '1' : '0');
    }
}

void printPageTableByMem(){
    for (int i = 0; i < mem_size; i++) {
        for (int j = 1; j < 21; j++) {
            if (j == mem[i]) {
                printf("Page: %d, R: %d, Counter: %d, Binary: ", j, page_table[j].R, page_table[j].counter);
                printBinary(page_table[j].counter);
                printf("\n");
            }
        }
    }
}

/* **************** End of your code *************** */
/* ************************************************************* */


/* ************** Do NOT change anything below this line *** */
/* Input: file handle */
/* Output: none */
/* This function writes the memory content to a file */

void print_mem(FILE *file)
{
    int i;

    for(i = 0; i < mem_size; i++)
        fprintf(file, "%d ", mem[i]);

    fprintf(file, "\n");
}

/***************************************************************/
/* input: page number  */
/* output: the page frame where the paper was found or inserted */
/* This function inserts page, whose number is given in the input, in the physical memory and returns the page frame.
   If the page already exists in the memory, it will return its page fram. */

int insert(int page)
{
    unsigned i;

    for(i = 0; i < mem_size; i++)

        if(mem[i] == page)
            return i;
        else if(!mem[i])
        {
            mem[i] = page;
            return i;
        }

    printf("Memory full and is not suppose to be!!\n");
    exit(1);
}


/***************************************************************/
/* Input:  page number */
/* Output: page hit or page miss */
int mem_check(int page)
{
    unsigned i;

    for(i = 0; i < mem_size; i++)
        if(mem[i] == page)
            return i;

    return -1;

}
/***************************************************************/
/* Input: none */
/* Ouput: 1 if memory is full, 0 otherwise */
int IsFull()
{
    unsigned i;

    for(i = 0; i < mem_size; i++)
        if(!mem[i])
            return 0;

    return 1;
}

/***************************************************************/
int main(int argc, char * argv[])
{
    int policy; /* replacement policy */
    int current;  /* current page accessed */
    FILE * fp; /* The file containing the page accesses */
    FILE * rp; /* output file */
    char filename[30]={""};
    const char * extension[] ={".nru", ".aging"};
    float num_accesses = 0.0; /* total number of page accesses */
    float page_faults = 0.0; /* total number of page faults */
    float mem_full = 0.0; /* total number of times the memory was full */
    unsigned victim = 0;  /* page to be replaced */
    int type = 0; /* The type of page access: 0: read .... 1: write */
    int frame = 0; /* page frame in physical memory */
    int i;

    /* Getting and checking the input from the command line */
    if(argc != 4)
    {
        printf("usage: ./paging policy size filename\n");
        printf("policy: 0 = nru  1 = aging\n");
        printf("size: physical memory size in pages (bigger than 0)\n");
        printf("filenmae: the input file\n");

        exit(1);
    }

    policy = atoi(argv[1]);
    mem_size = atoi(argv[2]);

    if( policy < 0 || policy > 1)
    {
        printf("policy must be 0 or 1\n");
        exit(1);
    }

    if(mem_size <= 0 )
    {
        printf("Size must be a positive integer.\n");
        exit(1);
    }

    /* Allocate and initialize the memory */
    mem = (int *)calloc(mem_size, sizeof(int));
    if(!mem)
    {
        printf("Cannot allocate mem\n");
        exit(1);
    }

    /* Initialize the page table */
    for(i = 0; i < 21; i++)
    {
        page_table[i].R =0;
        page_table[i].M =0;
        page_table[i].counter =0;
    }

    /* open the memory access file */
    fp = fopen(argv[3], "r");
    if(!fp)
    {
        printf("Cannot open file %s\n", argv[3]);
        exit(1);
    }

    /* Create the output file */
    strcat(filename, argv[3]);
    strcat(filename,extension[policy]);
    rp = fopen(filename, "w");
    if(!rp)
    {
        printf("Cannot create file %s\n", filename);
        exit(1);
    }

    /* The main loop of the program */
    while(fscanf(fp,"%d%d", &current, &type) == 2)
    {
        num_accesses++;
        frame = mem_check(current);
        if(frame == -1)
            page_faults++;

        switch(policy)
        {
            case 0: //NRU
                if(frame != -1) //The page is already in memory in page frame given
                    nru_pt_update(current, (type == 0)?1:page_table[current].R, (type == 1)?1:page_table[current].M);

                else  if( IsFull()) //page not in memory and memory is full
                {
                    mem_full++;
                    victim = nru(); //victim is the page frame in physical mem that contains the page to be replaced
                    mem[victim] = current;
                    page_table[current].R = 1-type; //R is 1 when the page is read
                    page_table[current].M = type;  // M is 1 when the pape is written
                    page_table[current].counter = 0; // counter can be neglected in the nru policy.
                }
                else //page not in memory and memory is not full
                {
                    frame = insert(current);
                    nru_pt_update(current, 1-type, type);
                }
                break;

            case 1: //aging
                //TODO REMOVE POST TESTING
                printf("PRE, CURRENT: %d \n", current);
                printPageTableByMem();
                printf("\n");
                if(frame != -1) //The page is already in memory in page frame given
                    aging_pt_update(current);
                else if( IsFull()) //page not in memory and memory is full
                {
                    mem_full++;
                    victim = aging(); //victim is the page frame in physical mem that contains the page to be replaced
                    mem[victim] = current;
                    page_table[current].R = 1; //page is accessed
                    aging_pt_update(current); // update all other pages
                    page_table[current].counter = 128;  //128 means 1 at the leftmost
                }
                else //page not in memory and memory is not full
                {
                    frame = insert(current);
                    aging_pt_update(current);
                }
                //TODO REMOVE POST TESTING
                printf("POST\n");
                printPageTableByMem();
                printf("***************************************************************\n");
                break;

            default: printf("Unknown policy ... Exiting\n");
                exit(1);

        }/* end switch-case */

        print_mem(rp); //print pages in the physical memory in the output file.

    }/* end while */

    fprintf(rp,"number of memory accesses = %f\n", num_accesses);
    fprintf(rp,"number of times memory was full when page fault = %f\n", mem_full);
    fprintf(rp,"number of page faults = %f\n", page_faults);
    fprintf(rp,"percentage of page faults = %f\n", page_faults/num_accesses);
    fprintf(rp,"percentage of time memory was full = %f\n", mem_full/num_accesses);

    /* wrap-up */
    fclose(fp);
    fclose(rp);
    free(mem);

    return 0;

}