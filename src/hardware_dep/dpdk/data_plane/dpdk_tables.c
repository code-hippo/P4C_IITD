// Copyright 2016 Eotvos Lorand University, Budapest, Hungary
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "backend.h"
#include "dataplane.h"
#include "dpdk_tables.h"
#include <rte_cycles.h>
// ============================================================================
// LOOKUP TABLE IMPLEMENTATIONS
#include <rte_prefetch.h>
#include <rte_hash.h>       // EXACT
#include <rte_hash_crc.h>
#include <nmmintrin.h> 
#include <rte_lpm.h>        // LPM (32 bit key)
#include <rte_lpm6.h>       // LPM (128 bit key)
#include "ternary_naive.h"  // TERNARY

#include <rte_malloc.h>     // extended tables
#include <rte_version.h>    // for conditional on rte_hash parameters
#include <rte_errno.h>

static uint32_t crc32(const void *data, uint32_t data_len, uint32_t init_val) {
    int32_t *data32 = (void*)data;
    uint32_t result = init_val; 
    result = _mm_crc32_u32 (result, *data32++);
    return result;
}

static uint8_t*
copy_to_socket(uint8_t* src, int length, int socketid) {
    uint8_t* dst = rte_malloc_socket("uint8_t", sizeof(uint8_t)*length, 0, socketid);
    memcpy(dst, src, length);
    return dst;
}

// ============================================================================
// LOWER LEVEL TABLE MANAGEMENT

void create_error_text(int socketid, char* table_type, char* error_text)
{
    rte_exit(EXIT_FAILURE, "DPDK: Unable to create the %s on socket %d: %s\n", table_type, socketid, error_text);
}

void create_error(int socketid, char* table_type)
{
    if (rte_errno == ENOENT) {
        create_error_text(socketid, table_type, "missing entry");
    }
    if (rte_errno == EINVAL) {
        create_error_text(socketid, table_type, "invalid parameter passed to function");
    }
    if (rte_errno == ENOSPC) {
        create_error_text(socketid, table_type, "the maximum number of memzones has already been allocated");
    }
    if (rte_errno == EEXIST) {
        create_error_text(socketid, table_type, "a memzone with the same name already exists");
    }
    if (rte_errno == ENOMEM) {
        create_error_text(socketid, table_type, "no appropriate memory area found in which to create memzone");
    }
}

struct rte_hash *
hash_create(int socketid, const char* name, uint32_t keylen, rte_hash_function hashfunc)
{
    struct rte_hash_parameters hash_params = {
        .name = NULL,
        .entries = HASH_ENTRIES,
#if RTE_VER_MAJOR == 2 && RTE_VER_MINOR == 0
        .bucket_entries = 4,
#endif
        .key_len = keylen,
        .hash_func = hashfunc,
        .hash_func_init_val = 0,
    };
    hash_params.name = name;
    hash_params.socket_id = socketid;
    struct rte_hash *h = rte_hash_create(&hash_params);
    if (h == NULL)
        create_error(socketid, "hash");
    return h;
}

struct rte_lpm *
lpm4_create(int socketid, const char* name, uint8_t max_size)
{
#if RTE_VERSION >= RTE_VERSION_NUM(16,04,0,0)
    struct rte_lpm_config config = {
        .max_rules = max_size,
        .number_tbl8s = (1 << 8), // TODO refine this
        .flags = 0
    };
    struct rte_lpm *l = rte_lpm_create(name, socketid, &config);
#else
    struct rte_lpm *l = rte_lpm_create(name, socketid, max_size, 0/*flags*/);
#endif
    if (l == NULL)
        create_error(socketid, "LPM");
    return l;
}

struct rte_lpm6 *
lpm6_create(int socketid, const char* name, uint8_t max_size)
{
    struct rte_lpm6_config config = {
        .max_rules = max_size,
        .number_tbl8s = (1 << 16),
        .flags = 0
    };
    struct rte_lpm6 *l = rte_lpm6_create(name, socketid, &config);
    if (l == NULL)
        create_error(socketid, "LPM6");
    return l;
}

