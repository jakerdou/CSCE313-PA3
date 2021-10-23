/*
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date  : 2/8/20
 */
#include "common.h"
#include "FIFOreqchannel.h"
#include "MQreqchannel.h"
#include "SHMreqchannel.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <sys/time.h>
#include <sys/wait.h>

using namespace std;

int main(int argc, char *argv[]){
    vector<RequestChannel*> channels;

    /* GET INPUT ARGS */
    int person = -1;
    double time = -1;
    int ecgno = -1;
    bool create_channel = false;
    char* filename = "";
    int buffercapacity = MAX_MESSAGE;
    char* serverBufferCap = "";
    string ipcmethod = "f";
    int nchannels = 1;


    int arg;
    while ((arg = getopt (argc, argv, "p:t:e:c:f:m:i:")) != -1) {
        switch (arg) {
            case 'p':
                person = atoi(optarg);
                break;
            case 't':
                time = atof(optarg);
                break;
            case 'e':
                ecgno = atoi(optarg);
                break;
            case 'c':
                create_channel = true;
                nchannels = atoi(optarg);
                break;
            case 'f':
                filename = optarg;
                break;
            case 'm':
                buffercapacity = atoi(optarg);
                serverBufferCap = optarg;
                break;
            case 'i':
                ipcmethod = optarg;
                break;
        }
    }

    /* START CHILD PROCESS */
    if (fork() == 0)
    {
        char* argvServer[6];
        argvServer[0] = "./server";

        if (serverBufferCap != "")
        {
            argvServer[1] = "-m";
            argvServer[2] = serverBufferCap;
            argvServer[3] = "-i";
            argvServer[4] = (char*) ipcmethod.c_str();
            argvServer[5] = NULL;
        }
        else {
            argvServer[1] = "-i";
            argvServer[2] = (char*) ipcmethod.c_str();
            argvServer[3] = NULL;
        }
        
        execvp("./server", argvServer);
    }

    RequestChannel* control_chan = NULL;

    if (ipcmethod == "f") {
        control_chan = new FIFORequestChannel ("control", FIFORequestChannel::CLIENT_SIDE);
        channels.push_back(control_chan);
    }
    else if (ipcmethod == "q") {
        control_chan = new MQRequestChannel ("control", MQRequestChannel::CLIENT_SIDE, buffercapacity);
        channels.push_back(control_chan);
    }
    else if (ipcmethod == "m") {
        control_chan = new SHMRequestChannel ("control", SHMRequestChannel::CLIENT_SIDE, buffercapacity);
        channels.push_back(control_chan);
    }
    

    char buf [buffercapacity]; 

    /* REQUESTING A NEW CHANNEL */
    if (create_channel) {
        RequestChannel* chan = control_chan;
        for (int i = 0; i < nchannels; i++)
        {
            MESSAGE_TYPE newChannel = NEWCHANNEL_MSG;
            control_chan->cwrite (&newChannel, sizeof (MESSAGE_TYPE));
            control_chan->cread(buf, buffercapacity);
            char* chan2Name = buf;

            if (ipcmethod == "f") {
                chan = new FIFORequestChannel (chan2Name, FIFORequestChannel::CLIENT_SIDE);
                channels.push_back(chan);
            }
            else if (ipcmethod == "q") {
                chan = new MQRequestChannel (chan2Name, MQRequestChannel::CLIENT_SIDE, buffercapacity);
                channels.push_back(chan);
            }
            else if (ipcmethod == "m") {
                chan = new SHMRequestChannel (chan2Name, SHMRequestChannel::CLIENT_SIDE, buffercapacity);
                channels.push_back(chan);
            }
        }
        
        // TESTING NEW CHANNEL
        // for (int p = 1; p < 6; p++) {
        //     datamsg* y = new datamsg (p, 0, 1);
        //     chan->cwrite (y, sizeof (datamsg));
        //     chan->cread (buf, buffercapacity);
        //     double reply2 = *(double *) buf;
            
        //     cout << "channel2 reply (p=" << p << " t=0 e=0" << "): " << reply2 << endl;

        //     delete y;
        // }

        // MESSAGE_TYPE n = QUIT_MSG;
        // chan->cwrite (&n, sizeof (MESSAGE_TYPE));
    }

    /* REQUESTING ONE DATA POINT */
    if (person > 0 && time >= 0 && ecgno > 0)
    {
        for (int i = 0; i < channels.size(); i++)
        {
            RequestChannel* curr_channel = channels.at(i);
            datamsg* x = new datamsg (person, time, ecgno);
        
            curr_channel->cwrite (x, sizeof (datamsg));
            
            int nbytes = curr_channel->cread (buf, buffercapacity);
            double reply = *(double *) buf;
            
            cout << curr_channel->name() << " reply (p=" << person << " t=" << time << " e=" << ecgno << "): " << reply << endl;

            delete x;
        }
    }
    
    /* REQUESTING 1,000 DATA POINTS */
    if (person > 0 && time == -1 && ecgno > 0)
    {
        fstream xOne, xOneTime;
        xOne.open("received/x1.csv", fstream::out);
        xOneTime.open("received/x1Time.csv", fstream::out);        

        struct timeval currentTime;
        
        double t;
        int curr_channel_idx = 0;

        struct timeval timeBefore, timeAfter;
        gettimeofday(&timeBefore, NULL);
        int timeBeforeUs = timeBefore.tv_sec * 1000000 + timeBefore.tv_usec;

        for(int i = 0; i < 1000; i++) {
            if (i == 0) {
                t = 0;
            }
            else {
                t = t + 0.004;
            }

            RequestChannel* curr_channel = channels.at(curr_channel_idx);
            // cout << "here in 1000: " << curr_channel->name() << endl;
            datamsg* x = new datamsg (person, t, ecgno);
            curr_channel->cwrite (x, sizeof (datamsg));
            curr_channel->cread (buf, buffercapacity);
            double reply = *(double *) buf;

            xOne << reply << "\n";

            gettimeofday(&currentTime, NULL);
            int currentTimeUs = currentTime.tv_sec * 1000000 + currentTime.tv_usec;
            xOneTime << currentTimeUs << "\n";

            delete x;

            if (curr_channel_idx == channels.size() - 1)
            {
                curr_channel_idx = 0;
            }
            else
            {
                curr_channel_idx++;
            }
        }

        gettimeofday(&timeAfter, NULL);
        int timeAfterUs = timeAfter.tv_sec * 1000000 + timeAfter.tv_usec;
        cout << "Time to get 1K data points: " << timeAfterUs - timeBeforeUs << " us\n";

        xOne.close();
        xOneTime.close();
        cout << "data has been written to x1.csv\n";
    }

    /* REQUESTING A FILE */
    if (filename != "")
    {
        string filenameStr = filename;
        filemsg fm(0, 0);
        char fileBuffer [sizeof(filemsg) + filenameStr.size() + 1];
        memcpy (fileBuffer, &fm, sizeof(filemsg));
        memcpy (fileBuffer + sizeof(filemsg), filenameStr.c_str(), filenameStr.size() + 1);

        control_chan->cwrite (fileBuffer, sizeof(filemsg) + filenameStr.size() + 1);

        __int64_t filelen;
        control_chan->cread(&filelen, sizeof(__int64_t));
        __int64_t bytesToConsume = filelen;

        ofstream outputFile("received/" + filenameStr, fstream::out);

        struct timeval timeBefore, timeAfter;
        gettimeofday(&timeBefore, NULL);
        int timeBeforeUs = timeBefore.tv_sec * 1000000 + timeBefore.tv_usec;
        
        int curr_channel_idx = 0;
        while (bytesToConsume > 0)
        {
            RequestChannel* curr_channel = channels.at(curr_channel_idx);
            // cout << "here: " << curr_channel->name() << endl;
            __int64_t offset = filelen - bytesToConsume;
            int length = buffercapacity;

            if (bytesToConsume >= buffercapacity)
            {
                bytesToConsume = bytesToConsume - buffercapacity;
            }
            else
            {
                length = bytesToConsume;
                bytesToConsume = 0;
            }
            filemsg fm(offset, length);
            char fileBuffer [sizeof(filemsg) + filenameStr.size() + 1];

            memcpy (fileBuffer, &fm, sizeof(filemsg));
            memcpy (fileBuffer + sizeof(filemsg), filenameStr.c_str(), filenameStr.size() + 1);

            curr_channel->cwrite (fileBuffer, sizeof(filemsg) + filenameStr.size() + 1);

            char outputBuffer[length]; 

            curr_channel->cread(outputBuffer, length);
            outputFile.write((char*) &outputBuffer, sizeof(outputBuffer));

            if (curr_channel_idx == channels.size() - 1)
            {
                curr_channel_idx = 0;
            }
            else
            {
                curr_channel_idx++;
            }
        }

        gettimeofday(&timeAfter, NULL);
        int timeAfterUs = timeAfter.tv_sec * 1000000 + timeAfter.tv_usec;
        cout << "Time to transfer " << filenameStr << " with buffer of " << buffercapacity << ": " << timeAfterUs - timeBeforeUs << " us\n";


        outputFile.close();
    }
    
    // closing the channel
    MESSAGE_TYPE m = QUIT_MSG;
    for (int i = 0; i < channels.size(); i++)
    {
        RequestChannel* curr_channel = channels.at(i);
        curr_channel->cwrite (&m, sizeof (MESSAGE_TYPE));
        delete curr_channel;
    }
    
    wait(&arg);
}
