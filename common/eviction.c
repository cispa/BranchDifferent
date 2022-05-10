#include "config.h"


#if CACHE == EVICTION 

#include <inttypes.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include <stdio.h>
#include <math.h>

#include "cache.h"
#include "timing.h"
#include "memory.h"

typedef struct elem
{
    struct elem *next;
    struct elem *prev;
    int set;
    size_t delta;
    char pad[32]; // up to 64B
} Elem;

typedef void (*traverser_t)(Elem*);

uint64_t threshold = 0;
Elem** eviction_sets = NULL;
uint64_t* eviction_sets_pages = NULL;
int evset_size = 0;
int evsets_count = 0;
size_t page_size = 0;
size_t evset_memory_size = 0;
traverser_t traverser = NULL;
size_t cache_line_size = 64;

int
gt_eviction(Elem **ptr, Elem **can, char *victim);

// setup global variables and initialize RNG
void evset_init(traverser_t ctraverser, int cevset_size, int cpage_size, int cmemory_size, int cthreshold) {
    srand(time(NULL));
    timer_start();
    
    // setup variables
    
    if(!traverser){
        traverser = ctraverser;
    }
    
    if(!evset_size){
        evset_size = cevset_size;
    }
    
    if(!page_size){
        page_size = cpage_size;
    }
    
    if(!evset_memory_size){
        evset_memory_size = cmemory_size;
    }
 
    if(!threshold){
        threshold = cthreshold;
    }
    
    if(!eviction_sets){
        eviction_sets = calloc(1, sizeof(Elem*));
    }
    
    if(!eviction_sets_pages){
        eviction_sets_pages = calloc(1, sizeof(char*));
    }
    
    timer_stop();
}

// initialize linked list in evset memory
// one entry per page at the correct offset!
static void
initialize_list(char *start, uint64_t size, uint64_t offset)
{
	uint64_t off = 0;
	Elem* last = NULL;
	for (off = offset; off < size - sizeof(Elem); off += page_size)
	{
		Elem* cur = (Elem*)(start + off);
		cur->set = -2;
		cur->delta = 0;
		cur->prev = last;
		cur->next = NULL;
		if(last){
		    last->next = cur;
		}
		last = cur;
	}
}

static Elem*
initialize_list_with_offset(Elem* reference, uint64_t offset){
    // xor mask to turn offset of reference into offset of 
    uint64_t xor = ((uint64_t)reference & (page_size - 1)) ^ offset;
    Elem* last = NULL;
    Elem* start = (Elem*)((uint64_t)reference ^ xor);
    while(reference){
        Elem* cur = (Elem*)((uint64_t)reference ^ xor);
        cur->set = -2;
        cur->delta = 0;
        cur->prev = last;
        cur->next = NULL;
        if(last){
            last->next = cur;
        } 
        last = cur;
        reference = reference->next;
    }
    
    return start;
}


void evset_find(void* addr){

    uint64_t offset = (page_size - 1) & (uint64_t)addr & ~(cache_line_size - 1);

    // check whether eviction set on same page exists
    int same_page = -1;
    uint64_t page = (uint64_t)addr / page_size;
    for(int i = 0; i < evsets_count - 1; i++){
        if(eviction_sets_pages[i] == page){
            same_page = i;
            break;
        }
    }
    
    Elem* start;
    
    if(same_page != -1){
        // eviction set on same page (same_page) found!
        start = initialize_list_with_offset(eviction_sets[same_page], offset);
       
    } else {
        // no eviction set on same page found
    
        // map memory
        char* memory_chunk = (char*) mmap (NULL, evset_memory_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, 0, 0);
    
        if(!memory_chunk){
            printf("MMAP FAILED\n");
        }
    
        // initialize list
        start = (Elem*)(void*)(memory_chunk + offset);
        initialize_list(memory_chunk, evset_memory_size, offset);
    
    
        // reduce list
        Elem* can = NULL;
        int retries = 0;
        while(gt_eviction(&start, &can, addr)){
            printf("finding eviction set failed!\n");
            if(retries > 20){
                printf("max retries exceeded!\n");
                break;
            }
            // unmap memory chunk
            munmap(memory_chunk, evset_memory_size);
            // map new memory chunk
            memory_chunk = (char*) mmap (NULL, evset_memory_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, 0, 0);
            start = (Elem*)(void*)(memory_chunk + offset);
            initialize_list(memory_chunk, evset_memory_size, offset);
            can = NULL;
            retries ++;
        }
        if(retries < 20){
            printf("Eviction set found!\n");
        }
    
        // unmap memory that is no longer needed
        while(can){
    	    Elem* next = can->next;
            if(munmap((void*)((uint64_t)can & (~(page_size-1))), page_size)){
                printf("munmap failed!\n");
            }
            can = next;
        }
 
    }
    // magic!
    eviction_sets[evsets_count - 1] = start;
    eviction_sets_pages[evsets_count - 1] = page;
}