int32_t
hash_add_key(struct rte_hash* h, void *key)
{
    int32_t ret;
    ret = rte_hash_add_key(h,(void *) key);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Unable to add entry to the hash.\n");
    return ret;
}

void
lpm4_add(struct rte_lpm* l, uint32_t key, uint8_t depth, uint8_t value)
{
    int ret = rte_lpm_add(l, key, depth, value);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Unable to add entry to the LPM table\n");
    //debug("LPM: Added 0x%08x / %d (%d)\n", (unsigned)key, depth, value);
}

void
lpm6_add(struct rte_lpm6* l, uint8_t key[16], uint8_t depth, uint8_t value)
{
    int ret = rte_lpm6_add(l, key, depth, value);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Unable to add entry to the LPM table\n");
    debug("LPM: Adding route %s / %d (%d)\n", "IPV6", depth, value);
}

// ============================================================================
// HIGHER LEVEL TABLE MANAGEMENT

// ----------------------------------------------------------------------------
// CREATE

static void
create_ext_table(lookup_table_t* t, void* rte_table, int socketid)
{
    extended_table_t* ext = rte_malloc_socket("extended_table_t", sizeof(extended_table_t), 0, socketid);
    ext->rte_table = rte_table;
    ext->size = 0;
    ext->content = rte_malloc_socket("uint8_t*", sizeof(uint8_t*)*TABLE_MAX, 0, socketid);
    t->table = ext;
}

void
exact_create(lookup_table_t* t, int socketid)
{
    char name[64];
    snprintf(name, sizeof(name), "%s_exact_%d_%d", t->name, socketid, t->instance);
    struct rte_hash* h = hash_create(socketid, name, t->key_size, rte_hash_crc);
    create_ext_table(t, h, socketid);
}

void
lpm_create(lookup_table_t* t, int socketid)
{
    char name[64];
    snprintf(name, sizeof(name), "%s_lpm_%d_%d", t->name, socketid, t->instance);
    if(t->key_size <= 4)
        create_ext_table(t, lpm4_create(socketid, name, t->max_size), socketid);
    else if(t->key_size <= 16)
        create_ext_table(t, lpm6_create(socketid, name, t->max_size), socketid);
    else
        rte_exit(EXIT_FAILURE, "LPM: key_size not supported\n");

}

void
ternary_create(lookup_table_t* t, int socketid)
{
    t->table = naive_ternary_create(t->key_size, t->max_size);
}

// ----------------------------------------------------------------------------
// SET DEFAULT VALUE

void
table_setdefault(lookup_table_t* t, uint8_t* value)
{
    debug("Default value set for table %s (on socket %d).\n", t->name, t->socketid);
    t->default_val = copy_to_socket(value, t->val_size, t->socketid);
}

// ----------------------------------------------------------------------------
// ADD

static uint8_t* add_index(uint8_t* value, int val_size, int index)
{
    // realloc doesn't work in this case ("invalid old size")
    uint8_t* value2 = malloc(val_size+sizeof(int));
    memcpy(value2, value, val_size);
    *(value+val_size) = index;
    return value2;
}

void
exact_add(lookup_table_t* t, uint8_t* key, uint8_t* value)
{
    if(t->key_size == 0) return; // don't add lines to keyless tables
    extended_table_t* ext = (extended_table_t*)t->table;
    uint32_t index = rte_hash_add_key(ext->rte_table, (void*) key);
    if(index < 0)
        rte_exit(EXIT_FAILURE, "HASH: add failed\n");
    value = add_index(value, t->val_size, t->counter++);
    ext->content[index%256] = copy_to_socket(value, t->val_size, t->socketid);
}

