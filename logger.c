/* $Id: logger.c,v 1.19 2001/09/03 10:06:35 jorge Exp $ */

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#include "logger.h"
#include "task.h"
#include "job.h"
#include "computer.h"

int loglevel = L_WARNING;
int logonscreen = 0;

void log_slave_task (struct task *task,int level,char *msg)
{
  FILE *f_log;
  char name[MAXNAMELEN];
  char buf[BUFFERLEN];
  time_t now;
  
  if (level > loglevel)
    return;

  if (gethostname (name,MAXNAMELEN-1) == -1) {
    strcpy (name,"UNKNOWN");
  }

  time (&now);
  strncpy (buf,ctime(&now),BUFFERLEN-1);
  buf[strlen(buf)-1] = '\0';

  if (!logonscreen) {
    f_log = log_slave_open_task (task);
    if (!f_log)
      f_log = stdout;
  } else {
    f_log = stdout;
  }
  
  if (loglevel < L_DEBUG)
    fprintf (f_log,"%8s : %8s -> Job: %8s || Owner: %8s || Frame: %4i || %s: %s\n",buf,name,
	     task->jobname,task->owner,task->frame,log_level_str(level),msg);
  else
    fprintf (f_log,"%8s : %8s -> Job: %8s || Owner: %8s || Frame: %4i || (%i) %s: %s\n",buf,name,
	     task->jobname,task->owner,task->frame,(int)getpid(),log_level_str(level),msg);

  if (!logonscreen)
    fclose (f_log);
}

FILE *log_slave_open_task (struct task *task)
{
  FILE *f;
  char filename[BUFFERLEN];
  char dir[BUFFERLEN];
  char *basedir;

  if ((basedir = getenv("DRQUEUE_ROOT")) == NULL) {
    fprintf (stderr,"Environment variable DRQUEUE_ROOT not set. Aborting...\n");
    kill(0,SIGINT);
  }

  snprintf(dir,BUFFERLEN-1,"%s/logs/%s",basedir,task->jobname);
  snprintf(filename,BUFFERLEN-1,"%s/%s.log",dir,task->jobname);
  if ((f = fopen (filename, "a")) == NULL) {
    if (errno == ENOENT) {
      /* If its because the directory does not exist we try creating it first */
      if (mkdir (dir,0775) == -1) {
	perror ("log_slave_open_task: Couldn't create directory for task logging");
	fprintf (stderr,"So... logging on screen.\n");
	logonscreen = 1;
	return f;
      }
      if ((f = fopen (filename, "a")) == NULL) {
	perror ("log_slave_open_task: Couldn't open file for writing.");
	fprintf (stderr,"So... logging on screen.\n");
	logonscreen = 1;
	return f;
      }
    }
    perror ("log_slave_open_task");
    fprintf (stderr,"So... logging on screen.\n");
    logonscreen = 1;
  }
  
  return f;
}

void log_slave_computer (int level, char *msg)
{
  FILE *f_log;
  char name2[MAXNAMELEN];
  char *name = NULL;		/* To only make a call to gethostname */
  char buf[BUFFERLEN];
  time_t now;
  
  if (level > loglevel)
    return;

  time (&now);
  strncpy (buf,ctime(&now),BUFFERLEN-1);
  buf[strlen(buf)-1] = '\0';

  if (name == NULL) {
    if (gethostname(name2,MAXNAMELEN-1) == -1) {
      strcpy (name2,"UNKNOWN");
    }
    name = name2;
  }

  if (!logonscreen) {
    f_log = log_slave_open_computer (name);
    if (!f_log)
      f_log = stdout;
  } else {
    f_log = stdout;
  }

  if (loglevel < L_DEBUG)
    fprintf (f_log,"%8s : %s: %s\n",buf,log_level_str(level),msg);
  else 
    fprintf (f_log,"%8s : (%i) %s: %s\n",buf,(int) getpid(), log_level_str(level),msg);

  if (!logonscreen)
    fclose (f_log);
}

FILE *log_slave_open_computer (char *name)
{
  FILE *f;
  char filename[BUFFERLEN];
  char *basedir;

  if ((basedir = getenv("DRQUEUE_ROOT")) == NULL) {
    fprintf (stderr,"Environment variable DRQUEUE_ROOT not set. Aborting...\n");
    kill(0,SIGINT);
  }

  snprintf(filename,BUFFERLEN-1,"%s/logs/%s.log",basedir,name);
  
  if ((f = fopen (filename,"a")) == NULL) {
    perror ("log_slave_open_computer: Couldn't open file for writing");
    fprintf (stderr,"So... logging on screen.\n");
    logonscreen = 1;
  }

  return f;
}

void log_master_job (struct job *job, int level, char *msg)
{
  FILE *f_log;
  char buf[BUFFERLEN];
  time_t now;
  
  if (level > loglevel)
    return;

  time (&now);
  strncpy (buf,ctime(&now),BUFFERLEN-1);
  buf[strlen(buf)-1] = '\0';

  if (!logonscreen) {
    f_log = log_master_open ();
    if (!f_log)
      f_log = stdout;
  } else {
    f_log = stdout;
  }
  
  if (loglevel < L_DEBUG)
    fprintf (f_log,"%8s : Job: %8s || Owner: %8s || %s: %s\n",buf,job->name,job->owner,log_level_str(level),msg);
  else
    fprintf (f_log,"%8s : Job: %8s || Owner: %8s || (%i) %s: %s\n",buf,job->name,job->owner,(int)getpid(),
	     log_level_str(level),msg);
  if (!logonscreen)
    fclose (f_log);
}

