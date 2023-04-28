/**
 * Buddy Allocator
 *
 * For the list library usage, see http://www.mcs.anl.gov/~kazutomo/list/
 */

/**************************************************************************
 * Conditional Compilation Options
 **************************************************************************/
#define USE_DEBUG 0

/**************************************************************************
 * Included Files
 **************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "buddy.h"
#include "list.h"
#include "stdbool.h"
#include "libpriqueue.h"

/**************************************************************************
 * Public Definitions
 **************************************************************************/
#define MIN_ORDER 12
#define MAX_ORDER 20

#define PAGE_SIZE (1<<MIN_ORDER)
/* page index to address */
#define PAGE_TO_ADDR(page_idx) (void *)((page_idx*PAGE_SIZE) + g_memory)

/* address to page index */
#define ADDR_TO_PAGE(addr) ((unsigned long)((void *)addr - (void *)g_memory) / PAGE_SIZE)

/* find buddy address */
#define BUDDY_ADDR(addr, o) (void *)((((unsigned long)addr - (unsigned long)g_memory) ^ (1<<o)) \
									 + (unsigned long)g_memory)

#if USE_DEBUG == 1
#  define PDEBUG(fmt, ...) \
	fprintf(stderr, "%s(), %s:%d: " fmt,			\
		__func__, __FILE__, __LINE__, ##__VA_ARGS__)
#  define IFDEBUG(x) x
#else
#  define PDEBUG(fmt, ...)
#  define IFDEBUG(x)
#endif

/**************************************************************************
 * Public Types
 **************************************************************************/
typedef struct {
	struct list_head list;
	/* TODO: DECLARE NECESSARY MEMBER VARIABLES */
	void* blockAddress;
	size_t blockSize;
	int pageIndex;
	bool inUse;
} page_t;

/**************************************************************************
 * Global Variables
 **************************************************************************/
/* free lists*/
//struct list_head free_area[MAX_ORDER+1]; //keeps track of the pages that are free, basically a node
priqueue_t free_area[MAX_ORDER+1];

/* memory area */
char g_memory[1<<MAX_ORDER]; //simulated memory space

/* page structures */
page_t g_pages[(1<<MAX_ORDER)/PAGE_SIZE]; //add/remove page structures to/from this as neccassary due to allocs/frees

/**************************************************************************
 * Public Function Prototypes
 **************************************************************************/
int comparer(const void * a, const void *b)
{
	page_t * page1 = (page_t *) a;
	page_t * page2 = (page_t *) b;
	return page1->pageIndex - page2->pageIndex;
}
/**************************************************************************
 * Local Functions
 **************************************************************************/

/**
 * Initialize the buddy system
 */
void buddy_init()
{
	int i;
	int n_pages = (1<<MAX_ORDER) / PAGE_SIZE;
	for (i = 0; i < n_pages; i++) {
		/* TODO: INITIALIZE PAGE STRUCTURES */
		INIT_LIST_HEAD(&(g_pages[i].list));
		g_pages[i].blockSize = MAX_ORDER; //The current size block the page is a part of
		g_pages[i].pageIndex = i;
		g_pages[i].blockAddress = PAGE_TO_ADDR(i);
	}

	/* initialize freelist */
	for (i = MIN_ORDER; i <= MAX_ORDER; i++) {
		//INIT_LIST_HEAD(&free_area[i]);
		priqueue_init(&free_area[i], comparer);
	}

	/* add the entire memory as a freeblock */
	//list_add(&g_pages[0].list, &free_area[MAX_ORDER]);
	priqueue_offer(&free_area[MAX_ORDER], (void*) &g_pages[0]);
}

/**
 * Allocate a memory block.
 *
 * On a memory request, the allocator returns the head of a free-list of the
 * matching size (i.e., smallest block that satisfies the request). If the
 * free-list of the matching block size is empty, then a larger block size will
 * be selected. The selected (large) block is then splitted into two smaller
 * blocks. Among the two blocks, left block will be used for allocation or be
 * further splitted while the right block will be added to the appropriate
 * free-list.
 *
 * @param size size in bytes
 * @return memory block address
 */
void *buddy_alloc(int size)
{
	/* TODO: IMPLEMENT THIS FUNCTION */
	/*
		int size is the size of the allocation that we want to perform
		General idea
		* Find minimum order that satisfies page size
		* Create more pages in free area (if necessary)
		* Select and return free page
		* If you need to create two pages and need to return one, chose the left one (its the standard)
	*/
	//STEP ONE
	//Finding the minimum order that satisfies page size;
	int required_page_size = 0;
	for (int i = MIN_ORDER; i <= MAX_ORDER; i++)
	{
		// See if the current order is big enough.
		if ((1 << i) >= size)
		{
			required_page_size = i;
			break;
		}
	}

	//STEP TWO
	//find the smallest available page size
	int available_page_size = required_page_size;
	while (priqueue_size(&free_area[available_page_size]) == 0) //Loop until you find an available page large enough to fit the process
	{
		++available_page_size;
	}
	if(available_page_size > MAX_ORDER)
	{
		return NULL;
	}

	if (available_page_size > MAX_ORDER)
		return NULL;

	//STEP THREE 
	//If neccessary, make new pages until one of the required size is available
	for (int i = available_page_size; i > required_page_size; --i)
	{
		//3.1
		//Remove 1st availabe page of curr size form free area
		page_t *curr_page = (page_t*) priqueue_remove_at(&free_area[i], 0);
		int this_index = curr_page->pageIndex;

		//3.2
		//Get the buddy's information
		void* buddy_address = BUDDY_ADDR(g_pages[this_index].blockAddress, (i-1));
		int buddy_index = ADDR_TO_PAGE(buddy_address);

		//3.3
		//CHange the block size at each index & indexes in between 
		int index_diff;
		if (buddy_index > this_index)
			index_diff = buddy_index - this_index;
		else
			index_diff = this_index - buddy_index;

		for (int j = 0; j < index_diff; ++j)
		{
			g_pages[buddy_index + j].blockSize = i-1;
			g_pages[this_index + j].blockSize = i-1;
		}

		//Make two new pages of size i-1
		priqueue_offer(&free_area[i-1], &g_pages[this_index]);
		priqueue_offer(&free_area[i-1], &g_pages[buddy_index]);
	}

	//STEP FOUR
	//Get the next available page of teh neccessary size from the free area
	page_t *curr_page = (page_t *) priqueue_remove_at(&free_area[required_page_size], 0);
	int this_index = curr_page->pageIndex;

	//STEP FIVE
	//Return the address of teh block
	return PAGE_TO_ADDR(this_index);

}

/**
 * Free an allocated memory block.
 *
 * Whenever a block is freed, the allocator checks its buddy. If the buddy is
 * free as well, then the two buddies are combined to form a bigger block. This
 * process continues until one of the buddies is not free.
 *
 * @param addr memory block address to be freed
 */
void buddy_free(void *addr)
{
	/* TODO: IMPLEMENT THIS FUNCTION */
	/*
	General Idea
	* check budy pages (continuously)
	* While buddy is free, combine
	* Keep checking to see how far up the tree you can go. (Don't just stop at the freed pair)
	*/
	
	//STEP ONE
	//Find the index of the currently freed address
	int freed_index = ADDR_TO_PAGE(addr);

	//STEP TWO (recursion base case)
	//If the size is already at max, do nothing!!!
	if (g_pages[freed_index].blockSize == MAX_ORDER)
	{
		int count = 0;
		for (int i = MIN_ORDER; i <= MAX_ORDER; ++i)
			count = count + priqueue_size(&free_area[i]);
		if (count == 0)
			priqueue_offer(&free_area[MAX_ORDER], &g_pages[freed_index]);
		return;
	}

	//STEP THREE
	//Find the buddy address to the current size
	int curr_size = g_pages[freed_index].blockSize;
	void* buddy_address = BUDDY_ADDR(addr, curr_size);
	int buddy_index = ADDR_TO_PAGE(buddy_address);

	//STEP FOUR
	//See if the buddy is in the free list and remove it if it is
	bool buddy_freed = false;
	for (int i = 0; i < priqueue_size(&free_area[curr_size]); ++i)
	{
		page_t * curr_page = priqueue_at(&free_area[curr_size], i);
		if(curr_page->pageIndex == buddy_index)
		{
			priqueue_remove_at(&free_area[curr_size], i);
			buddy_freed = true;
			break;
		}
	}

	//STEP FIVE
	//IF buddy was freed, change blcok info for the freed_index and buddy index
	// && call buddy_free recursively
	if(buddy_freed)
	{
		//change block info
		int index_diff;
		if (buddy_index > freed_index)
			index_diff = buddy_index - freed_index;
		else
			index_diff = freed_index - buddy_index;

		for (int i = 0; i < index_diff; ++i)
		{
			g_pages[buddy_index + i].blockSize = curr_size + 1;
			g_pages[freed_index + i].blockSize = curr_size + 1;
		}
		if(buddy_index > freed_index)
		{
			//priqueue_offer(&free_area[curr_size+1], &g_pages[freed_index]);
			buddy_free(PAGE_TO_ADDR(freed_index));
		}
		else
		{
			//priqueue_offer(&free_area[curr_size+1], &g_pages[buddy_index]);
			buddy_free(PAGE_TO_ADDR(buddy_index));
		}
	}
	else
	{
		priqueue_offer(&free_area[curr_size], &g_pages[freed_index]);
	}
}

/**
 * Print the buddy system status---order oriented
 *
 * print free pages in each order.
 */
void buddy_dump()
{
	int o;
	for (o = MIN_ORDER; o <= MAX_ORDER; o++) {
		//struct list_head *pos;
		int cnt = 0;
		//list_for_each(pos, &free_area[o]) {
		//	cnt++;
		//}
		cnt = priqueue_size(&free_area[o]);
		printf("%d:%dK ", cnt, (1<<o)/1024);
	}
	printf("\n");
}