void
lpm_add(lookup_table_t* t, uint8_t* key, uint8_t depth, uint8_t* value)
{
    if(t->key_size == 0) return; // don't add lines to keyless tables
    extended_table_t* ext = (extended_table_t*)t->table;
    value = add_index(value, t->val_size, t->counter++);
    if(t->key_size <= 4)
    {
        ext->content[ext->size] = copy_to_socket(value, t->val_size, t->socketid);

        // the rest is zeroed in case of keys smaller then 4 bytes
        uint32_t key32 = 0;
        memcpy(&key32, key, t->key_size);

        lpm4_add(ext->rte_table, key32, depth, ext->size++);
    }
    else if(t->key_size <= 16)
    {
        ext->content[ext->size] = copy_to_socket(value, t->val_size, t->socketid);

        static uint8_t key128[16];
        memset(key128, 0, 16);
        memcpy(key128, key, t->key_size);

        lpm6_add(ext->rte_table, key128, depth, ext->size++);
    }
}

void
ternary_add(lookup_table_t* t, uint8_t* key, uint8_t* mask, uint8_t* value)
{
    if(t->key_size == 0) return; // don't add lines to keyless tables
    value = add_index(value, t->val_size, t->counter++);
    naive_ternary_add(t->table, key, mask, copy_to_socket(value, t->val_size, t->socketid));
}

// ----------------------------------------------------------------------------
// LOOKUP
/*
#define NULL_SIGNATURE                  0

#define RTE_HASH_BUCKET_ENTRIES         4

static inline hash_sig_t
rte_hash_secondary_hash(const hash_sig_t primary_hash)
{
        static const unsigned all_bits_shift = 12;
        static const unsigned alt_bits_xor = 0x5bd1e995;

        uint32_t tag = primary_hash >> all_bits_shift;

        return primary_hash ^ ((tag + 1) * alt_bits_xor);
}
*/
/*
int32_t
rte_hash_lookup2(const struct rte_hash *h, const void *key)
{
        RETURN_IF_TRUE(((h == NULL) || (key == NULL)), -EINVAL);
        return __rte_hash_lookup_with_hash2(h, key, rte_hash_hash(h, key), NULL);
}

static inline int32_t
__rte_hash_lookup_with_hash2(const struct rte_hash *h, const void *key,
                                        hash_sig_t sig, void **data)
{
        uint32_t bucket_idx;
        hash_sig_t alt_hash;
        unsigned i;
        struct rte_hash_bucket *bkt;
        struct rte_hash_key *k, *keys = h->key_store;

        bucket_idx = sig & h->bucket_bitmask;
        bkt = &h->buckets[bucket_idx];
*/
        /* Check if key is in primary location */
  /*      for (i = 0; i < RTE_HASH_BUCKET_ENTRIES; i++) {
                if (bkt->signatures[i].current == sig &&
                                bkt->signatures[i].sig != NULL_SIGNATURE) {
                        k = (struct rte_hash_key *) ((char *)keys +
                                        bkt->key_idx[i] * h->key_entry_size);
                        if (rte_hash_cmp_eq(key, k->key, h) == 0) {
                                if (data != NULL)
                                        *data = k->pdata;
    */                            /*
                                 * Return index where key is stored,
                                 * substracting the first dummy index
                                 */
    /*                            return bkt->key_idx[i] - 1;
                        }
                }
        }

      */  /* Calculate secondary hash */
      /*  alt_hash = rte_hash_secondary_hash(sig);
        bucket_idx = alt_hash & h->bucket_bitmask;
        bkt = &h->buckets[bucket_idx];
*/
        /* Check if key is in secondary location */
  /*      for (i = 0; i < RTE_HASH_BUCKET_ENTRIES; i++) {
                if (bkt->signatures[i].current == alt_hash &&
                                bkt->signatures[i].alt == sig) {
                        k = (struct rte_hash_key *) ((char *)keys +
                                        bkt->key_idx[i] * h->key_entry_size);
                        if (rte_hash_cmp_eq(key, k->key, h) == 0) {
                                if (data != NULL)
                                        *data = k->pdata;
    */                            /*
                                 * Return index where key is stored,
                                 * substracting the first dummy index
                                 */
      /*                          return bkt->key_idx[i] - 1;
                        }
                }
        }

        return -ENOENT;
}

rte_prefetch0(rte_pktmbuf_mtod(pkts_burst[j+1], void *));
*/

