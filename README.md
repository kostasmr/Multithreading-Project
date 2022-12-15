# Multithreading-Project
Implementing multithreading on a storage engine data.

Processing of command line arguments is done in the kiwi-source/bench/bench.c file.
In the kiwi-source/bench/bench.h we define the total costs of the writers,readers and two locks that we will explain
below their use.

Ιn order to have multithreading, changes had to be made to kiwi-source/bench/kiwi.c.

Then, to support simultaneous operations of add and get for readers and writers, we had to change the kiwi-source/engine/db.c, kiwi-source/engine/db.h.
The code that helps to have multithreading, have comments so it is easy to understand it.

This project can run only in version 10 (“Buster”) of Debian Linux 64-bit as we run it in the report.
