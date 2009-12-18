#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cobo.h"

int ranks, my_rank;
char** procs;

size_t size = 1024;
size_t buffer_size;
char* sbuffer;
char* rbuffer;

/* fill the buffer with a pattern */
void init_sbuffer(int rank)
{
  size_t i;
  char value;
  for(i = 0; i < buffer_size; i++)
  {
    value = (char) ((i+1)*(rank+1) + i);
    sbuffer[i] = value;
  }
}

/* blank out the receive buffer */
void init_rbuffer(int rank)
{
  memset(rbuffer, 0, buffer_size);
}

/* check the send buffer for any deviation from expected pattern */
void check_sbuffer(char* op)
{
  size_t i;
  char value;
  for(i = 0; i < buffer_size; i++)
  {
    value = (char) ((i+1)*(my_rank+1) + i);
    if (sbuffer[i] != value)
    {
      printf("%d: %s: Send buffer corruption detected at sbuffer[%d]\n",
             my_rank, op, i);
    }
  }
}

/* check the receive buffer for any deviation from expected pattern */
void check_rbuffer(char* buffer, size_t byte_offset, int rank, size_t src_byte_offset, size_t element_count, char* op)
{
  size_t i, j;
  char value;
  buffer += byte_offset;
  for(i = 0, j = src_byte_offset; i < element_count; i++, j++)
  {
    value = (char) ((j+1)*(rank+1) + j);
    if (buffer[i] != value)
    {
      printf("%d: %s: Receive buffer corruption detected at rbuffer[%d] from rank %d\n",
             my_rank, op, byte_offset+i, rank);
    }
  }
}

int main(int argc, char* argv[])
{
  int root = 0;
  int i;

  int num_ports = 10;
  int portlist[10] = {5000,5100,5200,5300,5400,5500,5600,5700,5800,5900};

  /* initialize the client (read environment variables) */
  if (cobo_open(portlist, num_ports, &my_rank, &ranks) != COBO_SUCCESS) {
    printf("Failed to init\n");
    exit(1);
  }
  printf("Ranks: %d, Rank: %d\n", ranks, my_rank);

  buffer_size = ranks * size;
  sbuffer = malloc(buffer_size);
  rbuffer = malloc(buffer_size);

  /* test cobo_barrier */
  if (cobo_barrier() != COBO_SUCCESS) {
    printf("Barrier failed\n");
    exit(1);
  }

  /* test cobo_bcast */
  init_sbuffer(my_rank);
  init_rbuffer(my_rank);
  void* buf = (void*) rbuffer;
  if (my_rank == root) { buf = sbuffer; }
  if (cobo_bcast(buf, (int) size, root) != COBO_SUCCESS) {
    printf("Bcast failed\n");
    exit(1);
  }
/*  check_sbuffer(); */
  check_rbuffer(buf, 0, root, 0, size, "cobo_bcast");

  /* test cobo_scatter */
  init_sbuffer(my_rank);
  init_rbuffer(my_rank);
  if (cobo_scatter(sbuffer, (int) size, rbuffer, root) != COBO_SUCCESS) {
    printf("Scatter failed\n");
    exit(1);
  }
  check_sbuffer("cobo_scatter");
  check_rbuffer(rbuffer, 0, root, my_rank*size, size, "cobo_scatter");

  /* test cobo_gather */
  init_sbuffer(my_rank);
  init_rbuffer(my_rank);
  if (cobo_gather(sbuffer, (int) size, rbuffer, root) != COBO_SUCCESS) {
    printf("Gather failed\n");
    exit(1);
  }
  check_sbuffer("cobo_gather");
  if (my_rank == root) {
    for (i = 0; i < ranks; i++) {
      check_rbuffer(rbuffer, i*size, i, 0, size, "cobo_gather");
    }
  }

  /* test cobo_allgather */
  init_sbuffer(my_rank);
  init_rbuffer(my_rank);
  if (cobo_allgather(sbuffer, (int) size, rbuffer) != COBO_SUCCESS) {
    printf("Allgather failed\n");
    exit(1);
  }
  check_sbuffer("cobo_allgather");
  for (i = 0; i < ranks; i++) {
    check_rbuffer(rbuffer, i*size, i, 0, size, "cobo_allgather");
  }

#if 0
  /* test cobo_alltoall */
/*
  init_sbuffer(my_rank);
  init_rbuffer(my_rank);
  if (cobo_alltoall(sbuffer, (int) size, rbuffer) != COBO_SUCCESS) {
    printf("Alltoall failed\n");
    exit(1);
  }
  check_sbuffer("cobo_alltoall");
  for (i = 0; i < ranks; i++) {
    check_rbuffer(rbuffer, i*size, i, my_rank*size, size, "cobo_alltoall");
  }
*/

/*
  int max;
  if (cobo_allreducemaxint(&my_rank, &max) != COBO_SUCCESS) {
    printf("Allreducemaxint failed\n");
    exit(1);
  }
  printf("%d: Max int %d\n", my_rank, max);

  char** hosts;
  char*  hostsbuf;
  char   host[255];
  gethostname(host, 255);
  if (cobo_allgatherstr(host, &hosts, &hostsbuf) != COBO_SUCCESS) {
    printf("Allgatherstr failed\n");
    exit(1);
  }
  int i;
  if (my_rank == 0 || my_rank == ranks-1) {
    for (i=0; i<ranks; i++) {
      printf("%d: hosts[%d] = %s\n", my_rank, i, hosts[i]);
    }
  }
  free(hosts);
  free(hostsbuf);
*/

#endif

done:
  /* close connections (close connection with launcher and tear down the TCP tree) */
  if (cobo_close() != COBO_SUCCESS) {
    printf("Failed to close\n");
    exit(1);
  }

  /* needed this sleep so that server prints out all debug info (don't know why yet) */
  sleep(1);

  return 0;
}