/** A hash table structure. */

#define RTE_HASH_BUCKET_ENTRIES         4
/*
static inline void rte_prefetch0(const volatile void *p)
{
        asm volatile ("PRFM PLDL1KEEP, [%0]" : : "r" (p));
}
*/

struct rte_hash_signatures {
        union {
                struct {
                        hash_sig_t current;
                        hash_sig_t alt;
                };
                uint64_t sig;
        };
};



struct rte_hash_bucket {
        struct rte_hash_signatures signatures[RTE_HASH_BUCKET_ENTRIES];
        /* Includes dummy key index that always contains index 0 */
        uint32_t key_idx[RTE_HASH_BUCKET_ENTRIES + 1];
        uint8_t flag[RTE_HASH_BUCKET_ENTRIES];
} __rte_cache_aligned;


typedef struct {
        volatile int locked; /**< lock status 0 = unlocked, 1 = locked */
} rte_spinlock_t;


enum cmp_jump_table_case {
        KEY_CUSTOM = 0,
        KEY_16_BYTES,
        KEY_32_BYTES,
        KEY_48_BYTES,
        KEY_64_BYTES,
        KEY_80_BYTES,
        KEY_96_BYTES,
        KEY_112_BYTES,
        KEY_128_BYTES,
        KEY_OTHER_BYTES,
        NUM_KEY_CMP_CASES,
};

enum add_key_case {
        ADD_KEY_SINGLEWRITER = 0,
        ADD_KEY_MULTIWRITER,
        ADD_KEY_MULTIWRITER_TM,
};


struct rte_hash {
        char name[RTE_HASH_NAMESIZE];   /**< Name of the hash. */
        uint32_t entries;               /**< Total table entries. */
        uint32_t num_buckets;           /**< Number of buckets in table. */
        uint32_t key_len;               /**< Length of hash key. */
        rte_hash_function hash_func;    /**< Function used to calculate hash. */
        uint32_t hash_func_init_val;    /**< Init value used by hash_func. */
        rte_hash_cmp_eq_t rte_hash_custom_cmp_eq;
        /**< Custom function used to compare keys. */
        enum cmp_jump_table_case cmp_jump_table_idx;
        /**< Indicates which compare function to use. */
        uint32_t bucket_bitmask;        /**< Bitmask for getting bucket index
                                                from hash signature. */
        uint32_t key_entry_size;         /**< Size of each key entry. */

        struct rte_ring *free_slots;    /**< Ring that stores all indexes
                                                of the free slots in the key table */
        void *key_store;                /**< Table storing all keys and data */
        struct rte_hash_bucket *buckets;        /**< Table with buckets storing all the
                                                        hash values and key indexes
                                                        to the key table*/
        uint8_t hw_trans_mem_support;   /**< Hardware transactional
                                                        memory support */
        struct lcore_cache *local_free_slots;
        /**< Local cache per lcore, storing some indexes of the free slots */
        enum add_key_case add_key; /**< Multi-writer hash add behavior */

        rte_spinlock_t *multiwriter_lock; /**< Multi-writer spinlock for w/o TM */
} __rte_cache_aligned;

#define false 0
#define true 1
typedef int bool; // or #define bool int

//int batch_size=32;

#define NULL_SIGNATURE                  0

#define KEY_ALIGNMENT                   16


struct rte_hash_key {
        union {
                uintptr_t idata;
                void *pdata;
        };
        char key[0];
} __attribute__((aligned(KEY_ALIGNMENT)));

