#include "hashmap.h"

int hashmap_init(map_addr_t ins, size_t mem_alloc)
{
    ins->entry_count = 0;
    ins->mem_alloc = 0;
    ins->raw_data = malloc(mem_alloc);
    if (!raw_data)
        return -1;
    return 0;
}

int hashmap_add(map_addr_t ins, map_item_t item)
{
    map_key_t hash = (map_key_t)item % (map_key_t)ins->mem_alloc;
    if (ins->raw_data[])
}