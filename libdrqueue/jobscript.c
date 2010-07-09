//
// Copyright (C) 2001,2002,2003,2004,2005,2006 Jorge Daza Garcia-Blanes
// Copyright (C) 2010 Andreas Schroeder
//
// This file is part of DrQueue
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
// USA
//
#include <stdio.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "drq_stat.h"
#include "libdrqueue.h"

/* create a new script file */
struct jobscript_info *
jobscript_new (jobscript_type type, char *filename) {
  FILE *f = NULL;
  struct jobscript_info *jsi;

  if ( (type != JOBSCRIPT_TCSH) && (type != JOBSCRIPT_PYTHON) ) {
    fprintf (stderr,"ERROR: Job type requested unknown (%i)\n",type);
    fprintf (stderr,"ERROR: %s\n",drerrno_str());
    return NULL;
  }

  if ((f = fopen (filename, "a")) == NULL) {
    drerrno = DRE_COULDNOTCREATE;
    fprintf (stderr,"Could not create script at: %s\n",filename);
    fprintf (stderr,"ERROR: %s\n",drerrno_str());
    return NULL;
  }

#ifndef _WIN32
  fchmod (fileno(f),0777);
#endif
  jsi = (struct jobscript_info*) malloc (sizeof (struct jobscript_info));
  if(jsi) {
    jsi->type = type;
    jsi->file = f;
    strncpy(jsi->filename,filename,PATH_MAX);
  }

  return jsi;
}

/* check for null pointer */
int
jobscript_check_pointer (struct jobscript_info *ji) {
  if ( ji == NULL ) {
    fprintf (stderr,"ERROR: jobscript: invalid NULL pointer\n");
    return 0;
  }

  return 1;
}

/* write shebang line into script file */
int
jobscript_write_heading (struct jobscript_info *ji) {
  int rv;
  if ( (rv = jobscript_check_pointer (ji)) == 0 ) {
    return rv;
  }

  switch (ji->type) {
  case JOBSCRIPT_TCSH:
    return jobscript_tcsh_write_heading (ji);
    break;
  case JOBSCRIPT_PYTHON:
    return jobscript_python_write_heading (ji);
    break;
  }
  
  // FIXME: ERROR bla bla
  return 0;
}

/* set an integer variable */
int
jobscript_set_variable_int (struct jobscript_info *ji, char *name,int64_t value) {
  char str_value[JS_MAX_VAR_VALUE];
  snprintf (str_value,JS_MAX_VAR_VALUE,"%ji",(intmax_t)value);
  return jobscript_set_variable (ji,name,str_value);
}

/* set a general variable */
int
jobscript_set_variable (struct jobscript_info *ji, char *name, char *value) {
  int rv;
  if ((rv = jobscript_check_pointer (ji)) == 0 ) {
    return rv;
  }

  switch (ji->type) {
  case JOBSCRIPT_TCSH:
    return jobscript_tcsh_set_variable (ji,name,value);
    break;
  case JOBSCRIPT_PYTHON:
    return jobscript_python_set_variable (ji,name,value);
    break;
  }

  // FIXME: ERROR bla bla
  return 0;
}

/* check for valid TCSH script file */
int
jobscript_tcsh_check_pointer (struct jobscript_info *ji) {
  if ( ji == NULL ) {
    fprintf (stderr,"ERROR: tcsh jobscript: invalid NULL pointer\n");
    return 0;
  } else if ( ji->type != JOBSCRIPT_TCSH ) {
    fprintf (stderr,"ERROR: tcsh jobscript: pointer indicates not a tcsh script file (type is %i and should be JOBSCRIPT_TCSH)\n", ji->type);
    return 0;
  }

  return 1;
}

/* check for valid Python script file */
int
jobscript_python_check_pointer (struct jobscript_info *ji) {
  if ( ji == NULL ) {
    fprintf (stderr,"ERROR: python jobscript: invalid NULL pointer\n");
    return 0;
  } else if ( ji->type != JOBSCRIPT_PYTHON ) {
    fprintf (stderr,"ERROR: python jobscript: pointer indicates not a python script file (type is %i and should be JOBSCRIPT_PYTHON)\n", ji->type);
    return 0;
  }

  return 1;
}