void cache_remove(cache_ctx_t ctx){
    unsigned int evset = (unsigned int)(uint64_t)ctx;
    traverser(eviction_sets[evset]);
}

static
void
traverse_list_skylake(Elem *arg)
{
	Elem *ptr = arg;
	while (ptr && ptr->next && ptr->next->next)
	{
		memory_access (ptr);
		memory_access (ptr->next);
		memory_access (ptr->next->next);
		memory_access (ptr);
		memory_access (ptr->next);
		memory_access (ptr->next->next);
		ptr = ptr->next;
	}
}


uint64_t probe_cache_miss(){
    uint64_t result = 0;
    
    do {

        char* buf = malloc(PAGE_SIZE * 100);
    
        int page = rand() % 99;
    
        // get translation into TLB
        memory_access(buf + page * PAGE_SIZE + (rand() % (PAGE_SIZE / 2)));
    
        result = probe(buf + page * PAGE_SIZE + PAGE_SIZE / 2 + (rand() % PAGE_SIZE / 2));
    
        free(buf);
    } while(result == 0);
    
    return result;
}


uint64_t probe_cache_hit(){
    uint64_t result = 0;
    
    do {

        char* buf = malloc(PAGE_SIZE);
    
        memory_access(buf + 1024);
    
        result = probe(buf + 1024);
    
        free(buf);
    } while(result == 0);
    
    return result;
}

uint64_t find_threshold(){

    uint64_t miss = 0;
    uint64_t hit = 0;
    
    for(int i = 0; i < 1000; i++){
        miss += probe_cache_miss();
    }

    for(int i = 0; i < 1000; i++){
        hit += probe_cache_hit();
    }
    printf("miss: %llu, hit: %llu, threshold: %llu\n", miss / 1000, hit / 1000, (3*miss+hit) / 4000);
    
    return (3*miss+hit) / 4000;
}

cache_ctx_t cache_remove_prepare(char* addr){
    if(cache_line_size < sizeof(Elem)){
        printf("ERROR: Entries are bigger than cache line!\n");
    }
    if(!eviction_sets){
    #ifndef EVICTION_THRESHOLD
    #define EVICTION_THRESHOLD find_threshold()
    #endif /* EVICTION_THRESHOLD */
       evset_init(traverse_list_skylake, EVICTION_SET_SIZE, PAGE_SIZE, EVICTION_MEMORY_SIZE, EVICTION_THRESHOLD);
    }
    
    timer_start();
    
    // check if eviction set for given address already exists
    uint64_t time;
    for(uint64_t i = 0; i < (uint64_t)evsets_count; i++){
        // make sure it is cached
        memory_access(addr);
        memory_access(addr);
        memory_fence();
        // evict
        traverser(eviction_sets[i]);
        traverser(eviction_sets[i]);
        // check if evicted --> return current set id
        if(probe(addr) > threshold){
            return (void*)i;
        }
    }
    
    // if not, make space for one (if required) and find one
    uint64_t id = evsets_count ++;
    if(evsets_count % 8 == 1){
        eviction_sets = realloc(eviction_sets, sizeof(Elem*) * (evsets_count + 8));
        eviction_sets_pages = realloc(eviction_sets_pages, sizeof(uint64_t) * (evsets_count + 8));
    }
    evset_find(addr);
    
    timer_stop();
    return (void*)id;
    
}


#define ROUNDS 50
#define MAX_BACKTRACK 50

extern uint64_t threshold;
extern int evset_size;
extern traverser_t traverser;

int
list_length(Elem *ptr)
{
	int l = 0;
	while (ptr)
	{
		l = l + 1;
		ptr = ptr->next;
	}
	return l;
}

void
list_concat(Elem **ptr, Elem *chunk)
{
	Elem *tmp = (ptr) ? *ptr : NULL;
	if (!tmp)
	{
		*ptr = chunk;
		return;
	}
	while (tmp->next != NULL)
	{
		tmp = tmp->next;
	}
	tmp->next = chunk;
	if (chunk)
	{
		chunk->prev = tmp;
	}
}

void
list_split(Elem *ptr, Elem **chunks, int n)
{
	if (!ptr)
	{
		return;
	}
	int len = list_length (ptr), k = len / n, i = 0, j = 0;
	while (j < n)
	{
		i = 0;
		chunks[j] = ptr;
		if (ptr)
		{
			ptr->prev = NULL;
		}
		while (ptr != NULL && ((++i < k) || (j == n-1)))
		{
			ptr = ptr->next;
		}
		if (ptr)
		{
			ptr = ptr->next;
			if (ptr && ptr->prev) {
				ptr->prev->next = NULL;
			}
		}
		j++;
	}
}

