#include <bits/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct entry {
    size_t frequency;
};

struct bucket {
    size_t key;
    struct entry data;
    struct bucket *next;
};

struct hashmap {
    size_t capacity;
    struct bucket **buckets;
};

static bool hashmap_init(struct hashmap *map, const size_t capacity)
{
    map->buckets = calloc(capacity, sizeof *map->buckets);
    if (!map->buckets) return false;

    map->capacity = capacity;
    return true;
}

static size_t hashmap_hash(const struct hashmap *map, const size_t key)
{
    return key % map->capacity;
}

static bool hashmap_insert(const struct hashmap *map, const size_t key)
{
    const size_t hash = hashmap_hash(map, key);

    struct bucket *cur = map->buckets[hash];
    while (cur && cur->key != key) cur = cur->next;

    if (cur) {
        ++cur->data.frequency;
        return true;
    }

    struct bucket *node = malloc(sizeof *node);
    if (!node) return false;

    node->key = key;
    node->data.frequency = 1;
    node->next = map->buckets[hash];
    map->buckets[hash] = node;

    return true;
}

static bool hashmap_find(const struct hashmap *map, const size_t key,
                         struct entry *out)
{
    if (!map || !map->buckets || !map->capacity) return false;

    const size_t hash = hashmap_hash(map, key);

    const struct bucket *cur = map->buckets[hash];
    while (cur && cur->key != key) cur = cur->next;

    if (!cur) return false;

    *out = cur->data;
    return true;
}

static void hashmap_free(struct hashmap *map)
{
    for (size_t i = 0; i < map->capacity; ++i) {
        struct bucket *cur = map->buckets[i];
        while (cur) {
            struct bucket *next = cur->next;
            free(cur);
            cur = next;
        }
    }

    free(map->buckets);

    map->buckets = nullptr;
    map->capacity = 0;
}

static constexpr signed long int NS_PER_SEC = 1'000'000'000;
enum {
    RANDOM_SEED = 0xCAFEEE,
    AMOUNT_ELEMENTS = 10'000,
    AMOUNT_SEARCHES = 1U << 20U,
    AMOUNT_HASHMAPS = 2,
};

typedef size_t (*hashmap_fn)(const struct hashmap *map, size_t key);

static size_t op_insert(const struct hashmap *map, const size_t key)
{
    return hashmap_insert(map, key) ? 1 : 0;
}

static size_t op_find(const struct hashmap *map, const size_t key)
{
    struct entry found;
    return hashmap_find(map, key, &found) ? found.frequency : 0;
}

static long long int measure_fn(const struct hashmap *map, const size_t *keys,
                                const size_t count, const hashmap_fn op)
{
    struct timespec start;
    struct timespec end;
    volatile size_t sink = 0;

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (size_t i = 0; i < count; ++i) sink += op(map, keys[i]);
    clock_gettime(CLOCK_MONOTONIC, &end);

    (void)sink;
    return ((end.tv_sec - start.tv_sec) * NS_PER_SEC) +
           (end.tv_nsec - start.tv_nsec);
}

int main(void)
{
    size_t *insert_keys = malloc(AMOUNT_ELEMENTS * sizeof *insert_keys);
    size_t *search_keys = malloc(AMOUNT_SEARCHES * sizeof *search_keys);
    if (!insert_keys || !search_keys) {
        free(insert_keys);
        free(search_keys);
        return EXIT_FAILURE;
    }

    srandom(RANDOM_SEED);
    for (size_t i = 0; i < AMOUNT_ELEMENTS; ++i)
        insert_keys[i] = (size_t)random();
    for (size_t i = 0; i < AMOUNT_SEARCHES; ++i)
        search_keys[i] = (size_t)random();

    for (size_t i = 0; i < AMOUNT_HASHMAPS; ++i) {
        constexpr size_t HASHMAP_CAPACITIES[AMOUNT_HASHMAPS] = {1031, 1024};
        struct hashmap map = {};

        if (!hashmap_init(&map, HASHMAP_CAPACITIES[i])) {
            free(insert_keys);
            free(search_keys);
            return EXIT_FAILURE;
        }

        const long long int insert_ns =
            measure_fn(&map, insert_keys, AMOUNT_ELEMENTS, op_insert);
        const long long int find_ns =
            measure_fn(&map, search_keys, AMOUNT_SEARCHES, op_find);

        printf("Hash (capacity=%zu) -> inserir: %lld ns | buscar: %lld ns\n",
               map.capacity, insert_ns, find_ns);

        hashmap_free(&map);
    }

    free(insert_keys);
    free(search_keys);
    return EXIT_SUCCESS;
}
