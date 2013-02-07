Simply enough, it's (basic) threads implemented on the Cortex M3 platform. It implements a basic
round-robin TDB scheduler in pure C, using the program stack pointer (PSP) mechanism in the
Cortex M3 platform.

This requires the CMSIS libraries to build, and has been tested using CodeSourcery compilers.

First, call `thread_init` to init the system. `quantaUs` specifies the time quanta in microseconds. Create a thread
using `thread_create(fnc)` and kill it using `thread_kill`. Finally, call `thread_go` to set 
everything in motion. **`thread_go` does not return** - instead, use main as a bootstrap and
use the first registered thread an your master routine.

A thread returning from its routine will cause a hard fault, this will be fixed in future.
