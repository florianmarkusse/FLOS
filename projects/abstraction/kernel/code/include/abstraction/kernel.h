#ifndef ABSTRACTION_KERNEL_H
#define ABSTRACTION_KERNEL_H

// NOTE: If this grows out of control, we can collapse this into a single
// archInit function that takes a stage and a void* to arguments and take it
// from there.
void archInit(void *archInit);

#endif