static int
rte_hash_k16_cmp_eq(const void *key1, const void *key2, size_t key_len __rte_unused)
{
        const __m128i k1 = _mm_loadu_si128((const __m128i *) key1);
        const __m128i k2 = _mm_loadu_si128((const __m128i *) key2);
#ifdef RTE_MACHINE_CPUFLAG_SSE4_1
        const __m128i x = _mm_xor_si128(k1, k2);

        return !_mm_test_all_zeros(x, x);
#else
        const __m128i x = _mm_cmpeq_epi32(k1, k2);

        return _mm_movemask_epi8(x) != 0xffff;
#endif
}

static int
rte_hash_k32_cmp_eq(const void *key1, const void *key2, size_t key_len)
{
        return rte_hash_k16_cmp_eq(key1, key2, key_len) ||
                rte_hash_k16_cmp_eq((const char *) key1 + 16,
                                (const char *) key2 + 16, key_len);
}

static int
rte_hash_k48_cmp_eq(const void *key1, const void *key2, size_t key_len)
{
        return rte_hash_k16_cmp_eq(key1, key2, key_len) ||
                rte_hash_k16_cmp_eq((const char *) key1 + 16,
                                (const char *) key2 + 16, key_len) ||
                rte_hash_k16_cmp_eq((const char *) key1 + 32,
                                (const char *) key2 + 32, key_len);
}

static int
rte_hash_k64_cmp_eq(const void *key1, const void *key2, size_t key_len)
{
        return rte_hash_k32_cmp_eq(key1, key2, key_len) ||
                rte_hash_k32_cmp_eq((const char *) key1 + 32,
                                (const char *) key2 + 32, key_len);
}

static int
rte_hash_k80_cmp_eq(const void *key1, const void *key2, size_t key_len)
{
        return rte_hash_k64_cmp_eq(key1, key2, key_len) ||
                rte_hash_k16_cmp_eq((const char *) key1 + 64,
                                (const char *) key2 + 64, key_len);
}

static int
rte_hash_k96_cmp_eq(const void *key1, const void *key2, size_t key_len)
{
        return rte_hash_k64_cmp_eq(key1, key2, key_len) ||
                rte_hash_k32_cmp_eq((const char *) key1 + 64,
                                (const char *) key2 + 64, key_len);
}

static int
rte_hash_k112_cmp_eq(const void *key1, const void *key2, size_t key_len)
{
        return rte_hash_k64_cmp_eq(key1, key2, key_len) ||
                rte_hash_k32_cmp_eq((const char *) key1 + 64,
                                (const char *) key2 + 64, key_len) ||
                rte_hash_k16_cmp_eq((const char *) key1 + 96,
                                (const char *) key2 + 96, key_len);
}

static int
rte_hash_k128_cmp_eq(const void *key1, const void *key2, size_t key_len)
{
        return rte_hash_k64_cmp_eq(key1, key2, key_len) ||
                rte_hash_k64_cmp_eq((const char *) key1 + 64,
                                (const char *) key2 + 64, key_len);
}

const rte_hash_cmp_eq_t cmp_jump_table2[NUM_KEY_CMP_CASES] = {
        NULL,
        rte_hash_k16_cmp_eq,
        rte_hash_k32_cmp_eq,
        rte_hash_k48_cmp_eq,
        rte_hash_k64_cmp_eq,
        rte_hash_k80_cmp_eq,
        rte_hash_k96_cmp_eq,
        rte_hash_k112_cmp_eq,
        rte_hash_k128_cmp_eq,
        memcmp
};

static inline hash_sig_t
rte_hash_secondary_hash(const hash_sig_t primary_hash)
{
        static const unsigned all_bits_shift = 12;
        static const unsigned alt_bits_xor = 0x5bd1e995;

        uint32_t tag = primary_hash >> all_bits_shift;

        return primary_hash ^ ((tag + 1) * alt_bits_xor);
}


