//
// Copyright (C) 2001,2002,2003,2004,2005,2006 Jorge Daza Garcia-Blanes
//
// This file is part of DrQueue
//
// DrQueue is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// DrQueue is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
// USA
//
// $Id$
//

#include "common.h"
#include "drerrno.h"
#include "constants.h"
#include "logger.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

int common_environment_check (void) {
  /* This function checks the environment AND the directory structure */
  char *buf;
  struct stat s_stat;
  char dir_str[BUFFERLEN];

  if ((buf = getenv("DRQUEUE_MASTER")) == NULL) {
    drerrno = DRE_NOENVMASTER;
    return 0;
  }

  if ((buf = getenv("DRQUEUE_ROOT")) == NULL) {
    drerrno = DRE_NOENVROOT;
    return 0;
  }

  if (getenv("DRQUEUE_TMP") != NULL) {
    snprintf (dir_str,BUFFERLEN-1,"%s",getenv("DRQUEUE_TMP"));
  } else {
    snprintf (dir_str,BUFFERLEN-1,"%s/tmp",getenv("DRQUEUE_ROOT"));
  } 
  if (stat (dir_str,&s_stat) == -1) {
    drerrno = DRE_NOTMPDIR;
    return 0;
  } else {
    if ((!S_ISDIR(s_stat.st_mode)) || (!(S_IWOTH & s_stat.st_mode))) {
      drerrno = DRE_NOTMPDIR;
      return 0;
    }
  }

  if (getenv("DRQUEUE_DB") != NULL) {
    snprintf (dir_str,BUFFERLEN-1,"%s",getenv("DRQUEUE_DB"));
  } else {
    snprintf (dir_str,BUFFERLEN-1,"%s/db",getenv("DRQUEUE_ROOT"));
  } 
  if (stat (dir_str,&s_stat) == -1) {
    drerrno = DRE_NODBDIR;
    return 0;
  } else if ((!S_ISDIR(s_stat.st_mode)) || (!(S_IWUSR & s_stat.st_mode))) {
    drerrno = DRE_NODBDIR;
    return 0;
  }

  if (getenv("DRQUEUE_LOGS")) {
    snprintf (dir_str,BUFFERLEN-1,"%s",getenv("DRQUEUE_LOGS"));
  } else {
    snprintf (dir_str,BUFFERLEN-1,"%s/logs",getenv("DRQUEUE_ROOT"));
  } 
  if (stat (dir_str,&s_stat) == -1) {
    drerrno = DRE_NOLOGDIR;
    return 0;
  } else if ((!S_ISDIR(s_stat.st_mode)) || (!(S_IWUSR & s_stat.st_mode))) {
    drerrno = DRE_NOLOGDIR;
    return 0;
  }

  if (getenv("DRQUEUE_BIN")) {
    snprintf (dir_str,BUFFERLEN-1,"%s",getenv("DRQUEUE_BIN"));
  } else {
    snprintf (dir_str,BUFFERLEN-1,"%s/bin",getenv("DRQUEUE_ROOT"));
  } 
  if (stat (dir_str,&s_stat) == -1) {
    drerrno = DRE_NOBINDIR;
    return 0;
  }

  if (getenv("DRQUEUE_ETC")) {
    snprintf (dir_str,BUFFERLEN-1,"%s",getenv("DRQUEUE_ETC"));
  } else {
    snprintf (dir_str,BUFFERLEN-1,"%s/etc",getenv("DRQUEUE_ROOT"));
  } 
  if (stat (dir_str,&s_stat) == -1) {
    drerrno = DRE_NOETCDIR;
    return 0;
  }

  return 1;
}

void show_version (char **argv) {
  printf ("\nDistributed Rendering Queue\n");
  printf ("by Jorge Daza Garcia Blanes\n\n");
  printf ("%s version: %s\n",argv[0],VERSION);
  printf ("\n");
}

int rmdir_check_str (char *path) {
  // This function should test a path's validity
  // So we don't pass a wrong path to remove_dir by mistake

  if (strstr(path,"/../") != NULL)
    return 1;
  if (strchr(path,'"') != NULL)
    return 1;
  if (strchr(path,'\'') != NULL)
    return 1;

  return 0;
}

