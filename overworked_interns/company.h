/**
 * Overworked Interns Lab
 * CS 241 - Fall 2016
 */
#ifndef __COMPANY_H__
#define __COMPANY_H__
#include <pthread.h>

#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_WHITE "\x1b[37m"
/**
 * This flag is initialized in the simulator,
 * so we are extern'ing it to make it visable.
 */
extern volatile int running;

/**
 * This flag is used to determine how long operations like
 * trying to work, working, and meeting take.
 */
extern unsigned int delta;
extern volatile int working;
/**
 * The struct for a company object.
 * The member variables is private,
 * Please access them from public apis.
 * Trying to access member variables through
 * object directly will fail
 */
typedef struct Company Company;

/**
 * This is the constructor for Company.
 * By calling the constructor, you get a pointer to a
 * Company object. And it is your responsibility
 * to call Company's destructor on the object you
 * create.
 */
Company *Company_create();

/**
 * This is the start_routine that the company uses to grab
 * the intern to the left and right.
 */
void *work_interns(void *p);

/**
 * Call this function once you have locked both the left and right intern.
 * This is will increment 'company' billable_days.
 */
void Company_hire_interns(Company *company);

/**
 * Call this function so that your company can have a board meeting.
 * Remember: A company can only be working / trying to work or having a board
 * meeting.
 */
void Company_have_board_meeting(Company *company);

/**
 * Return a pointer to the left intern
 */
pthread_mutex_t *Company_get_left_intern(Company *company);

/**
 * Sets the pointer to the left intern
 */
void Company_set_left_intern(Company *company, pthread_mutex_t *left_intern);

/**
 * Return a pointer to the right intern
 */
pthread_mutex_t *Company_get_right_intern(Company *company);

/**
 * Sets the pointer to the right intern
 */
void Company_set_right_intern(Company *company, pthread_mutex_t *right_intern);

/**
 * Returns the company number of 'company'.
 */
int Company_get_company_number(Company *company);

/**
 * Sets the company number of 'company'.
 */
void Company_set_company_number(Company *company, int company_number);

/**
 * Returns the failed flag.
 */
int Company_get_failed_flag(Company *company);

/**
 * Sets the failed flag.
 */
void Company_set_failed_flag(Company *company, int fail);

/**
 * Sets how long a job at the company takes
 */
void Company_set_job_duration(Company *company, int job_duration);

/**
 * Gets the screen position X coordinate
 */
double Company_get_X(Company *company);

/**
 * Sets the screen position X coordinate
 */
void Company_set_X(Company *company, double x);

/**
 * Gets the screen position Y coordinate
 */
double Company_get_Y(Company *company);

/**
 * Sets the screen position y coordinate
 */
void Company_set_Y(Company *company, double y);

/**
 * Call this function once you have locked both the left and right intern.
 * This is will increment 'company' billable_days.
 */
void Company_hire_interns(Company *company);

/**
 * Returns a pointer to the thread that represents the company
 */
pthread_t *Company_get_thread(Company *company);

/**
 * Returns the failed flag.
 */
int Company_get_failed_flag(Company *company);

/**
 * Sets the failed flag.
 */
void Company_set_failed_flag(Company *company, int fail);

/**
 * Returns how long a job at the company takes
 */
int Company_get_job_duration(Company *company);

/**
 * Sets how long a job at the company takes
 */
void Company_set_job_duration(Company *company, int job_duration);

/**
 * Returns how many billable days the company has.
 */
int Company_get_billable_days(Company *company);
void Company_set_tid(Company *company, int tid);
int Company_get_tid(Company *company);
#endif
