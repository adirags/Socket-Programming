# Socket-Programming
This project involves the obtaining of a network map
remotely and using it to find the optimal path for routing.
This is done by having each of the four servers identifying
its neighbors and the costs associated in transmitting to
the neighbor. This neighbor information is then shared
with the client system. Thus the client gets the neighbor
information of each of the four servers. This transmission
of neighbor information is done over TCP with the client 
using a static port since it has to be known to each server
Once the client has the neighbor information of all servers
it can trivially find the network map and populate an
adjacency matrix denoting the connectivity and their
associated costs of the network.
This network map is then shared by the client with each of
the four servers. The transmission of this network map is
done over UDP.
Meanwhile, at the client side, the network map in the form
of the adjacency matrix is used to compute the optimal
routing path by finding the Minimum Spanning Tree.

References:
	www.beej.us/guide/bgnet (Overview of Socket Programming)