int remove_dir (char *dir) {
  /* Removes a directory recursively */
  char cmd[BUFFERLEN];

  if (rmdir_check_str (dir)) // Check the path. If wrong don't remove
    return 0;

  snprintf (cmd,BUFFERLEN,"rm -fR '%s'",dir);

  fprintf (stderr,"WARN: I was going to remove the logs directory with the command: %s\n",cmd);
  fprintf (stderr,"WARN: This 'feature' has been removed. You'll have to remove old log directories yourself by now.\n");
  return 0;
}

void mn_job_finished (struct job *job) {
  FILE *mail;
  char command[BUFFERLEN];
  uint32_t total;
  char total_str[BUFFERLEN];

  snprintf (command,BUFFERLEN-1,"Mail -s 'Your job (%s) is finished' %s",job->name,job->email);

  if ((mail = popen (command,"w")) == NULL) {
    /* There was a problem creating the piped command */
    return;
  }

  total = job->avg_frame_time*job_nframes(job);
  snprintf (total_str,BUFFERLEN-1,"%s",time_str(total));

  fprintf (mail,"\n"
           "Total number of frames: %i\n"
           "The average time per frame was: %s\n"
           "And the total rendering time: %s\n"
           "\n.\n\n",job_nframes(job),time_str(job->avg_frame_time),total_str);

  pclose (mail);
}

char *time_str (uint32_t nseconds) {
  static char msg[BUFFERLEN];

  if ((nseconds / 3600) > 0) {
    snprintf(msg,BUFFERLEN-1,"%i hours %i minutes %i seconds",
             nseconds / 3600,
             (nseconds % 3600) / 60,
             (nseconds % 3600) % 60);
  } else if ((nseconds / 60) > 0) {
    snprintf(msg,BUFFERLEN-1,"%i minutes %i seconds",
             nseconds / 60,
             nseconds % 60);
  } else {
    snprintf(msg,BUFFERLEN-1,"%i seconds",nseconds);
  }

  return msg;
}

int common_date_check (void) {
  time_t now;
  struct tm *tm_now;

  time(&now);
  tm_now = localtime (&now);

  /*  fprintf (stderr,"Date: %i.%i.%u\n",tm_now->tm_mday,tm_now->tm_mon,tm_now->tm_year); */
  if (((tm_now->tm_mon >= 4) && (tm_now->tm_year == 102))
      || (tm_now->tm_year > 102))
    return 0;

  return 1;
}

void set_default_env(void) {
  char *penv,renv[BUFFERLEN],*drq_root;

  if ((drq_root = getenv("DRQUEUE_ROOT")) == NULL) {
    drerrno = DRE_NOENVROOT;
    log_auto (L_ERROR,"Environment variable DRQUEUE_ROOT is not set. Please set your environment "
	      "variables properly. DRQUEUE_ROOT should point to the base path of DrQueue's "
	      "installation. Check the documentation for more help.");
    exit (1);
  }

  if (!getenv("DRQUEUE_TMP")) {
    snprintf(renv,BUFFERLEN,"DRQUEUE_TMP=%s/tmp",drq_root);
    penv = (char*) malloc (strlen(renv)+1);
    strncpy(penv,renv,strlen(renv)+1);
    putenv(penv);
  }

  if (!getenv("DRQUEUE_ETC")) {
    snprintf(renv,BUFFERLEN,"DRQUEUE_ETC=%s/etc",drq_root);
    penv = (char*) malloc (strlen(renv)+1);
    strncpy(penv,renv,strlen(renv)+1);
    putenv(penv);
  }

  if (!getenv("DRQUEUE_BIN")) {
    snprintf(renv,BUFFERLEN,"DRQUEUE_BIN=%s/bin",drq_root);
    penv = (char*) malloc (strlen(renv)+1);
    strncpy(penv,renv,strlen(renv)+1);
    putenv(penv);
  }

  if (!getenv("DRQUEUE_LOGS")) {
    snprintf(renv,BUFFERLEN,"DRQUEUE_LOGS=%s/logs",drq_root);
    penv = (char*) malloc (strlen(renv)+1);
    strncpy(penv,renv,strlen(renv)+1);
    putenv(penv);
  }

  if (!getenv("DRQUEUE_DB")) {
    snprintf(renv,BUFFERLEN,"DRQUEUE_DB=%s/db",drq_root);
    penv = (char*) malloc (strlen(renv)+1);
    strncpy(penv,renv,strlen(renv)+1);
    putenv(penv);
  }
}


