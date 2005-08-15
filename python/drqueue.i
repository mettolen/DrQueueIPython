// 
// Copyright (C) 2001,2002,2003,2004,2005 Jorge Daza Garcia-Blanes
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA	 02111-1307
// USA
// 
// $Id: /drqueue/remote/trunk/jobfinfo.c 2334 2005-07-05T03:50:01.502018Z jorge  $
//
// This program returns the number of times a frame has been requeued.
// Useful for avoiding endless loops
//

%define DOCSTRING
"The drqueue module allows the access to the libdrqueue library responsible
of all major operations that can be applied remotely to drqueue master and
slaves. Also provides access to all data structures of DrQueue."
%enddef
%module (docstring=DOCSTRING) drqueue
%{
#include "libdrqueue.h"
%}


%include "typemaps.i"

%typemap(in,numinputs=0) struct computer **computer (struct computer *computer) {
	$1 = &computer;
}
%typemap(argout) struct computer **computer {
	if (result < 0) {
		PyErr_SetString(PyExc_IOError,drerrno_str());
		$result = NULL;
	} else {
		int i;
		PyObject *l = PyList_New(0);
		struct computer *c = malloc (sizeof(struct computer)*result);
		if (!c)
			return PyErr_NoMemory();
		struct computer *tc = c;
		memcpy (c,*$1,sizeof(struct computer)*result);
		for (i=0; i<result; i++) {
			PyObject *o = SWIG_NewPointerObj((void*)(tc), SWIGTYPE_p_computer, 0);
			PyList_Append(l,o);
			tc++;
		}
		//free (c);
		free (*$1);
		$result = l;
	}
}

%typemap(in,numinputs=0) struct job **job (struct job *job) {
	$1 = &job;
}
%typemap(argout) struct job **job {
	if (result < 0) {
		PyErr_SetString(PyExc_IOError,drerrno_str());
		$result = NULL;
	} else {
		int i;
		PyObject *l = PyList_New(0);
		struct job *j = malloc (sizeof(struct job)*result);
		if (!j)
			return PyErr_NoMemory();
		struct job *tj = j;
		memcpy (j,*$1,sizeof(struct job)*result);
		for (i=0; i<result; i++) {
			PyObject *o = SWIG_NewPointerObj((void*)(tj), SWIGTYPE_p_job, 0);
			PyList_Append(l,o);
			tj++;
		}
		//free (j);
		free(*$1);
		$result = l;
	}
}

%include "libdrqueue.h"
%include "computer.h"
%include "computer_info.h"
%include "computer_status.h"
%include "task.h"
%include "request.h"
%include "constants.h"
%include "job.h"

typedef unsigned int time_t;
typedef unsigned short int uint16_t;
typedef unsigned long int uint32_t;
typedef unsigned char uint8_t;


// JOB
%extend job {
	job ()
	{
		struct job *j;
		j = malloc (sizeof(struct job));
		if (!j)
			return (struct job *)PyErr_NoMemory();

		job_init (j);
		return j;
	}

	~job ()
	{
		free (self);
	}

	PyObject *request_frame_list (int who)
	{
		PyObject *l = PyList_New(0);
		int nframes = job_nframes(self);
		int i;
		if (nframes) {
			struct frame_info *fi = malloc (sizeof(struct frame_info) * nframes);
			if (!fi) {
				return PyErr_NoMemory();
			}
			if (!request_job_xferfi (self->id,fi,nframes,who)) {
				PyErr_SetString(PyExc_IOError,drerrno_str());
				return NULL;
			}
			for (i=0; i<nframes; i++) {
				PyObject *o = SWIG_NewPointerObj((void*)(&fi[i]), SWIGTYPE_p_frame_info, 0);
				PyList_Append(l,o);
			}
			//free (fi);
		}
		return l;
	}

	int job_frame_index_to_number (int index)
	{
		if ((index < 0) || (index >= job_nframes(self))) {
			PyErr_SetString(PyExc_IndexError,"frame index out of range");
			return -1;
		}

		return job_frame_index_to_number (self,index);
	}

	int request_stop (int who)
	{
		if (!request_job_stop (self->id,who)) {
			PyErr_SetString(PyExc_IOError,drerrno_str());
			return 0;
		}
		return 1;
	}

	int request_hard_stop (int who)
	{
		if (!request_job_hstop (self->id,who)) {
			PyErr_SetString(PyExc_IOError,drerrno_str());
			return 0;
		}
		return 1;
	}

	int request_delete (int who)
	{
		if (!request_job_delete (self->id,who)) {
			PyErr_SetString(PyExc_IOError,drerrno_str());
			return 0;
		}
		return 1;
	}

	int request_continue (int who)
	{
		if (!request_job_continue (self->id,who)) {
			PyErr_SetString(PyExc_IOError,drerrno_str());
			return 0;
		}
		return 1;
	}
}


// COMPUTER
%extend computer {
	computer ()
	{
		struct computer *c;
		c = malloc (sizeof(struct computer));
		if (!c)
			return (struct computer *)PyErr_NoMemory();
		computer_init(c);
		return c;
	}

	~computer ()
	{
		free (self);
	}

	void request_enable (int who)
	{
		if (!request_slave_limits_enabled_set (self->hwinfo.name,1,who)) {
			PyErr_SetString(PyExc_IOError,drerrno_str());
		}
	}

	void request_disable (int who)
	{
		if (!request_slave_limits_enabled_set (self->hwinfo.name,0,who)) {
			PyErr_SetString(PyExc_IOError,drerrno_str());
		}
	}
}

/* COMPUTER LIMITS */
%extend computer_limits {
	%exception get_pool {
		$action
		if (!result) {
			PyErr_SetString(PyExc_IndexError,"Index out of range");
			return NULL;
		}
	}
	%newobject get_pool;
	struct pool *get_pool (int n)
	{
		struct pool *pool;
		
		if (n >= self->npools) {
			return NULL;
		} else if ( n < 0 ) {
			return NULL;
		}

		pool = (struct pool *) malloc (sizeof (struct pool));
		if (!pool)
			return (struct pool *)PyErr_NoMemory();

		if (self->npools) {
			if ((self->pool = (struct pool *) computer_pool_attach_shared_memory(self->poolshmid)) == (void*)-1) {
				perror ("Attaching");
				fprintf (stderr,"ERROR attaching memory %d shmid\n", self->poolshmid);
				return pool;
			}
		}
		memcpy(pool,&self->pool[n],sizeof(struct pool));

		computer_pool_detach_shared_memory (self->pool);

		return pool;
	}
}

/* COMPUTER STATUS */
%extend computer_status {
	%exception get_loadavg {
		$action
		if (result == (uint16_t)-1) {
			PyErr_SetString(PyExc_IndexError,"Index out of range");
			return NULL;
		}
	}
	uint16_t get_loadavg (int index)
	{
		if ((index < 0) || (index > 2)) {
			return -1;
		}

		return self->loadavg[index];
	}

	%exception get_task {
		$action
		if (!result) {
			PyErr_SetString(PyExc_IndexError,"Index out of range");
			return NULL;
		}
	}
	struct task *get_task (int index)
	{
		if ((index < 0) || (index >= MAXTASKS)) {
			return NULL;
		}
		return &self->task[index];
	}
}
