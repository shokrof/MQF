#ifndef QF_H
#define QF_H

#include <inttypes.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Can be
	0 (choose size at run-time),
	8, 16, 32, or 64 (for optimized versions),
	or other integer <= 56 (for compile-time-optimized bit-shifting-based versions)
	*/
#define BITS_PER_SLOT 0

	struct __attribute__ ((__packed__)) qfblock;
	typedef struct qfblock qfblock;

	uint64_t shift_into_b2(uint64_t a, uint64_t b, int bstart, int bend, int amount);

	typedef struct {
		uint64_t total_time_single;
		uint64_t total_time_spinning;
		uint64_t locks_taken;
		uint64_t locks_acquired_single_attempt;
	} wait_time_data;

	typedef struct quotient_filter_mem {
		int fd;
		volatile int metadata_lock;
		volatile int *locks;
		wait_time_data *wait_times;
	} quotient_filter_mem;

	typedef quotient_filter_mem qfmem;

	typedef struct quotient_filter_metadata {
		uint64_t size;
		uint32_t seed;
		uint64_t nslots;
		uint64_t xnslots;
		uint64_t key_bits;
		uint64_t tag_bits;
		uint64_t fixed_counter_size;
		uint64_t key_remainder_bits;
		uint64_t bits_per_slot;
		__uint128_t range;
		uint64_t nblocks;
		uint64_t nelts;
		uint64_t ndistinct_elts;
		uint64_t noccupied_slots;
		uint64_t maximum_occupied_slots;
		uint64_t num_locks;
		bool mem;
	} quotient_filter_metadata;

	typedef quotient_filter_metadata qfmetadata;

	typedef struct quotient_filter {
		qfmem *mem;
		qfmetadata *metadata;
		qfblock *blocks;
	} quotient_filter;

	typedef quotient_filter QF;

	typedef struct {
		uint64_t start_index;
		uint16_t length;
	} cluster_data;

	typedef struct quotient_filter_iterator {
		QF *qf;
		uint64_t run;
		uint64_t current;
		uint64_t cur_start_index;
		uint16_t cur_length;
		uint32_t num_clusters;
		cluster_data *c_info;
	} quotient_filter_iterator;

	typedef quotient_filter_iterator QFi;



	void qf_init(QF *qf, uint64_t nslots, uint64_t key_bits, uint64_t value_bits,uint64_t fixed_counter_size, bool mem, const char *path, uint32_t seed);

	void qf_reset(QF *qf);

	void qf_destroy(QF *qf);

	void qf_copy(QF *dest, QF *src);

	/* Increment the counter for this item by count.
		Qf* qf : pointer to the Filter
		uint64_t key : hash of the item to be insertedItems
		uint64_t count: Count to be added
		bool lock: For Multithreading, Lock the slot used by the current thread so that other threads can't change the value
		bool spin: For Multithreading, If there is a lock on the target slot. wait until the lock is freed and insert the count.
	 */
	bool qf_insert(QF *qf, uint64_t key, uint64_t count,
								 bool lock=false, bool spin=false);


	/* Remove all instances of this key/value pair. */
	//void qf_delete_key_value(QF *qf, uint64_t key, uint64_t value);

	/* Remove all instances of this key. */
	//void qf_delete_key(QF *qf, uint64_t key);

	/* Replace the association (key, oldvalue, count) with the association
		 (key, newvalue, count). If there is already an association (key,
		 newvalue, count'), then the two associations will be merged and
		 their counters will be summed, resulting in association (key,
		 newvalue, count' + count). */
	void qf_replace(QF *qf, uint64_t key, uint64_t oldvalue, uint64_t newvalue);

	/* Lookup the value associated with key.  Returns the count of that
		 key/value pair in the QF.  If it returns 0, then, the key is not
		 present in the QF. Only returns the first value associated with key
		 in the QF.  If you want to see others, use an iterator. */
	uint64_t qf_query(const QF *qf, uint64_t key, uint64_t *value);

	/* Return the number of times key has been inserted, with any value,
		 into qf. */
	uint64_t qf_count_key(const QF *qf, uint64_t key);

	/* Decrement the counter for this item by count.
		Qf* qf : pointer to the Filter
		uint64_t key : hash of the item to be removed
		uint64_t count: Count to be removed
		bool lock: For Multithreading, Lock the slot used by the current thread so that other threads can't change the value
		bool spin: For Multithreading, If there is a lock on the target slot. wait until the lock is freed and insert the count.
	 */
	bool qf_remove(QF *qf, uint64_t hash, uint64_t count,  bool lock=false, bool spin=false);


	uint64_t qf_add_tag(const QF *qf, uint64_t key, uint64_t tag, bool lock=false, bool spin=false);
	uint64_t qf_get_tag(const QF *qf, uint64_t key);
	uint64_t qf_remove_tag(const QF *qf, uint64_t key, bool lock=false, bool spin=false);

	/* Initialize an iterator */
	bool qf_iterator(QF *qf, QFi *qfi, uint64_t position);

	/* Returns 0 if the iterator is still valid (i.e. has not reached the
		 end of the QF. */
	int qfi_get(QFi *qfi, uint64_t *key, uint64_t *value, uint64_t *count);

	/* Advance to next entry.  Returns whether or not another entry is
		 found.  */
	int qfi_next(QFi *qfi);

	/* Check to see if the if the end of the QF */
	int qfi_end(QFi *qfi);

	/* For debugging */
	void qf_dump(const QF *);

	/* write data structure of to the disk */
	void qf_serialize(const QF *qf, const char *filename);

	/* read data structure off the disk */
	void qf_deserialize(QF *qf, const char *filename);

	/* mmap the QF from disk. */
	void qf_read(QF *qf, const char *path);

	/* merge two QFs into the third one. */
	void qf_merge(QF *qfa, QF *qfb, QF *qfc);

	void qf_intersect(QF *qfa, QF *qfb, QF *qfc);

	void qf_subtract(QF *qfa, QF *qfb, QF *qfc);
	/* merge multiple QFs into the final QF one. */
	void qf_multi_merge(QF *qf_arr[], int nqf, QF *qfr);

	/* resize the filter into a bigger or smaller one
	Qf* qf : pointer to the Filter
	uint64_t newQ: new number of slots(Q). the slot size will be recalculated to keep the range constant.
	string originalFilename(optional): dump the current filter to the disk to free space for the new filter. Filename is provided as the content of the string.
	string newFilename(optional): the new filter is created on disk. Filename is provided as the content of the string.
	*/
	QF* qf_resize(QF* qf, int newQ, const char * originalFilename=NULL, const char * newFilename=NULL);
	/* find cosine similarity between two QFs. */
	uint64_t qf_inner_product(QF *qfa, QF *qfb);

	/* magnitude of a QF. */
	uint64_t qf_magnitude(QF *qf);
	/* return the filled space(percent) */
	int qf_space(QF *qf);

	bool qf_equals(QF *qfa, QF *qfb);


	typedef struct qf_index{
		QF* main_qf;
		QF* index_qf;
	};
	void qf_index_init(qf_index* qf,const char *path,uint64_t index_slot_size,uint64_t index_tag_size);
	void may_exists(qf_index* qf,uint64_t key);
	void approx_count_range(qf_index* qf, uint64_t key, uint64_t* min_approx_count, uint64_t* max_approx_count);



#ifdef __cplusplus
}
#endif

#endif /* QF_H */
