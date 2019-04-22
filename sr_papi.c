#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "mpi.h"
#include "papi.h"
#include "papi_test.h"

#define N 100000
#define NMSG 100000
#define MAX_RAPL_EVENTS 64

int main(int argc, char *argv[])
{
    TESTS_QUIET = 1;

    int retval,cid,rapl_cid=-1,numcmp;
    int EventSet = PAPI_NULL;
    long long *values;
    int num_events=0;
    int code;
    char event_names[MAX_RAPL_EVENTS][PAPI_MAX_STR_LEN];
    char units[MAX_RAPL_EVENTS][PAPI_MIN_STR_LEN];
    int data_type[MAX_RAPL_EVENTS];
    int r,i;
    const PAPI_component_info_t *cmpinfo = NULL;
    PAPI_event_info_t evinfo;
    long long before_time,after_time;
    double elapsed_time;

    /* PAPI Initialization */
     retval = PAPI_library_init( PAPI_VER_CURRENT );
     if ( retval != PAPI_VER_CURRENT ) {
    test_fail(__FILE__, __LINE__,"PAPI_library_init failed\n",retval);
     }

     if (!TESTS_QUIET) {
        printf("Trying all RAPL events\n");
     }

     numcmp = PAPI_num_components();

     for(cid=0; cid<numcmp; cid++) {

    if ( (cmpinfo = PAPI_get_component_info(cid)) == NULL) {
       test_fail(__FILE__, __LINE__,"PAPI_get_component_info failed\n", 0);
    }

    if (strstr(cmpinfo->name,"rapl")) {

       rapl_cid=cid;

       if (!TESTS_QUIET) {
          printf("Found rapl component at cid %d\n",rapl_cid);
       }

           if (cmpinfo->disabled) {
          if (!TESTS_QUIET) {
         printf("RAPL component disabled: %s\n",
                        cmpinfo->disabled_reason);
          }
              test_skip(__FILE__,__LINE__,"RAPL component disabled",0);
           }
       break;
    }
     }

     /* Component not found */
     if (cid==numcmp) {
       test_skip(__FILE__,__LINE__,"No rapl component found\n",0);
     }

     /* Create EventSet */
     retval = PAPI_create_eventset( &EventSet );
     if (retval != PAPI_OK) {
    test_fail(__FILE__, __LINE__,
                              "PAPI_create_eventset()",retval);
     }

     /* Add all events */

     code = PAPI_NATIVE_MASK;

     r = PAPI_enum_cmp_event( &code, PAPI_ENUM_FIRST, rapl_cid );

     while ( r == PAPI_OK ) {

        retval = PAPI_event_code_to_name( code, event_names[num_events] );
    if ( retval != PAPI_OK ) {
       printf("Error translating %#x\n",code);
       test_fail( __FILE__, __LINE__,
                            "PAPI_event_code_to_name", retval );
    }

    retval = PAPI_get_event_info(code,&evinfo);
    if (retval != PAPI_OK) {
      test_fail( __FILE__, __LINE__,
             "Error getting event info\n",retval);
    }

    strncpy(units[num_events],evinfo.units,sizeof(units[0])-1);
    // buffer must be null terminated to safely use strstr operation on it below
    units[num_events][sizeof(units[0])-1] = '\0';

    data_type[num_events] = evinfo.data_type;

        retval = PAPI_add_event( EventSet, code );
        if (retval != PAPI_OK) {
      break; /* We've hit an event limit */
    }
    num_events++;

        r = PAPI_enum_cmp_event( &code, PAPI_ENUM_EVENTS, rapl_cid );
     }

     values=calloc(num_events,sizeof(long long));
     if (values==NULL) {
    test_fail(__FILE__, __LINE__,
                              "No memory",retval);
     }

     if (!TESTS_QUIET) {
    printf("\nStarting measurements...\n\n");
     }

     /* Start Counting */
     before_time=PAPI_get_real_nsec();
     retval = PAPI_start( EventSet);
     if (retval != PAPI_OK) {
    test_fail(__FILE__, __LINE__, "PAPI_start()",retval);
     }

    int rank, size;
    //char hostname[255];
    //int size_hostname;
    MPI_Status status;
    clock_t start_t, end_t;
    start_t = clock();
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    //MPI_Get_processor_name(hostname, &size_hostname);
    //printf("rank %d in %s\n", rank, hostname);
    if (rank < 12) {
        int* send_buffer = (int *)malloc(N * sizeof(int));
    for (i = 0; i < N; i++)
        send_buffer[i] = i;
    for (i = 0; i < NMSG; i++){
        MPI_Send(send_buffer, N, MPI_INT, rank + 12, 0, MPI_COMM_WORLD);
        }   
        printf("send conpleted\n");
    }
    else {
        int* recv_buffer = (int *)malloc(N * sizeof(int));
    for(i = 0; i < NMSG; i++)
        MPI_Recv(recv_buffer, N, MPI_INT, rank - 12, 0, MPI_COMM_WORLD, &status);
        end_t = clock();
        clock_t t = end_t - start_t;
        double time_taken = ((double)t)/CLOCKS_PER_SEC;

    printf("recv completed!\n");
        printf("Send/Recv took %f seconds to execute \n", time_taken);
    }
    MPI_Finalize();

    /* Stop Counting */
     after_time=PAPI_get_real_nsec();
     retval = PAPI_stop( EventSet, values);
     if (retval != PAPI_OK) {
    test_fail(__FILE__, __LINE__, "PAPI_stop()",retval);
     }

     elapsed_time=((double)(after_time-before_time))/1.0e9;

     if (rank == 0) {
        printf("\nStopping measurements, took %.3fs, gathering results...\n\n",
           elapsed_time);

        printf("Scaled energy measurements:\n");

        for(i=0;i<num_events;i++) {
           if (strstr(units[i],"nJ")) {

              printf("%-40s%12.6f J\t(Average Power %.1fW)\n",
                event_names[i],
                (double)values[i]/1.0e9,
                ((double)values[i]/1.0e9)/elapsed_time);
           }
        }

        // printf("\n");
        // printf("Energy measurement counts:\n");

        // for(i=0;i<num_events;i++) {
        //    if (strstr(event_names[i],"ENERGY_CNT")) {
        //       printf("%-40s%12lld\t%#08llx\n", event_names[i], values[i], values[i]);
        //    }
        // }

        // printf("\n");
        // printf("Scaled Fixed values:\n");

        // for(i=0;i<num_events;i++) {
        //    if (!strstr(event_names[i],"ENERGY")) {
        //      if (data_type[i] == PAPI_DATATYPE_FP64) {

        //          union {
        //            long long ll;
        //            double fp;
        //          } result;

        //         result.ll=values[i];
        //         printf("%-40s%12.3f %s\n", event_names[i], result.fp, units[i]);
        //       }
        //    }
        // }

        // printf("\n");
        // printf("Fixed value counts:\n");

        // for(i=0;i<num_events;i++) {
        //    if (!strstr(event_names[i],"ENERGY")) {
        //       if (data_type[i] == PAPI_DATATYPE_UINT64) {
        //         printf("%-40s%12lld\t%#08llx\n", event_names[i], values[i], values[i]);
        //       }
        //    }
        // }

     }

     /* Done, clean up */
     retval = PAPI_cleanup_eventset( EventSet );
     if (retval != PAPI_OK) {
    test_fail(__FILE__, __LINE__,
                              "PAPI_cleanup_eventset()",retval);
     }

     retval = PAPI_destroy_eventset( &EventSet );
     if (retval != PAPI_OK) {
    test_fail(__FILE__, __LINE__,
                              "PAPI_destroy_eventset()",retval);
     }

     //test_pass( __FILE__ );


    return 0;
}