static inline int
rte_hash_cmp_eq(const void *key1, const void *key2, const struct rte_hash *h)
{
        if (h->cmp_jump_table_idx == KEY_CUSTOM)
                return h->rte_hash_custom_cmp_eq(key1, key2, h->key_len);
        else
                return cmp_jump_table2[h->cmp_jump_table_idx](key1, key2, h->key_len);
}


uint8_t*
exact_lookup(lookup_table_t* t, uint8_t* key)
{
    if(t->key_size == 0) return t->default_val;
    extended_table_t* ext = (extended_table_t*)t->table;
    int ret = rte_hash_lookup(ext->rte_table, key);
    debug(" status : %d\n", ret);
    return (ret < 0)? t->default_val : ext->content[ret % 256];
}

// To check the deault (or dpdk hash_lookup) exact_lookups in batch mode
void
_exact_lookups(lookup_table_t* t, int  batch_size, uint8_t* key_[][6], uint8_t **I_)
{
    for( int i=0;i< batch_size ; i++){
    if(t->key_size == 0) return t->default_val;
    extended_table_t* ext = (extended_table_t*)t->table;
    int ret = rte_hash_lookup(ext->rte_table, (uint8_t *)key_[i]);
    debug(" status : %d\n", ret);
    I_[i] =  (ret < 0)? t->default_val : ext->content[ret % 256];
    }
       
}

uint64_t prev_tsc, diff_tsc, curr_tsc; // part of timing profiles      


