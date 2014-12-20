/*
 * Implement a simple task scheduler
 */
#include "avr-common.h"
#include "task.h"

#ifdef AVR_FEATURE_TASKS_MAX_SLOTS
# define MAX_TASKS   AVR_FEATURE_TASKS_MAX_SLOTS
#else
# define MAX_TASKS   4
#endif
 
#define TF_NONE     0x00
#define TF_VALID    0x01


typedef struct queued_task  queued_task;
struct queued_task
{
    uint8_t             flags;
    uint32_t            next_run;
    task_callback_t     *callback;
    uint16_t            retries;
    uint32_t            data;
};

static queued_task tasklist[MAX_TASKS];

void
task_init(void)
{
    for (uint8_t i = 0; i < MAX_TASKS; i++)
        tasklist[i].flags = TF_NONE;
}

void
task_submit(uint32_t next_run, uint16_t retries, task_callback_t *callback, uint32_t data)
{
    for (uint8_t i = 0; i < MAX_TASKS; i++)
    {
        queued_task *t  = &tasklist[i];

        if ((t->flags & TF_VALID) == 0)
        {
            t->next_run = next_run;
            t->callback = callback;
            t->retries = retries;
            t->data = data;
            t->flags |= TF_VALID;
            break;
        }
    }
}

void
task_run_ready(uint32_t now)
{
    for (uint8_t i = 0; i < MAX_TASKS; i++)
    {
        queued_task *t  = &tasklist[i];

        if (t->flags & TF_VALID && t->next_run < now)
        {
            uint32_t next = (*t->callback)(t->next_run, &t->data);

            if (t->retries != 1 && next != 0)
            {
                /* resubmit the task */
                t->next_run = next;

                if (t->retries != 0)
                    t->retries--;
            }
            else
            {
                /* delete this task */
                t->flags = TF_NONE;
            }
        }
    }
}
