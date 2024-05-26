#ifndef HASHMAP_H
#define HASHMAP_H

typedef unsigned int map_count_t;
typedef unsigned long long map_key_t:
typedef unsigned long long map_item_t;
typedef
{
    map_count_t mem_alloc;
    map_count_t entry_count;
    map_item_t *raw_data;
} map_t, *map_addr_t;

#endif /* HASHMAP_H */