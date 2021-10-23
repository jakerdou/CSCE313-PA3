#include "common.h"
#include "SHMreqchannel.h"
using namespace std;
/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR FOR CLASS   R e q u e s t C h a n n e l  */
/*--------------------------------------------------------------------------*/

SHMRequestChannel::SHMRequestChannel(const string _name, const Side _side, int _len) : RequestChannel(_name, _side) {
	s1 = "/SHM_" + my_name + "1";
	s2 = "/SHM_" + my_name + "2";
	len = _len;

	shmq1 = new SHMQ(s1, len);
	shmq2 = new SHMQ(s2, len);

	if (my_side == CLIENT_SIDE)
	{
		swap(shmq1, shmq2);
	}
	
		
	// if (_side == SERVER_SIDE){
	// 	wfd = open_ipc(s1, O_RDWR | O_CREAT);
	// 	rfd = open_ipc(s2, O_RDWR | O_CREAT);
	// }
	// else{
	// 	cout << "in client side opening\n";
	// 	rfd = open_ipc(s1, O_RDWR | O_CREAT);
	// 	cout << "done with first openipc\n";
	// 	wfd = open_ipc(s2, O_RDWR | O_CREAT);
	// }
	
}

SHMRequestChannel::~SHMRequestChannel(){ 
	delete shmq1;
	delete shmq2;
}

// int SHMRequestChannel::open_ipc(string _pipe_name, int mode){
// 	cout << "in openipc\n";
// 	int fd = (int) mq_open(_pipe_name.c_str(), O_RDWR | O_CREAT, 0600, 0);
// 	cout << "just ran mq_open\n";
// 	if (fd < 0){
// 		EXITONERROR(_pipe_name);
// 	}
// 	return fd;
// }

int SHMRequestChannel::cread(void* msgbuf, int bufcapacity){
	return shmq1->my_shm_recv (msgbuf, bufcapacity);
}

int SHMRequestChannel::cwrite(void* msgbuf, int len){
	return shmq2->my_shm_send (msgbuf, len);
}

