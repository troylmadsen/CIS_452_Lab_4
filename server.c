#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_FILE_NAME_LENGTH 256

/*
 * Awaits user input and spawns off a thread to handler the user "file" request.
 * @Version 1.0
 * @Author Troy Madsen
 * @Author Tim DeVries
 * @Date 2018/09/20
 */

/* Method signatures */
void* getFile( void* args );
static void sigHandler( int signum, siginfo_t* siginfo, void* context );

/* Globals */
int num_requested = 0;
int num_serviced = 0;
int service_time = 0;
pthread_mutex_t serviced_lock;
pthread_mutex_t service_time_lock;

/*
 * Runs a multi-threaded server that awaits user input to retrieve a file.
 */
int main() {

	/* Signal handlers */
	struct sigaction sa;

	sigemptyset( &sa.sa_mask );
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = &sigHandler;

	if ( sigaction( SIGINT, &sa, NULL ) == -1 ) {
		perror( "sigaction failure" );
		exit( 1 );
	}

	/* User input */
	char* input;

	/* Thread */
	int status;
	pthread_t thread;

	/* Setup mutexes */
	if ( pthread_mutex_init( &serviced_lock, NULL ) != 0 ) {
		perror( "Mutex initialization failed" );
		exit( 1 );
	}

	if ( pthread_mutex_init( &service_time_lock, NULL ) != 0 ) {	
		perror( "Mutex initialization failed" );
		exit( 1 );
	}

	/* Receive requests */
	while ( 1 ) {
		/* get user input */
		input = (char *)malloc( MAX_FILE_NAME_LENGTH * sizeof( char * ) );
		printf( ">" );
		fgets( input, MAX_FILE_NAME_LENGTH, stdin );

		/* Trim \n off input */
		int i = 0;
		while ( input[i++] != '\n' );
		input[i - 1] = '\0';

		/* Make baby */
		status = pthread_create( &thread, NULL, getFile, input );
		if ( status != 0 ) {
			perror( "Thread creation error" );
			exit( 1 );
		}

		/* Cut child off */
		pthread_detach( thread );

		/* Increment num_requested */
		num_requested++;
	}

	return 0;
}

/*
 * simulates a thread retrieving a file with a wait time.
 * @param args Pointer to a string filename
 * @returns NULL
 */
void* getFile( void* args ) {
	/* Casting passed parameters */
	char* file_name = (char *)args;

	/* Sleep the thread for a time */
	int sleep_time;
	if ( rand() % 5 ) {
		/* File found in cache */
		sleep_time = 1;
	}
	else {
		/* Retrieving from disk */
		sleep_time = (rand() % 4) + 7;
	}
	sleep(sleep_time);

	/* Print diagnostic message */
	printf( "File \"%s\" delivered\n", file_name );

	/* Increment num_serviced */
	pthread_mutex_lock( &serviced_lock );
	num_serviced++;
	pthread_mutex_unlock( &serviced_lock );

	/* Add service time */
	pthread_mutex_lock( &service_time_lock );
	service_time += sleep_time;
	pthread_mutex_unlock( &service_time_lock );

	/* Free memory */
	free( file_name );

	return NULL;
}

/*
 * Handles the signals sent to the program.
 * @param signum Signal number
 * @param siginfo Signal info
 * @param context Function to call at signal delivery
 */
static void sigHandler( int signum, siginfo_t* siginfo, void* context ) {
	printf( "\n" );
	printf( "Total files requested: %d\n", num_requested );
	printf( "Total files serviced: %d\n", num_serviced );
	printf( "Total service_time duration: %d\n", service_time );
	exit( 0 );
}
