#include <string.h>
#include "../engine/db.h"
#include "../engine/variant.h"
#include "bench.h"

#define DATAS ("testdb")

DB* db; //make db global 

////////////////////////////////////////////////////////
//helper method to open db
void _db_open()
{
	db = db_open(DATAS);
}

//helper method to close db 
void _db_close()
{
	db_close(db);
}

//helper method to print total cost of writers
void _print_writers_costs(long int count)
{
	printf(LINE);
	printf("|Random-Write	(done:%ld): %.6f sec/op; %.1f writes/sec(estimated); cost:%.3f(sec);\n"
		,count, (double)(cost_of_writers / count)
		,(double)(count / cost_of_writers)
		,cost_of_writers);
}

//helper method to print total cost of readers
void _print_readers_costs(long int count)
{
	printf(LINE);
	printf("|Random-Read	(done:%ld): %.6f sec/op; %.1f reads/sec(estimated); cost:%.3f(sec);\n"
		,count, (double)(cost_of_readers / count)
		,(double)(count / cost_of_readers)
		,cost_of_readers);
}
////////////////////////////////////////////////////////////

void _write_test(long int count, int r,int threads)			//add an argument to know threads number
{
	int i;
	double cost;
	long long start,end;
	Variant sk, sv;
	long int threads_count;					//new count

	char key[KSIZE + 1];
	char val[VSIZE + 1];
	char sbuf[1024];

	memset(key, 0, KSIZE + 1);
	memset(val, 0, VSIZE + 1);
	memset(sbuf, 0, 1024);

	start = get_ustime_sec();

	threads_count = count/threads;			//split the work for every thread
	for (i = 0; i < threads_count; i++) {
		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", i);
		fprintf(stderr, "%d adding %s\n", i, key);
		snprintf(val, VSIZE, "val-%d", i);

		sk.length = KSIZE;
		sk.mem = key;
		sv.length = VSIZE;
		sv.mem = val;

		db_add_helper(db, &sk, &sv);		//call a helper method to add 
		if ((i % 10000) == 0) {
			fprintf(stderr,"random write finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}
	}
	end = get_ustime_sec();
	cost = end -start;
	pthread_mutex_lock(&writer_lock);			//lock from other threads 
	cost_of_writers=cost_of_writers+cost;		//add on total cost the cost of this thread
	pthread_mutex_unlock(&writer_lock);			//unlock
}

void _read_test(long int count, int r,int threads)
{
	int i;
	int ret;
	int found = 0;
	double cost;
	long long start,end;
	Variant sk;
	Variant sv;

	char key[KSIZE + 1];

	long int threads_count;			//new count 

	start = get_ustime_sec();
	threads_count = count/threads;				//split the work for every thread
	for (i = 0; i < threads_count; i++) {
		memset(key, 0, KSIZE + 1);

		/* if you want to test random write, use the following */
		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", i);
		fprintf(stderr, "%d searching %s\n", i, key);
		sk.length = KSIZE;
		sk.mem = key;
		ret = db_get_helper(db, &sk, &sv);		//call a helper method to get 
		if (ret) {
			//db_free_data(sv.mem);
			found++;
		} else {
			INFO("not found key#%s", 
					sk.mem);
    	}

		if ((i % 10000) == 0) {
			fprintf(stderr,"random read finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}
	}
	end = get_ustime_sec();
	cost = end - start;
	pthread_mutex_lock(&reader_lock);			//lock from other threads
	cost_of_readers=cost_of_readers+cost;		//add on total cost the cost of this thread
	pthread_mutex_unlock(&reader_lock);			//unlock
}
