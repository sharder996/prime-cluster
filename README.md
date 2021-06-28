# prime-cluster

A computer cluster written in Python that will exhaustively find increasingly larger prime numbers.

The server will actively listen for connections on a specified port and once a slave node has connected, a task will be delegated to it. The cluster operates with socket communication and sends messages with simple objects that can be easily converted to `JSON`. Since the cluster will find all primes a considerably large amount of data is generated from a single task and so the current limiting factor is the network connection speed between a master and a slave node as well as the rate at which results can be written to data files.

Future revisions could see improvement in how the master and slave communicate as to reduce the strain on the network connection. Alternatively, the cluster could be easily converted to find the largest prime number within a range and not all prime numbers within the range.