void
list_from_chunks(Elem **ptr, Elem **chunks, int avoid, int len)
{
	int next = (avoid + 1) % len;
	if (!(*ptr) || !chunks || !chunks[next])
	{
		return;
	}
	// Disconnect avoided chunk
	Elem *tmp = chunks[avoid];
	if (tmp) {
		tmp->prev = NULL;
	}
	while (tmp && tmp->next != NULL && tmp->next != chunks[next])
	{
		tmp = tmp->next;
	}
	if (tmp)
	{
		tmp->next = NULL;
	}
	// Link rest starting from next
	tmp = *ptr = chunks[next];
	if (tmp)
	{
		tmp->prev = NULL;
	}
	while (next != avoid && chunks[next] != NULL)
	{
		next = (next + 1) % len;
		while (tmp && tmp->next != NULL && tmp->next != chunks[next])
		{
			if (tmp->next)
			{
				tmp->next->prev = tmp;
			}
			tmp = tmp->next;
		}
		if (tmp)
		{
			tmp->next = chunks[next];
		}
		if (chunks[next])
		{
			chunks[next]->prev = tmp;
		}
	}
	if (tmp)
	{
		tmp->next = NULL;
	}
}

void
shuffle(int *array, size_t n)
{
	size_t i;
	if (n > 1)
	{
		for (i = 0; i < n - 1; i++)
		{
			size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
			int t = array[j];
			array[j] = array[i];
			array[i] = t;
		}
	}
}

int
test_set(Elem *ptr, char *victim)
{
        // page walk
	memory_access (victim);
	memory_access (victim);

	memory_fence();
	traverser(ptr);
        
	return probe(victim);
}

int
tests_avg(Elem *ptr, char *victim, int rep)
{
	int i = 0, ret =0;
	unsigned int delta = 0;
	unsigned int delta_d = 0;
	for (i=0; i < rep; i++)
	{
		delta = test_set (ptr, victim);
		if (delta < 800) {
			delta_d += delta;
		}
	}
	ret = (float)delta_d / rep;
	return ret > threshold;
}

int
gt_eviction(Elem **ptr, Elem **can, char *victim)
{
	// Random chunk selection
	Elem **chunks = (Elem**) calloc (evset_size + 1, sizeof (Elem*));
	if (!chunks)
	{
	        printf("Could not allocate chunks!\n");
		return 1;
	}
	int *ichunks = (int*) calloc (evset_size + 1, sizeof (int)), i;
	if (!ichunks)
	{
		printf("Could not allocate ichunks!\n");
		free (chunks);
		return 1;
	}

	int len = list_length(*ptr);
	int cans = 0;

	// Calculate length: h = log(a/(a+1), a/n)
	double sz = (double)evset_size / len;
	double rate = (double)evset_size / (evset_size + 1);
	int h = ceil(log(sz) / log(rate)), l = 0;

	// Backtrack record
	Elem **back = (Elem**) calloc (h * 2, sizeof (Elem*));
	if (!back)
	{
		printf("Could not allocate back (%d, %f, %f, %d)!\n", h, sz, rate, list_length(*ptr));
		free (chunks);
		free (ichunks);
		return 1;
	}

	int repeat = 0;
	do {

		for (i=0; i < evset_size + 1; i++)
		{
			ichunks[i] = i;
		}
		shuffle (ichunks, evset_size + 1);

		// Reduce
		while (len > evset_size)
		{

			list_split (*ptr, chunks, evset_size + 1);
			int n = 0, ret = 0;

			// Try paths
			do
			{
				list_from_chunks (ptr, chunks, ichunks[n], evset_size + 1);
				n = n + 1;
				ret = tests_avg (*ptr, victim, ROUNDS);
			}
			while (!ret && (n < evset_size + 1));

			// If find smaller eviction set remove chunk
			if (ret && n <= evset_size)
			{
				back[l] = chunks[ichunks[n-1]]; // store ptr to discarded chunk
				cans += list_length (back[l]); // add length of removed chunk
				len = list_length (*ptr);
				l = l + 1; // go to next lvl
			}
			// Else, re-add last removed chunk and try again
			else if (l > 0)
			{
				list_concat (ptr, chunks[ichunks[n-1]]); // recover last case
				l = l - 1;
				cans -= list_length (back[l]);
				list_concat (ptr, back[l]);
				back[l] = NULL;
				len = list_length (*ptr);
				goto mycont;
			}
			else
			{
				list_concat (ptr, chunks[ichunks[n-1]]); // recover last case
				break;
			}
		}

		break;
		mycont:
		;

	} while (l > 0 && repeat++ < MAX_BACKTRACK);

	// recover discarded elements
	for (i = 0; i < h * 2; i++)
	{
		list_concat (can, back[i]);
	}

	free (chunks);
	free (ichunks);
	free (back);

    int ret = 0;
    
    ret = tests_avg (*ptr, victim, ROUNDS);
    
    if (ret)
	{
		if (len > evset_size)
		{
			return 1;
		}
	}
	else
	{
		return 1;
	}

	return 0;
}

#endif /* CACHE */
