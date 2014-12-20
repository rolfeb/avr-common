#ifndef __INCLUDE_TASK_H
#define __INCLUDE_TASK_H

#include <stdint.h>

typedef uint32_t    (task_callback_t)(uint32_t now, uint32_t *data);

extern void
task_init(void);

extern void
task_submit(uint32_t next_run, uint16_t retries, task_callback_t *callback, uint32_t data);

extern void
task_run_ready(uint32_t now);

#endif /* __INCLUDE_TASK_H */