void
exact_lookups(lookup_table_t* t, int batch_size, uint8_t* key_[][6], uint8_t **I_)
{
    extended_table_t* ext = (extended_table_t*)t->table;
    const struct rte_hash *h = ext->rte_table;

    int I[batch_size];
    struct rte_hash_bucket *bkt[batch_size];

    uint32_t bucket_idx;
    
    void *key[batch_size];
    /* Storage of primary hash */
    /* Storage of secondary hash */
    hash_sig_t sig[batch_size];
    hash_sig_t alt_hash[batch_size];
    //debug(" control entered exact_lookups \n");
    bool found[batch_size];
    for(int temp=0; temp<batch_size; temp++){  
//	prev_tsc = rte_rdtsc();
	found[temp] = false; 
	if(t->key_size == 0){
	    I[temp] = -ENOENT;
	    found[temp] =true;
            continue;
	 }
        
        key[temp] = (void *)(uint8_t *)key_[temp];
	/* Calculate primary hash */
	sig[temp] = rte_hash_hash(h, key[temp]);
        bucket_idx = sig[temp] & h->bucket_bitmask;

//	curr_tsc = rte_rdtsc();
//	debug("loop1 exact_lookups cycles : %llu\n", curr_tsc - prev_tsc);
        
	bkt[temp] = &(h->buckets[bucket_idx]);
	rte_prefetch0(bkt[temp]);
    }

    unsigned i; 
    struct rte_hash_key *k, *keys = h->key_store;
     
//fpp_label1 :
        
//  debug(" control loop1(primary index lookup) in exact_lookups \n");
    for(int temp =0; temp < batch_size; temp++){
        /* Check if key is in primary location */

//	prev_tsc = rte_rdtsc();  	
	if(found[temp] ==true){
		continue;
	}
       
        for (i = 0; i < RTE_HASH_BUCKET_ENTRIES; i++) {
            if (bkt[temp]->signatures[i].current == sig[temp] &&
            	    bkt[temp]->signatures[i].sig != NULL_SIGNATURE) {
                k = (struct rte_hash_key *) ((char *)keys +
                     	bkt[temp]->key_idx[i] * h->key_entry_size);
                if (rte_hash_cmp_eq(key[temp], k->key, h) == 0) {
                        /*
                        * Return index where key is stored,
                        * substracting the first dummy index
                        */
                    I[temp] =  bkt[temp]->key_idx[i] - 1;
		    found[temp] = true;
		    break;
                }
            }
        }

	if(found[temp]){
//		curr_tsc = rte_rdtsc();
//		debug("loop2_found exact_lookups cycles : %llu\n", curr_tsc - prev_tsc);
	    continue;
	}

        /* Calculate secondary hash */
        alt_hash[temp] = rte_hash_secondary_hash(sig[temp]);
        bucket_idx = alt_hash[temp] & h->bucket_bitmask;

//	curr_tsc = rte_rdtsc();

//	debug("loop2_found exact_lookups cycles : %llu\n", curr_tsc - prev_tsc);
	bkt[temp] = &h->buckets[bucket_idx];
	rte_prefetch0(bkt[temp]);
    }   

//fpp_label2 :  
          
//  debug("control in loop2(secondary index lookup) of exact_lookups \n");
    for( int temp =0; temp < batch_size; temp++){
    	/* Check if key is in secondary location */
//      prev_tsc = rte_rdtsc();
        if(found[temp]) {
	    continue;
	}
        sig[temp] = rte_hash_hash(h, key[temp]);


        for (i = 0; i < RTE_HASH_BUCKET_ENTRIES; i++) {
            if (bkt[temp]->signatures[i].current == alt_hash[temp] &&
                            bkt[temp]->signatures[i].alt == sig[temp]) {
                k = (struct rte_hash_key *) ((char *)keys +
                                bkt[temp]->key_idx[i] * h->key_entry_size);
                if (rte_hash_cmp_eq(key[temp], k->key, h) == 0) {
//              	if (data != NULL)
//	                    *data = k->pdata;
                        /*
                        * Return index where key is stored,
                        * substracting the first dummy index
                        */
                    I[temp] =  bkt[temp]->key_idx[i] - 1;
		    found[temp] = true;
		    break;
                }
            }   
        }
	if(found[temp]) {
//	    curr_tsc = rte_rdtsc();
//	    debug("loop3_found exact_lookups cycles : %llu\n", curr_tsc - prev_tsc);
	    continue;
	}
	I[temp] = -ENOENT;
	
//	curr_tsc = rte_rdtsc();
//	debug("loop3 exact_lookups cycles : %llu\n", curr_tsc - prev_tsc);
    }

//fpp_end :
        //debug("end_loop in exact_lookups \n");
	for(int i=0;i<batch_size;i++){
//          debug("size : %d",sizeof(t->default_val));
//	    debug("the lookup success status %d\n", I[i]);  
	
//	    prev_tsc = rte_rdtsc();   	
	    I_[i] =  (I[i] < 0)? t->default_val : ext->content[I[i] % 256];

//	    curr_tsc = rte_rdtsc();
//	    debug("loop1 exact_lookups cycles : %llu", curr_tsc - prev_tsc);
//	    debug(" print actins : %p \n",(uint8_t *) I_[i]);
	};
//      debug(" end of exact_lookups \n");
}



uint8_t*
lpm_lookup(lookup_table_t* t, uint8_t* key)
{
    if (t->key_size == 0) return t->default_val;
    extended_table_t* ext = (extended_table_t*)t->table;

    if(t->key_size <= 4)
    {
        uint32_t key32 = 0;
        memcpy(&key32, key, t->key_size);

        uint8_t result;
	int ret = 0;
        //int ret = rte_lpm_lookup(ext->rte_table, key32, &result);
        return ret == 0 ? ext->content[result] : t->default_val;
    }
    else if(t->key_size <= 16)
    {
        static uint8_t key128[16];
        memset(key128, 0, 16);
        memcpy(key128, key, t->key_size);

        uint8_t result;
        int ret = rte_lpm6_lookup(ext->rte_table, key128, &result);
        return ret == 0 ? ext->content[result] : t->default_val;
    }
    return NULL;
}

uint8_t*
ternary_lookup(lookup_table_t* t, uint8_t* key)
{
    if(t->key_size == 0) return t->default_val;
    uint8_t* ret = naive_ternary_lookup(t->table, key);
    return ret == NULL ? t->default_val : ret;
}
