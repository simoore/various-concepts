# Parallel Program Design

## Partioning

Decompose the problem at hand into smaller parts. You can partion the data associated
with the problem into multiple sub-elements (Domain decomposition).
Functional decomposition: divides computation of a problem into subtasks.

## Communication

How to share data and synchronize between tasks.

## Agglomeration

Combine task to take into account the hardware performing the computations.
Granulaity is the ratio of computation/communication. For a large amount of 
tasks, there may be a significiant amount if communication that takes up a 
significant amount of the processing time. Reduce the granularity to dedicate
more time to computation by using less tasks. If the granularity is too course
then long-running tasks may prevent other tasks from running. Make granularity a
parameter that can be adjusted to find the optimal granularity.

## Mapping

Where each task will execute. Not a consideration for single machine with multiple
cores. The OS handles this for us. For distributed computing this is not the case.
Goal: minimize total execution time.
Strategy 1) Place tasks that can execute concurrently on differenet processors.
Strategy 2) Place tasks that communicate with each other on the same processor.
