#include "bench.h"


/////////////////////////////////////////////////////////////////////
///kanw ena struct gia na pernaw ta orismata otan kanw pthread_create
struct arg_struct{
	long int count;
	int r;
	int threads;
};

//helper function for pthread_create to call _write_test method
void *call_write(void *arg){
	struct arg_struct *s = (struct arg_struct *) arg;
	_write_test(s->count, s->r, s->threads);
	return 0;
}

//helper function for pthread_create to call _read_test method
void *call_read(void *arg){
	struct arg_struct *s = (struct arg_struct *) arg;
	_read_test(s->count, s->r, s->threads);
	return 0;
}

//create threads to write or read
void _create_writers_or_readers(long count,int r,int threads,int select)
{
	struct arg_struct thread_args;
	pthread_t some_thread[threads];
	int i;

	//initialize thread arguments
	thread_args.count=count;
	thread_args.r=r;
	thread_args.threads=threads;
	if(select==0)		//create writers
	{
		//create threads 
		for(i=0;i<threads;i++) 
		{
			pthread_create(&some_thread[i],NULL,call_write,(void *) &thread_args);
		}
		//wait thread to end
		for(i=0;i<threads;i++)
		{
			pthread_join(some_thread[i],NULL);
		}
	}
	if(select==1)		//create readers
	{
		//create threads
		for(i=0;i<threads;i++)
		{
			pthread_create(&some_thread[i],NULL,call_read,(void *) &thread_args);
		}
		//wait thread to end
		for(i=0;i<threads;i++)
		{
			pthread_join(some_thread[i],NULL);
		}
	}
}

//create thread to write and read 
void _create_writers_and_readers(long int count,int r,int threads,int writer_percentage,int reader_percentage)
{
	struct arg_struct writer_args;
	struct arg_struct reader_args;
	pthread_t writer[threads];
	pthread_t reader[threads];
	int i;

	//initialize writer thread arguments
	writer_args.count=(long) (count*writer_percentage/100);
	writer_args.r=r;
	writer_args.threads=(int) (threads*writer_percentage/100);

	//initialize reader thread arguments
	reader_args.count=(long) (count*reader_percentage/100);
	reader_args.r=r;
	reader_args.threads=(int) (threads *reader_percentage/100);
	
	//create writer thread 
	for(i=0;i<writer_args.threads;i++)
	{
		pthread_create(&writer[i],NULL,call_write,(void *) &writer_args);
	}
	//create reader thread 
	for(i=0;i<reader_args.threads;i++)
	{
		pthread_create(&reader[i],NULL,call_read,(void *) &reader_args);
	}
	///wait readers to end
	for(i=0;i<reader_args.threads;i++)
	{
		pthread_join(reader[i],NULL);
	}
	///wait writers to end
	for(i=0;i<writer_args.threads;i++)
	{
		pthread_join(writer[i],NULL);
	}
}

//scan the percentages from user
int* scan_percentages(int *array)
{
	char percentages[5];
	char space[]=" ";
	int writer_percentage;
	int reader_percentage;

	printf("select a percentage for writers and readers(50 50,70 30,40 60, etc.)\n");
	scanf("%[^\n]s",percentages);	
	//check if the user write something wrong
	while(isdigit(percentages[0])==0 || isdigit(percentages[1])==0 || isspace(percentages[2])==0 
		|| isdigit(percentages[3])==0 || isdigit(percentages[4])==0 || strlen(percentages)>5)
	{
		printf("Wrong percentage please try again(50 50,70 30,40 60, etc.)\n");
		getchar();
		scanf("%[^\n]s",percentages);
	}

	char *ptr = strtok(percentages, space);
	writer_percentage =atoi(ptr); 
	ptr = strtok(NULL, space);
	reader_percentage =atoi(ptr);
	array[0]=writer_percentage;
	array[1]=reader_percentage;
	return array;	//return the percentages 
}
//return count multiplied by the percentage
long int  calculate_count(long int count,int percentage)
{
	long int percentage_count;
	percentage_count = (long) (count*percentage/100);
	return percentage_count;
}
////////////////////////////////////////////////////////////////////////////

void _random_key(char *key,int length) {
	int i;
	char salt[36]= "abcdefghijklmnopqrstuvwxyz0123456789";

	for (i = 0; i < length; i++)
		key[i] = salt[rand() % 36];
}

