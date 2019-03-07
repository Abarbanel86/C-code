/****************************************************************************/
/*                    Exercise: Schedular                                   */
/*							Schedule										*/
/*                    Date: October 22, 2018.                               */
/*                    Team Name: RD111                                      */
/****************************************************************************/

#include <stdlib.h>	  /*size_t*/
#include <sys/time.h> /*gettimeofday*/
#include <assert.h>		/*assert*/

#include "./include/priority_queue.h"
#include "./include/schedule.h"     /*tsk_t*/

struct schedule
{
	pqlist_t *schedule;
	int flag_stop;
};

int SCDIsBefore(const void *data, const void *param);
int IsSameUID(const void *data, const void *param);


/*******scd mamagment*******************************/
scd_t *SCDCreate()
{
    scd_t *new = NULL;

    new = (scd_t*) malloc (sizeof(scd_t));
    if (NULL == new)
    {
        return NULL;
    }

    new->schedule = PQCreate(SCDIsBefore);
    if (NULL == new->schedule)
    {
        free(new);
        return NULL;
    }

    return new;
}

void SCDDestroy(scd_t *schedule)
{
	tsk_t *data = NULL;

    assert(schedule);

	while (0 == PQIsEmpty(schedule->schedule))
	{
		data = PQPeek(schedule->schedule);
    	PQDequeue(schedule->schedule);
		TSKDestroy(data);
	}

	PQDestroy(schedule->schedule);
    schedule->schedule = NULL;
    free(schedule);
    schedule = NULL;
}

uid_type SCDAdd(scd_t *schedule, struct timeval start, size_t interval,
			 task_func_t func, void *param)
{
	tsk_t *new = NULL;
	uid_type bad = {0};

	assert(schedule);
	assert(func);

	new = TSKCreate(start, interval, func, param);
	if (NULL == new)
	{
		return bad;
	}

	PQEnqueue(schedule->schedule, new);

	return TSKGetUid(new);
}

void SCDRun(scd_t *schedule)
{
	tsk_t *next = NULL;
	struct timeval current;
	void *destroy = NULL;

    assert(schedule);
	gettimeofday(&current, NULL);

    schedule->flag_stop = 0;

    while (schedule->flag_stop == 0)
    {
		next = PQPeek(schedule->schedule);
		gettimeofday(&current, NULL);
		if (current.tv_sec <= TSKGetCallTime(next).tv_sec)
		{/*task is ahead of current time*/
			if (current.tv_sec < TSKGetCallTime(next).tv_sec)
			{
				sleep(TSKGetCallTime(next).tv_sec - current.tv_sec);
			}
			TSKRun(next);
			TSKUpdateCallTime(next);
			if (0 != TSKGetCallTime(next).tv_sec)
			{
				PQDequeue(schedule->schedule);
				PQEnqueue(schedule->schedule, next);
			}
			else
			{
				destroy = PQPeek(schedule->schedule);
				PQDequeue(schedule->schedule);
				TSKDestroy(destroy);
			}
		}
		else
		{
			TSKUpdateCallTime(next);
			if (0 != TSKGetCallTime(next).tv_sec)
			{/*it has interval*/
				TSKRun(next);
				PQDequeue(schedule->schedule);
				PQEnqueue(schedule->schedule, next);
			}
			else
			{
				destroy = PQPeek(schedule->schedule);
				PQDequeue(schedule->schedule);
				TSKDestroy(destroy);
			}
		}
    }
}

void SCDStop(scd_t *schedule)
{
    assert(schedule);

    schedule->flag_stop = 1;
}

int SCDRemove(scd_t *schedule, uid_type uid)
{
	void *data = NULL;

    assert(schedule);

    data = PQErase(schedule->schedule, IsSameUID, &uid);
	if (data != NULL)
	{
		TSKDestroy(data);
		return 0;
	}
	return 1;
}
/******Schedule info**********************/

size_t SCDSize(scd_t *schedule)
{
    assert(schedule);

    return PQSize(schedule->schedule);
}

int SCDIsEmpty(scd_t *schedule)
{
    assert(schedule);

    return PQIsEmpty(schedule->schedule);
}

/*****SET schedule func**************************/
int SCDIsBefore(const void *data, const void *param)
{
	tsk_t *task1 = (tsk_t *) data;
	tsk_t *task2 = (tsk_t *) param;
	struct timeval time1 = TSKGetCallTime(task1);
	struct timeval time2 = TSKGetCallTime(task2);

    return (time1.tv_sec <= time2.tv_sec);
}

int IsSameUID(const void *data, const void *param)
{
	uid_type uid = {0};
	tsk_t *task = NULL;

	assert(param);
	assert(data);

	uid = *((const uid_type *) param);
	task = (tsk_t *) data;

	return !TSKIsMatch(TSKGetUid(task), uid);
}
