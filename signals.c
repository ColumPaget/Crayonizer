#include "signals.h"
#include <sys/ioctl.h>
#include <signal.h>
#include "status_bar.h"


void HandleSigwinch(STREAM *Pipe)
{
    struct winsize w;
    char *Tempstr=NULL;

    ioctl(0, TIOCGWINSZ, &w);

    ScreenRows=w.ws_row;
    ScreenCols=w.ws_col;
    if (GlobalFlags & HAS_STATUSBAR) SetupStatusBars(NULL, NULL);
    else
    {
//		w.ws_row--;

        if (Pipe) ioctl(Pipe->out_fd, TIOCSWINSZ, &w);
    }
    Destroy(Tempstr);
}


void HandleSignal(int sig)
{
    int wid, len;

    if (sig==SIGWINCH) GlobalFlags |= GOT_SIGWINCH;
    if (sig==SIGTERM) GlobalFlags |= GOT_SIGTERM;
    if (sig==SIGHUP) GlobalFlags |= GOT_SIGTERM;
    if (sig==SIGINT) GlobalFlags |= GOT_SIGINT;
}



void PropagateSignals(STREAM *Pipe)
{
    int PeerPID;

    if (! Pipe) return;

    PeerPID=atoi(STREAMGetValue(Pipe,"PeerPID"));
    if (GlobalFlags & GOT_SIGWINCH)
    {
        HandleSigwinch(Pipe);
        kill(PeerPID,SIGWINCH);
    }

    if (GlobalFlags & GOT_SIGTERM) kill(PeerPID,SIGTERM);
    if (GlobalFlags & GOT_SIGINT) kill(PeerPID,SIGINT);
    GlobalFlags &= ~(GOT_SIGWINCH | GOT_SIGTERM | GOT_SIGINT);
}


void SetupSignals()
{
    signal(SIGHUP, HandleSignal);
    signal(SIGTERM, HandleSignal);
    signal(SIGINT, HandleSignal);
    signal(SIGWINCH, HandleSignal);
}