void _print_header(int count)
{
	double index_size = (double)((double)(KSIZE + 8 + 1) * count) / 1048576.0;
	double data_size = (double)((double)(VSIZE + 4) * count) / 1048576.0;

	printf("Keys:\t\t%d bytes each\n", 
			KSIZE);
	printf("Values: \t%d bytes each\n", 
			VSIZE);
	printf("Entries:\t%d\n", 
			count);
	printf("IndexSize:\t%.1f MB (estimated)\n",
			index_size);
	printf("DataSize:\t%.1f MB (estimated)\n",
			data_size);

	printf(LINE1);
}

void _print_environment()
{
	time_t now = time(NULL);

	printf("Date:\t\t%s", 
			(char*)ctime(&now));

	int num_cpus = 0;
	char cpu_type[256] = {0};
	char cache_size[256] = {0};

	FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
	if (cpuinfo) {
		char line[1024] = {0};
		while (fgets(line, sizeof(line), cpuinfo) != NULL) {
			const char* sep = strchr(line, ':');
			if (sep == NULL || strlen(sep) < 10)
				continue;

			char key[1024] = {0};
			char val[1024] = {0};
			strncpy(key, line, sep-1-line);
			strncpy(val, sep+1, strlen(sep)-1);
			if (strcmp("model name", key) == 0) {
				num_cpus++;
				strcpy(cpu_type, val);
			}
			else if (strcmp("cache size", key) == 0)
				strncpy(cache_size, val + 1, strlen(val) - 1);	
		}

		fclose(cpuinfo);
		printf("CPU:\t\t%d * %s", 
				num_cpus, 
				cpu_type);

		printf("CPUCache:\t%s\n", 
				cache_size);
	}
}

int main(int argc,char** argv)
{
	long int count;
	int threads;
	//initialize costs and locks
	cost_of_writers=0;
	cost_of_readers=0;
	pthread_mutex_init(&writer_lock,NULL);
	pthread_mutex_init(&reader_lock,NULL);

	int writers = 0; 	//to select write in method _create_writers_or_readers
	int readers = 1;	//to select read in method _create_writers_or_readers

	srand(time(NULL));
	if (argc < 4) { 	///+1
		fprintf(stderr,"Usage: db-bench <write | read | write_read> <count> <threads>\n");
		exit(1);
	}
	
	if (strcmp(argv[1], "write") == 0) {
		int r = 0;

		count = atoi(argv[2]);
		_print_header(count);
		_print_environment();
		if (argc == 5) ///+1
			r = 1;

		/////////////////////////////////////////////
		threads = atoi(argv[3]);					//take threads number from command line 
		_db_open();									//open db to write 
		_create_writers_or_readers(count,r,threads,writers);		//write
		_db_close();								//close db
		_print_writers_costs(count);				//threads are finished so print the total cost of writers
		////////////////////////////////////////////
	} else if (strcmp(argv[1], "read") == 0) {
		int r = 0;

		count = atoi(argv[2]);
		_print_header(count);
		_print_environment();
		if (argc == 5) ///+1
			r = 1;
		
		/////////////////////////////////////////////
		threads = atoi(argv[3]);					//take threads number from command line
		_db_open();									//open db to read 
		_create_writers_or_readers(count,r,threads,readers);		//read 
		_db_close();								//close db
		_print_readers_costs(count);				//threads are finished so print the total cost of readers
		////////////////////////////////////////////
		
	///erotima 5
	} else if(strcmp(argv[1], "write_read")==0){

		int r = 0;
		int writer_percentage;
		int reader_percentage;
		long int writer_count;
		long int reader_count;
		int array[2];
		int* percentages;
		
		count = atoi(argv[2]);
		_print_header(count);
		_print_environment();
		if (argc == 5) ///+1
			r = 1;
		
		threads = atoi(argv[3]);		//take threads number from command line 

		percentages = scan_percentages(array);		//take percentages from user
		writer_percentage = percentages[0];
		reader_percentage = percentages[1];
		writer_count = calculate_count(count,writer_percentage);	//to know the new count
		reader_count = calculate_count(count,reader_percentage);	//to know the new count

		_db_open();			//open db to write or read
		_create_writers_and_readers(count,r,threads,writer_percentage,reader_percentage); 		//write and read
		_db_close();		//close db	
		_print_writers_costs(writer_count);				//threads are finished so print the total cost of writers
		_print_readers_costs(reader_count);				//threads are finished so print the total cost of readers
	} else {
		fprintf(stderr,"Usage: db-bench <write | read | write_read> <count> <threads>\n");
		exit(1);
	}

	return 1;
}
