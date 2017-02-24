/**
 * Machine Problem: Wearables
 * CS 241 - Fall 2016
 */

#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils.h"
#include "vector.h"
#include "wearable.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

const suseconds_t MICRO_TOLERANCE = 100 * 1000;

const char *sPort_ = NULL;

FILE *user_data_;
FILE *actual_data_;
FILE *report_;

struct timeval start_time_;

typedef struct StatsInfo {
  size_t size_;
  int *content_;

  Wearable **wearables_;
  size_t wearable_size_;
} StatsInfo;

// sets an alarm to trigger when we should have received
// the desired timestamp. This alarm triggers at a tolerance
// of MICRO_TOLERANCE. Note, this imlies that if the timestamp
// has already passed the alarm will trigger after MICRO_TOLERANCE
void set_alarm(int timestamp) {
  // printf("Setting new alarm for %i\n", timestamp);
  struct itimerval tout_val;
  struct timeval now, interval, temp;
  gettimeofday(&now, NULL);

  tout_val.it_interval.tv_sec = 0;
  tout_val.it_interval.tv_usec = 0;
  timersub(&now, &start_time_, &interval);

  if (interval.tv_sec * 1000 + interval.tv_usec / 1000 > timestamp) {
    tout_val.it_value.tv_sec = 0;
    tout_val.it_value.tv_usec = MICRO_TOLERANCE;
    setitimer(ITIMER_REAL, &tout_val, 0);
  } else {
    temp.tv_sec = timestamp / 1000; // timestamp is in millis
    temp.tv_usec = timestamp % 1000;
    timersub(&temp, &interval, &now);

    tout_val.it_value.tv_sec =
        now.tv_sec + (now.tv_usec + MICRO_TOLERANCE) / 1000000;
    tout_val.it_value.tv_usec = (now.tv_usec + MICRO_TOLERANCE) % 1000000;
    // printf("Timer set for %zu %i\n", tout_val.it_value.tv_sec,
    // tout_val.it_value.tv_usec);
  }

  setitimer(ITIMER_REAL, &tout_val, 0);
}

int get_end_timestamp(int e, StatsInfo *info) {
  unsigned int max = (unsigned int)e;
  for (size_t i = 0; i < info->wearable_size_; i++) {
    Wearable *cur = info->wearables_[i];
    for (size_t k = 0; cur->data_size_; k++) {
      if (k != cur->data_size_ - 1 &&
          cur->start_sleep_ + cur->sample_interval_ * k < (unsigned int)e)
        max = MAX(cur->start_sleep_ + cur->sample_interval_ * (k + 1), max);
      else
        break;
    }
  }
  return max;
}

// The thread sends the requests for statistics from the user
// It triggers alarms to fire to test if the statistics take
// too long to come in.
void *stats_thread(void *arg) {
  StatsInfo *info = (StatsInfo *)arg;
  int socketfd = socket(AF_INET, SOCK_STREAM, 0);
  gettimeofday(&start_time_, NULL);

  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  int ret;
  if ((ret = getaddrinfo(NULL, sPort_, &hints, &result)) != 0) {
    printf("Failed to get address info\n");
    fprintf(report_, "Failed to get address info\n");
    exit(EXIT_FAILURE);
  }

  // open connection
  connect(socketfd, result->ai_addr, result->ai_addrlen);
  freeaddrinfo(result);

  int *sample_times = info->content_;
  int sample_size = info->size_;
  int i;
  for (i = 0; i < sample_size * 3;
       i += 3) {                     // Loop through the statitic requests...
    int wait_time = sample_times[i]; // how long to wait before sending the
                                     // request... (in millis)
    int t1 = sample_times[i + 1];    // start timestamp
    int t2 = sample_times[i + 2];    // end timestamp

    usleep(wait_time * 1000); // sleep the desired time
    set_alarm(get_end_timestamp(
        t2,
        info)); // set an alarm to fire after we should have received our data

    char buffer[1024];
    sprintf(buffer, "%i:%i", t1, t2);
    int wret = write(socketfd, buffer, strlen(buffer));
    if (-1 == wret) {
      perror("write");
      exit(1);
    }

    // read here
    int current_size = 0;
    char *complete_message = NULL;
    while (1) {
      int b_read = read(socketfd, buffer, 1024);
      complete_message = realloc(complete_message, current_size + b_read + 1);
      memcpy(complete_message + current_size, buffer, b_read);
      current_size += b_read;
      if (strncmp(&complete_message[current_size - 2], "\r\n", 2) == 0)
        break;
    }
    complete_message[current_size - 2] = '\0';
    printf("Receieved:%s\n", complete_message);
    fprintf(user_data_, "%s", complete_message);
    free(complete_message);

    set_alarm(1000 * 1000 * 30); // clear out our alarm...
  }

  set_alarm(1000 * 1000 * 30);
  close(socketfd);
  return NULL;
}