/* write TCSH shebang line into script file */
int
jobscript_tcsh_write_heading (struct jobscript_info *ji) {
  int rv;
  if ( (rv = jobscript_tcsh_check_pointer (ji)) == 0 ) {
    return rv;
  }
  fprintf(ji->file,"#!/bin/tcsh\n");
  fprintf(ji->file,"### GENERATED BY JOBSCRIPT TOOLS\n");
  fprintf(ji->file,"# generated using the jobscript tools from libdrqueue\n");
  fprintf(ji->file,"### END OF HEADING\n");
  return 1;
}

/* write Python shebang line into script file */
int
jobscript_python_write_heading (struct jobscript_info *ji) {
  int rv;
  if ( (rv = jobscript_python_check_pointer (ji)) == 0 ) {
    return rv;
  }
  fprintf(ji->file,"#!/usr/bin/env python\n");
  fprintf(ji->file,"### GENERATED BY JOBSCRIPT TOOLS\n");
  fprintf(ji->file,"# generated using the jobscript tools from libdrqueue\n");
  fprintf(ji->file,"### END OF HEADING\n");
  return 1;
}

/* set a variable in TCSH style */
int
jobscript_tcsh_set_variable (struct jobscript_info *ji, char *name,char *value) {
  int rv;
  if ( (rv = jobscript_tcsh_check_pointer (ji)) == 0 ) {
    return rv;
  }
  // FIXME: correct name of variable
  fprintf (ji->file,"\n## tcsh variable set by jobscript tools\n");
  fprintf (ji->file,"set %s=\"%s\"\n",name,value);
  return 1;
}

/* set a variable in Python style */
int
jobscript_python_set_variable (struct jobscript_info *ji, char *name,char *value) {
  int rv;
  if ( (rv = jobscript_python_check_pointer (ji)) == 0 ) {
    return rv;
  }
  // FIXME: correct name of variable
  fprintf (ji->file,"\n## python variable set by jobscript tools\n");
  fprintf (ji->file,"%s=\"%s\"\n",name,value);
  return 1;
}

/* use template to create script file */
int
jobscript_template_write (struct jobscript_info *ji, char *template_file_name)
{
  int rv;
  char template_file_path[PATH_MAX];
  char *templates_directory;
  FILE *file_template;
  int size;
  char buf[BUFFERLEN];

  // FIXME: Check template type (bash,tcsh,...), detect and report errors

  if ( (rv = jobscript_check_pointer (ji)) == 0 ) {
    return rv;
  }
  
  if ((templates_directory = getenv ("DRQUEUE_ETC")) == NULL)
  {
    if ((templates_directory = getenv ("DRQUEUE_ROOT")) == NULL) {
      fprintf (stderr,"ERROR: No templates directory found. Check environment variables DRQUEUE_ETC and DRQUEUE_ROOT\n");
      return 0;
    } else {
      snprintf(template_file_path,PATH_MAX,"%s/etc/%s",templates_directory,template_file_name);
    }
  } else {
    snprintf(template_file_path,PATH_MAX,"%s/%s",templates_directory,template_file_name);
  }

  if ((file_template = fopen (template_file_path,"r")) == NULL) {
    fprintf (stderr,"ERROR: No template file could not be read at '%s'\n",template_file_path);
    return 0;
  }
  
  fprintf (ji->file,"\n# jobscript starts writing template file '%s'\n",template_file_path);

  fflush (ji->file);
  while ((size = fread (buf, 1, BUFFERLEN, file_template)) != 0) {
    fwrite (buf, size, 1, ji->file);
  }
  fclose(file_template);
  fflush (ji->file);

  fprintf (ji->file,"\n# jobscript finished writing template file '%s'\n",template_file_path);

  return 1;
}

/* close script file */
int
jobscript_close (struct jobscript_info *ji) {
  int rv = 0;
  
  if ( (rv = jobscript_check_pointer (ji)) == 0 ) {
    return rv;
  }

  rv = fclose (ji->file);
  free(ji);

  return rv;
}