void log_master_computer (struct computer *computer, int level, char *msg)
{
  FILE *f_log;
  char buf[BUFFERLEN];
  time_t now;
  
  if (level > loglevel)
    return;

  time (&now);
  strncpy (buf,ctime(&now),BUFFERLEN-1);
  buf[strlen(buf)-1] = '\0';

  if (!logonscreen) {
    f_log = log_master_open ();
    if (!f_log)
      f_log = stdout;
  } else {
    f_log = stdout;
  }

  if (loglevel < L_DEBUG)
    fprintf (f_log,"%8s : Computer: %8s || %s: %s\n",buf,computer->hwinfo.name,log_level_str(level),msg);
  else
    fprintf (f_log,"%8s : Computer: %8s || (%i) %s: %s\n",buf,computer->hwinfo.name,(int) getpid(),log_level_str(level),msg);

  if (!logonscreen)
    fclose (f_log);
}

void log_master (int level,char *msg)
{
  FILE *f_log;
  char buf[BUFFERLEN];
  time_t now;
  
  if (level > loglevel)
    return;

  time (&now);
  strncpy (buf,ctime(&now),BUFFERLEN-1);
  buf[strlen(buf)-1] = '\0';

  if (!logonscreen) {
    f_log = log_master_open ();
    if (!f_log)
      f_log = stdout;
  } else {
    f_log = stdout;
  }
  
  if (loglevel < L_DEBUG)
    fprintf (f_log,"%8s : %s: %s\n",buf,log_level_str(level),msg);
  else
    fprintf (f_log,"%8s : (%i) %s: %s\n",buf,(int)getpid(),log_level_str(level),msg);

  if (!logonscreen)
    fclose (f_log);
}

FILE *log_master_open (void)
{
  FILE *f;
  char filename[BUFFERLEN];
  char *basedir;

  if ((basedir = getenv("DRQUEUE_ROOT")) == NULL) {
    fprintf (stderr,"Environment variable DRQUEUE_ROOT not set. Aborting...\n");
    kill(0,SIGINT);
  }

  snprintf(filename,BUFFERLEN-1,"%s/logs/master.log",basedir);
#ifdef __LINUX
  if ((f = fopen (filename,"a")) == NULL) {
    perror ("log_master_open: Couldn't open file for writing");
    fprintf (stderr,"So... logging on screen.\n");
    logonscreen = 1;
  }
#else
  if ((f = fopen (filename,"ab")) == NULL) {
    perror ("log_master_open: Couldn't open file for writing");
    fprintf (stderr,"So... logging on screen.\n");
    logonscreen = 1;
  }
#endif 

  return f;
}

char *log_level_str (int level)
{
  char *msg;

  switch (level) {
  case L_ERROR:
    msg = "ERROR";
    break;
  case L_WARNING:
    msg = "Warning";
    break;
  case L_INFO:
    msg = "Info";
    break;
  case L_DEBUG:
    msg = "Debug";
    break;
  default:
    msg = "UNKNOWN";
  }

  return msg;
}

int log_dumptask_open (struct task *t)
{
  int lfd;
  char filename[BUFFERLEN];
  char dir[BUFFERLEN];
  char *basedir;
  char name[MAXNAMELEN];
  time_t tm;

  if ((basedir = getenv("DRQUEUE_ROOT")) == NULL) {
    fprintf (stderr,"Environment variable DRQUEUE_ROOT not set. Aborting...\n");
    kill(0,SIGINT);
  }

  snprintf(dir,BUFFERLEN-1,"%s/logs/%s",basedir,t->jobname);
  snprintf(filename,BUFFERLEN-1,"%s/%s.%04i",dir,t->jobname,t->frame);
  if ((lfd = open (filename, O_CREAT|O_APPEND|O_RDWR, 0664)) == -1) {
    if (errno == ENOENT) {
      /* If its because the directory does not exist we try creating it first */
      if (mkdir (dir,0775) == -1) {
	log_slave_task (t,L_ERROR,"Couldn't create directory for task logs");
	return -1;
      }
      if ((lfd = open (filename, O_CREAT|O_APPEND|O_RDWR, 0664)) == -1) {
	log_slave_task (t,L_ERROR,"Couldn't open file for task log");
	return -1;
      }
    }
  }
  
  time (&tm);
  gethostname (name,MAXNAMELEN-1);
  sprintf(filename,"Log started at %sComputer: %s\n\n",ctime(&tm),name);
  write(lfd,filename,strlen(filename));
  return lfd;
}

int log_dumptask_open_ro (struct task *t)
{
  /* Open in read only for clients */
  int lfd;
  char filename[BUFFERLEN];
  char dir[BUFFERLEN];
  char *basedir;

  if ((basedir = getenv("DRQUEUE_ROOT")) == NULL) {
    fprintf (stderr,"Environment variable DRQUEUE_ROOT not set. Aborting...\n");
    kill(0,SIGINT);
  }

  snprintf(dir,BUFFERLEN-1,"%s/logs/%s",basedir,t->jobname);
  snprintf(filename,BUFFERLEN-1,"%s/%s.%04i",dir,t->jobname,t->frame);
  if ((lfd = open (filename,O_RDONLY)) == -1) {
    return -1;
  }
  
  return lfd;
}
