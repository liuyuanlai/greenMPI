#include <stdio.h>
#include <mpi.h>
#include <string.h>
void SetCPUFreq(long freq, int core_id);

int MPI_Send(const void *buf, int count, MPI_Datatype type, int to, int tag, MPI_Comm comm)
{
	printf("Calling C MPI_Send to %d\n", to);
	system("sudo cpupower frequency-set -f 1.2Ghz");
	int res =  PMPI_Send(buf, count, type, to, tag, comm);
	system("sudo cpupower frequency-set -f 2.6Ghz");
	return res;
}

void SetCPUFreq(long freq, int core_id){
    int i;
    char command[100], freq_buffer[7], index_buffer[2];
    strcpy(command, "echo ");
    sprintf(freq_buffer, "%ld", freq);
    strcat(command, freq_buffer);
    strcat(command, " > /sys/devices/system/cpu/cpu");
    sprintf(index_buffer, "%d", core_id);
    strcat(command, index_buffer);
    strcat(command, "/cpufreq/scaling_setspeed");
    system(command);
}
