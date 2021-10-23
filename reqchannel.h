
#ifndef _reqchannel_H_
#define _reqchannel_H_

#include "common.h"

class RequestChannel
{
public:
	enum Side {SERVER_SIDE, CLIENT_SIDE};
	enum Mode {READ_MODE, WRITE_MODE};
	
protected:
	/*  The current implementation uses named pipes. */
	
	
	string my_name;
	Side my_side;
	
	int wfd;
	int rfd;
	
	string s1, s2;
	int open_ipc(string _ipc_name, int mode){;};
	
public:
	RequestChannel(const string _name, const Side _side): my_name(_name), my_side(_side){}
	

	virtual ~RequestChannel(){};

	virtual int cread (void* msgbuf, int bufcapacity) = 0;
	/* Blocking read of data from the channel. You must provide the address to properly allocated
	memory buffer and its capacity as arguments. The 2nd argument is needed because the recepient 
	side may not have as much capacity as the sender wants to send.
	
	In reply, the function puts the read data in the buffer and  
	returns an integer that tells how much data is read. If the read fails, it returns -1. */
	
	virtual int cwrite (void *msgbuf , int msglen) = 0;
	/* Writes msglen bytes from the msgbuf to the channel. The function returns the actual number of 
	bytes written and that can be less than msglen (even 0) probably due to buffer limitation (e.g., the recepient
	cannot accept msglen bytes due to its own buffer capacity. */
	 
	string name() {
		return my_name;
	}; 
};

#endif
