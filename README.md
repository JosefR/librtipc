librtipc
========

IPC objects
-----------

* Mutex
* Barrier
* Flag
* Wait-free single-producer single-consumer queue
* Wait-free multi-producer single-consumer queue
* Lock-free multi-producer single-consumer queue
* Lock-free multi-producer multi-consumer queue
* 'Sensor-Buffer' (single overwriting writer, single-reader)

Implementation
--------------

This library implements these IPC objects for the x86 and the ARMv7 architecture. Due to the way atomic operations have to be implemented for the ARMv7 architecture, the ARMv7 implementation of the multi-producer queues and the 'Sensor-Buffer' object can only be considered beeing lock-free, although the rest of the algorithm fullfills the wait free characteristic.
