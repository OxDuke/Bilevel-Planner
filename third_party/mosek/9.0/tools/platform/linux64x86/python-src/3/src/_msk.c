#include <Python.h>
#include <mosek.h>
#include <string.h>
#include <stddef.h>

typedef struct {
  PyObject_HEAD
  /* Type-specific fields go here. */
  PyObject * stream_func[4];

  MSKenv_t ptr;
} mosek_EnvObject;

typedef struct {
  PyObject_HEAD
  /* Type-specific fields go here. */

  MSKtask_t ptr;

  PyObject * stream_func[4];
  PyObject * callback_func;
  PyObject * infocallback_func;

  int       killflag;
} mosek_TaskObject;

static void       Task_dealloc(mosek_TaskObject *self);
static PyObject * Task_new    (PyTypeObject     *type, PyObject *args, PyObject *kwds);
static int        Task_init   (mosek_TaskObject *self, PyObject *args, PyObject *kwds);
static void       Env_dealloc (mosek_EnvObject  *self);
static PyObject * Env_new     (PyTypeObject     *type, PyObject *args, PyObject *kwds);
static int        Env_init    (mosek_EnvObject  *self, PyObject *args, PyObject *kwds);

#if defined(WIN32X86) && WIN32X86
extern PyMethodDef Env_methods[];
extern PyMethodDef Task_methods[];
#else
static PyMethodDef Env_methods[];
static PyMethodDef Task_methods[];
#endif


static PyTypeObject mosek_EnvType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_msk.Env",                /* tp_name */
    sizeof(mosek_EnvObject),   /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)Env_dealloc,   /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /* tp_flags */
    "Env objects",             /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    Env_methods,               /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Env_init,        /* tp_init */
    0,                         /* tp_alloc */
    Env_new,                   /* tp_new */
};

static PyTypeObject mosek_TaskType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_msk.Task",              /* tp_name */
    sizeof(mosek_TaskObject),  /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)Task_dealloc,  /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /* tp_flags */
    "Task objects",            /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    Task_methods,              /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Task_init,      /* tp_init */
    0,                         /* tp_alloc */
    Task_new,                 /* tp_new */
};


/******************************************************************************/
/* Env */

static PyObject * PyMSK_env_dispose(mosek_EnvObject* self)
{
  if (self->ptr)
  {
    MSK_deleteenv(&self->ptr);
    self->ptr = NULL;

    Py_XDECREF(self->stream_func[0]);  self->stream_func[0] = NULL;
    Py_XDECREF(self->stream_func[1]);  self->stream_func[1] = NULL;
    Py_XDECREF(self->stream_func[2]);  self->stream_func[2] = NULL;
    Py_XDECREF(self->stream_func[3]);  self->stream_func[3] = NULL;
  }
  Py_XINCREF(Py_None);
  return Py_None;
}

static void
Env_dealloc(mosek_EnvObject* self)
{
  if (self->ptr)
  {
    MSK_deleteenv(&self->ptr);
    self->ptr = NULL;

    Py_XDECREF(self->stream_func[0]);  self->stream_func[0] = NULL;
    Py_XDECREF(self->stream_func[1]);  self->stream_func[1] = NULL;
    Py_XDECREF(self->stream_func[2]);  self->stream_func[2] = NULL;
    Py_XDECREF(self->stream_func[3]);  self->stream_func[3] = NULL;
  }
  Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *
Env_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    mosek_EnvObject *self;

    self = (mosek_EnvObject *)type->tp_alloc(type, 0);
    if (self != NULL)
      self->ptr = NULL;

    return (PyObject *)self;
}




static void env_stream_func(mosek_EnvObject * self, MSKstreamtypee whichstream, const char * msg)
{
  if (self->stream_func[whichstream])
  {
    PyGILState_STATE gstate;
    PyObject * res;
    gstate = PyGILState_Ensure();

    if (self->stream_func[whichstream])
    {
      res = PyObject_CallFunction(self->stream_func[whichstream],"(s)",msg);
      if (!res)
        PyErr_Clear();
      else
        Py_DECREF(res);
    }
    PyGILState_Release(gstate);
  }
}

static void MSKAPI log_env_stream_func(mosek_EnvObject * self, const char * msg) { env_stream_func(self,MSK_STREAM_LOG,msg); }
static void MSKAPI err_env_stream_func(mosek_EnvObject * self, const char * msg) { env_stream_func(self,MSK_STREAM_ERR,msg); }
static void MSKAPI msg_env_stream_func(mosek_EnvObject * self, const char * msg) { env_stream_func(self,MSK_STREAM_MSG,msg); }
static void MSKAPI wrn_env_stream_func(mosek_EnvObject * self, const char * msg) { env_stream_func(self,MSK_STREAM_WRN,msg); }

static int
Env_init(mosek_EnvObject *self, PyObject *args, PyObject *kwds)
{
    char * dbgfile = NULL;

    static char *kwlist[] = {"dbgfile", NULL};

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|s", kwlist,
                                      &dbgfile))
        return -1;


    {
      MSKrescodee r = MSK_makeenv(&self->ptr,dbgfile);
      if (r != MSK_RES_OK)
      {
        PyErr_SetString(PyExc_ValueError,"Failed to create Env object");
        return -1;
      }

      MSK_linkfunctoenvstream(self->ptr,MSK_STREAM_LOG,self,(MSKstreamfunc)log_env_stream_func);
      MSK_linkfunctoenvstream(self->ptr,MSK_STREAM_MSG,self,(MSKstreamfunc)msg_env_stream_func);
      MSK_linkfunctoenvstream(self->ptr,MSK_STREAM_WRN,self,(MSKstreamfunc)wrn_env_stream_func);
      MSK_linkfunctoenvstream(self->ptr,MSK_STREAM_ERR,self,(MSKstreamfunc)err_env_stream_func);

      self->stream_func[0] = NULL;
      self->stream_func[1] = NULL;
      self->stream_func[2] = NULL;
      self->stream_func[3] = NULL;

    }

    return 0;
}

static int Env_check(PyObject * self) { return self != NULL && Py_TYPE(self) == &mosek_EnvType; }

extern MSKrescodee MSKAPI MSK_enablegarcolenv(MSKenv_t e);

static PyObject * PyMSK_enablegarcolenv(mosek_EnvObject * self)
{
  MSKrescodee r = MSK_enablegarcolenv(self->ptr);
  return PyLong_FromLong(r);
}


static PyObject * PyMSK_env_set_Stream(mosek_EnvObject * self, PyObject * args)
{
  MSKstreamtypee whichstream;
  PyObject * func = NULL;

  if (! PyArg_ParseTuple(args,"iO",&whichstream,&func))
    goto ERROR;

  if (whichstream >= 0 && whichstream <= 4)
  {
    Py_XDECREF(self->stream_func[whichstream]);
    self->stream_func[whichstream] = func;
    Py_INCREF(self->stream_func[whichstream]);
  }
  Py_INCREF(Py_None);
  return Py_None;

 ERROR:
  return NULL;
}

static PyObject * PyMSK_env_remove_Stream(mosek_EnvObject * self, PyObject * args)
{
  MSKstreamtypee whichstream;

  if (! PyArg_ParseTuple(args,"i",&whichstream))
    goto ERROR;

  if (whichstream >= 0 && whichstream <= 4)
  {
    Py_XDECREF(self->stream_func[whichstream]);
    self->stream_func[whichstream] = NULL;
  }
  Py_INCREF(Py_None);
  return Py_None;

 ERROR:
  return NULL;
}



/******************************************************************************/
/* Task */

static PyObject *
PyMSK_task_dispose(mosek_TaskObject* self)
{
  if (self->ptr)
  {
    MSK_deletetask(&self->ptr);
    self->ptr = NULL;
  }

  Py_XDECREF(self->callback_func); self->callback_func = NULL;
  Py_XDECREF(self->infocallback_func);  self->infocallback_func = NULL;
  Py_XDECREF(self->stream_func[0]);  self->stream_func[0] = NULL;
  Py_XDECREF(self->stream_func[1]);  self->stream_func[1] = NULL;
  Py_XDECREF(self->stream_func[2]);  self->stream_func[2] = NULL;
  Py_XDECREF(self->stream_func[3]);  self->stream_func[3] = NULL;

  Py_XINCREF(Py_None);
  return Py_None;
}

static void
Task_dealloc(mosek_TaskObject* self)
{

  if (self->ptr)
  {
    MSK_deletetask(&self->ptr);
    self->ptr = NULL;
  }

  Py_XDECREF(self->callback_func); self->callback_func = NULL;
  Py_XDECREF(self->infocallback_func);  self->infocallback_func = NULL;
  Py_XDECREF(self->stream_func[0]);  self->stream_func[0] = NULL;
  Py_XDECREF(self->stream_func[1]);  self->stream_func[1] = NULL;
  Py_XDECREF(self->stream_func[2]);  self->stream_func[2] = NULL;
  Py_XDECREF(self->stream_func[3]);  self->stream_func[3] = NULL;


  Py_TYPE(self)->tp_free((PyObject*)self);
}


static PyObject *
Task_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    mosek_TaskObject *self;

    /* printf("%s:%d: Task_new....\n",__FILE__,__LINE__); */
    self = (mosek_TaskObject *)type->tp_alloc(type, 0);
    /* printf("%s:%d: allocate %d\n",__FILE__,__LINE__,(int)sizeof(mosek_TaskObject)); */
    if (self != NULL)
    {
      self->callback_func = NULL;
      self->infocallback_func = NULL;
      self->stream_func[0] = NULL;
      self->stream_func[1] = NULL;
      self->stream_func[2] = NULL;
      self->stream_func[3] = NULL;
      self->ptr = NULL;
    }
    /*    printf("%s:%d: self = %p\n",__FILE__,__LINE__,self);*/
    return (PyObject *)self;
}

static int MSKAPI callback_func(MSKtask_t t, mosek_TaskObject * self, MSKcallbackcodee code, double * dinf, int * iinf, long long * liinf)
{
  int callback_res = 0;
  if (self->callback_func || self->infocallback_func)
  {
    PyGILState_STATE gstate;
    PyObject * res;
    gstate = PyGILState_Ensure();

    if (self->callback_func)
    {
      res = PyObject_CallFunction(self->callback_func,"(i)",code);
      if (!res)
        callback_res = 1;
      else if (PyObject_IsTrue(res))
        callback_res = 1;
      Py_XDECREF(res);
    }

    
    if (self->infocallback_func && callback_res == 0)
    {
      PyObject * dinf_obj = NULL;
      PyObject * iinf_obj = NULL;
      PyObject * liinf_obj = NULL;

      if ( NULL != (dinf_obj  = PyList_New(MSK_DINF_END)) &&
           NULL != (iinf_obj  = PyList_New(MSK_IINF_END)) &&
           NULL != (liinf_obj = PyList_New(MSK_LIINF_END)))
      {
        ptrdiff_t i;
        for (i = 0; i < MSK_DINF_END; ++i)  PyList_SET_ITEM(dinf_obj,  i, PyFloat_FromDouble(dinf[i]));
        for (i = 0; i < MSK_IINF_END; ++i)  PyList_SET_ITEM(iinf_obj,  i, PyLong_FromLong(iinf[i]));
        for (i = 0; i < MSK_LIINF_END; ++i) PyList_SET_ITEM(liinf_obj, i, PyLong_FromLongLong(liinf[i]));

        res = PyObject_CallFunction(self->infocallback_func,"(iOOO)",code,dinf_obj,iinf_obj,liinf_obj);

        if (!res)
          callback_res = 1;
        else if (PyObject_IsTrue(res))
          callback_res = 1;
        Py_XDECREF(res);
      }
      
      Py_XDECREF(dinf_obj);
      Py_XDECREF(iinf_obj);
      Py_XDECREF(liinf_obj);
    }

    PyGILState_Release(gstate);
  }
  return callback_res;
}

static void stream_func(mosek_TaskObject * self, MSKstreamtypee whichstream, const char * msg)
{
  if (self->stream_func[whichstream])
  {
    PyGILState_STATE gstate;
    PyObject * res;
    gstate = PyGILState_Ensure();

    if (self->stream_func[whichstream])
    {
      res = PyObject_CallFunction(self->stream_func[whichstream],"(s)",msg);
      if (res)
        Py_DECREF(res);
    }
    PyGILState_Release(gstate);
  }
}

static void MSKAPI log_task_stream_func(mosek_TaskObject * self, const char * msg) { stream_func(self,MSK_STREAM_LOG,msg); }
static void MSKAPI err_task_stream_func(mosek_TaskObject * self, const char * msg) { stream_func(self,MSK_STREAM_ERR,msg); }
static void MSKAPI msg_task_stream_func(mosek_TaskObject * self, const char * msg) { stream_func(self,MSK_STREAM_MSG,msg); }
static void MSKAPI wrn_task_stream_func(mosek_TaskObject * self, const char * msg) { stream_func(self,MSK_STREAM_WRN,msg); }


static int Task_init(mosek_TaskObject *self, PyObject *args, PyObject *kwds)
{
    PyObject *env_o = NULL, * other_o = NULL;
    int numcon = 0,numvar = 0;


    static char *kwlist[] = { "env", "numcon","numvar", "other", NULL };

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|OiiO", kwlist, &env_o,&numcon,&numvar,&other_o))
        return -1;

    if (other_o == NULL && ! Env_check(env_o)) {
      PyErr_SetString(PyExc_TypeError,"Expected a mosek.Env value");
      return -1;
    }
    else {
      MSKrescodee r = MSK_RES_OK;
      
      if (other_o != NULL) {
        MSKtask_t othertask  = ((mosek_TaskObject*)other_o)->ptr;
        r = MSK_clonetask(othertask,&self->ptr);
      }
      else {
        MSKenv_t env  = ((mosek_EnvObject*)env_o)->ptr;
        /*printf("%s%d: Task_init env = %p\n",__FILE__,__LINE__,env);*/
        r = MSK_makeemptytask(env,&self->ptr);
      }
      if (r != MSK_RES_OK)
      {
        PyErr_SetString(PyExc_ValueError,"Failed to create Task object");
        return -1;
      }

      MSK_linkfunctotaskstream(self->ptr,MSK_STREAM_LOG,self,(MSKstreamfunc)log_task_stream_func);
      MSK_linkfunctotaskstream(self->ptr,MSK_STREAM_MSG,self,(MSKstreamfunc)msg_task_stream_func);
      MSK_linkfunctotaskstream(self->ptr,MSK_STREAM_WRN,self,(MSKstreamfunc)wrn_task_stream_func);
      MSK_linkfunctotaskstream(self->ptr,MSK_STREAM_ERR,self,(MSKstreamfunc)err_task_stream_func);
      MSK_putcallbackfunc(self->ptr,(MSKcallbackfunc)callback_func,self);

      self->callback_func = NULL;
      self->infocallback_func = NULL;
      self->stream_func[0] = NULL;
      self->stream_func[1] = NULL;
      self->stream_func[2] = NULL;
      self->stream_func[3] = NULL;
    }
    return 0;
}

#if 0
static int Task_check(PyObject * self) { return self != NULL && Py_TYPE(self) == &mosek_TaskType; }
#endif

static PyObject * PyMSK_set_Stream(mosek_TaskObject * self, PyObject * args)
{
  MSKstreamtypee whichstream;
  PyObject * func = NULL;

  if (! PyArg_ParseTuple(args,"iO",&whichstream,&func))
    goto ERROR;

  if (whichstream >= 0 && whichstream <= 4)
  {
    Py_XDECREF(self->stream_func[whichstream]);
    self->stream_func[whichstream] = func;
    Py_INCREF(self->stream_func[whichstream]);
  }
  Py_INCREF(Py_None);
  return Py_None;

 ERROR:
  return NULL;
}

static PyObject * PyMSK_set_Progress(mosek_TaskObject * self, PyObject * args)
{
  PyObject * func = NULL;

  if (! PyArg_ParseTuple(args,"O",&func))
    goto ERROR;

  Py_XDECREF(self->callback_func);
  self->callback_func = func;
  Py_INCREF(self->callback_func);

  Py_INCREF(Py_None);
  return Py_None;

 ERROR:
  return NULL;
}

static PyObject * PyMSK_set_InfoCallback(mosek_TaskObject * self, PyObject * args)
{
  PyObject * func = NULL;

  if (! PyArg_ParseTuple(args,"O",&func))
    goto ERROR;

  Py_XDECREF(self->infocallback_func);
  self->infocallback_func = func;
  Py_INCREF(self->infocallback_func);

  Py_INCREF(Py_None);
  return Py_None;

 ERROR:
  return NULL;
}

static PyObject * PyMSK_remove_Stream(mosek_TaskObject * self, PyObject * args)
{
  MSKstreamtypee whichstream;

  if (! PyArg_ParseTuple(args,"i",&whichstream))
    goto ERROR;

  if (whichstream >= 0 && whichstream <= 4)
  {
    Py_XDECREF(self->stream_func[whichstream]);
    self->stream_func[whichstream] = NULL;
  }
  Py_INCREF(Py_None);
  return Py_None;

 ERROR:
  return NULL;
}

static PyObject * PyMSK_remove_Progress(mosek_TaskObject * self, PyObject * args)
{
  Py_XDECREF(self->callback_func);
  self->callback_func = NULL;

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * PyMSK_remove_InfoCallback(mosek_TaskObject * self, PyObject * args)
{
  Py_XDECREF(self->infocallback_func);
  self->infocallback_func = NULL;

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * PyMSK_getlasterror(mosek_TaskObject * self, PyObject * args)
{
  PyObject * memview_msg = NULL;
  MSKrescodee lasterr;
  int msglen;
  MSKrescodee r;

  r = MSK_getlasterror(self->ptr, &lasterr, 0, &msglen, NULL);
  if (r == MSK_RES_OK)
  {
    if (msglen > 0)
    {
      PyObject * res;
      char * buf = malloc(msglen+1); buf[0] = 0;
      MSK_getlasterror(self->ptr, &lasterr, msglen+1, NULL, buf);
      res = Py_BuildValue("(is)",lasterr,buf);
      free(buf);
      return res;
    }
    else
      return Py_BuildValue("(is)",lasterr,"");
  }
  else
    return Py_BuildValue("(is)",0,"");
}


static PyObject * PyMSK_analyzeproblem(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKstreamtypee _arg1_whichstream;
  if (! PyArg_ParseTuple(_args,"i",&_arg1_whichstream)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_analyzeproblem(self->ptr,_arg1_whichstream);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* analyzeproblem */

static PyObject * PyMSK_analyzenames(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKstreamtypee _arg1_whichstream;
  MSKnametypee _arg2_nametype;
  if (! PyArg_ParseTuple(_args,"ii",&_arg1_whichstream,&_arg2_nametype)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_analyzenames(self->ptr,_arg1_whichstream,_arg2_nametype);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* analyzenames */

static PyObject * PyMSK_analyzesolution(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKstreamtypee _arg1_whichstream;
  MSKsoltypee _arg2_whichsol;
  if (! PyArg_ParseTuple(_args,"ii",&_arg1_whichstream,&_arg2_whichsol)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_analyzesolution(self->ptr,_arg1_whichstream,_arg2_whichsol);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* analyzesolution */

static PyObject * PyMSK_initbasissolve(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  PyObject * memview__arg1_basis = NULL;
  Py_buffer * buf__arg1_basis;
  int * _arg1_basis = NULL;
  if (! PyArg_ParseTuple(_args,"O",&memview__arg1_basis)) goto ERROR;
  if (Py_None != memview__arg1_basis)
  {
    if (! PyMemoryView_Check(memview__arg1_basis))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg1_basis");
      goto ERROR;
    }
    buf__arg1_basis = PyMemoryView_GET_BUFFER(memview__arg1_basis);
    if (buf__arg1_basis->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg1_basis");
        goto ERROR;
      
    }
    _arg1_basis = (int*) buf__arg1_basis->buf;
    if (buf__arg1_basis->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg1_basis");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_initbasissolve(self->ptr,_arg1_basis);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* initbasissolve */

static PyObject * PyMSK_solvewithbasis(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_transp;
  int _arg2_numnz; //  t = <aisread.aisTypeRef object at 0x7fb398592550>
  PyObject * memview__arg3_sub = NULL;
  Py_buffer * buf__arg3_sub;
  int * _arg3_sub = NULL;
  PyObject * memview__arg4_val = NULL;
  Py_buffer * buf__arg4_val;
  double * _arg4_val = NULL;
  if (! PyArg_ParseTuple(_args,"iiOO",&_arg1_transp,&_arg2_numnz,&memview__arg3_sub,&memview__arg4_val)) goto ERROR;
  if (Py_None != memview__arg3_sub)
  {
    if (! PyMemoryView_Check(memview__arg3_sub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_sub");
      goto ERROR;
    }
    buf__arg3_sub = PyMemoryView_GET_BUFFER(memview__arg3_sub);
    if (buf__arg3_sub->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_sub");
        goto ERROR;
      
    }
    _arg3_sub = (int*) buf__arg3_sub->buf;
    if (buf__arg3_sub->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg3_sub");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg4_val)
  {
    if (! PyMemoryView_Check(memview__arg4_val))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_val");
      goto ERROR;
    }
    buf__arg4_val = PyMemoryView_GET_BUFFER(memview__arg4_val);
    if (buf__arg4_val->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_val");
        goto ERROR;
      
    }
    _arg4_val = (double*) buf__arg4_val->buf;
    if (buf__arg4_val->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_val");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_solvewithbasis(self->ptr,_arg1_transp,&_arg2_numnz,_arg3_sub,_arg4_val);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_numnz);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* solvewithbasis */

static PyObject * PyMSK_basiscond(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  double _arg1_nrmbasis; //  t = <aisread.aisTypeRef object at 0x7fb398592cc0>
  double _arg2_nrminvbasis; //  t = <aisread.aisTypeRef object at 0x7fb398592dd8>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_basiscond(self->ptr,&_arg1_nrmbasis,&_arg2_nrminvbasis);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(i(dd))",0,_arg1_nrmbasis,_arg2_nrminvbasis);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* basiscond */

static PyObject * PyMSK_appendcons(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  if (! PyArg_ParseTuple(_args,"i",&_arg1_num)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_appendcons(self->ptr,_arg1_num);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* appendcons */

static PyObject * PyMSK_appendvars(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  if (! PyArg_ParseTuple(_args,"i",&_arg1_num)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_appendvars(self->ptr,_arg1_num);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* appendvars */

static PyObject * PyMSK_removecons(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  PyObject * memview__arg2_subset = NULL;
  Py_buffer * buf__arg2_subset;
  int * _arg2_subset = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_num,&memview__arg2_subset)) goto ERROR;
  if (Py_None != memview__arg2_subset)
  {
    if (! PyMemoryView_Check(memview__arg2_subset))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_subset");
      goto ERROR;
    }
    buf__arg2_subset = PyMemoryView_GET_BUFFER(memview__arg2_subset);
    if (buf__arg2_subset->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_subset");
        goto ERROR;
      
    }
    _arg2_subset = (int*) buf__arg2_subset->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_removecons(self->ptr,_arg1_num,_arg2_subset);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* removecons */

static PyObject * PyMSK_removevars(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  PyObject * memview__arg2_subset = NULL;
  Py_buffer * buf__arg2_subset;
  int * _arg2_subset = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_num,&memview__arg2_subset)) goto ERROR;
  if (Py_None != memview__arg2_subset)
  {
    if (! PyMemoryView_Check(memview__arg2_subset))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_subset");
      goto ERROR;
    }
    buf__arg2_subset = PyMemoryView_GET_BUFFER(memview__arg2_subset);
    if (buf__arg2_subset->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_subset");
        goto ERROR;
      
    }
    _arg2_subset = (int*) buf__arg2_subset->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_removevars(self->ptr,_arg1_num,_arg2_subset);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* removevars */

static PyObject * PyMSK_removebarvars(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  PyObject * memview__arg2_subset = NULL;
  Py_buffer * buf__arg2_subset;
  int * _arg2_subset = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_num,&memview__arg2_subset)) goto ERROR;
  if (Py_None != memview__arg2_subset)
  {
    if (! PyMemoryView_Check(memview__arg2_subset))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_subset");
      goto ERROR;
    }
    buf__arg2_subset = PyMemoryView_GET_BUFFER(memview__arg2_subset);
    if (buf__arg2_subset->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_subset");
        goto ERROR;
      
    }
    _arg2_subset = (int*) buf__arg2_subset->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_removebarvars(self->ptr,_arg1_num,_arg2_subset);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* removebarvars */

static PyObject * PyMSK_removecones(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  PyObject * memview__arg2_subset = NULL;
  Py_buffer * buf__arg2_subset;
  int * _arg2_subset = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_num,&memview__arg2_subset)) goto ERROR;
  if (Py_None != memview__arg2_subset)
  {
    if (! PyMemoryView_Check(memview__arg2_subset))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_subset");
      goto ERROR;
    }
    buf__arg2_subset = PyMemoryView_GET_BUFFER(memview__arg2_subset);
    if (buf__arg2_subset->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_subset");
        goto ERROR;
      
    }
    _arg2_subset = (int*) buf__arg2_subset->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_removecones(self->ptr,_arg1_num,_arg2_subset);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* removecones */

static PyObject * PyMSK_appendbarvars(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  PyObject * memview__arg2_dim = NULL;
  Py_buffer * buf__arg2_dim;
  int * _arg2_dim = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_num,&memview__arg2_dim)) goto ERROR;
  if (Py_None != memview__arg2_dim)
  {
    if (! PyMemoryView_Check(memview__arg2_dim))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_dim");
      goto ERROR;
    }
    buf__arg2_dim = PyMemoryView_GET_BUFFER(memview__arg2_dim);
    if (buf__arg2_dim->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_dim");
        goto ERROR;
      
    }
    _arg2_dim = (int*) buf__arg2_dim->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_appendbarvars(self->ptr,_arg1_num,_arg2_dim);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* appendbarvars */

static PyObject * PyMSK_appendcone(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKconetypee _arg1_ct;
  double _arg2_conepar;
  int _arg3_nummem;
  PyObject * memview__arg4_submem = NULL;
  Py_buffer * buf__arg4_submem;
  int * _arg4_submem = NULL;
  if (! PyArg_ParseTuple(_args,"idiO",&_arg1_ct,&_arg2_conepar,&_arg3_nummem,&memview__arg4_submem)) goto ERROR;
  if (Py_None != memview__arg4_submem)
  {
    if (! PyMemoryView_Check(memview__arg4_submem))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_submem");
      goto ERROR;
    }
    buf__arg4_submem = PyMemoryView_GET_BUFFER(memview__arg4_submem);
    if (buf__arg4_submem->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_submem");
        goto ERROR;
      
    }
    _arg4_submem = (int*) buf__arg4_submem->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_appendcone(self->ptr,_arg1_ct,_arg2_conepar,_arg3_nummem,_arg4_submem);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* appendcone */

static PyObject * PyMSK_appendconeseq(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKconetypee _arg1_ct;
  double _arg2_conepar;
  int _arg3_nummem;
  int _arg4_j;
  if (! PyArg_ParseTuple(_args,"idii",&_arg1_ct,&_arg2_conepar,&_arg3_nummem,&_arg4_j)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_appendconeseq(self->ptr,_arg1_ct,_arg2_conepar,_arg3_nummem,_arg4_j);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* appendconeseq */

static PyObject * PyMSK_appendconesseq(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  PyObject * memview__arg2_ct = NULL;
  Py_buffer * buf__arg2_ct;
  MSKconetypee * _arg2_ct = NULL;
  PyObject * memview__arg3_conepar = NULL;
  Py_buffer * buf__arg3_conepar;
  double * _arg3_conepar = NULL;
  PyObject * memview__arg4_nummem = NULL;
  Py_buffer * buf__arg4_nummem;
  int * _arg4_nummem = NULL;
  int _arg5_j;
  if (! PyArg_ParseTuple(_args,"iOOOi",&_arg1_num,&memview__arg2_ct,&memview__arg3_conepar,&memview__arg4_nummem,&_arg5_j)) goto ERROR;
  if (Py_None != memview__arg2_ct)
  {
    if (! PyMemoryView_Check(memview__arg2_ct))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_ct");
      goto ERROR;
    }
    buf__arg2_ct = PyMemoryView_GET_BUFFER(memview__arg2_ct);
    if (buf__arg2_ct->strides[0] != sizeof(MSKconetypee))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_ct");
        goto ERROR;
      
    }
    _arg2_ct = (MSKconetypee*) buf__arg2_ct->buf;
  }
  if (Py_None != memview__arg3_conepar)
  {
    if (! PyMemoryView_Check(memview__arg3_conepar))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_conepar");
      goto ERROR;
    }
    buf__arg3_conepar = PyMemoryView_GET_BUFFER(memview__arg3_conepar);
    if (buf__arg3_conepar->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_conepar");
        goto ERROR;
      
    }
    _arg3_conepar = (double*) buf__arg3_conepar->buf;
  }
  if (Py_None != memview__arg4_nummem)
  {
    if (! PyMemoryView_Check(memview__arg4_nummem))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_nummem");
      goto ERROR;
    }
    buf__arg4_nummem = PyMemoryView_GET_BUFFER(memview__arg4_nummem);
    if (buf__arg4_nummem->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_nummem");
        goto ERROR;
      
    }
    _arg4_nummem = (int*) buf__arg4_nummem->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_appendconesseq(self->ptr,_arg1_num,_arg2_ct,_arg3_conepar,_arg4_nummem,_arg5_j);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* appendconesseq */

static PyObject * PyMSK_chgconbound(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_i;
  int _arg2_lower;
  int _arg3_finite;
  double _arg4_value;
  if (! PyArg_ParseTuple(_args,"iiid",&_arg1_i,&_arg2_lower,&_arg3_finite,&_arg4_value)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_chgconbound(self->ptr,_arg1_i,_arg2_lower,_arg3_finite,_arg4_value);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* chgconbound */

static PyObject * PyMSK_chgvarbound(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_j;
  int _arg2_lower;
  int _arg3_finite;
  double _arg4_value;
  if (! PyArg_ParseTuple(_args,"iiid",&_arg1_j,&_arg2_lower,&_arg3_finite,&_arg4_value)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_chgvarbound(self->ptr,_arg1_j,_arg2_lower,_arg3_finite,_arg4_value);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* chgvarbound */

static PyObject * PyMSK_getaij(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_i;
  int _arg2_j;
  double _arg3_aij; //  t = <aisread.aisTypeRef object at 0x7fb398539668>
  if (! PyArg_ParseTuple(_args,"ii",&_arg1_i,&_arg2_j)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getaij(self->ptr,_arg1_i,_arg2_j,&_arg3_aij);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(id)",0,_arg3_aij);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getaij */

static PyObject * PyMSK_getapiecenumnz(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_firsti;
  int _arg2_lasti;
  int _arg3_firstj;
  int _arg4_lastj;
  int _arg5_numnz; //  t = <aisread.aisTypeRef object at 0x7fb398539d68>
  if (! PyArg_ParseTuple(_args,"iiii",&_arg1_firsti,&_arg2_lasti,&_arg3_firstj,&_arg4_lastj)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getapiecenumnz(self->ptr,_arg1_firsti,_arg2_lasti,_arg3_firstj,_arg4_lastj,&_arg5_numnz);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg5_numnz);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getapiecenumnz */

static PyObject * PyMSK_getacolnumnz(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_i;
  int _arg2_nzj; //  t = <aisread.aisTypeRef object at 0x7fb398540358>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_i)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getacolnumnz(self->ptr,_arg1_i,&_arg2_nzj);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_nzj);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getacolnumnz */

static PyObject * PyMSK_getacol(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_j;
  int _arg2_nzj; //  t = <aisread.aisTypeRef object at 0x7fb3985407f0>
  PyObject * memview__arg3_subj = NULL;
  Py_buffer * buf__arg3_subj;
  int * _arg3_subj = NULL;
  PyObject * memview__arg4_valj = NULL;
  Py_buffer * buf__arg4_valj;
  double * _arg4_valj = NULL;
  if (! PyArg_ParseTuple(_args,"iOO",&_arg1_j,&memview__arg3_subj,&memview__arg4_valj)) goto ERROR;
  if (Py_None != memview__arg3_subj)
  {
    if (! PyMemoryView_Check(memview__arg3_subj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_subj");
      goto ERROR;
    }
    buf__arg3_subj = PyMemoryView_GET_BUFFER(memview__arg3_subj);
    if (buf__arg3_subj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_subj");
        goto ERROR;
      
    }
    _arg3_subj = (int*) buf__arg3_subj->buf;
    if (buf__arg3_subj->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg3_subj");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg4_valj)
  {
    if (! PyMemoryView_Check(memview__arg4_valj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_valj");
      goto ERROR;
    }
    buf__arg4_valj = PyMemoryView_GET_BUFFER(memview__arg4_valj);
    if (buf__arg4_valj->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_valj");
        goto ERROR;
      
    }
    _arg4_valj = (double*) buf__arg4_valj->buf;
    if (buf__arg4_valj->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_valj");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getacol(self->ptr,_arg1_j,&_arg2_nzj,_arg3_subj,_arg4_valj);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_nzj);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getacol */

static PyObject * PyMSK_getacolslice64(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_first;
  int _arg2_last;
  long long _arg3_maxnumnz;
  long long _arg4_surp; //  t = <aisread.aisTypeRef object at 0x7fb39854d080>
  PyObject * memview__arg5_ptrb = NULL;
  Py_buffer * buf__arg5_ptrb;
  long long * _arg5_ptrb = NULL;
  PyObject * memview__arg6_ptre = NULL;
  Py_buffer * buf__arg6_ptre;
  long long * _arg6_ptre = NULL;
  PyObject * memview__arg7_sub = NULL;
  Py_buffer * buf__arg7_sub;
  int * _arg7_sub = NULL;
  PyObject * memview__arg8_val = NULL;
  Py_buffer * buf__arg8_val;
  double * _arg8_val = NULL;
  if (! PyArg_ParseTuple(_args,"iiLLOOOO",&_arg1_first,&_arg2_last,&_arg3_maxnumnz,&_arg4_surp,&memview__arg5_ptrb,&memview__arg6_ptre,&memview__arg7_sub,&memview__arg8_val)) goto ERROR;
  if (Py_None != memview__arg5_ptrb)
  {
    if (! PyMemoryView_Check(memview__arg5_ptrb))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_ptrb");
      goto ERROR;
    }
    buf__arg5_ptrb = PyMemoryView_GET_BUFFER(memview__arg5_ptrb);
    if (buf__arg5_ptrb->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_ptrb");
        goto ERROR;
      
    }
    _arg5_ptrb = (long long*) buf__arg5_ptrb->buf;
    if (buf__arg5_ptrb->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg5_ptrb");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg6_ptre)
  {
    if (! PyMemoryView_Check(memview__arg6_ptre))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_ptre");
      goto ERROR;
    }
    buf__arg6_ptre = PyMemoryView_GET_BUFFER(memview__arg6_ptre);
    if (buf__arg6_ptre->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_ptre");
        goto ERROR;
      
    }
    _arg6_ptre = (long long*) buf__arg6_ptre->buf;
    if (buf__arg6_ptre->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg6_ptre");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg7_sub)
  {
    if (! PyMemoryView_Check(memview__arg7_sub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg7_sub");
      goto ERROR;
    }
    buf__arg7_sub = PyMemoryView_GET_BUFFER(memview__arg7_sub);
    if (buf__arg7_sub->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg7_sub");
        goto ERROR;
      
    }
    _arg7_sub = (int*) buf__arg7_sub->buf;
    if (buf__arg7_sub->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg7_sub");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg8_val)
  {
    if (! PyMemoryView_Check(memview__arg8_val))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg8_val");
      goto ERROR;
    }
    buf__arg8_val = PyMemoryView_GET_BUFFER(memview__arg8_val);
    if (buf__arg8_val->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg8_val");
        goto ERROR;
      
    }
    _arg8_val = (double*) buf__arg8_val->buf;
    if (buf__arg8_val->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg8_val");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getacolslice64(self->ptr,_arg1_first,_arg2_last,_arg3_maxnumnz,&_arg4_surp,_arg5_ptrb,_arg6_ptre,_arg7_sub,_arg8_val);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getacolslice64 */

static PyObject * PyMSK_getarownumnz(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_i;
  int _arg2_nzi; //  t = <aisread.aisTypeRef object at 0x7fb39854db38>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_i)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getarownumnz(self->ptr,_arg1_i,&_arg2_nzi);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_nzi);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getarownumnz */

static PyObject * PyMSK_getarow(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_i;
  int _arg2_nzi; //  t = <aisread.aisTypeRef object at 0x7fb39854dfd0>
  PyObject * memview__arg3_subi = NULL;
  Py_buffer * buf__arg3_subi;
  int * _arg3_subi = NULL;
  PyObject * memview__arg4_vali = NULL;
  Py_buffer * buf__arg4_vali;
  double * _arg4_vali = NULL;
  if (! PyArg_ParseTuple(_args,"iOO",&_arg1_i,&memview__arg3_subi,&memview__arg4_vali)) goto ERROR;
  if (Py_None != memview__arg3_subi)
  {
    if (! PyMemoryView_Check(memview__arg3_subi))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_subi");
      goto ERROR;
    }
    buf__arg3_subi = PyMemoryView_GET_BUFFER(memview__arg3_subi);
    if (buf__arg3_subi->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_subi");
        goto ERROR;
      
    }
    _arg3_subi = (int*) buf__arg3_subi->buf;
    if (buf__arg3_subi->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg3_subi");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg4_vali)
  {
    if (! PyMemoryView_Check(memview__arg4_vali))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_vali");
      goto ERROR;
    }
    buf__arg4_vali = PyMemoryView_GET_BUFFER(memview__arg4_vali);
    if (buf__arg4_vali->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_vali");
        goto ERROR;
      
    }
    _arg4_vali = (double*) buf__arg4_vali->buf;
    if (buf__arg4_vali->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_vali");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getarow(self->ptr,_arg1_i,&_arg2_nzi,_arg3_subi,_arg4_vali);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_nzi);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getarow */

static PyObject * PyMSK_getacolslicenumnz64(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_first;
  int _arg2_last;
  long long _arg3_numnz; //  t = <aisread.aisTypeRef object at 0x7fb398551e80>
  if (! PyArg_ParseTuple(_args,"ii",&_arg1_first,&_arg2_last)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getacolslicenumnz64(self->ptr,_arg1_first,_arg2_last,&_arg3_numnz);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iL)",0,_arg3_numnz);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getacolslicenumnz64 */

static PyObject * PyMSK_getarowslicenumnz64(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_first;
  int _arg2_last;
  long long _arg3_numnz; //  t = <aisread.aisTypeRef object at 0x7fb398557a90>
  if (! PyArg_ParseTuple(_args,"ii",&_arg1_first,&_arg2_last)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getarowslicenumnz64(self->ptr,_arg1_first,_arg2_last,&_arg3_numnz);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iL)",0,_arg3_numnz);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getarowslicenumnz64 */

static PyObject * PyMSK_getarowslice64(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_first;
  int _arg2_last;
  long long _arg3_maxnumnz;
  long long _arg4_surp; //  t = <aisread.aisTypeRef object at 0x7fb398563048>
  PyObject * memview__arg5_ptrb = NULL;
  Py_buffer * buf__arg5_ptrb;
  long long * _arg5_ptrb = NULL;
  PyObject * memview__arg6_ptre = NULL;
  Py_buffer * buf__arg6_ptre;
  long long * _arg6_ptre = NULL;
  PyObject * memview__arg7_sub = NULL;
  Py_buffer * buf__arg7_sub;
  int * _arg7_sub = NULL;
  PyObject * memview__arg8_val = NULL;
  Py_buffer * buf__arg8_val;
  double * _arg8_val = NULL;
  if (! PyArg_ParseTuple(_args,"iiLLOOOO",&_arg1_first,&_arg2_last,&_arg3_maxnumnz,&_arg4_surp,&memview__arg5_ptrb,&memview__arg6_ptre,&memview__arg7_sub,&memview__arg8_val)) goto ERROR;
  if (Py_None != memview__arg5_ptrb)
  {
    if (! PyMemoryView_Check(memview__arg5_ptrb))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_ptrb");
      goto ERROR;
    }
    buf__arg5_ptrb = PyMemoryView_GET_BUFFER(memview__arg5_ptrb);
    if (buf__arg5_ptrb->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_ptrb");
        goto ERROR;
      
    }
    _arg5_ptrb = (long long*) buf__arg5_ptrb->buf;
    if (buf__arg5_ptrb->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg5_ptrb");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg6_ptre)
  {
    if (! PyMemoryView_Check(memview__arg6_ptre))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_ptre");
      goto ERROR;
    }
    buf__arg6_ptre = PyMemoryView_GET_BUFFER(memview__arg6_ptre);
    if (buf__arg6_ptre->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_ptre");
        goto ERROR;
      
    }
    _arg6_ptre = (long long*) buf__arg6_ptre->buf;
    if (buf__arg6_ptre->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg6_ptre");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg7_sub)
  {
    if (! PyMemoryView_Check(memview__arg7_sub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg7_sub");
      goto ERROR;
    }
    buf__arg7_sub = PyMemoryView_GET_BUFFER(memview__arg7_sub);
    if (buf__arg7_sub->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg7_sub");
        goto ERROR;
      
    }
    _arg7_sub = (int*) buf__arg7_sub->buf;
    if (buf__arg7_sub->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg7_sub");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg8_val)
  {
    if (! PyMemoryView_Check(memview__arg8_val))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg8_val");
      goto ERROR;
    }
    buf__arg8_val = PyMemoryView_GET_BUFFER(memview__arg8_val);
    if (buf__arg8_val->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg8_val");
        goto ERROR;
      
    }
    _arg8_val = (double*) buf__arg8_val->buf;
    if (buf__arg8_val->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg8_val");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getarowslice64(self->ptr,_arg1_first,_arg2_last,_arg3_maxnumnz,&_arg4_surp,_arg5_ptrb,_arg6_ptre,_arg7_sub,_arg8_val);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getarowslice64 */

static PyObject * PyMSK_getarowslicetrip(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_first;
  int _arg2_last;
  long long _arg3_maxnumnz;
  long long _arg4_surp; //  t = <aisread.aisTypeRef object at 0x7fb398563d30>
  PyObject * memview__arg5_subi = NULL;
  Py_buffer * buf__arg5_subi;
  int * _arg5_subi = NULL;
  PyObject * memview__arg6_subj = NULL;
  Py_buffer * buf__arg6_subj;
  int * _arg6_subj = NULL;
  PyObject * memview__arg7_val = NULL;
  Py_buffer * buf__arg7_val;
  double * _arg7_val = NULL;
  if (! PyArg_ParseTuple(_args,"iiLLOOO",&_arg1_first,&_arg2_last,&_arg3_maxnumnz,&_arg4_surp,&memview__arg5_subi,&memview__arg6_subj,&memview__arg7_val)) goto ERROR;
  if (Py_None != memview__arg5_subi)
  {
    if (! PyMemoryView_Check(memview__arg5_subi))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_subi");
      goto ERROR;
    }
    buf__arg5_subi = PyMemoryView_GET_BUFFER(memview__arg5_subi);
    if (buf__arg5_subi->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_subi");
        goto ERROR;
      
    }
    _arg5_subi = (int*) buf__arg5_subi->buf;
    if (buf__arg5_subi->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg5_subi");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg6_subj)
  {
    if (! PyMemoryView_Check(memview__arg6_subj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_subj");
      goto ERROR;
    }
    buf__arg6_subj = PyMemoryView_GET_BUFFER(memview__arg6_subj);
    if (buf__arg6_subj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_subj");
        goto ERROR;
      
    }
    _arg6_subj = (int*) buf__arg6_subj->buf;
    if (buf__arg6_subj->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg6_subj");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg7_val)
  {
    if (! PyMemoryView_Check(memview__arg7_val))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg7_val");
      goto ERROR;
    }
    buf__arg7_val = PyMemoryView_GET_BUFFER(memview__arg7_val);
    if (buf__arg7_val->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg7_val");
        goto ERROR;
      
    }
    _arg7_val = (double*) buf__arg7_val->buf;
    if (buf__arg7_val->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg7_val");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getarowslicetrip(self->ptr,_arg1_first,_arg2_last,_arg3_maxnumnz,&_arg4_surp,_arg5_subi,_arg6_subj,_arg7_val);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getarowslicetrip */

static PyObject * PyMSK_getacolslicetrip(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_first;
  int _arg2_last;
  long long _arg3_maxnumnz;
  long long _arg4_surp; //  t = <aisread.aisTypeRef object at 0x7fb39856a940>
  PyObject * memview__arg5_subi = NULL;
  Py_buffer * buf__arg5_subi;
  int * _arg5_subi = NULL;
  PyObject * memview__arg6_subj = NULL;
  Py_buffer * buf__arg6_subj;
  int * _arg6_subj = NULL;
  PyObject * memview__arg7_val = NULL;
  Py_buffer * buf__arg7_val;
  double * _arg7_val = NULL;
  if (! PyArg_ParseTuple(_args,"iiLLOOO",&_arg1_first,&_arg2_last,&_arg3_maxnumnz,&_arg4_surp,&memview__arg5_subi,&memview__arg6_subj,&memview__arg7_val)) goto ERROR;
  if (Py_None != memview__arg5_subi)
  {
    if (! PyMemoryView_Check(memview__arg5_subi))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_subi");
      goto ERROR;
    }
    buf__arg5_subi = PyMemoryView_GET_BUFFER(memview__arg5_subi);
    if (buf__arg5_subi->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_subi");
        goto ERROR;
      
    }
    _arg5_subi = (int*) buf__arg5_subi->buf;
    if (buf__arg5_subi->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg5_subi");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg6_subj)
  {
    if (! PyMemoryView_Check(memview__arg6_subj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_subj");
      goto ERROR;
    }
    buf__arg6_subj = PyMemoryView_GET_BUFFER(memview__arg6_subj);
    if (buf__arg6_subj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_subj");
        goto ERROR;
      
    }
    _arg6_subj = (int*) buf__arg6_subj->buf;
    if (buf__arg6_subj->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg6_subj");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg7_val)
  {
    if (! PyMemoryView_Check(memview__arg7_val))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg7_val");
      goto ERROR;
    }
    buf__arg7_val = PyMemoryView_GET_BUFFER(memview__arg7_val);
    if (buf__arg7_val->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg7_val");
        goto ERROR;
      
    }
    _arg7_val = (double*) buf__arg7_val->buf;
    if (buf__arg7_val->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg7_val");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getacolslicetrip(self->ptr,_arg1_first,_arg2_last,_arg3_maxnumnz,&_arg4_surp,_arg5_subi,_arg6_subj,_arg7_val);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getacolslicetrip */

static PyObject * PyMSK_getconbound(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_i;
  MSKboundkeye _arg2_bk; //  t = <aisread.aisTypeRef object at 0x7fb398570320>
  double _arg3_bl; //  t = <aisread.aisTypeRef object at 0x7fb398570400>
  double _arg4_bu; //  t = <aisread.aisTypeRef object at 0x7fb398570518>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_i)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getconbound(self->ptr,_arg1_i,&_arg2_bk,&_arg3_bl,&_arg4_bu);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(i(idd))",0,_arg2_bk,_arg3_bl,_arg4_bu);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getconbound */

static PyObject * PyMSK_getvarbound(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_i;
  MSKboundkeye _arg2_bk; //  t = <aisread.aisTypeRef object at 0x7fb398570978>
  double _arg3_bl; //  t = <aisread.aisTypeRef object at 0x7fb398570a58>
  double _arg4_bu; //  t = <aisread.aisTypeRef object at 0x7fb398570b70>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_i)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getvarbound(self->ptr,_arg1_i,&_arg2_bk,&_arg3_bl,&_arg4_bu);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(i(idd))",0,_arg2_bk,_arg3_bl,_arg4_bu);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getvarbound */

static PyObject * PyMSK_getconboundslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_first;
  int _arg2_last;
  PyObject * memview__arg3_bk = NULL;
  Py_buffer * buf__arg3_bk;
  MSKboundkeye * _arg3_bk = NULL;
  PyObject * memview__arg4_bl = NULL;
  Py_buffer * buf__arg4_bl;
  double * _arg4_bl = NULL;
  PyObject * memview__arg5_bu = NULL;
  Py_buffer * buf__arg5_bu;
  double * _arg5_bu = NULL;
  if (! PyArg_ParseTuple(_args,"iiOOO",&_arg1_first,&_arg2_last,&memview__arg3_bk,&memview__arg4_bl,&memview__arg5_bu)) goto ERROR;
  if (Py_None != memview__arg3_bk)
  {
    if (! PyMemoryView_Check(memview__arg3_bk))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_bk");
      goto ERROR;
    }
    buf__arg3_bk = PyMemoryView_GET_BUFFER(memview__arg3_bk);
    if (buf__arg3_bk->strides[0] != sizeof(MSKboundkeye))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_bk");
        goto ERROR;
      
    }
    _arg3_bk = (MSKboundkeye*) buf__arg3_bk->buf;
    if (buf__arg3_bk->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg3_bk");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg4_bl)
  {
    if (! PyMemoryView_Check(memview__arg4_bl))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_bl");
      goto ERROR;
    }
    buf__arg4_bl = PyMemoryView_GET_BUFFER(memview__arg4_bl);
    if (buf__arg4_bl->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_bl");
        goto ERROR;
      
    }
    _arg4_bl = (double*) buf__arg4_bl->buf;
    if (buf__arg4_bl->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_bl");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg5_bu)
  {
    if (! PyMemoryView_Check(memview__arg5_bu))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_bu");
      goto ERROR;
    }
    buf__arg5_bu = PyMemoryView_GET_BUFFER(memview__arg5_bu);
    if (buf__arg5_bu->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_bu");
        goto ERROR;
      
    }
    _arg5_bu = (double*) buf__arg5_bu->buf;
    if (buf__arg5_bu->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg5_bu");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getconboundslice(self->ptr,_arg1_first,_arg2_last,_arg3_bk,_arg4_bl,_arg5_bu);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getconboundslice */

static PyObject * PyMSK_getvarboundslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_first;
  int _arg2_last;
  PyObject * memview__arg3_bk = NULL;
  Py_buffer * buf__arg3_bk;
  MSKboundkeye * _arg3_bk = NULL;
  PyObject * memview__arg4_bl = NULL;
  Py_buffer * buf__arg4_bl;
  double * _arg4_bl = NULL;
  PyObject * memview__arg5_bu = NULL;
  Py_buffer * buf__arg5_bu;
  double * _arg5_bu = NULL;
  if (! PyArg_ParseTuple(_args,"iiOOO",&_arg1_first,&_arg2_last,&memview__arg3_bk,&memview__arg4_bl,&memview__arg5_bu)) goto ERROR;
  if (Py_None != memview__arg3_bk)
  {
    if (! PyMemoryView_Check(memview__arg3_bk))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_bk");
      goto ERROR;
    }
    buf__arg3_bk = PyMemoryView_GET_BUFFER(memview__arg3_bk);
    if (buf__arg3_bk->strides[0] != sizeof(MSKboundkeye))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_bk");
        goto ERROR;
      
    }
    _arg3_bk = (MSKboundkeye*) buf__arg3_bk->buf;
    if (buf__arg3_bk->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg3_bk");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg4_bl)
  {
    if (! PyMemoryView_Check(memview__arg4_bl))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_bl");
      goto ERROR;
    }
    buf__arg4_bl = PyMemoryView_GET_BUFFER(memview__arg4_bl);
    if (buf__arg4_bl->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_bl");
        goto ERROR;
      
    }
    _arg4_bl = (double*) buf__arg4_bl->buf;
    if (buf__arg4_bl->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_bl");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg5_bu)
  {
    if (! PyMemoryView_Check(memview__arg5_bu))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_bu");
      goto ERROR;
    }
    buf__arg5_bu = PyMemoryView_GET_BUFFER(memview__arg5_bu);
    if (buf__arg5_bu->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_bu");
        goto ERROR;
      
    }
    _arg5_bu = (double*) buf__arg5_bu->buf;
    if (buf__arg5_bu->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg5_bu");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getvarboundslice(self->ptr,_arg1_first,_arg2_last,_arg3_bk,_arg4_bl,_arg5_bu);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getvarboundslice */

static PyObject * PyMSK_getcj(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_j;
  double _arg2_cj; //  t = <aisread.aisTypeRef object at 0x7fb3984fc048>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_j)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getcj(self->ptr,_arg1_j,&_arg2_cj);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(id)",0,_arg2_cj);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getcj */

static PyObject * PyMSK_getc(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  PyObject * memview__arg1_c = NULL;
  Py_buffer * buf__arg1_c;
  double * _arg1_c = NULL;
  if (! PyArg_ParseTuple(_args,"O",&memview__arg1_c)) goto ERROR;
  if (Py_None != memview__arg1_c)
  {
    if (! PyMemoryView_Check(memview__arg1_c))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg1_c");
      goto ERROR;
    }
    buf__arg1_c = PyMemoryView_GET_BUFFER(memview__arg1_c);
    if (buf__arg1_c->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg1_c");
        goto ERROR;
      
    }
    _arg1_c = (double*) buf__arg1_c->buf;
    if (buf__arg1_c->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg1_c");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getc(self->ptr,_arg1_c);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getc */

static PyObject * PyMSK_getcfix(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  double _arg1_cfix; //  t = <aisread.aisTypeRef object at 0x7fb3984fcdd8>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getcfix(self->ptr,&_arg1_cfix);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(id)",0,_arg1_cfix);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getcfix */

static PyObject * PyMSK_getcone(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_k;
  MSKconetypee _arg2_ct; //  t = <aisread.aisTypeRef object at 0x7fb3984ff2b0>
  double _arg3_conepar; //  t = <aisread.aisTypeRef object at 0x7fb3984ff390>
  int _arg4_nummem; //  t = <aisread.aisTypeRef object at 0x7fb3984ff4a8>
  PyObject * memview__arg5_submem = NULL;
  Py_buffer * buf__arg5_submem;
  int * _arg5_submem = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_k,&memview__arg5_submem)) goto ERROR;
  if (Py_None != memview__arg5_submem)
  {
    if (! PyMemoryView_Check(memview__arg5_submem))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_submem");
      goto ERROR;
    }
    buf__arg5_submem = PyMemoryView_GET_BUFFER(memview__arg5_submem);
    if (buf__arg5_submem->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_submem");
        goto ERROR;
      
    }
    _arg5_submem = (int*) buf__arg5_submem->buf;
    if (buf__arg5_submem->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg5_submem");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getcone(self->ptr,_arg1_k,&_arg2_ct,&_arg3_conepar,&_arg4_nummem,_arg5_submem);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(i(idi))",0,_arg2_ct,_arg3_conepar,_arg4_nummem);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getcone */

static PyObject * PyMSK_getconeinfo(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_k;
  MSKconetypee _arg2_ct; //  t = <aisread.aisTypeRef object at 0x7fb3984ffa58>
  double _arg3_conepar; //  t = <aisread.aisTypeRef object at 0x7fb3984ffb38>
  int _arg4_nummem; //  t = <aisread.aisTypeRef object at 0x7fb3984ffc50>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_k)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getconeinfo(self->ptr,_arg1_k,&_arg2_ct,&_arg3_conepar,&_arg4_nummem);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(i(idi))",0,_arg2_ct,_arg3_conepar,_arg4_nummem);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getconeinfo */

static PyObject * PyMSK_getclist(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  PyObject * memview__arg2_subj = NULL;
  Py_buffer * buf__arg2_subj;
  int * _arg2_subj = NULL;
  PyObject * memview__arg3_c = NULL;
  Py_buffer * buf__arg3_c;
  double * _arg3_c = NULL;
  if (! PyArg_ParseTuple(_args,"iOO",&_arg1_num,&memview__arg2_subj,&memview__arg3_c)) goto ERROR;
  if (Py_None != memview__arg2_subj)
  {
    if (! PyMemoryView_Check(memview__arg2_subj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_subj");
      goto ERROR;
    }
    buf__arg2_subj = PyMemoryView_GET_BUFFER(memview__arg2_subj);
    if (buf__arg2_subj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_subj");
        goto ERROR;
      
    }
    _arg2_subj = (int*) buf__arg2_subj->buf;
  }
  if (Py_None != memview__arg3_c)
  {
    if (! PyMemoryView_Check(memview__arg3_c))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_c");
      goto ERROR;
    }
    buf__arg3_c = PyMemoryView_GET_BUFFER(memview__arg3_c);
    if (buf__arg3_c->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_c");
        goto ERROR;
      
    }
    _arg3_c = (double*) buf__arg3_c->buf;
    if (buf__arg3_c->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg3_c");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getclist(self->ptr,_arg1_num,_arg2_subj,_arg3_c);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getclist */

static PyObject * PyMSK_getcslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_first;
  int _arg2_last;
  PyObject * memview__arg3_c = NULL;
  Py_buffer * buf__arg3_c;
  double * _arg3_c = NULL;
  if (! PyArg_ParseTuple(_args,"iiO",&_arg1_first,&_arg2_last,&memview__arg3_c)) goto ERROR;
  if (Py_None != memview__arg3_c)
  {
    if (! PyMemoryView_Check(memview__arg3_c))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_c");
      goto ERROR;
    }
    buf__arg3_c = PyMemoryView_GET_BUFFER(memview__arg3_c);
    if (buf__arg3_c->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_c");
        goto ERROR;
      
    }
    _arg3_c = (double*) buf__arg3_c->buf;
    if (buf__arg3_c->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg3_c");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getcslice(self->ptr,_arg1_first,_arg2_last,_arg3_c);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getcslice */

static PyObject * PyMSK_getdouinf(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKdinfiteme _arg1_whichdinf;
  double _arg2_dvalue; //  t = <aisread.aisTypeRef object at 0x7fb398506c18>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_whichdinf)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getdouinf(self->ptr,_arg1_whichdinf,&_arg2_dvalue);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(id)",0,_arg2_dvalue);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getdouinf */

static PyObject * PyMSK_getdouparam(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKdparame _arg1_param;
  double _arg2_parvalue; //  t = <aisread.aisTypeRef object at 0x7fb39850b0b8>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_param)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getdouparam(self->ptr,_arg1_param,&_arg2_parvalue);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(id)",0,_arg2_parvalue);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getdouparam */

static PyObject * PyMSK_getdualobj(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  double _arg2_dualobj; //  t = <aisread.aisTypeRef object at 0x7fb39850b518>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_whichsol)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getdualobj(self->ptr,_arg1_whichsol,&_arg2_dualobj);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(id)",0,_arg2_dualobj);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getdualobj */

static PyObject * PyMSK_getintinf(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKiinfiteme _arg1_whichiinf;
  int _arg2_ivalue; //  t = <aisread.aisTypeRef object at 0x7fb398510eb8>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_whichiinf)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getintinf(self->ptr,_arg1_whichiinf,&_arg2_ivalue);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_ivalue);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getintinf */

static PyObject * PyMSK_getlintinf(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKliinfiteme _arg1_whichliinf;
  long long _arg2_ivalue; //  t = <aisread.aisTypeRef object at 0x7fb398514320>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_whichliinf)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getlintinf(self->ptr,_arg1_whichliinf,&_arg2_ivalue);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iL)",0,_arg2_ivalue);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getlintinf */

static PyObject * PyMSK_getintparam(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKiparame _arg1_param;
  int _arg2_parvalue; //  t = <aisread.aisTypeRef object at 0x7fb398514780>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_param)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getintparam(self->ptr,_arg1_param,&_arg2_parvalue);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_parvalue);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getintparam */

static PyObject * PyMSK_getmaxnumanz64(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_maxnumanz; //  t = <aisread.aisTypeRef object at 0x7fb3985194e0>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getmaxnumanz64(self->ptr,&_arg1_maxnumanz);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iL)",0,_arg1_maxnumanz);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getmaxnumanz64 */

static PyObject * PyMSK_getmaxnumcon(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_maxnumcon; //  t = <aisread.aisTypeRef object at 0x7fb398519860>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getmaxnumcon(self->ptr,&_arg1_maxnumcon);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg1_maxnumcon);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getmaxnumcon */

static PyObject * PyMSK_getmaxnumvar(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_maxnumvar; //  t = <aisread.aisTypeRef object at 0x7fb398519be0>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getmaxnumvar(self->ptr,&_arg1_maxnumvar);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg1_maxnumvar);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getmaxnumvar */

static PyObject * PyMSK_getbarvarnamelen(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_i;
  int _arg2_len; //  t = <aisread.aisTypeRef object at 0x7fb3985224e0>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_i)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getbarvarnamelen(self->ptr,_arg1_i,&_arg2_len);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_len);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getbarvarnamelen */

static PyObject * PyMSK_getbarvarname(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_i;
  int _arg2_sizename;
  PyObject * obj__arg3_name = NULL;
  Py_buffer * buf__arg3_name = NULL;
  if (! PyArg_ParseTuple(_args,"iiO",&_arg1_i,&_arg2_sizename,&obj__arg3_name)) goto ERROR;
  if (! PyMemoryView_Check(obj__arg3_name))
  {
    PyErr_SetString(PyExc_TypeError,"Expected a memoryview object");
    goto ERROR;
  }
  buf__arg3_name = PyMemoryView_GET_BUFFER(obj__arg3_name);
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getbarvarname(self->ptr,_arg1_i,_arg2_sizename,(char*)buf__arg3_name->buf);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iO)",0,Py_None);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getbarvarname */

static PyObject * PyMSK_getbarvarnameindex(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_somename = NULL;
  int _arg2_asgn; //  t = <aisread.aisTypeRef object at 0x7fb398526080>
  int _arg3_index; //  t = <aisread.aisTypeRef object at 0x7fb398526198>
  if (! PyArg_ParseTuple(_args,"s",&str__arg1_somename)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getbarvarnameindex(self->ptr,str__arg1_somename,&_arg2_asgn,&_arg3_index);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(i(ii))",0,_arg2_asgn,_arg3_index);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getbarvarnameindex */

static PyObject * PyMSK_generatevarnames(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  PyObject * memview__arg2_subj = NULL;
  Py_buffer * buf__arg2_subj;
  int * _arg2_subj = NULL;
  char * str__arg3_fmt = NULL;
  int _arg4_ndims;
  PyObject * memview__arg5_dims = NULL;
  Py_buffer * buf__arg5_dims;
  int * _arg5_dims = NULL;
  PyObject * memview__arg6_sp = NULL;
  Py_buffer * buf__arg6_sp;
  long long * _arg6_sp = NULL;
  if (! PyArg_ParseTuple(_args,"iOsiOO",&_arg1_num,&memview__arg2_subj,&str__arg3_fmt,&_arg4_ndims,&memview__arg5_dims,&memview__arg6_sp)) goto ERROR;
  if (Py_None != memview__arg2_subj)
  {
    if (! PyMemoryView_Check(memview__arg2_subj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_subj");
      goto ERROR;
    }
    buf__arg2_subj = PyMemoryView_GET_BUFFER(memview__arg2_subj);
    if (buf__arg2_subj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_subj");
        goto ERROR;
      
    }
    _arg2_subj = (int*) buf__arg2_subj->buf;
  }
  if (Py_None != memview__arg5_dims)
  {
    if (! PyMemoryView_Check(memview__arg5_dims))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_dims");
      goto ERROR;
    }
    buf__arg5_dims = PyMemoryView_GET_BUFFER(memview__arg5_dims);
    if (buf__arg5_dims->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_dims");
        goto ERROR;
      
    }
    _arg5_dims = (int*) buf__arg5_dims->buf;
  }
  if (Py_None != memview__arg6_sp)
  {
    if (! PyMemoryView_Check(memview__arg6_sp))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_sp");
      goto ERROR;
    }
    buf__arg6_sp = PyMemoryView_GET_BUFFER(memview__arg6_sp);
    if (buf__arg6_sp->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_sp");
        goto ERROR;
      
    }
    _arg6_sp = (long long*) buf__arg6_sp->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_generatevarnames(self->ptr,_arg1_num,_arg2_subj,str__arg3_fmt,_arg4_ndims,_arg5_dims,_arg6_sp);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* generatevarnames */

static PyObject * PyMSK_generateconnames(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  PyObject * memview__arg2_subi = NULL;
  Py_buffer * buf__arg2_subi;
  int * _arg2_subi = NULL;
  char * str__arg3_fmt = NULL;
  int _arg4_ndims;
  PyObject * memview__arg5_dims = NULL;
  Py_buffer * buf__arg5_dims;
  int * _arg5_dims = NULL;
  PyObject * memview__arg6_sp = NULL;
  Py_buffer * buf__arg6_sp;
  long long * _arg6_sp = NULL;
  if (! PyArg_ParseTuple(_args,"iOsiOO",&_arg1_num,&memview__arg2_subi,&str__arg3_fmt,&_arg4_ndims,&memview__arg5_dims,&memview__arg6_sp)) goto ERROR;
  if (Py_None != memview__arg2_subi)
  {
    if (! PyMemoryView_Check(memview__arg2_subi))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_subi");
      goto ERROR;
    }
    buf__arg2_subi = PyMemoryView_GET_BUFFER(memview__arg2_subi);
    if (buf__arg2_subi->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_subi");
        goto ERROR;
      
    }
    _arg2_subi = (int*) buf__arg2_subi->buf;
  }
  if (Py_None != memview__arg5_dims)
  {
    if (! PyMemoryView_Check(memview__arg5_dims))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_dims");
      goto ERROR;
    }
    buf__arg5_dims = PyMemoryView_GET_BUFFER(memview__arg5_dims);
    if (buf__arg5_dims->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_dims");
        goto ERROR;
      
    }
    _arg5_dims = (int*) buf__arg5_dims->buf;
  }
  if (Py_None != memview__arg6_sp)
  {
    if (! PyMemoryView_Check(memview__arg6_sp))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_sp");
      goto ERROR;
    }
    buf__arg6_sp = PyMemoryView_GET_BUFFER(memview__arg6_sp);
    if (buf__arg6_sp->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_sp");
        goto ERROR;
      
    }
    _arg6_sp = (long long*) buf__arg6_sp->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_generateconnames(self->ptr,_arg1_num,_arg2_subi,str__arg3_fmt,_arg4_ndims,_arg5_dims,_arg6_sp);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* generateconnames */

static PyObject * PyMSK_generateconenames(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  PyObject * memview__arg2_subk = NULL;
  Py_buffer * buf__arg2_subk;
  int * _arg2_subk = NULL;
  char * str__arg3_fmt = NULL;
  int _arg4_ndims;
  PyObject * memview__arg5_dims = NULL;
  Py_buffer * buf__arg5_dims;
  int * _arg5_dims = NULL;
  PyObject * memview__arg6_sp = NULL;
  Py_buffer * buf__arg6_sp;
  long long * _arg6_sp = NULL;
  if (! PyArg_ParseTuple(_args,"iOsiOO",&_arg1_num,&memview__arg2_subk,&str__arg3_fmt,&_arg4_ndims,&memview__arg5_dims,&memview__arg6_sp)) goto ERROR;
  if (Py_None != memview__arg2_subk)
  {
    if (! PyMemoryView_Check(memview__arg2_subk))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_subk");
      goto ERROR;
    }
    buf__arg2_subk = PyMemoryView_GET_BUFFER(memview__arg2_subk);
    if (buf__arg2_subk->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_subk");
        goto ERROR;
      
    }
    _arg2_subk = (int*) buf__arg2_subk->buf;
  }
  if (Py_None != memview__arg5_dims)
  {
    if (! PyMemoryView_Check(memview__arg5_dims))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_dims");
      goto ERROR;
    }
    buf__arg5_dims = PyMemoryView_GET_BUFFER(memview__arg5_dims);
    if (buf__arg5_dims->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_dims");
        goto ERROR;
      
    }
    _arg5_dims = (int*) buf__arg5_dims->buf;
  }
  if (Py_None != memview__arg6_sp)
  {
    if (! PyMemoryView_Check(memview__arg6_sp))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_sp");
      goto ERROR;
    }
    buf__arg6_sp = PyMemoryView_GET_BUFFER(memview__arg6_sp);
    if (buf__arg6_sp->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_sp");
        goto ERROR;
      
    }
    _arg6_sp = (long long*) buf__arg6_sp->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_generateconenames(self->ptr,_arg1_num,_arg2_subk,str__arg3_fmt,_arg4_ndims,_arg5_dims,_arg6_sp);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* generateconenames */

static PyObject * PyMSK_putconname(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_i;
  char * str__arg2_name = NULL;
  if (! PyArg_ParseTuple(_args,"is",&_arg1_i,&str__arg2_name)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putconname(self->ptr,_arg1_i,str__arg2_name);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putconname */

static PyObject * PyMSK_putvarname(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_j;
  char * str__arg2_name = NULL;
  if (! PyArg_ParseTuple(_args,"is",&_arg1_j,&str__arg2_name)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putvarname(self->ptr,_arg1_j,str__arg2_name);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putvarname */

static PyObject * PyMSK_putconename(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_j;
  char * str__arg2_name = NULL;
  if (! PyArg_ParseTuple(_args,"is",&_arg1_j,&str__arg2_name)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putconename(self->ptr,_arg1_j,str__arg2_name);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putconename */

static PyObject * PyMSK_putbarvarname(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_j;
  char * str__arg2_name = NULL;
  if (! PyArg_ParseTuple(_args,"is",&_arg1_j,&str__arg2_name)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putbarvarname(self->ptr,_arg1_j,str__arg2_name);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putbarvarname */

static PyObject * PyMSK_getvarnamelen(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_i;
  int _arg2_len; //  t = <aisread.aisTypeRef object at 0x7fb3984b63c8>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_i)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getvarnamelen(self->ptr,_arg1_i,&_arg2_len);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_len);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getvarnamelen */

static PyObject * PyMSK_getvarname(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_j;
  int _arg2_sizename;
  PyObject * obj__arg3_name = NULL;
  Py_buffer * buf__arg3_name = NULL;
  if (! PyArg_ParseTuple(_args,"iiO",&_arg1_j,&_arg2_sizename,&obj__arg3_name)) goto ERROR;
  if (! PyMemoryView_Check(obj__arg3_name))
  {
    PyErr_SetString(PyExc_TypeError,"Expected a memoryview object");
    goto ERROR;
  }
  buf__arg3_name = PyMemoryView_GET_BUFFER(obj__arg3_name);
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getvarname(self->ptr,_arg1_j,_arg2_sizename,(char*)buf__arg3_name->buf);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iO)",0,Py_None);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getvarname */

static PyObject * PyMSK_getconnamelen(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_i;
  int _arg2_len; //  t = <aisread.aisTypeRef object at 0x7fb3984b6f60>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_i)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getconnamelen(self->ptr,_arg1_i,&_arg2_len);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_len);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getconnamelen */

static PyObject * PyMSK_getconname(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_i;
  int _arg2_sizename;
  PyObject * obj__arg3_name = NULL;
  Py_buffer * buf__arg3_name = NULL;
  if (! PyArg_ParseTuple(_args,"iiO",&_arg1_i,&_arg2_sizename,&obj__arg3_name)) goto ERROR;
  if (! PyMemoryView_Check(obj__arg3_name))
  {
    PyErr_SetString(PyExc_TypeError,"Expected a memoryview object");
    goto ERROR;
  }
  buf__arg3_name = PyMemoryView_GET_BUFFER(obj__arg3_name);
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getconname(self->ptr,_arg1_i,_arg2_sizename,(char*)buf__arg3_name->buf);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iO)",0,Py_None);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getconname */

static PyObject * PyMSK_getconnameindex(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_somename = NULL;
  int _arg2_asgn; //  t = <aisread.aisTypeRef object at 0x7fb3984bdb00>
  int _arg3_index; //  t = <aisread.aisTypeRef object at 0x7fb3984bdc18>
  if (! PyArg_ParseTuple(_args,"s",&str__arg1_somename)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getconnameindex(self->ptr,str__arg1_somename,&_arg2_asgn,&_arg3_index);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(i(ii))",0,_arg2_asgn,_arg3_index);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getconnameindex */

static PyObject * PyMSK_getvarnameindex(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_somename = NULL;
  int _arg2_asgn; //  t = <aisread.aisTypeRef object at 0x7fb3984c10b8>
  int _arg3_index; //  t = <aisread.aisTypeRef object at 0x7fb3984c11d0>
  if (! PyArg_ParseTuple(_args,"s",&str__arg1_somename)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getvarnameindex(self->ptr,str__arg1_somename,&_arg2_asgn,&_arg3_index);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(i(ii))",0,_arg2_asgn,_arg3_index);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getvarnameindex */

static PyObject * PyMSK_getconenamelen(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_i;
  int _arg2_len; //  t = <aisread.aisTypeRef object at 0x7fb3984c1668>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_i)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getconenamelen(self->ptr,_arg1_i,&_arg2_len);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_len);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getconenamelen */

static PyObject * PyMSK_getconename(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_i;
  int _arg2_sizename;
  PyObject * obj__arg3_name = NULL;
  Py_buffer * buf__arg3_name = NULL;
  if (! PyArg_ParseTuple(_args,"iiO",&_arg1_i,&_arg2_sizename,&obj__arg3_name)) goto ERROR;
  if (! PyMemoryView_Check(obj__arg3_name))
  {
    PyErr_SetString(PyExc_TypeError,"Expected a memoryview object");
    goto ERROR;
  }
  buf__arg3_name = PyMemoryView_GET_BUFFER(obj__arg3_name);
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getconename(self->ptr,_arg1_i,_arg2_sizename,(char*)buf__arg3_name->buf);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iO)",0,Py_None);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getconename */

static PyObject * PyMSK_getconenameindex(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_somename = NULL;
  int _arg2_asgn; //  t = <aisread.aisTypeRef object at 0x7fb3984c7208>
  int _arg3_index; //  t = <aisread.aisTypeRef object at 0x7fb3984c7320>
  if (! PyArg_ParseTuple(_args,"s",&str__arg1_somename)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getconenameindex(self->ptr,str__arg1_somename,&_arg2_asgn,&_arg3_index);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(i(ii))",0,_arg2_asgn,_arg3_index);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getconenameindex */

static PyObject * PyMSK_getnumanz(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_numanz; //  t = <aisread.aisTypeRef object at 0x7fb3984c7e48>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getnumanz(self->ptr,&_arg1_numanz);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg1_numanz);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getnumanz */

static PyObject * PyMSK_getnumanz64(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_numanz; //  t = <aisread.aisTypeRef object at 0x7fb3984cb208>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getnumanz64(self->ptr,&_arg1_numanz);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iL)",0,_arg1_numanz);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getnumanz64 */

static PyObject * PyMSK_getnumcon(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_numcon; //  t = <aisread.aisTypeRef object at 0x7fb3984cb588>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getnumcon(self->ptr,&_arg1_numcon);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg1_numcon);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getnumcon */

static PyObject * PyMSK_getnumcone(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_numcone; //  t = <aisread.aisTypeRef object at 0x7fb3984cb908>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getnumcone(self->ptr,&_arg1_numcone);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg1_numcone);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getnumcone */

static PyObject * PyMSK_getnumconemem(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_k;
  int _arg2_nummem; //  t = <aisread.aisTypeRef object at 0x7fb3984cbd68>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_k)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getnumconemem(self->ptr,_arg1_k,&_arg2_nummem);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_nummem);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getnumconemem */

static PyObject * PyMSK_getnumintvar(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_numintvar; //  t = <aisread.aisTypeRef object at 0x7fb3984d0128>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getnumintvar(self->ptr,&_arg1_numintvar);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg1_numintvar);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getnumintvar */

static PyObject * PyMSK_getnumparam(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKparametertypee _arg1_partype;
  int _arg2_numparam; //  t = <aisread.aisTypeRef object at 0x7fb3984d0550>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_partype)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getnumparam(self->ptr,_arg1_partype,&_arg2_numparam);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_numparam);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getnumparam */

static PyObject * PyMSK_getnumqconknz64(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_k;
  long long _arg2_numqcnz; //  t = <aisread.aisTypeRef object at 0x7fb3984d0f60>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_k)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getnumqconknz64(self->ptr,_arg1_k,&_arg2_numqcnz);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iL)",0,_arg2_numqcnz);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getnumqconknz64 */

static PyObject * PyMSK_getnumqobjnz64(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_numqonz; //  t = <aisread.aisTypeRef object at 0x7fb3984d4828>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getnumqobjnz64(self->ptr,&_arg1_numqonz);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iL)",0,_arg1_numqonz);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getnumqobjnz64 */

static PyObject * PyMSK_getnumvar(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_numvar; //  t = <aisread.aisTypeRef object at 0x7fb3984d4ba8>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getnumvar(self->ptr,&_arg1_numvar);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg1_numvar);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getnumvar */

static PyObject * PyMSK_getnumbarvar(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_numbarvar; //  t = <aisread.aisTypeRef object at 0x7fb3984d4f28>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getnumbarvar(self->ptr,&_arg1_numbarvar);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg1_numbarvar);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getnumbarvar */

static PyObject * PyMSK_getmaxnumbarvar(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_maxnumbarvar; //  t = <aisread.aisTypeRef object at 0x7fb3984d92e8>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getmaxnumbarvar(self->ptr,&_arg1_maxnumbarvar);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg1_maxnumbarvar);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getmaxnumbarvar */

static PyObject * PyMSK_getdimbarvarj(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_j;
  int _arg2_dimbarvarj; //  t = <aisread.aisTypeRef object at 0x7fb3984d9748>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_j)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getdimbarvarj(self->ptr,_arg1_j,&_arg2_dimbarvarj);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_dimbarvarj);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getdimbarvarj */

static PyObject * PyMSK_getlenbarvarj(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_j;
  long long _arg2_lenbarvarj; //  t = <aisread.aisTypeRef object at 0x7fb3984d9ba8>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_j)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getlenbarvarj(self->ptr,_arg1_j,&_arg2_lenbarvarj);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iL)",0,_arg2_lenbarvarj);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getlenbarvarj */

static PyObject * PyMSK_getobjname(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_sizeobjname;
  PyObject * obj__arg2_objname = NULL;
  Py_buffer * buf__arg2_objname = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_sizeobjname,&obj__arg2_objname)) goto ERROR;
  if (! PyMemoryView_Check(obj__arg2_objname))
  {
    PyErr_SetString(PyExc_TypeError,"Expected a memoryview object");
    goto ERROR;
  }
  buf__arg2_objname = PyMemoryView_GET_BUFFER(obj__arg2_objname);
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getobjname(self->ptr,_arg1_sizeobjname,(char*)buf__arg2_objname->buf);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iO)",0,Py_None);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getobjname */

static PyObject * PyMSK_getobjnamelen(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_len; //  t = <aisread.aisTypeRef object at 0x7fb3984dd438>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getobjnamelen(self->ptr,&_arg1_len);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg1_len);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getobjnamelen */

static PyObject * PyMSK_getprimalobj(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  double _arg2_primalobj; //  t = <aisread.aisTypeRef object at 0x7fb3984e3358>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_whichsol)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getprimalobj(self->ptr,_arg1_whichsol,&_arg2_primalobj);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(id)",0,_arg2_primalobj);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getprimalobj */

static PyObject * PyMSK_getprobtype(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKproblemtypee _arg1_probtype; //  t = <aisread.aisTypeRef object at 0x7fb3984e36d8>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getprobtype(self->ptr,&_arg1_probtype);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg1_probtype);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getprobtype */

static PyObject * PyMSK_getqconk64(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_k;
  long long _arg2_maxnumqcnz;
  long long _arg3_qcsurp; //  t = <aisread.aisTypeRef object at 0x7fb3984e3c50>
  long long _arg4_numqcnz; //  t = <aisread.aisTypeRef object at 0x7fb3984e3dd8>
  PyObject * memview__arg5_qcsubi = NULL;
  Py_buffer * buf__arg5_qcsubi;
  int * _arg5_qcsubi = NULL;
  PyObject * memview__arg6_qcsubj = NULL;
  Py_buffer * buf__arg6_qcsubj;
  int * _arg6_qcsubj = NULL;
  PyObject * memview__arg7_qcval = NULL;
  Py_buffer * buf__arg7_qcval;
  double * _arg7_qcval = NULL;
  if (! PyArg_ParseTuple(_args,"iLLOOO",&_arg1_k,&_arg2_maxnumqcnz,&_arg3_qcsurp,&memview__arg5_qcsubi,&memview__arg6_qcsubj,&memview__arg7_qcval)) goto ERROR;
  if (Py_None != memview__arg5_qcsubi)
  {
    if (! PyMemoryView_Check(memview__arg5_qcsubi))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_qcsubi");
      goto ERROR;
    }
    buf__arg5_qcsubi = PyMemoryView_GET_BUFFER(memview__arg5_qcsubi);
    if (buf__arg5_qcsubi->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_qcsubi");
        goto ERROR;
      
    }
    _arg5_qcsubi = (int*) buf__arg5_qcsubi->buf;
    if (buf__arg5_qcsubi->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg5_qcsubi");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg6_qcsubj)
  {
    if (! PyMemoryView_Check(memview__arg6_qcsubj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_qcsubj");
      goto ERROR;
    }
    buf__arg6_qcsubj = PyMemoryView_GET_BUFFER(memview__arg6_qcsubj);
    if (buf__arg6_qcsubj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_qcsubj");
        goto ERROR;
      
    }
    _arg6_qcsubj = (int*) buf__arg6_qcsubj->buf;
    if (buf__arg6_qcsubj->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg6_qcsubj");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg7_qcval)
  {
    if (! PyMemoryView_Check(memview__arg7_qcval))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg7_qcval");
      goto ERROR;
    }
    buf__arg7_qcval = PyMemoryView_GET_BUFFER(memview__arg7_qcval);
    if (buf__arg7_qcval->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg7_qcval");
        goto ERROR;
      
    }
    _arg7_qcval = (double*) buf__arg7_qcval->buf;
    if (buf__arg7_qcval->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg7_qcval");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getqconk64(self->ptr,_arg1_k,_arg2_maxnumqcnz,&_arg3_qcsurp,&_arg4_numqcnz,_arg5_qcsubi,_arg6_qcsubj,_arg7_qcval);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iL)",0,_arg4_numqcnz);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getqconk64 */

static PyObject * PyMSK_getqobj64(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_maxnumqonz;
  long long _arg2_qosurp; //  t = <aisread.aisTypeRef object at 0x7fb3984efef0>
  long long _arg3_numqonz; //  t = <aisread.aisTypeRef object at 0x7fb3984f50b8>
  PyObject * memview__arg4_qosubi = NULL;
  Py_buffer * buf__arg4_qosubi;
  int * _arg4_qosubi = NULL;
  PyObject * memview__arg5_qosubj = NULL;
  Py_buffer * buf__arg5_qosubj;
  int * _arg5_qosubj = NULL;
  PyObject * memview__arg6_qoval = NULL;
  Py_buffer * buf__arg6_qoval;
  double * _arg6_qoval = NULL;
  if (! PyArg_ParseTuple(_args,"LLOOO",&_arg1_maxnumqonz,&_arg2_qosurp,&memview__arg4_qosubi,&memview__arg5_qosubj,&memview__arg6_qoval)) goto ERROR;
  if (Py_None != memview__arg4_qosubi)
  {
    if (! PyMemoryView_Check(memview__arg4_qosubi))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_qosubi");
      goto ERROR;
    }
    buf__arg4_qosubi = PyMemoryView_GET_BUFFER(memview__arg4_qosubi);
    if (buf__arg4_qosubi->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_qosubi");
        goto ERROR;
      
    }
    _arg4_qosubi = (int*) buf__arg4_qosubi->buf;
    if (buf__arg4_qosubi->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_qosubi");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg5_qosubj)
  {
    if (! PyMemoryView_Check(memview__arg5_qosubj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_qosubj");
      goto ERROR;
    }
    buf__arg5_qosubj = PyMemoryView_GET_BUFFER(memview__arg5_qosubj);
    if (buf__arg5_qosubj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_qosubj");
        goto ERROR;
      
    }
    _arg5_qosubj = (int*) buf__arg5_qosubj->buf;
    if (buf__arg5_qosubj->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg5_qosubj");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg6_qoval)
  {
    if (! PyMemoryView_Check(memview__arg6_qoval))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_qoval");
      goto ERROR;
    }
    buf__arg6_qoval = PyMemoryView_GET_BUFFER(memview__arg6_qoval);
    if (buf__arg6_qoval->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_qoval");
        goto ERROR;
      
    }
    _arg6_qoval = (double*) buf__arg6_qoval->buf;
    if (buf__arg6_qoval->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg6_qoval");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getqobj64(self->ptr,_arg1_maxnumqonz,&_arg2_qosurp,&_arg3_numqonz,_arg4_qosubi,_arg5_qosubj,_arg6_qoval);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iL)",0,_arg3_numqonz);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getqobj64 */

static PyObject * PyMSK_getqobjij(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_i;
  int _arg2_j;
  double _arg3_qoij; //  t = <aisread.aisTypeRef object at 0x7fb3984f5ac8>
  if (! PyArg_ParseTuple(_args,"ii",&_arg1_i,&_arg2_j)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getqobjij(self->ptr,_arg1_i,_arg2_j,&_arg3_qoij);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(id)",0,_arg3_qoij);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getqobjij */

static PyObject * PyMSK_getsolution(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  MSKprostae _arg2_prosta; //  t = <aisread.aisTypeRef object at 0x7fb3984f5f28>
  MSKsolstae _arg3_solsta; //  t = <aisread.aisTypeRef object at 0x7fb398479080>
  PyObject * memview__arg4_skc = NULL;
  Py_buffer * buf__arg4_skc;
  MSKstakeye * _arg4_skc = NULL;
  PyObject * memview__arg5_skx = NULL;
  Py_buffer * buf__arg5_skx;
  MSKstakeye * _arg5_skx = NULL;
  PyObject * memview__arg6_skn = NULL;
  Py_buffer * buf__arg6_skn;
  MSKstakeye * _arg6_skn = NULL;
  PyObject * memview__arg7_xc = NULL;
  Py_buffer * buf__arg7_xc;
  double * _arg7_xc = NULL;
  PyObject * memview__arg8_xx = NULL;
  Py_buffer * buf__arg8_xx;
  double * _arg8_xx = NULL;
  PyObject * memview__arg9_y = NULL;
  Py_buffer * buf__arg9_y;
  double * _arg9_y = NULL;
  PyObject * memview__arg10_slc = NULL;
  Py_buffer * buf__arg10_slc;
  double * _arg10_slc = NULL;
  PyObject * memview__arg11_suc = NULL;
  Py_buffer * buf__arg11_suc;
  double * _arg11_suc = NULL;
  PyObject * memview__arg12_slx = NULL;
  Py_buffer * buf__arg12_slx;
  double * _arg12_slx = NULL;
  PyObject * memview__arg13_sux = NULL;
  Py_buffer * buf__arg13_sux;
  double * _arg13_sux = NULL;
  PyObject * memview__arg14_snx = NULL;
  Py_buffer * buf__arg14_snx;
  double * _arg14_snx = NULL;
  if (! PyArg_ParseTuple(_args,"iOOOOOOOOOOO",&_arg1_whichsol,&memview__arg4_skc,&memview__arg5_skx,&memview__arg6_skn,&memview__arg7_xc,&memview__arg8_xx,&memview__arg9_y,&memview__arg10_slc,&memview__arg11_suc,&memview__arg12_slx,&memview__arg13_sux,&memview__arg14_snx)) goto ERROR;
  if (Py_None != memview__arg4_skc)
  {
    if (! PyMemoryView_Check(memview__arg4_skc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_skc");
      goto ERROR;
    }
    buf__arg4_skc = PyMemoryView_GET_BUFFER(memview__arg4_skc);
    if (buf__arg4_skc->strides[0] != sizeof(MSKstakeye))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_skc");
        goto ERROR;
      
    }
    _arg4_skc = (MSKstakeye*) buf__arg4_skc->buf;
    if (buf__arg4_skc->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_skc");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg5_skx)
  {
    if (! PyMemoryView_Check(memview__arg5_skx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_skx");
      goto ERROR;
    }
    buf__arg5_skx = PyMemoryView_GET_BUFFER(memview__arg5_skx);
    if (buf__arg5_skx->strides[0] != sizeof(MSKstakeye))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_skx");
        goto ERROR;
      
    }
    _arg5_skx = (MSKstakeye*) buf__arg5_skx->buf;
    if (buf__arg5_skx->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg5_skx");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg6_skn)
  {
    if (! PyMemoryView_Check(memview__arg6_skn))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_skn");
      goto ERROR;
    }
    buf__arg6_skn = PyMemoryView_GET_BUFFER(memview__arg6_skn);
    if (buf__arg6_skn->strides[0] != sizeof(MSKstakeye))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_skn");
        goto ERROR;
      
    }
    _arg6_skn = (MSKstakeye*) buf__arg6_skn->buf;
    if (buf__arg6_skn->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg6_skn");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg7_xc)
  {
    if (! PyMemoryView_Check(memview__arg7_xc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg7_xc");
      goto ERROR;
    }
    buf__arg7_xc = PyMemoryView_GET_BUFFER(memview__arg7_xc);
    if (buf__arg7_xc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg7_xc");
        goto ERROR;
      
    }
    _arg7_xc = (double*) buf__arg7_xc->buf;
    if (buf__arg7_xc->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg7_xc");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg8_xx)
  {
    if (! PyMemoryView_Check(memview__arg8_xx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg8_xx");
      goto ERROR;
    }
    buf__arg8_xx = PyMemoryView_GET_BUFFER(memview__arg8_xx);
    if (buf__arg8_xx->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg8_xx");
        goto ERROR;
      
    }
    _arg8_xx = (double*) buf__arg8_xx->buf;
    if (buf__arg8_xx->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg8_xx");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg9_y)
  {
    if (! PyMemoryView_Check(memview__arg9_y))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg9_y");
      goto ERROR;
    }
    buf__arg9_y = PyMemoryView_GET_BUFFER(memview__arg9_y);
    if (buf__arg9_y->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg9_y");
        goto ERROR;
      
    }
    _arg9_y = (double*) buf__arg9_y->buf;
    if (buf__arg9_y->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg9_y");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg10_slc)
  {
    if (! PyMemoryView_Check(memview__arg10_slc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg10_slc");
      goto ERROR;
    }
    buf__arg10_slc = PyMemoryView_GET_BUFFER(memview__arg10_slc);
    if (buf__arg10_slc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg10_slc");
        goto ERROR;
      
    }
    _arg10_slc = (double*) buf__arg10_slc->buf;
    if (buf__arg10_slc->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg10_slc");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg11_suc)
  {
    if (! PyMemoryView_Check(memview__arg11_suc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg11_suc");
      goto ERROR;
    }
    buf__arg11_suc = PyMemoryView_GET_BUFFER(memview__arg11_suc);
    if (buf__arg11_suc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg11_suc");
        goto ERROR;
      
    }
    _arg11_suc = (double*) buf__arg11_suc->buf;
    if (buf__arg11_suc->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg11_suc");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg12_slx)
  {
    if (! PyMemoryView_Check(memview__arg12_slx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg12_slx");
      goto ERROR;
    }
    buf__arg12_slx = PyMemoryView_GET_BUFFER(memview__arg12_slx);
    if (buf__arg12_slx->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg12_slx");
        goto ERROR;
      
    }
    _arg12_slx = (double*) buf__arg12_slx->buf;
    if (buf__arg12_slx->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg12_slx");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg13_sux)
  {
    if (! PyMemoryView_Check(memview__arg13_sux))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg13_sux");
      goto ERROR;
    }
    buf__arg13_sux = PyMemoryView_GET_BUFFER(memview__arg13_sux);
    if (buf__arg13_sux->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg13_sux");
        goto ERROR;
      
    }
    _arg13_sux = (double*) buf__arg13_sux->buf;
    if (buf__arg13_sux->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg13_sux");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg14_snx)
  {
    if (! PyMemoryView_Check(memview__arg14_snx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg14_snx");
      goto ERROR;
    }
    buf__arg14_snx = PyMemoryView_GET_BUFFER(memview__arg14_snx);
    if (buf__arg14_snx->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg14_snx");
        goto ERROR;
      
    }
    _arg14_snx = (double*) buf__arg14_snx->buf;
    if (buf__arg14_snx->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg14_snx");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getsolution(self->ptr,_arg1_whichsol,&_arg2_prosta,&_arg3_solsta,_arg4_skc,_arg5_skx,_arg6_skn,_arg7_xc,_arg8_xx,_arg9_y,_arg10_slc,_arg11_suc,_arg12_slx,_arg13_sux,_arg14_snx);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(i(ii))",0,_arg2_prosta,_arg3_solsta);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getsolution */

static PyObject * PyMSK_getsolsta(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  MSKsolstae _arg2_solsta; //  t = <aisread.aisTypeRef object at 0x7fb398480438>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_whichsol)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getsolsta(self->ptr,_arg1_whichsol,&_arg2_solsta);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_solsta);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getsolsta */

static PyObject * PyMSK_getprosta(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  MSKprostae _arg2_prosta; //  t = <aisread.aisTypeRef object at 0x7fb398480898>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_whichsol)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getprosta(self->ptr,_arg1_whichsol,&_arg2_prosta);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_prosta);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getprosta */

static PyObject * PyMSK_getskc(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  PyObject * memview__arg2_skc = NULL;
  Py_buffer * buf__arg2_skc;
  MSKstakeye * _arg2_skc = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_whichsol,&memview__arg2_skc)) goto ERROR;
  if (Py_None != memview__arg2_skc)
  {
    if (! PyMemoryView_Check(memview__arg2_skc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_skc");
      goto ERROR;
    }
    buf__arg2_skc = PyMemoryView_GET_BUFFER(memview__arg2_skc);
    if (buf__arg2_skc->strides[0] != sizeof(MSKstakeye))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_skc");
        goto ERROR;
      
    }
    _arg2_skc = (MSKstakeye*) buf__arg2_skc->buf;
    if (buf__arg2_skc->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg2_skc");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getskc(self->ptr,_arg1_whichsol,_arg2_skc);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getskc */

static PyObject * PyMSK_getskx(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  PyObject * memview__arg2_skx = NULL;
  Py_buffer * buf__arg2_skx;
  MSKstakeye * _arg2_skx = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_whichsol,&memview__arg2_skx)) goto ERROR;
  if (Py_None != memview__arg2_skx)
  {
    if (! PyMemoryView_Check(memview__arg2_skx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_skx");
      goto ERROR;
    }
    buf__arg2_skx = PyMemoryView_GET_BUFFER(memview__arg2_skx);
    if (buf__arg2_skx->strides[0] != sizeof(MSKstakeye))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_skx");
        goto ERROR;
      
    }
    _arg2_skx = (MSKstakeye*) buf__arg2_skx->buf;
    if (buf__arg2_skx->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg2_skx");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getskx(self->ptr,_arg1_whichsol,_arg2_skx);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getskx */

static PyObject * PyMSK_getskn(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  PyObject * memview__arg2_skn = NULL;
  Py_buffer * buf__arg2_skn;
  MSKstakeye * _arg2_skn = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_whichsol,&memview__arg2_skn)) goto ERROR;
  if (Py_None != memview__arg2_skn)
  {
    if (! PyMemoryView_Check(memview__arg2_skn))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_skn");
      goto ERROR;
    }
    buf__arg2_skn = PyMemoryView_GET_BUFFER(memview__arg2_skn);
    if (buf__arg2_skn->strides[0] != sizeof(MSKstakeye))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_skn");
        goto ERROR;
      
    }
    _arg2_skn = (MSKstakeye*) buf__arg2_skn->buf;
    if (buf__arg2_skn->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg2_skn");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getskn(self->ptr,_arg1_whichsol,_arg2_skn);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getskn */

static PyObject * PyMSK_getxc(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  PyObject * memview__arg2_xc = NULL;
  Py_buffer * buf__arg2_xc;
  double * _arg2_xc = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_whichsol,&memview__arg2_xc)) goto ERROR;
  if (Py_None != memview__arg2_xc)
  {
    if (! PyMemoryView_Check(memview__arg2_xc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_xc");
      goto ERROR;
    }
    buf__arg2_xc = PyMemoryView_GET_BUFFER(memview__arg2_xc);
    if (buf__arg2_xc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_xc");
        goto ERROR;
      
    }
    _arg2_xc = (double*) buf__arg2_xc->buf;
    if (buf__arg2_xc->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg2_xc");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getxc(self->ptr,_arg1_whichsol,_arg2_xc);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getxc */

static PyObject * PyMSK_getxx(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  PyObject * memview__arg2_xx = NULL;
  Py_buffer * buf__arg2_xx;
  double * _arg2_xx = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_whichsol,&memview__arg2_xx)) goto ERROR;
  if (Py_None != memview__arg2_xx)
  {
    if (! PyMemoryView_Check(memview__arg2_xx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_xx");
      goto ERROR;
    }
    buf__arg2_xx = PyMemoryView_GET_BUFFER(memview__arg2_xx);
    if (buf__arg2_xx->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_xx");
        goto ERROR;
      
    }
    _arg2_xx = (double*) buf__arg2_xx->buf;
    if (buf__arg2_xx->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg2_xx");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getxx(self->ptr,_arg1_whichsol,_arg2_xx);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getxx */

static PyObject * PyMSK_gety(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  PyObject * memview__arg2_y = NULL;
  Py_buffer * buf__arg2_y;
  double * _arg2_y = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_whichsol,&memview__arg2_y)) goto ERROR;
  if (Py_None != memview__arg2_y)
  {
    if (! PyMemoryView_Check(memview__arg2_y))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_y");
      goto ERROR;
    }
    buf__arg2_y = PyMemoryView_GET_BUFFER(memview__arg2_y);
    if (buf__arg2_y->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_y");
        goto ERROR;
      
    }
    _arg2_y = (double*) buf__arg2_y->buf;
    if (buf__arg2_y->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg2_y");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_gety(self->ptr,_arg1_whichsol,_arg2_y);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* gety */

static PyObject * PyMSK_getslc(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  PyObject * memview__arg2_slc = NULL;
  Py_buffer * buf__arg2_slc;
  double * _arg2_slc = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_whichsol,&memview__arg2_slc)) goto ERROR;
  if (Py_None != memview__arg2_slc)
  {
    if (! PyMemoryView_Check(memview__arg2_slc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_slc");
      goto ERROR;
    }
    buf__arg2_slc = PyMemoryView_GET_BUFFER(memview__arg2_slc);
    if (buf__arg2_slc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_slc");
        goto ERROR;
      
    }
    _arg2_slc = (double*) buf__arg2_slc->buf;
    if (buf__arg2_slc->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg2_slc");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getslc(self->ptr,_arg1_whichsol,_arg2_slc);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getslc */

static PyObject * PyMSK_getsuc(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  PyObject * memview__arg2_suc = NULL;
  Py_buffer * buf__arg2_suc;
  double * _arg2_suc = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_whichsol,&memview__arg2_suc)) goto ERROR;
  if (Py_None != memview__arg2_suc)
  {
    if (! PyMemoryView_Check(memview__arg2_suc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_suc");
      goto ERROR;
    }
    buf__arg2_suc = PyMemoryView_GET_BUFFER(memview__arg2_suc);
    if (buf__arg2_suc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_suc");
        goto ERROR;
      
    }
    _arg2_suc = (double*) buf__arg2_suc->buf;
    if (buf__arg2_suc->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg2_suc");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getsuc(self->ptr,_arg1_whichsol,_arg2_suc);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getsuc */

static PyObject * PyMSK_getslx(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  PyObject * memview__arg2_slx = NULL;
  Py_buffer * buf__arg2_slx;
  double * _arg2_slx = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_whichsol,&memview__arg2_slx)) goto ERROR;
  if (Py_None != memview__arg2_slx)
  {
    if (! PyMemoryView_Check(memview__arg2_slx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_slx");
      goto ERROR;
    }
    buf__arg2_slx = PyMemoryView_GET_BUFFER(memview__arg2_slx);
    if (buf__arg2_slx->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_slx");
        goto ERROR;
      
    }
    _arg2_slx = (double*) buf__arg2_slx->buf;
    if (buf__arg2_slx->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg2_slx");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getslx(self->ptr,_arg1_whichsol,_arg2_slx);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getslx */

static PyObject * PyMSK_getsux(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  PyObject * memview__arg2_sux = NULL;
  Py_buffer * buf__arg2_sux;
  double * _arg2_sux = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_whichsol,&memview__arg2_sux)) goto ERROR;
  if (Py_None != memview__arg2_sux)
  {
    if (! PyMemoryView_Check(memview__arg2_sux))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_sux");
      goto ERROR;
    }
    buf__arg2_sux = PyMemoryView_GET_BUFFER(memview__arg2_sux);
    if (buf__arg2_sux->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_sux");
        goto ERROR;
      
    }
    _arg2_sux = (double*) buf__arg2_sux->buf;
    if (buf__arg2_sux->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg2_sux");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getsux(self->ptr,_arg1_whichsol,_arg2_sux);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getsux */

static PyObject * PyMSK_getsnx(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  PyObject * memview__arg2_snx = NULL;
  Py_buffer * buf__arg2_snx;
  double * _arg2_snx = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_whichsol,&memview__arg2_snx)) goto ERROR;
  if (Py_None != memview__arg2_snx)
  {
    if (! PyMemoryView_Check(memview__arg2_snx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_snx");
      goto ERROR;
    }
    buf__arg2_snx = PyMemoryView_GET_BUFFER(memview__arg2_snx);
    if (buf__arg2_snx->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_snx");
        goto ERROR;
      
    }
    _arg2_snx = (double*) buf__arg2_snx->buf;
    if (buf__arg2_snx->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg2_snx");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getsnx(self->ptr,_arg1_whichsol,_arg2_snx);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getsnx */

static PyObject * PyMSK_getskcslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_first;
  int _arg3_last;
  PyObject * memview__arg4_skc = NULL;
  Py_buffer * buf__arg4_skc;
  MSKstakeye * _arg4_skc = NULL;
  if (! PyArg_ParseTuple(_args,"iiiO",&_arg1_whichsol,&_arg2_first,&_arg3_last,&memview__arg4_skc)) goto ERROR;
  if (Py_None != memview__arg4_skc)
  {
    if (! PyMemoryView_Check(memview__arg4_skc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_skc");
      goto ERROR;
    }
    buf__arg4_skc = PyMemoryView_GET_BUFFER(memview__arg4_skc);
    if (buf__arg4_skc->strides[0] != sizeof(MSKstakeye))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_skc");
        goto ERROR;
      
    }
    _arg4_skc = (MSKstakeye*) buf__arg4_skc->buf;
    if (buf__arg4_skc->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_skc");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getskcslice(self->ptr,_arg1_whichsol,_arg2_first,_arg3_last,_arg4_skc);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getskcslice */

static PyObject * PyMSK_getskxslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_first;
  int _arg3_last;
  PyObject * memview__arg4_skx = NULL;
  Py_buffer * buf__arg4_skx;
  MSKstakeye * _arg4_skx = NULL;
  if (! PyArg_ParseTuple(_args,"iiiO",&_arg1_whichsol,&_arg2_first,&_arg3_last,&memview__arg4_skx)) goto ERROR;
  if (Py_None != memview__arg4_skx)
  {
    if (! PyMemoryView_Check(memview__arg4_skx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_skx");
      goto ERROR;
    }
    buf__arg4_skx = PyMemoryView_GET_BUFFER(memview__arg4_skx);
    if (buf__arg4_skx->strides[0] != sizeof(MSKstakeye))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_skx");
        goto ERROR;
      
    }
    _arg4_skx = (MSKstakeye*) buf__arg4_skx->buf;
    if (buf__arg4_skx->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_skx");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getskxslice(self->ptr,_arg1_whichsol,_arg2_first,_arg3_last,_arg4_skx);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getskxslice */

static PyObject * PyMSK_getxcslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_first;
  int _arg3_last;
  PyObject * memview__arg4_xc = NULL;
  Py_buffer * buf__arg4_xc;
  double * _arg4_xc = NULL;
  if (! PyArg_ParseTuple(_args,"iiiO",&_arg1_whichsol,&_arg2_first,&_arg3_last,&memview__arg4_xc)) goto ERROR;
  if (Py_None != memview__arg4_xc)
  {
    if (! PyMemoryView_Check(memview__arg4_xc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_xc");
      goto ERROR;
    }
    buf__arg4_xc = PyMemoryView_GET_BUFFER(memview__arg4_xc);
    if (buf__arg4_xc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_xc");
        goto ERROR;
      
    }
    _arg4_xc = (double*) buf__arg4_xc->buf;
    if (buf__arg4_xc->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_xc");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getxcslice(self->ptr,_arg1_whichsol,_arg2_first,_arg3_last,_arg4_xc);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getxcslice */

static PyObject * PyMSK_getxxslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_first;
  int _arg3_last;
  PyObject * memview__arg4_xx = NULL;
  Py_buffer * buf__arg4_xx;
  double * _arg4_xx = NULL;
  if (! PyArg_ParseTuple(_args,"iiiO",&_arg1_whichsol,&_arg2_first,&_arg3_last,&memview__arg4_xx)) goto ERROR;
  if (Py_None != memview__arg4_xx)
  {
    if (! PyMemoryView_Check(memview__arg4_xx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_xx");
      goto ERROR;
    }
    buf__arg4_xx = PyMemoryView_GET_BUFFER(memview__arg4_xx);
    if (buf__arg4_xx->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_xx");
        goto ERROR;
      
    }
    _arg4_xx = (double*) buf__arg4_xx->buf;
    if (buf__arg4_xx->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_xx");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getxxslice(self->ptr,_arg1_whichsol,_arg2_first,_arg3_last,_arg4_xx);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getxxslice */

static PyObject * PyMSK_getyslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_first;
  int _arg3_last;
  PyObject * memview__arg4_y = NULL;
  Py_buffer * buf__arg4_y;
  double * _arg4_y = NULL;
  if (! PyArg_ParseTuple(_args,"iiiO",&_arg1_whichsol,&_arg2_first,&_arg3_last,&memview__arg4_y)) goto ERROR;
  if (Py_None != memview__arg4_y)
  {
    if (! PyMemoryView_Check(memview__arg4_y))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_y");
      goto ERROR;
    }
    buf__arg4_y = PyMemoryView_GET_BUFFER(memview__arg4_y);
    if (buf__arg4_y->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_y");
        goto ERROR;
      
    }
    _arg4_y = (double*) buf__arg4_y->buf;
    if (buf__arg4_y->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_y");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getyslice(self->ptr,_arg1_whichsol,_arg2_first,_arg3_last,_arg4_y);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getyslice */

static PyObject * PyMSK_getslcslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_first;
  int _arg3_last;
  PyObject * memview__arg4_slc = NULL;
  Py_buffer * buf__arg4_slc;
  double * _arg4_slc = NULL;
  if (! PyArg_ParseTuple(_args,"iiiO",&_arg1_whichsol,&_arg2_first,&_arg3_last,&memview__arg4_slc)) goto ERROR;
  if (Py_None != memview__arg4_slc)
  {
    if (! PyMemoryView_Check(memview__arg4_slc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_slc");
      goto ERROR;
    }
    buf__arg4_slc = PyMemoryView_GET_BUFFER(memview__arg4_slc);
    if (buf__arg4_slc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_slc");
        goto ERROR;
      
    }
    _arg4_slc = (double*) buf__arg4_slc->buf;
    if (buf__arg4_slc->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_slc");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getslcslice(self->ptr,_arg1_whichsol,_arg2_first,_arg3_last,_arg4_slc);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getslcslice */

static PyObject * PyMSK_getsucslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_first;
  int _arg3_last;
  PyObject * memview__arg4_suc = NULL;
  Py_buffer * buf__arg4_suc;
  double * _arg4_suc = NULL;
  if (! PyArg_ParseTuple(_args,"iiiO",&_arg1_whichsol,&_arg2_first,&_arg3_last,&memview__arg4_suc)) goto ERROR;
  if (Py_None != memview__arg4_suc)
  {
    if (! PyMemoryView_Check(memview__arg4_suc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_suc");
      goto ERROR;
    }
    buf__arg4_suc = PyMemoryView_GET_BUFFER(memview__arg4_suc);
    if (buf__arg4_suc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_suc");
        goto ERROR;
      
    }
    _arg4_suc = (double*) buf__arg4_suc->buf;
    if (buf__arg4_suc->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_suc");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getsucslice(self->ptr,_arg1_whichsol,_arg2_first,_arg3_last,_arg4_suc);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getsucslice */

static PyObject * PyMSK_getslxslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_first;
  int _arg3_last;
  PyObject * memview__arg4_slx = NULL;
  Py_buffer * buf__arg4_slx;
  double * _arg4_slx = NULL;
  if (! PyArg_ParseTuple(_args,"iiiO",&_arg1_whichsol,&_arg2_first,&_arg3_last,&memview__arg4_slx)) goto ERROR;
  if (Py_None != memview__arg4_slx)
  {
    if (! PyMemoryView_Check(memview__arg4_slx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_slx");
      goto ERROR;
    }
    buf__arg4_slx = PyMemoryView_GET_BUFFER(memview__arg4_slx);
    if (buf__arg4_slx->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_slx");
        goto ERROR;
      
    }
    _arg4_slx = (double*) buf__arg4_slx->buf;
    if (buf__arg4_slx->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_slx");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getslxslice(self->ptr,_arg1_whichsol,_arg2_first,_arg3_last,_arg4_slx);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getslxslice */

static PyObject * PyMSK_getsuxslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_first;
  int _arg3_last;
  PyObject * memview__arg4_sux = NULL;
  Py_buffer * buf__arg4_sux;
  double * _arg4_sux = NULL;
  if (! PyArg_ParseTuple(_args,"iiiO",&_arg1_whichsol,&_arg2_first,&_arg3_last,&memview__arg4_sux)) goto ERROR;
  if (Py_None != memview__arg4_sux)
  {
    if (! PyMemoryView_Check(memview__arg4_sux))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_sux");
      goto ERROR;
    }
    buf__arg4_sux = PyMemoryView_GET_BUFFER(memview__arg4_sux);
    if (buf__arg4_sux->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_sux");
        goto ERROR;
      
    }
    _arg4_sux = (double*) buf__arg4_sux->buf;
    if (buf__arg4_sux->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_sux");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getsuxslice(self->ptr,_arg1_whichsol,_arg2_first,_arg3_last,_arg4_sux);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getsuxslice */

static PyObject * PyMSK_getsnxslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_first;
  int _arg3_last;
  PyObject * memview__arg4_snx = NULL;
  Py_buffer * buf__arg4_snx;
  double * _arg4_snx = NULL;
  if (! PyArg_ParseTuple(_args,"iiiO",&_arg1_whichsol,&_arg2_first,&_arg3_last,&memview__arg4_snx)) goto ERROR;
  if (Py_None != memview__arg4_snx)
  {
    if (! PyMemoryView_Check(memview__arg4_snx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_snx");
      goto ERROR;
    }
    buf__arg4_snx = PyMemoryView_GET_BUFFER(memview__arg4_snx);
    if (buf__arg4_snx->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_snx");
        goto ERROR;
      
    }
    _arg4_snx = (double*) buf__arg4_snx->buf;
    if (buf__arg4_snx->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_snx");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getsnxslice(self->ptr,_arg1_whichsol,_arg2_first,_arg3_last,_arg4_snx);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getsnxslice */

static PyObject * PyMSK_getbarxj(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_j;
  PyObject * memview__arg3_barxj = NULL;
  Py_buffer * buf__arg3_barxj;
  double * _arg3_barxj = NULL;
  if (! PyArg_ParseTuple(_args,"iiO",&_arg1_whichsol,&_arg2_j,&memview__arg3_barxj)) goto ERROR;
  if (Py_None != memview__arg3_barxj)
  {
    if (! PyMemoryView_Check(memview__arg3_barxj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_barxj");
      goto ERROR;
    }
    buf__arg3_barxj = PyMemoryView_GET_BUFFER(memview__arg3_barxj);
    if (buf__arg3_barxj->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_barxj");
        goto ERROR;
      
    }
    _arg3_barxj = (double*) buf__arg3_barxj->buf;
    if (buf__arg3_barxj->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg3_barxj");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getbarxj(self->ptr,_arg1_whichsol,_arg2_j,_arg3_barxj);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getbarxj */

static PyObject * PyMSK_getbarxslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_first;
  int _arg3_last;
  long long _arg4_slicesize;
  PyObject * memview__arg5_barxslice = NULL;
  Py_buffer * buf__arg5_barxslice;
  double * _arg5_barxslice = NULL;
  if (! PyArg_ParseTuple(_args,"iiiLO",&_arg1_whichsol,&_arg2_first,&_arg3_last,&_arg4_slicesize,&memview__arg5_barxslice)) goto ERROR;
  if (Py_None != memview__arg5_barxslice)
  {
    if (! PyMemoryView_Check(memview__arg5_barxslice))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_barxslice");
      goto ERROR;
    }
    buf__arg5_barxslice = PyMemoryView_GET_BUFFER(memview__arg5_barxslice);
    if (buf__arg5_barxslice->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_barxslice");
        goto ERROR;
      
    }
    _arg5_barxslice = (double*) buf__arg5_barxslice->buf;
    if (buf__arg5_barxslice->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg5_barxslice");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getbarxslice(self->ptr,_arg1_whichsol,_arg2_first,_arg3_last,_arg4_slicesize,_arg5_barxslice);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getbarxslice */

static PyObject * PyMSK_getbarsj(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_j;
  PyObject * memview__arg3_barsj = NULL;
  Py_buffer * buf__arg3_barsj;
  double * _arg3_barsj = NULL;
  if (! PyArg_ParseTuple(_args,"iiO",&_arg1_whichsol,&_arg2_j,&memview__arg3_barsj)) goto ERROR;
  if (Py_None != memview__arg3_barsj)
  {
    if (! PyMemoryView_Check(memview__arg3_barsj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_barsj");
      goto ERROR;
    }
    buf__arg3_barsj = PyMemoryView_GET_BUFFER(memview__arg3_barsj);
    if (buf__arg3_barsj->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_barsj");
        goto ERROR;
      
    }
    _arg3_barsj = (double*) buf__arg3_barsj->buf;
    if (buf__arg3_barsj->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg3_barsj");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getbarsj(self->ptr,_arg1_whichsol,_arg2_j,_arg3_barsj);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getbarsj */

static PyObject * PyMSK_getbarsslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_first;
  int _arg3_last;
  long long _arg4_slicesize;
  PyObject * memview__arg5_barsslice = NULL;
  Py_buffer * buf__arg5_barsslice;
  double * _arg5_barsslice = NULL;
  if (! PyArg_ParseTuple(_args,"iiiLO",&_arg1_whichsol,&_arg2_first,&_arg3_last,&_arg4_slicesize,&memview__arg5_barsslice)) goto ERROR;
  if (Py_None != memview__arg5_barsslice)
  {
    if (! PyMemoryView_Check(memview__arg5_barsslice))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_barsslice");
      goto ERROR;
    }
    buf__arg5_barsslice = PyMemoryView_GET_BUFFER(memview__arg5_barsslice);
    if (buf__arg5_barsslice->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_barsslice");
        goto ERROR;
      
    }
    _arg5_barsslice = (double*) buf__arg5_barsslice->buf;
    if (buf__arg5_barsslice->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg5_barsslice");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getbarsslice(self->ptr,_arg1_whichsol,_arg2_first,_arg3_last,_arg4_slicesize,_arg5_barsslice);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getbarsslice */

static PyObject * PyMSK_putskc(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  PyObject * memview__arg2_skc = NULL;
  Py_buffer * buf__arg2_skc;
  MSKstakeye * _arg2_skc = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_whichsol,&memview__arg2_skc)) goto ERROR;
  if (Py_None != memview__arg2_skc)
  {
    if (! PyMemoryView_Check(memview__arg2_skc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_skc");
      goto ERROR;
    }
    buf__arg2_skc = PyMemoryView_GET_BUFFER(memview__arg2_skc);
    if (buf__arg2_skc->strides[0] != sizeof(MSKstakeye))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_skc");
        goto ERROR;
      
    }
    _arg2_skc = (MSKstakeye*) buf__arg2_skc->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putskc(self->ptr,_arg1_whichsol,_arg2_skc);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putskc */

static PyObject * PyMSK_putskx(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  PyObject * memview__arg2_skx = NULL;
  Py_buffer * buf__arg2_skx;
  MSKstakeye * _arg2_skx = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_whichsol,&memview__arg2_skx)) goto ERROR;
  if (Py_None != memview__arg2_skx)
  {
    if (! PyMemoryView_Check(memview__arg2_skx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_skx");
      goto ERROR;
    }
    buf__arg2_skx = PyMemoryView_GET_BUFFER(memview__arg2_skx);
    if (buf__arg2_skx->strides[0] != sizeof(MSKstakeye))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_skx");
        goto ERROR;
      
    }
    _arg2_skx = (MSKstakeye*) buf__arg2_skx->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putskx(self->ptr,_arg1_whichsol,_arg2_skx);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putskx */

static PyObject * PyMSK_putxc(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  PyObject * memview__arg2_xc = NULL;
  Py_buffer * buf__arg2_xc;
  double * _arg2_xc = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_whichsol,&memview__arg2_xc)) goto ERROR;
  if (Py_None != memview__arg2_xc)
  {
    if (! PyMemoryView_Check(memview__arg2_xc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_xc");
      goto ERROR;
    }
    buf__arg2_xc = PyMemoryView_GET_BUFFER(memview__arg2_xc);
    if (buf__arg2_xc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_xc");
        goto ERROR;
      
    }
    _arg2_xc = (double*) buf__arg2_xc->buf;
    if (buf__arg2_xc->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg2_xc");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putxc(self->ptr,_arg1_whichsol,_arg2_xc);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putxc */

static PyObject * PyMSK_putxx(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  PyObject * memview__arg2_xx = NULL;
  Py_buffer * buf__arg2_xx;
  double * _arg2_xx = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_whichsol,&memview__arg2_xx)) goto ERROR;
  if (Py_None != memview__arg2_xx)
  {
    if (! PyMemoryView_Check(memview__arg2_xx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_xx");
      goto ERROR;
    }
    buf__arg2_xx = PyMemoryView_GET_BUFFER(memview__arg2_xx);
    if (buf__arg2_xx->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_xx");
        goto ERROR;
      
    }
    _arg2_xx = (double*) buf__arg2_xx->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putxx(self->ptr,_arg1_whichsol,_arg2_xx);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putxx */

static PyObject * PyMSK_puty(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  PyObject * memview__arg2_y = NULL;
  Py_buffer * buf__arg2_y;
  double * _arg2_y = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_whichsol,&memview__arg2_y)) goto ERROR;
  if (Py_None != memview__arg2_y)
  {
    if (! PyMemoryView_Check(memview__arg2_y))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_y");
      goto ERROR;
    }
    buf__arg2_y = PyMemoryView_GET_BUFFER(memview__arg2_y);
    if (buf__arg2_y->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_y");
        goto ERROR;
      
    }
    _arg2_y = (double*) buf__arg2_y->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_puty(self->ptr,_arg1_whichsol,_arg2_y);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* puty */

static PyObject * PyMSK_putslc(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  PyObject * memview__arg2_slc = NULL;
  Py_buffer * buf__arg2_slc;
  double * _arg2_slc = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_whichsol,&memview__arg2_slc)) goto ERROR;
  if (Py_None != memview__arg2_slc)
  {
    if (! PyMemoryView_Check(memview__arg2_slc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_slc");
      goto ERROR;
    }
    buf__arg2_slc = PyMemoryView_GET_BUFFER(memview__arg2_slc);
    if (buf__arg2_slc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_slc");
        goto ERROR;
      
    }
    _arg2_slc = (double*) buf__arg2_slc->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putslc(self->ptr,_arg1_whichsol,_arg2_slc);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putslc */

static PyObject * PyMSK_putsuc(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  PyObject * memview__arg2_suc = NULL;
  Py_buffer * buf__arg2_suc;
  double * _arg2_suc = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_whichsol,&memview__arg2_suc)) goto ERROR;
  if (Py_None != memview__arg2_suc)
  {
    if (! PyMemoryView_Check(memview__arg2_suc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_suc");
      goto ERROR;
    }
    buf__arg2_suc = PyMemoryView_GET_BUFFER(memview__arg2_suc);
    if (buf__arg2_suc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_suc");
        goto ERROR;
      
    }
    _arg2_suc = (double*) buf__arg2_suc->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putsuc(self->ptr,_arg1_whichsol,_arg2_suc);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putsuc */

static PyObject * PyMSK_putslx(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  PyObject * memview__arg2_slx = NULL;
  Py_buffer * buf__arg2_slx;
  double * _arg2_slx = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_whichsol,&memview__arg2_slx)) goto ERROR;
  if (Py_None != memview__arg2_slx)
  {
    if (! PyMemoryView_Check(memview__arg2_slx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_slx");
      goto ERROR;
    }
    buf__arg2_slx = PyMemoryView_GET_BUFFER(memview__arg2_slx);
    if (buf__arg2_slx->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_slx");
        goto ERROR;
      
    }
    _arg2_slx = (double*) buf__arg2_slx->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putslx(self->ptr,_arg1_whichsol,_arg2_slx);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putslx */

static PyObject * PyMSK_putsux(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  PyObject * memview__arg2_sux = NULL;
  Py_buffer * buf__arg2_sux;
  double * _arg2_sux = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_whichsol,&memview__arg2_sux)) goto ERROR;
  if (Py_None != memview__arg2_sux)
  {
    if (! PyMemoryView_Check(memview__arg2_sux))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_sux");
      goto ERROR;
    }
    buf__arg2_sux = PyMemoryView_GET_BUFFER(memview__arg2_sux);
    if (buf__arg2_sux->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_sux");
        goto ERROR;
      
    }
    _arg2_sux = (double*) buf__arg2_sux->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putsux(self->ptr,_arg1_whichsol,_arg2_sux);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putsux */

static PyObject * PyMSK_putsnx(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  PyObject * memview__arg2_sux = NULL;
  Py_buffer * buf__arg2_sux;
  double * _arg2_sux = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_whichsol,&memview__arg2_sux)) goto ERROR;
  if (Py_None != memview__arg2_sux)
  {
    if (! PyMemoryView_Check(memview__arg2_sux))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_sux");
      goto ERROR;
    }
    buf__arg2_sux = PyMemoryView_GET_BUFFER(memview__arg2_sux);
    if (buf__arg2_sux->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_sux");
        goto ERROR;
      
    }
    _arg2_sux = (double*) buf__arg2_sux->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putsnx(self->ptr,_arg1_whichsol,_arg2_sux);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putsnx */

static PyObject * PyMSK_putskcslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_first;
  int _arg3_last;
  PyObject * memview__arg4_skc = NULL;
  Py_buffer * buf__arg4_skc;
  MSKstakeye * _arg4_skc = NULL;
  if (! PyArg_ParseTuple(_args,"iiiO",&_arg1_whichsol,&_arg2_first,&_arg3_last,&memview__arg4_skc)) goto ERROR;
  if (Py_None != memview__arg4_skc)
  {
    if (! PyMemoryView_Check(memview__arg4_skc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_skc");
      goto ERROR;
    }
    buf__arg4_skc = PyMemoryView_GET_BUFFER(memview__arg4_skc);
    if (buf__arg4_skc->strides[0] != sizeof(MSKstakeye))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_skc");
        goto ERROR;
      
    }
    _arg4_skc = (MSKstakeye*) buf__arg4_skc->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putskcslice(self->ptr,_arg1_whichsol,_arg2_first,_arg3_last,_arg4_skc);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putskcslice */

static PyObject * PyMSK_putskxslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_first;
  int _arg3_last;
  PyObject * memview__arg4_skx = NULL;
  Py_buffer * buf__arg4_skx;
  MSKstakeye * _arg4_skx = NULL;
  if (! PyArg_ParseTuple(_args,"iiiO",&_arg1_whichsol,&_arg2_first,&_arg3_last,&memview__arg4_skx)) goto ERROR;
  if (Py_None != memview__arg4_skx)
  {
    if (! PyMemoryView_Check(memview__arg4_skx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_skx");
      goto ERROR;
    }
    buf__arg4_skx = PyMemoryView_GET_BUFFER(memview__arg4_skx);
    if (buf__arg4_skx->strides[0] != sizeof(MSKstakeye))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_skx");
        goto ERROR;
      
    }
    _arg4_skx = (MSKstakeye*) buf__arg4_skx->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putskxslice(self->ptr,_arg1_whichsol,_arg2_first,_arg3_last,_arg4_skx);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putskxslice */

static PyObject * PyMSK_putxcslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_first;
  int _arg3_last;
  PyObject * memview__arg4_xc = NULL;
  Py_buffer * buf__arg4_xc;
  double * _arg4_xc = NULL;
  if (! PyArg_ParseTuple(_args,"iiiO",&_arg1_whichsol,&_arg2_first,&_arg3_last,&memview__arg4_xc)) goto ERROR;
  if (Py_None != memview__arg4_xc)
  {
    if (! PyMemoryView_Check(memview__arg4_xc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_xc");
      goto ERROR;
    }
    buf__arg4_xc = PyMemoryView_GET_BUFFER(memview__arg4_xc);
    if (buf__arg4_xc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_xc");
        goto ERROR;
      
    }
    _arg4_xc = (double*) buf__arg4_xc->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putxcslice(self->ptr,_arg1_whichsol,_arg2_first,_arg3_last,_arg4_xc);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putxcslice */

static PyObject * PyMSK_putxxslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_first;
  int _arg3_last;
  PyObject * memview__arg4_xx = NULL;
  Py_buffer * buf__arg4_xx;
  double * _arg4_xx = NULL;
  if (! PyArg_ParseTuple(_args,"iiiO",&_arg1_whichsol,&_arg2_first,&_arg3_last,&memview__arg4_xx)) goto ERROR;
  if (Py_None != memview__arg4_xx)
  {
    if (! PyMemoryView_Check(memview__arg4_xx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_xx");
      goto ERROR;
    }
    buf__arg4_xx = PyMemoryView_GET_BUFFER(memview__arg4_xx);
    if (buf__arg4_xx->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_xx");
        goto ERROR;
      
    }
    _arg4_xx = (double*) buf__arg4_xx->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putxxslice(self->ptr,_arg1_whichsol,_arg2_first,_arg3_last,_arg4_xx);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putxxslice */

static PyObject * PyMSK_putyslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_first;
  int _arg3_last;
  PyObject * memview__arg4_y = NULL;
  Py_buffer * buf__arg4_y;
  double * _arg4_y = NULL;
  if (! PyArg_ParseTuple(_args,"iiiO",&_arg1_whichsol,&_arg2_first,&_arg3_last,&memview__arg4_y)) goto ERROR;
  if (Py_None != memview__arg4_y)
  {
    if (! PyMemoryView_Check(memview__arg4_y))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_y");
      goto ERROR;
    }
    buf__arg4_y = PyMemoryView_GET_BUFFER(memview__arg4_y);
    if (buf__arg4_y->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_y");
        goto ERROR;
      
    }
    _arg4_y = (double*) buf__arg4_y->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putyslice(self->ptr,_arg1_whichsol,_arg2_first,_arg3_last,_arg4_y);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putyslice */

static PyObject * PyMSK_putslcslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_first;
  int _arg3_last;
  PyObject * memview__arg4_slc = NULL;
  Py_buffer * buf__arg4_slc;
  double * _arg4_slc = NULL;
  if (! PyArg_ParseTuple(_args,"iiiO",&_arg1_whichsol,&_arg2_first,&_arg3_last,&memview__arg4_slc)) goto ERROR;
  if (Py_None != memview__arg4_slc)
  {
    if (! PyMemoryView_Check(memview__arg4_slc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_slc");
      goto ERROR;
    }
    buf__arg4_slc = PyMemoryView_GET_BUFFER(memview__arg4_slc);
    if (buf__arg4_slc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_slc");
        goto ERROR;
      
    }
    _arg4_slc = (double*) buf__arg4_slc->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putslcslice(self->ptr,_arg1_whichsol,_arg2_first,_arg3_last,_arg4_slc);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putslcslice */

static PyObject * PyMSK_putsucslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_first;
  int _arg3_last;
  PyObject * memview__arg4_suc = NULL;
  Py_buffer * buf__arg4_suc;
  double * _arg4_suc = NULL;
  if (! PyArg_ParseTuple(_args,"iiiO",&_arg1_whichsol,&_arg2_first,&_arg3_last,&memview__arg4_suc)) goto ERROR;
  if (Py_None != memview__arg4_suc)
  {
    if (! PyMemoryView_Check(memview__arg4_suc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_suc");
      goto ERROR;
    }
    buf__arg4_suc = PyMemoryView_GET_BUFFER(memview__arg4_suc);
    if (buf__arg4_suc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_suc");
        goto ERROR;
      
    }
    _arg4_suc = (double*) buf__arg4_suc->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putsucslice(self->ptr,_arg1_whichsol,_arg2_first,_arg3_last,_arg4_suc);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putsucslice */

static PyObject * PyMSK_putslxslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_first;
  int _arg3_last;
  PyObject * memview__arg4_slx = NULL;
  Py_buffer * buf__arg4_slx;
  double * _arg4_slx = NULL;
  if (! PyArg_ParseTuple(_args,"iiiO",&_arg1_whichsol,&_arg2_first,&_arg3_last,&memview__arg4_slx)) goto ERROR;
  if (Py_None != memview__arg4_slx)
  {
    if (! PyMemoryView_Check(memview__arg4_slx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_slx");
      goto ERROR;
    }
    buf__arg4_slx = PyMemoryView_GET_BUFFER(memview__arg4_slx);
    if (buf__arg4_slx->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_slx");
        goto ERROR;
      
    }
    _arg4_slx = (double*) buf__arg4_slx->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putslxslice(self->ptr,_arg1_whichsol,_arg2_first,_arg3_last,_arg4_slx);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putslxslice */

static PyObject * PyMSK_putsuxslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_first;
  int _arg3_last;
  PyObject * memview__arg4_sux = NULL;
  Py_buffer * buf__arg4_sux;
  double * _arg4_sux = NULL;
  if (! PyArg_ParseTuple(_args,"iiiO",&_arg1_whichsol,&_arg2_first,&_arg3_last,&memview__arg4_sux)) goto ERROR;
  if (Py_None != memview__arg4_sux)
  {
    if (! PyMemoryView_Check(memview__arg4_sux))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_sux");
      goto ERROR;
    }
    buf__arg4_sux = PyMemoryView_GET_BUFFER(memview__arg4_sux);
    if (buf__arg4_sux->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_sux");
        goto ERROR;
      
    }
    _arg4_sux = (double*) buf__arg4_sux->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putsuxslice(self->ptr,_arg1_whichsol,_arg2_first,_arg3_last,_arg4_sux);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putsuxslice */

static PyObject * PyMSK_putsnxslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_first;
  int _arg3_last;
  PyObject * memview__arg4_snx = NULL;
  Py_buffer * buf__arg4_snx;
  double * _arg4_snx = NULL;
  if (! PyArg_ParseTuple(_args,"iiiO",&_arg1_whichsol,&_arg2_first,&_arg3_last,&memview__arg4_snx)) goto ERROR;
  if (Py_None != memview__arg4_snx)
  {
    if (! PyMemoryView_Check(memview__arg4_snx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_snx");
      goto ERROR;
    }
    buf__arg4_snx = PyMemoryView_GET_BUFFER(memview__arg4_snx);
    if (buf__arg4_snx->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_snx");
        goto ERROR;
      
    }
    _arg4_snx = (double*) buf__arg4_snx->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putsnxslice(self->ptr,_arg1_whichsol,_arg2_first,_arg3_last,_arg4_snx);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putsnxslice */

static PyObject * PyMSK_putbarxj(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_j;
  PyObject * memview__arg3_barxj = NULL;
  Py_buffer * buf__arg3_barxj;
  double * _arg3_barxj = NULL;
  if (! PyArg_ParseTuple(_args,"iiO",&_arg1_whichsol,&_arg2_j,&memview__arg3_barxj)) goto ERROR;
  if (Py_None != memview__arg3_barxj)
  {
    if (! PyMemoryView_Check(memview__arg3_barxj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_barxj");
      goto ERROR;
    }
    buf__arg3_barxj = PyMemoryView_GET_BUFFER(memview__arg3_barxj);
    if (buf__arg3_barxj->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_barxj");
        goto ERROR;
      
    }
    _arg3_barxj = (double*) buf__arg3_barxj->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putbarxj(self->ptr,_arg1_whichsol,_arg2_j,_arg3_barxj);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putbarxj */

static PyObject * PyMSK_putbarsj(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_j;
  PyObject * memview__arg3_barsj = NULL;
  Py_buffer * buf__arg3_barsj;
  double * _arg3_barsj = NULL;
  if (! PyArg_ParseTuple(_args,"iiO",&_arg1_whichsol,&_arg2_j,&memview__arg3_barsj)) goto ERROR;
  if (Py_None != memview__arg3_barsj)
  {
    if (! PyMemoryView_Check(memview__arg3_barsj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_barsj");
      goto ERROR;
    }
    buf__arg3_barsj = PyMemoryView_GET_BUFFER(memview__arg3_barsj);
    if (buf__arg3_barsj->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_barsj");
        goto ERROR;
      
    }
    _arg3_barsj = (double*) buf__arg3_barsj->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putbarsj(self->ptr,_arg1_whichsol,_arg2_j,_arg3_barsj);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putbarsj */

static PyObject * PyMSK_getpviolcon(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_num;
  PyObject * memview__arg3_sub = NULL;
  Py_buffer * buf__arg3_sub;
  int * _arg3_sub = NULL;
  PyObject * memview__arg4_viol = NULL;
  Py_buffer * buf__arg4_viol;
  double * _arg4_viol = NULL;
  if (! PyArg_ParseTuple(_args,"iiOO",&_arg1_whichsol,&_arg2_num,&memview__arg3_sub,&memview__arg4_viol)) goto ERROR;
  if (Py_None != memview__arg3_sub)
  {
    if (! PyMemoryView_Check(memview__arg3_sub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_sub");
      goto ERROR;
    }
    buf__arg3_sub = PyMemoryView_GET_BUFFER(memview__arg3_sub);
    if (buf__arg3_sub->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_sub");
        goto ERROR;
      
    }
    _arg3_sub = (int*) buf__arg3_sub->buf;
  }
  if (Py_None != memview__arg4_viol)
  {
    if (! PyMemoryView_Check(memview__arg4_viol))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_viol");
      goto ERROR;
    }
    buf__arg4_viol = PyMemoryView_GET_BUFFER(memview__arg4_viol);
    if (buf__arg4_viol->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_viol");
        goto ERROR;
      
    }
    _arg4_viol = (double*) buf__arg4_viol->buf;
    if (buf__arg4_viol->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_viol");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getpviolcon(self->ptr,_arg1_whichsol,_arg2_num,_arg3_sub,_arg4_viol);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getpviolcon */

static PyObject * PyMSK_getpviolvar(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_num;
  PyObject * memview__arg3_sub = NULL;
  Py_buffer * buf__arg3_sub;
  int * _arg3_sub = NULL;
  PyObject * memview__arg4_viol = NULL;
  Py_buffer * buf__arg4_viol;
  double * _arg4_viol = NULL;
  if (! PyArg_ParseTuple(_args,"iiOO",&_arg1_whichsol,&_arg2_num,&memview__arg3_sub,&memview__arg4_viol)) goto ERROR;
  if (Py_None != memview__arg3_sub)
  {
    if (! PyMemoryView_Check(memview__arg3_sub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_sub");
      goto ERROR;
    }
    buf__arg3_sub = PyMemoryView_GET_BUFFER(memview__arg3_sub);
    if (buf__arg3_sub->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_sub");
        goto ERROR;
      
    }
    _arg3_sub = (int*) buf__arg3_sub->buf;
  }
  if (Py_None != memview__arg4_viol)
  {
    if (! PyMemoryView_Check(memview__arg4_viol))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_viol");
      goto ERROR;
    }
    buf__arg4_viol = PyMemoryView_GET_BUFFER(memview__arg4_viol);
    if (buf__arg4_viol->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_viol");
        goto ERROR;
      
    }
    _arg4_viol = (double*) buf__arg4_viol->buf;
    if (buf__arg4_viol->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_viol");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getpviolvar(self->ptr,_arg1_whichsol,_arg2_num,_arg3_sub,_arg4_viol);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getpviolvar */

static PyObject * PyMSK_getpviolbarvar(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_num;
  PyObject * memview__arg3_sub = NULL;
  Py_buffer * buf__arg3_sub;
  int * _arg3_sub = NULL;
  PyObject * memview__arg4_viol = NULL;
  Py_buffer * buf__arg4_viol;
  double * _arg4_viol = NULL;
  if (! PyArg_ParseTuple(_args,"iiOO",&_arg1_whichsol,&_arg2_num,&memview__arg3_sub,&memview__arg4_viol)) goto ERROR;
  if (Py_None != memview__arg3_sub)
  {
    if (! PyMemoryView_Check(memview__arg3_sub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_sub");
      goto ERROR;
    }
    buf__arg3_sub = PyMemoryView_GET_BUFFER(memview__arg3_sub);
    if (buf__arg3_sub->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_sub");
        goto ERROR;
      
    }
    _arg3_sub = (int*) buf__arg3_sub->buf;
  }
  if (Py_None != memview__arg4_viol)
  {
    if (! PyMemoryView_Check(memview__arg4_viol))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_viol");
      goto ERROR;
    }
    buf__arg4_viol = PyMemoryView_GET_BUFFER(memview__arg4_viol);
    if (buf__arg4_viol->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_viol");
        goto ERROR;
      
    }
    _arg4_viol = (double*) buf__arg4_viol->buf;
    if (buf__arg4_viol->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_viol");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getpviolbarvar(self->ptr,_arg1_whichsol,_arg2_num,_arg3_sub,_arg4_viol);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getpviolbarvar */

static PyObject * PyMSK_getpviolcones(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_num;
  PyObject * memview__arg3_sub = NULL;
  Py_buffer * buf__arg3_sub;
  int * _arg3_sub = NULL;
  PyObject * memview__arg4_viol = NULL;
  Py_buffer * buf__arg4_viol;
  double * _arg4_viol = NULL;
  if (! PyArg_ParseTuple(_args,"iiOO",&_arg1_whichsol,&_arg2_num,&memview__arg3_sub,&memview__arg4_viol)) goto ERROR;
  if (Py_None != memview__arg3_sub)
  {
    if (! PyMemoryView_Check(memview__arg3_sub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_sub");
      goto ERROR;
    }
    buf__arg3_sub = PyMemoryView_GET_BUFFER(memview__arg3_sub);
    if (buf__arg3_sub->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_sub");
        goto ERROR;
      
    }
    _arg3_sub = (int*) buf__arg3_sub->buf;
  }
  if (Py_None != memview__arg4_viol)
  {
    if (! PyMemoryView_Check(memview__arg4_viol))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_viol");
      goto ERROR;
    }
    buf__arg4_viol = PyMemoryView_GET_BUFFER(memview__arg4_viol);
    if (buf__arg4_viol->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_viol");
        goto ERROR;
      
    }
    _arg4_viol = (double*) buf__arg4_viol->buf;
    if (buf__arg4_viol->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_viol");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getpviolcones(self->ptr,_arg1_whichsol,_arg2_num,_arg3_sub,_arg4_viol);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getpviolcones */

static PyObject * PyMSK_getdviolcon(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_num;
  PyObject * memview__arg3_sub = NULL;
  Py_buffer * buf__arg3_sub;
  int * _arg3_sub = NULL;
  PyObject * memview__arg4_viol = NULL;
  Py_buffer * buf__arg4_viol;
  double * _arg4_viol = NULL;
  if (! PyArg_ParseTuple(_args,"iiOO",&_arg1_whichsol,&_arg2_num,&memview__arg3_sub,&memview__arg4_viol)) goto ERROR;
  if (Py_None != memview__arg3_sub)
  {
    if (! PyMemoryView_Check(memview__arg3_sub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_sub");
      goto ERROR;
    }
    buf__arg3_sub = PyMemoryView_GET_BUFFER(memview__arg3_sub);
    if (buf__arg3_sub->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_sub");
        goto ERROR;
      
    }
    _arg3_sub = (int*) buf__arg3_sub->buf;
  }
  if (Py_None != memview__arg4_viol)
  {
    if (! PyMemoryView_Check(memview__arg4_viol))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_viol");
      goto ERROR;
    }
    buf__arg4_viol = PyMemoryView_GET_BUFFER(memview__arg4_viol);
    if (buf__arg4_viol->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_viol");
        goto ERROR;
      
    }
    _arg4_viol = (double*) buf__arg4_viol->buf;
    if (buf__arg4_viol->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_viol");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getdviolcon(self->ptr,_arg1_whichsol,_arg2_num,_arg3_sub,_arg4_viol);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getdviolcon */

static PyObject * PyMSK_getdviolvar(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_num;
  PyObject * memview__arg3_sub = NULL;
  Py_buffer * buf__arg3_sub;
  int * _arg3_sub = NULL;
  PyObject * memview__arg4_viol = NULL;
  Py_buffer * buf__arg4_viol;
  double * _arg4_viol = NULL;
  if (! PyArg_ParseTuple(_args,"iiOO",&_arg1_whichsol,&_arg2_num,&memview__arg3_sub,&memview__arg4_viol)) goto ERROR;
  if (Py_None != memview__arg3_sub)
  {
    if (! PyMemoryView_Check(memview__arg3_sub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_sub");
      goto ERROR;
    }
    buf__arg3_sub = PyMemoryView_GET_BUFFER(memview__arg3_sub);
    if (buf__arg3_sub->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_sub");
        goto ERROR;
      
    }
    _arg3_sub = (int*) buf__arg3_sub->buf;
  }
  if (Py_None != memview__arg4_viol)
  {
    if (! PyMemoryView_Check(memview__arg4_viol))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_viol");
      goto ERROR;
    }
    buf__arg4_viol = PyMemoryView_GET_BUFFER(memview__arg4_viol);
    if (buf__arg4_viol->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_viol");
        goto ERROR;
      
    }
    _arg4_viol = (double*) buf__arg4_viol->buf;
    if (buf__arg4_viol->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_viol");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getdviolvar(self->ptr,_arg1_whichsol,_arg2_num,_arg3_sub,_arg4_viol);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getdviolvar */

static PyObject * PyMSK_getdviolbarvar(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_num;
  PyObject * memview__arg3_sub = NULL;
  Py_buffer * buf__arg3_sub;
  int * _arg3_sub = NULL;
  PyObject * memview__arg4_viol = NULL;
  Py_buffer * buf__arg4_viol;
  double * _arg4_viol = NULL;
  if (! PyArg_ParseTuple(_args,"iiOO",&_arg1_whichsol,&_arg2_num,&memview__arg3_sub,&memview__arg4_viol)) goto ERROR;
  if (Py_None != memview__arg3_sub)
  {
    if (! PyMemoryView_Check(memview__arg3_sub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_sub");
      goto ERROR;
    }
    buf__arg3_sub = PyMemoryView_GET_BUFFER(memview__arg3_sub);
    if (buf__arg3_sub->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_sub");
        goto ERROR;
      
    }
    _arg3_sub = (int*) buf__arg3_sub->buf;
  }
  if (Py_None != memview__arg4_viol)
  {
    if (! PyMemoryView_Check(memview__arg4_viol))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_viol");
      goto ERROR;
    }
    buf__arg4_viol = PyMemoryView_GET_BUFFER(memview__arg4_viol);
    if (buf__arg4_viol->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_viol");
        goto ERROR;
      
    }
    _arg4_viol = (double*) buf__arg4_viol->buf;
    if (buf__arg4_viol->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_viol");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getdviolbarvar(self->ptr,_arg1_whichsol,_arg2_num,_arg3_sub,_arg4_viol);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getdviolbarvar */

static PyObject * PyMSK_getdviolcones(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_num;
  PyObject * memview__arg3_sub = NULL;
  Py_buffer * buf__arg3_sub;
  int * _arg3_sub = NULL;
  PyObject * memview__arg4_viol = NULL;
  Py_buffer * buf__arg4_viol;
  double * _arg4_viol = NULL;
  if (! PyArg_ParseTuple(_args,"iiOO",&_arg1_whichsol,&_arg2_num,&memview__arg3_sub,&memview__arg4_viol)) goto ERROR;
  if (Py_None != memview__arg3_sub)
  {
    if (! PyMemoryView_Check(memview__arg3_sub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_sub");
      goto ERROR;
    }
    buf__arg3_sub = PyMemoryView_GET_BUFFER(memview__arg3_sub);
    if (buf__arg3_sub->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_sub");
        goto ERROR;
      
    }
    _arg3_sub = (int*) buf__arg3_sub->buf;
  }
  if (Py_None != memview__arg4_viol)
  {
    if (! PyMemoryView_Check(memview__arg4_viol))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_viol");
      goto ERROR;
    }
    buf__arg4_viol = PyMemoryView_GET_BUFFER(memview__arg4_viol);
    if (buf__arg4_viol->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_viol");
        goto ERROR;
      
    }
    _arg4_viol = (double*) buf__arg4_viol->buf;
    if (buf__arg4_viol->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_viol");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getdviolcones(self->ptr,_arg1_whichsol,_arg2_num,_arg3_sub,_arg4_viol);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getdviolcones */

static PyObject * PyMSK_getsolutioninfo(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  double _arg2_pobj; //  t = <aisread.aisTypeRef object at 0x7fb398470ef0>
  double _arg3_pviolcon; //  t = <aisread.aisTypeRef object at 0x7fb398475048>
  double _arg4_pviolvar; //  t = <aisread.aisTypeRef object at 0x7fb398475160>
  double _arg5_pviolbarvar; //  t = <aisread.aisTypeRef object at 0x7fb398475278>
  double _arg6_pviolcone; //  t = <aisread.aisTypeRef object at 0x7fb398475390>
  double _arg7_pviolitg; //  t = <aisread.aisTypeRef object at 0x7fb3984754a8>
  double _arg8_dobj; //  t = <aisread.aisTypeRef object at 0x7fb3984755c0>
  double _arg9_dviolcon; //  t = <aisread.aisTypeRef object at 0x7fb3984756d8>
  double _arg10_dviolvar; //  t = <aisread.aisTypeRef object at 0x7fb3984757f0>
  double _arg11_dviolbarvar; //  t = <aisread.aisTypeRef object at 0x7fb398475908>
  double _arg12_dviolcone; //  t = <aisread.aisTypeRef object at 0x7fb398475a20>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_whichsol)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getsolutioninfo(self->ptr,_arg1_whichsol,&_arg2_pobj,&_arg3_pviolcon,&_arg4_pviolvar,&_arg5_pviolbarvar,&_arg6_pviolcone,&_arg7_pviolitg,&_arg8_dobj,&_arg9_dviolcon,&_arg10_dviolvar,&_arg11_dviolbarvar,&_arg12_dviolcone);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(i(ddddddddddd))",0,_arg2_pobj,_arg3_pviolcon,_arg4_pviolvar,_arg5_pviolbarvar,_arg6_pviolcone,_arg7_pviolitg,_arg8_dobj,_arg9_dviolcon,_arg10_dviolvar,_arg11_dviolbarvar,_arg12_dviolcone);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getsolutioninfo */

static PyObject * PyMSK_getdualsolutionnorms(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  double _arg2_nrmy; //  t = <aisread.aisTypeRef object at 0x7fb3983fa5f8>
  double _arg3_nrmslc; //  t = <aisread.aisTypeRef object at 0x7fb3983fa710>
  double _arg4_nrmsuc; //  t = <aisread.aisTypeRef object at 0x7fb3983fa828>
  double _arg5_nrmslx; //  t = <aisread.aisTypeRef object at 0x7fb3983fa940>
  double _arg6_nrmsux; //  t = <aisread.aisTypeRef object at 0x7fb3983faa58>
  double _arg7_nrmsnx; //  t = <aisread.aisTypeRef object at 0x7fb3983fab70>
  double _arg8_nrmbars; //  t = <aisread.aisTypeRef object at 0x7fb3983fac88>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_whichsol)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getdualsolutionnorms(self->ptr,_arg1_whichsol,&_arg2_nrmy,&_arg3_nrmslc,&_arg4_nrmsuc,&_arg5_nrmslx,&_arg6_nrmsux,&_arg7_nrmsnx,&_arg8_nrmbars);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(i(ddddddd))",0,_arg2_nrmy,_arg3_nrmslc,_arg4_nrmsuc,_arg5_nrmslx,_arg6_nrmsux,_arg7_nrmsnx,_arg8_nrmbars);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getdualsolutionnorms */

static PyObject * PyMSK_getprimalsolutionnorms(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  double _arg2_nrmxc; //  t = <aisread.aisTypeRef object at 0x7fb3983ff128>
  double _arg3_nrmxx; //  t = <aisread.aisTypeRef object at 0x7fb3983ff240>
  double _arg4_nrmbarx; //  t = <aisread.aisTypeRef object at 0x7fb3983ff358>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_whichsol)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getprimalsolutionnorms(self->ptr,_arg1_whichsol,&_arg2_nrmxc,&_arg3_nrmxx,&_arg4_nrmbarx);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(i(ddd))",0,_arg2_nrmxc,_arg3_nrmxx,_arg4_nrmbarx);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getprimalsolutionnorms */

static PyObject * PyMSK_getsolutionslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  MSKsoliteme _arg2_solitem;
  int _arg3_first;
  int _arg4_last;
  PyObject * memview__arg5_values = NULL;
  Py_buffer * buf__arg5_values;
  double * _arg5_values = NULL;
  if (! PyArg_ParseTuple(_args,"iiiiO",&_arg1_whichsol,&_arg2_solitem,&_arg3_first,&_arg4_last,&memview__arg5_values)) goto ERROR;
  if (Py_None != memview__arg5_values)
  {
    if (! PyMemoryView_Check(memview__arg5_values))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_values");
      goto ERROR;
    }
    buf__arg5_values = PyMemoryView_GET_BUFFER(memview__arg5_values);
    if (buf__arg5_values->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_values");
        goto ERROR;
      
    }
    _arg5_values = (double*) buf__arg5_values->buf;
    if (buf__arg5_values->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg5_values");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getsolutionslice(self->ptr,_arg1_whichsol,_arg2_solitem,_arg3_first,_arg4_last,_arg5_values);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getsolutionslice */

static PyObject * PyMSK_getreducedcosts(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_first;
  int _arg3_last;
  PyObject * memview__arg4_redcosts = NULL;
  Py_buffer * buf__arg4_redcosts;
  double * _arg4_redcosts = NULL;
  if (! PyArg_ParseTuple(_args,"iiiO",&_arg1_whichsol,&_arg2_first,&_arg3_last,&memview__arg4_redcosts)) goto ERROR;
  if (Py_None != memview__arg4_redcosts)
  {
    if (! PyMemoryView_Check(memview__arg4_redcosts))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_redcosts");
      goto ERROR;
    }
    buf__arg4_redcosts = PyMemoryView_GET_BUFFER(memview__arg4_redcosts);
    if (buf__arg4_redcosts->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_redcosts");
        goto ERROR;
      
    }
    _arg4_redcosts = (double*) buf__arg4_redcosts->buf;
    if (buf__arg4_redcosts->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_redcosts");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getreducedcosts(self->ptr,_arg1_whichsol,_arg2_first,_arg3_last,_arg4_redcosts);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getreducedcosts */

static PyObject * PyMSK_getstrparam(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsparame _arg1_param;
  int _arg2_maxlen;
  int _arg3_len; //  t = <aisread.aisTypeRef object at 0x7fb398404780>
  PyObject * obj__arg4_parvalue = NULL;
  Py_buffer * buf__arg4_parvalue = NULL;
  if (! PyArg_ParseTuple(_args,"iiO",&_arg1_param,&_arg2_maxlen,&obj__arg4_parvalue)) goto ERROR;
  if (! PyMemoryView_Check(obj__arg4_parvalue))
  {
    PyErr_SetString(PyExc_TypeError,"Expected a memoryview object");
    goto ERROR;
  }
  buf__arg4_parvalue = PyMemoryView_GET_BUFFER(obj__arg4_parvalue);
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getstrparam(self->ptr,_arg1_param,_arg2_maxlen,&_arg3_len,(char*)buf__arg4_parvalue->buf);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(i(iO))",0,_arg3_len,Py_None);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getstrparam */

static PyObject * PyMSK_getstrparamlen(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsparame _arg1_param;
  int _arg2_len; //  t = <aisread.aisTypeRef object at 0x7fb398404cf8>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_param)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getstrparamlen(self->ptr,_arg1_param,&_arg2_len);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_len);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getstrparamlen */

static PyObject * PyMSK_gettasknamelen(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_len; //  t = <aisread.aisTypeRef object at 0x7fb398410588>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_gettasknamelen(self->ptr,&_arg1_len);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg1_len);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* gettasknamelen */

static PyObject * PyMSK_gettaskname(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_sizetaskname;
  PyObject * obj__arg2_taskname = NULL;
  Py_buffer * buf__arg2_taskname = NULL;
  if (! PyArg_ParseTuple(_args,"iO",&_arg1_sizetaskname,&obj__arg2_taskname)) goto ERROR;
  if (! PyMemoryView_Check(obj__arg2_taskname))
  {
    PyErr_SetString(PyExc_TypeError,"Expected a memoryview object");
    goto ERROR;
  }
  buf__arg2_taskname = PyMemoryView_GET_BUFFER(obj__arg2_taskname);
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_gettaskname(self->ptr,_arg1_sizetaskname,(char*)buf__arg2_taskname->buf);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iO)",0,Py_None);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* gettaskname */

static PyObject * PyMSK_getvartype(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_j;
  MSKvariabletypee _arg2_vartype; //  t = <aisread.aisTypeRef object at 0x7fb398415320>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_j)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getvartype(self->ptr,_arg1_j,&_arg2_vartype);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_vartype);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getvartype */

static PyObject * PyMSK_getvartypelist(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  PyObject * memview__arg2_subj = NULL;
  Py_buffer * buf__arg2_subj;
  int * _arg2_subj = NULL;
  PyObject * memview__arg3_vartype = NULL;
  Py_buffer * buf__arg3_vartype;
  MSKvariabletypee * _arg3_vartype = NULL;
  if (! PyArg_ParseTuple(_args,"iOO",&_arg1_num,&memview__arg2_subj,&memview__arg3_vartype)) goto ERROR;
  if (Py_None != memview__arg2_subj)
  {
    if (! PyMemoryView_Check(memview__arg2_subj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_subj");
      goto ERROR;
    }
    buf__arg2_subj = PyMemoryView_GET_BUFFER(memview__arg2_subj);
    if (buf__arg2_subj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_subj");
        goto ERROR;
      
    }
    _arg2_subj = (int*) buf__arg2_subj->buf;
  }
  if (Py_None != memview__arg3_vartype)
  {
    if (! PyMemoryView_Check(memview__arg3_vartype))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_vartype");
      goto ERROR;
    }
    buf__arg3_vartype = PyMemoryView_GET_BUFFER(memview__arg3_vartype);
    if (buf__arg3_vartype->strides[0] != sizeof(MSKvariabletypee))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_vartype");
        goto ERROR;
      
    }
    _arg3_vartype = (MSKvariabletypee*) buf__arg3_vartype->buf;
    if (buf__arg3_vartype->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg3_vartype");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getvartypelist(self->ptr,_arg1_num,_arg2_subj,_arg3_vartype);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getvartypelist */

static PyObject * PyMSK_inputdata64(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_maxnumcon;
  int _arg2_maxnumvar;
  int _arg3_numcon;
  int _arg4_numvar;
  PyObject * memview__arg5_c = NULL;
  Py_buffer * buf__arg5_c;
  double * _arg5_c = NULL;
  double _arg6_cfix;
  PyObject * memview__arg7_aptrb = NULL;
  Py_buffer * buf__arg7_aptrb;
  long long * _arg7_aptrb = NULL;
  PyObject * memview__arg8_aptre = NULL;
  Py_buffer * buf__arg8_aptre;
  long long * _arg8_aptre = NULL;
  PyObject * memview__arg9_asub = NULL;
  Py_buffer * buf__arg9_asub;
  int * _arg9_asub = NULL;
  PyObject * memview__arg10_aval = NULL;
  Py_buffer * buf__arg10_aval;
  double * _arg10_aval = NULL;
  PyObject * memview__arg11_bkc = NULL;
  Py_buffer * buf__arg11_bkc;
  MSKboundkeye * _arg11_bkc = NULL;
  PyObject * memview__arg12_blc = NULL;
  Py_buffer * buf__arg12_blc;
  double * _arg12_blc = NULL;
  PyObject * memview__arg13_buc = NULL;
  Py_buffer * buf__arg13_buc;
  double * _arg13_buc = NULL;
  PyObject * memview__arg14_bkx = NULL;
  Py_buffer * buf__arg14_bkx;
  MSKboundkeye * _arg14_bkx = NULL;
  PyObject * memview__arg15_blx = NULL;
  Py_buffer * buf__arg15_blx;
  double * _arg15_blx = NULL;
  PyObject * memview__arg16_bux = NULL;
  Py_buffer * buf__arg16_bux;
  double * _arg16_bux = NULL;
  if (! PyArg_ParseTuple(_args,"iiiiOdOOOOOOOOOO",&_arg1_maxnumcon,&_arg2_maxnumvar,&_arg3_numcon,&_arg4_numvar,&memview__arg5_c,&_arg6_cfix,&memview__arg7_aptrb,&memview__arg8_aptre,&memview__arg9_asub,&memview__arg10_aval,&memview__arg11_bkc,&memview__arg12_blc,&memview__arg13_buc,&memview__arg14_bkx,&memview__arg15_blx,&memview__arg16_bux)) goto ERROR;
  if (Py_None != memview__arg5_c)
  {
    if (! PyMemoryView_Check(memview__arg5_c))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_c");
      goto ERROR;
    }
    buf__arg5_c = PyMemoryView_GET_BUFFER(memview__arg5_c);
    if (buf__arg5_c->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_c");
        goto ERROR;
      
    }
    _arg5_c = (double*) buf__arg5_c->buf;
  }
  if (Py_None != memview__arg7_aptrb)
  {
    if (! PyMemoryView_Check(memview__arg7_aptrb))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg7_aptrb");
      goto ERROR;
    }
    buf__arg7_aptrb = PyMemoryView_GET_BUFFER(memview__arg7_aptrb);
    if (buf__arg7_aptrb->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg7_aptrb");
        goto ERROR;
      
    }
    _arg7_aptrb = (long long*) buf__arg7_aptrb->buf;
  }
  if (Py_None != memview__arg8_aptre)
  {
    if (! PyMemoryView_Check(memview__arg8_aptre))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg8_aptre");
      goto ERROR;
    }
    buf__arg8_aptre = PyMemoryView_GET_BUFFER(memview__arg8_aptre);
    if (buf__arg8_aptre->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg8_aptre");
        goto ERROR;
      
    }
    _arg8_aptre = (long long*) buf__arg8_aptre->buf;
  }
  if (Py_None != memview__arg9_asub)
  {
    if (! PyMemoryView_Check(memview__arg9_asub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg9_asub");
      goto ERROR;
    }
    buf__arg9_asub = PyMemoryView_GET_BUFFER(memview__arg9_asub);
    if (buf__arg9_asub->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg9_asub");
        goto ERROR;
      
    }
    _arg9_asub = (int*) buf__arg9_asub->buf;
  }
  if (Py_None != memview__arg10_aval)
  {
    if (! PyMemoryView_Check(memview__arg10_aval))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg10_aval");
      goto ERROR;
    }
    buf__arg10_aval = PyMemoryView_GET_BUFFER(memview__arg10_aval);
    if (buf__arg10_aval->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg10_aval");
        goto ERROR;
      
    }
    _arg10_aval = (double*) buf__arg10_aval->buf;
  }
  if (Py_None != memview__arg11_bkc)
  {
    if (! PyMemoryView_Check(memview__arg11_bkc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg11_bkc");
      goto ERROR;
    }
    buf__arg11_bkc = PyMemoryView_GET_BUFFER(memview__arg11_bkc);
    if (buf__arg11_bkc->strides[0] != sizeof(MSKboundkeye))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg11_bkc");
        goto ERROR;
      
    }
    _arg11_bkc = (MSKboundkeye*) buf__arg11_bkc->buf;
  }
  if (Py_None != memview__arg12_blc)
  {
    if (! PyMemoryView_Check(memview__arg12_blc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg12_blc");
      goto ERROR;
    }
    buf__arg12_blc = PyMemoryView_GET_BUFFER(memview__arg12_blc);
    if (buf__arg12_blc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg12_blc");
        goto ERROR;
      
    }
    _arg12_blc = (double*) buf__arg12_blc->buf;
  }
  if (Py_None != memview__arg13_buc)
  {
    if (! PyMemoryView_Check(memview__arg13_buc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg13_buc");
      goto ERROR;
    }
    buf__arg13_buc = PyMemoryView_GET_BUFFER(memview__arg13_buc);
    if (buf__arg13_buc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg13_buc");
        goto ERROR;
      
    }
    _arg13_buc = (double*) buf__arg13_buc->buf;
  }
  if (Py_None != memview__arg14_bkx)
  {
    if (! PyMemoryView_Check(memview__arg14_bkx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg14_bkx");
      goto ERROR;
    }
    buf__arg14_bkx = PyMemoryView_GET_BUFFER(memview__arg14_bkx);
    if (buf__arg14_bkx->strides[0] != sizeof(MSKboundkeye))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg14_bkx");
        goto ERROR;
      
    }
    _arg14_bkx = (MSKboundkeye*) buf__arg14_bkx->buf;
  }
  if (Py_None != memview__arg15_blx)
  {
    if (! PyMemoryView_Check(memview__arg15_blx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg15_blx");
      goto ERROR;
    }
    buf__arg15_blx = PyMemoryView_GET_BUFFER(memview__arg15_blx);
    if (buf__arg15_blx->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg15_blx");
        goto ERROR;
      
    }
    _arg15_blx = (double*) buf__arg15_blx->buf;
  }
  if (Py_None != memview__arg16_bux)
  {
    if (! PyMemoryView_Check(memview__arg16_bux))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg16_bux");
      goto ERROR;
    }
    buf__arg16_bux = PyMemoryView_GET_BUFFER(memview__arg16_bux);
    if (buf__arg16_bux->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg16_bux");
        goto ERROR;
      
    }
    _arg16_bux = (double*) buf__arg16_bux->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_inputdata64(self->ptr,_arg1_maxnumcon,_arg2_maxnumvar,_arg3_numcon,_arg4_numvar,_arg5_c,_arg6_cfix,_arg7_aptrb,_arg8_aptre,_arg9_asub,_arg10_aval,_arg11_bkc,_arg12_blc,_arg13_buc,_arg14_bkx,_arg15_blx,_arg16_bux);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* inputdata64 */

static PyObject * PyMSK_isdouparname(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_parname = NULL;
  MSKdparame _arg2_param; //  t = <aisread.aisTypeRef object at 0x7fb398425630>
  if (! PyArg_ParseTuple(_args,"s",&str__arg1_parname)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_isdouparname(self->ptr,str__arg1_parname,&_arg2_param);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_param);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* isdouparname */

static PyObject * PyMSK_isintparname(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_parname = NULL;
  MSKiparame _arg2_param; //  t = <aisread.aisTypeRef object at 0x7fb398425a90>
  if (! PyArg_ParseTuple(_args,"s",&str__arg1_parname)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_isintparname(self->ptr,str__arg1_parname,&_arg2_param);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_param);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* isintparname */

static PyObject * PyMSK_isstrparname(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_parname = NULL;
  MSKsparame _arg2_param; //  t = <aisread.aisTypeRef object at 0x7fb398425ef0>
  if (! PyArg_ParseTuple(_args,"s",&str__arg1_parname)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_isstrparname(self->ptr,str__arg1_parname,&_arg2_param);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_param);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* isstrparname */

static PyObject * PyMSK_linkfiletotaskstream(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKstreamtypee _arg1_whichstream;
  char * str__arg2_filename = NULL;
  int _arg3_append;
  if (! PyArg_ParseTuple(_args,"isi",&_arg1_whichstream,&str__arg2_filename,&_arg3_append)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_linkfiletotaskstream(self->ptr,_arg1_whichstream,str__arg2_filename,_arg3_append);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* linkfiletotaskstream */

static PyObject * PyMSK_primalrepair(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  PyObject * memview__arg1_wlc = NULL;
  Py_buffer * buf__arg1_wlc;
  double * _arg1_wlc = NULL;
  PyObject * memview__arg2_wuc = NULL;
  Py_buffer * buf__arg2_wuc;
  double * _arg2_wuc = NULL;
  PyObject * memview__arg3_wlx = NULL;
  Py_buffer * buf__arg3_wlx;
  double * _arg3_wlx = NULL;
  PyObject * memview__arg4_wux = NULL;
  Py_buffer * buf__arg4_wux;
  double * _arg4_wux = NULL;
  if (! PyArg_ParseTuple(_args,"OOOO",&memview__arg1_wlc,&memview__arg2_wuc,&memview__arg3_wlx,&memview__arg4_wux)) goto ERROR;
  if (Py_None != memview__arg1_wlc)
  {
    if (! PyMemoryView_Check(memview__arg1_wlc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg1_wlc");
      goto ERROR;
    }
    buf__arg1_wlc = PyMemoryView_GET_BUFFER(memview__arg1_wlc);
    if (buf__arg1_wlc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg1_wlc");
        goto ERROR;
      
    }
    _arg1_wlc = (double*) buf__arg1_wlc->buf;
  }
  if (Py_None != memview__arg2_wuc)
  {
    if (! PyMemoryView_Check(memview__arg2_wuc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_wuc");
      goto ERROR;
    }
    buf__arg2_wuc = PyMemoryView_GET_BUFFER(memview__arg2_wuc);
    if (buf__arg2_wuc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_wuc");
        goto ERROR;
      
    }
    _arg2_wuc = (double*) buf__arg2_wuc->buf;
  }
  if (Py_None != memview__arg3_wlx)
  {
    if (! PyMemoryView_Check(memview__arg3_wlx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_wlx");
      goto ERROR;
    }
    buf__arg3_wlx = PyMemoryView_GET_BUFFER(memview__arg3_wlx);
    if (buf__arg3_wlx->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_wlx");
        goto ERROR;
      
    }
    _arg3_wlx = (double*) buf__arg3_wlx->buf;
  }
  if (Py_None != memview__arg4_wux)
  {
    if (! PyMemoryView_Check(memview__arg4_wux))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_wux");
      goto ERROR;
    }
    buf__arg4_wux = PyMemoryView_GET_BUFFER(memview__arg4_wux);
    if (buf__arg4_wux->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_wux");
        goto ERROR;
      
    }
    _arg4_wux = (double*) buf__arg4_wux->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_primalrepair(self->ptr,_arg1_wlc,_arg2_wuc,_arg3_wlx,_arg4_wux);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* primalrepair */

static PyObject * PyMSK_toconic(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_toconic(self->ptr);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* toconic */

static PyObject * PyMSK_optimizetrm(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKrescodee _arg1_trmcode; //  t = <aisread.aisTypeRef object at 0x7fb3984348d0>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_optimizetrm(self->ptr,&_arg1_trmcode);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg1_trmcode);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* optimizetrm */

static PyObject * PyMSK_commitchanges(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_commitchanges(self->ptr);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* commitchanges */

static PyObject * PyMSK_getatruncatetol(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  PyObject * memview__arg1_tolzero = NULL;
  Py_buffer * buf__arg1_tolzero;
  double * _arg1_tolzero = NULL;
  if (! PyArg_ParseTuple(_args,"O",&memview__arg1_tolzero)) goto ERROR;
  if (Py_None != memview__arg1_tolzero)
  {
    if (! PyMemoryView_Check(memview__arg1_tolzero))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg1_tolzero");
      goto ERROR;
    }
    buf__arg1_tolzero = PyMemoryView_GET_BUFFER(memview__arg1_tolzero);
    if (buf__arg1_tolzero->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg1_tolzero");
        goto ERROR;
      
    }
    _arg1_tolzero = (double*) buf__arg1_tolzero->buf;
    if (buf__arg1_tolzero->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg1_tolzero");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getatruncatetol(self->ptr,_arg1_tolzero);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getatruncatetol */

static PyObject * PyMSK_putatruncatetol(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  double _arg1_tolzero;
  if (! PyArg_ParseTuple(_args,"d",&_arg1_tolzero)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putatruncatetol(self->ptr,_arg1_tolzero);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putatruncatetol */

static PyObject * PyMSK_putaij(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_i;
  int _arg2_j;
  double _arg3_aij;
  if (! PyArg_ParseTuple(_args,"iid",&_arg1_i,&_arg2_j,&_arg3_aij)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putaij(self->ptr,_arg1_i,_arg2_j,_arg3_aij);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putaij */

static PyObject * PyMSK_putaijlist64(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_num;
  PyObject * memview__arg2_subi = NULL;
  Py_buffer * buf__arg2_subi;
  int * _arg2_subi = NULL;
  PyObject * memview__arg3_subj = NULL;
  Py_buffer * buf__arg3_subj;
  int * _arg3_subj = NULL;
  PyObject * memview__arg4_valij = NULL;
  Py_buffer * buf__arg4_valij;
  double * _arg4_valij = NULL;
  if (! PyArg_ParseTuple(_args,"LOOO",&_arg1_num,&memview__arg2_subi,&memview__arg3_subj,&memview__arg4_valij)) goto ERROR;
  if (Py_None != memview__arg2_subi)
  {
    if (! PyMemoryView_Check(memview__arg2_subi))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_subi");
      goto ERROR;
    }
    buf__arg2_subi = PyMemoryView_GET_BUFFER(memview__arg2_subi);
    if (buf__arg2_subi->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_subi");
        goto ERROR;
      
    }
    _arg2_subi = (int*) buf__arg2_subi->buf;
  }
  if (Py_None != memview__arg3_subj)
  {
    if (! PyMemoryView_Check(memview__arg3_subj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_subj");
      goto ERROR;
    }
    buf__arg3_subj = PyMemoryView_GET_BUFFER(memview__arg3_subj);
    if (buf__arg3_subj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_subj");
        goto ERROR;
      
    }
    _arg3_subj = (int*) buf__arg3_subj->buf;
  }
  if (Py_None != memview__arg4_valij)
  {
    if (! PyMemoryView_Check(memview__arg4_valij))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_valij");
      goto ERROR;
    }
    buf__arg4_valij = PyMemoryView_GET_BUFFER(memview__arg4_valij);
    if (buf__arg4_valij->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_valij");
        goto ERROR;
      
    }
    _arg4_valij = (double*) buf__arg4_valij->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putaijlist64(self->ptr,_arg1_num,_arg2_subi,_arg3_subj,_arg4_valij);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putaijlist64 */

static PyObject * PyMSK_putacol(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_j;
  int _arg2_nzj;
  PyObject * memview__arg3_subj = NULL;
  Py_buffer * buf__arg3_subj;
  int * _arg3_subj = NULL;
  PyObject * memview__arg4_valj = NULL;
  Py_buffer * buf__arg4_valj;
  double * _arg4_valj = NULL;
  if (! PyArg_ParseTuple(_args,"iiOO",&_arg1_j,&_arg2_nzj,&memview__arg3_subj,&memview__arg4_valj)) goto ERROR;
  if (Py_None != memview__arg3_subj)
  {
    if (! PyMemoryView_Check(memview__arg3_subj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_subj");
      goto ERROR;
    }
    buf__arg3_subj = PyMemoryView_GET_BUFFER(memview__arg3_subj);
    if (buf__arg3_subj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_subj");
        goto ERROR;
      
    }
    _arg3_subj = (int*) buf__arg3_subj->buf;
  }
  if (Py_None != memview__arg4_valj)
  {
    if (! PyMemoryView_Check(memview__arg4_valj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_valj");
      goto ERROR;
    }
    buf__arg4_valj = PyMemoryView_GET_BUFFER(memview__arg4_valj);
    if (buf__arg4_valj->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_valj");
        goto ERROR;
      
    }
    _arg4_valj = (double*) buf__arg4_valj->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putacol(self->ptr,_arg1_j,_arg2_nzj,_arg3_subj,_arg4_valj);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putacol */

static PyObject * PyMSK_putarow(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_i;
  int _arg2_nzi;
  PyObject * memview__arg3_subi = NULL;
  Py_buffer * buf__arg3_subi;
  int * _arg3_subi = NULL;
  PyObject * memview__arg4_vali = NULL;
  Py_buffer * buf__arg4_vali;
  double * _arg4_vali = NULL;
  if (! PyArg_ParseTuple(_args,"iiOO",&_arg1_i,&_arg2_nzi,&memview__arg3_subi,&memview__arg4_vali)) goto ERROR;
  if (Py_None != memview__arg3_subi)
  {
    if (! PyMemoryView_Check(memview__arg3_subi))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_subi");
      goto ERROR;
    }
    buf__arg3_subi = PyMemoryView_GET_BUFFER(memview__arg3_subi);
    if (buf__arg3_subi->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_subi");
        goto ERROR;
      
    }
    _arg3_subi = (int*) buf__arg3_subi->buf;
  }
  if (Py_None != memview__arg4_vali)
  {
    if (! PyMemoryView_Check(memview__arg4_vali))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_vali");
      goto ERROR;
    }
    buf__arg4_vali = PyMemoryView_GET_BUFFER(memview__arg4_vali);
    if (buf__arg4_vali->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_vali");
        goto ERROR;
      
    }
    _arg4_vali = (double*) buf__arg4_vali->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putarow(self->ptr,_arg1_i,_arg2_nzi,_arg3_subi,_arg4_vali);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putarow */

static PyObject * PyMSK_putarowslice64(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_first;
  int _arg2_last;
  PyObject * memview__arg3_ptrb = NULL;
  Py_buffer * buf__arg3_ptrb;
  long long * _arg3_ptrb = NULL;
  PyObject * memview__arg4_ptre = NULL;
  Py_buffer * buf__arg4_ptre;
  long long * _arg4_ptre = NULL;
  PyObject * memview__arg5_asub = NULL;
  Py_buffer * buf__arg5_asub;
  int * _arg5_asub = NULL;
  PyObject * memview__arg6_aval = NULL;
  Py_buffer * buf__arg6_aval;
  double * _arg6_aval = NULL;
  if (! PyArg_ParseTuple(_args,"iiOOOO",&_arg1_first,&_arg2_last,&memview__arg3_ptrb,&memview__arg4_ptre,&memview__arg5_asub,&memview__arg6_aval)) goto ERROR;
  if (Py_None != memview__arg3_ptrb)
  {
    if (! PyMemoryView_Check(memview__arg3_ptrb))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_ptrb");
      goto ERROR;
    }
    buf__arg3_ptrb = PyMemoryView_GET_BUFFER(memview__arg3_ptrb);
    if (buf__arg3_ptrb->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_ptrb");
        goto ERROR;
      
    }
    _arg3_ptrb = (long long*) buf__arg3_ptrb->buf;
  }
  if (Py_None != memview__arg4_ptre)
  {
    if (! PyMemoryView_Check(memview__arg4_ptre))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_ptre");
      goto ERROR;
    }
    buf__arg4_ptre = PyMemoryView_GET_BUFFER(memview__arg4_ptre);
    if (buf__arg4_ptre->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_ptre");
        goto ERROR;
      
    }
    _arg4_ptre = (long long*) buf__arg4_ptre->buf;
  }
  if (Py_None != memview__arg5_asub)
  {
    if (! PyMemoryView_Check(memview__arg5_asub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_asub");
      goto ERROR;
    }
    buf__arg5_asub = PyMemoryView_GET_BUFFER(memview__arg5_asub);
    if (buf__arg5_asub->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_asub");
        goto ERROR;
      
    }
    _arg5_asub = (int*) buf__arg5_asub->buf;
  }
  if (Py_None != memview__arg6_aval)
  {
    if (! PyMemoryView_Check(memview__arg6_aval))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_aval");
      goto ERROR;
    }
    buf__arg6_aval = PyMemoryView_GET_BUFFER(memview__arg6_aval);
    if (buf__arg6_aval->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_aval");
        goto ERROR;
      
    }
    _arg6_aval = (double*) buf__arg6_aval->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putarowslice64(self->ptr,_arg1_first,_arg2_last,_arg3_ptrb,_arg4_ptre,_arg5_asub,_arg6_aval);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putarowslice64 */

static PyObject * PyMSK_putarowlist64(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  PyObject * memview__arg2_sub = NULL;
  Py_buffer * buf__arg2_sub;
  int * _arg2_sub = NULL;
  PyObject * memview__arg3_ptrb = NULL;
  Py_buffer * buf__arg3_ptrb;
  long long * _arg3_ptrb = NULL;
  PyObject * memview__arg4_ptre = NULL;
  Py_buffer * buf__arg4_ptre;
  long long * _arg4_ptre = NULL;
  PyObject * memview__arg5_asub = NULL;
  Py_buffer * buf__arg5_asub;
  int * _arg5_asub = NULL;
  PyObject * memview__arg6_aval = NULL;
  Py_buffer * buf__arg6_aval;
  double * _arg6_aval = NULL;
  if (! PyArg_ParseTuple(_args,"iOOOOO",&_arg1_num,&memview__arg2_sub,&memview__arg3_ptrb,&memview__arg4_ptre,&memview__arg5_asub,&memview__arg6_aval)) goto ERROR;
  if (Py_None != memview__arg2_sub)
  {
    if (! PyMemoryView_Check(memview__arg2_sub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_sub");
      goto ERROR;
    }
    buf__arg2_sub = PyMemoryView_GET_BUFFER(memview__arg2_sub);
    if (buf__arg2_sub->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_sub");
        goto ERROR;
      
    }
    _arg2_sub = (int*) buf__arg2_sub->buf;
  }
  if (Py_None != memview__arg3_ptrb)
  {
    if (! PyMemoryView_Check(memview__arg3_ptrb))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_ptrb");
      goto ERROR;
    }
    buf__arg3_ptrb = PyMemoryView_GET_BUFFER(memview__arg3_ptrb);
    if (buf__arg3_ptrb->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_ptrb");
        goto ERROR;
      
    }
    _arg3_ptrb = (long long*) buf__arg3_ptrb->buf;
  }
  if (Py_None != memview__arg4_ptre)
  {
    if (! PyMemoryView_Check(memview__arg4_ptre))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_ptre");
      goto ERROR;
    }
    buf__arg4_ptre = PyMemoryView_GET_BUFFER(memview__arg4_ptre);
    if (buf__arg4_ptre->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_ptre");
        goto ERROR;
      
    }
    _arg4_ptre = (long long*) buf__arg4_ptre->buf;
  }
  if (Py_None != memview__arg5_asub)
  {
    if (! PyMemoryView_Check(memview__arg5_asub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_asub");
      goto ERROR;
    }
    buf__arg5_asub = PyMemoryView_GET_BUFFER(memview__arg5_asub);
    if (buf__arg5_asub->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_asub");
        goto ERROR;
      
    }
    _arg5_asub = (int*) buf__arg5_asub->buf;
  }
  if (Py_None != memview__arg6_aval)
  {
    if (! PyMemoryView_Check(memview__arg6_aval))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_aval");
      goto ERROR;
    }
    buf__arg6_aval = PyMemoryView_GET_BUFFER(memview__arg6_aval);
    if (buf__arg6_aval->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_aval");
        goto ERROR;
      
    }
    _arg6_aval = (double*) buf__arg6_aval->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putarowlist64(self->ptr,_arg1_num,_arg2_sub,_arg3_ptrb,_arg4_ptre,_arg5_asub,_arg6_aval);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putarowlist64 */

static PyObject * PyMSK_putacolslice64(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_first;
  int _arg2_last;
  PyObject * memview__arg3_ptrb = NULL;
  Py_buffer * buf__arg3_ptrb;
  long long * _arg3_ptrb = NULL;
  PyObject * memview__arg4_ptre = NULL;
  Py_buffer * buf__arg4_ptre;
  long long * _arg4_ptre = NULL;
  PyObject * memview__arg5_asub = NULL;
  Py_buffer * buf__arg5_asub;
  int * _arg5_asub = NULL;
  PyObject * memview__arg6_aval = NULL;
  Py_buffer * buf__arg6_aval;
  double * _arg6_aval = NULL;
  if (! PyArg_ParseTuple(_args,"iiOOOO",&_arg1_first,&_arg2_last,&memview__arg3_ptrb,&memview__arg4_ptre,&memview__arg5_asub,&memview__arg6_aval)) goto ERROR;
  if (Py_None != memview__arg3_ptrb)
  {
    if (! PyMemoryView_Check(memview__arg3_ptrb))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_ptrb");
      goto ERROR;
    }
    buf__arg3_ptrb = PyMemoryView_GET_BUFFER(memview__arg3_ptrb);
    if (buf__arg3_ptrb->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_ptrb");
        goto ERROR;
      
    }
    _arg3_ptrb = (long long*) buf__arg3_ptrb->buf;
  }
  if (Py_None != memview__arg4_ptre)
  {
    if (! PyMemoryView_Check(memview__arg4_ptre))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_ptre");
      goto ERROR;
    }
    buf__arg4_ptre = PyMemoryView_GET_BUFFER(memview__arg4_ptre);
    if (buf__arg4_ptre->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_ptre");
        goto ERROR;
      
    }
    _arg4_ptre = (long long*) buf__arg4_ptre->buf;
  }
  if (Py_None != memview__arg5_asub)
  {
    if (! PyMemoryView_Check(memview__arg5_asub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_asub");
      goto ERROR;
    }
    buf__arg5_asub = PyMemoryView_GET_BUFFER(memview__arg5_asub);
    if (buf__arg5_asub->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_asub");
        goto ERROR;
      
    }
    _arg5_asub = (int*) buf__arg5_asub->buf;
  }
  if (Py_None != memview__arg6_aval)
  {
    if (! PyMemoryView_Check(memview__arg6_aval))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_aval");
      goto ERROR;
    }
    buf__arg6_aval = PyMemoryView_GET_BUFFER(memview__arg6_aval);
    if (buf__arg6_aval->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_aval");
        goto ERROR;
      
    }
    _arg6_aval = (double*) buf__arg6_aval->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putacolslice64(self->ptr,_arg1_first,_arg2_last,_arg3_ptrb,_arg4_ptre,_arg5_asub,_arg6_aval);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putacolslice64 */

static PyObject * PyMSK_putacollist64(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  PyObject * memview__arg2_sub = NULL;
  Py_buffer * buf__arg2_sub;
  int * _arg2_sub = NULL;
  PyObject * memview__arg3_ptrb = NULL;
  Py_buffer * buf__arg3_ptrb;
  long long * _arg3_ptrb = NULL;
  PyObject * memview__arg4_ptre = NULL;
  Py_buffer * buf__arg4_ptre;
  long long * _arg4_ptre = NULL;
  PyObject * memview__arg5_asub = NULL;
  Py_buffer * buf__arg5_asub;
  int * _arg5_asub = NULL;
  PyObject * memview__arg6_aval = NULL;
  Py_buffer * buf__arg6_aval;
  double * _arg6_aval = NULL;
  if (! PyArg_ParseTuple(_args,"iOOOOO",&_arg1_num,&memview__arg2_sub,&memview__arg3_ptrb,&memview__arg4_ptre,&memview__arg5_asub,&memview__arg6_aval)) goto ERROR;
  if (Py_None != memview__arg2_sub)
  {
    if (! PyMemoryView_Check(memview__arg2_sub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_sub");
      goto ERROR;
    }
    buf__arg2_sub = PyMemoryView_GET_BUFFER(memview__arg2_sub);
    if (buf__arg2_sub->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_sub");
        goto ERROR;
      
    }
    _arg2_sub = (int*) buf__arg2_sub->buf;
  }
  if (Py_None != memview__arg3_ptrb)
  {
    if (! PyMemoryView_Check(memview__arg3_ptrb))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_ptrb");
      goto ERROR;
    }
    buf__arg3_ptrb = PyMemoryView_GET_BUFFER(memview__arg3_ptrb);
    if (buf__arg3_ptrb->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_ptrb");
        goto ERROR;
      
    }
    _arg3_ptrb = (long long*) buf__arg3_ptrb->buf;
  }
  if (Py_None != memview__arg4_ptre)
  {
    if (! PyMemoryView_Check(memview__arg4_ptre))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_ptre");
      goto ERROR;
    }
    buf__arg4_ptre = PyMemoryView_GET_BUFFER(memview__arg4_ptre);
    if (buf__arg4_ptre->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_ptre");
        goto ERROR;
      
    }
    _arg4_ptre = (long long*) buf__arg4_ptre->buf;
  }
  if (Py_None != memview__arg5_asub)
  {
    if (! PyMemoryView_Check(memview__arg5_asub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_asub");
      goto ERROR;
    }
    buf__arg5_asub = PyMemoryView_GET_BUFFER(memview__arg5_asub);
    if (buf__arg5_asub->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_asub");
        goto ERROR;
      
    }
    _arg5_asub = (int*) buf__arg5_asub->buf;
  }
  if (Py_None != memview__arg6_aval)
  {
    if (! PyMemoryView_Check(memview__arg6_aval))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_aval");
      goto ERROR;
    }
    buf__arg6_aval = PyMemoryView_GET_BUFFER(memview__arg6_aval);
    if (buf__arg6_aval->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_aval");
        goto ERROR;
      
    }
    _arg6_aval = (double*) buf__arg6_aval->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putacollist64(self->ptr,_arg1_num,_arg2_sub,_arg3_ptrb,_arg4_ptre,_arg5_asub,_arg6_aval);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putacollist64 */

static PyObject * PyMSK_putbaraij(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_i;
  int _arg2_j;
  long long _arg3_num;
  PyObject * memview__arg4_sub = NULL;
  Py_buffer * buf__arg4_sub;
  long long * _arg4_sub = NULL;
  PyObject * memview__arg5_weights = NULL;
  Py_buffer * buf__arg5_weights;
  double * _arg5_weights = NULL;
  if (! PyArg_ParseTuple(_args,"iiLOO",&_arg1_i,&_arg2_j,&_arg3_num,&memview__arg4_sub,&memview__arg5_weights)) goto ERROR;
  if (Py_None != memview__arg4_sub)
  {
    if (! PyMemoryView_Check(memview__arg4_sub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_sub");
      goto ERROR;
    }
    buf__arg4_sub = PyMemoryView_GET_BUFFER(memview__arg4_sub);
    if (buf__arg4_sub->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_sub");
        goto ERROR;
      
    }
    _arg4_sub = (long long*) buf__arg4_sub->buf;
  }
  if (Py_None != memview__arg5_weights)
  {
    if (! PyMemoryView_Check(memview__arg5_weights))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_weights");
      goto ERROR;
    }
    buf__arg5_weights = PyMemoryView_GET_BUFFER(memview__arg5_weights);
    if (buf__arg5_weights->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_weights");
        goto ERROR;
      
    }
    _arg5_weights = (double*) buf__arg5_weights->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putbaraij(self->ptr,_arg1_i,_arg2_j,_arg3_num,_arg4_sub,_arg5_weights);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putbaraij */

static PyObject * PyMSK_putbaraijlist(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  PyObject * memview__arg2_subi = NULL;
  Py_buffer * buf__arg2_subi;
  int * _arg2_subi = NULL;
  PyObject * memview__arg3_subj = NULL;
  Py_buffer * buf__arg3_subj;
  int * _arg3_subj = NULL;
  PyObject * memview__arg4_alphaptrb = NULL;
  Py_buffer * buf__arg4_alphaptrb;
  long long * _arg4_alphaptrb = NULL;
  PyObject * memview__arg5_alphaptre = NULL;
  Py_buffer * buf__arg5_alphaptre;
  long long * _arg5_alphaptre = NULL;
  PyObject * memview__arg6_matidx = NULL;
  Py_buffer * buf__arg6_matidx;
  long long * _arg6_matidx = NULL;
  PyObject * memview__arg7_weights = NULL;
  Py_buffer * buf__arg7_weights;
  double * _arg7_weights = NULL;
  if (! PyArg_ParseTuple(_args,"iOOOOOO",&_arg1_num,&memview__arg2_subi,&memview__arg3_subj,&memview__arg4_alphaptrb,&memview__arg5_alphaptre,&memview__arg6_matidx,&memview__arg7_weights)) goto ERROR;
  if (Py_None != memview__arg2_subi)
  {
    if (! PyMemoryView_Check(memview__arg2_subi))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_subi");
      goto ERROR;
    }
    buf__arg2_subi = PyMemoryView_GET_BUFFER(memview__arg2_subi);
    if (buf__arg2_subi->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_subi");
        goto ERROR;
      
    }
    _arg2_subi = (int*) buf__arg2_subi->buf;
  }
  if (Py_None != memview__arg3_subj)
  {
    if (! PyMemoryView_Check(memview__arg3_subj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_subj");
      goto ERROR;
    }
    buf__arg3_subj = PyMemoryView_GET_BUFFER(memview__arg3_subj);
    if (buf__arg3_subj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_subj");
        goto ERROR;
      
    }
    _arg3_subj = (int*) buf__arg3_subj->buf;
  }
  if (Py_None != memview__arg4_alphaptrb)
  {
    if (! PyMemoryView_Check(memview__arg4_alphaptrb))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_alphaptrb");
      goto ERROR;
    }
    buf__arg4_alphaptrb = PyMemoryView_GET_BUFFER(memview__arg4_alphaptrb);
    if (buf__arg4_alphaptrb->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_alphaptrb");
        goto ERROR;
      
    }
    _arg4_alphaptrb = (long long*) buf__arg4_alphaptrb->buf;
  }
  if (Py_None != memview__arg5_alphaptre)
  {
    if (! PyMemoryView_Check(memview__arg5_alphaptre))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_alphaptre");
      goto ERROR;
    }
    buf__arg5_alphaptre = PyMemoryView_GET_BUFFER(memview__arg5_alphaptre);
    if (buf__arg5_alphaptre->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_alphaptre");
        goto ERROR;
      
    }
    _arg5_alphaptre = (long long*) buf__arg5_alphaptre->buf;
  }
  if (Py_None != memview__arg6_matidx)
  {
    if (! PyMemoryView_Check(memview__arg6_matidx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_matidx");
      goto ERROR;
    }
    buf__arg6_matidx = PyMemoryView_GET_BUFFER(memview__arg6_matidx);
    if (buf__arg6_matidx->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_matidx");
        goto ERROR;
      
    }
    _arg6_matidx = (long long*) buf__arg6_matidx->buf;
  }
  if (Py_None != memview__arg7_weights)
  {
    if (! PyMemoryView_Check(memview__arg7_weights))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg7_weights");
      goto ERROR;
    }
    buf__arg7_weights = PyMemoryView_GET_BUFFER(memview__arg7_weights);
    if (buf__arg7_weights->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg7_weights");
        goto ERROR;
      
    }
    _arg7_weights = (double*) buf__arg7_weights->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putbaraijlist(self->ptr,_arg1_num,_arg2_subi,_arg3_subj,_arg4_alphaptrb,_arg5_alphaptre,_arg6_matidx,_arg7_weights);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putbaraijlist */

static PyObject * PyMSK_putbararowlist(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  PyObject * memview__arg2_subi = NULL;
  Py_buffer * buf__arg2_subi;
  int * _arg2_subi = NULL;
  PyObject * memview__arg3_ptrb = NULL;
  Py_buffer * buf__arg3_ptrb;
  long long * _arg3_ptrb = NULL;
  PyObject * memview__arg4_ptre = NULL;
  Py_buffer * buf__arg4_ptre;
  long long * _arg4_ptre = NULL;
  PyObject * memview__arg5_subj = NULL;
  Py_buffer * buf__arg5_subj;
  int * _arg5_subj = NULL;
  PyObject * memview__arg6_nummat = NULL;
  Py_buffer * buf__arg6_nummat;
  long long * _arg6_nummat = NULL;
  PyObject * memview__arg7_matidx = NULL;
  Py_buffer * buf__arg7_matidx;
  long long * _arg7_matidx = NULL;
  PyObject * memview__arg8_weights = NULL;
  Py_buffer * buf__arg8_weights;
  double * _arg8_weights = NULL;
  if (! PyArg_ParseTuple(_args,"iOOOOOOO",&_arg1_num,&memview__arg2_subi,&memview__arg3_ptrb,&memview__arg4_ptre,&memview__arg5_subj,&memview__arg6_nummat,&memview__arg7_matidx,&memview__arg8_weights)) goto ERROR;
  if (Py_None != memview__arg2_subi)
  {
    if (! PyMemoryView_Check(memview__arg2_subi))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_subi");
      goto ERROR;
    }
    buf__arg2_subi = PyMemoryView_GET_BUFFER(memview__arg2_subi);
    if (buf__arg2_subi->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_subi");
        goto ERROR;
      
    }
    _arg2_subi = (int*) buf__arg2_subi->buf;
  }
  if (Py_None != memview__arg3_ptrb)
  {
    if (! PyMemoryView_Check(memview__arg3_ptrb))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_ptrb");
      goto ERROR;
    }
    buf__arg3_ptrb = PyMemoryView_GET_BUFFER(memview__arg3_ptrb);
    if (buf__arg3_ptrb->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_ptrb");
        goto ERROR;
      
    }
    _arg3_ptrb = (long long*) buf__arg3_ptrb->buf;
  }
  if (Py_None != memview__arg4_ptre)
  {
    if (! PyMemoryView_Check(memview__arg4_ptre))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_ptre");
      goto ERROR;
    }
    buf__arg4_ptre = PyMemoryView_GET_BUFFER(memview__arg4_ptre);
    if (buf__arg4_ptre->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_ptre");
        goto ERROR;
      
    }
    _arg4_ptre = (long long*) buf__arg4_ptre->buf;
  }
  if (Py_None != memview__arg5_subj)
  {
    if (! PyMemoryView_Check(memview__arg5_subj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_subj");
      goto ERROR;
    }
    buf__arg5_subj = PyMemoryView_GET_BUFFER(memview__arg5_subj);
    if (buf__arg5_subj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_subj");
        goto ERROR;
      
    }
    _arg5_subj = (int*) buf__arg5_subj->buf;
  }
  if (Py_None != memview__arg6_nummat)
  {
    if (! PyMemoryView_Check(memview__arg6_nummat))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_nummat");
      goto ERROR;
    }
    buf__arg6_nummat = PyMemoryView_GET_BUFFER(memview__arg6_nummat);
    if (buf__arg6_nummat->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_nummat");
        goto ERROR;
      
    }
    _arg6_nummat = (long long*) buf__arg6_nummat->buf;
  }
  if (Py_None != memview__arg7_matidx)
  {
    if (! PyMemoryView_Check(memview__arg7_matidx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg7_matidx");
      goto ERROR;
    }
    buf__arg7_matidx = PyMemoryView_GET_BUFFER(memview__arg7_matidx);
    if (buf__arg7_matidx->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg7_matidx");
        goto ERROR;
      
    }
    _arg7_matidx = (long long*) buf__arg7_matidx->buf;
  }
  if (Py_None != memview__arg8_weights)
  {
    if (! PyMemoryView_Check(memview__arg8_weights))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg8_weights");
      goto ERROR;
    }
    buf__arg8_weights = PyMemoryView_GET_BUFFER(memview__arg8_weights);
    if (buf__arg8_weights->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg8_weights");
        goto ERROR;
      
    }
    _arg8_weights = (double*) buf__arg8_weights->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putbararowlist(self->ptr,_arg1_num,_arg2_subi,_arg3_ptrb,_arg4_ptre,_arg5_subj,_arg6_nummat,_arg7_matidx,_arg8_weights);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putbararowlist */

static PyObject * PyMSK_getnumbarcnz(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_nz; //  t = <aisread.aisTypeRef object at 0x7fb3983f4080>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getnumbarcnz(self->ptr,&_arg1_nz);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iL)",0,_arg1_nz);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getnumbarcnz */

static PyObject * PyMSK_getnumbaranz(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_nz; //  t = <aisread.aisTypeRef object at 0x7fb3983f4400>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getnumbaranz(self->ptr,&_arg1_nz);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iL)",0,_arg1_nz);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getnumbaranz */

static PyObject * PyMSK_getbarcsparsity(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_maxnumnz;
  long long _arg2_numnz; //  t = <aisread.aisTypeRef object at 0x7fb3983f48d0>
  PyObject * memview__arg3_idxj = NULL;
  Py_buffer * buf__arg3_idxj;
  long long * _arg3_idxj = NULL;
  if (! PyArg_ParseTuple(_args,"LO",&_arg1_maxnumnz,&memview__arg3_idxj)) goto ERROR;
  if (Py_None != memview__arg3_idxj)
  {
    if (! PyMemoryView_Check(memview__arg3_idxj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_idxj");
      goto ERROR;
    }
    buf__arg3_idxj = PyMemoryView_GET_BUFFER(memview__arg3_idxj);
    if (buf__arg3_idxj->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_idxj");
        goto ERROR;
      
    }
    _arg3_idxj = (long long*) buf__arg3_idxj->buf;
    if (buf__arg3_idxj->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg3_idxj");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getbarcsparsity(self->ptr,_arg1_maxnumnz,&_arg2_numnz,_arg3_idxj);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iL)",0,_arg2_numnz);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getbarcsparsity */

static PyObject * PyMSK_getbarasparsity(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_maxnumnz;
  long long _arg2_numnz; //  t = <aisread.aisTypeRef object at 0x7fb3983f4ef0>
  PyObject * memview__arg3_idxij = NULL;
  Py_buffer * buf__arg3_idxij;
  long long * _arg3_idxij = NULL;
  if (! PyArg_ParseTuple(_args,"LO",&_arg1_maxnumnz,&memview__arg3_idxij)) goto ERROR;
  if (Py_None != memview__arg3_idxij)
  {
    if (! PyMemoryView_Check(memview__arg3_idxij))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_idxij");
      goto ERROR;
    }
    buf__arg3_idxij = PyMemoryView_GET_BUFFER(memview__arg3_idxij);
    if (buf__arg3_idxij->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_idxij");
        goto ERROR;
      
    }
    _arg3_idxij = (long long*) buf__arg3_idxij->buf;
    if (buf__arg3_idxij->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg3_idxij");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getbarasparsity(self->ptr,_arg1_maxnumnz,&_arg2_numnz,_arg3_idxij);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iL)",0,_arg2_numnz);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getbarasparsity */

static PyObject * PyMSK_getbarcidxinfo(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_idx;
  long long _arg2_num; //  t = <aisread.aisTypeRef object at 0x7fb39837c4e0>
  if (! PyArg_ParseTuple(_args,"L",&_arg1_idx)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getbarcidxinfo(self->ptr,_arg1_idx,&_arg2_num);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iL)",0,_arg2_num);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getbarcidxinfo */

static PyObject * PyMSK_getbarcidxj(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_idx;
  int _arg2_j; //  t = <aisread.aisTypeRef object at 0x7fb39837c940>
  if (! PyArg_ParseTuple(_args,"L",&_arg1_idx)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getbarcidxj(self->ptr,_arg1_idx,&_arg2_j);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_j);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getbarcidxj */

static PyObject * PyMSK_getbarcidx(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_idx;
  long long _arg2_maxnum;
  int _arg3_j; //  t = <aisread.aisTypeRef object at 0x7fb39837cef0>
  long long _arg4_num; //  t = <aisread.aisTypeRef object at 0x7fb398383048>
  PyObject * memview__arg5_sub = NULL;
  Py_buffer * buf__arg5_sub;
  long long * _arg5_sub = NULL;
  PyObject * memview__arg6_weights = NULL;
  Py_buffer * buf__arg6_weights;
  double * _arg6_weights = NULL;
  if (! PyArg_ParseTuple(_args,"LLOO",&_arg1_idx,&_arg2_maxnum,&memview__arg5_sub,&memview__arg6_weights)) goto ERROR;
  if (Py_None != memview__arg5_sub)
  {
    if (! PyMemoryView_Check(memview__arg5_sub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_sub");
      goto ERROR;
    }
    buf__arg5_sub = PyMemoryView_GET_BUFFER(memview__arg5_sub);
    if (buf__arg5_sub->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_sub");
        goto ERROR;
      
    }
    _arg5_sub = (long long*) buf__arg5_sub->buf;
    if (buf__arg5_sub->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg5_sub");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg6_weights)
  {
    if (! PyMemoryView_Check(memview__arg6_weights))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_weights");
      goto ERROR;
    }
    buf__arg6_weights = PyMemoryView_GET_BUFFER(memview__arg6_weights);
    if (buf__arg6_weights->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_weights");
        goto ERROR;
      
    }
    _arg6_weights = (double*) buf__arg6_weights->buf;
    if (buf__arg6_weights->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg6_weights");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getbarcidx(self->ptr,_arg1_idx,_arg2_maxnum,&_arg3_j,&_arg4_num,_arg5_sub,_arg6_weights);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(i(iL))",0,_arg3_j,_arg4_num);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getbarcidx */

static PyObject * PyMSK_getbaraidxinfo(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_idx;
  long long _arg2_num; //  t = <aisread.aisTypeRef object at 0x7fb398383748>
  if (! PyArg_ParseTuple(_args,"L",&_arg1_idx)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getbaraidxinfo(self->ptr,_arg1_idx,&_arg2_num);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iL)",0,_arg2_num);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getbaraidxinfo */

static PyObject * PyMSK_getbaraidxij(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_idx;
  int _arg2_i; //  t = <aisread.aisTypeRef object at 0x7fb398383ba8>
  int _arg3_j; //  t = <aisread.aisTypeRef object at 0x7fb398383cc0>
  if (! PyArg_ParseTuple(_args,"L",&_arg1_idx)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getbaraidxij(self->ptr,_arg1_idx,&_arg2_i,&_arg3_j);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(i(ii))",0,_arg2_i,_arg3_j);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getbaraidxij */

static PyObject * PyMSK_getbaraidx(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_idx;
  long long _arg2_maxnum;
  int _arg3_i; //  t = <aisread.aisTypeRef object at 0x7fb3983872b0>
  int _arg4_j; //  t = <aisread.aisTypeRef object at 0x7fb3983873c8>
  long long _arg5_num; //  t = <aisread.aisTypeRef object at 0x7fb3983874e0>
  PyObject * memview__arg6_sub = NULL;
  Py_buffer * buf__arg6_sub;
  long long * _arg6_sub = NULL;
  PyObject * memview__arg7_weights = NULL;
  Py_buffer * buf__arg7_weights;
  double * _arg7_weights = NULL;
  if (! PyArg_ParseTuple(_args,"LLOO",&_arg1_idx,&_arg2_maxnum,&memview__arg6_sub,&memview__arg7_weights)) goto ERROR;
  if (Py_None != memview__arg6_sub)
  {
    if (! PyMemoryView_Check(memview__arg6_sub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_sub");
      goto ERROR;
    }
    buf__arg6_sub = PyMemoryView_GET_BUFFER(memview__arg6_sub);
    if (buf__arg6_sub->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_sub");
        goto ERROR;
      
    }
    _arg6_sub = (long long*) buf__arg6_sub->buf;
    if (buf__arg6_sub->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg6_sub");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg7_weights)
  {
    if (! PyMemoryView_Check(memview__arg7_weights))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg7_weights");
      goto ERROR;
    }
    buf__arg7_weights = PyMemoryView_GET_BUFFER(memview__arg7_weights);
    if (buf__arg7_weights->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg7_weights");
        goto ERROR;
      
    }
    _arg7_weights = (double*) buf__arg7_weights->buf;
    if (buf__arg7_weights->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg7_weights");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getbaraidx(self->ptr,_arg1_idx,_arg2_maxnum,&_arg3_i,&_arg4_j,&_arg5_num,_arg6_sub,_arg7_weights);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(i(iiL))",0,_arg3_i,_arg4_j,_arg5_num);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getbaraidx */

static PyObject * PyMSK_getnumbarcblocktriplets(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_num; //  t = <aisread.aisTypeRef object at 0x7fb398387b00>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getnumbarcblocktriplets(self->ptr,&_arg1_num);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iL)",0,_arg1_num);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getnumbarcblocktriplets */

static PyObject * PyMSK_putbarcblocktriplet(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_num;
  PyObject * memview__arg2_subj = NULL;
  Py_buffer * buf__arg2_subj;
  int * _arg2_subj = NULL;
  PyObject * memview__arg3_subk = NULL;
  Py_buffer * buf__arg3_subk;
  int * _arg3_subk = NULL;
  PyObject * memview__arg4_subl = NULL;
  Py_buffer * buf__arg4_subl;
  int * _arg4_subl = NULL;
  PyObject * memview__arg5_valjkl = NULL;
  Py_buffer * buf__arg5_valjkl;
  double * _arg5_valjkl = NULL;
  if (! PyArg_ParseTuple(_args,"LOOOO",&_arg1_num,&memview__arg2_subj,&memview__arg3_subk,&memview__arg4_subl,&memview__arg5_valjkl)) goto ERROR;
  if (Py_None != memview__arg2_subj)
  {
    if (! PyMemoryView_Check(memview__arg2_subj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_subj");
      goto ERROR;
    }
    buf__arg2_subj = PyMemoryView_GET_BUFFER(memview__arg2_subj);
    if (buf__arg2_subj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_subj");
        goto ERROR;
      
    }
    _arg2_subj = (int*) buf__arg2_subj->buf;
  }
  if (Py_None != memview__arg3_subk)
  {
    if (! PyMemoryView_Check(memview__arg3_subk))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_subk");
      goto ERROR;
    }
    buf__arg3_subk = PyMemoryView_GET_BUFFER(memview__arg3_subk);
    if (buf__arg3_subk->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_subk");
        goto ERROR;
      
    }
    _arg3_subk = (int*) buf__arg3_subk->buf;
  }
  if (Py_None != memview__arg4_subl)
  {
    if (! PyMemoryView_Check(memview__arg4_subl))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_subl");
      goto ERROR;
    }
    buf__arg4_subl = PyMemoryView_GET_BUFFER(memview__arg4_subl);
    if (buf__arg4_subl->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_subl");
        goto ERROR;
      
    }
    _arg4_subl = (int*) buf__arg4_subl->buf;
  }
  if (Py_None != memview__arg5_valjkl)
  {
    if (! PyMemoryView_Check(memview__arg5_valjkl))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_valjkl");
      goto ERROR;
    }
    buf__arg5_valjkl = PyMemoryView_GET_BUFFER(memview__arg5_valjkl);
    if (buf__arg5_valjkl->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_valjkl");
        goto ERROR;
      
    }
    _arg5_valjkl = (double*) buf__arg5_valjkl->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putbarcblocktriplet(self->ptr,_arg1_num,_arg2_subj,_arg3_subk,_arg4_subl,_arg5_valjkl);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putbarcblocktriplet */

static PyObject * PyMSK_getbarcblocktriplet(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_maxnum;
  long long _arg2_num; //  t = <aisread.aisTypeRef object at 0x7fb39838e898>
  PyObject * memview__arg3_subj = NULL;
  Py_buffer * buf__arg3_subj;
  int * _arg3_subj = NULL;
  PyObject * memview__arg4_subk = NULL;
  Py_buffer * buf__arg4_subk;
  int * _arg4_subk = NULL;
  PyObject * memview__arg5_subl = NULL;
  Py_buffer * buf__arg5_subl;
  int * _arg5_subl = NULL;
  PyObject * memview__arg6_valjkl = NULL;
  Py_buffer * buf__arg6_valjkl;
  double * _arg6_valjkl = NULL;
  if (! PyArg_ParseTuple(_args,"LOOOO",&_arg1_maxnum,&memview__arg3_subj,&memview__arg4_subk,&memview__arg5_subl,&memview__arg6_valjkl)) goto ERROR;
  if (Py_None != memview__arg3_subj)
  {
    if (! PyMemoryView_Check(memview__arg3_subj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_subj");
      goto ERROR;
    }
    buf__arg3_subj = PyMemoryView_GET_BUFFER(memview__arg3_subj);
    if (buf__arg3_subj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_subj");
        goto ERROR;
      
    }
    _arg3_subj = (int*) buf__arg3_subj->buf;
    if (buf__arg3_subj->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg3_subj");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg4_subk)
  {
    if (! PyMemoryView_Check(memview__arg4_subk))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_subk");
      goto ERROR;
    }
    buf__arg4_subk = PyMemoryView_GET_BUFFER(memview__arg4_subk);
    if (buf__arg4_subk->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_subk");
        goto ERROR;
      
    }
    _arg4_subk = (int*) buf__arg4_subk->buf;
    if (buf__arg4_subk->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_subk");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg5_subl)
  {
    if (! PyMemoryView_Check(memview__arg5_subl))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_subl");
      goto ERROR;
    }
    buf__arg5_subl = PyMemoryView_GET_BUFFER(memview__arg5_subl);
    if (buf__arg5_subl->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_subl");
        goto ERROR;
      
    }
    _arg5_subl = (int*) buf__arg5_subl->buf;
    if (buf__arg5_subl->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg5_subl");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg6_valjkl)
  {
    if (! PyMemoryView_Check(memview__arg6_valjkl))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_valjkl");
      goto ERROR;
    }
    buf__arg6_valjkl = PyMemoryView_GET_BUFFER(memview__arg6_valjkl);
    if (buf__arg6_valjkl->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_valjkl");
        goto ERROR;
      
    }
    _arg6_valjkl = (double*) buf__arg6_valjkl->buf;
    if (buf__arg6_valjkl->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg6_valjkl");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getbarcblocktriplet(self->ptr,_arg1_maxnum,&_arg2_num,_arg3_subj,_arg4_subk,_arg5_subl,_arg6_valjkl);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iL)",0,_arg2_num);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getbarcblocktriplet */

static PyObject * PyMSK_putbarablocktriplet(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_num;
  PyObject * memview__arg2_subi = NULL;
  Py_buffer * buf__arg2_subi;
  int * _arg2_subi = NULL;
  PyObject * memview__arg3_subj = NULL;
  Py_buffer * buf__arg3_subj;
  int * _arg3_subj = NULL;
  PyObject * memview__arg4_subk = NULL;
  Py_buffer * buf__arg4_subk;
  int * _arg4_subk = NULL;
  PyObject * memview__arg5_subl = NULL;
  Py_buffer * buf__arg5_subl;
  int * _arg5_subl = NULL;
  PyObject * memview__arg6_valijkl = NULL;
  Py_buffer * buf__arg6_valijkl;
  double * _arg6_valijkl = NULL;
  if (! PyArg_ParseTuple(_args,"LOOOOO",&_arg1_num,&memview__arg2_subi,&memview__arg3_subj,&memview__arg4_subk,&memview__arg5_subl,&memview__arg6_valijkl)) goto ERROR;
  if (Py_None != memview__arg2_subi)
  {
    if (! PyMemoryView_Check(memview__arg2_subi))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_subi");
      goto ERROR;
    }
    buf__arg2_subi = PyMemoryView_GET_BUFFER(memview__arg2_subi);
    if (buf__arg2_subi->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_subi");
        goto ERROR;
      
    }
    _arg2_subi = (int*) buf__arg2_subi->buf;
  }
  if (Py_None != memview__arg3_subj)
  {
    if (! PyMemoryView_Check(memview__arg3_subj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_subj");
      goto ERROR;
    }
    buf__arg3_subj = PyMemoryView_GET_BUFFER(memview__arg3_subj);
    if (buf__arg3_subj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_subj");
        goto ERROR;
      
    }
    _arg3_subj = (int*) buf__arg3_subj->buf;
  }
  if (Py_None != memview__arg4_subk)
  {
    if (! PyMemoryView_Check(memview__arg4_subk))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_subk");
      goto ERROR;
    }
    buf__arg4_subk = PyMemoryView_GET_BUFFER(memview__arg4_subk);
    if (buf__arg4_subk->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_subk");
        goto ERROR;
      
    }
    _arg4_subk = (int*) buf__arg4_subk->buf;
  }
  if (Py_None != memview__arg5_subl)
  {
    if (! PyMemoryView_Check(memview__arg5_subl))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_subl");
      goto ERROR;
    }
    buf__arg5_subl = PyMemoryView_GET_BUFFER(memview__arg5_subl);
    if (buf__arg5_subl->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_subl");
        goto ERROR;
      
    }
    _arg5_subl = (int*) buf__arg5_subl->buf;
  }
  if (Py_None != memview__arg6_valijkl)
  {
    if (! PyMemoryView_Check(memview__arg6_valijkl))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_valijkl");
      goto ERROR;
    }
    buf__arg6_valijkl = PyMemoryView_GET_BUFFER(memview__arg6_valijkl);
    if (buf__arg6_valijkl->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_valijkl");
        goto ERROR;
      
    }
    _arg6_valijkl = (double*) buf__arg6_valijkl->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putbarablocktriplet(self->ptr,_arg1_num,_arg2_subi,_arg3_subj,_arg4_subk,_arg5_subl,_arg6_valijkl);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putbarablocktriplet */

static PyObject * PyMSK_getnumbarablocktriplets(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_num; //  t = <aisread.aisTypeRef object at 0x7fb398394b70>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getnumbarablocktriplets(self->ptr,&_arg1_num);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iL)",0,_arg1_num);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getnumbarablocktriplets */

static PyObject * PyMSK_getbarablocktriplet(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_maxnum;
  long long _arg2_num; //  t = <aisread.aisTypeRef object at 0x7fb39839a080>
  PyObject * memview__arg3_subi = NULL;
  Py_buffer * buf__arg3_subi;
  int * _arg3_subi = NULL;
  PyObject * memview__arg4_subj = NULL;
  Py_buffer * buf__arg4_subj;
  int * _arg4_subj = NULL;
  PyObject * memview__arg5_subk = NULL;
  Py_buffer * buf__arg5_subk;
  int * _arg5_subk = NULL;
  PyObject * memview__arg6_subl = NULL;
  Py_buffer * buf__arg6_subl;
  int * _arg6_subl = NULL;
  PyObject * memview__arg7_valijkl = NULL;
  Py_buffer * buf__arg7_valijkl;
  double * _arg7_valijkl = NULL;
  if (! PyArg_ParseTuple(_args,"LOOOOO",&_arg1_maxnum,&memview__arg3_subi,&memview__arg4_subj,&memview__arg5_subk,&memview__arg6_subl,&memview__arg7_valijkl)) goto ERROR;
  if (Py_None != memview__arg3_subi)
  {
    if (! PyMemoryView_Check(memview__arg3_subi))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_subi");
      goto ERROR;
    }
    buf__arg3_subi = PyMemoryView_GET_BUFFER(memview__arg3_subi);
    if (buf__arg3_subi->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_subi");
        goto ERROR;
      
    }
    _arg3_subi = (int*) buf__arg3_subi->buf;
    if (buf__arg3_subi->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg3_subi");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg4_subj)
  {
    if (! PyMemoryView_Check(memview__arg4_subj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_subj");
      goto ERROR;
    }
    buf__arg4_subj = PyMemoryView_GET_BUFFER(memview__arg4_subj);
    if (buf__arg4_subj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_subj");
        goto ERROR;
      
    }
    _arg4_subj = (int*) buf__arg4_subj->buf;
    if (buf__arg4_subj->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_subj");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg5_subk)
  {
    if (! PyMemoryView_Check(memview__arg5_subk))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_subk");
      goto ERROR;
    }
    buf__arg5_subk = PyMemoryView_GET_BUFFER(memview__arg5_subk);
    if (buf__arg5_subk->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_subk");
        goto ERROR;
      
    }
    _arg5_subk = (int*) buf__arg5_subk->buf;
    if (buf__arg5_subk->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg5_subk");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg6_subl)
  {
    if (! PyMemoryView_Check(memview__arg6_subl))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_subl");
      goto ERROR;
    }
    buf__arg6_subl = PyMemoryView_GET_BUFFER(memview__arg6_subl);
    if (buf__arg6_subl->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_subl");
        goto ERROR;
      
    }
    _arg6_subl = (int*) buf__arg6_subl->buf;
    if (buf__arg6_subl->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg6_subl");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg7_valijkl)
  {
    if (! PyMemoryView_Check(memview__arg7_valijkl))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg7_valijkl");
      goto ERROR;
    }
    buf__arg7_valijkl = PyMemoryView_GET_BUFFER(memview__arg7_valijkl);
    if (buf__arg7_valijkl->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg7_valijkl");
        goto ERROR;
      
    }
    _arg7_valijkl = (double*) buf__arg7_valijkl->buf;
    if (buf__arg7_valijkl->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg7_valijkl");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getbarablocktriplet(self->ptr,_arg1_maxnum,&_arg2_num,_arg3_subi,_arg4_subj,_arg5_subk,_arg6_subl,_arg7_valijkl);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iL)",0,_arg2_num);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getbarablocktriplet */

static PyObject * PyMSK_putconbound(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_i;
  MSKboundkeye _arg2_bkc;
  double _arg3_blc;
  double _arg4_buc;
  if (! PyArg_ParseTuple(_args,"iidd",&_arg1_i,&_arg2_bkc,&_arg3_blc,&_arg4_buc)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putconbound(self->ptr,_arg1_i,_arg2_bkc,_arg3_blc,_arg4_buc);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putconbound */

static PyObject * PyMSK_putconboundlist(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  PyObject * memview__arg2_sub = NULL;
  Py_buffer * buf__arg2_sub;
  int * _arg2_sub = NULL;
  PyObject * memview__arg3_bkc = NULL;
  Py_buffer * buf__arg3_bkc;
  MSKboundkeye * _arg3_bkc = NULL;
  PyObject * memview__arg4_blc = NULL;
  Py_buffer * buf__arg4_blc;
  double * _arg4_blc = NULL;
  PyObject * memview__arg5_buc = NULL;
  Py_buffer * buf__arg5_buc;
  double * _arg5_buc = NULL;
  if (! PyArg_ParseTuple(_args,"iOOOO",&_arg1_num,&memview__arg2_sub,&memview__arg3_bkc,&memview__arg4_blc,&memview__arg5_buc)) goto ERROR;
  if (Py_None != memview__arg2_sub)
  {
    if (! PyMemoryView_Check(memview__arg2_sub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_sub");
      goto ERROR;
    }
    buf__arg2_sub = PyMemoryView_GET_BUFFER(memview__arg2_sub);
    if (buf__arg2_sub->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_sub");
        goto ERROR;
      
    }
    _arg2_sub = (int*) buf__arg2_sub->buf;
  }
  if (Py_None != memview__arg3_bkc)
  {
    if (! PyMemoryView_Check(memview__arg3_bkc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_bkc");
      goto ERROR;
    }
    buf__arg3_bkc = PyMemoryView_GET_BUFFER(memview__arg3_bkc);
    if (buf__arg3_bkc->strides[0] != sizeof(MSKboundkeye))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_bkc");
        goto ERROR;
      
    }
    _arg3_bkc = (MSKboundkeye*) buf__arg3_bkc->buf;
  }
  if (Py_None != memview__arg4_blc)
  {
    if (! PyMemoryView_Check(memview__arg4_blc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_blc");
      goto ERROR;
    }
    buf__arg4_blc = PyMemoryView_GET_BUFFER(memview__arg4_blc);
    if (buf__arg4_blc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_blc");
        goto ERROR;
      
    }
    _arg4_blc = (double*) buf__arg4_blc->buf;
  }
  if (Py_None != memview__arg5_buc)
  {
    if (! PyMemoryView_Check(memview__arg5_buc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_buc");
      goto ERROR;
    }
    buf__arg5_buc = PyMemoryView_GET_BUFFER(memview__arg5_buc);
    if (buf__arg5_buc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_buc");
        goto ERROR;
      
    }
    _arg5_buc = (double*) buf__arg5_buc->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putconboundlist(self->ptr,_arg1_num,_arg2_sub,_arg3_bkc,_arg4_blc,_arg5_buc);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putconboundlist */

static PyObject * PyMSK_putconboundlistconst(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  PyObject * memview__arg2_sub = NULL;
  Py_buffer * buf__arg2_sub;
  int * _arg2_sub = NULL;
  MSKboundkeye _arg3_bkc;
  double _arg4_blc;
  double _arg5_buc;
  if (! PyArg_ParseTuple(_args,"iOidd",&_arg1_num,&memview__arg2_sub,&_arg3_bkc,&_arg4_blc,&_arg5_buc)) goto ERROR;
  if (Py_None != memview__arg2_sub)
  {
    if (! PyMemoryView_Check(memview__arg2_sub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_sub");
      goto ERROR;
    }
    buf__arg2_sub = PyMemoryView_GET_BUFFER(memview__arg2_sub);
    if (buf__arg2_sub->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_sub");
        goto ERROR;
      
    }
    _arg2_sub = (int*) buf__arg2_sub->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putconboundlistconst(self->ptr,_arg1_num,_arg2_sub,_arg3_bkc,_arg4_blc,_arg5_buc);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putconboundlistconst */

static PyObject * PyMSK_putconboundslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_first;
  int _arg2_last;
  PyObject * memview__arg3_bkc = NULL;
  Py_buffer * buf__arg3_bkc;
  MSKboundkeye * _arg3_bkc = NULL;
  PyObject * memview__arg4_blc = NULL;
  Py_buffer * buf__arg4_blc;
  double * _arg4_blc = NULL;
  PyObject * memview__arg5_buc = NULL;
  Py_buffer * buf__arg5_buc;
  double * _arg5_buc = NULL;
  if (! PyArg_ParseTuple(_args,"iiOOO",&_arg1_first,&_arg2_last,&memview__arg3_bkc,&memview__arg4_blc,&memview__arg5_buc)) goto ERROR;
  if (Py_None != memview__arg3_bkc)
  {
    if (! PyMemoryView_Check(memview__arg3_bkc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_bkc");
      goto ERROR;
    }
    buf__arg3_bkc = PyMemoryView_GET_BUFFER(memview__arg3_bkc);
    if (buf__arg3_bkc->strides[0] != sizeof(MSKboundkeye))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_bkc");
        goto ERROR;
      
    }
    _arg3_bkc = (MSKboundkeye*) buf__arg3_bkc->buf;
  }
  if (Py_None != memview__arg4_blc)
  {
    if (! PyMemoryView_Check(memview__arg4_blc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_blc");
      goto ERROR;
    }
    buf__arg4_blc = PyMemoryView_GET_BUFFER(memview__arg4_blc);
    if (buf__arg4_blc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_blc");
        goto ERROR;
      
    }
    _arg4_blc = (double*) buf__arg4_blc->buf;
  }
  if (Py_None != memview__arg5_buc)
  {
    if (! PyMemoryView_Check(memview__arg5_buc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_buc");
      goto ERROR;
    }
    buf__arg5_buc = PyMemoryView_GET_BUFFER(memview__arg5_buc);
    if (buf__arg5_buc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_buc");
        goto ERROR;
      
    }
    _arg5_buc = (double*) buf__arg5_buc->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putconboundslice(self->ptr,_arg1_first,_arg2_last,_arg3_bkc,_arg4_blc,_arg5_buc);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putconboundslice */

static PyObject * PyMSK_putconboundsliceconst(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_first;
  int _arg2_last;
  MSKboundkeye _arg3_bkc;
  double _arg4_blc;
  double _arg5_buc;
  if (! PyArg_ParseTuple(_args,"iiidd",&_arg1_first,&_arg2_last,&_arg3_bkc,&_arg4_blc,&_arg5_buc)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putconboundsliceconst(self->ptr,_arg1_first,_arg2_last,_arg3_bkc,_arg4_blc,_arg5_buc);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putconboundsliceconst */

static PyObject * PyMSK_putvarbound(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_j;
  MSKboundkeye _arg2_bkx;
  double _arg3_blx;
  double _arg4_bux;
  if (! PyArg_ParseTuple(_args,"iidd",&_arg1_j,&_arg2_bkx,&_arg3_blx,&_arg4_bux)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putvarbound(self->ptr,_arg1_j,_arg2_bkx,_arg3_blx,_arg4_bux);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putvarbound */

static PyObject * PyMSK_putvarboundlist(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  PyObject * memview__arg2_sub = NULL;
  Py_buffer * buf__arg2_sub;
  int * _arg2_sub = NULL;
  PyObject * memview__arg3_bkx = NULL;
  Py_buffer * buf__arg3_bkx;
  MSKboundkeye * _arg3_bkx = NULL;
  PyObject * memview__arg4_blx = NULL;
  Py_buffer * buf__arg4_blx;
  double * _arg4_blx = NULL;
  PyObject * memview__arg5_bux = NULL;
  Py_buffer * buf__arg5_bux;
  double * _arg5_bux = NULL;
  if (! PyArg_ParseTuple(_args,"iOOOO",&_arg1_num,&memview__arg2_sub,&memview__arg3_bkx,&memview__arg4_blx,&memview__arg5_bux)) goto ERROR;
  if (Py_None != memview__arg2_sub)
  {
    if (! PyMemoryView_Check(memview__arg2_sub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_sub");
      goto ERROR;
    }
    buf__arg2_sub = PyMemoryView_GET_BUFFER(memview__arg2_sub);
    if (buf__arg2_sub->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_sub");
        goto ERROR;
      
    }
    _arg2_sub = (int*) buf__arg2_sub->buf;
  }
  if (Py_None != memview__arg3_bkx)
  {
    if (! PyMemoryView_Check(memview__arg3_bkx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_bkx");
      goto ERROR;
    }
    buf__arg3_bkx = PyMemoryView_GET_BUFFER(memview__arg3_bkx);
    if (buf__arg3_bkx->strides[0] != sizeof(MSKboundkeye))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_bkx");
        goto ERROR;
      
    }
    _arg3_bkx = (MSKboundkeye*) buf__arg3_bkx->buf;
  }
  if (Py_None != memview__arg4_blx)
  {
    if (! PyMemoryView_Check(memview__arg4_blx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_blx");
      goto ERROR;
    }
    buf__arg4_blx = PyMemoryView_GET_BUFFER(memview__arg4_blx);
    if (buf__arg4_blx->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_blx");
        goto ERROR;
      
    }
    _arg4_blx = (double*) buf__arg4_blx->buf;
  }
  if (Py_None != memview__arg5_bux)
  {
    if (! PyMemoryView_Check(memview__arg5_bux))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_bux");
      goto ERROR;
    }
    buf__arg5_bux = PyMemoryView_GET_BUFFER(memview__arg5_bux);
    if (buf__arg5_bux->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_bux");
        goto ERROR;
      
    }
    _arg5_bux = (double*) buf__arg5_bux->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putvarboundlist(self->ptr,_arg1_num,_arg2_sub,_arg3_bkx,_arg4_blx,_arg5_bux);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putvarboundlist */

static PyObject * PyMSK_putvarboundlistconst(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  PyObject * memview__arg2_sub = NULL;
  Py_buffer * buf__arg2_sub;
  int * _arg2_sub = NULL;
  MSKboundkeye _arg3_bkx;
  double _arg4_blx;
  double _arg5_bux;
  if (! PyArg_ParseTuple(_args,"iOidd",&_arg1_num,&memview__arg2_sub,&_arg3_bkx,&_arg4_blx,&_arg5_bux)) goto ERROR;
  if (Py_None != memview__arg2_sub)
  {
    if (! PyMemoryView_Check(memview__arg2_sub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_sub");
      goto ERROR;
    }
    buf__arg2_sub = PyMemoryView_GET_BUFFER(memview__arg2_sub);
    if (buf__arg2_sub->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_sub");
        goto ERROR;
      
    }
    _arg2_sub = (int*) buf__arg2_sub->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putvarboundlistconst(self->ptr,_arg1_num,_arg2_sub,_arg3_bkx,_arg4_blx,_arg5_bux);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putvarboundlistconst */

static PyObject * PyMSK_putvarboundslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_first;
  int _arg2_last;
  PyObject * memview__arg3_bkx = NULL;
  Py_buffer * buf__arg3_bkx;
  MSKboundkeye * _arg3_bkx = NULL;
  PyObject * memview__arg4_blx = NULL;
  Py_buffer * buf__arg4_blx;
  double * _arg4_blx = NULL;
  PyObject * memview__arg5_bux = NULL;
  Py_buffer * buf__arg5_bux;
  double * _arg5_bux = NULL;
  if (! PyArg_ParseTuple(_args,"iiOOO",&_arg1_first,&_arg2_last,&memview__arg3_bkx,&memview__arg4_blx,&memview__arg5_bux)) goto ERROR;
  if (Py_None != memview__arg3_bkx)
  {
    if (! PyMemoryView_Check(memview__arg3_bkx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_bkx");
      goto ERROR;
    }
    buf__arg3_bkx = PyMemoryView_GET_BUFFER(memview__arg3_bkx);
    if (buf__arg3_bkx->strides[0] != sizeof(MSKboundkeye))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_bkx");
        goto ERROR;
      
    }
    _arg3_bkx = (MSKboundkeye*) buf__arg3_bkx->buf;
  }
  if (Py_None != memview__arg4_blx)
  {
    if (! PyMemoryView_Check(memview__arg4_blx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_blx");
      goto ERROR;
    }
    buf__arg4_blx = PyMemoryView_GET_BUFFER(memview__arg4_blx);
    if (buf__arg4_blx->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_blx");
        goto ERROR;
      
    }
    _arg4_blx = (double*) buf__arg4_blx->buf;
  }
  if (Py_None != memview__arg5_bux)
  {
    if (! PyMemoryView_Check(memview__arg5_bux))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_bux");
      goto ERROR;
    }
    buf__arg5_bux = PyMemoryView_GET_BUFFER(memview__arg5_bux);
    if (buf__arg5_bux->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_bux");
        goto ERROR;
      
    }
    _arg5_bux = (double*) buf__arg5_bux->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putvarboundslice(self->ptr,_arg1_first,_arg2_last,_arg3_bkx,_arg4_blx,_arg5_bux);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putvarboundslice */

static PyObject * PyMSK_putvarboundsliceconst(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_first;
  int _arg2_last;
  MSKboundkeye _arg3_bkx;
  double _arg4_blx;
  double _arg5_bux;
  if (! PyArg_ParseTuple(_args,"iiidd",&_arg1_first,&_arg2_last,&_arg3_bkx,&_arg4_blx,&_arg5_bux)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putvarboundsliceconst(self->ptr,_arg1_first,_arg2_last,_arg3_bkx,_arg4_blx,_arg5_bux);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putvarboundsliceconst */

static PyObject * PyMSK_putcfix(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  double _arg1_cfix;
  if (! PyArg_ParseTuple(_args,"d",&_arg1_cfix)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putcfix(self->ptr,_arg1_cfix);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putcfix */

static PyObject * PyMSK_putcj(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_j;
  double _arg2_cj;
  if (! PyArg_ParseTuple(_args,"id",&_arg1_j,&_arg2_cj)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putcj(self->ptr,_arg1_j,_arg2_cj);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putcj */

static PyObject * PyMSK_putobjsense(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKobjsensee _arg1_sense;
  if (! PyArg_ParseTuple(_args,"i",&_arg1_sense)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putobjsense(self->ptr,_arg1_sense);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putobjsense */

static PyObject * PyMSK_getobjsense(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKobjsensee _arg1_sense; //  t = <aisread.aisTypeRef object at 0x7fb3983434a8>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getobjsense(self->ptr,&_arg1_sense);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg1_sense);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getobjsense */

static PyObject * PyMSK_putclist(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  PyObject * memview__arg2_subj = NULL;
  Py_buffer * buf__arg2_subj;
  int * _arg2_subj = NULL;
  PyObject * memview__arg3_val = NULL;
  Py_buffer * buf__arg3_val;
  double * _arg3_val = NULL;
  if (! PyArg_ParseTuple(_args,"iOO",&_arg1_num,&memview__arg2_subj,&memview__arg3_val)) goto ERROR;
  if (Py_None != memview__arg2_subj)
  {
    if (! PyMemoryView_Check(memview__arg2_subj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_subj");
      goto ERROR;
    }
    buf__arg2_subj = PyMemoryView_GET_BUFFER(memview__arg2_subj);
    if (buf__arg2_subj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_subj");
        goto ERROR;
      
    }
    _arg2_subj = (int*) buf__arg2_subj->buf;
  }
  if (Py_None != memview__arg3_val)
  {
    if (! PyMemoryView_Check(memview__arg3_val))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_val");
      goto ERROR;
    }
    buf__arg3_val = PyMemoryView_GET_BUFFER(memview__arg3_val);
    if (buf__arg3_val->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_val");
        goto ERROR;
      
    }
    _arg3_val = (double*) buf__arg3_val->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putclist(self->ptr,_arg1_num,_arg2_subj,_arg3_val);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putclist */

static PyObject * PyMSK_putcslice(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_first;
  int _arg2_last;
  PyObject * memview__arg3_slice = NULL;
  Py_buffer * buf__arg3_slice;
  double * _arg3_slice = NULL;
  if (! PyArg_ParseTuple(_args,"iiO",&_arg1_first,&_arg2_last,&memview__arg3_slice)) goto ERROR;
  if (Py_None != memview__arg3_slice)
  {
    if (! PyMemoryView_Check(memview__arg3_slice))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_slice");
      goto ERROR;
    }
    buf__arg3_slice = PyMemoryView_GET_BUFFER(memview__arg3_slice);
    if (buf__arg3_slice->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_slice");
        goto ERROR;
      
    }
    _arg3_slice = (double*) buf__arg3_slice->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putcslice(self->ptr,_arg1_first,_arg2_last,_arg3_slice);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putcslice */

static PyObject * PyMSK_putbarcj(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_j;
  long long _arg2_num;
  PyObject * memview__arg3_sub = NULL;
  Py_buffer * buf__arg3_sub;
  long long * _arg3_sub = NULL;
  PyObject * memview__arg4_weights = NULL;
  Py_buffer * buf__arg4_weights;
  double * _arg4_weights = NULL;
  if (! PyArg_ParseTuple(_args,"iLOO",&_arg1_j,&_arg2_num,&memview__arg3_sub,&memview__arg4_weights)) goto ERROR;
  if (Py_None != memview__arg3_sub)
  {
    if (! PyMemoryView_Check(memview__arg3_sub))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_sub");
      goto ERROR;
    }
    buf__arg3_sub = PyMemoryView_GET_BUFFER(memview__arg3_sub);
    if (buf__arg3_sub->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_sub");
        goto ERROR;
      
    }
    _arg3_sub = (long long*) buf__arg3_sub->buf;
  }
  if (Py_None != memview__arg4_weights)
  {
    if (! PyMemoryView_Check(memview__arg4_weights))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_weights");
      goto ERROR;
    }
    buf__arg4_weights = PyMemoryView_GET_BUFFER(memview__arg4_weights);
    if (buf__arg4_weights->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_weights");
        goto ERROR;
      
    }
    _arg4_weights = (double*) buf__arg4_weights->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putbarcj(self->ptr,_arg1_j,_arg2_num,_arg3_sub,_arg4_weights);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putbarcj */

static PyObject * PyMSK_putcone(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_k;
  MSKconetypee _arg2_ct;
  double _arg3_conepar;
  int _arg4_nummem;
  PyObject * memview__arg5_submem = NULL;
  Py_buffer * buf__arg5_submem;
  int * _arg5_submem = NULL;
  if (! PyArg_ParseTuple(_args,"iidiO",&_arg1_k,&_arg2_ct,&_arg3_conepar,&_arg4_nummem,&memview__arg5_submem)) goto ERROR;
  if (Py_None != memview__arg5_submem)
  {
    if (! PyMemoryView_Check(memview__arg5_submem))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_submem");
      goto ERROR;
    }
    buf__arg5_submem = PyMemoryView_GET_BUFFER(memview__arg5_submem);
    if (buf__arg5_submem->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_submem");
        goto ERROR;
      
    }
    _arg5_submem = (int*) buf__arg5_submem->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putcone(self->ptr,_arg1_k,_arg2_ct,_arg3_conepar,_arg4_nummem,_arg5_submem);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putcone */

static PyObject * PyMSK_appendsparsesymmat(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_dim;
  long long _arg2_nz;
  PyObject * memview__arg3_subi = NULL;
  Py_buffer * buf__arg3_subi;
  int * _arg3_subi = NULL;
  PyObject * memview__arg4_subj = NULL;
  Py_buffer * buf__arg4_subj;
  int * _arg4_subj = NULL;
  PyObject * memview__arg5_valij = NULL;
  Py_buffer * buf__arg5_valij;
  double * _arg5_valij = NULL;
  long long _arg6_idx; //  t = <aisread.aisTypeRef object at 0x7fb39834e748>
  if (! PyArg_ParseTuple(_args,"iLOOO",&_arg1_dim,&_arg2_nz,&memview__arg3_subi,&memview__arg4_subj,&memview__arg5_valij)) goto ERROR;
  if (Py_None != memview__arg3_subi)
  {
    if (! PyMemoryView_Check(memview__arg3_subi))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_subi");
      goto ERROR;
    }
    buf__arg3_subi = PyMemoryView_GET_BUFFER(memview__arg3_subi);
    if (buf__arg3_subi->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_subi");
        goto ERROR;
      
    }
    _arg3_subi = (int*) buf__arg3_subi->buf;
  }
  if (Py_None != memview__arg4_subj)
  {
    if (! PyMemoryView_Check(memview__arg4_subj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_subj");
      goto ERROR;
    }
    buf__arg4_subj = PyMemoryView_GET_BUFFER(memview__arg4_subj);
    if (buf__arg4_subj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_subj");
        goto ERROR;
      
    }
    _arg4_subj = (int*) buf__arg4_subj->buf;
  }
  if (Py_None != memview__arg5_valij)
  {
    if (! PyMemoryView_Check(memview__arg5_valij))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_valij");
      goto ERROR;
    }
    buf__arg5_valij = PyMemoryView_GET_BUFFER(memview__arg5_valij);
    if (buf__arg5_valij->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_valij");
        goto ERROR;
      
    }
    _arg5_valij = (double*) buf__arg5_valij->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_appendsparsesymmat(self->ptr,_arg1_dim,_arg2_nz,_arg3_subi,_arg4_subj,_arg5_valij,&_arg6_idx);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iL)",0,_arg6_idx);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* appendsparsesymmat */

static PyObject * PyMSK_appendsparsesymmatlist(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  PyObject * memview__arg2_dims = NULL;
  Py_buffer * buf__arg2_dims;
  int * _arg2_dims = NULL;
  PyObject * memview__arg3_nz = NULL;
  Py_buffer * buf__arg3_nz;
  long long * _arg3_nz = NULL;
  PyObject * memview__arg4_subi = NULL;
  Py_buffer * buf__arg4_subi;
  int * _arg4_subi = NULL;
  PyObject * memview__arg5_subj = NULL;
  Py_buffer * buf__arg5_subj;
  int * _arg5_subj = NULL;
  PyObject * memview__arg6_valij = NULL;
  Py_buffer * buf__arg6_valij;
  double * _arg6_valij = NULL;
  PyObject * memview__arg7_idx = NULL;
  Py_buffer * buf__arg7_idx;
  long long * _arg7_idx = NULL;
  if (! PyArg_ParseTuple(_args,"iOOOOOO",&_arg1_num,&memview__arg2_dims,&memview__arg3_nz,&memview__arg4_subi,&memview__arg5_subj,&memview__arg6_valij,&memview__arg7_idx)) goto ERROR;
  if (Py_None != memview__arg2_dims)
  {
    if (! PyMemoryView_Check(memview__arg2_dims))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_dims");
      goto ERROR;
    }
    buf__arg2_dims = PyMemoryView_GET_BUFFER(memview__arg2_dims);
    if (buf__arg2_dims->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_dims");
        goto ERROR;
      
    }
    _arg2_dims = (int*) buf__arg2_dims->buf;
  }
  if (Py_None != memview__arg3_nz)
  {
    if (! PyMemoryView_Check(memview__arg3_nz))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_nz");
      goto ERROR;
    }
    buf__arg3_nz = PyMemoryView_GET_BUFFER(memview__arg3_nz);
    if (buf__arg3_nz->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_nz");
        goto ERROR;
      
    }
    _arg3_nz = (long long*) buf__arg3_nz->buf;
  }
  if (Py_None != memview__arg4_subi)
  {
    if (! PyMemoryView_Check(memview__arg4_subi))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_subi");
      goto ERROR;
    }
    buf__arg4_subi = PyMemoryView_GET_BUFFER(memview__arg4_subi);
    if (buf__arg4_subi->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_subi");
        goto ERROR;
      
    }
    _arg4_subi = (int*) buf__arg4_subi->buf;
  }
  if (Py_None != memview__arg5_subj)
  {
    if (! PyMemoryView_Check(memview__arg5_subj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_subj");
      goto ERROR;
    }
    buf__arg5_subj = PyMemoryView_GET_BUFFER(memview__arg5_subj);
    if (buf__arg5_subj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_subj");
        goto ERROR;
      
    }
    _arg5_subj = (int*) buf__arg5_subj->buf;
  }
  if (Py_None != memview__arg6_valij)
  {
    if (! PyMemoryView_Check(memview__arg6_valij))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_valij");
      goto ERROR;
    }
    buf__arg6_valij = PyMemoryView_GET_BUFFER(memview__arg6_valij);
    if (buf__arg6_valij->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_valij");
        goto ERROR;
      
    }
    _arg6_valij = (double*) buf__arg6_valij->buf;
  }
  if (Py_None != memview__arg7_idx)
  {
    if (! PyMemoryView_Check(memview__arg7_idx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg7_idx");
      goto ERROR;
    }
    buf__arg7_idx = PyMemoryView_GET_BUFFER(memview__arg7_idx);
    if (buf__arg7_idx->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg7_idx");
        goto ERROR;
      
    }
    _arg7_idx = (long long*) buf__arg7_idx->buf;
    if (buf__arg7_idx->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg7_idx");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_appendsparsesymmatlist(self->ptr,_arg1_num,_arg2_dims,_arg3_nz,_arg4_subi,_arg5_subj,_arg6_valij,_arg7_idx);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* appendsparsesymmatlist */

static PyObject * PyMSK_getsymmatinfo(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_idx;
  int _arg2_dim; //  t = <aisread.aisTypeRef object at 0x7fb3983546d8>
  long long _arg3_nz; //  t = <aisread.aisTypeRef object at 0x7fb3983547f0>
  MSKsymmattypee _arg4_type; //  t = <aisread.aisTypeRef object at 0x7fb398354908>
  if (! PyArg_ParseTuple(_args,"L",&_arg1_idx)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getsymmatinfo(self->ptr,_arg1_idx,&_arg2_dim,&_arg3_nz,&_arg4_type);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(i(iLi))",0,_arg2_dim,_arg3_nz,_arg4_type);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getsymmatinfo */

static PyObject * PyMSK_getnumsymmat(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_num; //  t = <aisread.aisTypeRef object at 0x7fb398354c50>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getnumsymmat(self->ptr,&_arg1_num);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iL)",0,_arg1_num);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getnumsymmat */

static PyObject * PyMSK_getsparsesymmat(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_idx;
  long long _arg2_maxlen;
  PyObject * memview__arg3_subi = NULL;
  Py_buffer * buf__arg3_subi;
  int * _arg3_subi = NULL;
  PyObject * memview__arg4_subj = NULL;
  Py_buffer * buf__arg4_subj;
  int * _arg4_subj = NULL;
  PyObject * memview__arg5_valij = NULL;
  Py_buffer * buf__arg5_valij;
  double * _arg5_valij = NULL;
  if (! PyArg_ParseTuple(_args,"LLOOO",&_arg1_idx,&_arg2_maxlen,&memview__arg3_subi,&memview__arg4_subj,&memview__arg5_valij)) goto ERROR;
  if (Py_None != memview__arg3_subi)
  {
    if (! PyMemoryView_Check(memview__arg3_subi))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_subi");
      goto ERROR;
    }
    buf__arg3_subi = PyMemoryView_GET_BUFFER(memview__arg3_subi);
    if (buf__arg3_subi->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_subi");
        goto ERROR;
      
    }
    _arg3_subi = (int*) buf__arg3_subi->buf;
    if (buf__arg3_subi->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg3_subi");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg4_subj)
  {
    if (! PyMemoryView_Check(memview__arg4_subj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_subj");
      goto ERROR;
    }
    buf__arg4_subj = PyMemoryView_GET_BUFFER(memview__arg4_subj);
    if (buf__arg4_subj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_subj");
        goto ERROR;
      
    }
    _arg4_subj = (int*) buf__arg4_subj->buf;
    if (buf__arg4_subj->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_subj");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg5_valij)
  {
    if (! PyMemoryView_Check(memview__arg5_valij))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_valij");
      goto ERROR;
    }
    buf__arg5_valij = PyMemoryView_GET_BUFFER(memview__arg5_valij);
    if (buf__arg5_valij->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_valij");
        goto ERROR;
      
    }
    _arg5_valij = (double*) buf__arg5_valij->buf;
    if (buf__arg5_valij->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg5_valij");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getsparsesymmat(self->ptr,_arg1_idx,_arg2_maxlen,_arg3_subi,_arg4_subj,_arg5_valij);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* getsparsesymmat */

static PyObject * PyMSK_putdouparam(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKdparame _arg1_param;
  double _arg2_parvalue;
  if (! PyArg_ParseTuple(_args,"id",&_arg1_param,&_arg2_parvalue)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putdouparam(self->ptr,_arg1_param,_arg2_parvalue);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putdouparam */

static PyObject * PyMSK_putintparam(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKiparame _arg1_param;
  int _arg2_parvalue;
  if (! PyArg_ParseTuple(_args,"ii",&_arg1_param,&_arg2_parvalue)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putintparam(self->ptr,_arg1_param,_arg2_parvalue);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putintparam */

static PyObject * PyMSK_putmaxnumcon(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_maxnumcon;
  if (! PyArg_ParseTuple(_args,"i",&_arg1_maxnumcon)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putmaxnumcon(self->ptr,_arg1_maxnumcon);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putmaxnumcon */

static PyObject * PyMSK_putmaxnumcone(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_maxnumcone;
  if (! PyArg_ParseTuple(_args,"i",&_arg1_maxnumcone)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putmaxnumcone(self->ptr,_arg1_maxnumcone);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putmaxnumcone */

static PyObject * PyMSK_getmaxnumcone(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_maxnumcone; //  t = <aisread.aisTypeRef object at 0x7fb39835f7b8>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getmaxnumcone(self->ptr,&_arg1_maxnumcone);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg1_maxnumcone);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getmaxnumcone */

static PyObject * PyMSK_putmaxnumvar(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_maxnumvar;
  if (! PyArg_ParseTuple(_args,"i",&_arg1_maxnumvar)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putmaxnumvar(self->ptr,_arg1_maxnumvar);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putmaxnumvar */

static PyObject * PyMSK_putmaxnumbarvar(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_maxnumbarvar;
  if (! PyArg_ParseTuple(_args,"i",&_arg1_maxnumbarvar)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putmaxnumbarvar(self->ptr,_arg1_maxnumbarvar);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putmaxnumbarvar */

static PyObject * PyMSK_putmaxnumanz(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_maxnumanz;
  if (! PyArg_ParseTuple(_args,"L",&_arg1_maxnumanz)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putmaxnumanz(self->ptr,_arg1_maxnumanz);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putmaxnumanz */

static PyObject * PyMSK_putmaxnumqnz(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_maxnumqnz;
  if (! PyArg_ParseTuple(_args,"L",&_arg1_maxnumqnz)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putmaxnumqnz(self->ptr,_arg1_maxnumqnz);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putmaxnumqnz */

static PyObject * PyMSK_getmaxnumqnz64(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_maxnumqnz; //  t = <aisread.aisTypeRef object at 0x7fb398363d68>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getmaxnumqnz64(self->ptr,&_arg1_maxnumqnz);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iL)",0,_arg1_maxnumqnz);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getmaxnumqnz64 */

static PyObject * PyMSK_putnadouparam(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_paramname = NULL;
  double _arg2_parvalue;
  if (! PyArg_ParseTuple(_args,"sd",&str__arg1_paramname,&_arg2_parvalue)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putnadouparam(self->ptr,str__arg1_paramname,_arg2_parvalue);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putnadouparam */

static PyObject * PyMSK_putnaintparam(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_paramname = NULL;
  int _arg2_parvalue;
  if (! PyArg_ParseTuple(_args,"si",&str__arg1_paramname,&_arg2_parvalue)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putnaintparam(self->ptr,str__arg1_paramname,_arg2_parvalue);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putnaintparam */

static PyObject * PyMSK_putnastrparam(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_paramname = NULL;
  char * str__arg2_parvalue = NULL;
  if (! PyArg_ParseTuple(_args,"ss",&str__arg1_paramname,&str__arg2_parvalue)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putnastrparam(self->ptr,str__arg1_paramname,str__arg2_parvalue);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putnastrparam */

static PyObject * PyMSK_putobjname(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_objname = NULL;
  if (! PyArg_ParseTuple(_args,"s",&str__arg1_objname)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putobjname(self->ptr,str__arg1_objname);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putobjname */

static PyObject * PyMSK_putparam(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_parname = NULL;
  char * str__arg2_parvalue = NULL;
  if (! PyArg_ParseTuple(_args,"ss",&str__arg1_parname,&str__arg2_parvalue)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putparam(self->ptr,str__arg1_parname,str__arg2_parvalue);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putparam */

static PyObject * PyMSK_putqcon(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_numqcnz;
  PyObject * memview__arg2_qcsubk = NULL;
  Py_buffer * buf__arg2_qcsubk;
  int * _arg2_qcsubk = NULL;
  PyObject * memview__arg3_qcsubi = NULL;
  Py_buffer * buf__arg3_qcsubi;
  int * _arg3_qcsubi = NULL;
  PyObject * memview__arg4_qcsubj = NULL;
  Py_buffer * buf__arg4_qcsubj;
  int * _arg4_qcsubj = NULL;
  PyObject * memview__arg5_qcval = NULL;
  Py_buffer * buf__arg5_qcval;
  double * _arg5_qcval = NULL;
  if (! PyArg_ParseTuple(_args,"iOOOO",&_arg1_numqcnz,&memview__arg2_qcsubk,&memview__arg3_qcsubi,&memview__arg4_qcsubj,&memview__arg5_qcval)) goto ERROR;
  if (Py_None != memview__arg2_qcsubk)
  {
    if (! PyMemoryView_Check(memview__arg2_qcsubk))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_qcsubk");
      goto ERROR;
    }
    buf__arg2_qcsubk = PyMemoryView_GET_BUFFER(memview__arg2_qcsubk);
    if (buf__arg2_qcsubk->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_qcsubk");
        goto ERROR;
      
    }
    _arg2_qcsubk = (int*) buf__arg2_qcsubk->buf;
  }
  if (Py_None != memview__arg3_qcsubi)
  {
    if (! PyMemoryView_Check(memview__arg3_qcsubi))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_qcsubi");
      goto ERROR;
    }
    buf__arg3_qcsubi = PyMemoryView_GET_BUFFER(memview__arg3_qcsubi);
    if (buf__arg3_qcsubi->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_qcsubi");
        goto ERROR;
      
    }
    _arg3_qcsubi = (int*) buf__arg3_qcsubi->buf;
  }
  if (Py_None != memview__arg4_qcsubj)
  {
    if (! PyMemoryView_Check(memview__arg4_qcsubj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_qcsubj");
      goto ERROR;
    }
    buf__arg4_qcsubj = PyMemoryView_GET_BUFFER(memview__arg4_qcsubj);
    if (buf__arg4_qcsubj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_qcsubj");
        goto ERROR;
      
    }
    _arg4_qcsubj = (int*) buf__arg4_qcsubj->buf;
  }
  if (Py_None != memview__arg5_qcval)
  {
    if (! PyMemoryView_Check(memview__arg5_qcval))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_qcval");
      goto ERROR;
    }
    buf__arg5_qcval = PyMemoryView_GET_BUFFER(memview__arg5_qcval);
    if (buf__arg5_qcval->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_qcval");
        goto ERROR;
      
    }
    _arg5_qcval = (double*) buf__arg5_qcval->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putqcon(self->ptr,_arg1_numqcnz,_arg2_qcsubk,_arg3_qcsubi,_arg4_qcsubj,_arg5_qcval);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putqcon */

static PyObject * PyMSK_putqconk(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_k;
  int _arg2_numqcnz;
  PyObject * memview__arg3_qcsubi = NULL;
  Py_buffer * buf__arg3_qcsubi;
  int * _arg3_qcsubi = NULL;
  PyObject * memview__arg4_qcsubj = NULL;
  Py_buffer * buf__arg4_qcsubj;
  int * _arg4_qcsubj = NULL;
  PyObject * memview__arg5_qcval = NULL;
  Py_buffer * buf__arg5_qcval;
  double * _arg5_qcval = NULL;
  if (! PyArg_ParseTuple(_args,"iiOOO",&_arg1_k,&_arg2_numqcnz,&memview__arg3_qcsubi,&memview__arg4_qcsubj,&memview__arg5_qcval)) goto ERROR;
  if (Py_None != memview__arg3_qcsubi)
  {
    if (! PyMemoryView_Check(memview__arg3_qcsubi))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_qcsubi");
      goto ERROR;
    }
    buf__arg3_qcsubi = PyMemoryView_GET_BUFFER(memview__arg3_qcsubi);
    if (buf__arg3_qcsubi->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_qcsubi");
        goto ERROR;
      
    }
    _arg3_qcsubi = (int*) buf__arg3_qcsubi->buf;
  }
  if (Py_None != memview__arg4_qcsubj)
  {
    if (! PyMemoryView_Check(memview__arg4_qcsubj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_qcsubj");
      goto ERROR;
    }
    buf__arg4_qcsubj = PyMemoryView_GET_BUFFER(memview__arg4_qcsubj);
    if (buf__arg4_qcsubj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_qcsubj");
        goto ERROR;
      
    }
    _arg4_qcsubj = (int*) buf__arg4_qcsubj->buf;
  }
  if (Py_None != memview__arg5_qcval)
  {
    if (! PyMemoryView_Check(memview__arg5_qcval))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_qcval");
      goto ERROR;
    }
    buf__arg5_qcval = PyMemoryView_GET_BUFFER(memview__arg5_qcval);
    if (buf__arg5_qcval->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_qcval");
        goto ERROR;
      
    }
    _arg5_qcval = (double*) buf__arg5_qcval->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putqconk(self->ptr,_arg1_k,_arg2_numqcnz,_arg3_qcsubi,_arg4_qcsubj,_arg5_qcval);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putqconk */

static PyObject * PyMSK_putqobj(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_numqonz;
  PyObject * memview__arg2_qosubi = NULL;
  Py_buffer * buf__arg2_qosubi;
  int * _arg2_qosubi = NULL;
  PyObject * memview__arg3_qosubj = NULL;
  Py_buffer * buf__arg3_qosubj;
  int * _arg3_qosubj = NULL;
  PyObject * memview__arg4_qoval = NULL;
  Py_buffer * buf__arg4_qoval;
  double * _arg4_qoval = NULL;
  if (! PyArg_ParseTuple(_args,"iOOO",&_arg1_numqonz,&memview__arg2_qosubi,&memview__arg3_qosubj,&memview__arg4_qoval)) goto ERROR;
  if (Py_None != memview__arg2_qosubi)
  {
    if (! PyMemoryView_Check(memview__arg2_qosubi))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_qosubi");
      goto ERROR;
    }
    buf__arg2_qosubi = PyMemoryView_GET_BUFFER(memview__arg2_qosubi);
    if (buf__arg2_qosubi->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_qosubi");
        goto ERROR;
      
    }
    _arg2_qosubi = (int*) buf__arg2_qosubi->buf;
  }
  if (Py_None != memview__arg3_qosubj)
  {
    if (! PyMemoryView_Check(memview__arg3_qosubj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_qosubj");
      goto ERROR;
    }
    buf__arg3_qosubj = PyMemoryView_GET_BUFFER(memview__arg3_qosubj);
    if (buf__arg3_qosubj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_qosubj");
        goto ERROR;
      
    }
    _arg3_qosubj = (int*) buf__arg3_qosubj->buf;
  }
  if (Py_None != memview__arg4_qoval)
  {
    if (! PyMemoryView_Check(memview__arg4_qoval))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_qoval");
      goto ERROR;
    }
    buf__arg4_qoval = PyMemoryView_GET_BUFFER(memview__arg4_qoval);
    if (buf__arg4_qoval->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_qoval");
        goto ERROR;
      
    }
    _arg4_qoval = (double*) buf__arg4_qoval->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putqobj(self->ptr,_arg1_numqonz,_arg2_qosubi,_arg3_qosubj,_arg4_qoval);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putqobj */

static PyObject * PyMSK_putqobjij(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_i;
  int _arg2_j;
  double _arg3_qoij;
  if (! PyArg_ParseTuple(_args,"iid",&_arg1_i,&_arg2_j,&_arg3_qoij)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putqobjij(self->ptr,_arg1_i,_arg2_j,_arg3_qoij);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putqobjij */

static PyObject * PyMSK_putsolution(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  PyObject * memview__arg2_skc = NULL;
  Py_buffer * buf__arg2_skc;
  MSKstakeye * _arg2_skc = NULL;
  PyObject * memview__arg3_skx = NULL;
  Py_buffer * buf__arg3_skx;
  MSKstakeye * _arg3_skx = NULL;
  PyObject * memview__arg4_skn = NULL;
  Py_buffer * buf__arg4_skn;
  MSKstakeye * _arg4_skn = NULL;
  PyObject * memview__arg5_xc = NULL;
  Py_buffer * buf__arg5_xc;
  double * _arg5_xc = NULL;
  PyObject * memview__arg6_xx = NULL;
  Py_buffer * buf__arg6_xx;
  double * _arg6_xx = NULL;
  PyObject * memview__arg7_y = NULL;
  Py_buffer * buf__arg7_y;
  double * _arg7_y = NULL;
  PyObject * memview__arg8_slc = NULL;
  Py_buffer * buf__arg8_slc;
  double * _arg8_slc = NULL;
  PyObject * memview__arg9_suc = NULL;
  Py_buffer * buf__arg9_suc;
  double * _arg9_suc = NULL;
  PyObject * memview__arg10_slx = NULL;
  Py_buffer * buf__arg10_slx;
  double * _arg10_slx = NULL;
  PyObject * memview__arg11_sux = NULL;
  Py_buffer * buf__arg11_sux;
  double * _arg11_sux = NULL;
  PyObject * memview__arg12_snx = NULL;
  Py_buffer * buf__arg12_snx;
  double * _arg12_snx = NULL;
  if (! PyArg_ParseTuple(_args,"iOOOOOOOOOOO",&_arg1_whichsol,&memview__arg2_skc,&memview__arg3_skx,&memview__arg4_skn,&memview__arg5_xc,&memview__arg6_xx,&memview__arg7_y,&memview__arg8_slc,&memview__arg9_suc,&memview__arg10_slx,&memview__arg11_sux,&memview__arg12_snx)) goto ERROR;
  if (Py_None != memview__arg2_skc)
  {
    if (! PyMemoryView_Check(memview__arg2_skc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_skc");
      goto ERROR;
    }
    buf__arg2_skc = PyMemoryView_GET_BUFFER(memview__arg2_skc);
    if (buf__arg2_skc->strides[0] != sizeof(MSKstakeye))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_skc");
        goto ERROR;
      
    }
    _arg2_skc = (MSKstakeye*) buf__arg2_skc->buf;
  }
  if (Py_None != memview__arg3_skx)
  {
    if (! PyMemoryView_Check(memview__arg3_skx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_skx");
      goto ERROR;
    }
    buf__arg3_skx = PyMemoryView_GET_BUFFER(memview__arg3_skx);
    if (buf__arg3_skx->strides[0] != sizeof(MSKstakeye))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_skx");
        goto ERROR;
      
    }
    _arg3_skx = (MSKstakeye*) buf__arg3_skx->buf;
  }
  if (Py_None != memview__arg4_skn)
  {
    if (! PyMemoryView_Check(memview__arg4_skn))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_skn");
      goto ERROR;
    }
    buf__arg4_skn = PyMemoryView_GET_BUFFER(memview__arg4_skn);
    if (buf__arg4_skn->strides[0] != sizeof(MSKstakeye))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_skn");
        goto ERROR;
      
    }
    _arg4_skn = (MSKstakeye*) buf__arg4_skn->buf;
  }
  if (Py_None != memview__arg5_xc)
  {
    if (! PyMemoryView_Check(memview__arg5_xc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_xc");
      goto ERROR;
    }
    buf__arg5_xc = PyMemoryView_GET_BUFFER(memview__arg5_xc);
    if (buf__arg5_xc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_xc");
        goto ERROR;
      
    }
    _arg5_xc = (double*) buf__arg5_xc->buf;
  }
  if (Py_None != memview__arg6_xx)
  {
    if (! PyMemoryView_Check(memview__arg6_xx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_xx");
      goto ERROR;
    }
    buf__arg6_xx = PyMemoryView_GET_BUFFER(memview__arg6_xx);
    if (buf__arg6_xx->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_xx");
        goto ERROR;
      
    }
    _arg6_xx = (double*) buf__arg6_xx->buf;
  }
  if (Py_None != memview__arg7_y)
  {
    if (! PyMemoryView_Check(memview__arg7_y))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg7_y");
      goto ERROR;
    }
    buf__arg7_y = PyMemoryView_GET_BUFFER(memview__arg7_y);
    if (buf__arg7_y->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg7_y");
        goto ERROR;
      
    }
    _arg7_y = (double*) buf__arg7_y->buf;
  }
  if (Py_None != memview__arg8_slc)
  {
    if (! PyMemoryView_Check(memview__arg8_slc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg8_slc");
      goto ERROR;
    }
    buf__arg8_slc = PyMemoryView_GET_BUFFER(memview__arg8_slc);
    if (buf__arg8_slc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg8_slc");
        goto ERROR;
      
    }
    _arg8_slc = (double*) buf__arg8_slc->buf;
  }
  if (Py_None != memview__arg9_suc)
  {
    if (! PyMemoryView_Check(memview__arg9_suc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg9_suc");
      goto ERROR;
    }
    buf__arg9_suc = PyMemoryView_GET_BUFFER(memview__arg9_suc);
    if (buf__arg9_suc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg9_suc");
        goto ERROR;
      
    }
    _arg9_suc = (double*) buf__arg9_suc->buf;
  }
  if (Py_None != memview__arg10_slx)
  {
    if (! PyMemoryView_Check(memview__arg10_slx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg10_slx");
      goto ERROR;
    }
    buf__arg10_slx = PyMemoryView_GET_BUFFER(memview__arg10_slx);
    if (buf__arg10_slx->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg10_slx");
        goto ERROR;
      
    }
    _arg10_slx = (double*) buf__arg10_slx->buf;
  }
  if (Py_None != memview__arg11_sux)
  {
    if (! PyMemoryView_Check(memview__arg11_sux))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg11_sux");
      goto ERROR;
    }
    buf__arg11_sux = PyMemoryView_GET_BUFFER(memview__arg11_sux);
    if (buf__arg11_sux->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg11_sux");
        goto ERROR;
      
    }
    _arg11_sux = (double*) buf__arg11_sux->buf;
  }
  if (Py_None != memview__arg12_snx)
  {
    if (! PyMemoryView_Check(memview__arg12_snx))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg12_snx");
      goto ERROR;
    }
    buf__arg12_snx = PyMemoryView_GET_BUFFER(memview__arg12_snx);
    if (buf__arg12_snx->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg12_snx");
        goto ERROR;
      
    }
    _arg12_snx = (double*) buf__arg12_snx->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putsolution(self->ptr,_arg1_whichsol,_arg2_skc,_arg3_skx,_arg4_skn,_arg5_xc,_arg6_xx,_arg7_y,_arg8_slc,_arg9_suc,_arg10_slx,_arg11_sux,_arg12_snx);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putsolution */

static PyObject * PyMSK_putconsolutioni(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_i;
  MSKsoltypee _arg2_whichsol;
  MSKstakeye _arg3_sk;
  double _arg4_x;
  double _arg5_sl;
  double _arg6_su;
  if (! PyArg_ParseTuple(_args,"iiiddd",&_arg1_i,&_arg2_whichsol,&_arg3_sk,&_arg4_x,&_arg5_sl,&_arg6_su)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putconsolutioni(self->ptr,_arg1_i,_arg2_whichsol,_arg3_sk,_arg4_x,_arg5_sl,_arg6_su);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putconsolutioni */

static PyObject * PyMSK_putvarsolutionj(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_j;
  MSKsoltypee _arg2_whichsol;
  MSKstakeye _arg3_sk;
  double _arg4_x;
  double _arg5_sl;
  double _arg6_su;
  double _arg7_sn;
  if (! PyArg_ParseTuple(_args,"iiidddd",&_arg1_j,&_arg2_whichsol,&_arg3_sk,&_arg4_x,&_arg5_sl,&_arg6_su,&_arg7_sn)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putvarsolutionj(self->ptr,_arg1_j,_arg2_whichsol,_arg3_sk,_arg4_x,_arg5_sl,_arg6_su,_arg7_sn);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putvarsolutionj */

static PyObject * PyMSK_putsolutionyi(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_i;
  MSKsoltypee _arg2_whichsol;
  double _arg3_y;
  if (! PyArg_ParseTuple(_args,"iid",&_arg1_i,&_arg2_whichsol,&_arg3_y)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putsolutionyi(self->ptr,_arg1_i,_arg2_whichsol,_arg3_y);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putsolutionyi */

static PyObject * PyMSK_putstrparam(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsparame _arg1_param;
  char * str__arg2_parvalue = NULL;
  if (! PyArg_ParseTuple(_args,"is",&_arg1_param,&str__arg2_parvalue)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putstrparam(self->ptr,_arg1_param,str__arg2_parvalue);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putstrparam */

static PyObject * PyMSK_puttaskname(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_taskname = NULL;
  if (! PyArg_ParseTuple(_args,"s",&str__arg1_taskname)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_puttaskname(self->ptr,str__arg1_taskname);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* puttaskname */

static PyObject * PyMSK_putvartype(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_j;
  MSKvariabletypee _arg2_vartype;
  if (! PyArg_ParseTuple(_args,"ii",&_arg1_j,&_arg2_vartype)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putvartype(self->ptr,_arg1_j,_arg2_vartype);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putvartype */

static PyObject * PyMSK_putvartypelist(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_num;
  PyObject * memview__arg2_subj = NULL;
  Py_buffer * buf__arg2_subj;
  int * _arg2_subj = NULL;
  PyObject * memview__arg3_vartype = NULL;
  Py_buffer * buf__arg3_vartype;
  MSKvariabletypee * _arg3_vartype = NULL;
  if (! PyArg_ParseTuple(_args,"iOO",&_arg1_num,&memview__arg2_subj,&memview__arg3_vartype)) goto ERROR;
  if (Py_None != memview__arg2_subj)
  {
    if (! PyMemoryView_Check(memview__arg2_subj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_subj");
      goto ERROR;
    }
    buf__arg2_subj = PyMemoryView_GET_BUFFER(memview__arg2_subj);
    if (buf__arg2_subj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_subj");
        goto ERROR;
      
    }
    _arg2_subj = (int*) buf__arg2_subj->buf;
  }
  if (Py_None != memview__arg3_vartype)
  {
    if (! PyMemoryView_Check(memview__arg3_vartype))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_vartype");
      goto ERROR;
    }
    buf__arg3_vartype = PyMemoryView_GET_BUFFER(memview__arg3_vartype);
    if (buf__arg3_vartype->strides[0] != sizeof(MSKvariabletypee))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_vartype");
        goto ERROR;
      
    }
    _arg3_vartype = (MSKvariabletypee*) buf__arg3_vartype->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putvartypelist(self->ptr,_arg1_num,_arg2_subj,_arg3_vartype);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putvartypelist */

static PyObject * PyMSK_readdataformat(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_filename = NULL;
  MSKdataformate _arg2_format;
  MSKcompresstypee _arg3_compress;
  if (! PyArg_ParseTuple(_args,"sii",&str__arg1_filename,&_arg2_format,&_arg3_compress)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_readdataformat(self->ptr,str__arg1_filename,_arg2_format,_arg3_compress);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* readdataformat */

static PyObject * PyMSK_readdataautoformat(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_filename = NULL;
  if (! PyArg_ParseTuple(_args,"s",&str__arg1_filename)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_readdataautoformat(self->ptr,str__arg1_filename);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* readdataautoformat */

static PyObject * PyMSK_readparamfile(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_filename = NULL;
  if (! PyArg_ParseTuple(_args,"s",&str__arg1_filename)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_readparamfile(self->ptr,str__arg1_filename);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* readparamfile */

static PyObject * PyMSK_readsolution(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  char * str__arg2_filename = NULL;
  if (! PyArg_ParseTuple(_args,"is",&_arg1_whichsol,&str__arg2_filename)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_readsolution(self->ptr,_arg1_whichsol,str__arg2_filename);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* readsolution */

static PyObject * PyMSK_readsummary(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKstreamtypee _arg1_whichstream;
  if (! PyArg_ParseTuple(_args,"i",&_arg1_whichstream)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_readsummary(self->ptr,_arg1_whichstream);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* readsummary */

static PyObject * PyMSK_resizetask(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_maxnumcon;
  int _arg2_maxnumvar;
  int _arg3_maxnumcone;
  long long _arg4_maxnumanz;
  long long _arg5_maxnumqnz;
  if (! PyArg_ParseTuple(_args,"iiiLL",&_arg1_maxnumcon,&_arg2_maxnumvar,&_arg3_maxnumcone,&_arg4_maxnumanz,&_arg5_maxnumqnz)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_resizetask(self->ptr,_arg1_maxnumcon,_arg2_maxnumvar,_arg3_maxnumcone,_arg4_maxnumanz,_arg5_maxnumqnz);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* resizetask */

static PyObject * PyMSK_checkmemtask(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_file = NULL;
  int _arg2_line;
  if (! PyArg_ParseTuple(_args,"si",&str__arg1_file,&_arg2_line)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_checkmemtask(self->ptr,str__arg1_file,_arg2_line);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* checkmemtask */

static PyObject * PyMSK_getmemusagetask(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  long long _arg1_meminuse; //  t = <aisread.aisTypeRef object at 0x7fb398314160>
  long long _arg2_maxmemuse; //  t = <aisread.aisTypeRef object at 0x7fb398314278>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getmemusagetask(self->ptr,&_arg1_meminuse,&_arg2_maxmemuse);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(i(LL))",0,_arg1_meminuse,_arg2_maxmemuse);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getmemusagetask */

static PyObject * PyMSK_setdefaults(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_setdefaults(self->ptr);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* setdefaults */

static PyObject * PyMSK_solutiondef(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  int _arg2_isdef; //  t = <aisread.aisTypeRef object at 0x7fb3983183c8>
  if (! PyArg_ParseTuple(_args,"i",&_arg1_whichsol)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_solutiondef(self->ptr,_arg1_whichsol,&_arg2_isdef);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iO)",0,_arg2_isdef ? Py_True : Py_False);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* solutiondef */

static PyObject * PyMSK_deletesolution(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  if (! PyArg_ParseTuple(_args,"i",&_arg1_whichsol)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_deletesolution(self->ptr,_arg1_whichsol);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* deletesolution */

static PyObject * PyMSK_onesolutionsummary(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKstreamtypee _arg1_whichstream;
  MSKsoltypee _arg2_whichsol;
  if (! PyArg_ParseTuple(_args,"ii",&_arg1_whichstream,&_arg2_whichsol)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_onesolutionsummary(self->ptr,_arg1_whichstream,_arg2_whichsol);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* onesolutionsummary */

static PyObject * PyMSK_solutionsummary(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKstreamtypee _arg1_whichstream;
  if (! PyArg_ParseTuple(_args,"i",&_arg1_whichstream)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_solutionsummary(self->ptr,_arg1_whichstream);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* solutionsummary */

static PyObject * PyMSK_updatesolutioninfo(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  if (! PyArg_ParseTuple(_args,"i",&_arg1_whichsol)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_updatesolutioninfo(self->ptr,_arg1_whichsol);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* updatesolutioninfo */

static PyObject * PyMSK_optimizersummary(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKstreamtypee _arg1_whichstream;
  if (! PyArg_ParseTuple(_args,"i",&_arg1_whichstream)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_optimizersummary(self->ptr,_arg1_whichstream);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* optimizersummary */

static PyObject * PyMSK_strtoconetype(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_str = NULL;
  MSKconetypee _arg2_conetype; //  t = <aisread.aisTypeRef object at 0x7fb398323940>
  if (! PyArg_ParseTuple(_args,"s",&str__arg1_str)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_strtoconetype(self->ptr,str__arg1_str,&_arg2_conetype);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_conetype);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* strtoconetype */

static PyObject * PyMSK_strtosk(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_str = NULL;
  MSKstakeye _arg2_sk; //  t = <aisread.aisTypeRef object at 0x7fb398323d68>
  if (! PyArg_ParseTuple(_args,"s",&str__arg1_str)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_strtosk(self->ptr,str__arg1_str,&_arg2_sk);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg2_sk);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* strtosk */

static PyObject * PyMSK_writedata(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_filename = NULL;
  if (! PyArg_ParseTuple(_args,"s",&str__arg1_filename)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_writedata(self->ptr,str__arg1_filename);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* writedata */

static PyObject * PyMSK_writetask(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_filename = NULL;
  if (! PyArg_ParseTuple(_args,"s",&str__arg1_filename)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_writetask(self->ptr,str__arg1_filename);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* writetask */

static PyObject * PyMSK_readtask(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_filename = NULL;
  if (! PyArg_ParseTuple(_args,"s",&str__arg1_filename)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_readtask(self->ptr,str__arg1_filename);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* readtask */

static PyObject * PyMSK_readopfstring(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_data = NULL;
  if (! PyArg_ParseTuple(_args,"s",&str__arg1_data)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_readopfstring(self->ptr,str__arg1_data);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* readopfstring */

static PyObject * PyMSK_readlpstring(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_data = NULL;
  if (! PyArg_ParseTuple(_args,"s",&str__arg1_data)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_readlpstring(self->ptr,str__arg1_data);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* readlpstring */

static PyObject * PyMSK_readjsonstring(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_data = NULL;
  if (! PyArg_ParseTuple(_args,"s",&str__arg1_data)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_readjsonstring(self->ptr,str__arg1_data);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* readjsonstring */

static PyObject * PyMSK_readptfstring(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_data = NULL;
  if (! PyArg_ParseTuple(_args,"s",&str__arg1_data)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_readptfstring(self->ptr,str__arg1_data);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* readptfstring */

static PyObject * PyMSK_writeparamfile(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_filename = NULL;
  if (! PyArg_ParseTuple(_args,"s",&str__arg1_filename)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_writeparamfile(self->ptr,str__arg1_filename);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* writeparamfile */

static PyObject * PyMSK_getinfeasiblesubproblem(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  MSKtask_t _arg2_inftask; //  t = <aisread.aisTypeRef object at 0x7fb398332208>
  PyObject * _arg2_inftask_obj = NULL;
  if (! PyArg_ParseTuple(_args,"i",&_arg1_whichsol)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getinfeasiblesubproblem(self->ptr,_arg1_whichsol,&_arg2_inftask);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  {
    mosek_TaskObject * _t = (mosek_TaskObject *)Task_new(&mosek_TaskType, NULL, NULL);
    if (_t) {
      _t->ptr = _arg2_inftask;
      MSK_linkfunctotaskstream(_t->ptr,MSK_STREAM_LOG,_t,(MSKstreamfunc)log_task_stream_func);
      MSK_linkfunctotaskstream(_t->ptr,MSK_STREAM_MSG,_t,(MSKstreamfunc)msg_task_stream_func);
      MSK_linkfunctotaskstream(_t->ptr,MSK_STREAM_WRN,_t,(MSKstreamfunc)wrn_task_stream_func);
      MSK_linkfunctotaskstream(_t->ptr,MSK_STREAM_ERR,_t,(MSKstreamfunc)err_task_stream_func);
      MSK_putcallbackfunc(_t->ptr,(MSKcallbackfunc)callback_func,_t);
      _t->callback_func = NULL;
      _t->infocallback_func = NULL;
      _t->stream_func[0] = NULL;
      _t->stream_func[1] = NULL;
      _t->stream_func[2] = NULL;
      _t->stream_func[3] = NULL;
    }
    _arg2_inftask_obj = (PyObject*)_t;
  }
  return Py_BuildValue("(iO)",0,_arg2_inftask_obj);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getinfeasiblesubproblem */

static PyObject * PyMSK_writesolution(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKsoltypee _arg1_whichsol;
  char * str__arg2_filename = NULL;
  if (! PyArg_ParseTuple(_args,"is",&_arg1_whichsol,&str__arg2_filename)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_writesolution(self->ptr,_arg1_whichsol,str__arg2_filename);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* writesolution */

static PyObject * PyMSK_writejsonsol(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_filename = NULL;
  if (! PyArg_ParseTuple(_args,"s",&str__arg1_filename)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_writejsonsol(self->ptr,str__arg1_filename);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* writejsonsol */

static PyObject * PyMSK_primalsensitivity(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_numi;
  PyObject * memview__arg2_subi = NULL;
  Py_buffer * buf__arg2_subi;
  int * _arg2_subi = NULL;
  PyObject * memview__arg3_marki = NULL;
  Py_buffer * buf__arg3_marki;
  MSKmarke * _arg3_marki = NULL;
  int _arg4_numj;
  PyObject * memview__arg5_subj = NULL;
  Py_buffer * buf__arg5_subj;
  int * _arg5_subj = NULL;
  PyObject * memview__arg6_markj = NULL;
  Py_buffer * buf__arg6_markj;
  MSKmarke * _arg6_markj = NULL;
  PyObject * memview__arg7_leftpricei = NULL;
  Py_buffer * buf__arg7_leftpricei;
  double * _arg7_leftpricei = NULL;
  PyObject * memview__arg8_rightpricei = NULL;
  Py_buffer * buf__arg8_rightpricei;
  double * _arg8_rightpricei = NULL;
  PyObject * memview__arg9_leftrangei = NULL;
  Py_buffer * buf__arg9_leftrangei;
  double * _arg9_leftrangei = NULL;
  PyObject * memview__arg10_rightrangei = NULL;
  Py_buffer * buf__arg10_rightrangei;
  double * _arg10_rightrangei = NULL;
  PyObject * memview__arg11_leftpricej = NULL;
  Py_buffer * buf__arg11_leftpricej;
  double * _arg11_leftpricej = NULL;
  PyObject * memview__arg12_rightpricej = NULL;
  Py_buffer * buf__arg12_rightpricej;
  double * _arg12_rightpricej = NULL;
  PyObject * memview__arg13_leftrangej = NULL;
  Py_buffer * buf__arg13_leftrangej;
  double * _arg13_leftrangej = NULL;
  PyObject * memview__arg14_rightrangej = NULL;
  Py_buffer * buf__arg14_rightrangej;
  double * _arg14_rightrangej = NULL;
  if (! PyArg_ParseTuple(_args,"iOOiOOOOOOOOOO",&_arg1_numi,&memview__arg2_subi,&memview__arg3_marki,&_arg4_numj,&memview__arg5_subj,&memview__arg6_markj,&memview__arg7_leftpricei,&memview__arg8_rightpricei,&memview__arg9_leftrangei,&memview__arg10_rightrangei,&memview__arg11_leftpricej,&memview__arg12_rightpricej,&memview__arg13_leftrangej,&memview__arg14_rightrangej)) goto ERROR;
  if (Py_None != memview__arg2_subi)
  {
    if (! PyMemoryView_Check(memview__arg2_subi))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_subi");
      goto ERROR;
    }
    buf__arg2_subi = PyMemoryView_GET_BUFFER(memview__arg2_subi);
    if (buf__arg2_subi->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_subi");
        goto ERROR;
      
    }
    _arg2_subi = (int*) buf__arg2_subi->buf;
  }
  if (Py_None != memview__arg3_marki)
  {
    if (! PyMemoryView_Check(memview__arg3_marki))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_marki");
      goto ERROR;
    }
    buf__arg3_marki = PyMemoryView_GET_BUFFER(memview__arg3_marki);
    if (buf__arg3_marki->strides[0] != sizeof(MSKmarke))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_marki");
        goto ERROR;
      
    }
    _arg3_marki = (MSKmarke*) buf__arg3_marki->buf;
  }
  if (Py_None != memview__arg5_subj)
  {
    if (! PyMemoryView_Check(memview__arg5_subj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_subj");
      goto ERROR;
    }
    buf__arg5_subj = PyMemoryView_GET_BUFFER(memview__arg5_subj);
    if (buf__arg5_subj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_subj");
        goto ERROR;
      
    }
    _arg5_subj = (int*) buf__arg5_subj->buf;
  }
  if (Py_None != memview__arg6_markj)
  {
    if (! PyMemoryView_Check(memview__arg6_markj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_markj");
      goto ERROR;
    }
    buf__arg6_markj = PyMemoryView_GET_BUFFER(memview__arg6_markj);
    if (buf__arg6_markj->strides[0] != sizeof(MSKmarke))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_markj");
        goto ERROR;
      
    }
    _arg6_markj = (MSKmarke*) buf__arg6_markj->buf;
  }
  if (Py_None != memview__arg7_leftpricei)
  {
    if (! PyMemoryView_Check(memview__arg7_leftpricei))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg7_leftpricei");
      goto ERROR;
    }
    buf__arg7_leftpricei = PyMemoryView_GET_BUFFER(memview__arg7_leftpricei);
    if (buf__arg7_leftpricei->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg7_leftpricei");
        goto ERROR;
      
    }
    _arg7_leftpricei = (double*) buf__arg7_leftpricei->buf;
    if (buf__arg7_leftpricei->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg7_leftpricei");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg8_rightpricei)
  {
    if (! PyMemoryView_Check(memview__arg8_rightpricei))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg8_rightpricei");
      goto ERROR;
    }
    buf__arg8_rightpricei = PyMemoryView_GET_BUFFER(memview__arg8_rightpricei);
    if (buf__arg8_rightpricei->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg8_rightpricei");
        goto ERROR;
      
    }
    _arg8_rightpricei = (double*) buf__arg8_rightpricei->buf;
    if (buf__arg8_rightpricei->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg8_rightpricei");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg9_leftrangei)
  {
    if (! PyMemoryView_Check(memview__arg9_leftrangei))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg9_leftrangei");
      goto ERROR;
    }
    buf__arg9_leftrangei = PyMemoryView_GET_BUFFER(memview__arg9_leftrangei);
    if (buf__arg9_leftrangei->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg9_leftrangei");
        goto ERROR;
      
    }
    _arg9_leftrangei = (double*) buf__arg9_leftrangei->buf;
    if (buf__arg9_leftrangei->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg9_leftrangei");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg10_rightrangei)
  {
    if (! PyMemoryView_Check(memview__arg10_rightrangei))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg10_rightrangei");
      goto ERROR;
    }
    buf__arg10_rightrangei = PyMemoryView_GET_BUFFER(memview__arg10_rightrangei);
    if (buf__arg10_rightrangei->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg10_rightrangei");
        goto ERROR;
      
    }
    _arg10_rightrangei = (double*) buf__arg10_rightrangei->buf;
    if (buf__arg10_rightrangei->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg10_rightrangei");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg11_leftpricej)
  {
    if (! PyMemoryView_Check(memview__arg11_leftpricej))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg11_leftpricej");
      goto ERROR;
    }
    buf__arg11_leftpricej = PyMemoryView_GET_BUFFER(memview__arg11_leftpricej);
    if (buf__arg11_leftpricej->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg11_leftpricej");
        goto ERROR;
      
    }
    _arg11_leftpricej = (double*) buf__arg11_leftpricej->buf;
    if (buf__arg11_leftpricej->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg11_leftpricej");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg12_rightpricej)
  {
    if (! PyMemoryView_Check(memview__arg12_rightpricej))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg12_rightpricej");
      goto ERROR;
    }
    buf__arg12_rightpricej = PyMemoryView_GET_BUFFER(memview__arg12_rightpricej);
    if (buf__arg12_rightpricej->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg12_rightpricej");
        goto ERROR;
      
    }
    _arg12_rightpricej = (double*) buf__arg12_rightpricej->buf;
    if (buf__arg12_rightpricej->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg12_rightpricej");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg13_leftrangej)
  {
    if (! PyMemoryView_Check(memview__arg13_leftrangej))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg13_leftrangej");
      goto ERROR;
    }
    buf__arg13_leftrangej = PyMemoryView_GET_BUFFER(memview__arg13_leftrangej);
    if (buf__arg13_leftrangej->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg13_leftrangej");
        goto ERROR;
      
    }
    _arg13_leftrangej = (double*) buf__arg13_leftrangej->buf;
    if (buf__arg13_leftrangej->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg13_leftrangej");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg14_rightrangej)
  {
    if (! PyMemoryView_Check(memview__arg14_rightrangej))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg14_rightrangej");
      goto ERROR;
    }
    buf__arg14_rightrangej = PyMemoryView_GET_BUFFER(memview__arg14_rightrangej);
    if (buf__arg14_rightrangej->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg14_rightrangej");
        goto ERROR;
      
    }
    _arg14_rightrangej = (double*) buf__arg14_rightrangej->buf;
    if (buf__arg14_rightrangej->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg14_rightrangej");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_primalsensitivity(self->ptr,_arg1_numi,_arg2_subi,_arg3_marki,_arg4_numj,_arg5_subj,_arg6_markj,_arg7_leftpricei,_arg8_rightpricei,_arg9_leftrangei,_arg10_rightrangei,_arg11_leftpricej,_arg12_rightpricej,_arg13_leftrangej,_arg14_rightrangej);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* primalsensitivity */

static PyObject * PyMSK_sensitivityreport(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKstreamtypee _arg1_whichstream;
  if (! PyArg_ParseTuple(_args,"i",&_arg1_whichstream)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_sensitivityreport(self->ptr,_arg1_whichstream);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* sensitivityreport */

static PyObject * PyMSK_dualsensitivity(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_numj;
  PyObject * memview__arg2_subj = NULL;
  Py_buffer * buf__arg2_subj;
  int * _arg2_subj = NULL;
  PyObject * memview__arg3_leftpricej = NULL;
  Py_buffer * buf__arg3_leftpricej;
  double * _arg3_leftpricej = NULL;
  PyObject * memview__arg4_rightpricej = NULL;
  Py_buffer * buf__arg4_rightpricej;
  double * _arg4_rightpricej = NULL;
  PyObject * memview__arg5_leftrangej = NULL;
  Py_buffer * buf__arg5_leftrangej;
  double * _arg5_leftrangej = NULL;
  PyObject * memview__arg6_rightrangej = NULL;
  Py_buffer * buf__arg6_rightrangej;
  double * _arg6_rightrangej = NULL;
  if (! PyArg_ParseTuple(_args,"iOOOOO",&_arg1_numj,&memview__arg2_subj,&memview__arg3_leftpricej,&memview__arg4_rightpricej,&memview__arg5_leftrangej,&memview__arg6_rightrangej)) goto ERROR;
  if (Py_None != memview__arg2_subj)
  {
    if (! PyMemoryView_Check(memview__arg2_subj))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_subj");
      goto ERROR;
    }
    buf__arg2_subj = PyMemoryView_GET_BUFFER(memview__arg2_subj);
    if (buf__arg2_subj->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_subj");
        goto ERROR;
      
    }
    _arg2_subj = (int*) buf__arg2_subj->buf;
  }
  if (Py_None != memview__arg3_leftpricej)
  {
    if (! PyMemoryView_Check(memview__arg3_leftpricej))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_leftpricej");
      goto ERROR;
    }
    buf__arg3_leftpricej = PyMemoryView_GET_BUFFER(memview__arg3_leftpricej);
    if (buf__arg3_leftpricej->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_leftpricej");
        goto ERROR;
      
    }
    _arg3_leftpricej = (double*) buf__arg3_leftpricej->buf;
    if (buf__arg3_leftpricej->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg3_leftpricej");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg4_rightpricej)
  {
    if (! PyMemoryView_Check(memview__arg4_rightpricej))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_rightpricej");
      goto ERROR;
    }
    buf__arg4_rightpricej = PyMemoryView_GET_BUFFER(memview__arg4_rightpricej);
    if (buf__arg4_rightpricej->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_rightpricej");
        goto ERROR;
      
    }
    _arg4_rightpricej = (double*) buf__arg4_rightpricej->buf;
    if (buf__arg4_rightpricej->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_rightpricej");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg5_leftrangej)
  {
    if (! PyMemoryView_Check(memview__arg5_leftrangej))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_leftrangej");
      goto ERROR;
    }
    buf__arg5_leftrangej = PyMemoryView_GET_BUFFER(memview__arg5_leftrangej);
    if (buf__arg5_leftrangej->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_leftrangej");
        goto ERROR;
      
    }
    _arg5_leftrangej = (double*) buf__arg5_leftrangej->buf;
    if (buf__arg5_leftrangej->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg5_leftrangej");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg6_rightrangej)
  {
    if (! PyMemoryView_Check(memview__arg6_rightrangej))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_rightrangej");
      goto ERROR;
    }
    buf__arg6_rightrangej = PyMemoryView_GET_BUFFER(memview__arg6_rightrangej);
    if (buf__arg6_rightrangej->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_rightrangej");
        goto ERROR;
      
    }
    _arg6_rightrangej = (double*) buf__arg6_rightrangej->buf;
    if (buf__arg6_rightrangej->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg6_rightrangej");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_dualsensitivity(self->ptr,_arg1_numj,_arg2_subj,_arg3_leftpricej,_arg4_rightpricej,_arg5_leftrangej,_arg6_rightrangej);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* dualsensitivity */

static PyObject * PyMSK_optimizermt(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_server = NULL;
  char * str__arg2_port = NULL;
  MSKrescodee _arg3_trmcode; //  t = <aisread.aisTypeRef object at 0x7fb3982cb240>
  if (! PyArg_ParseTuple(_args,"ss",&str__arg1_server,&str__arg2_port)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_optimizermt(self->ptr,str__arg1_server,str__arg2_port,&_arg3_trmcode);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(ii)",0,_arg3_trmcode);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* optimizermt */

static PyObject * PyMSK_asyncoptimize(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_server = NULL;
  char * str__arg2_port = NULL;
  PyObject * obj__arg3_token = NULL;
  Py_buffer * buf__arg3_token = NULL;
  if (! PyArg_ParseTuple(_args,"ssO",&str__arg1_server,&str__arg2_port,&obj__arg3_token)) goto ERROR;
  if (! PyMemoryView_Check(obj__arg3_token))
  {
    PyErr_SetString(PyExc_TypeError,"Expected a memoryview object");
    goto ERROR;
  }
  buf__arg3_token = PyMemoryView_GET_BUFFER(obj__arg3_token);
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_asyncoptimize(self->ptr,str__arg1_server,str__arg2_port,(char*)buf__arg3_token->buf);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(iO)",0,Py_None);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* asyncoptimize */

static PyObject * PyMSK_asyncstop(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_server = NULL;
  char * str__arg2_port = NULL;
  char * str__arg3_token = NULL;
  if (! PyArg_ParseTuple(_args,"sss",&str__arg1_server,&str__arg2_port,&str__arg3_token)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_asyncstop(self->ptr,str__arg1_server,str__arg2_port,str__arg3_token);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* asyncstop */

static PyObject * PyMSK_asyncpoll(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_server = NULL;
  char * str__arg2_port = NULL;
  char * str__arg3_token = NULL;
  int _arg4_respavailable; //  t = <aisread.aisTypeRef object at 0x7fb3982ea400>
  MSKrescodee _arg5_resp; //  t = <aisread.aisTypeRef object at 0x7fb3982ea4e0>
  MSKrescodee _arg6_trm; //  t = <aisread.aisTypeRef object at 0x7fb3982ea5f8>
  if (! PyArg_ParseTuple(_args,"sss",&str__arg1_server,&str__arg2_port,&str__arg3_token)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_asyncpoll(self->ptr,str__arg1_server,str__arg2_port,str__arg3_token,&_arg4_respavailable,&_arg5_resp,&_arg6_trm);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(i(Oii))",0,_arg4_respavailable ? Py_True : Py_False,_arg5_resp,_arg6_trm);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* asyncpoll */

static PyObject * PyMSK_asyncgetresult(mosek_TaskObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_server = NULL;
  char * str__arg2_port = NULL;
  char * str__arg3_token = NULL;
  int _arg4_respavailable; //  t = <aisread.aisTypeRef object at 0x7fb3982eadd8>
  MSKrescodee _arg5_resp; //  t = <aisread.aisTypeRef object at 0x7fb3982eaeb8>
  MSKrescodee _arg6_trm; //  t = <aisread.aisTypeRef object at 0x7fb3982eafd0>
  if (! PyArg_ParseTuple(_args,"sss",&str__arg1_server,&str__arg2_port,&str__arg3_token)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_asyncgetresult(self->ptr,str__arg1_server,str__arg2_port,str__arg3_token,&_arg4_respavailable,&_arg5_resp,&_arg6_trm);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(i(Oii))",0,_arg4_respavailable ? Py_True : Py_False,_arg5_resp,_arg6_trm);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* asyncgetresult */

static PyObject * PyMSK_checkoutlicense(mosek_EnvObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKfeaturee _arg1_feature;
  if (! PyArg_ParseTuple(_args,"i",&_arg1_feature)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_checkoutlicense(self->ptr,_arg1_feature);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* checkoutlicense */

static PyObject * PyMSK_checkinlicense(mosek_EnvObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKfeaturee _arg1_feature;
  if (! PyArg_ParseTuple(_args,"i",&_arg1_feature)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_checkinlicense(self->ptr,_arg1_feature);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* checkinlicense */

static PyObject * PyMSK_checkinall(mosek_EnvObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_checkinall(self->ptr);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* checkinall */

static PyObject * PyMSK_setupthreads(mosek_EnvObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_numthreads;
  if (! PyArg_ParseTuple(_args,"i",&_arg1_numthreads)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_setupthreads(self->ptr,_arg1_numthreads);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* setupthreads */

static PyObject * PyMSK_echointro(mosek_EnvObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_longver;
  if (! PyArg_ParseTuple(_args,"i",&_arg1_longver)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_echointro(self->ptr,_arg1_longver);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* echointro */

static PyObject * PyMSK_getcodedesc(PyObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKrescodee _arg0_code;
  PyObject * obj__arg1_symname = NULL;
  Py_buffer * buf__arg1_symname = NULL;
  PyObject * obj__arg2_str = NULL;
  Py_buffer * buf__arg2_str = NULL;
  if (! PyArg_ParseTuple(_args,"iOO",&_arg0_code,&obj__arg1_symname,&obj__arg2_str)) goto ERROR;
  if (! PyMemoryView_Check(obj__arg1_symname))
  {
    PyErr_SetString(PyExc_TypeError,"Expected a memoryview object");
    goto ERROR;
  }
  buf__arg1_symname = PyMemoryView_GET_BUFFER(obj__arg1_symname);
  if (! PyMemoryView_Check(obj__arg2_str))
  {
    PyErr_SetString(PyExc_TypeError,"Expected a memoryview object");
    goto ERROR;
  }
  buf__arg2_str = PyMemoryView_GET_BUFFER(obj__arg2_str);
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getcodedesc(_arg0_code,(char*)buf__arg1_symname->buf,(char*)buf__arg2_str->buf);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(i(OO))",0,Py_None,Py_None);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getcodedesc */

static PyObject * PyMSK_getversion(PyObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg0_major; //  t = <aisread.aisTypeRef object at 0x7fb398287748>
  int _arg1_minor; //  t = <aisread.aisTypeRef object at 0x7fb398287860>
  int _arg2_revision; //  t = <aisread.aisTypeRef object at 0x7fb398287978>
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_getversion(&_arg0_major,&_arg1_minor,&_arg2_revision);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(i(iii))",0,_arg0_major,_arg1_minor,_arg2_revision);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* getversion */

static PyObject * PyMSK_linkfiletoenvstream(mosek_EnvObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKstreamtypee _arg1_whichstream;
  char * str__arg2_filename = NULL;
  int _arg3_append;
  if (! PyArg_ParseTuple(_args,"isi",&_arg1_whichstream,&str__arg2_filename,&_arg3_append)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_linkfiletoenvstream(self->ptr,_arg1_whichstream,str__arg2_filename,_arg3_append);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* linkfiletoenvstream */

static PyObject * PyMSK_putlicensedebug(mosek_EnvObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_licdebug;
  if (! PyArg_ParseTuple(_args,"i",&_arg1_licdebug)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putlicensedebug(self->ptr,_arg1_licdebug);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putlicensedebug */

static PyObject * PyMSK_putlicensecode(mosek_EnvObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  PyObject * memview__arg1_code = NULL;
  Py_buffer * buf__arg1_code;
  int * _arg1_code = NULL;
  if (! PyArg_ParseTuple(_args,"O",&memview__arg1_code)) goto ERROR;
  if (Py_None != memview__arg1_code)
  {
    if (! PyMemoryView_Check(memview__arg1_code))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg1_code");
      goto ERROR;
    }
    buf__arg1_code = PyMemoryView_GET_BUFFER(memview__arg1_code);
    if (buf__arg1_code->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg1_code");
        goto ERROR;
      
    }
    _arg1_code = (int*) buf__arg1_code->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putlicensecode(self->ptr,_arg1_code);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putlicensecode */

static PyObject * PyMSK_putlicensewait(mosek_EnvObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_licwait;
  if (! PyArg_ParseTuple(_args,"i",&_arg1_licwait)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putlicensewait(self->ptr,_arg1_licwait);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putlicensewait */

static PyObject * PyMSK_putlicensepath(mosek_EnvObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  char * str__arg1_licensepath = NULL;
  if (! PyArg_ParseTuple(_args,"s",&str__arg1_licensepath)) goto ERROR;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_putlicensepath(self->ptr,str__arg1_licensepath);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* putlicensepath */

static PyObject * PyMSK_axpy(mosek_EnvObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_n;
  double _arg2_alpha;
  PyObject * memview__arg3_x = NULL;
  Py_buffer * buf__arg3_x;
  double * _arg3_x = NULL;
  PyObject * memview__arg4_y = NULL;
  Py_buffer * buf__arg4_y;
  double * _arg4_y = NULL;
  if (! PyArg_ParseTuple(_args,"idOO",&_arg1_n,&_arg2_alpha,&memview__arg3_x,&memview__arg4_y)) goto ERROR;
  if (Py_None != memview__arg3_x)
  {
    if (! PyMemoryView_Check(memview__arg3_x))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_x");
      goto ERROR;
    }
    buf__arg3_x = PyMemoryView_GET_BUFFER(memview__arg3_x);
    if (buf__arg3_x->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_x");
        goto ERROR;
      
    }
    _arg3_x = (double*) buf__arg3_x->buf;
  }
  if (Py_None != memview__arg4_y)
  {
    if (! PyMemoryView_Check(memview__arg4_y))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_y");
      goto ERROR;
    }
    buf__arg4_y = PyMemoryView_GET_BUFFER(memview__arg4_y);
    if (buf__arg4_y->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_y");
        goto ERROR;
      
    }
    _arg4_y = (double*) buf__arg4_y->buf;
    if (buf__arg4_y->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_y");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_axpy(self->ptr,_arg1_n,_arg2_alpha,_arg3_x,_arg4_y);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* axpy */

static PyObject * PyMSK_dot(mosek_EnvObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_n;
  PyObject * memview__arg2_x = NULL;
  Py_buffer * buf__arg2_x;
  double * _arg2_x = NULL;
  PyObject * memview__arg3_y = NULL;
  Py_buffer * buf__arg3_y;
  double * _arg3_y = NULL;
  double _arg4_xty; //  t = <aisread.aisTypeRef object at 0x7fb3982ab4e0>
  if (! PyArg_ParseTuple(_args,"iOO",&_arg1_n,&memview__arg2_x,&memview__arg3_y)) goto ERROR;
  if (Py_None != memview__arg2_x)
  {
    if (! PyMemoryView_Check(memview__arg2_x))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg2_x");
      goto ERROR;
    }
    buf__arg2_x = PyMemoryView_GET_BUFFER(memview__arg2_x);
    if (buf__arg2_x->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg2_x");
        goto ERROR;
      
    }
    _arg2_x = (double*) buf__arg2_x->buf;
  }
  if (Py_None != memview__arg3_y)
  {
    if (! PyMemoryView_Check(memview__arg3_y))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_y");
      goto ERROR;
    }
    buf__arg3_y = PyMemoryView_GET_BUFFER(memview__arg3_y);
    if (buf__arg3_y->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_y");
        goto ERROR;
      
    }
    _arg3_y = (double*) buf__arg3_y->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_dot(self->ptr,_arg1_n,_arg2_x,_arg3_y,&_arg4_xty);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("(id)",0,_arg4_xty);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* dot */

static PyObject * PyMSK_gemv(mosek_EnvObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKtransposee _arg1_transa;
  int _arg2_m;
  int _arg3_n;
  double _arg4_alpha;
  PyObject * memview__arg5_a = NULL;
  Py_buffer * buf__arg5_a;
  double * _arg5_a = NULL;
  PyObject * memview__arg6_x = NULL;
  Py_buffer * buf__arg6_x;
  double * _arg6_x = NULL;
  double _arg7_beta;
  PyObject * memview__arg8_y = NULL;
  Py_buffer * buf__arg8_y;
  double * _arg8_y = NULL;
  if (! PyArg_ParseTuple(_args,"iiidOOdO",&_arg1_transa,&_arg2_m,&_arg3_n,&_arg4_alpha,&memview__arg5_a,&memview__arg6_x,&_arg7_beta,&memview__arg8_y)) goto ERROR;
  if (Py_None != memview__arg5_a)
  {
    if (! PyMemoryView_Check(memview__arg5_a))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_a");
      goto ERROR;
    }
    buf__arg5_a = PyMemoryView_GET_BUFFER(memview__arg5_a);
    if (buf__arg5_a->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_a");
        goto ERROR;
      
    }
    _arg5_a = (double*) buf__arg5_a->buf;
  }
  if (Py_None != memview__arg6_x)
  {
    if (! PyMemoryView_Check(memview__arg6_x))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_x");
      goto ERROR;
    }
    buf__arg6_x = PyMemoryView_GET_BUFFER(memview__arg6_x);
    if (buf__arg6_x->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_x");
        goto ERROR;
      
    }
    _arg6_x = (double*) buf__arg6_x->buf;
  }
  if (Py_None != memview__arg8_y)
  {
    if (! PyMemoryView_Check(memview__arg8_y))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg8_y");
      goto ERROR;
    }
    buf__arg8_y = PyMemoryView_GET_BUFFER(memview__arg8_y);
    if (buf__arg8_y->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg8_y");
        goto ERROR;
      
    }
    _arg8_y = (double*) buf__arg8_y->buf;
    if (buf__arg8_y->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg8_y");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_gemv(self->ptr,_arg1_transa,_arg2_m,_arg3_n,_arg4_alpha,_arg5_a,_arg6_x,_arg7_beta,_arg8_y);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* gemv */

static PyObject * PyMSK_gemm(mosek_EnvObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKtransposee _arg1_transa;
  MSKtransposee _arg2_transb;
  int _arg3_m;
  int _arg4_n;
  int _arg5_k;
  double _arg6_alpha;
  PyObject * memview__arg7_a = NULL;
  Py_buffer * buf__arg7_a;
  double * _arg7_a = NULL;
  PyObject * memview__arg8_b = NULL;
  Py_buffer * buf__arg8_b;
  double * _arg8_b = NULL;
  double _arg9_beta;
  PyObject * memview__arg10_c = NULL;
  Py_buffer * buf__arg10_c;
  double * _arg10_c = NULL;
  if (! PyArg_ParseTuple(_args,"iiiiidOOdO",&_arg1_transa,&_arg2_transb,&_arg3_m,&_arg4_n,&_arg5_k,&_arg6_alpha,&memview__arg7_a,&memview__arg8_b,&_arg9_beta,&memview__arg10_c)) goto ERROR;
  if (Py_None != memview__arg7_a)
  {
    if (! PyMemoryView_Check(memview__arg7_a))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg7_a");
      goto ERROR;
    }
    buf__arg7_a = PyMemoryView_GET_BUFFER(memview__arg7_a);
    if (buf__arg7_a->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg7_a");
        goto ERROR;
      
    }
    _arg7_a = (double*) buf__arg7_a->buf;
  }
  if (Py_None != memview__arg8_b)
  {
    if (! PyMemoryView_Check(memview__arg8_b))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg8_b");
      goto ERROR;
    }
    buf__arg8_b = PyMemoryView_GET_BUFFER(memview__arg8_b);
    if (buf__arg8_b->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg8_b");
        goto ERROR;
      
    }
    _arg8_b = (double*) buf__arg8_b->buf;
  }
  if (Py_None != memview__arg10_c)
  {
    if (! PyMemoryView_Check(memview__arg10_c))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg10_c");
      goto ERROR;
    }
    buf__arg10_c = PyMemoryView_GET_BUFFER(memview__arg10_c);
    if (buf__arg10_c->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg10_c");
        goto ERROR;
      
    }
    _arg10_c = (double*) buf__arg10_c->buf;
    if (buf__arg10_c->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg10_c");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_gemm(self->ptr,_arg1_transa,_arg2_transb,_arg3_m,_arg4_n,_arg5_k,_arg6_alpha,_arg7_a,_arg8_b,_arg9_beta,_arg10_c);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* gemm */

static PyObject * PyMSK_syrk(mosek_EnvObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKuploe _arg1_uplo;
  MSKtransposee _arg2_trans;
  int _arg3_n;
  int _arg4_k;
  double _arg5_alpha;
  PyObject * memview__arg6_a = NULL;
  Py_buffer * buf__arg6_a;
  double * _arg6_a = NULL;
  double _arg7_beta;
  PyObject * memview__arg8_c = NULL;
  Py_buffer * buf__arg8_c;
  double * _arg8_c = NULL;
  if (! PyArg_ParseTuple(_args,"iiiidOdO",&_arg1_uplo,&_arg2_trans,&_arg3_n,&_arg4_k,&_arg5_alpha,&memview__arg6_a,&_arg7_beta,&memview__arg8_c)) goto ERROR;
  if (Py_None != memview__arg6_a)
  {
    if (! PyMemoryView_Check(memview__arg6_a))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_a");
      goto ERROR;
    }
    buf__arg6_a = PyMemoryView_GET_BUFFER(memview__arg6_a);
    if (buf__arg6_a->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_a");
        goto ERROR;
      
    }
    _arg6_a = (double*) buf__arg6_a->buf;
  }
  if (Py_None != memview__arg8_c)
  {
    if (! PyMemoryView_Check(memview__arg8_c))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg8_c");
      goto ERROR;
    }
    buf__arg8_c = PyMemoryView_GET_BUFFER(memview__arg8_c);
    if (buf__arg8_c->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg8_c");
        goto ERROR;
      
    }
    _arg8_c = (double*) buf__arg8_c->buf;
    if (buf__arg8_c->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg8_c");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_syrk(self->ptr,_arg1_uplo,_arg2_trans,_arg3_n,_arg4_k,_arg5_alpha,_arg6_a,_arg7_beta,_arg8_c);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* syrk */

static PyObject * PyMSK_computesparsecholesky(mosek_EnvObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  int _arg1_multithread;
  int _arg2_ordermethod;
  double _arg3_tolsingular;
  int _arg4_n;
  PyObject * memview__arg5_anzc = NULL;
  Py_buffer * buf__arg5_anzc;
  int * _arg5_anzc = NULL;
  PyObject * memview__arg6_aptrc = NULL;
  Py_buffer * buf__arg6_aptrc;
  long long * _arg6_aptrc = NULL;
  PyObject * memview__arg7_asubc = NULL;
  Py_buffer * buf__arg7_asubc;
  int * _arg7_asubc = NULL;
  PyObject * memview__arg8_avalc = NULL;
  Py_buffer * buf__arg8_avalc;
  double * _arg8_avalc = NULL;
  int * _arg9_perm = NULL;
  ptrdiff_t _i__arg9_perm;
  PyObject * __arg9_perm = NULL;
  double * _arg10_diag = NULL;
  ptrdiff_t _i__arg10_diag;
  PyObject * __arg10_diag = NULL;
  int * _arg11_lnzc = NULL;
  ptrdiff_t _i__arg11_lnzc;
  PyObject * __arg11_lnzc = NULL;
  long long * _arg12_lptrc = NULL;
  ptrdiff_t _i__arg12_lptrc;
  PyObject * __arg12_lptrc = NULL;
  long long _arg13_lensubnval; //  t = <aisread.aisTypeRef object at 0x7fb39823d9e8>
  int * _arg14_lsubc = NULL;
  ptrdiff_t _i__arg14_lsubc;
  PyObject * __arg14_lsubc = NULL;
  double * _arg15_lvalc = NULL;
  ptrdiff_t _i__arg15_lvalc;
  PyObject * __arg15_lvalc = NULL;
  if (! PyArg_ParseTuple(_args,"iidiOOOO",&_arg1_multithread,&_arg2_ordermethod,&_arg3_tolsingular,&_arg4_n,&memview__arg5_anzc,&memview__arg6_aptrc,&memview__arg7_asubc,&memview__arg8_avalc)) goto ERROR;
  if (Py_None != memview__arg5_anzc)
  {
    if (! PyMemoryView_Check(memview__arg5_anzc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg5_anzc");
      goto ERROR;
    }
    buf__arg5_anzc = PyMemoryView_GET_BUFFER(memview__arg5_anzc);
    if (buf__arg5_anzc->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg5_anzc");
        goto ERROR;
      
    }
    _arg5_anzc = (int*) buf__arg5_anzc->buf;
  }
  if (Py_None != memview__arg6_aptrc)
  {
    if (! PyMemoryView_Check(memview__arg6_aptrc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_aptrc");
      goto ERROR;
    }
    buf__arg6_aptrc = PyMemoryView_GET_BUFFER(memview__arg6_aptrc);
    if (buf__arg6_aptrc->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_aptrc");
        goto ERROR;
      
    }
    _arg6_aptrc = (long long*) buf__arg6_aptrc->buf;
  }
  if (Py_None != memview__arg7_asubc)
  {
    if (! PyMemoryView_Check(memview__arg7_asubc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg7_asubc");
      goto ERROR;
    }
    buf__arg7_asubc = PyMemoryView_GET_BUFFER(memview__arg7_asubc);
    if (buf__arg7_asubc->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg7_asubc");
        goto ERROR;
      
    }
    _arg7_asubc = (int*) buf__arg7_asubc->buf;
  }
  if (Py_None != memview__arg8_avalc)
  {
    if (! PyMemoryView_Check(memview__arg8_avalc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg8_avalc");
      goto ERROR;
    }
    buf__arg8_avalc = PyMemoryView_GET_BUFFER(memview__arg8_avalc);
    if (buf__arg8_avalc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg8_avalc");
        goto ERROR;
      
    }
    _arg8_avalc = (double*) buf__arg8_avalc->buf;
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_computesparsecholesky(self->ptr,_arg1_multithread,_arg2_ordermethod,_arg3_tolsingular,_arg4_n,_arg5_anzc,_arg6_aptrc,_arg7_asubc,_arg8_avalc,&_arg9_perm,&_arg10_diag,&_arg11_lnzc,&_arg12_lptrc,&_arg13_lensubnval,&_arg14_lsubc,&_arg15_lvalc);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  __arg9_perm = PyList_New(_arg4_n);
  for (_i__arg9_perm = 0; _i__arg9_perm < _arg4_n && !PyErr_Occurred(); ++_i__arg9_perm) {
    PyList_SET_ITEM(__arg9_perm,_i__arg9_perm,PyLong_FromLong(_arg9_perm[_i__arg9_perm])); /*3 !!!!*/
  }
  if (PyErr_Occurred()) goto ERROR;
  __arg10_diag = PyList_New(_arg4_n);
  for (_i__arg10_diag = 0; _i__arg10_diag < _arg4_n && !PyErr_Occurred(); ++_i__arg10_diag) {
    PyList_SET_ITEM(__arg10_diag,_i__arg10_diag,PyFloat_FromDouble(_arg10_diag[_i__arg10_diag])); /*3 !!!!*/
  }
  if (PyErr_Occurred()) goto ERROR;
  __arg11_lnzc = PyList_New(_arg4_n);
  for (_i__arg11_lnzc = 0; _i__arg11_lnzc < _arg4_n && !PyErr_Occurred(); ++_i__arg11_lnzc) {
    PyList_SET_ITEM(__arg11_lnzc,_i__arg11_lnzc,PyLong_FromLong(_arg11_lnzc[_i__arg11_lnzc])); /*3 !!!!*/
  }
  if (PyErr_Occurred()) goto ERROR;
  __arg12_lptrc = PyList_New(_arg4_n);
  for (_i__arg12_lptrc = 0; _i__arg12_lptrc < _arg4_n && !PyErr_Occurred(); ++_i__arg12_lptrc) {
    PyList_SET_ITEM(__arg12_lptrc,_i__arg12_lptrc,PyLong_FromLongLong(_arg12_lptrc[_i__arg12_lptrc])); /*3 !!!!*/
  }
  if (PyErr_Occurred()) goto ERROR;
  __arg14_lsubc = PyList_New(_arg13_lensubnval);
  for (_i__arg14_lsubc = 0; _i__arg14_lsubc < _arg13_lensubnval && !PyErr_Occurred(); ++_i__arg14_lsubc) {
    PyList_SET_ITEM(__arg14_lsubc,_i__arg14_lsubc,PyLong_FromLong(_arg14_lsubc[_i__arg14_lsubc])); /*3 !!!!*/
  }
  if (PyErr_Occurred()) goto ERROR;
  __arg15_lvalc = PyList_New(_arg13_lensubnval);
  for (_i__arg15_lvalc = 0; _i__arg15_lvalc < _arg13_lensubnval && !PyErr_Occurred(); ++_i__arg15_lvalc) {
    PyList_SET_ITEM(__arg15_lvalc,_i__arg15_lvalc,PyFloat_FromDouble(_arg15_lvalc[_i__arg15_lvalc])); /*3 !!!!*/
  }
  if (PyErr_Occurred()) goto ERROR;
  return Py_BuildValue("(i(OOOOLOO))",0,__arg9_perm,__arg10_diag,__arg11_lnzc,__arg12_lptrc,_arg13_lensubnval,__arg14_lsubc,__arg15_lvalc);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("(iO)",_r,Py_None);
} /* computesparsecholesky */

static PyObject * PyMSK_sparsetriangularsolvedense(mosek_EnvObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKtransposee _arg1_transposed;
  int _arg2_n;
  PyObject * memview__arg3_lnzc = NULL;
  Py_buffer * buf__arg3_lnzc;
  int * _arg3_lnzc = NULL;
  PyObject * memview__arg4_lptrc = NULL;
  Py_buffer * buf__arg4_lptrc;
  long long * _arg4_lptrc = NULL;
  long long _arg5_lensubnval;
  PyObject * memview__arg6_lsubc = NULL;
  Py_buffer * buf__arg6_lsubc;
  int * _arg6_lsubc = NULL;
  PyObject * memview__arg7_lvalc = NULL;
  Py_buffer * buf__arg7_lvalc;
  double * _arg7_lvalc = NULL;
  PyObject * memview__arg8_b = NULL;
  Py_buffer * buf__arg8_b;
  double * _arg8_b = NULL;
  if (! PyArg_ParseTuple(_args,"iiOOLOOO",&_arg1_transposed,&_arg2_n,&memview__arg3_lnzc,&memview__arg4_lptrc,&_arg5_lensubnval,&memview__arg6_lsubc,&memview__arg7_lvalc,&memview__arg8_b)) goto ERROR;
  if (Py_None != memview__arg3_lnzc)
  {
    if (! PyMemoryView_Check(memview__arg3_lnzc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_lnzc");
      goto ERROR;
    }
    buf__arg3_lnzc = PyMemoryView_GET_BUFFER(memview__arg3_lnzc);
    if (buf__arg3_lnzc->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_lnzc");
        goto ERROR;
      
    }
    _arg3_lnzc = (int*) buf__arg3_lnzc->buf;
  }
  if (Py_None != memview__arg4_lptrc)
  {
    if (! PyMemoryView_Check(memview__arg4_lptrc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_lptrc");
      goto ERROR;
    }
    buf__arg4_lptrc = PyMemoryView_GET_BUFFER(memview__arg4_lptrc);
    if (buf__arg4_lptrc->strides[0] != sizeof(long long))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_lptrc");
        goto ERROR;
      
    }
    _arg4_lptrc = (long long*) buf__arg4_lptrc->buf;
  }
  if (Py_None != memview__arg6_lsubc)
  {
    if (! PyMemoryView_Check(memview__arg6_lsubc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg6_lsubc");
      goto ERROR;
    }
    buf__arg6_lsubc = PyMemoryView_GET_BUFFER(memview__arg6_lsubc);
    if (buf__arg6_lsubc->strides[0] != sizeof(int))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg6_lsubc");
        goto ERROR;
      
    }
    _arg6_lsubc = (int*) buf__arg6_lsubc->buf;
  }
  if (Py_None != memview__arg7_lvalc)
  {
    if (! PyMemoryView_Check(memview__arg7_lvalc))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg7_lvalc");
      goto ERROR;
    }
    buf__arg7_lvalc = PyMemoryView_GET_BUFFER(memview__arg7_lvalc);
    if (buf__arg7_lvalc->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg7_lvalc");
        goto ERROR;
      
    }
    _arg7_lvalc = (double*) buf__arg7_lvalc->buf;
  }
  if (Py_None != memview__arg8_b)
  {
    if (! PyMemoryView_Check(memview__arg8_b))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg8_b");
      goto ERROR;
    }
    buf__arg8_b = PyMemoryView_GET_BUFFER(memview__arg8_b);
    if (buf__arg8_b->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg8_b");
        goto ERROR;
      
    }
    _arg8_b = (double*) buf__arg8_b->buf;
    if (buf__arg8_b->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg8_b");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_sparsetriangularsolvedense(self->ptr,_arg1_transposed,_arg2_n,_arg3_lnzc,_arg4_lptrc,_arg5_lensubnval,_arg6_lsubc,_arg7_lvalc,_arg8_b);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* sparsetriangularsolvedense */

static PyObject * PyMSK_potrf(mosek_EnvObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKuploe _arg1_uplo;
  int _arg2_n;
  PyObject * memview__arg3_a = NULL;
  Py_buffer * buf__arg3_a;
  double * _arg3_a = NULL;
  if (! PyArg_ParseTuple(_args,"iiO",&_arg1_uplo,&_arg2_n,&memview__arg3_a)) goto ERROR;
  if (Py_None != memview__arg3_a)
  {
    if (! PyMemoryView_Check(memview__arg3_a))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_a");
      goto ERROR;
    }
    buf__arg3_a = PyMemoryView_GET_BUFFER(memview__arg3_a);
    if (buf__arg3_a->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_a");
        goto ERROR;
      
    }
    _arg3_a = (double*) buf__arg3_a->buf;
    if (buf__arg3_a->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg3_a");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_potrf(self->ptr,_arg1_uplo,_arg2_n,_arg3_a);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* potrf */

static PyObject * PyMSK_syeig(mosek_EnvObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKuploe _arg1_uplo;
  int _arg2_n;
  PyObject * memview__arg3_a = NULL;
  Py_buffer * buf__arg3_a;
  double * _arg3_a = NULL;
  PyObject * memview__arg4_w = NULL;
  Py_buffer * buf__arg4_w;
  double * _arg4_w = NULL;
  if (! PyArg_ParseTuple(_args,"iiOO",&_arg1_uplo,&_arg2_n,&memview__arg3_a,&memview__arg4_w)) goto ERROR;
  if (Py_None != memview__arg3_a)
  {
    if (! PyMemoryView_Check(memview__arg3_a))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_a");
      goto ERROR;
    }
    buf__arg3_a = PyMemoryView_GET_BUFFER(memview__arg3_a);
    if (buf__arg3_a->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_a");
        goto ERROR;
      
    }
    _arg3_a = (double*) buf__arg3_a->buf;
  }
  if (Py_None != memview__arg4_w)
  {
    if (! PyMemoryView_Check(memview__arg4_w))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_w");
      goto ERROR;
    }
    buf__arg4_w = PyMemoryView_GET_BUFFER(memview__arg4_w);
    if (buf__arg4_w->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_w");
        goto ERROR;
      
    }
    _arg4_w = (double*) buf__arg4_w->buf;
    if (buf__arg4_w->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_w");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_syeig(self->ptr,_arg1_uplo,_arg2_n,_arg3_a,_arg4_w);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* syeig */

static PyObject * PyMSK_syevd(mosek_EnvObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  MSKuploe _arg1_uplo;
  int _arg2_n;
  PyObject * memview__arg3_a = NULL;
  Py_buffer * buf__arg3_a;
  double * _arg3_a = NULL;
  PyObject * memview__arg4_w = NULL;
  Py_buffer * buf__arg4_w;
  double * _arg4_w = NULL;
  if (! PyArg_ParseTuple(_args,"iiOO",&_arg1_uplo,&_arg2_n,&memview__arg3_a,&memview__arg4_w)) goto ERROR;
  if (Py_None != memview__arg3_a)
  {
    if (! PyMemoryView_Check(memview__arg3_a))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg3_a");
      goto ERROR;
    }
    buf__arg3_a = PyMemoryView_GET_BUFFER(memview__arg3_a);
    if (buf__arg3_a->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg3_a");
        goto ERROR;
      
    }
    _arg3_a = (double*) buf__arg3_a->buf;
    if (buf__arg3_a->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg3_a");
        goto ERROR;
      
    }
  }
  if (Py_None != memview__arg4_w)
  {
    if (! PyMemoryView_Check(memview__arg4_w))
    {
      PyErr_SetString(PyExc_TypeError,"Expected a memoryview object for argument _arg4_w");
      goto ERROR;
    }
    buf__arg4_w = PyMemoryView_GET_BUFFER(memview__arg4_w);
    if (buf__arg4_w->strides[0] != sizeof(double))
    {
        PyErr_SetString(PyExc_TypeError,"Expected a continuous memoryview object for argument _arg4_w");
        goto ERROR;
      
    }
    _arg4_w = (double*) buf__arg4_w->buf;
    if (buf__arg4_w->readonly)
    {
        PyErr_SetString(PyExc_TypeError,"Expected a writable memoryview object for argument _arg4_w");
        goto ERROR;
      
    }
  }
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_syevd(self->ptr,_arg1_uplo,_arg2_n,_arg3_a,_arg4_w);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* syevd */

static PyObject * PyMSK_licensecleanup(PyObject * self, PyObject * _args) /*3*/
{
  MSKrescodee _r = MSK_RES_OK;
  {
    Py_BEGIN_ALLOW_THREADS
    _r = MSK_licensecleanup();
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) goto ERROR;
    if (_r != MSK_RES_OK) goto RES_ERROR;
  }
  return Py_BuildValue("i",0);
ERROR:
  return NULL;
RES_ERROR:
  return Py_BuildValue("i",_r,Py_None);
} /* licensecleanup */

/******************************************************************************/
/* Lists */

static PyMethodDef Env_methods[] = {
    { "set_Stream",          (PyCFunction)PyMSK_env_set_Stream, METH_VARARGS, "set_Stream(whichstream,func)" },
    { "remove_Stream",       (PyCFunction)PyMSK_env_remove_Stream, METH_VARARGS, "remove_Stream(whichstream)" },
    { "enablegarcolenv",     (PyCFunction)PyMSK_enablegarcolenv, METH_NOARGS,"enablegarcolenv()" },
  { "checkoutlicense",(PyCFunction)PyMSK_checkoutlicense, METH_VARARGS, "checkoutlicense(_arg1_feature)" },
  { "checkinlicense",(PyCFunction)PyMSK_checkinlicense, METH_VARARGS, "checkinlicense(_arg1_feature)" },
  { "checkinall",(PyCFunction)PyMSK_checkinall, METH_NOARGS, "checkinall()" },
  { "setupthreads",(PyCFunction)PyMSK_setupthreads, METH_VARARGS, "setupthreads(_arg1_numthreads)" },
  { "echointro",(PyCFunction)PyMSK_echointro, METH_VARARGS, "echointro(_arg1_longver)" },
  { "getcodedesc",(PyCFunction)PyMSK_getcodedesc, METH_VARARGS|METH_STATIC, "getcodedesc(_arg0_code,_arg1_symname,_arg2_str)" },
  { "getversion",(PyCFunction)PyMSK_getversion, METH_NOARGS|METH_STATIC, "getversion()" },
  { "linkfiletoenvstream",(PyCFunction)PyMSK_linkfiletoenvstream, METH_VARARGS, "linkfiletoenvstream(_arg1_whichstream,_arg2_filename,_arg3_append)" },
  { "putlicensedebug",(PyCFunction)PyMSK_putlicensedebug, METH_VARARGS, "putlicensedebug(_arg1_licdebug)" },
  { "putlicensecode",(PyCFunction)PyMSK_putlicensecode, METH_VARARGS, "putlicensecode(_arg1_code)" },
  { "putlicensewait",(PyCFunction)PyMSK_putlicensewait, METH_VARARGS, "putlicensewait(_arg1_licwait)" },
  { "putlicensepath",(PyCFunction)PyMSK_putlicensepath, METH_VARARGS, "putlicensepath(_arg1_licensepath)" },
  { "axpy",(PyCFunction)PyMSK_axpy, METH_VARARGS, "axpy(_arg1_n,_arg2_alpha,_arg3_x,_arg4_y)" },
  { "dot",(PyCFunction)PyMSK_dot, METH_VARARGS, "dot(_arg1_n,_arg2_x,_arg3_y)" },
  { "gemv",(PyCFunction)PyMSK_gemv, METH_VARARGS, "gemv(_arg1_transa,_arg2_m,_arg3_n,_arg4_alpha,_arg5_a,_arg6_x,_arg7_beta,_arg8_y)" },
  { "gemm",(PyCFunction)PyMSK_gemm, METH_VARARGS, "gemm(_arg1_transa,_arg2_transb,_arg3_m,_arg4_n,_arg5_k,_arg6_alpha,_arg7_a,_arg8_b,_arg9_beta,_arg10_c)" },
  { "syrk",(PyCFunction)PyMSK_syrk, METH_VARARGS, "syrk(_arg1_uplo,_arg2_trans,_arg3_n,_arg4_k,_arg5_alpha,_arg6_a,_arg7_beta,_arg8_c)" },
  { "computesparsecholesky",(PyCFunction)PyMSK_computesparsecholesky, METH_VARARGS, "computesparsecholesky(_arg1_multithread,_arg2_ordermethod,_arg3_tolsingular,_arg4_n,_arg5_anzc,_arg6_aptrc,_arg7_asubc,_arg8_avalc)" },
  { "sparsetriangularsolvedense",(PyCFunction)PyMSK_sparsetriangularsolvedense, METH_VARARGS, "sparsetriangularsolvedense(_arg1_transposed,_arg2_n,_arg3_lnzc,_arg4_lptrc,_arg5_lensubnval,_arg6_lsubc,_arg7_lvalc,_arg8_b)" },
  { "potrf",(PyCFunction)PyMSK_potrf, METH_VARARGS, "potrf(_arg1_uplo,_arg2_n,_arg3_a)" },
  { "syeig",(PyCFunction)PyMSK_syeig, METH_VARARGS, "syeig(_arg1_uplo,_arg2_n,_arg3_a,_arg4_w)" },
  { "syevd",(PyCFunction)PyMSK_syevd, METH_VARARGS, "syevd(_arg1_uplo,_arg2_n,_arg3_a,_arg4_w)" },
  { "licensecleanup",(PyCFunction)PyMSK_licensecleanup, METH_NOARGS|METH_STATIC, "licensecleanup()" },
{ "dispose",             (PyCFunction)PyMSK_env_dispose, METH_NOARGS,"dispose()" },
    { NULL }  /* Sentinel */
};

static PyMethodDef Task_methods[] = {
  { "analyzeproblem",(PyCFunction)PyMSK_analyzeproblem, METH_VARARGS, "analyzeproblem(_arg1_whichstream)" },
  { "analyzenames",(PyCFunction)PyMSK_analyzenames, METH_VARARGS, "analyzenames(_arg1_whichstream,_arg2_nametype)" },
  { "analyzesolution",(PyCFunction)PyMSK_analyzesolution, METH_VARARGS, "analyzesolution(_arg1_whichstream,_arg2_whichsol)" },
  { "initbasissolve",(PyCFunction)PyMSK_initbasissolve, METH_VARARGS, "initbasissolve(_arg1_basis)" },
  { "solvewithbasis",(PyCFunction)PyMSK_solvewithbasis, METH_VARARGS, "solvewithbasis(_arg1_transp,_arg2_numnz,_arg3_sub,_arg4_val)" },
  { "basiscond",(PyCFunction)PyMSK_basiscond, METH_NOARGS, "basiscond()" },
  { "appendcons",(PyCFunction)PyMSK_appendcons, METH_VARARGS, "appendcons(_arg1_num)" },
  { "appendvars",(PyCFunction)PyMSK_appendvars, METH_VARARGS, "appendvars(_arg1_num)" },
  { "removecons",(PyCFunction)PyMSK_removecons, METH_VARARGS, "removecons(_arg1_num,_arg2_subset)" },
  { "removevars",(PyCFunction)PyMSK_removevars, METH_VARARGS, "removevars(_arg1_num,_arg2_subset)" },
  { "removebarvars",(PyCFunction)PyMSK_removebarvars, METH_VARARGS, "removebarvars(_arg1_num,_arg2_subset)" },
  { "removecones",(PyCFunction)PyMSK_removecones, METH_VARARGS, "removecones(_arg1_num,_arg2_subset)" },
  { "appendbarvars",(PyCFunction)PyMSK_appendbarvars, METH_VARARGS, "appendbarvars(_arg1_num,_arg2_dim)" },
  { "appendcone",(PyCFunction)PyMSK_appendcone, METH_VARARGS, "appendcone(_arg1_ct,_arg2_conepar,_arg3_nummem,_arg4_submem)" },
  { "appendconeseq",(PyCFunction)PyMSK_appendconeseq, METH_VARARGS, "appendconeseq(_arg1_ct,_arg2_conepar,_arg3_nummem,_arg4_j)" },
  { "appendconesseq",(PyCFunction)PyMSK_appendconesseq, METH_VARARGS, "appendconesseq(_arg1_num,_arg2_ct,_arg3_conepar,_arg4_nummem,_arg5_j)" },
  { "chgconbound",(PyCFunction)PyMSK_chgconbound, METH_VARARGS, "chgconbound(_arg1_i,_arg2_lower,_arg3_finite,_arg4_value)" },
  { "chgvarbound",(PyCFunction)PyMSK_chgvarbound, METH_VARARGS, "chgvarbound(_arg1_j,_arg2_lower,_arg3_finite,_arg4_value)" },
  { "getaij",(PyCFunction)PyMSK_getaij, METH_VARARGS, "getaij(_arg1_i,_arg2_j)" },
  { "getapiecenumnz",(PyCFunction)PyMSK_getapiecenumnz, METH_VARARGS, "getapiecenumnz(_arg1_firsti,_arg2_lasti,_arg3_firstj,_arg4_lastj)" },
  { "getacolnumnz",(PyCFunction)PyMSK_getacolnumnz, METH_VARARGS, "getacolnumnz(_arg1_i)" },
  { "getacol",(PyCFunction)PyMSK_getacol, METH_VARARGS, "getacol(_arg1_j,_arg3_subj,_arg4_valj)" },
  { "getacolslice64",(PyCFunction)PyMSK_getacolslice64, METH_VARARGS, "getacolslice64(_arg1_first,_arg2_last,_arg3_maxnumnz,_arg4_surp,_arg5_ptrb,_arg6_ptre,_arg7_sub,_arg8_val)" },
  { "getarownumnz",(PyCFunction)PyMSK_getarownumnz, METH_VARARGS, "getarownumnz(_arg1_i)" },
  { "getarow",(PyCFunction)PyMSK_getarow, METH_VARARGS, "getarow(_arg1_i,_arg3_subi,_arg4_vali)" },
  { "getacolslicenumnz64",(PyCFunction)PyMSK_getacolslicenumnz64, METH_VARARGS, "getacolslicenumnz64(_arg1_first,_arg2_last)" },
  { "getarowslicenumnz64",(PyCFunction)PyMSK_getarowslicenumnz64, METH_VARARGS, "getarowslicenumnz64(_arg1_first,_arg2_last)" },
  { "getarowslice64",(PyCFunction)PyMSK_getarowslice64, METH_VARARGS, "getarowslice64(_arg1_first,_arg2_last,_arg3_maxnumnz,_arg4_surp,_arg5_ptrb,_arg6_ptre,_arg7_sub,_arg8_val)" },
  { "getarowslicetrip",(PyCFunction)PyMSK_getarowslicetrip, METH_VARARGS, "getarowslicetrip(_arg1_first,_arg2_last,_arg3_maxnumnz,_arg4_surp,_arg5_subi,_arg6_subj,_arg7_val)" },
  { "getacolslicetrip",(PyCFunction)PyMSK_getacolslicetrip, METH_VARARGS, "getacolslicetrip(_arg1_first,_arg2_last,_arg3_maxnumnz,_arg4_surp,_arg5_subi,_arg6_subj,_arg7_val)" },
  { "getconbound",(PyCFunction)PyMSK_getconbound, METH_VARARGS, "getconbound(_arg1_i)" },
  { "getvarbound",(PyCFunction)PyMSK_getvarbound, METH_VARARGS, "getvarbound(_arg1_i)" },
  { "getconboundslice",(PyCFunction)PyMSK_getconboundslice, METH_VARARGS, "getconboundslice(_arg1_first,_arg2_last,_arg3_bk,_arg4_bl,_arg5_bu)" },
  { "getvarboundslice",(PyCFunction)PyMSK_getvarboundslice, METH_VARARGS, "getvarboundslice(_arg1_first,_arg2_last,_arg3_bk,_arg4_bl,_arg5_bu)" },
  { "getcj",(PyCFunction)PyMSK_getcj, METH_VARARGS, "getcj(_arg1_j)" },
  { "getc",(PyCFunction)PyMSK_getc, METH_VARARGS, "getc(_arg1_c)" },
  { "getcfix",(PyCFunction)PyMSK_getcfix, METH_NOARGS, "getcfix()" },
  { "getcone",(PyCFunction)PyMSK_getcone, METH_VARARGS, "getcone(_arg1_k,_arg5_submem)" },
  { "getconeinfo",(PyCFunction)PyMSK_getconeinfo, METH_VARARGS, "getconeinfo(_arg1_k)" },
  { "getclist",(PyCFunction)PyMSK_getclist, METH_VARARGS, "getclist(_arg1_num,_arg2_subj,_arg3_c)" },
  { "getcslice",(PyCFunction)PyMSK_getcslice, METH_VARARGS, "getcslice(_arg1_first,_arg2_last,_arg3_c)" },
  { "getdouinf",(PyCFunction)PyMSK_getdouinf, METH_VARARGS, "getdouinf(_arg1_whichdinf)" },
  { "getdouparam",(PyCFunction)PyMSK_getdouparam, METH_VARARGS, "getdouparam(_arg1_param)" },
  { "getdualobj",(PyCFunction)PyMSK_getdualobj, METH_VARARGS, "getdualobj(_arg1_whichsol)" },
  { "getintinf",(PyCFunction)PyMSK_getintinf, METH_VARARGS, "getintinf(_arg1_whichiinf)" },
  { "getlintinf",(PyCFunction)PyMSK_getlintinf, METH_VARARGS, "getlintinf(_arg1_whichliinf)" },
  { "getintparam",(PyCFunction)PyMSK_getintparam, METH_VARARGS, "getintparam(_arg1_param)" },
  { "getmaxnumanz64",(PyCFunction)PyMSK_getmaxnumanz64, METH_NOARGS, "getmaxnumanz64()" },
  { "getmaxnumcon",(PyCFunction)PyMSK_getmaxnumcon, METH_NOARGS, "getmaxnumcon()" },
  { "getmaxnumvar",(PyCFunction)PyMSK_getmaxnumvar, METH_NOARGS, "getmaxnumvar()" },
  { "getbarvarnamelen",(PyCFunction)PyMSK_getbarvarnamelen, METH_VARARGS, "getbarvarnamelen(_arg1_i)" },
  { "getbarvarname",(PyCFunction)PyMSK_getbarvarname, METH_VARARGS, "getbarvarname(_arg1_i,_arg2_sizename,_arg3_name)" },
  { "getbarvarnameindex",(PyCFunction)PyMSK_getbarvarnameindex, METH_VARARGS, "getbarvarnameindex(_arg1_somename)" },
  { "generatevarnames",(PyCFunction)PyMSK_generatevarnames, METH_VARARGS, "generatevarnames(_arg1_num,_arg2_subj,_arg3_fmt,_arg4_ndims,_arg5_dims,_arg6_sp)" },
  { "generateconnames",(PyCFunction)PyMSK_generateconnames, METH_VARARGS, "generateconnames(_arg1_num,_arg2_subi,_arg3_fmt,_arg4_ndims,_arg5_dims,_arg6_sp)" },
  { "generateconenames",(PyCFunction)PyMSK_generateconenames, METH_VARARGS, "generateconenames(_arg1_num,_arg2_subk,_arg3_fmt,_arg4_ndims,_arg5_dims,_arg6_sp)" },
  { "putconname",(PyCFunction)PyMSK_putconname, METH_VARARGS, "putconname(_arg1_i,_arg2_name)" },
  { "putvarname",(PyCFunction)PyMSK_putvarname, METH_VARARGS, "putvarname(_arg1_j,_arg2_name)" },
  { "putconename",(PyCFunction)PyMSK_putconename, METH_VARARGS, "putconename(_arg1_j,_arg2_name)" },
  { "putbarvarname",(PyCFunction)PyMSK_putbarvarname, METH_VARARGS, "putbarvarname(_arg1_j,_arg2_name)" },
  { "getvarnamelen",(PyCFunction)PyMSK_getvarnamelen, METH_VARARGS, "getvarnamelen(_arg1_i)" },
  { "getvarname",(PyCFunction)PyMSK_getvarname, METH_VARARGS, "getvarname(_arg1_j,_arg2_sizename,_arg3_name)" },
  { "getconnamelen",(PyCFunction)PyMSK_getconnamelen, METH_VARARGS, "getconnamelen(_arg1_i)" },
  { "getconname",(PyCFunction)PyMSK_getconname, METH_VARARGS, "getconname(_arg1_i,_arg2_sizename,_arg3_name)" },
  { "getconnameindex",(PyCFunction)PyMSK_getconnameindex, METH_VARARGS, "getconnameindex(_arg1_somename)" },
  { "getvarnameindex",(PyCFunction)PyMSK_getvarnameindex, METH_VARARGS, "getvarnameindex(_arg1_somename)" },
  { "getconenamelen",(PyCFunction)PyMSK_getconenamelen, METH_VARARGS, "getconenamelen(_arg1_i)" },
  { "getconename",(PyCFunction)PyMSK_getconename, METH_VARARGS, "getconename(_arg1_i,_arg2_sizename,_arg3_name)" },
  { "getconenameindex",(PyCFunction)PyMSK_getconenameindex, METH_VARARGS, "getconenameindex(_arg1_somename)" },
  { "getnumanz",(PyCFunction)PyMSK_getnumanz, METH_NOARGS, "getnumanz()" },
  { "getnumanz64",(PyCFunction)PyMSK_getnumanz64, METH_NOARGS, "getnumanz64()" },
  { "getnumcon",(PyCFunction)PyMSK_getnumcon, METH_NOARGS, "getnumcon()" },
  { "getnumcone",(PyCFunction)PyMSK_getnumcone, METH_NOARGS, "getnumcone()" },
  { "getnumconemem",(PyCFunction)PyMSK_getnumconemem, METH_VARARGS, "getnumconemem(_arg1_k)" },
  { "getnumintvar",(PyCFunction)PyMSK_getnumintvar, METH_NOARGS, "getnumintvar()" },
  { "getnumparam",(PyCFunction)PyMSK_getnumparam, METH_VARARGS, "getnumparam(_arg1_partype)" },
  { "getnumqconknz64",(PyCFunction)PyMSK_getnumqconknz64, METH_VARARGS, "getnumqconknz64(_arg1_k)" },
  { "getnumqobjnz64",(PyCFunction)PyMSK_getnumqobjnz64, METH_NOARGS, "getnumqobjnz64()" },
  { "getnumvar",(PyCFunction)PyMSK_getnumvar, METH_NOARGS, "getnumvar()" },
  { "getnumbarvar",(PyCFunction)PyMSK_getnumbarvar, METH_NOARGS, "getnumbarvar()" },
  { "getmaxnumbarvar",(PyCFunction)PyMSK_getmaxnumbarvar, METH_NOARGS, "getmaxnumbarvar()" },
  { "getdimbarvarj",(PyCFunction)PyMSK_getdimbarvarj, METH_VARARGS, "getdimbarvarj(_arg1_j)" },
  { "getlenbarvarj",(PyCFunction)PyMSK_getlenbarvarj, METH_VARARGS, "getlenbarvarj(_arg1_j)" },
  { "getobjname",(PyCFunction)PyMSK_getobjname, METH_VARARGS, "getobjname(_arg1_sizeobjname,_arg2_objname)" },
  { "getobjnamelen",(PyCFunction)PyMSK_getobjnamelen, METH_NOARGS, "getobjnamelen()" },
  { "getprimalobj",(PyCFunction)PyMSK_getprimalobj, METH_VARARGS, "getprimalobj(_arg1_whichsol)" },
  { "getprobtype",(PyCFunction)PyMSK_getprobtype, METH_NOARGS, "getprobtype()" },
  { "getqconk64",(PyCFunction)PyMSK_getqconk64, METH_VARARGS, "getqconk64(_arg1_k,_arg2_maxnumqcnz,_arg3_qcsurp,_arg5_qcsubi,_arg6_qcsubj,_arg7_qcval)" },
  { "getqobj64",(PyCFunction)PyMSK_getqobj64, METH_VARARGS, "getqobj64(_arg1_maxnumqonz,_arg2_qosurp,_arg4_qosubi,_arg5_qosubj,_arg6_qoval)" },
  { "getqobjij",(PyCFunction)PyMSK_getqobjij, METH_VARARGS, "getqobjij(_arg1_i,_arg2_j)" },
  { "getsolution",(PyCFunction)PyMSK_getsolution, METH_VARARGS, "getsolution(_arg1_whichsol,_arg4_skc,_arg5_skx,_arg6_skn,_arg7_xc,_arg8_xx,_arg9_y,_arg10_slc,_arg11_suc,_arg12_slx,_arg13_sux,_arg14_snx)" },
  { "getsolsta",(PyCFunction)PyMSK_getsolsta, METH_VARARGS, "getsolsta(_arg1_whichsol)" },
  { "getprosta",(PyCFunction)PyMSK_getprosta, METH_VARARGS, "getprosta(_arg1_whichsol)" },
  { "getskc",(PyCFunction)PyMSK_getskc, METH_VARARGS, "getskc(_arg1_whichsol,_arg2_skc)" },
  { "getskx",(PyCFunction)PyMSK_getskx, METH_VARARGS, "getskx(_arg1_whichsol,_arg2_skx)" },
  { "getskn",(PyCFunction)PyMSK_getskn, METH_VARARGS, "getskn(_arg1_whichsol,_arg2_skn)" },
  { "getxc",(PyCFunction)PyMSK_getxc, METH_VARARGS, "getxc(_arg1_whichsol,_arg2_xc)" },
  { "getxx",(PyCFunction)PyMSK_getxx, METH_VARARGS, "getxx(_arg1_whichsol,_arg2_xx)" },
  { "gety",(PyCFunction)PyMSK_gety, METH_VARARGS, "gety(_arg1_whichsol,_arg2_y)" },
  { "getslc",(PyCFunction)PyMSK_getslc, METH_VARARGS, "getslc(_arg1_whichsol,_arg2_slc)" },
  { "getsuc",(PyCFunction)PyMSK_getsuc, METH_VARARGS, "getsuc(_arg1_whichsol,_arg2_suc)" },
  { "getslx",(PyCFunction)PyMSK_getslx, METH_VARARGS, "getslx(_arg1_whichsol,_arg2_slx)" },
  { "getsux",(PyCFunction)PyMSK_getsux, METH_VARARGS, "getsux(_arg1_whichsol,_arg2_sux)" },
  { "getsnx",(PyCFunction)PyMSK_getsnx, METH_VARARGS, "getsnx(_arg1_whichsol,_arg2_snx)" },
  { "getskcslice",(PyCFunction)PyMSK_getskcslice, METH_VARARGS, "getskcslice(_arg1_whichsol,_arg2_first,_arg3_last,_arg4_skc)" },
  { "getskxslice",(PyCFunction)PyMSK_getskxslice, METH_VARARGS, "getskxslice(_arg1_whichsol,_arg2_first,_arg3_last,_arg4_skx)" },
  { "getxcslice",(PyCFunction)PyMSK_getxcslice, METH_VARARGS, "getxcslice(_arg1_whichsol,_arg2_first,_arg3_last,_arg4_xc)" },
  { "getxxslice",(PyCFunction)PyMSK_getxxslice, METH_VARARGS, "getxxslice(_arg1_whichsol,_arg2_first,_arg3_last,_arg4_xx)" },
  { "getyslice",(PyCFunction)PyMSK_getyslice, METH_VARARGS, "getyslice(_arg1_whichsol,_arg2_first,_arg3_last,_arg4_y)" },
  { "getslcslice",(PyCFunction)PyMSK_getslcslice, METH_VARARGS, "getslcslice(_arg1_whichsol,_arg2_first,_arg3_last,_arg4_slc)" },
  { "getsucslice",(PyCFunction)PyMSK_getsucslice, METH_VARARGS, "getsucslice(_arg1_whichsol,_arg2_first,_arg3_last,_arg4_suc)" },
  { "getslxslice",(PyCFunction)PyMSK_getslxslice, METH_VARARGS, "getslxslice(_arg1_whichsol,_arg2_first,_arg3_last,_arg4_slx)" },
  { "getsuxslice",(PyCFunction)PyMSK_getsuxslice, METH_VARARGS, "getsuxslice(_arg1_whichsol,_arg2_first,_arg3_last,_arg4_sux)" },
  { "getsnxslice",(PyCFunction)PyMSK_getsnxslice, METH_VARARGS, "getsnxslice(_arg1_whichsol,_arg2_first,_arg3_last,_arg4_snx)" },
  { "getbarxj",(PyCFunction)PyMSK_getbarxj, METH_VARARGS, "getbarxj(_arg1_whichsol,_arg2_j,_arg3_barxj)" },
  { "getbarxslice",(PyCFunction)PyMSK_getbarxslice, METH_VARARGS, "getbarxslice(_arg1_whichsol,_arg2_first,_arg3_last,_arg4_slicesize,_arg5_barxslice)" },
  { "getbarsj",(PyCFunction)PyMSK_getbarsj, METH_VARARGS, "getbarsj(_arg1_whichsol,_arg2_j,_arg3_barsj)" },
  { "getbarsslice",(PyCFunction)PyMSK_getbarsslice, METH_VARARGS, "getbarsslice(_arg1_whichsol,_arg2_first,_arg3_last,_arg4_slicesize,_arg5_barsslice)" },
  { "putskc",(PyCFunction)PyMSK_putskc, METH_VARARGS, "putskc(_arg1_whichsol,_arg2_skc)" },
  { "putskx",(PyCFunction)PyMSK_putskx, METH_VARARGS, "putskx(_arg1_whichsol,_arg2_skx)" },
  { "putxc",(PyCFunction)PyMSK_putxc, METH_VARARGS, "putxc(_arg1_whichsol,_arg2_xc)" },
  { "putxx",(PyCFunction)PyMSK_putxx, METH_VARARGS, "putxx(_arg1_whichsol,_arg2_xx)" },
  { "puty",(PyCFunction)PyMSK_puty, METH_VARARGS, "puty(_arg1_whichsol,_arg2_y)" },
  { "putslc",(PyCFunction)PyMSK_putslc, METH_VARARGS, "putslc(_arg1_whichsol,_arg2_slc)" },
  { "putsuc",(PyCFunction)PyMSK_putsuc, METH_VARARGS, "putsuc(_arg1_whichsol,_arg2_suc)" },
  { "putslx",(PyCFunction)PyMSK_putslx, METH_VARARGS, "putslx(_arg1_whichsol,_arg2_slx)" },
  { "putsux",(PyCFunction)PyMSK_putsux, METH_VARARGS, "putsux(_arg1_whichsol,_arg2_sux)" },
  { "putsnx",(PyCFunction)PyMSK_putsnx, METH_VARARGS, "putsnx(_arg1_whichsol,_arg2_sux)" },
  { "putskcslice",(PyCFunction)PyMSK_putskcslice, METH_VARARGS, "putskcslice(_arg1_whichsol,_arg2_first,_arg3_last,_arg4_skc)" },
  { "putskxslice",(PyCFunction)PyMSK_putskxslice, METH_VARARGS, "putskxslice(_arg1_whichsol,_arg2_first,_arg3_last,_arg4_skx)" },
  { "putxcslice",(PyCFunction)PyMSK_putxcslice, METH_VARARGS, "putxcslice(_arg1_whichsol,_arg2_first,_arg3_last,_arg4_xc)" },
  { "putxxslice",(PyCFunction)PyMSK_putxxslice, METH_VARARGS, "putxxslice(_arg1_whichsol,_arg2_first,_arg3_last,_arg4_xx)" },
  { "putyslice",(PyCFunction)PyMSK_putyslice, METH_VARARGS, "putyslice(_arg1_whichsol,_arg2_first,_arg3_last,_arg4_y)" },
  { "putslcslice",(PyCFunction)PyMSK_putslcslice, METH_VARARGS, "putslcslice(_arg1_whichsol,_arg2_first,_arg3_last,_arg4_slc)" },
  { "putsucslice",(PyCFunction)PyMSK_putsucslice, METH_VARARGS, "putsucslice(_arg1_whichsol,_arg2_first,_arg3_last,_arg4_suc)" },
  { "putslxslice",(PyCFunction)PyMSK_putslxslice, METH_VARARGS, "putslxslice(_arg1_whichsol,_arg2_first,_arg3_last,_arg4_slx)" },
  { "putsuxslice",(PyCFunction)PyMSK_putsuxslice, METH_VARARGS, "putsuxslice(_arg1_whichsol,_arg2_first,_arg3_last,_arg4_sux)" },
  { "putsnxslice",(PyCFunction)PyMSK_putsnxslice, METH_VARARGS, "putsnxslice(_arg1_whichsol,_arg2_first,_arg3_last,_arg4_snx)" },
  { "putbarxj",(PyCFunction)PyMSK_putbarxj, METH_VARARGS, "putbarxj(_arg1_whichsol,_arg2_j,_arg3_barxj)" },
  { "putbarsj",(PyCFunction)PyMSK_putbarsj, METH_VARARGS, "putbarsj(_arg1_whichsol,_arg2_j,_arg3_barsj)" },
  { "getpviolcon",(PyCFunction)PyMSK_getpviolcon, METH_VARARGS, "getpviolcon(_arg1_whichsol,_arg2_num,_arg3_sub,_arg4_viol)" },
  { "getpviolvar",(PyCFunction)PyMSK_getpviolvar, METH_VARARGS, "getpviolvar(_arg1_whichsol,_arg2_num,_arg3_sub,_arg4_viol)" },
  { "getpviolbarvar",(PyCFunction)PyMSK_getpviolbarvar, METH_VARARGS, "getpviolbarvar(_arg1_whichsol,_arg2_num,_arg3_sub,_arg4_viol)" },
  { "getpviolcones",(PyCFunction)PyMSK_getpviolcones, METH_VARARGS, "getpviolcones(_arg1_whichsol,_arg2_num,_arg3_sub,_arg4_viol)" },
  { "getdviolcon",(PyCFunction)PyMSK_getdviolcon, METH_VARARGS, "getdviolcon(_arg1_whichsol,_arg2_num,_arg3_sub,_arg4_viol)" },
  { "getdviolvar",(PyCFunction)PyMSK_getdviolvar, METH_VARARGS, "getdviolvar(_arg1_whichsol,_arg2_num,_arg3_sub,_arg4_viol)" },
  { "getdviolbarvar",(PyCFunction)PyMSK_getdviolbarvar, METH_VARARGS, "getdviolbarvar(_arg1_whichsol,_arg2_num,_arg3_sub,_arg4_viol)" },
  { "getdviolcones",(PyCFunction)PyMSK_getdviolcones, METH_VARARGS, "getdviolcones(_arg1_whichsol,_arg2_num,_arg3_sub,_arg4_viol)" },
  { "getsolutioninfo",(PyCFunction)PyMSK_getsolutioninfo, METH_VARARGS, "getsolutioninfo(_arg1_whichsol)" },
  { "getdualsolutionnorms",(PyCFunction)PyMSK_getdualsolutionnorms, METH_VARARGS, "getdualsolutionnorms(_arg1_whichsol)" },
  { "getprimalsolutionnorms",(PyCFunction)PyMSK_getprimalsolutionnorms, METH_VARARGS, "getprimalsolutionnorms(_arg1_whichsol)" },
  { "getsolutionslice",(PyCFunction)PyMSK_getsolutionslice, METH_VARARGS, "getsolutionslice(_arg1_whichsol,_arg2_solitem,_arg3_first,_arg4_last,_arg5_values)" },
  { "getreducedcosts",(PyCFunction)PyMSK_getreducedcosts, METH_VARARGS, "getreducedcosts(_arg1_whichsol,_arg2_first,_arg3_last,_arg4_redcosts)" },
  { "getstrparam",(PyCFunction)PyMSK_getstrparam, METH_VARARGS, "getstrparam(_arg1_param,_arg2_maxlen,_arg4_parvalue)" },
  { "getstrparamlen",(PyCFunction)PyMSK_getstrparamlen, METH_VARARGS, "getstrparamlen(_arg1_param)" },
  { "gettasknamelen",(PyCFunction)PyMSK_gettasknamelen, METH_NOARGS, "gettasknamelen()" },
  { "gettaskname",(PyCFunction)PyMSK_gettaskname, METH_VARARGS, "gettaskname(_arg1_sizetaskname,_arg2_taskname)" },
  { "getvartype",(PyCFunction)PyMSK_getvartype, METH_VARARGS, "getvartype(_arg1_j)" },
  { "getvartypelist",(PyCFunction)PyMSK_getvartypelist, METH_VARARGS, "getvartypelist(_arg1_num,_arg2_subj,_arg3_vartype)" },
  { "inputdata64",(PyCFunction)PyMSK_inputdata64, METH_VARARGS, "inputdata64(_arg1_maxnumcon,_arg2_maxnumvar,_arg3_numcon,_arg4_numvar,_arg5_c,_arg6_cfix,_arg7_aptrb,_arg8_aptre,_arg9_asub,_arg10_aval,_arg11_bkc,_arg12_blc,_arg13_buc,_arg14_bkx,_arg15_blx,_arg16_bux)" },
  { "isdouparname",(PyCFunction)PyMSK_isdouparname, METH_VARARGS, "isdouparname(_arg1_parname)" },
  { "isintparname",(PyCFunction)PyMSK_isintparname, METH_VARARGS, "isintparname(_arg1_parname)" },
  { "isstrparname",(PyCFunction)PyMSK_isstrparname, METH_VARARGS, "isstrparname(_arg1_parname)" },
  { "linkfiletotaskstream",(PyCFunction)PyMSK_linkfiletotaskstream, METH_VARARGS, "linkfiletotaskstream(_arg1_whichstream,_arg2_filename,_arg3_append)" },
  { "primalrepair",(PyCFunction)PyMSK_primalrepair, METH_VARARGS, "primalrepair(_arg1_wlc,_arg2_wuc,_arg3_wlx,_arg4_wux)" },
  { "toconic",(PyCFunction)PyMSK_toconic, METH_NOARGS, "toconic()" },
  { "optimizetrm",(PyCFunction)PyMSK_optimizetrm, METH_NOARGS, "optimizetrm()" },
  { "commitchanges",(PyCFunction)PyMSK_commitchanges, METH_NOARGS, "commitchanges()" },
  { "getatruncatetol",(PyCFunction)PyMSK_getatruncatetol, METH_VARARGS, "getatruncatetol(_arg1_tolzero)" },
  { "putatruncatetol",(PyCFunction)PyMSK_putatruncatetol, METH_VARARGS, "putatruncatetol(_arg1_tolzero)" },
  { "putaij",(PyCFunction)PyMSK_putaij, METH_VARARGS, "putaij(_arg1_i,_arg2_j,_arg3_aij)" },
  { "putaijlist64",(PyCFunction)PyMSK_putaijlist64, METH_VARARGS, "putaijlist64(_arg1_num,_arg2_subi,_arg3_subj,_arg4_valij)" },
  { "putacol",(PyCFunction)PyMSK_putacol, METH_VARARGS, "putacol(_arg1_j,_arg2_nzj,_arg3_subj,_arg4_valj)" },
  { "putarow",(PyCFunction)PyMSK_putarow, METH_VARARGS, "putarow(_arg1_i,_arg2_nzi,_arg3_subi,_arg4_vali)" },
  { "putarowslice64",(PyCFunction)PyMSK_putarowslice64, METH_VARARGS, "putarowslice64(_arg1_first,_arg2_last,_arg3_ptrb,_arg4_ptre,_arg5_asub,_arg6_aval)" },
  { "putarowlist64",(PyCFunction)PyMSK_putarowlist64, METH_VARARGS, "putarowlist64(_arg1_num,_arg2_sub,_arg3_ptrb,_arg4_ptre,_arg5_asub,_arg6_aval)" },
  { "putacolslice64",(PyCFunction)PyMSK_putacolslice64, METH_VARARGS, "putacolslice64(_arg1_first,_arg2_last,_arg3_ptrb,_arg4_ptre,_arg5_asub,_arg6_aval)" },
  { "putacollist64",(PyCFunction)PyMSK_putacollist64, METH_VARARGS, "putacollist64(_arg1_num,_arg2_sub,_arg3_ptrb,_arg4_ptre,_arg5_asub,_arg6_aval)" },
  { "putbaraij",(PyCFunction)PyMSK_putbaraij, METH_VARARGS, "putbaraij(_arg1_i,_arg2_j,_arg3_num,_arg4_sub,_arg5_weights)" },
  { "putbaraijlist",(PyCFunction)PyMSK_putbaraijlist, METH_VARARGS, "putbaraijlist(_arg1_num,_arg2_subi,_arg3_subj,_arg4_alphaptrb,_arg5_alphaptre,_arg6_matidx,_arg7_weights)" },
  { "putbararowlist",(PyCFunction)PyMSK_putbararowlist, METH_VARARGS, "putbararowlist(_arg1_num,_arg2_subi,_arg3_ptrb,_arg4_ptre,_arg5_subj,_arg6_nummat,_arg7_matidx,_arg8_weights)" },
  { "getnumbarcnz",(PyCFunction)PyMSK_getnumbarcnz, METH_NOARGS, "getnumbarcnz()" },
  { "getnumbaranz",(PyCFunction)PyMSK_getnumbaranz, METH_NOARGS, "getnumbaranz()" },
  { "getbarcsparsity",(PyCFunction)PyMSK_getbarcsparsity, METH_VARARGS, "getbarcsparsity(_arg1_maxnumnz,_arg3_idxj)" },
  { "getbarasparsity",(PyCFunction)PyMSK_getbarasparsity, METH_VARARGS, "getbarasparsity(_arg1_maxnumnz,_arg3_idxij)" },
  { "getbarcidxinfo",(PyCFunction)PyMSK_getbarcidxinfo, METH_VARARGS, "getbarcidxinfo(_arg1_idx)" },
  { "getbarcidxj",(PyCFunction)PyMSK_getbarcidxj, METH_VARARGS, "getbarcidxj(_arg1_idx)" },
  { "getbarcidx",(PyCFunction)PyMSK_getbarcidx, METH_VARARGS, "getbarcidx(_arg1_idx,_arg2_maxnum,_arg5_sub,_arg6_weights)" },
  { "getbaraidxinfo",(PyCFunction)PyMSK_getbaraidxinfo, METH_VARARGS, "getbaraidxinfo(_arg1_idx)" },
  { "getbaraidxij",(PyCFunction)PyMSK_getbaraidxij, METH_VARARGS, "getbaraidxij(_arg1_idx)" },
  { "getbaraidx",(PyCFunction)PyMSK_getbaraidx, METH_VARARGS, "getbaraidx(_arg1_idx,_arg2_maxnum,_arg6_sub,_arg7_weights)" },
  { "getnumbarcblocktriplets",(PyCFunction)PyMSK_getnumbarcblocktriplets, METH_NOARGS, "getnumbarcblocktriplets()" },
  { "putbarcblocktriplet",(PyCFunction)PyMSK_putbarcblocktriplet, METH_VARARGS, "putbarcblocktriplet(_arg1_num,_arg2_subj,_arg3_subk,_arg4_subl,_arg5_valjkl)" },
  { "getbarcblocktriplet",(PyCFunction)PyMSK_getbarcblocktriplet, METH_VARARGS, "getbarcblocktriplet(_arg1_maxnum,_arg3_subj,_arg4_subk,_arg5_subl,_arg6_valjkl)" },
  { "putbarablocktriplet",(PyCFunction)PyMSK_putbarablocktriplet, METH_VARARGS, "putbarablocktriplet(_arg1_num,_arg2_subi,_arg3_subj,_arg4_subk,_arg5_subl,_arg6_valijkl)" },
  { "getnumbarablocktriplets",(PyCFunction)PyMSK_getnumbarablocktriplets, METH_NOARGS, "getnumbarablocktriplets()" },
  { "getbarablocktriplet",(PyCFunction)PyMSK_getbarablocktriplet, METH_VARARGS, "getbarablocktriplet(_arg1_maxnum,_arg3_subi,_arg4_subj,_arg5_subk,_arg6_subl,_arg7_valijkl)" },
  { "putconbound",(PyCFunction)PyMSK_putconbound, METH_VARARGS, "putconbound(_arg1_i,_arg2_bkc,_arg3_blc,_arg4_buc)" },
  { "putconboundlist",(PyCFunction)PyMSK_putconboundlist, METH_VARARGS, "putconboundlist(_arg1_num,_arg2_sub,_arg3_bkc,_arg4_blc,_arg5_buc)" },
  { "putconboundlistconst",(PyCFunction)PyMSK_putconboundlistconst, METH_VARARGS, "putconboundlistconst(_arg1_num,_arg2_sub,_arg3_bkc,_arg4_blc,_arg5_buc)" },
  { "putconboundslice",(PyCFunction)PyMSK_putconboundslice, METH_VARARGS, "putconboundslice(_arg1_first,_arg2_last,_arg3_bkc,_arg4_blc,_arg5_buc)" },
  { "putconboundsliceconst",(PyCFunction)PyMSK_putconboundsliceconst, METH_VARARGS, "putconboundsliceconst(_arg1_first,_arg2_last,_arg3_bkc,_arg4_blc,_arg5_buc)" },
  { "putvarbound",(PyCFunction)PyMSK_putvarbound, METH_VARARGS, "putvarbound(_arg1_j,_arg2_bkx,_arg3_blx,_arg4_bux)" },
  { "putvarboundlist",(PyCFunction)PyMSK_putvarboundlist, METH_VARARGS, "putvarboundlist(_arg1_num,_arg2_sub,_arg3_bkx,_arg4_blx,_arg5_bux)" },
  { "putvarboundlistconst",(PyCFunction)PyMSK_putvarboundlistconst, METH_VARARGS, "putvarboundlistconst(_arg1_num,_arg2_sub,_arg3_bkx,_arg4_blx,_arg5_bux)" },
  { "putvarboundslice",(PyCFunction)PyMSK_putvarboundslice, METH_VARARGS, "putvarboundslice(_arg1_first,_arg2_last,_arg3_bkx,_arg4_blx,_arg5_bux)" },
  { "putvarboundsliceconst",(PyCFunction)PyMSK_putvarboundsliceconst, METH_VARARGS, "putvarboundsliceconst(_arg1_first,_arg2_last,_arg3_bkx,_arg4_blx,_arg5_bux)" },
  { "putcfix",(PyCFunction)PyMSK_putcfix, METH_VARARGS, "putcfix(_arg1_cfix)" },
  { "putcj",(PyCFunction)PyMSK_putcj, METH_VARARGS, "putcj(_arg1_j,_arg2_cj)" },
  { "putobjsense",(PyCFunction)PyMSK_putobjsense, METH_VARARGS, "putobjsense(_arg1_sense)" },
  { "getobjsense",(PyCFunction)PyMSK_getobjsense, METH_NOARGS, "getobjsense()" },
  { "putclist",(PyCFunction)PyMSK_putclist, METH_VARARGS, "putclist(_arg1_num,_arg2_subj,_arg3_val)" },
  { "putcslice",(PyCFunction)PyMSK_putcslice, METH_VARARGS, "putcslice(_arg1_first,_arg2_last,_arg3_slice)" },
  { "putbarcj",(PyCFunction)PyMSK_putbarcj, METH_VARARGS, "putbarcj(_arg1_j,_arg2_num,_arg3_sub,_arg4_weights)" },
  { "putcone",(PyCFunction)PyMSK_putcone, METH_VARARGS, "putcone(_arg1_k,_arg2_ct,_arg3_conepar,_arg4_nummem,_arg5_submem)" },
  { "appendsparsesymmat",(PyCFunction)PyMSK_appendsparsesymmat, METH_VARARGS, "appendsparsesymmat(_arg1_dim,_arg2_nz,_arg3_subi,_arg4_subj,_arg5_valij)" },
  { "appendsparsesymmatlist",(PyCFunction)PyMSK_appendsparsesymmatlist, METH_VARARGS, "appendsparsesymmatlist(_arg1_num,_arg2_dims,_arg3_nz,_arg4_subi,_arg5_subj,_arg6_valij,_arg7_idx)" },
  { "getsymmatinfo",(PyCFunction)PyMSK_getsymmatinfo, METH_VARARGS, "getsymmatinfo(_arg1_idx)" },
  { "getnumsymmat",(PyCFunction)PyMSK_getnumsymmat, METH_NOARGS, "getnumsymmat()" },
  { "getsparsesymmat",(PyCFunction)PyMSK_getsparsesymmat, METH_VARARGS, "getsparsesymmat(_arg1_idx,_arg2_maxlen,_arg3_subi,_arg4_subj,_arg5_valij)" },
  { "putdouparam",(PyCFunction)PyMSK_putdouparam, METH_VARARGS, "putdouparam(_arg1_param,_arg2_parvalue)" },
  { "putintparam",(PyCFunction)PyMSK_putintparam, METH_VARARGS, "putintparam(_arg1_param,_arg2_parvalue)" },
  { "putmaxnumcon",(PyCFunction)PyMSK_putmaxnumcon, METH_VARARGS, "putmaxnumcon(_arg1_maxnumcon)" },
  { "putmaxnumcone",(PyCFunction)PyMSK_putmaxnumcone, METH_VARARGS, "putmaxnumcone(_arg1_maxnumcone)" },
  { "getmaxnumcone",(PyCFunction)PyMSK_getmaxnumcone, METH_NOARGS, "getmaxnumcone()" },
  { "putmaxnumvar",(PyCFunction)PyMSK_putmaxnumvar, METH_VARARGS, "putmaxnumvar(_arg1_maxnumvar)" },
  { "putmaxnumbarvar",(PyCFunction)PyMSK_putmaxnumbarvar, METH_VARARGS, "putmaxnumbarvar(_arg1_maxnumbarvar)" },
  { "putmaxnumanz",(PyCFunction)PyMSK_putmaxnumanz, METH_VARARGS, "putmaxnumanz(_arg1_maxnumanz)" },
  { "putmaxnumqnz",(PyCFunction)PyMSK_putmaxnumqnz, METH_VARARGS, "putmaxnumqnz(_arg1_maxnumqnz)" },
  { "getmaxnumqnz64",(PyCFunction)PyMSK_getmaxnumqnz64, METH_NOARGS, "getmaxnumqnz64()" },
  { "putnadouparam",(PyCFunction)PyMSK_putnadouparam, METH_VARARGS, "putnadouparam(_arg1_paramname,_arg2_parvalue)" },
  { "putnaintparam",(PyCFunction)PyMSK_putnaintparam, METH_VARARGS, "putnaintparam(_arg1_paramname,_arg2_parvalue)" },
  { "putnastrparam",(PyCFunction)PyMSK_putnastrparam, METH_VARARGS, "putnastrparam(_arg1_paramname,_arg2_parvalue)" },
  { "putobjname",(PyCFunction)PyMSK_putobjname, METH_VARARGS, "putobjname(_arg1_objname)" },
  { "putparam",(PyCFunction)PyMSK_putparam, METH_VARARGS, "putparam(_arg1_parname,_arg2_parvalue)" },
  { "putqcon",(PyCFunction)PyMSK_putqcon, METH_VARARGS, "putqcon(_arg1_numqcnz,_arg2_qcsubk,_arg3_qcsubi,_arg4_qcsubj,_arg5_qcval)" },
  { "putqconk",(PyCFunction)PyMSK_putqconk, METH_VARARGS, "putqconk(_arg1_k,_arg2_numqcnz,_arg3_qcsubi,_arg4_qcsubj,_arg5_qcval)" },
  { "putqobj",(PyCFunction)PyMSK_putqobj, METH_VARARGS, "putqobj(_arg1_numqonz,_arg2_qosubi,_arg3_qosubj,_arg4_qoval)" },
  { "putqobjij",(PyCFunction)PyMSK_putqobjij, METH_VARARGS, "putqobjij(_arg1_i,_arg2_j,_arg3_qoij)" },
  { "putsolution",(PyCFunction)PyMSK_putsolution, METH_VARARGS, "putsolution(_arg1_whichsol,_arg2_skc,_arg3_skx,_arg4_skn,_arg5_xc,_arg6_xx,_arg7_y,_arg8_slc,_arg9_suc,_arg10_slx,_arg11_sux,_arg12_snx)" },
  { "putconsolutioni",(PyCFunction)PyMSK_putconsolutioni, METH_VARARGS, "putconsolutioni(_arg1_i,_arg2_whichsol,_arg3_sk,_arg4_x,_arg5_sl,_arg6_su)" },
  { "putvarsolutionj",(PyCFunction)PyMSK_putvarsolutionj, METH_VARARGS, "putvarsolutionj(_arg1_j,_arg2_whichsol,_arg3_sk,_arg4_x,_arg5_sl,_arg6_su,_arg7_sn)" },
  { "putsolutionyi",(PyCFunction)PyMSK_putsolutionyi, METH_VARARGS, "putsolutionyi(_arg1_i,_arg2_whichsol,_arg3_y)" },
  { "putstrparam",(PyCFunction)PyMSK_putstrparam, METH_VARARGS, "putstrparam(_arg1_param,_arg2_parvalue)" },
  { "puttaskname",(PyCFunction)PyMSK_puttaskname, METH_VARARGS, "puttaskname(_arg1_taskname)" },
  { "putvartype",(PyCFunction)PyMSK_putvartype, METH_VARARGS, "putvartype(_arg1_j,_arg2_vartype)" },
  { "putvartypelist",(PyCFunction)PyMSK_putvartypelist, METH_VARARGS, "putvartypelist(_arg1_num,_arg2_subj,_arg3_vartype)" },
  { "readdataformat",(PyCFunction)PyMSK_readdataformat, METH_VARARGS, "readdataformat(_arg1_filename,_arg2_format,_arg3_compress)" },
  { "readdataautoformat",(PyCFunction)PyMSK_readdataautoformat, METH_VARARGS, "readdataautoformat(_arg1_filename)" },
  { "readparamfile",(PyCFunction)PyMSK_readparamfile, METH_VARARGS, "readparamfile(_arg1_filename)" },
  { "readsolution",(PyCFunction)PyMSK_readsolution, METH_VARARGS, "readsolution(_arg1_whichsol,_arg2_filename)" },
  { "readsummary",(PyCFunction)PyMSK_readsummary, METH_VARARGS, "readsummary(_arg1_whichstream)" },
  { "resizetask",(PyCFunction)PyMSK_resizetask, METH_VARARGS, "resizetask(_arg1_maxnumcon,_arg2_maxnumvar,_arg3_maxnumcone,_arg4_maxnumanz,_arg5_maxnumqnz)" },
  { "checkmemtask",(PyCFunction)PyMSK_checkmemtask, METH_VARARGS, "checkmemtask(_arg1_file,_arg2_line)" },
  { "getmemusagetask",(PyCFunction)PyMSK_getmemusagetask, METH_NOARGS, "getmemusagetask()" },
  { "setdefaults",(PyCFunction)PyMSK_setdefaults, METH_NOARGS, "setdefaults()" },
  { "solutiondef",(PyCFunction)PyMSK_solutiondef, METH_VARARGS, "solutiondef(_arg1_whichsol)" },
  { "deletesolution",(PyCFunction)PyMSK_deletesolution, METH_VARARGS, "deletesolution(_arg1_whichsol)" },
  { "onesolutionsummary",(PyCFunction)PyMSK_onesolutionsummary, METH_VARARGS, "onesolutionsummary(_arg1_whichstream,_arg2_whichsol)" },
  { "solutionsummary",(PyCFunction)PyMSK_solutionsummary, METH_VARARGS, "solutionsummary(_arg1_whichstream)" },
  { "updatesolutioninfo",(PyCFunction)PyMSK_updatesolutioninfo, METH_VARARGS, "updatesolutioninfo(_arg1_whichsol)" },
  { "optimizersummary",(PyCFunction)PyMSK_optimizersummary, METH_VARARGS, "optimizersummary(_arg1_whichstream)" },
  { "strtoconetype",(PyCFunction)PyMSK_strtoconetype, METH_VARARGS, "strtoconetype(_arg1_str)" },
  { "strtosk",(PyCFunction)PyMSK_strtosk, METH_VARARGS, "strtosk(_arg1_str)" },
  { "writedata",(PyCFunction)PyMSK_writedata, METH_VARARGS, "writedata(_arg1_filename)" },
  { "writetask",(PyCFunction)PyMSK_writetask, METH_VARARGS, "writetask(_arg1_filename)" },
  { "readtask",(PyCFunction)PyMSK_readtask, METH_VARARGS, "readtask(_arg1_filename)" },
  { "readopfstring",(PyCFunction)PyMSK_readopfstring, METH_VARARGS, "readopfstring(_arg1_data)" },
  { "readlpstring",(PyCFunction)PyMSK_readlpstring, METH_VARARGS, "readlpstring(_arg1_data)" },
  { "readjsonstring",(PyCFunction)PyMSK_readjsonstring, METH_VARARGS, "readjsonstring(_arg1_data)" },
  { "readptfstring",(PyCFunction)PyMSK_readptfstring, METH_VARARGS, "readptfstring(_arg1_data)" },
  { "writeparamfile",(PyCFunction)PyMSK_writeparamfile, METH_VARARGS, "writeparamfile(_arg1_filename)" },
  { "getinfeasiblesubproblem",(PyCFunction)PyMSK_getinfeasiblesubproblem, METH_VARARGS, "getinfeasiblesubproblem(_arg1_whichsol)" },
  { "writesolution",(PyCFunction)PyMSK_writesolution, METH_VARARGS, "writesolution(_arg1_whichsol,_arg2_filename)" },
  { "writejsonsol",(PyCFunction)PyMSK_writejsonsol, METH_VARARGS, "writejsonsol(_arg1_filename)" },
  { "primalsensitivity",(PyCFunction)PyMSK_primalsensitivity, METH_VARARGS, "primalsensitivity(_arg1_numi,_arg2_subi,_arg3_marki,_arg4_numj,_arg5_subj,_arg6_markj,_arg7_leftpricei,_arg8_rightpricei,_arg9_leftrangei,_arg10_rightrangei,_arg11_leftpricej,_arg12_rightpricej,_arg13_leftrangej,_arg14_rightrangej)" },
  { "sensitivityreport",(PyCFunction)PyMSK_sensitivityreport, METH_VARARGS, "sensitivityreport(_arg1_whichstream)" },
  { "dualsensitivity",(PyCFunction)PyMSK_dualsensitivity, METH_VARARGS, "dualsensitivity(_arg1_numj,_arg2_subj,_arg3_leftpricej,_arg4_rightpricej,_arg5_leftrangej,_arg6_rightrangej)" },
  { "optimizermt",(PyCFunction)PyMSK_optimizermt, METH_VARARGS, "optimizermt(_arg1_server,_arg2_port)" },
  { "asyncoptimize",(PyCFunction)PyMSK_asyncoptimize, METH_VARARGS, "asyncoptimize(_arg1_server,_arg2_port,_arg3_token)" },
  { "asyncstop",(PyCFunction)PyMSK_asyncstop, METH_VARARGS, "asyncstop(_arg1_server,_arg2_port,_arg3_token)" },
  { "asyncpoll",(PyCFunction)PyMSK_asyncpoll, METH_VARARGS, "asyncpoll(_arg1_server,_arg2_port,_arg3_token)" },
  { "asyncgetresult",(PyCFunction)PyMSK_asyncgetresult, METH_VARARGS, "asyncgetresult(_arg1_server,_arg2_port,_arg3_token)" },
{ "set_Stream",          (PyCFunction)PyMSK_set_Stream, METH_VARARGS, "set_Stream(whichstream,func)" },
    /*{ "set_Progress",        (PyCFunction)PyMSK_set_InfoCallback, METH_VARARGS, "set_Progress(func)" },*/
    { "set_Progress",        (PyCFunction)PyMSK_set_Progress, METH_VARARGS, "set_Progress(func)" },
    { "set_InfoCallback",    (PyCFunction)PyMSK_set_InfoCallback, METH_VARARGS, "set_InfoCallback(func)" },
    { "remove_Stream",       (PyCFunction)PyMSK_remove_Stream, METH_VARARGS, "remove_Stream(whichstream)" },
    { "remove_Progress",     (PyCFunction)PyMSK_remove_Progress, METH_NOARGS, "remove_Progress()" },
    { "remove_InfoCallback", (PyCFunction)PyMSK_remove_InfoCallback, METH_NOARGS, "remove_InfoCallback()" },
    { "getlasterror",        (PyCFunction)PyMSK_getlasterror, METH_NOARGS, "getlasterror(lastmsg)"  },
    { "dispose",             (PyCFunction)PyMSK_task_dispose, METH_NOARGS,"dispose()" }, 
   { NULL }  /* Sentinel */
};

#if 0
static PyMethodDef Module_methods[] = {
{ NULL }  /* Sentinel */
};
#endif

static PyModuleDef mosekmodule = {
    PyModuleDef_HEAD_INIT,
    "_msk",
    "Low level MOSEK optimizer API",
    -1,
    NULL, NULL, NULL, NULL, NULL
};

PyMODINIT_FUNC PyInit__msk(void);

PyMODINIT_FUNC
PyInit__msk(void)
{
    PyObject* m;

    if (PyType_Ready(&mosek_EnvType) < 0 ||
        PyType_Ready(&mosek_TaskType) < 0)
        return NULL;

    m = PyModule_Create(&mosekmodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&mosek_EnvType);
    Py_INCREF(&mosek_EnvType);
    Py_INCREF(&mosek_TaskType);
    PyModule_AddObject(m, "Env", (PyObject *)&mosek_EnvType);
    PyModule_AddObject(m, "Task", (PyObject *)&mosek_TaskType);
    return m;
}