bool get_type1_selector(timestamp_entry *arg) {
  SampleData *data = (SampleData *)arg->data;
  return strcmp(data->type_, TYPE1) == 0;
}

bool get_type2_selector(timestamp_entry *arg) {
  SampleData *data = (SampleData *)arg->data;
  return strcmp(data->type_, TYPE2) == 0;
}

bool get_type3_selector(timestamp_entry *arg) {
  SampleData *data = (SampleData *)arg->data;
  return strcmp(data->type_, TYPE3) == 0;
}

int compare_num(const void *a, const void *b) {
  return (*(int *)a - *(int *)b);
}

// Prints the results... see the template solution (its the same)
void print_results(const char *type, timestamp_entry *results, int size) {
  long avg = 0;
  int i;

  int temp_array[size];
  printf("Results for %s:\n", type);
  fprintf(actual_data_, "Results for %s:\n", type);

  printf("Size:%i\n", size);
  fprintf(actual_data_, "Size:%i\n", size);
  for (i = 0; i < size; i++) {
    temp_array[i] = ((SampleData *)(results[i].data))->data_;
    printf("%i %i\n", i, temp_array[i]);
    avg += ((SampleData *)(results[i].data))->data_;
  }

  qsort(temp_array, size, sizeof(int), compare_num);

  if (size > 0) {
    printf("Median:%i\n",
           ((size % 2 == 0)
                ? (temp_array[size / 2] + temp_array[size / 2 - 1]) / 2
                : temp_array[size / 2]));
    fprintf(actual_data_, "Median:%i\n",
            ((size % 2 == 0)
                 ? (temp_array[size / 2] + temp_array[size / 2 - 1]) / 2
                 : temp_array[size / 2]));
  } else {
    printf("Median:0\n");
    fprintf(actual_data_, "Median:0\n");
  }

  printf("Average:%li\n\n", (size == 0 ? 0 : avg / size));
  fprintf(actual_data_, "Average:%li\n\n", (size == 0 ? 0 : avg / size));
}

void alarm_wakeup(int i) {
  printf("Failed to deliver messages in time! (%i)\n", i);
  fprintf(report_, "Failed to deliver messages in time!\n");
}

int main(int argc, const char *argv[]) {
  report_ = fopen("_error_report.rp", "w+");
  if (argc != 4) {
    printf("Usage:\n\t%s wearable_port request_port input_file\n", argv[0]);
    fprintf(report_, "Invalid input size\n");
    fclose(report_);
    exit(EXIT_FAILURE);
  }

  actual_data_ = fopen("_expected.rp", "w+");
  user_data_ = fopen("_received.rp", "w+");

  signal(SIGALRM, alarm_wakeup);

  const char *wearable_port = argv[1];
  sPort_ = argv[2];

  useconds_t latest_launch;
  size_t count;
  size_t sample_count;
  int *sample_times;
  pthread_t stats_threadp;

  // create wearables
  Wearable **wearables =
      get_wearables(argv[3], &count, &sample_times, &sample_count,
                    &latest_launch, wearable_port);
  // wait for all wearables to finish
  void *ret_val = NULL;
  Vector *complete_data_set = Vector_create(timestamp_entry_copy_constructor,
                                            timestamp_entry_destructor);

  pthread_t threads[count];
  for (size_t i = 0; i < count; i++)
    pthread_create(&threads[i], NULL, wearable_thread, wearables[i]);

  StatsInfo info;
  info.wearables_ = wearables;
  info.wearable_size_ = count;
  info.size_ = sample_count;
  info.content_ = sample_times;
  pthread_create(&stats_threadp, NULL, stats_thread, &info);

  for (size_t i = 0; i < count; i++) {
    pthread_join(threads[i], &ret_val);
    for (size_t k = 0; k < Vector_size(wearables[i]->results_); k++) {
      timestamp_entry *entry = Vector_get(wearables[i]->results_, k);
      Vector_append(complete_data_set, entry);
    }
  }

  pthread_join(stats_threadp, NULL);
  // print results of all infomation
  for (size_t i = 0; i < sample_count * 3; i += 3) {
    int t1 = sample_times[i + 1];
    int t2 = sample_times[i + 2];

    ssize_t size;
    timestamp_entry *results;
    size = gather_timestamps(complete_data_set, t1, t2, get_type1_selector,
                             &results);
    print_results(TYPE1, results, size);
    free(results);

    size = gather_timestamps(complete_data_set, t1, t2, get_type2_selector,
                             &results);
    print_results(TYPE2, results, size);
    free(results);

    size = gather_timestamps(complete_data_set, t1, t2, get_type3_selector,
                             &results);
    print_results(TYPE3, results, size);
    free(results);
  }
  free(sample_times);

  for (size_t i = 0; i < count; i++)
    free_wearable(wearables[i]);

  free(wearables);
  Vector_destroy(complete_data_set);

  fclose(report_);
  fclose(user_data_);
  fclose(actual_data_);

  return 0;
}
