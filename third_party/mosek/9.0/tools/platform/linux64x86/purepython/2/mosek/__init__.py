"""
    Mosek/Python Module. A Python interface to Mosek.

    Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
"""
import weakref
import sys
import threading
import os
if sys.version_info[0] != 2 or sys.version_info[1] < 5:
    raise ImportError('Invalid python version')
try:
    import ctypes
except ImportError:
    raise ImportError('Mosek module requires ctypes')
try:
    import numpy
except ImportError:
    raise ImportError('Mosek module requires numpy (http://www.numpy.org/)')

try:
    from mosekorigin import __mosekinstpath__
except ImportError:
    __mosekinstpath__ = None

import new
import re
import exceptions

# Due to a bug in some python versions, lookup may fail in a multithreaded context.
import codecs
codecs.lookup('utf-8')

def synchronized(f):
    def sync(self,*args,**kwds):
        with self._lock:
            return f(self,*args,**kwds)
    return sync


def __makelibname(base):
    import platform as __pf
    libname = None
    sysname = __pf.system()

    if  sysname == 'Darwin':
        libname = 'lib%s.dylib' % base
    elif sysname == 'Windows':
        libname = '%s.dll' % base
    elif sysname == 'Linux':
        libname = 'lib%s.so' % base
    else: # assume linux/posix
        raise ImportError('Unknown system "%s"' % sysname)
    del __pf
    return libname

class TypeAcceptError(Exception):
    pass

class MSKException(Exception): pass

class MosekException(MSKException):
    def __init__(self,res,msg):
        MSKException.__init__(self,msg)
        self.msg   = msg
        self.errno = res
    def __str__(self):
        return "(%d) %s" % (self.errno,self.msg)

class Error(MosekException):
    pass

def _make_intvector(v): return v
def _make_longvector(v): return v
def _make_doublevector(v): return v

def _check_stringlist(v):
    for i in v:
        if not (isinstance(i,str) or isinstance(i,unicode)):
            raise TypeAcceptError('expected an array of string objests')
    return v
def _check_taskvector(v):
    for i in v:
        if not (i is None or isinstance(i,Task)):
            raise TypeAcceptError('expected an array of mosek.task objests')
    return v

def _accept_intvector(v):
    return v

def _accept_str(v):
    if not (isinstance(v,str) or isinstance(v,unicode)):
        raise TypeAcceptError("Expected a string argument")
    return v

def _make_anyenumvector(e):
    def ident(v):
        return v

    return ident

def _accept_longvector(v):
    return v

def _accept_doublevector(v):
    return v

def _accept_any(v):
    return v



_make_int = int
_make_long = long
_make_double = float
def _make_str(v):
    if isinstance(v,str) or isinstance(v,unicode):
        return v
    else:
        return unicode(v)


def _accept_anyenum(e):
    def acceptenum(v):
        if isinstance(v,e):
            return v
        else:
            raise TypeError('Expected an %s enum type' % e.__name__)
    return acceptenum

def accepts(*argtlst):
    """
    Decorator for checking function arguments.
    """
    def acceptsfun(fun):
        def accept(*args):
            if len(args) != len(argtlst):
                raise TypeError('Expected %d argument(s) (%d given)' % (len(argtlst), len(args)))
            try:
                return fun(*[ t(a) for (t,a) in zip(argtlst,args) ])
            except TypeAcceptError, e:
                raise TypeError(e)
        accept.__doc__ = fun.__doc__
        accept.__name__ = fun.__name__
        return accept

    return acceptsfun

class EnumBase(int):
    """
    Base class for enums.
    """
    enumnamere = re.compile(r'[a-zA-Z][a-zA-Z0-9_]*$')
    def __new__(cls,value):
        if hasattr(value,'__int__'):
            return cls._valdict[value]
        elif isinstance(value,str) or isinstance(value,unicode):
            return cls._namedict[value.split('.')[-1]]
        else:
            print (value,repr(value))
            raise TypeError("Invalid type for enum construction (%s)" % type(value))
    def __str__(self):
        return '%s.%s' % (self.__class__.__name__,self.__name__)
    def __repr__(self):
        return self.__name__

    @classmethod
    def members(cls):
        return iter(cls._values)

    @classmethod
    def _initialize(cls, names,values=None):
        for n in names:
            if not cls.enumnamere.match(n):
                raise ValueError("Invalid enum item name '%s' in %s" % (n,cls.__name__))
        if values is None:
            values = range(len(names))
        if len(values) != len(names):
            raise ValueError("Lengths of names and values do not match")
        items = []
        for (n,v) in zip(names,values):
            item = int.__new__(cls,v)
            item.__name__ = n
            setattr(cls,n,item)
            items.append(item)

        cls._values   = items
        cls.values    = items
        cls._namedict = dict([ (v.__name__,v) for v in items ])
        cls._valdict  = dict([ (v,v) for v in items ]) # map int -> enum value (sneaky, eh?)
        

def Enum(name,names,values=None):
    """
    Create a new enum class with the given names and values.
    
    Parameters:
     [name]   A string denoting the name of the enum.
     [names]  A list of strings denoting the names of the individual enum values.
     [values] (optional) A list of integer values of the enums. If given, the
       list must have same length as the [names] parameter. If not given, the
       default values 0, 1, ... will be used.
    """
    e = new.classobj(name,(EnumBase,),{})
    e._initialize(names,values)
    return e
    
# module initialization
import platform
if platform.system() == 'Windows':
    __library_factory__ = ctypes.WinDLL 
    __callback_factory__ = ctypes.WINFUNCTYPE # stdcall
elif platform.system() in [ 'Darwin', 'Linux' ]:
    __library_factory__ = ctypes.CDLL 
    __callback_factory__ = ctypes.CFUNCTYPE # cdecl

def loadmosek(libbasename):
    __libname = __makelibname(libbasename)
    ptrsize   = ctypes.sizeof(ctypes.c_void_p) # figure out if python is 32bit or 64bit
    libver    = libbasename[-3:]
    unixlibver = libver.replace('_','.')
    if platform.system() == 'Windows':
        if ptrsize == 8:
            pfname = 'win64x86'
            libdeps = ['mosek64_%s.dll' % libver]
        else:
            pfname = 'win32x86'
            libdeps = ['mosek%s.dll' % libver]
    elif platform.system() in [ 'Darwin', 'Linux' ]:
        libdeps = []

        if platform.system() == 'Darwin':
            pfname = 'osx64x86'
        else:
            if ptrsize == 8: pfname = 'linux64x86'
            else:            pfname = 'linux32x86'
    else:
        raise ImportError("Unsupported system or architecture")



    if platform.system() == 'Windows':
        # _NOT_ pretty or threadsafe, as it globally modifies the load path
        kernel32 = ctypes.WinDLL('Kernel32')
        kernel32.GetDllDirectoryA.argtypes = [ ctypes.c_int32, ctypes.c_char_p ]
        kernel32.GetDllDirectoryA.restype = ctypes.c_int32
        kernel32.SetDllDirectoryA.argtypes = [ ctypes.c_char_p ]
        kernel32.SetDllDirectoryA.restype = ctypes.c_int32

        def loadfrom(dlldir):
            dlldirbuf = (ctypes.c_char*2048)()
            kernel32.GetDllDirectoryA(2048,dlldirbuf)
            kernel32.SetDllDirectoryA(dlldir)
            try:
                return __library_factory__(os.path.join(dlldir,__libname))
            finally:
                kernel32.SetDllDirectoryA(dlldirbuf)
    else:
        def loadfrom(dlldir):
            deps    = []
            for ldep in libdeps:
              try:
                deps.append(__library_factory__(os.path.join(dlldir,ldep)))
              except OSError,e:
                raise ImportError('Failed to import dll "%s" from %s' % (ldep,dlldir))
            try:
                return __library_factory__(os.path.join(dlldir,__libname))
            except OSError,e:
                print e
                raise ImportError('Failed to import dll "%s" from %s' % (__libname,dlldir))

    dlldirs = [ ]
    if __mosekinstpath__ is not None:
        dlldirs.append(os.path.join(__mosekinstpath__))

    # location of libs if this file is located in mosek-py2.zip
    dlldirs.append(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
    library = None
    for dlldir in dlldirs:
        try:
            if ( os.path.isfile(os.path.join(dlldir,__libname)) and
                 all([ os.path.isfile(os.path.join(dlldir,ldep)) for ldep in libdeps ]) ):
                library = loadfrom(dlldir)
                break
        except ImportError:
            pass
    if library is None:
        dlldir = None
        # attempt to load from using global path settings
        try:
            library = __library_factory__(__libname)
        except OSError,e:
            raise ImportError('Failed to import dll "%s" from %s' % (__libname,dlldir))

    return library,dlldir


__library__,__dlldir__ = loadmosek("mosekxx9_0")

__progress_cb_type__ = __callback_factory__(ctypes.c_int,  # return int
                                            ctypes.POINTER(ctypes.c_void_p), # task
                                            ctypes.c_void_p,  # handle
                                            ctypes.c_int,  # caller
                                            ctypes.c_void_p,#ctypes.POINTER(ctypes.c_double),  # dinf
                                            ctypes.c_void_p,#ctypes.POINTER(ctypes.c_int), # iinf
                                            ctypes.c_void_p,#ctypes.POINTER(ctypes.c_longlong))# liinf
                                        )
__stream_cb_type__   = __callback_factory__(None, ctypes.c_void_p, ctypes.c_char_p)

__library__.MSK_XX_makeenv.restype      =   ctypes.c_int
__library__.MSK_XX_makeenv.argtypes     = [ ctypes.POINTER(ctypes.c_void_p), 
                                            ctypes.c_char_p ] 
__library__.MSK_XX_deleteenv.argtypes   = [ ctypes.POINTER(ctypes.c_void_p) ] # envp
__library__.MSK_XX_deleteenv.restype    =   ctypes.c_int
__library__.MSK_XX_maketask.argtypes    = [ ctypes.c_void_p,# env
                                         ctypes.c_int, # maxnumcon
                                         ctypes.c_int, # maxnumvar
                                         ctypes.POINTER(ctypes.c_void_p)] # taskp
__library__.MSK_XX_maketask.restype     =   ctypes.c_int
__library__.MSK_XX_deletetask.argtypes  = [ ctypes.POINTER(ctypes.c_void_p) ] # envp
__library__.MSK_XX_deletetask.restype   =   ctypes.c_int
__library__.MSK_XX_putcallbackfunc.argtypes      = [ ctypes.c_void_p, __progress_cb_type__, ctypes.c_void_p ]
__library__.MSK_XX_putcallbackfunc.restype       =   ctypes.c_int
__library__.MSK_XX_linkfunctotaskstream.argtypes = [ ctypes.c_void_p,    # task 
                                                  ctypes.c_int,       # whichstream
                                                  ctypes.c_void_p,    # handle
                                                  __stream_cb_type__ ] # func
__library__.MSK_XX_linkfunctotaskstream.restype  =   ctypes.c_int
__library__.MSK_XX_linkfunctoenvstream.argtypes  = [ ctypes.c_void_p,    # env 
                                                  ctypes.c_int,       # whichstream
                                                  ctypes.c_void_p,    # handle
                                                  __stream_cb_type__ ] # func
__library__.MSK_XX_linkfunctoenvstream.restype   =   ctypes.c_int
__library__.MSK_XX_clonetask.restype     = ctypes.c_int
__library__.MSK_XX_clonetask.argtypes    = [ ctypes.c_void_p, ctypes.POINTER(ctypes.c_void_p) ]
__library__.MSK_XX_getlasterror64.restype  = ctypes.c_int
__library__.MSK_XX_getlasterror64.argtypes = [ ctypes.c_void_p, # task
                                               ctypes.POINTER(ctypes.c_int), # lastrescode
                                               ctypes.c_int64, # maxlen
                                               ctypes.POINTER(ctypes.c_size_t), # msg len
                                               ctypes.c_char_p, ] # msg
__library__.MSK_XX_putresponsefunc.restype  = ctypes.c_int
__library__.MSK_XX_putresponsefunc.argtypes = [ ctypes.c_void_p,  # task
                                             ctypes.c_void_p,  # func
                                             ctypes.c_void_p ] # handle
__library__.MSK_XX_enablegarcolenv.argtypes = [ ctypes.c_void_p ] 
__library__.MSK_XX_enablegarcolenv.restype  =   ctypes.c_int 

__library__.MSK_XX_freeenv.restype  = None
__library__.MSK_XX_freeenv.argtypes  = [ ctypes.c_void_p, ctypes.c_void_p ]

__library__.MSK_XX_freetask.restype  = None
__library__.MSK_XX_freetask.argtypes = [ ctypes.c_void_p, ctypes.c_void_p ]

# from fusionsort.c

__library__.MSK_bucketsort__3JSS_3III_3V.restype = None
__library__.MSK_bucketsort__3JSS_3III_3V.argtypes = [ ctypes.POINTER(ctypes.c_longlong),
                                                      ctypes.c_size_t,
                                                      ctypes.c_size_t,
                                                      ctypes.POINTER(ctypes.c_int),
                                                      ctypes.c_int,
                                                      ctypes.c_int,
                                                      ctypes.c_void_p ]  
__library__.MSK_bucketsort__3JSS_3JJJ_3V.restype = None
__library__.MSK_bucketsort__3JSS_3JJJ_3V.argtypes = [ ctypes.POINTER(ctypes.c_longlong),
                                                      ctypes.c_size_t,
                                                      ctypes.c_size_t,
                                                      ctypes.POINTER(ctypes.c_longlong),
                                                      ctypes.c_longlong,
                                                      ctypes.c_longlong,
                                                      ctypes.c_void_p ]  

__library__.MSK_XX_analyzeproblem.restype  = ctypes.c_int
__library__.MSK_XX_analyzeproblem.argtypes = [ctypes.c_void_p,ctypes.c_int32]
__library__.MSK_XX_analyzenames.restype  = ctypes.c_int
__library__.MSK_XX_analyzenames.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32]
__library__.MSK_XX_analyzesolution.restype  = ctypes.c_int
__library__.MSK_XX_analyzesolution.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32]
__library__.MSK_XX_initbasissolve.restype  = ctypes.c_int
__library__.MSK_XX_initbasissolve.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_solvewithbasis.restype  = ctypes.c_int
__library__.MSK_XX_solvewithbasis.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_basiscond.restype  = ctypes.c_int
__library__.MSK_XX_basiscond.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_appendcons.restype  = ctypes.c_int
__library__.MSK_XX_appendcons.argtypes = [ctypes.c_void_p,ctypes.c_int32]
__library__.MSK_XX_appendvars.restype  = ctypes.c_int
__library__.MSK_XX_appendvars.argtypes = [ctypes.c_void_p,ctypes.c_int32]
__library__.MSK_XX_removecons.restype  = ctypes.c_int
__library__.MSK_XX_removecons.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_removevars.restype  = ctypes.c_int
__library__.MSK_XX_removevars.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_removebarvars.restype  = ctypes.c_int
__library__.MSK_XX_removebarvars.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_removecones.restype  = ctypes.c_int
__library__.MSK_XX_removecones.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_appendbarvars.restype  = ctypes.c_int
__library__.MSK_XX_appendbarvars.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_appendcone.restype  = ctypes.c_int
__library__.MSK_XX_appendcone.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_double,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_appendconeseq.restype  = ctypes.c_int
__library__.MSK_XX_appendconeseq.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_double,ctypes.c_int32,ctypes.c_int32]
__library__.MSK_XX_appendconesseq.restype  = ctypes.c_int
__library__.MSK_XX_appendconesseq.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_int32),ctypes.c_int32]
__library__.MSK_XX_chgconbound.restype  = ctypes.c_int
__library__.MSK_XX_chgconbound.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.c_double]
__library__.MSK_XX_chgvarbound.restype  = ctypes.c_int
__library__.MSK_XX_chgvarbound.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.c_double]
__library__.MSK_XX_getaij.restype  = ctypes.c_int
__library__.MSK_XX_getaij.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getapiecenumnz.restype  = ctypes.c_int
__library__.MSK_XX_getapiecenumnz.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getacolnumnz.restype  = ctypes.c_int
__library__.MSK_XX_getacolnumnz.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getacol.restype  = ctypes.c_int
__library__.MSK_XX_getacol.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getacolslice64.restype  = ctypes.c_int
__library__.MSK_XX_getacolslice64.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int64,ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getarownumnz.restype  = ctypes.c_int
__library__.MSK_XX_getarownumnz.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getarow.restype  = ctypes.c_int
__library__.MSK_XX_getarow.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getacolslicenumnz64.restype  = ctypes.c_int
__library__.MSK_XX_getacolslicenumnz64.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_getarowslicenumnz64.restype  = ctypes.c_int
__library__.MSK_XX_getarowslicenumnz64.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_getarowslice64.restype  = ctypes.c_int
__library__.MSK_XX_getarowslice64.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int64,ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getarowslicetrip.restype  = ctypes.c_int
__library__.MSK_XX_getarowslicetrip.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int64,ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getacolslicetrip.restype  = ctypes.c_int
__library__.MSK_XX_getacolslicetrip.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int64,ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getconbound.restype  = ctypes.c_int
__library__.MSK_XX_getconbound.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getvarbound.restype  = ctypes.c_int
__library__.MSK_XX_getvarbound.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getconboundslice.restype  = ctypes.c_int
__library__.MSK_XX_getconboundslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getvarboundslice.restype  = ctypes.c_int
__library__.MSK_XX_getvarboundslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getcj.restype  = ctypes.c_int
__library__.MSK_XX_getcj.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getc.restype  = ctypes.c_int
__library__.MSK_XX_getc.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getcfix.restype  = ctypes.c_int
__library__.MSK_XX_getcfix.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getcone.restype  = ctypes.c_int
__library__.MSK_XX_getcone.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getconeinfo.restype  = ctypes.c_int
__library__.MSK_XX_getconeinfo.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getclist.restype  = ctypes.c_int
__library__.MSK_XX_getclist.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getcslice.restype  = ctypes.c_int
__library__.MSK_XX_getcslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getdouinf.restype  = ctypes.c_int
__library__.MSK_XX_getdouinf.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getdouparam.restype  = ctypes.c_int
__library__.MSK_XX_getdouparam.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getdualobj.restype  = ctypes.c_int
__library__.MSK_XX_getdualobj.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getintinf.restype  = ctypes.c_int
__library__.MSK_XX_getintinf.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getlintinf.restype  = ctypes.c_int
__library__.MSK_XX_getlintinf.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_getintparam.restype  = ctypes.c_int
__library__.MSK_XX_getintparam.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getmaxnumanz64.restype  = ctypes.c_int
__library__.MSK_XX_getmaxnumanz64.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_getmaxnumcon.restype  = ctypes.c_int
__library__.MSK_XX_getmaxnumcon.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getmaxnumvar.restype  = ctypes.c_int
__library__.MSK_XX_getmaxnumvar.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getbarvarnamelen.restype  = ctypes.c_int
__library__.MSK_XX_getbarvarnamelen.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getbarvarname.restype  = ctypes.c_int
__library__.MSK_XX_getbarvarname.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_char_p]
__library__.MSK_XX_getbarvarnameindex.restype  = ctypes.c_int
__library__.MSK_XX_getbarvarnameindex.argtypes = [ctypes.c_void_p,ctypes.c_char_p,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_generatevarnames.restype  = ctypes.c_int
__library__.MSK_XX_generatevarnames.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.c_char_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_generateconnames.restype  = ctypes.c_int
__library__.MSK_XX_generateconnames.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.c_char_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_generateconenames.restype  = ctypes.c_int
__library__.MSK_XX_generateconenames.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.c_char_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_putconname.restype  = ctypes.c_int
__library__.MSK_XX_putconname.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_char_p]
__library__.MSK_XX_putvarname.restype  = ctypes.c_int
__library__.MSK_XX_putvarname.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_char_p]
__library__.MSK_XX_putconename.restype  = ctypes.c_int
__library__.MSK_XX_putconename.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_char_p]
__library__.MSK_XX_putbarvarname.restype  = ctypes.c_int
__library__.MSK_XX_putbarvarname.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_char_p]
__library__.MSK_XX_getvarnamelen.restype  = ctypes.c_int
__library__.MSK_XX_getvarnamelen.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getvarname.restype  = ctypes.c_int
__library__.MSK_XX_getvarname.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_char_p]
__library__.MSK_XX_getconnamelen.restype  = ctypes.c_int
__library__.MSK_XX_getconnamelen.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getconname.restype  = ctypes.c_int
__library__.MSK_XX_getconname.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_char_p]
__library__.MSK_XX_getconnameindex.restype  = ctypes.c_int
__library__.MSK_XX_getconnameindex.argtypes = [ctypes.c_void_p,ctypes.c_char_p,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getvarnameindex.restype  = ctypes.c_int
__library__.MSK_XX_getvarnameindex.argtypes = [ctypes.c_void_p,ctypes.c_char_p,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getconenamelen.restype  = ctypes.c_int
__library__.MSK_XX_getconenamelen.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getconename.restype  = ctypes.c_int
__library__.MSK_XX_getconename.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_char_p]
__library__.MSK_XX_getconenameindex.restype  = ctypes.c_int
__library__.MSK_XX_getconenameindex.argtypes = [ctypes.c_void_p,ctypes.c_char_p,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getnumanz.restype  = ctypes.c_int
__library__.MSK_XX_getnumanz.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getnumanz64.restype  = ctypes.c_int
__library__.MSK_XX_getnumanz64.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_getnumcon.restype  = ctypes.c_int
__library__.MSK_XX_getnumcon.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getnumcone.restype  = ctypes.c_int
__library__.MSK_XX_getnumcone.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getnumconemem.restype  = ctypes.c_int
__library__.MSK_XX_getnumconemem.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getnumintvar.restype  = ctypes.c_int
__library__.MSK_XX_getnumintvar.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getnumparam.restype  = ctypes.c_int
__library__.MSK_XX_getnumparam.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getnumqconknz64.restype  = ctypes.c_int
__library__.MSK_XX_getnumqconknz64.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_getnumqobjnz64.restype  = ctypes.c_int
__library__.MSK_XX_getnumqobjnz64.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_getnumvar.restype  = ctypes.c_int
__library__.MSK_XX_getnumvar.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getnumbarvar.restype  = ctypes.c_int
__library__.MSK_XX_getnumbarvar.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getmaxnumbarvar.restype  = ctypes.c_int
__library__.MSK_XX_getmaxnumbarvar.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getdimbarvarj.restype  = ctypes.c_int
__library__.MSK_XX_getdimbarvarj.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getlenbarvarj.restype  = ctypes.c_int
__library__.MSK_XX_getlenbarvarj.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_getobjname.restype  = ctypes.c_int
__library__.MSK_XX_getobjname.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_char_p]
__library__.MSK_XX_getobjnamelen.restype  = ctypes.c_int
__library__.MSK_XX_getobjnamelen.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getprimalobj.restype  = ctypes.c_int
__library__.MSK_XX_getprimalobj.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getprobtype.restype  = ctypes.c_int
__library__.MSK_XX_getprobtype.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getqconk64.restype  = ctypes.c_int
__library__.MSK_XX_getqconk64.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int64,ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getqobj64.restype  = ctypes.c_int
__library__.MSK_XX_getqobj64.argtypes = [ctypes.c_void_p,ctypes.c_int64,ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getqobjij.restype  = ctypes.c_int
__library__.MSK_XX_getqobjij.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getsolution.restype  = ctypes.c_int
__library__.MSK_XX_getsolution.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int),ctypes.POINTER(ctypes.c_int),ctypes.POINTER(ctypes.c_int),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getsolsta.restype  = ctypes.c_int
__library__.MSK_XX_getsolsta.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getprosta.restype  = ctypes.c_int
__library__.MSK_XX_getprosta.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getskc.restype  = ctypes.c_int
__library__.MSK_XX_getskc.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int)]
__library__.MSK_XX_getskx.restype  = ctypes.c_int
__library__.MSK_XX_getskx.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int)]
__library__.MSK_XX_getskn.restype  = ctypes.c_int
__library__.MSK_XX_getskn.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int)]
__library__.MSK_XX_getxc.restype  = ctypes.c_int
__library__.MSK_XX_getxc.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getxx.restype  = ctypes.c_int
__library__.MSK_XX_getxx.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_gety.restype  = ctypes.c_int
__library__.MSK_XX_gety.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getslc.restype  = ctypes.c_int
__library__.MSK_XX_getslc.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getsuc.restype  = ctypes.c_int
__library__.MSK_XX_getsuc.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getslx.restype  = ctypes.c_int
__library__.MSK_XX_getslx.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getsux.restype  = ctypes.c_int
__library__.MSK_XX_getsux.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getsnx.restype  = ctypes.c_int
__library__.MSK_XX_getsnx.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getskcslice.restype  = ctypes.c_int
__library__.MSK_XX_getskcslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int)]
__library__.MSK_XX_getskxslice.restype  = ctypes.c_int
__library__.MSK_XX_getskxslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int)]
__library__.MSK_XX_getxcslice.restype  = ctypes.c_int
__library__.MSK_XX_getxcslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getxxslice.restype  = ctypes.c_int
__library__.MSK_XX_getxxslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getyslice.restype  = ctypes.c_int
__library__.MSK_XX_getyslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getslcslice.restype  = ctypes.c_int
__library__.MSK_XX_getslcslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getsucslice.restype  = ctypes.c_int
__library__.MSK_XX_getsucslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getslxslice.restype  = ctypes.c_int
__library__.MSK_XX_getslxslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getsuxslice.restype  = ctypes.c_int
__library__.MSK_XX_getsuxslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getsnxslice.restype  = ctypes.c_int
__library__.MSK_XX_getsnxslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getbarxj.restype  = ctypes.c_int
__library__.MSK_XX_getbarxj.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getbarxslice.restype  = ctypes.c_int
__library__.MSK_XX_getbarxslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.c_int64,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getbarsj.restype  = ctypes.c_int
__library__.MSK_XX_getbarsj.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getbarsslice.restype  = ctypes.c_int
__library__.MSK_XX_getbarsslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.c_int64,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putskc.restype  = ctypes.c_int
__library__.MSK_XX_putskc.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int)]
__library__.MSK_XX_putskx.restype  = ctypes.c_int
__library__.MSK_XX_putskx.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int)]
__library__.MSK_XX_putxc.restype  = ctypes.c_int
__library__.MSK_XX_putxc.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putxx.restype  = ctypes.c_int
__library__.MSK_XX_putxx.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_puty.restype  = ctypes.c_int
__library__.MSK_XX_puty.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putslc.restype  = ctypes.c_int
__library__.MSK_XX_putslc.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putsuc.restype  = ctypes.c_int
__library__.MSK_XX_putsuc.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putslx.restype  = ctypes.c_int
__library__.MSK_XX_putslx.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putsux.restype  = ctypes.c_int
__library__.MSK_XX_putsux.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putsnx.restype  = ctypes.c_int
__library__.MSK_XX_putsnx.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putskcslice.restype  = ctypes.c_int
__library__.MSK_XX_putskcslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int)]
__library__.MSK_XX_putskxslice.restype  = ctypes.c_int
__library__.MSK_XX_putskxslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int)]
__library__.MSK_XX_putxcslice.restype  = ctypes.c_int
__library__.MSK_XX_putxcslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putxxslice.restype  = ctypes.c_int
__library__.MSK_XX_putxxslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putyslice.restype  = ctypes.c_int
__library__.MSK_XX_putyslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putslcslice.restype  = ctypes.c_int
__library__.MSK_XX_putslcslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putsucslice.restype  = ctypes.c_int
__library__.MSK_XX_putsucslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putslxslice.restype  = ctypes.c_int
__library__.MSK_XX_putslxslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putsuxslice.restype  = ctypes.c_int
__library__.MSK_XX_putsuxslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putsnxslice.restype  = ctypes.c_int
__library__.MSK_XX_putsnxslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putbarxj.restype  = ctypes.c_int
__library__.MSK_XX_putbarxj.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putbarsj.restype  = ctypes.c_int
__library__.MSK_XX_putbarsj.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getpviolcon.restype  = ctypes.c_int
__library__.MSK_XX_getpviolcon.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getpviolvar.restype  = ctypes.c_int
__library__.MSK_XX_getpviolvar.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getpviolbarvar.restype  = ctypes.c_int
__library__.MSK_XX_getpviolbarvar.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getpviolcones.restype  = ctypes.c_int
__library__.MSK_XX_getpviolcones.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getdviolcon.restype  = ctypes.c_int
__library__.MSK_XX_getdviolcon.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getdviolvar.restype  = ctypes.c_int
__library__.MSK_XX_getdviolvar.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getdviolbarvar.restype  = ctypes.c_int
__library__.MSK_XX_getdviolbarvar.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getdviolcones.restype  = ctypes.c_int
__library__.MSK_XX_getdviolcones.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getsolutioninfo.restype  = ctypes.c_int
__library__.MSK_XX_getsolutioninfo.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getdualsolutionnorms.restype  = ctypes.c_int
__library__.MSK_XX_getdualsolutionnorms.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getprimalsolutionnorms.restype  = ctypes.c_int
__library__.MSK_XX_getprimalsolutionnorms.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getsolutionslice.restype  = ctypes.c_int
__library__.MSK_XX_getsolutionslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getreducedcosts.restype  = ctypes.c_int
__library__.MSK_XX_getreducedcosts.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getstrparam.restype  = ctypes.c_int
__library__.MSK_XX_getstrparam.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.c_char_p]
__library__.MSK_XX_getstrparamlen.restype  = ctypes.c_int
__library__.MSK_XX_getstrparamlen.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_gettasknamelen.restype  = ctypes.c_int
__library__.MSK_XX_gettasknamelen.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_gettaskname.restype  = ctypes.c_int
__library__.MSK_XX_gettaskname.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_char_p]
__library__.MSK_XX_getvartype.restype  = ctypes.c_int
__library__.MSK_XX_getvartype.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getvartypelist.restype  = ctypes.c_int
__library__.MSK_XX_getvartypelist.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int)]
__library__.MSK_XX_inputdata64.restype  = ctypes.c_int
__library__.MSK_XX_inputdata64.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double),ctypes.c_double,ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_int),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_int),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_isdouparname.restype  = ctypes.c_int
__library__.MSK_XX_isdouparname.argtypes = [ctypes.c_void_p,ctypes.c_char_p,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_isintparname.restype  = ctypes.c_int
__library__.MSK_XX_isintparname.argtypes = [ctypes.c_void_p,ctypes.c_char_p,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_isstrparname.restype  = ctypes.c_int
__library__.MSK_XX_isstrparname.argtypes = [ctypes.c_void_p,ctypes.c_char_p,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_linkfiletotaskstream.restype  = ctypes.c_int
__library__.MSK_XX_linkfiletotaskstream.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_char_p,ctypes.c_int32]
__library__.MSK_XX_primalrepair.restype  = ctypes.c_int
__library__.MSK_XX_primalrepair.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_toconic.restype  = ctypes.c_int
__library__.MSK_XX_toconic.argtypes = [ctypes.c_void_p]
__library__.MSK_XX_optimizetrm.restype  = ctypes.c_int
__library__.MSK_XX_optimizetrm.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_commitchanges.restype  = ctypes.c_int
__library__.MSK_XX_commitchanges.argtypes = [ctypes.c_void_p]
__library__.MSK_XX_getatruncatetol.restype  = ctypes.c_int
__library__.MSK_XX_getatruncatetol.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putatruncatetol.restype  = ctypes.c_int
__library__.MSK_XX_putatruncatetol.argtypes = [ctypes.c_void_p,ctypes.c_double]
__library__.MSK_XX_putaij.restype  = ctypes.c_int
__library__.MSK_XX_putaij.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_double]
__library__.MSK_XX_putaijlist64.restype  = ctypes.c_int
__library__.MSK_XX_putaijlist64.argtypes = [ctypes.c_void_p,ctypes.c_int64,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putacol.restype  = ctypes.c_int
__library__.MSK_XX_putacol.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putarow.restype  = ctypes.c_int
__library__.MSK_XX_putarow.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putarowslice64.restype  = ctypes.c_int
__library__.MSK_XX_putarowslice64.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putarowlist64.restype  = ctypes.c_int
__library__.MSK_XX_putarowlist64.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putacolslice64.restype  = ctypes.c_int
__library__.MSK_XX_putacolslice64.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putacollist64.restype  = ctypes.c_int
__library__.MSK_XX_putacollist64.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putbaraij.restype  = ctypes.c_int
__library__.MSK_XX_putbaraij.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int64,ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putbaraijlist.restype  = ctypes.c_int
__library__.MSK_XX_putbaraijlist.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putbararowlist.restype  = ctypes.c_int
__library__.MSK_XX_putbararowlist.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getnumbarcnz.restype  = ctypes.c_int
__library__.MSK_XX_getnumbarcnz.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_getnumbaranz.restype  = ctypes.c_int
__library__.MSK_XX_getnumbaranz.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_getbarcsparsity.restype  = ctypes.c_int
__library__.MSK_XX_getbarcsparsity.argtypes = [ctypes.c_void_p,ctypes.c_int64,ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_getbarasparsity.restype  = ctypes.c_int
__library__.MSK_XX_getbarasparsity.argtypes = [ctypes.c_void_p,ctypes.c_int64,ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_getbarcidxinfo.restype  = ctypes.c_int
__library__.MSK_XX_getbarcidxinfo.argtypes = [ctypes.c_void_p,ctypes.c_int64,ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_getbarcidxj.restype  = ctypes.c_int
__library__.MSK_XX_getbarcidxj.argtypes = [ctypes.c_void_p,ctypes.c_int64,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getbarcidx.restype  = ctypes.c_int
__library__.MSK_XX_getbarcidx.argtypes = [ctypes.c_void_p,ctypes.c_int64,ctypes.c_int64,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getbaraidxinfo.restype  = ctypes.c_int
__library__.MSK_XX_getbaraidxinfo.argtypes = [ctypes.c_void_p,ctypes.c_int64,ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_getbaraidxij.restype  = ctypes.c_int
__library__.MSK_XX_getbaraidxij.argtypes = [ctypes.c_void_p,ctypes.c_int64,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getbaraidx.restype  = ctypes.c_int
__library__.MSK_XX_getbaraidx.argtypes = [ctypes.c_void_p,ctypes.c_int64,ctypes.c_int64,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getnumbarcblocktriplets.restype  = ctypes.c_int
__library__.MSK_XX_getnumbarcblocktriplets.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_putbarcblocktriplet.restype  = ctypes.c_int
__library__.MSK_XX_putbarcblocktriplet.argtypes = [ctypes.c_void_p,ctypes.c_int64,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getbarcblocktriplet.restype  = ctypes.c_int
__library__.MSK_XX_getbarcblocktriplet.argtypes = [ctypes.c_void_p,ctypes.c_int64,ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putbarablocktriplet.restype  = ctypes.c_int
__library__.MSK_XX_putbarablocktriplet.argtypes = [ctypes.c_void_p,ctypes.c_int64,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_getnumbarablocktriplets.restype  = ctypes.c_int
__library__.MSK_XX_getnumbarablocktriplets.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_getbarablocktriplet.restype  = ctypes.c_int
__library__.MSK_XX_getbarablocktriplet.argtypes = [ctypes.c_void_p,ctypes.c_int64,ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putconbound.restype  = ctypes.c_int
__library__.MSK_XX_putconbound.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_double,ctypes.c_double]
__library__.MSK_XX_putconboundlist.restype  = ctypes.c_int
__library__.MSK_XX_putconboundlist.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putconboundlistconst.restype  = ctypes.c_int
__library__.MSK_XX_putconboundlistconst.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.c_int32,ctypes.c_double,ctypes.c_double]
__library__.MSK_XX_putconboundslice.restype  = ctypes.c_int
__library__.MSK_XX_putconboundslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putconboundsliceconst.restype  = ctypes.c_int
__library__.MSK_XX_putconboundsliceconst.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.c_double,ctypes.c_double]
__library__.MSK_XX_putvarbound.restype  = ctypes.c_int
__library__.MSK_XX_putvarbound.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_double,ctypes.c_double]
__library__.MSK_XX_putvarboundlist.restype  = ctypes.c_int
__library__.MSK_XX_putvarboundlist.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putvarboundlistconst.restype  = ctypes.c_int
__library__.MSK_XX_putvarboundlistconst.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.c_int32,ctypes.c_double,ctypes.c_double]
__library__.MSK_XX_putvarboundslice.restype  = ctypes.c_int
__library__.MSK_XX_putvarboundslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putvarboundsliceconst.restype  = ctypes.c_int
__library__.MSK_XX_putvarboundsliceconst.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.c_double,ctypes.c_double]
__library__.MSK_XX_putcfix.restype  = ctypes.c_int
__library__.MSK_XX_putcfix.argtypes = [ctypes.c_void_p,ctypes.c_double]
__library__.MSK_XX_putcj.restype  = ctypes.c_int
__library__.MSK_XX_putcj.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_double]
__library__.MSK_XX_putobjsense.restype  = ctypes.c_int
__library__.MSK_XX_putobjsense.argtypes = [ctypes.c_void_p,ctypes.c_int32]
__library__.MSK_XX_getobjsense.restype  = ctypes.c_int
__library__.MSK_XX_getobjsense.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_putclist.restype  = ctypes.c_int
__library__.MSK_XX_putclist.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putcslice.restype  = ctypes.c_int
__library__.MSK_XX_putcslice.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putbarcj.restype  = ctypes.c_int
__library__.MSK_XX_putbarcj.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int64,ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putcone.restype  = ctypes.c_int
__library__.MSK_XX_putcone.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_double,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_appendsparsesymmat.restype  = ctypes.c_int
__library__.MSK_XX_appendsparsesymmat.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int64,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_appendsparsesymmatlist.restype  = ctypes.c_int
__library__.MSK_XX_appendsparsesymmatlist.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_getsymmatinfo.restype  = ctypes.c_int
__library__.MSK_XX_getsymmatinfo.argtypes = [ctypes.c_void_p,ctypes.c_int64,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_getnumsymmat.restype  = ctypes.c_int
__library__.MSK_XX_getnumsymmat.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_getsparsesymmat.restype  = ctypes.c_int
__library__.MSK_XX_getsparsesymmat.argtypes = [ctypes.c_void_p,ctypes.c_int64,ctypes.c_int64,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putdouparam.restype  = ctypes.c_int
__library__.MSK_XX_putdouparam.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_double]
__library__.MSK_XX_putintparam.restype  = ctypes.c_int
__library__.MSK_XX_putintparam.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32]
__library__.MSK_XX_putmaxnumcon.restype  = ctypes.c_int
__library__.MSK_XX_putmaxnumcon.argtypes = [ctypes.c_void_p,ctypes.c_int32]
__library__.MSK_XX_putmaxnumcone.restype  = ctypes.c_int
__library__.MSK_XX_putmaxnumcone.argtypes = [ctypes.c_void_p,ctypes.c_int32]
__library__.MSK_XX_getmaxnumcone.restype  = ctypes.c_int
__library__.MSK_XX_getmaxnumcone.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_putmaxnumvar.restype  = ctypes.c_int
__library__.MSK_XX_putmaxnumvar.argtypes = [ctypes.c_void_p,ctypes.c_int32]
__library__.MSK_XX_putmaxnumbarvar.restype  = ctypes.c_int
__library__.MSK_XX_putmaxnumbarvar.argtypes = [ctypes.c_void_p,ctypes.c_int32]
__library__.MSK_XX_putmaxnumanz.restype  = ctypes.c_int
__library__.MSK_XX_putmaxnumanz.argtypes = [ctypes.c_void_p,ctypes.c_int64]
__library__.MSK_XX_putmaxnumqnz.restype  = ctypes.c_int
__library__.MSK_XX_putmaxnumqnz.argtypes = [ctypes.c_void_p,ctypes.c_int64]
__library__.MSK_XX_getmaxnumqnz64.restype  = ctypes.c_int
__library__.MSK_XX_getmaxnumqnz64.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_putnadouparam.restype  = ctypes.c_int
__library__.MSK_XX_putnadouparam.argtypes = [ctypes.c_void_p,ctypes.c_char_p,ctypes.c_double]
__library__.MSK_XX_putnaintparam.restype  = ctypes.c_int
__library__.MSK_XX_putnaintparam.argtypes = [ctypes.c_void_p,ctypes.c_char_p,ctypes.c_int32]
__library__.MSK_XX_putnastrparam.restype  = ctypes.c_int
__library__.MSK_XX_putnastrparam.argtypes = [ctypes.c_void_p,ctypes.c_char_p,ctypes.c_char_p]
__library__.MSK_XX_putobjname.restype  = ctypes.c_int
__library__.MSK_XX_putobjname.argtypes = [ctypes.c_void_p,ctypes.c_char_p]
__library__.MSK_XX_putparam.restype  = ctypes.c_int
__library__.MSK_XX_putparam.argtypes = [ctypes.c_void_p,ctypes.c_char_p,ctypes.c_char_p]
__library__.MSK_XX_putqcon.restype  = ctypes.c_int
__library__.MSK_XX_putqcon.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putqconk.restype  = ctypes.c_int
__library__.MSK_XX_putqconk.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putqobj.restype  = ctypes.c_int
__library__.MSK_XX_putqobj.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putqobjij.restype  = ctypes.c_int
__library__.MSK_XX_putqobjij.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_double]
__library__.MSK_XX_putsolution.restype  = ctypes.c_int
__library__.MSK_XX_putsolution.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int),ctypes.POINTER(ctypes.c_int),ctypes.POINTER(ctypes.c_int),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_putconsolutioni.restype  = ctypes.c_int
__library__.MSK_XX_putconsolutioni.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.c_double,ctypes.c_double,ctypes.c_double]
__library__.MSK_XX_putvarsolutionj.restype  = ctypes.c_int
__library__.MSK_XX_putvarsolutionj.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.c_double,ctypes.c_double,ctypes.c_double,ctypes.c_double]
__library__.MSK_XX_putsolutionyi.restype  = ctypes.c_int
__library__.MSK_XX_putsolutionyi.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_double]
__library__.MSK_XX_putstrparam.restype  = ctypes.c_int
__library__.MSK_XX_putstrparam.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_char_p]
__library__.MSK_XX_puttaskname.restype  = ctypes.c_int
__library__.MSK_XX_puttaskname.argtypes = [ctypes.c_void_p,ctypes.c_char_p]
__library__.MSK_XX_putvartype.restype  = ctypes.c_int
__library__.MSK_XX_putvartype.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32]
__library__.MSK_XX_putvartypelist.restype  = ctypes.c_int
__library__.MSK_XX_putvartypelist.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int)]
__library__.MSK_XX_readdataformat.restype  = ctypes.c_int
__library__.MSK_XX_readdataformat.argtypes = [ctypes.c_void_p,ctypes.c_char_p,ctypes.c_int32,ctypes.c_int32]
__library__.MSK_XX_readdataautoformat.restype  = ctypes.c_int
__library__.MSK_XX_readdataautoformat.argtypes = [ctypes.c_void_p,ctypes.c_char_p]
__library__.MSK_XX_readparamfile.restype  = ctypes.c_int
__library__.MSK_XX_readparamfile.argtypes = [ctypes.c_void_p,ctypes.c_char_p]
__library__.MSK_XX_readsolution.restype  = ctypes.c_int
__library__.MSK_XX_readsolution.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_char_p]
__library__.MSK_XX_readsummary.restype  = ctypes.c_int
__library__.MSK_XX_readsummary.argtypes = [ctypes.c_void_p,ctypes.c_int32]
__library__.MSK_XX_resizetask.restype  = ctypes.c_int
__library__.MSK_XX_resizetask.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.c_int64,ctypes.c_int64]
__library__.MSK_XX_checkmemtask.restype  = ctypes.c_int
__library__.MSK_XX_checkmemtask.argtypes = [ctypes.c_void_p,ctypes.c_char_p,ctypes.c_int32]
__library__.MSK_XX_getmemusagetask.restype  = ctypes.c_int
__library__.MSK_XX_getmemusagetask.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int64)]
__library__.MSK_XX_setdefaults.restype  = ctypes.c_int
__library__.MSK_XX_setdefaults.argtypes = [ctypes.c_void_p]
__library__.MSK_XX_solutiondef.restype  = ctypes.c_int
__library__.MSK_XX_solutiondef.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_deletesolution.restype  = ctypes.c_int
__library__.MSK_XX_deletesolution.argtypes = [ctypes.c_void_p,ctypes.c_int32]
__library__.MSK_XX_onesolutionsummary.restype  = ctypes.c_int
__library__.MSK_XX_onesolutionsummary.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32]
__library__.MSK_XX_solutionsummary.restype  = ctypes.c_int
__library__.MSK_XX_solutionsummary.argtypes = [ctypes.c_void_p,ctypes.c_int32]
__library__.MSK_XX_updatesolutioninfo.restype  = ctypes.c_int
__library__.MSK_XX_updatesolutioninfo.argtypes = [ctypes.c_void_p,ctypes.c_int32]
__library__.MSK_XX_optimizersummary.restype  = ctypes.c_int
__library__.MSK_XX_optimizersummary.argtypes = [ctypes.c_void_p,ctypes.c_int32]
__library__.MSK_XX_strtoconetype.restype  = ctypes.c_int
__library__.MSK_XX_strtoconetype.argtypes = [ctypes.c_void_p,ctypes.c_char_p,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_strtosk.restype  = ctypes.c_int
__library__.MSK_XX_strtosk.argtypes = [ctypes.c_void_p,ctypes.c_char_p,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_writedata.restype  = ctypes.c_int
__library__.MSK_XX_writedata.argtypes = [ctypes.c_void_p,ctypes.c_char_p]
__library__.MSK_XX_writetask.restype  = ctypes.c_int
__library__.MSK_XX_writetask.argtypes = [ctypes.c_void_p,ctypes.c_char_p]
__library__.MSK_XX_readtask.restype  = ctypes.c_int
__library__.MSK_XX_readtask.argtypes = [ctypes.c_void_p,ctypes.c_char_p]
__library__.MSK_XX_readopfstring.restype  = ctypes.c_int
__library__.MSK_XX_readopfstring.argtypes = [ctypes.c_void_p,ctypes.c_char_p]
__library__.MSK_XX_readlpstring.restype  = ctypes.c_int
__library__.MSK_XX_readlpstring.argtypes = [ctypes.c_void_p,ctypes.c_char_p]
__library__.MSK_XX_readjsonstring.restype  = ctypes.c_int
__library__.MSK_XX_readjsonstring.argtypes = [ctypes.c_void_p,ctypes.c_char_p]
__library__.MSK_XX_readptfstring.restype  = ctypes.c_int
__library__.MSK_XX_readptfstring.argtypes = [ctypes.c_void_p,ctypes.c_char_p]
__library__.MSK_XX_writeparamfile.restype  = ctypes.c_int
__library__.MSK_XX_writeparamfile.argtypes = [ctypes.c_void_p,ctypes.c_char_p]
__library__.MSK_XX_getinfeasiblesubproblem.restype  = ctypes.c_int
__library__.MSK_XX_getinfeasiblesubproblem.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_void_p)]
__library__.MSK_XX_writesolution.restype  = ctypes.c_int
__library__.MSK_XX_writesolution.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_char_p]
__library__.MSK_XX_writejsonsol.restype  = ctypes.c_int
__library__.MSK_XX_writejsonsol.argtypes = [ctypes.c_void_p,ctypes.c_char_p]
__library__.MSK_XX_primalsensitivity.restype  = ctypes.c_int
__library__.MSK_XX_primalsensitivity.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int),ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_sensitivityreport.restype  = ctypes.c_int
__library__.MSK_XX_sensitivityreport.argtypes = [ctypes.c_void_p,ctypes.c_int32]
__library__.MSK_XX_dualsensitivity.restype  = ctypes.c_int
__library__.MSK_XX_dualsensitivity.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_optimizermt.restype  = ctypes.c_int
__library__.MSK_XX_optimizermt.argtypes = [ctypes.c_void_p,ctypes.c_char_p,ctypes.c_char_p,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_asyncoptimize.restype  = ctypes.c_int
__library__.MSK_XX_asyncoptimize.argtypes = [ctypes.c_void_p,ctypes.c_char_p,ctypes.c_char_p,ctypes.c_char_p]
__library__.MSK_XX_asyncstop.restype  = ctypes.c_int
__library__.MSK_XX_asyncstop.argtypes = [ctypes.c_void_p,ctypes.c_char_p,ctypes.c_char_p,ctypes.c_char_p]
__library__.MSK_XX_asyncpoll.restype  = ctypes.c_int
__library__.MSK_XX_asyncpoll.argtypes = [ctypes.c_void_p,ctypes.c_char_p,ctypes.c_char_p,ctypes.c_char_p,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_asyncgetresult.restype  = ctypes.c_int
__library__.MSK_XX_asyncgetresult.argtypes = [ctypes.c_void_p,ctypes.c_char_p,ctypes.c_char_p,ctypes.c_char_p,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_checkoutlicense.restype  = ctypes.c_int
__library__.MSK_XX_checkoutlicense.argtypes = [ctypes.c_void_p,ctypes.c_int32]
__library__.MSK_XX_checkinlicense.restype  = ctypes.c_int
__library__.MSK_XX_checkinlicense.argtypes = [ctypes.c_void_p,ctypes.c_int32]
__library__.MSK_XX_checkinall.restype  = ctypes.c_int
__library__.MSK_XX_checkinall.argtypes = [ctypes.c_void_p]
__library__.MSK_XX_setupthreads.restype  = ctypes.c_int
__library__.MSK_XX_setupthreads.argtypes = [ctypes.c_void_p,ctypes.c_int32]
__library__.MSK_XX_echointro.restype  = ctypes.c_int
__library__.MSK_XX_echointro.argtypes = [ctypes.c_void_p,ctypes.c_int32]
__library__.MSK_XX_getcodedesc.restype  = ctypes.c_int
__library__.MSK_XX_getcodedesc.argtypes = [ctypes.c_int32,ctypes.c_char_p,ctypes.c_char_p]
__library__.MSK_XX_getversion.restype  = ctypes.c_int
__library__.MSK_XX_getversion.argtypes = [ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_linkfiletoenvstream.restype  = ctypes.c_int
__library__.MSK_XX_linkfiletoenvstream.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_char_p,ctypes.c_int32]
__library__.MSK_XX_putlicensedebug.restype  = ctypes.c_int
__library__.MSK_XX_putlicensedebug.argtypes = [ctypes.c_void_p,ctypes.c_int32]
__library__.MSK_XX_putlicensecode.restype  = ctypes.c_int
__library__.MSK_XX_putlicensecode.argtypes = [ctypes.c_void_p,ctypes.POINTER(ctypes.c_int32)]
__library__.MSK_XX_putlicensewait.restype  = ctypes.c_int
__library__.MSK_XX_putlicensewait.argtypes = [ctypes.c_void_p,ctypes.c_int32]
__library__.MSK_XX_putlicensepath.restype  = ctypes.c_int
__library__.MSK_XX_putlicensepath.argtypes = [ctypes.c_void_p,ctypes.c_char_p]
__library__.MSK_XX_axpy.restype  = ctypes.c_int
__library__.MSK_XX_axpy.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_double,ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_dot.restype  = ctypes.c_int
__library__.MSK_XX_dot.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_gemv.restype  = ctypes.c_int
__library__.MSK_XX_gemv.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.c_double,ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.c_double,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_gemm.restype  = ctypes.c_int
__library__.MSK_XX_gemm.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.c_double,ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double),ctypes.c_double,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_syrk.restype  = ctypes.c_int
__library__.MSK_XX_syrk.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.c_double,ctypes.POINTER(ctypes.c_double),ctypes.c_double,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_computesparsecholesky.restype  = ctypes.c_int
__library__.MSK_XX_computesparsecholesky.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_double,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.POINTER(ctypes.c_int32)),ctypes.POINTER(ctypes.POINTER(ctypes.c_double)),ctypes.POINTER(ctypes.POINTER(ctypes.c_int32)),ctypes.POINTER(ctypes.POINTER(ctypes.c_int64)),ctypes.POINTER(ctypes.c_int64),ctypes.POINTER(ctypes.POINTER(ctypes.c_int32)),ctypes.POINTER(ctypes.POINTER(ctypes.c_double))]
__library__.MSK_XX_sparsetriangularsolvedense.restype  = ctypes.c_int
__library__.MSK_XX_sparsetriangularsolvedense.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_int64),ctypes.c_int64,ctypes.POINTER(ctypes.c_int32),ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_potrf.restype  = ctypes.c_int
__library__.MSK_XX_potrf.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_syeig.restype  = ctypes.c_int
__library__.MSK_XX_syeig.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_syevd.restype  = ctypes.c_int
__library__.MSK_XX_syevd.argtypes = [ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_double)]
__library__.MSK_XX_licensecleanup.restype  = ctypes.c_int
__library__.MSK_XX_licensecleanup.argtypes = []

solveform = Enum("solveform", ["dual","free","primal"], [2,0,1])
purify = Enum("purify", ["auto","dual","none","primal","primal_dual"], [4,2,0,1,3])
sensitivitytype = Enum("sensitivitytype", ["basis"], [0])
simreform = Enum("simreform", ["aggressive","free","off","on"], [3,2,0,1])
simhotstart = Enum("simhotstart", ["free","none","status_keys"], [1,0,2])
streamtype = Enum("streamtype", ["err","log","msg","wrn"], [2,0,1,3])
symmattype = Enum("symmattype", ["sparse"], [0])
iinfitem = Enum("iinfitem", ["ana_pro_num_con","ana_pro_num_con_eq","ana_pro_num_con_fr","ana_pro_num_con_lo","ana_pro_num_con_ra","ana_pro_num_con_up","ana_pro_num_var","ana_pro_num_var_bin","ana_pro_num_var_cont","ana_pro_num_var_eq","ana_pro_num_var_fr","ana_pro_num_var_int","ana_pro_num_var_lo","ana_pro_num_var_ra","ana_pro_num_var_up","intpnt_factor_dim_dense","intpnt_iter","intpnt_num_threads","intpnt_solve_dual","mio_absgap_satisfied","mio_clique_table_size","mio_construct_solution","mio_node_depth","mio_num_active_nodes","mio_num_branch","mio_num_clique_cuts","mio_num_cmir_cuts","mio_num_gomory_cuts","mio_num_implied_bound_cuts","mio_num_int_solutions","mio_num_knapsack_cover_cuts","mio_num_relax","mio_num_repeated_presolve","mio_numbin","mio_numbinconevar","mio_numcon","mio_numcone","mio_numconevar","mio_numcont","mio_numcontconevar","mio_numdexpcones","mio_numdpowcones","mio_numint","mio_numintconevar","mio_numpexpcones","mio_numppowcones","mio_numqcones","mio_numrqcones","mio_numvar","mio_obj_bound_defined","mio_presolved_numbin","mio_presolved_numbinconevar","mio_presolved_numcon","mio_presolved_numcone","mio_presolved_numconevar","mio_presolved_numcont","mio_presolved_numcontconevar","mio_presolved_numdexpcones","mio_presolved_numdpowcones","mio_presolved_numint","mio_presolved_numintconevar","mio_presolved_numpexpcones","mio_presolved_numppowcones","mio_presolved_numqcones","mio_presolved_numrqcones","mio_presolved_numvar","mio_relgap_satisfied","mio_total_num_cuts","mio_user_obj_cut","opt_numcon","opt_numvar","optimize_response","purify_dual_success","purify_primal_success","rd_numbarvar","rd_numcon","rd_numcone","rd_numintvar","rd_numq","rd_numvar","rd_protype","sim_dual_deg_iter","sim_dual_hotstart","sim_dual_hotstart_lu","sim_dual_inf_iter","sim_dual_iter","sim_numcon","sim_numvar","sim_primal_deg_iter","sim_primal_hotstart","sim_primal_hotstart_lu","sim_primal_inf_iter","sim_primal_iter","sim_solve_dual","sol_bas_prosta","sol_bas_solsta","sol_itg_prosta","sol_itg_solsta","sol_itr_prosta","sol_itr_solsta","sto_num_a_realloc"], [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100])
dataformat = Enum("dataformat", ["cb","extension","free_mps","json_task","lp","mps","op","ptf","task"], [7,0,4,8,2,1,3,6,5])
prosta = Enum("prosta", ["dual_feas","dual_infeas","ill_posed","prim_and_dual_feas","prim_and_dual_infeas","prim_feas","prim_infeas","prim_infeas_or_unbounded","unknown"], [3,5,7,1,6,2,4,8,0])
boundkey = Enum("boundkey", ["fr","fx","lo","ra","up"], [3,2,0,4,1])
liinfitem = Enum("liinfitem", ["bi_clean_dual_deg_iter","bi_clean_dual_iter","bi_clean_primal_deg_iter","bi_clean_primal_iter","bi_dual_iter","bi_primal_iter","intpnt_factor_num_nz","mio_anz","mio_intpnt_iter","mio_presolved_anz","mio_simplex_iter","rd_numanz","rd_numqnz"], [0,1,2,3,4,5,6,7,8,9,10,11,12])
inftype = Enum("inftype", ["dou_type","int_type","lint_type"], [0,1,2])
orderingtype = Enum("orderingtype", ["appminloc","experimental","force_graphpar","free","none","try_graphpar"], [1,2,4,0,5,3])
basindtype = Enum("basindtype", ["always","if_feasible","never","no_error","reservered"], [1,3,0,2,4])
parametertype = Enum("parametertype", ["dou_type","int_type","invalid_type","str_type"], [1,2,0,3])
dinfitem = Enum("dinfitem", ["bi_clean_dual_time","bi_clean_primal_time","bi_clean_time","bi_dual_time","bi_primal_time","bi_time","intpnt_dual_feas","intpnt_dual_obj","intpnt_factor_num_flops","intpnt_opt_status","intpnt_order_time","intpnt_primal_feas","intpnt_primal_obj","intpnt_time","mio_clique_separation_time","mio_cmir_separation_time","mio_construct_solution_obj","mio_dual_bound_after_presolve","mio_gmi_separation_time","mio_implied_bound_time","mio_knapsack_cover_separation_time","mio_obj_abs_gap","mio_obj_bound","mio_obj_int","mio_obj_rel_gap","mio_probing_time","mio_root_cutgen_time","mio_root_optimizer_time","mio_root_presolve_time","mio_time","mio_user_obj_cut","optimizer_time","presolve_eli_time","presolve_lindep_time","presolve_time","primal_repair_penalty_obj","qcqo_reformulate_max_perturbation","qcqo_reformulate_time","qcqo_reformulate_worst_cholesky_column_scaling","qcqo_reformulate_worst_cholesky_diag_scaling","rd_time","sim_dual_time","sim_feas","sim_obj","sim_primal_time","sim_time","sol_bas_dual_obj","sol_bas_dviolcon","sol_bas_dviolvar","sol_bas_nrm_barx","sol_bas_nrm_slc","sol_bas_nrm_slx","sol_bas_nrm_suc","sol_bas_nrm_sux","sol_bas_nrm_xc","sol_bas_nrm_xx","sol_bas_nrm_y","sol_bas_primal_obj","sol_bas_pviolcon","sol_bas_pviolvar","sol_itg_nrm_barx","sol_itg_nrm_xc","sol_itg_nrm_xx","sol_itg_primal_obj","sol_itg_pviolbarvar","sol_itg_pviolcon","sol_itg_pviolcones","sol_itg_pviolitg","sol_itg_pviolvar","sol_itr_dual_obj","sol_itr_dviolbarvar","sol_itr_dviolcon","sol_itr_dviolcones","sol_itr_dviolvar","sol_itr_nrm_bars","sol_itr_nrm_barx","sol_itr_nrm_slc","sol_itr_nrm_slx","sol_itr_nrm_snx","sol_itr_nrm_suc","sol_itr_nrm_sux","sol_itr_nrm_xc","sol_itr_nrm_xx","sol_itr_nrm_y","sol_itr_primal_obj","sol_itr_pviolbarvar","sol_itr_pviolcon","sol_itr_pviolcones","sol_itr_pviolvar","to_conic_time"], [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89])
miomode = Enum("miomode", ["ignored","satisfied"], [0,1])
miocontsoltype = Enum("miocontsoltype", ["itg","itg_rel","none","root"], [2,3,0,1])
scalingmethod = Enum("scalingmethod", ["free","pow2"], [1,0])
branchdir = Enum("branchdir", ["down","far","free","guided","near","pseudocost","root_lp","up"], [2,4,0,6,3,7,5,1])
solitem = Enum("solitem", ["slc","slx","snx","suc","sux","xc","xx","y"], [3,5,7,4,6,0,1,2])
checkconvexitytype = Enum("checkconvexitytype", ["full","none","simple"], [2,0,1])
intpnthotstart = Enum("intpnthotstart", ["dual","none","primal","primal_dual"], [2,0,1,3])
mionodeseltype = Enum("mionodeseltype", ["best","first","free","pseudo"], [2,1,0,3])
scalingtype = Enum("scalingtype", ["aggressive","free","moderate","none"], [3,0,2,1])
problemtype = Enum("problemtype", ["conic","lo","mixed","qcqo","qo"], [3,0,4,2,1])
mark = Enum("mark", ["lo","up"], [0,1])
stakey = Enum("stakey", ["bas","fix","inf","low","supbas","unk","upr"], [1,5,6,3,2,0,4])
callbackcode = Enum("callbackcode", ["begin_bi","begin_conic","begin_dual_bi","begin_dual_sensitivity","begin_dual_setup_bi","begin_dual_simplex","begin_dual_simplex_bi","begin_full_convexity_check","begin_infeas_ana","begin_intpnt","begin_license_wait","begin_mio","begin_optimizer","begin_presolve","begin_primal_bi","begin_primal_repair","begin_primal_sensitivity","begin_primal_setup_bi","begin_primal_simplex","begin_primal_simplex_bi","begin_qcqo_reformulate","begin_read","begin_root_cutgen","begin_simplex","begin_simplex_bi","begin_to_conic","begin_write","conic","dual_simplex","end_bi","end_conic","end_dual_bi","end_dual_sensitivity","end_dual_setup_bi","end_dual_simplex","end_dual_simplex_bi","end_full_convexity_check","end_infeas_ana","end_intpnt","end_license_wait","end_mio","end_optimizer","end_presolve","end_primal_bi","end_primal_repair","end_primal_sensitivity","end_primal_setup_bi","end_primal_simplex","end_primal_simplex_bi","end_qcqo_reformulate","end_read","end_root_cutgen","end_simplex","end_simplex_bi","end_to_conic","end_write","im_bi","im_conic","im_dual_bi","im_dual_sensivity","im_dual_simplex","im_full_convexity_check","im_intpnt","im_license_wait","im_lu","im_mio","im_mio_dual_simplex","im_mio_intpnt","im_mio_primal_simplex","im_order","im_presolve","im_primal_bi","im_primal_sensivity","im_primal_simplex","im_qo_reformulate","im_read","im_root_cutgen","im_simplex","im_simplex_bi","intpnt","new_int_mio","primal_simplex","read_opf","read_opf_section","solving_remote","update_dual_bi","update_dual_simplex","update_dual_simplex_bi","update_presolve","update_primal_bi","update_primal_simplex","update_primal_simplex_bi","write_opf"], [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92])
soltype = Enum("soltype", ["bas","itg","itr"], [1,2,0])
feature = Enum("feature", ["pton","pts"], [1,0])
uplo = Enum("uplo", ["lo","up"], [0,1])
value = Enum("value", ["license_buffer_length","max_str_len"], [21,1024])
scopr = Enum("scopr", ["ent","exp","log","pow","sqrt"], [0,1,2,3,4])
problemitem = Enum("problemitem", ["con","cone","var"], [1,2,0])
transpose = Enum("transpose", ["no","yes"], [0,1])
simseltype = Enum("simseltype", ["ase","devex","free","full","partial","se"], [2,3,0,1,5,4])
xmlwriteroutputtype = Enum("xmlwriteroutputtype", ["col","row"], [1,0])
objsense = Enum("objsense", ["maximize","minimize"], [1,0])
rescodetype = Enum("rescodetype", ["err","ok","trm","unk","wrn"], [3,0,2,4,1])
solsta = Enum("solsta", ["dual_feas","dual_illposed_cer","dual_infeas_cer","integer_optimal","optimal","prim_and_dual_feas","prim_feas","prim_illposed_cer","prim_infeas_cer","unknown"], [3,8,6,9,1,4,2,7,5,0])
conetype = Enum("conetype", ["dexp","dpow","pexp","ppow","quad","rquad","zero"], [3,5,2,4,0,1,6])
onoffkey = Enum("onoffkey", ["off","on"], [0,1])
sparam = Enum("sparam", ["bas_sol_file_name","data_file_name","debug_file_name","int_sol_file_name","itr_sol_file_name","mio_debug_string","param_comment_sign","param_read_file_name","param_write_file_name","read_mps_bou_name","read_mps_obj_name","read_mps_ran_name","read_mps_rhs_name","remote_access_token","sensitivity_file_name","sensitivity_res_file_name","sol_filter_xc_low","sol_filter_xc_upr","sol_filter_xx_low","sol_filter_xx_upr","stat_file_name","stat_key","stat_name","write_lp_gen_var_name"], [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23])
compresstype = Enum("compresstype", ["free","gzip","none","zstd"], [1,2,0,3])
startpointtype = Enum("startpointtype", ["constant","free","guess","satisfy_bounds"], [2,0,1,3])
simdupvec = Enum("simdupvec", ["free","off","on"], [2,0,1])
simdegen = Enum("simdegen", ["aggressive","free","minimum","moderate","none"], [2,1,4,3,0])
mpsformat = Enum("mpsformat", ["cplex","free","relaxed","strict"], [3,2,1,0])
dparam = Enum("dparam", ["ana_sol_infeas_tol","basis_rel_tol_s","basis_tol_s","basis_tol_x","check_convexity_rel_tol","data_sym_mat_tol","data_sym_mat_tol_huge","data_sym_mat_tol_large","data_tol_aij_huge","data_tol_aij_large","data_tol_bound_inf","data_tol_bound_wrn","data_tol_c_huge","data_tol_cj_large","data_tol_qij","data_tol_x","intpnt_co_tol_dfeas","intpnt_co_tol_infeas","intpnt_co_tol_mu_red","intpnt_co_tol_near_rel","intpnt_co_tol_pfeas","intpnt_co_tol_rel_gap","intpnt_qo_tol_dfeas","intpnt_qo_tol_infeas","intpnt_qo_tol_mu_red","intpnt_qo_tol_near_rel","intpnt_qo_tol_pfeas","intpnt_qo_tol_rel_gap","intpnt_tol_dfeas","intpnt_tol_dsafe","intpnt_tol_infeas","intpnt_tol_mu_red","intpnt_tol_path","intpnt_tol_pfeas","intpnt_tol_psafe","intpnt_tol_rel_gap","intpnt_tol_rel_step","intpnt_tol_step_size","lower_obj_cut","lower_obj_cut_finite_trh","mio_max_time","mio_rel_gap_const","mio_tol_abs_gap","mio_tol_abs_relax_int","mio_tol_feas","mio_tol_rel_dual_bound_improvement","mio_tol_rel_gap","optimizer_max_time","presolve_tol_abs_lindep","presolve_tol_aij","presolve_tol_rel_lindep","presolve_tol_s","presolve_tol_x","qcqo_reformulate_rel_drop_tol","semidefinite_tol_approx","sim_lu_tol_rel_piv","simplex_abs_tol_piv","upper_obj_cut","upper_obj_cut_finite_trh"], [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58])
iparam = Enum("iparam", ["ana_sol_basis","ana_sol_print_violated","auto_sort_a_before_opt","auto_update_sol_info","basis_solve_use_plus_one","bi_clean_optimizer","bi_ignore_max_iter","bi_ignore_num_error","bi_max_iterations","cache_license","check_convexity","compress_statfile","infeas_generic_names","infeas_prefer_primal","infeas_report_auto","infeas_report_level","intpnt_basis","intpnt_diff_step","intpnt_hotstart","intpnt_max_iterations","intpnt_max_num_cor","intpnt_max_num_refinement_steps","intpnt_multi_thread","intpnt_off_col_trh","intpnt_order_gp_num_seeds","intpnt_order_method","intpnt_purify","intpnt_regularization_use","intpnt_scaling","intpnt_solve_form","intpnt_starting_point","license_debug","license_pause_time","license_suppress_expire_wrns","license_trh_expiry_wrn","license_wait","log","log_ana_pro","log_bi","log_bi_freq","log_check_convexity","log_cut_second_opt","log_expand","log_feas_repair","log_file","log_include_summary","log_infeas_ana","log_intpnt","log_local_info","log_mio","log_mio_freq","log_order","log_presolve","log_response","log_sensitivity","log_sensitivity_opt","log_sim","log_sim_freq","log_sim_minor","log_storage","max_num_warnings","mio_branch_dir","mio_conic_outer_approximation","mio_cut_clique","mio_cut_cmir","mio_cut_gmi","mio_cut_implied_bound","mio_cut_knapsack_cover","mio_cut_selection_level","mio_feaspump_level","mio_heuristic_level","mio_max_num_branches","mio_max_num_relaxs","mio_max_num_root_cut_rounds","mio_max_num_solutions","mio_mode","mio_node_optimizer","mio_node_selection","mio_perspective_reformulate","mio_probing_level","mio_propagate_objective_constraint","mio_rins_max_nodes","mio_root_optimizer","mio_root_repeat_presolve_level","mio_seed","mio_vb_detection_level","mt_spincount","num_threads","opf_write_header","opf_write_hints","opf_write_line_length","opf_write_parameters","opf_write_problem","opf_write_sol_bas","opf_write_sol_itg","opf_write_sol_itr","opf_write_solutions","optimizer","param_read_case_name","param_read_ign_error","presolve_eliminator_max_fill","presolve_eliminator_max_num_tries","presolve_level","presolve_lindep_abs_work_trh","presolve_lindep_rel_work_trh","presolve_lindep_use","presolve_max_num_pass","presolve_max_num_reductions","presolve_use","primal_repair_optimizer","ptf_write_transform","read_debug","read_keep_free_con","read_lp_drop_new_vars_in_bou","read_lp_quoted_names","read_mps_format","read_mps_width","read_task_ignore_param","remove_unused_solutions","sensitivity_all","sensitivity_optimizer","sensitivity_type","sim_basis_factor_use","sim_degen","sim_dual_crash","sim_dual_phaseone_method","sim_dual_restrict_selection","sim_dual_selection","sim_exploit_dupvec","sim_hotstart","sim_hotstart_lu","sim_max_iterations","sim_max_num_setbacks","sim_non_singular","sim_primal_crash","sim_primal_phaseone_method","sim_primal_restrict_selection","sim_primal_selection","sim_refactor_freq","sim_reformulation","sim_save_lu","sim_scaling","sim_scaling_method","sim_seed","sim_solve_form","sim_stability_priority","sim_switch_optimizer","sol_filter_keep_basic","sol_filter_keep_ranged","sol_read_name_width","sol_read_width","solution_callback","timing_level","write_bas_constraints","write_bas_head","write_bas_variables","write_compression","write_data_param","write_free_con","write_generic_names","write_generic_names_io","write_ignore_incompatible_items","write_int_constraints","write_int_head","write_int_variables","write_lp_full_obj","write_lp_line_width","write_lp_quoted_names","write_lp_strict_format","write_lp_terms_per_line","write_mps_format","write_mps_int","write_precision","write_sol_barvariables","write_sol_constraints","write_sol_head","write_sol_ignore_invalid_names","write_sol_variables","write_task_inc_sol","write_xml_mode"], [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179])
optimizertype = Enum("optimizertype", ["conic","dual_simplex","free","free_simplex","intpnt","mixed_int","primal_simplex"], [0,1,2,3,4,5,6])
rescode = Enum("rescode", ["err_ad_invalid_codelist","err_api_array_too_small","err_api_cb_connect","err_api_fatal_error","err_api_internal","err_appending_too_big_cone","err_arg_is_too_large","err_arg_is_too_small","err_argument_dimension","err_argument_is_too_large","err_argument_lenneq","err_argument_perm_array","err_argument_type","err_bar_var_dim","err_basis","err_basis_factor","err_basis_singular","err_blank_name","err_cbf_duplicate_acoord","err_cbf_duplicate_bcoord","err_cbf_duplicate_con","err_cbf_duplicate_int","err_cbf_duplicate_obj","err_cbf_duplicate_objacoord","err_cbf_duplicate_pow_cones","err_cbf_duplicate_pow_star_cones","err_cbf_duplicate_psdvar","err_cbf_duplicate_var","err_cbf_invalid_con_type","err_cbf_invalid_dimension_of_cones","err_cbf_invalid_domain_dimension","err_cbf_invalid_exp_dimension","err_cbf_invalid_int_index","err_cbf_invalid_number_of_cones","err_cbf_invalid_power","err_cbf_invalid_power_cone_index","err_cbf_invalid_power_star_cone_index","err_cbf_invalid_psdvar_dimension","err_cbf_invalid_var_type","err_cbf_no_variables","err_cbf_no_version_specified","err_cbf_obj_sense","err_cbf_parse","err_cbf_power_cone_is_too_long","err_cbf_power_cone_mismatch","err_cbf_power_star_cone_mismatch","err_cbf_syntax","err_cbf_too_few_constraints","err_cbf_too_few_ints","err_cbf_too_few_psdvar","err_cbf_too_few_variables","err_cbf_too_many_constraints","err_cbf_too_many_ints","err_cbf_too_many_variables","err_cbf_unhandled_power_cone_type","err_cbf_unhandled_power_star_cone_type","err_cbf_unsupported","err_con_q_not_nsd","err_con_q_not_psd","err_cone_index","err_cone_overlap","err_cone_overlap_append","err_cone_parameter","err_cone_rep_var","err_cone_size","err_cone_type","err_cone_type_str","err_data_file_ext","err_dup_name","err_duplicate_aij","err_duplicate_barvariable_names","err_duplicate_cone_names","err_duplicate_constraint_names","err_duplicate_variable_names","err_end_of_file","err_factor","err_feasrepair_cannot_relax","err_feasrepair_inconsistent_bound","err_feasrepair_solving_relaxed","err_file_license","err_file_open","err_file_read","err_file_write","err_final_solution","err_first","err_firsti","err_firstj","err_fixed_bound_values","err_flexlm","err_format_string","err_global_inv_conic_problem","err_huge_aij","err_huge_c","err_identical_tasks","err_in_argument","err_index","err_index_arr_is_too_large","err_index_arr_is_too_small","err_index_is_too_large","err_index_is_too_small","err_inf_dou_index","err_inf_dou_name","err_inf_int_index","err_inf_int_name","err_inf_lint_index","err_inf_lint_name","err_inf_type","err_infeas_undefined","err_infinite_bound","err_int64_to_int32_cast","err_internal","err_internal_test_failed","err_inv_aptre","err_inv_bk","err_inv_bkc","err_inv_bkx","err_inv_cone_type","err_inv_cone_type_str","err_inv_marki","err_inv_markj","err_inv_name_item","err_inv_numi","err_inv_numj","err_inv_optimizer","err_inv_problem","err_inv_qcon_subi","err_inv_qcon_subj","err_inv_qcon_subk","err_inv_qcon_val","err_inv_qobj_subi","err_inv_qobj_subj","err_inv_qobj_val","err_inv_sk","err_inv_sk_str","err_inv_skc","err_inv_skn","err_inv_skx","err_inv_var_type","err_invalid_aij","err_invalid_ampl_stub","err_invalid_barvar_name","err_invalid_compression","err_invalid_con_name","err_invalid_cone_name","err_invalid_file_format_for_cfix","err_invalid_file_format_for_cones","err_invalid_file_format_for_free_constraints","err_invalid_file_format_for_nonlinear","err_invalid_file_format_for_ranged_constraints","err_invalid_file_format_for_sym_mat","err_invalid_file_name","err_invalid_format_type","err_invalid_idx","err_invalid_iomode","err_invalid_max_num","err_invalid_name_in_sol_file","err_invalid_obj_name","err_invalid_objective_sense","err_invalid_problem_type","err_invalid_sol_file_name","err_invalid_stream","err_invalid_surplus","err_invalid_sym_mat_dim","err_invalid_task","err_invalid_utf8","err_invalid_var_name","err_invalid_wchar","err_invalid_whichsol","err_json_data","err_json_format","err_json_missing_data","err_json_number_overflow","err_json_string","err_json_syntax","err_last","err_lasti","err_lastj","err_lau_arg_k","err_lau_arg_m","err_lau_arg_n","err_lau_arg_trans","err_lau_arg_transa","err_lau_arg_transb","err_lau_arg_uplo","err_lau_invalid_lower_triangular_matrix","err_lau_invalid_sparse_symmetric_matrix","err_lau_not_positive_definite","err_lau_singular_matrix","err_lau_unknown","err_license","err_license_cannot_allocate","err_license_cannot_connect","err_license_expired","err_license_feature","err_license_invalid_hostid","err_license_max","err_license_moseklm_daemon","err_license_no_server_line","err_license_no_server_support","err_license_server","err_license_server_version","err_license_version","err_link_file_dll","err_living_tasks","err_lower_bound_is_a_nan","err_lp_dup_slack_name","err_lp_empty","err_lp_file_format","err_lp_format","err_lp_free_constraint","err_lp_incompatible","err_lp_invalid_con_name","err_lp_invalid_var_name","err_lp_write_conic_problem","err_lp_write_geco_problem","err_lu_max_num_tries","err_max_len_is_too_small","err_maxnumbarvar","err_maxnumcon","err_maxnumcone","err_maxnumqnz","err_maxnumvar","err_mio_internal","err_mio_invalid_node_optimizer","err_mio_invalid_root_optimizer","err_mio_no_optimizer","err_missing_license_file","err_mixed_conic_and_nl","err_mps_cone_overlap","err_mps_cone_repeat","err_mps_cone_type","err_mps_duplicate_q_element","err_mps_file","err_mps_inv_bound_key","err_mps_inv_con_key","err_mps_inv_field","err_mps_inv_marker","err_mps_inv_sec_name","err_mps_inv_sec_order","err_mps_invalid_obj_name","err_mps_invalid_objsense","err_mps_mul_con_name","err_mps_mul_csec","err_mps_mul_qobj","err_mps_mul_qsec","err_mps_no_objective","err_mps_non_symmetric_q","err_mps_null_con_name","err_mps_null_var_name","err_mps_splitted_var","err_mps_tab_in_field2","err_mps_tab_in_field3","err_mps_tab_in_field5","err_mps_undef_con_name","err_mps_undef_var_name","err_mul_a_element","err_name_is_null","err_name_max_len","err_nan_in_blc","err_nan_in_blx","err_nan_in_buc","err_nan_in_bux","err_nan_in_c","err_nan_in_double_data","err_negative_append","err_negative_surplus","err_newer_dll","err_no_bars_for_solution","err_no_barx_for_solution","err_no_basis_sol","err_no_dual_for_itg_sol","err_no_dual_infeas_cer","err_no_init_env","err_no_optimizer_var_type","err_no_primal_infeas_cer","err_no_snx_for_bas_sol","err_no_solution_in_callback","err_non_unique_array","err_nonconvex","err_nonlinear_equality","err_nonlinear_ranged","err_null_env","err_null_pointer","err_null_task","err_num_arguments","err_numconlim","err_numvarlim","err_obj_q_not_nsd","err_obj_q_not_psd","err_objective_range","err_older_dll","err_opf_format","err_opf_new_variable","err_opf_premature_eof","err_optimizer_license","err_overflow","err_param_index","err_param_is_too_large","err_param_is_too_small","err_param_name","err_param_name_dou","err_param_name_int","err_param_name_str","err_param_type","err_param_value_str","err_platform_not_licensed","err_postsolve","err_pro_item","err_prob_license","err_ptf_format","err_qcon_subi_too_large","err_qcon_subi_too_small","err_qcon_upper_triangle","err_qobj_upper_triangle","err_read_format","err_read_lp_missing_end_tag","err_read_lp_nonexisting_name","err_remove_cone_variable","err_repair_invalid_problem","err_repair_optimization_failed","err_sen_bound_invalid_lo","err_sen_bound_invalid_up","err_sen_format","err_sen_index_invalid","err_sen_index_range","err_sen_invalid_regexp","err_sen_numerical","err_sen_solution_status","err_sen_undef_name","err_sen_unhandled_problem_type","err_server_connect","err_server_protocol","err_server_status","err_server_token","err_shape_is_too_large","err_size_license","err_size_license_con","err_size_license_intvar","err_size_license_numcores","err_size_license_var","err_slice_size","err_sol_file_invalid_number","err_solitem","err_solver_probtype","err_space","err_space_leaking","err_space_no_info","err_sym_mat_duplicate","err_sym_mat_huge","err_sym_mat_invalid","err_sym_mat_invalid_col_index","err_sym_mat_invalid_row_index","err_sym_mat_invalid_value","err_sym_mat_not_lower_tringular","err_task_incompatible","err_task_invalid","err_task_write","err_thread_cond_init","err_thread_create","err_thread_mutex_init","err_thread_mutex_lock","err_thread_mutex_unlock","err_toconic_constr_not_conic","err_toconic_constr_q_not_psd","err_toconic_constraint_fx","err_toconic_constraint_ra","err_toconic_objective_not_psd","err_too_small_a_truncation_value","err_too_small_max_num_nz","err_too_small_maxnumanz","err_unb_step_size","err_undef_solution","err_undefined_objective_sense","err_unhandled_solution_status","err_unknown","err_upper_bound_is_a_nan","err_upper_triangle","err_whichitem_not_allowed","err_whichsol","err_write_lp_format","err_write_lp_non_unique_name","err_write_mps_invalid_name","err_write_opf_invalid_var_name","err_writing_file","err_xml_invalid_problem_type","err_y_is_undefined","ok","trm_internal","trm_internal_stop","trm_max_iterations","trm_max_num_setbacks","trm_max_time","trm_mio_num_branches","trm_mio_num_relaxs","trm_num_max_num_int_solutions","trm_numerical_problem","trm_objective_range","trm_stall","trm_user_callback","wrn_ana_almost_int_bounds","wrn_ana_c_zero","wrn_ana_close_bounds","wrn_ana_empty_cols","wrn_ana_large_bounds","wrn_dropped_nz_qobj","wrn_duplicate_barvariable_names","wrn_duplicate_cone_names","wrn_duplicate_constraint_names","wrn_duplicate_variable_names","wrn_eliminator_space","wrn_empty_name","wrn_exp_cones_with_variables_fixed_at_zero","wrn_ignore_integer","wrn_incomplete_linear_dependency_check","wrn_large_aij","wrn_large_bound","wrn_large_cj","wrn_large_con_fx","wrn_large_lo_bound","wrn_large_up_bound","wrn_license_expire","wrn_license_feature_expire","wrn_license_server","wrn_lp_drop_variable","wrn_lp_old_quad_format","wrn_mio_infeasible_final","wrn_mps_split_bou_vector","wrn_mps_split_ran_vector","wrn_mps_split_rhs_vector","wrn_name_max_len","wrn_no_dualizer","wrn_no_global_optimizer","wrn_nz_in_upr_tri","wrn_open_param_file","wrn_param_ignored_cmio","wrn_param_name_dou","wrn_param_name_int","wrn_param_name_str","wrn_param_str_value","wrn_pow_cones_with_root_fixed_at_zero","wrn_presolve_outofspace","wrn_quad_cones_with_root_fixed_at_zero","wrn_rquad_cones_with_root_fixed_at_zero","wrn_sol_file_ignored_con","wrn_sol_file_ignored_var","wrn_sol_filter","wrn_spar_max_len","wrn_sym_mat_large","wrn_too_few_basis_vars","wrn_too_many_basis_vars","wrn_undef_sol_file_name","wrn_using_generic_names","wrn_write_changed_names","wrn_write_discarded_cfix","wrn_zero_aij","wrn_zeros_in_sparse_col","wrn_zeros_in_sparse_row"], [3102,3001,3002,3005,3999,1311,1227,1226,1201,5005,1197,1299,1198,3920,1266,1610,1615,1070,7116,7115,7108,7110,7107,7114,7130,7131,7123,7109,7112,7741,7113,7126,7121,7740,7132,7134,7135,7124,7111,7102,7105,7101,7100,7133,7138,7139,7106,7118,7119,7125,7117,7103,7120,7104,7136,7137,7122,1294,1293,1300,1302,1307,1320,1303,1301,1305,1306,1055,1071,1385,4502,4503,4500,4501,1059,1650,1700,1702,1701,1007,1052,1053,1054,1560,1570,1285,1287,1420,1014,1072,1503,1380,1375,3101,1200,1235,1222,1221,1204,1203,1219,1230,1220,1231,1225,1234,1232,3910,1400,3800,3000,3500,1253,1255,1256,1257,1272,1271,2501,2502,1280,2503,2504,1550,1500,1405,1406,1404,1407,1401,1402,1403,1270,1269,1267,1274,1268,1258,1473,3700,1079,1800,1076,1078,4001,4005,4003,4010,4002,4000,1056,1283,1246,1801,1247,1170,1075,1445,6000,1057,1062,1275,3950,1064,2900,1077,2901,1228,1179,1178,1180,1177,1176,1175,1571,1286,1288,7012,7010,7011,7018,7015,7016,7017,7002,7019,7001,7000,7005,1000,1020,1021,1001,1018,1025,1016,1017,1028,1027,1015,1026,1002,1040,1066,1390,1152,1151,1157,1160,1155,1150,1171,1154,1163,1164,2800,1289,1242,1240,1304,1243,1241,5010,7701,7700,1551,1008,1501,1118,1119,1117,1121,1100,1108,1107,1101,1102,1109,1115,1128,1122,1112,1116,1114,1113,1110,1120,1103,1104,1111,1125,1126,1127,1105,1106,1254,1760,1750,1461,1471,1462,1472,1470,1450,1578,1573,1036,3916,3915,1600,2950,2001,1063,1552,2000,2953,2500,5000,1291,1290,1292,1060,1065,1061,1199,1250,1251,1296,1295,1260,1035,1168,1169,1172,1013,1590,1210,1215,1216,1205,1206,1207,1208,1218,1217,1019,1580,1281,1006,1167,1409,1408,1417,1415,1090,1159,1162,1310,1710,1711,3054,3053,3050,3055,3052,3056,3058,3057,3051,3080,8000,8001,8002,8003,1202,1005,1010,1012,3900,1011,1572,1350,1237,1259,1051,1080,1081,3944,1482,1480,3941,3940,3943,3942,2560,2561,2562,1049,1048,1045,1046,1047,7803,7800,7801,7802,7804,1421,1245,1252,3100,1265,1446,6010,1050,1391,6020,1238,1236,1158,1161,1153,1156,1166,3600,1449,0,10030,10031,10000,10020,10001,10009,10008,10015,10025,10002,10006,10007,904,901,903,902,900,201,852,853,850,851,801,502,932,250,800,62,51,57,54,52,53,500,505,501,85,80,270,72,71,70,65,950,251,200,50,516,510,511,512,515,933,802,930,931,351,352,300,66,960,400,405,350,503,803,804,63,710,705])
presolvemode = Enum("presolvemode", ["free","off","on"], [2,0,1])
iomode = Enum("iomode", ["read","readwrite","write"], [0,2,1])
nametype = Enum("nametype", ["gen","lp","mps"], [0,2,1])
variabletype = Enum("variabletype", ["type_cont","type_int"], [0,1])



class Env:
  """
  The MOSEK environment. 
  """
  def __init__(self,licensefile=None,debugfile=None):
      self.__nativep = ctypes.c_void_p()
      self.__library = __library__
      self._lock = threading.RLock()
      res = self.__library.MSK_XX_makeenv(ctypes.byref(self.__nativep),debugfile)
      if res != 0:
          raise Error(rescode(res),"Error %d" % res)
      try:
          if licensefile is not None:
              res = self.__library.MSK_XX_putlicensepath(self.__nativep,licensefile)
              if res != 0:
                  raise Error(rescode(res),"Error %d" % res)
          
          # user stream functions: 
          self.__stream_func   = 4 * [ None ]
          # strema proxy functions and wrappers:
          self.__stream_cb   = 4 * [ None ]
          for whichstream in xrange(4): 
              # Note: Apparently closures doesn't work when the function is wrapped in a C function... So we use default parameter value instead.
              def stream_proxy(handle, msg, whichstream=whichstream):
                  func = self.__stream_func[whichstream]
                  try:
                      if func:
                          func(msg)
                  except:
                      pass
              self.__stream_cb[whichstream] = __stream_cb_type__(stream_proxy)
          self.__enablegarcolenv()
      except:
          self.__library.MSK_XX_deleteenv(ctypes.byref(self.__nativep))
          raise
      
  def set_Stream(self,whichstream,func):
      if isinstance(whichstream, streamtype):
          self.__stream_func[whichstream] = func
          if func is None:
              res = self.__library.MSK_XX_linkfunctoenvstream(self.__nativep,whichstream,None,ctypes.cast(None,__stream_cb_type__))
          else:
              res = self.__library.MSK_XX_linkfunctoenvstream(self.__nativep,whichstream,None,self.__stream_cb[whichstream])
      else:
          raise TypeError("Invalid stream %s" % whichstream)
  def __enablegarcolenv(self):
        self.__library.MSK_XX_enablegarcolenv(self.__nativep)

  def _getNativeP(self):
      return self.__nativep
  def __del__(self):
    if self.__nativep is not None:
      for f in feature.members():
        self.checkinlicense(f)
      self.__library.MSK_XX_deleteenv(ctypes.byref(self.__nativep))
      del self.__stream_func
      del self.__stream_cb
      del self.__library
    self.__nativep  = None
  def Task(self,maxnumcon=0,maxnumvar=0):
    return Task(self,maxnumcon,maxnumvar)

  # Implementation of disposable protocol  
  def __enter__(self):
      return self

  def __exit__(self,exc_type,exc_value,traceback):
      self.__del__()
  @accepts(_accept_any,_accept_anyenum(feature))
  @synchronized
  def checkoutlicense(self,feature_):
    """
    Check out a license feature from the license server ahead of time.
  
    checkoutlicense(self,feature_)
      feature: mosek.feature. Feature to check out from the license system.
    """
    res = __library__.MSK_XX_checkoutlicense(self.__nativep,feature_)
    if res != 0:
      raise Error(rescode(res),Env.getcodedesc(rescode(res))[1])
  @accepts(_accept_any,_accept_anyenum(feature))
  @synchronized
  def checkinlicense(self,feature_):
    """
    Check in a license feature back to the license server ahead of time.
  
    checkinlicense(self,feature_)
      feature: mosek.feature. Feature to check in to the license system.
    """
    res = __library__.MSK_XX_checkinlicense(self.__nativep,feature_)
    if res != 0:
      raise Error(rescode(res),Env.getcodedesc(rescode(res))[1])
  @accepts(_accept_any)
  @synchronized
  def checkinall(self):
    """
    Check in all unused license features to the license token server.
  
    checkinall(self)
    """
    res = __library__.MSK_XX_checkinall(self.__nativep)
    if res != 0:
      raise Error(rescode(res),Env.getcodedesc(rescode(res))[1])
  @accepts(_accept_any,_make_int)
  @synchronized
  def setupthreads(self,numthreads_):
    """
    Preallocates a thread pool.
  
    setupthreads(self,numthreads_)
      numthreads: int. Number of threads.
    """
    res = __library__.MSK_XX_setupthreads(self.__nativep,numthreads_)
    if res != 0:
      raise Error(rescode(res),Env.getcodedesc(rescode(res))[1])
  @accepts(_accept_any,_make_int)
  @synchronized
  def echointro(self,longver_):
    """
    Prints an intro to message stream.
  
    echointro(self,longver_)
      longver: int. If non-zero, then the intro is slightly longer.
    """
    res = __library__.MSK_XX_echointro(self.__nativep,longver_)
    if res != 0:
      raise Error(rescode(res),Env.getcodedesc(rescode(res))[1])
  @staticmethod
  @accepts(_accept_anyenum(rescode))
  def getcodedesc(code_):
    """
    Obtains a short description of a response code.
  
    getcodedesc(code_)
      code: mosek.rescode. A valid response code.
    returns: symname,str
      symname: unicode. Symbolic name corresponding to the code.
      str: unicode. Obtains a short description of a response code.
    """
    symname_ = (ctypes.c_char * value.max_str_len)()
    str_ = (ctypes.c_char * value.max_str_len)()
    res = __library__.MSK_XX_getcodedesc(code_,symname_,str_)
    if res != 0:
      raise Error(rescode(res),Env.getcodedesc(rescode(res))[1])
    _symname_retval = symname_.value.decode("utf-8",errors="replace")
    _str_retval = str_.value.decode("utf-8",errors="replace")
    return (_symname_retval,_str_retval)
  @staticmethod
  @accepts()
  def getversion():
    """
    Obtains MOSEK version information.
  
    getversion()
    returns: major,minor,revision
      major: int. Major version number.
      minor: int. Minor version number.
      revision: int. Revision number.
    """
    major_ = ctypes.c_int32()
    minor_ = ctypes.c_int32()
    revision_ = ctypes.c_int32()
    res = __library__.MSK_XX_getversion(ctypes.byref(major_),ctypes.byref(minor_),ctypes.byref(revision_))
    if res != 0:
      raise Error(rescode(res),Env.getcodedesc(rescode(res))[1])
    major_ = major_.value
    _major_return_value = major_
    minor_ = minor_.value
    _minor_return_value = minor_
    revision_ = revision_.value
    _revision_return_value = revision_
    return (_major_return_value,_minor_return_value,_revision_return_value)
  @accepts(_accept_any,_accept_anyenum(streamtype),_accept_str,_make_int)
  @synchronized
  def linkfiletostream(self,whichstream_,filename_,append_):
    """
    Directs all output from a stream to a file.
  
    linkfiletostream(self,whichstream_,filename_,append_)
      whichstream: mosek.streamtype. Index of the stream.
      filename: str|unicode. A valid file name.
      append: int. If this argument is 0 the file will be overwritten, otherwise it will be appended to.
    """
    if isinstance(filename_,unicode):
      filename_ = filename_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_linkfiletoenvstream(self.__nativep,whichstream_,filename_,append_)
    if res != 0:
      raise Error(rescode(res),Env.getcodedesc(rescode(res))[1])
  @accepts(_accept_any,_make_int)
  @synchronized
  def putlicensedebug(self,licdebug_):
    """
    Enables debug information for the license system.
  
    putlicensedebug(self,licdebug_)
      licdebug: int. Enable output of license check-out debug information.
    """
    res = __library__.MSK_XX_putlicensedebug(self.__nativep,licdebug_)
    if res != 0:
      raise Error(rescode(res),Env.getcodedesc(rescode(res))[1])
  @accepts(_accept_any,_make_intvector)
  @synchronized
  def putlicensecode(self,code_):
    """
    Input a runtime license code.
  
    putlicensecode(self,code_)
      code: array of int. A license key string.
    """
    _code_minlength = value.license_buffer_length
    if value.license_buffer_length > 0 and code_ is not None and len(code_) != value.license_buffer_length:
      raise ValueError("Array argument code is not long enough: Is %d, expected %d" % (len(code_),value.license_buffer_length))
    if isinstance(code_, numpy.ndarray) and code_.dtype is numpy.dtype(numpy.int32) and code_.flags.contiguous:
      _code_copyarray = False
      _code_tmp = ctypes.cast(code_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif code_ is not None:
      _code_copyarray = True
      _code_np_tmp = numpy.zeros(len(code_),numpy.dtype(numpy.int32))
      _code_np_tmp[:] = code_
      assert _code_np_tmp.flags.contiguous
      _code_tmp = ctypes.cast(_code_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _code_copyarray = False
      _code_tmp = None
      
    res = __library__.MSK_XX_putlicensecode(self.__nativep,_code_tmp)
    if res != 0:
      raise Error(rescode(res),Env.getcodedesc(rescode(res))[1])
  @accepts(_accept_any,_make_int)
  @synchronized
  def putlicensewait(self,licwait_):
    """
    Control whether mosek should wait for an available license if no license is available.
  
    putlicensewait(self,licwait_)
      licwait: int. Enable waiting for a license.
    """
    res = __library__.MSK_XX_putlicensewait(self.__nativep,licwait_)
    if res != 0:
      raise Error(rescode(res),Env.getcodedesc(rescode(res))[1])
  @accepts(_accept_any,_accept_str)
  @synchronized
  def putlicensepath(self,licensepath_):
    """
    Set the path to the license file.
  
    putlicensepath(self,licensepath_)
      licensepath: str|unicode. A path specifying where to search for the license.
    """
    if isinstance(licensepath_,unicode):
      licensepath_ = licensepath_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_putlicensepath(self.__nativep,licensepath_)
    if res != 0:
      raise Error(rescode(res),Env.getcodedesc(rescode(res))[1])
  @accepts(_accept_any,_make_int,_make_double,_make_doublevector,_accept_doublevector)
  @synchronized
  def axpy(self,n_,alpha_,x_,y_):
    """
    Computes vector addition and multiplication by a scalar.
  
    axpy(self,n_,alpha_,x_,y_)
      n: int. Length of the vectors.
      alpha: double. The scalar that multiplies x.
      x: array of double. The x vector.
      y: array of double. The y vector.
    """
    _x_minlength = (n_)
    if (n_) > 0 and x_ is not None and len(x_) != (n_):
      raise ValueError("Array argument x is not long enough: Is %d, expected %d" % (len(x_),(n_)))
    if x_ is None:
      raise ValueError("Argument x cannot be None")
    if x_ is None:
      raise ValueError("Argument x may not be None")
    if isinstance(x_, numpy.ndarray) and x_.dtype is numpy.dtype(numpy.float64) and x_.flags.contiguous:
      _x_copyarray = False
      _x_tmp = ctypes.cast(x_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif x_ is not None:
      _x_copyarray = True
      _x_np_tmp = numpy.zeros(len(x_),numpy.dtype(numpy.float64))
      _x_np_tmp[:] = x_
      assert _x_np_tmp.flags.contiguous
      _x_tmp = ctypes.cast(_x_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _x_copyarray = False
      _x_tmp = None
      
    _y_minlength = (n_)
    if (n_) > 0 and y_ is not None and len(y_) != (n_):
      raise ValueError("Array argument y is not long enough: Is %d, expected %d" % (len(y_),(n_)))
    if isinstance(y_,numpy.ndarray) and not y_.flags.writeable:
      raise ValueError("Argument y must be writable")
    if y_ is None:
      raise ValueError("Argument y may not be None")
    if isinstance(y_, numpy.ndarray) and y_.dtype is numpy.dtype(numpy.float64) and y_.flags.contiguous:
      _y_copyarray = False
      _y_tmp = ctypes.cast(y_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif y_ is not None:
      _y_copyarray = True
      _y_np_tmp = numpy.zeros(len(y_),numpy.dtype(numpy.float64))
      _y_np_tmp[:] = y_
      assert _y_np_tmp.flags.contiguous
      _y_tmp = ctypes.cast(_y_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _y_copyarray = False
      _y_tmp = None
      
    res = __library__.MSK_XX_axpy(self.__nativep,n_,alpha_,_x_tmp,_y_tmp)
    if res != 0:
      raise Error(rescode(res),Env.getcodedesc(rescode(res))[1])
    if _y_copyarray:
      y_[:] = _y_np_tmp
  @accepts(_accept_any,_make_int,_make_doublevector,_make_doublevector)
  @synchronized
  def dot(self,n_,x_,y_):
    """
    Computes the inner product of two vectors.
  
    dot(self,n_,x_,y_)
      n: int. Length of the vectors.
      x: array of double. The x vector.
      y: array of double. The y vector.
    returns: xty
      xty: double. The result of the inner product.
    """
    _x_minlength = (n_)
    if (n_) > 0 and x_ is not None and len(x_) != (n_):
      raise ValueError("Array argument x is not long enough: Is %d, expected %d" % (len(x_),(n_)))
    if x_ is None:
      raise ValueError("Argument x cannot be None")
    if x_ is None:
      raise ValueError("Argument x may not be None")
    if isinstance(x_, numpy.ndarray) and x_.dtype is numpy.dtype(numpy.float64) and x_.flags.contiguous:
      _x_copyarray = False
      _x_tmp = ctypes.cast(x_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif x_ is not None:
      _x_copyarray = True
      _x_np_tmp = numpy.zeros(len(x_),numpy.dtype(numpy.float64))
      _x_np_tmp[:] = x_
      assert _x_np_tmp.flags.contiguous
      _x_tmp = ctypes.cast(_x_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _x_copyarray = False
      _x_tmp = None
      
    _y_minlength = (n_)
    if (n_) > 0 and y_ is not None and len(y_) != (n_):
      raise ValueError("Array argument y is not long enough: Is %d, expected %d" % (len(y_),(n_)))
    if y_ is None:
      raise ValueError("Argument y cannot be None")
    if y_ is None:
      raise ValueError("Argument y may not be None")
    if isinstance(y_, numpy.ndarray) and y_.dtype is numpy.dtype(numpy.float64) and y_.flags.contiguous:
      _y_copyarray = False
      _y_tmp = ctypes.cast(y_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif y_ is not None:
      _y_copyarray = True
      _y_np_tmp = numpy.zeros(len(y_),numpy.dtype(numpy.float64))
      _y_np_tmp[:] = y_
      assert _y_np_tmp.flags.contiguous
      _y_tmp = ctypes.cast(_y_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _y_copyarray = False
      _y_tmp = None
      
    xty_ = ctypes.c_double()
    res = __library__.MSK_XX_dot(self.__nativep,n_,_x_tmp,_y_tmp,ctypes.byref(xty_))
    if res != 0:
      raise Error(rescode(res),Env.getcodedesc(rescode(res))[1])
    xty_ = xty_.value
    _xty_return_value = xty_
    return (_xty_return_value)
  @accepts(_accept_any,_accept_anyenum(transpose),_make_int,_make_int,_make_double,_make_doublevector,_make_doublevector,_make_double,_accept_doublevector)
  @synchronized
  def gemv(self,transa_,m_,n_,alpha_,a_,x_,beta_,y_):
    """
    Computes dense matrix times a dense vector product.
  
    gemv(self,transa_,m_,n_,alpha_,a_,x_,beta_,y_)
      transa: mosek.transpose. Indicates whether the matrix A must be transposed.
      m: int. Specifies the number of rows of the matrix A.
      n: int. Specifies the number of columns of the matrix A.
      alpha: double. A scalar value multiplying the matrix A.
      a: array of double. A pointer to the array storing matrix A in a column-major format.
      x: array of double. A pointer to the array storing the vector x.
      beta: double. A scalar value multiplying the vector y.
      y: array of double. A pointer to the array storing the vector y.
    """
    _a_minlength = ((n_) * (m_))
    if ((n_) * (m_)) > 0 and a_ is not None and len(a_) != ((n_) * (m_)):
      raise ValueError("Array argument a is not long enough: Is %d, expected %d" % (len(a_),((n_) * (m_))))
    if a_ is None:
      raise ValueError("Argument a cannot be None")
    if a_ is None:
      raise ValueError("Argument a may not be None")
    if isinstance(a_, numpy.ndarray) and a_.dtype is numpy.dtype(numpy.float64) and a_.flags.contiguous:
      _a_copyarray = False
      _a_tmp = ctypes.cast(a_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif a_ is not None:
      _a_copyarray = True
      _a_np_tmp = numpy.zeros(len(a_),numpy.dtype(numpy.float64))
      _a_np_tmp[:] = a_
      assert _a_np_tmp.flags.contiguous
      _a_tmp = ctypes.cast(_a_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _a_copyarray = False
      _a_tmp = None
      
    if ((transa_) == transpose.no):
      __tmp_var_0 = (n_);
    else:
      __tmp_var_0 = (m_);
    _x_minlength = __tmp_var_0
    if __tmp_var_0 > 0 and x_ is not None and len(x_) != __tmp_var_0:
      raise ValueError("Array argument x is not long enough: Is %d, expected %d" % (len(x_),__tmp_var_0))
    if x_ is None:
      raise ValueError("Argument x cannot be None")
    if x_ is None:
      raise ValueError("Argument x may not be None")
    if isinstance(x_, numpy.ndarray) and x_.dtype is numpy.dtype(numpy.float64) and x_.flags.contiguous:
      _x_copyarray = False
      _x_tmp = ctypes.cast(x_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif x_ is not None:
      _x_copyarray = True
      _x_np_tmp = numpy.zeros(len(x_),numpy.dtype(numpy.float64))
      _x_np_tmp[:] = x_
      assert _x_np_tmp.flags.contiguous
      _x_tmp = ctypes.cast(_x_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _x_copyarray = False
      _x_tmp = None
      
    if ((transa_) == transpose.no):
      __tmp_var_1 = (m_);
    else:
      __tmp_var_1 = (n_);
    _y_minlength = __tmp_var_1
    if __tmp_var_1 > 0 and y_ is not None and len(y_) != __tmp_var_1:
      raise ValueError("Array argument y is not long enough: Is %d, expected %d" % (len(y_),__tmp_var_1))
    if isinstance(y_,numpy.ndarray) and not y_.flags.writeable:
      raise ValueError("Argument y must be writable")
    if y_ is None:
      raise ValueError("Argument y may not be None")
    if isinstance(y_, numpy.ndarray) and y_.dtype is numpy.dtype(numpy.float64) and y_.flags.contiguous:
      _y_copyarray = False
      _y_tmp = ctypes.cast(y_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif y_ is not None:
      _y_copyarray = True
      _y_np_tmp = numpy.zeros(len(y_),numpy.dtype(numpy.float64))
      _y_np_tmp[:] = y_
      assert _y_np_tmp.flags.contiguous
      _y_tmp = ctypes.cast(_y_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _y_copyarray = False
      _y_tmp = None
      
    res = __library__.MSK_XX_gemv(self.__nativep,transa_,m_,n_,alpha_,_a_tmp,_x_tmp,beta_,_y_tmp)
    if res != 0:
      raise Error(rescode(res),Env.getcodedesc(rescode(res))[1])
    if _y_copyarray:
      y_[:] = _y_np_tmp
  @accepts(_accept_any,_accept_anyenum(transpose),_accept_anyenum(transpose),_make_int,_make_int,_make_int,_make_double,_make_doublevector,_make_doublevector,_make_double,_accept_doublevector)
  @synchronized
  def gemm(self,transa_,transb_,m_,n_,k_,alpha_,a_,b_,beta_,c_):
    """
    Performs a dense matrix multiplication.
  
    gemm(self,transa_,transb_,m_,n_,k_,alpha_,a_,b_,beta_,c_)
      transa: mosek.transpose. Indicates whether the matrix A must be transposed.
      transb: mosek.transpose. Indicates whether the matrix B must be transposed.
      m: int. Indicates the number of rows of matrix C.
      n: int. Indicates the number of columns of matrix C.
      k: int. Specifies the common dimension along which op(A) and op(B) are multiplied.
      alpha: double. A scalar value multiplying the result of the matrix multiplication.
      a: array of double. The pointer to the array storing matrix A in a column-major format.
      b: array of double. The pointer to the array storing matrix B in a column-major format.
      beta: double. A scalar value that multiplies C.
      c: array of double. The pointer to the array storing matrix C in a column-major format.
    """
    _a_minlength = ((m_) * (k_))
    if ((m_) * (k_)) > 0 and a_ is not None and len(a_) != ((m_) * (k_)):
      raise ValueError("Array argument a is not long enough: Is %d, expected %d" % (len(a_),((m_) * (k_))))
    if a_ is None:
      raise ValueError("Argument a cannot be None")
    if a_ is None:
      raise ValueError("Argument a may not be None")
    if isinstance(a_, numpy.ndarray) and a_.dtype is numpy.dtype(numpy.float64) and a_.flags.contiguous:
      _a_copyarray = False
      _a_tmp = ctypes.cast(a_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif a_ is not None:
      _a_copyarray = True
      _a_np_tmp = numpy.zeros(len(a_),numpy.dtype(numpy.float64))
      _a_np_tmp[:] = a_
      assert _a_np_tmp.flags.contiguous
      _a_tmp = ctypes.cast(_a_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _a_copyarray = False
      _a_tmp = None
      
    _b_minlength = ((k_) * (n_))
    if ((k_) * (n_)) > 0 and b_ is not None and len(b_) != ((k_) * (n_)):
      raise ValueError("Array argument b is not long enough: Is %d, expected %d" % (len(b_),((k_) * (n_))))
    if b_ is None:
      raise ValueError("Argument b cannot be None")
    if b_ is None:
      raise ValueError("Argument b may not be None")
    if isinstance(b_, numpy.ndarray) and b_.dtype is numpy.dtype(numpy.float64) and b_.flags.contiguous:
      _b_copyarray = False
      _b_tmp = ctypes.cast(b_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif b_ is not None:
      _b_copyarray = True
      _b_np_tmp = numpy.zeros(len(b_),numpy.dtype(numpy.float64))
      _b_np_tmp[:] = b_
      assert _b_np_tmp.flags.contiguous
      _b_tmp = ctypes.cast(_b_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _b_copyarray = False
      _b_tmp = None
      
    _c_minlength = ((m_) * (n_))
    if ((m_) * (n_)) > 0 and c_ is not None and len(c_) != ((m_) * (n_)):
      raise ValueError("Array argument c is not long enough: Is %d, expected %d" % (len(c_),((m_) * (n_))))
    if isinstance(c_,numpy.ndarray) and not c_.flags.writeable:
      raise ValueError("Argument c must be writable")
    if c_ is None:
      raise ValueError("Argument c may not be None")
    if isinstance(c_, numpy.ndarray) and c_.dtype is numpy.dtype(numpy.float64) and c_.flags.contiguous:
      _c_copyarray = False
      _c_tmp = ctypes.cast(c_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif c_ is not None:
      _c_copyarray = True
      _c_np_tmp = numpy.zeros(len(c_),numpy.dtype(numpy.float64))
      _c_np_tmp[:] = c_
      assert _c_np_tmp.flags.contiguous
      _c_tmp = ctypes.cast(_c_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _c_copyarray = False
      _c_tmp = None
      
    res = __library__.MSK_XX_gemm(self.__nativep,transa_,transb_,m_,n_,k_,alpha_,_a_tmp,_b_tmp,beta_,_c_tmp)
    if res != 0:
      raise Error(rescode(res),Env.getcodedesc(rescode(res))[1])
    if _c_copyarray:
      c_[:] = _c_np_tmp
  @accepts(_accept_any,_accept_anyenum(uplo),_accept_anyenum(transpose),_make_int,_make_int,_make_double,_make_doublevector,_make_double,_accept_doublevector)
  @synchronized
  def syrk(self,uplo_,trans_,n_,k_,alpha_,a_,beta_,c_):
    """
    Performs a rank-k update of a symmetric matrix.
  
    syrk(self,uplo_,trans_,n_,k_,alpha_,a_,beta_,c_)
      uplo: mosek.uplo. Indicates whether the upper or lower triangular part of C is used.
      trans: mosek.transpose. Indicates whether the matrix A must be transposed.
      n: int. Specifies the order of C.
      k: int. Indicates the number of rows or columns of A, and its rank.
      alpha: double. A scalar value multiplying the result of the matrix multiplication.
      a: array of double. The pointer to the array storing matrix A in a column-major format.
      beta: double. A scalar value that multiplies C.
      c: array of double. The pointer to the array storing matrix C in a column-major format.
    """
    _a_minlength = ((n_) * (k_))
    if ((n_) * (k_)) > 0 and a_ is not None and len(a_) != ((n_) * (k_)):
      raise ValueError("Array argument a is not long enough: Is %d, expected %d" % (len(a_),((n_) * (k_))))
    if a_ is None:
      raise ValueError("Argument a cannot be None")
    if a_ is None:
      raise ValueError("Argument a may not be None")
    if isinstance(a_, numpy.ndarray) and a_.dtype is numpy.dtype(numpy.float64) and a_.flags.contiguous:
      _a_copyarray = False
      _a_tmp = ctypes.cast(a_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif a_ is not None:
      _a_copyarray = True
      _a_np_tmp = numpy.zeros(len(a_),numpy.dtype(numpy.float64))
      _a_np_tmp[:] = a_
      assert _a_np_tmp.flags.contiguous
      _a_tmp = ctypes.cast(_a_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _a_copyarray = False
      _a_tmp = None
      
    _c_minlength = ((n_) * (n_))
    if ((n_) * (n_)) > 0 and c_ is not None and len(c_) != ((n_) * (n_)):
      raise ValueError("Array argument c is not long enough: Is %d, expected %d" % (len(c_),((n_) * (n_))))
    if isinstance(c_,numpy.ndarray) and not c_.flags.writeable:
      raise ValueError("Argument c must be writable")
    if c_ is None:
      raise ValueError("Argument c may not be None")
    if isinstance(c_, numpy.ndarray) and c_.dtype is numpy.dtype(numpy.float64) and c_.flags.contiguous:
      _c_copyarray = False
      _c_tmp = ctypes.cast(c_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif c_ is not None:
      _c_copyarray = True
      _c_np_tmp = numpy.zeros(len(c_),numpy.dtype(numpy.float64))
      _c_np_tmp[:] = c_
      assert _c_np_tmp.flags.contiguous
      _c_tmp = ctypes.cast(_c_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _c_copyarray = False
      _c_tmp = None
      
    res = __library__.MSK_XX_syrk(self.__nativep,uplo_,trans_,n_,k_,alpha_,_a_tmp,beta_,_c_tmp)
    if res != 0:
      raise Error(rescode(res),Env.getcodedesc(rescode(res))[1])
    if _c_copyarray:
      c_[:] = _c_np_tmp
  @accepts(_accept_any,_make_int,_make_int,_make_double,_make_intvector,_make_longvector,_make_intvector,_make_doublevector)
  @synchronized
  def computesparsecholesky(self,multithread_,ordermethod_,tolsingular_,anzc_,aptrc_,asubc_,avalc_):
    """
    Computes a Cholesky factorization of sparse matrix.
  
    computesparsecholesky(self,multithread_,ordermethod_,tolsingular_,anzc_,aptrc_,asubc_,avalc_)
      multithread: int. If nonzero then the function may exploit multiple threads.
      ordermethod: int. If nonzero, then a sparsity preserving ordering will be employed.
      tolsingular: double. A positive parameter controlling when a pivot is declared zero.
      anzc: array of int. anzc[j] is the number of nonzeros in the jth column of A.
      aptrc: array of long. aptrc[j] is a pointer to the first element in column j.
      asubc: array of int. Row indexes for each column stored in increasing order.
      avalc: array of double. The value corresponding to row indexed stored in asubc.
    returns: perm,diag,lnzc,lptrc,lensubnval,lsubc,lvalc
      perm: int. Permutation array used to specify the permutation matrix P computed by the function.
      diag: double. The diagonal elements of matrix D.
      lnzc: int. lnzc[j] is the number of non zero elements in column j.
      lptrc: long. lptrc[j] is a pointer to the first row index and value in column j.
      lensubnval: long. Number of elements in lsubc and lvalc.
      lsubc: int. Row indexes for each column stored in increasing order.
      lvalc: double. The values corresponding to row indexed stored in lsubc.
    """
    n_ = None
    if n_ is None:
      n_ = len(anzc_)
    elif n_ != len(anzc_):
      raise IndexError("Inconsistent length of array anzc")
    if n_ is None:
      n_ = len(aptrc_)
    elif n_ != len(aptrc_):
      raise IndexError("Inconsistent length of array aptrc")
    if anzc_ is None:
      raise ValueError("Argument anzc cannot be None")
    if anzc_ is None:
      raise ValueError("Argument anzc may not be None")
    if isinstance(anzc_, numpy.ndarray) and anzc_.dtype is numpy.dtype(numpy.int32) and anzc_.flags.contiguous:
      _anzc_copyarray = False
      _anzc_tmp = ctypes.cast(anzc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif anzc_ is not None:
      _anzc_copyarray = True
      _anzc_np_tmp = numpy.zeros(len(anzc_),numpy.dtype(numpy.int32))
      _anzc_np_tmp[:] = anzc_
      assert _anzc_np_tmp.flags.contiguous
      _anzc_tmp = ctypes.cast(_anzc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _anzc_copyarray = False
      _anzc_tmp = None
      
    if aptrc_ is None:
      raise ValueError("Argument aptrc cannot be None")
    if aptrc_ is None:
      raise ValueError("Argument aptrc may not be None")
    if isinstance(aptrc_, numpy.ndarray) and aptrc_.dtype is numpy.dtype(numpy.int64) and aptrc_.flags.contiguous:
      _aptrc_copyarray = False
      _aptrc_tmp = ctypes.cast(aptrc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif aptrc_ is not None:
      _aptrc_copyarray = True
      _aptrc_np_tmp = numpy.zeros(len(aptrc_),numpy.dtype(numpy.int64))
      _aptrc_np_tmp[:] = aptrc_
      assert _aptrc_np_tmp.flags.contiguous
      _aptrc_tmp = ctypes.cast(_aptrc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _aptrc_copyarray = False
      _aptrc_tmp = None
      
    if asubc_ is None:
      raise ValueError("Argument asubc cannot be None")
    if asubc_ is None:
      raise ValueError("Argument asubc may not be None")
    if isinstance(asubc_, numpy.ndarray) and asubc_.dtype is numpy.dtype(numpy.int32) and asubc_.flags.contiguous:
      _asubc_copyarray = False
      _asubc_tmp = ctypes.cast(asubc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif asubc_ is not None:
      _asubc_copyarray = True
      _asubc_np_tmp = numpy.zeros(len(asubc_),numpy.dtype(numpy.int32))
      _asubc_np_tmp[:] = asubc_
      assert _asubc_np_tmp.flags.contiguous
      _asubc_tmp = ctypes.cast(_asubc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _asubc_copyarray = False
      _asubc_tmp = None
      
    if avalc_ is None:
      raise ValueError("Argument avalc cannot be None")
    if avalc_ is None:
      raise ValueError("Argument avalc may not be None")
    if isinstance(avalc_, numpy.ndarray) and avalc_.dtype is numpy.dtype(numpy.float64) and avalc_.flags.contiguous:
      _avalc_copyarray = False
      _avalc_tmp = ctypes.cast(avalc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif avalc_ is not None:
      _avalc_copyarray = True
      _avalc_np_tmp = numpy.zeros(len(avalc_),numpy.dtype(numpy.float64))
      _avalc_np_tmp[:] = avalc_
      assert _avalc_np_tmp.flags.contiguous
      _avalc_tmp = ctypes.cast(_avalc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _avalc_copyarray = False
      _avalc_tmp = None
      
    perm_ptr = ctypes.POINTER(ctypes.c_int32)()
    diag_ptr = ctypes.POINTER(ctypes.c_double)()
    lnzc_ptr = ctypes.POINTER(ctypes.c_int32)()
    lptrc_ptr = ctypes.POINTER(ctypes.c_int64)()
    lensubnval_ = ctypes.c_int64()
    lsubc_ptr = ctypes.POINTER(ctypes.c_int32)()
    lvalc_ptr = ctypes.POINTER(ctypes.c_double)()
    res = __library__.MSK_XX_computesparsecholesky(self.__nativep,multithread_,ordermethod_,tolsingular_,n_,_anzc_tmp,_aptrc_tmp,_asubc_tmp,_avalc_tmp,ctypes.byref(perm_ptr),ctypes.byref(diag_ptr),ctypes.byref(lnzc_ptr),ctypes.byref(lptrc_ptr),ctypes.byref(lensubnval_),ctypes.byref(lsubc_ptr),ctypes.byref(lvalc_ptr))
    if res != 0:
      raise Error(rescode(res),Env.getcodedesc(rescode(res))[1])
    perm_arr = perm_ptr[0:n_]
    __library__.MSK_XX_freeenv(self.__nativep,perm_ptr)
    diag_arr = diag_ptr[0:n_]
    __library__.MSK_XX_freeenv(self.__nativep,diag_ptr)
    lnzc_arr = lnzc_ptr[0:n_]
    __library__.MSK_XX_freeenv(self.__nativep,lnzc_ptr)
    lptrc_arr = lptrc_ptr[0:n_]
    __library__.MSK_XX_freeenv(self.__nativep,lptrc_ptr)
    lensubnval_ = lensubnval_.value
    _lensubnval_return_value = lensubnval_
    lsubc_arr = lsubc_ptr[0:lensubnval_]
    __library__.MSK_XX_freeenv(self.__nativep,lsubc_ptr)
    lvalc_arr = lvalc_ptr[0:lensubnval_]
    __library__.MSK_XX_freeenv(self.__nativep,lvalc_ptr)
    return (perm_arr,diag_arr,lnzc_arr,lptrc_arr,_lensubnval_return_value,lsubc_arr,lvalc_arr)
  @accepts(_accept_any,_accept_anyenum(transpose),_make_intvector,_make_longvector,_make_intvector,_make_doublevector,_accept_doublevector)
  @synchronized
  def sparsetriangularsolvedense(self,transposed_,lnzc_,lptrc_,lsubc_,lvalc_,b_):
    """
    Solves a sparse triangular system of linear equations.
  
    sparsetriangularsolvedense(self,transposed_,lnzc_,lptrc_,lsubc_,lvalc_,b_)
      transposed: mosek.transpose. Controls whether the solve is with L or the transposed L.
      lnzc: array of int. lnzc[j] is the number of nonzeros in column j.
      lptrc: array of long. lptrc[j] is a pointer to the first row index and value in column j.
      lsubc: array of int. Row indexes for each column stored sequentially.
      lvalc: array of double. The value corresponding to row indexed stored lsubc.
      b: array of double. The right-hand side of linear equation system to be solved as a dense vector.
    """
    n_ = None
    if n_ is None:
      n_ = len(b_)
    elif n_ != len(b_):
      raise IndexError("Inconsistent length of array b")
    if n_ is None:
      n_ = len(lnzc_)
    elif n_ != len(lnzc_):
      raise IndexError("Inconsistent length of array lnzc")
    if n_ is None:
      n_ = len(lptrc_)
    elif n_ != len(lptrc_):
      raise IndexError("Inconsistent length of array lptrc")
    _lnzc_minlength = (n_)
    if (n_) > 0 and lnzc_ is not None and len(lnzc_) != (n_):
      raise ValueError("Array argument lnzc is not long enough: Is %d, expected %d" % (len(lnzc_),(n_)))
    if lnzc_ is None:
      raise ValueError("Argument lnzc cannot be None")
    if lnzc_ is None:
      raise ValueError("Argument lnzc may not be None")
    if isinstance(lnzc_, numpy.ndarray) and lnzc_.dtype is numpy.dtype(numpy.int32) and lnzc_.flags.contiguous:
      _lnzc_copyarray = False
      _lnzc_tmp = ctypes.cast(lnzc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif lnzc_ is not None:
      _lnzc_copyarray = True
      _lnzc_np_tmp = numpy.zeros(len(lnzc_),numpy.dtype(numpy.int32))
      _lnzc_np_tmp[:] = lnzc_
      assert _lnzc_np_tmp.flags.contiguous
      _lnzc_tmp = ctypes.cast(_lnzc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _lnzc_copyarray = False
      _lnzc_tmp = None
      
    _lptrc_minlength = (n_)
    if (n_) > 0 and lptrc_ is not None and len(lptrc_) != (n_):
      raise ValueError("Array argument lptrc is not long enough: Is %d, expected %d" % (len(lptrc_),(n_)))
    if lptrc_ is None:
      raise ValueError("Argument lptrc cannot be None")
    if lptrc_ is None:
      raise ValueError("Argument lptrc may not be None")
    if isinstance(lptrc_, numpy.ndarray) and lptrc_.dtype is numpy.dtype(numpy.int64) and lptrc_.flags.contiguous:
      _lptrc_copyarray = False
      _lptrc_tmp = ctypes.cast(lptrc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif lptrc_ is not None:
      _lptrc_copyarray = True
      _lptrc_np_tmp = numpy.zeros(len(lptrc_),numpy.dtype(numpy.int64))
      _lptrc_np_tmp[:] = lptrc_
      assert _lptrc_np_tmp.flags.contiguous
      _lptrc_tmp = ctypes.cast(_lptrc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _lptrc_copyarray = False
      _lptrc_tmp = None
      
    lensubnval_ = None
    if lensubnval_ is None:
      lensubnval_ = len(lsubc_)
    elif lensubnval_ != len(lsubc_):
      raise IndexError("Inconsistent length of array lsubc")
    if lensubnval_ is None:
      lensubnval_ = len(lvalc_)
    elif lensubnval_ != len(lvalc_):
      raise IndexError("Inconsistent length of array lvalc")
    _lsubc_minlength = (lensubnval_)
    if (lensubnval_) > 0 and lsubc_ is not None and len(lsubc_) != (lensubnval_):
      raise ValueError("Array argument lsubc is not long enough: Is %d, expected %d" % (len(lsubc_),(lensubnval_)))
    if lsubc_ is None:
      raise ValueError("Argument lsubc cannot be None")
    if lsubc_ is None:
      raise ValueError("Argument lsubc may not be None")
    if isinstance(lsubc_, numpy.ndarray) and lsubc_.dtype is numpy.dtype(numpy.int32) and lsubc_.flags.contiguous:
      _lsubc_copyarray = False
      _lsubc_tmp = ctypes.cast(lsubc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif lsubc_ is not None:
      _lsubc_copyarray = True
      _lsubc_np_tmp = numpy.zeros(len(lsubc_),numpy.dtype(numpy.int32))
      _lsubc_np_tmp[:] = lsubc_
      assert _lsubc_np_tmp.flags.contiguous
      _lsubc_tmp = ctypes.cast(_lsubc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _lsubc_copyarray = False
      _lsubc_tmp = None
      
    _lvalc_minlength = (lensubnval_)
    if (lensubnval_) > 0 and lvalc_ is not None and len(lvalc_) != (lensubnval_):
      raise ValueError("Array argument lvalc is not long enough: Is %d, expected %d" % (len(lvalc_),(lensubnval_)))
    if lvalc_ is None:
      raise ValueError("Argument lvalc cannot be None")
    if lvalc_ is None:
      raise ValueError("Argument lvalc may not be None")
    if isinstance(lvalc_, numpy.ndarray) and lvalc_.dtype is numpy.dtype(numpy.float64) and lvalc_.flags.contiguous:
      _lvalc_copyarray = False
      _lvalc_tmp = ctypes.cast(lvalc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif lvalc_ is not None:
      _lvalc_copyarray = True
      _lvalc_np_tmp = numpy.zeros(len(lvalc_),numpy.dtype(numpy.float64))
      _lvalc_np_tmp[:] = lvalc_
      assert _lvalc_np_tmp.flags.contiguous
      _lvalc_tmp = ctypes.cast(_lvalc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _lvalc_copyarray = False
      _lvalc_tmp = None
      
    _b_minlength = (n_)
    if (n_) > 0 and b_ is not None and len(b_) != (n_):
      raise ValueError("Array argument b is not long enough: Is %d, expected %d" % (len(b_),(n_)))
    if isinstance(b_,numpy.ndarray) and not b_.flags.writeable:
      raise ValueError("Argument b must be writable")
    if b_ is None:
      raise ValueError("Argument b may not be None")
    if isinstance(b_, numpy.ndarray) and b_.dtype is numpy.dtype(numpy.float64) and b_.flags.contiguous:
      _b_copyarray = False
      _b_tmp = ctypes.cast(b_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif b_ is not None:
      _b_copyarray = True
      _b_np_tmp = numpy.zeros(len(b_),numpy.dtype(numpy.float64))
      _b_np_tmp[:] = b_
      assert _b_np_tmp.flags.contiguous
      _b_tmp = ctypes.cast(_b_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _b_copyarray = False
      _b_tmp = None
      
    res = __library__.MSK_XX_sparsetriangularsolvedense(self.__nativep,transposed_,n_,_lnzc_tmp,_lptrc_tmp,lensubnval_,_lsubc_tmp,_lvalc_tmp,_b_tmp)
    if res != 0:
      raise Error(rescode(res),Env.getcodedesc(rescode(res))[1])
    if _b_copyarray:
      b_[:] = _b_np_tmp
  @accepts(_accept_any,_accept_anyenum(uplo),_make_int,_accept_doublevector)
  @synchronized
  def potrf(self,uplo_,n_,a_):
    """
    Computes a Cholesky factorization of a dense matrix.
  
    potrf(self,uplo_,n_,a_)
      uplo: mosek.uplo. Indicates whether the upper or lower triangular part of the matrix is stored.
      n: int. Dimension of the symmetric matrix.
      a: array of double. A symmetric matrix stored in column-major order.
    """
    _a_minlength = ((n_) * (n_))
    if ((n_) * (n_)) > 0 and a_ is not None and len(a_) != ((n_) * (n_)):
      raise ValueError("Array argument a is not long enough: Is %d, expected %d" % (len(a_),((n_) * (n_))))
    if isinstance(a_,numpy.ndarray) and not a_.flags.writeable:
      raise ValueError("Argument a must be writable")
    if a_ is None:
      raise ValueError("Argument a may not be None")
    if isinstance(a_, numpy.ndarray) and a_.dtype is numpy.dtype(numpy.float64) and a_.flags.contiguous:
      _a_copyarray = False
      _a_tmp = ctypes.cast(a_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif a_ is not None:
      _a_copyarray = True
      _a_np_tmp = numpy.zeros(len(a_),numpy.dtype(numpy.float64))
      _a_np_tmp[:] = a_
      assert _a_np_tmp.flags.contiguous
      _a_tmp = ctypes.cast(_a_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _a_copyarray = False
      _a_tmp = None
      
    res = __library__.MSK_XX_potrf(self.__nativep,uplo_,n_,_a_tmp)
    if res != 0:
      raise Error(rescode(res),Env.getcodedesc(rescode(res))[1])
    if _a_copyarray:
      a_[:] = _a_np_tmp
  @accepts(_accept_any,_accept_anyenum(uplo),_make_int,_make_doublevector,_accept_doublevector)
  @synchronized
  def syeig(self,uplo_,n_,a_,w_):
    """
    Computes all eigenvalues of a symmetric dense matrix.
  
    syeig(self,uplo_,n_,a_,w_)
      uplo: mosek.uplo. Indicates whether the upper or lower triangular part is used.
      n: int. Dimension of the symmetric input matrix.
      a: array of double. Input matrix A.
      w: array of double. Array of length at least n containing the eigenvalues of A.
    """
    _a_minlength = ((n_) * (n_))
    if ((n_) * (n_)) > 0 and a_ is not None and len(a_) != ((n_) * (n_)):
      raise ValueError("Array argument a is not long enough: Is %d, expected %d" % (len(a_),((n_) * (n_))))
    if a_ is None:
      raise ValueError("Argument a cannot be None")
    if a_ is None:
      raise ValueError("Argument a may not be None")
    if isinstance(a_, numpy.ndarray) and a_.dtype is numpy.dtype(numpy.float64) and a_.flags.contiguous:
      _a_copyarray = False
      _a_tmp = ctypes.cast(a_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif a_ is not None:
      _a_copyarray = True
      _a_np_tmp = numpy.zeros(len(a_),numpy.dtype(numpy.float64))
      _a_np_tmp[:] = a_
      assert _a_np_tmp.flags.contiguous
      _a_tmp = ctypes.cast(_a_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _a_copyarray = False
      _a_tmp = None
      
    _w_minlength = (n_)
    if (n_) > 0 and w_ is not None and len(w_) != (n_):
      raise ValueError("Array argument w is not long enough: Is %d, expected %d" % (len(w_),(n_)))
    if isinstance(w_,numpy.ndarray) and not w_.flags.writeable:
      raise ValueError("Argument w must be writable")
    if w_ is None:
      raise ValueError("Argument w may not be None")
    if isinstance(w_, numpy.ndarray) and w_.dtype is numpy.dtype(numpy.float64) and w_.flags.contiguous:
      _w_copyarray = False
      _w_tmp = ctypes.cast(w_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif w_ is not None:
      _w_copyarray = True
      _w_np_tmp = numpy.zeros(len(w_),numpy.dtype(numpy.float64))
      _w_np_tmp[:] = w_
      assert _w_np_tmp.flags.contiguous
      _w_tmp = ctypes.cast(_w_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _w_copyarray = False
      _w_tmp = None
      
    res = __library__.MSK_XX_syeig(self.__nativep,uplo_,n_,_a_tmp,_w_tmp)
    if res != 0:
      raise Error(rescode(res),Env.getcodedesc(rescode(res))[1])
    if _w_copyarray:
      w_[:] = _w_np_tmp
  @accepts(_accept_any,_accept_anyenum(uplo),_make_int,_accept_doublevector,_accept_doublevector)
  @synchronized
  def syevd(self,uplo_,n_,a_,w_):
    """
    Computes all the eigenvalues and eigenvectors of a symmetric dense matrix, and thus its eigenvalue decomposition.
  
    syevd(self,uplo_,n_,a_,w_)
      uplo: mosek.uplo. Indicates whether the upper or lower triangular part is used.
      n: int. Dimension of the symmetric input matrix.
      a: array of double. Input matrix A.
      w: array of double. Array of length at least n containing the eigenvalues of A.
    """
    _a_minlength = ((n_) * (n_))
    if ((n_) * (n_)) > 0 and a_ is not None and len(a_) != ((n_) * (n_)):
      raise ValueError("Array argument a is not long enough: Is %d, expected %d" % (len(a_),((n_) * (n_))))
    if isinstance(a_,numpy.ndarray) and not a_.flags.writeable:
      raise ValueError("Argument a must be writable")
    if a_ is None:
      raise ValueError("Argument a may not be None")
    if isinstance(a_, numpy.ndarray) and a_.dtype is numpy.dtype(numpy.float64) and a_.flags.contiguous:
      _a_copyarray = False
      _a_tmp = ctypes.cast(a_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif a_ is not None:
      _a_copyarray = True
      _a_np_tmp = numpy.zeros(len(a_),numpy.dtype(numpy.float64))
      _a_np_tmp[:] = a_
      assert _a_np_tmp.flags.contiguous
      _a_tmp = ctypes.cast(_a_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _a_copyarray = False
      _a_tmp = None
      
    _w_minlength = (n_)
    if (n_) > 0 and w_ is not None and len(w_) != (n_):
      raise ValueError("Array argument w is not long enough: Is %d, expected %d" % (len(w_),(n_)))
    if isinstance(w_,numpy.ndarray) and not w_.flags.writeable:
      raise ValueError("Argument w must be writable")
    if w_ is None:
      raise ValueError("Argument w may not be None")
    if isinstance(w_, numpy.ndarray) and w_.dtype is numpy.dtype(numpy.float64) and w_.flags.contiguous:
      _w_copyarray = False
      _w_tmp = ctypes.cast(w_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif w_ is not None:
      _w_copyarray = True
      _w_np_tmp = numpy.zeros(len(w_),numpy.dtype(numpy.float64))
      _w_np_tmp[:] = w_
      assert _w_np_tmp.flags.contiguous
      _w_tmp = ctypes.cast(_w_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _w_copyarray = False
      _w_tmp = None
      
    res = __library__.MSK_XX_syevd(self.__nativep,uplo_,n_,_a_tmp,_w_tmp)
    if res != 0:
      raise Error(rescode(res),Env.getcodedesc(rescode(res))[1])
    if _a_copyarray:
      a_[:] = _a_np_tmp
    if _w_copyarray:
      w_[:] = _w_np_tmp
  @staticmethod
  @accepts()
  def licensecleanup():
    """
    Stops all threads and delete all handles used by the license system.
  
    licensecleanup()
    """
    res = __library__.MSK_XX_licensecleanup()
    if res != 0:
      raise Error(rescode(res),Env.getcodedesc(rescode(res))[1])


class Task:
  """
  The MOSEK task class. This object contains information about one optimization problem.
  """
  
  def __init__(self,env=None,maxnumcon=0,maxnumvar=0,nativep=None,other=None):
      """
      Construct a new Task object.
      
      Task(env=None,maxnumcon=0,maxnumvar=0,nativep=None,other=None)
        env: mosek.Env. 
        maxnumcon: int. Reserve space for this number of constraints. Default is 0. 
        maxnumcvar: int. Reserve space for this number of variables. Default is 0. 
        nativep: native pointer. For internal use only.
        other: mosek.Task. Another task.
      
      Valid usage:
        Specifying "env", and optionally "maxnumcon" and "maxnumvar" will create a new Task.
        Specifying "nativep" will create a new Task from the native mosek task defined by the pointer.
        Specifying "other" will create a new Task as a copy of the other task. 
      """
      self.__library = __library__
      self.__nativep = None
      self.__schandle = None

      self._lock = threading.RLock()

      if isinstance(env,Task):
          other = env
          env = None
      
      try: 
          if nativep is not None:
              self.__nativep = nativep
              res = 0
          elif other is not None:
              self.__nativep = ctypes.c_void_p()
              res = self.__library.MSK_XX_clonetask(other.__nativep, ctypes.byref(self.__nativep))
          else:
              if not isinstance(env,Env):
                  raise TypeError('Expected an Env for argument')
              self.__nativep = ctypes.c_void_p()
              res = self.__library.MSK_XX_maketask(env._getNativeP(),maxnumcon,maxnumvar,ctypes.byref(self.__nativep))
          if res != 0:
              raise Error(rescode(res),"Error %d" % res)

          # user progress function:
          self.__progress_func = None
          self.__infocallback_func = None
          # callback proxy function definition:
          def progress_proxy(nativep, handle, caller, dinfptr, iinfptr, liinfptr):
              r = 0
              try:
                  if self.__progress_func or self.__infocallback_func:
                      caller = callbackcode(caller)
                      f = self.__infocallback_func
                      if f is not None:
                          r = f(caller,
                                ctypes.cast(dinfptr, ctypes.POINTER(ctypes.c_double))[:len(dinfitem._values)]    if dinfptr  is not None else None,
                                ctypes.cast(iinfptr, ctypes.POINTER(ctypes.c_int))[:len(iinfitem._values)]       if iinfptr  is not None else None,
                                ctypes.cast(liinfptr,ctypes.POINTER(ctypes.c_longlong))[:len(liinfitem._values)] if liinfptr is not None else None,
                            )
                      f = self.__progress_func
                      if f is not None:
                          r = f(caller)
                      if not isinstance(r,int):
                          r = 0
              except:
                  import traceback
                  traceback.print_exc()
                  return -1
              return r
          # callback proxy C wrapper:
          self.__progress_cb = __progress_cb_type__(progress_proxy)
        
          # user stream functions: 
          self.__stream_func   = 4 * [ None ]
          # strema proxy functions and wrappers:
          self.__stream_cb   = 4 * [ None ]
          for whichstream in xrange(4): 
              # Note: Apparently closures doesn't work when the function is wrapped in a C function... So we use default parameter value instead.
              def stream_proxy(handle, msg, whichstream=whichstream):
                  func = self.__stream_func[whichstream]
                  try:
                      if func is not None:
                          func(msg)
                  except:
                      pass
              self.__stream_cb[whichstream] = __stream_cb_type__(stream_proxy)
          assert self.__nativep



      except:
          #import traceback
          #traceback.print_exc()
          if hasattr(self,'_Task__nativep') and self.__nativep is not None:
              #print "DELETE TASK 2",id(self)
              self.__library.MSK_XX_deletetask(ctypes.byref(self.__nativep))
              self.__nativep = None
          raise
  
  @synchronized
  def __del__(self):
      #print "DELETE TASK 1",id(self)
      if self.__nativep is not None:
          self.__library.MSK_XX_deletetask(ctypes.byref(self.__nativep))
          del self.__library
          del self.__schandle
          del self.__progress_func
          del self.__progress_cb
          del self.__stream_func
          del self.__stream_cb
      self.__nativep = None

  def __enter__(self):
      return self

  def __exit__(self,exc_type,exc_value,traceback):
      self.__del__()
      

  @synchronized
  def __getlasterror(self,res):
      msglen = ctypes.c_size_t(1024)
      lasterr = ctypes.c_int()
      r = self.__library.MSK_XX_getlasterror64(self.__nativep, ctypes.byref(lasterr), 0, ctypes.byref(msglen),None)
      if r == 0:
          #msg = (ctypes.c_char * (msglen.value+1))()
          len = (msglen.value+1)
          msg = ctypes.create_string_buffer(len)
          r = self.__library.MSK_XX_getlasterror64(self.__nativep, ctypes.byref(lasterr), len, None,msg)
          if r == 0:
              result,msg = lasterr.value,msg.value.decode('utf-8',errors="replace")
          else:
              result,msg = lasterr.value,''
      else:
          result,msg = res,''
      return result,msg


  @synchronized
  def set_Progress(self,func):
      """
      Set the progress callback function. If func is None, progress callbacks are detached and disabled.
      """
      if func is None:
          self.__progress_func = None
          #res = self.__library.MSK_XX_putcallbackfunc(self.__nativep,None,None)
      else:
          self.__progress_func = func          
          res = self.__library.MSK_XX_putcallbackfunc(self.__nativep,self.__progress_cb,None)

  @synchronized
  def set_InfoCallback(self,func):
      """
      Set the progress callback function. If func is None, progress callbacks are detached and disabled.
      """
      if func is None:
          self.__infocallback_func = None
          #res = self.__library.MSK_XX_putcallbackfunc(self.__nativep,None,None)
      else:
          self.__infocallback_func = func          
          res = self.__library.MSK_XX_putcallbackfunc(self.__nativep,self.__progress_cb,None)
          
  @synchronized
  def set_Stream(self,whichstream,func):
      if isinstance(whichstream, streamtype):
          if func is None:
              self.__stream_func[whichstream] = None
              res = self.__library.MSK_XX_linkfunctotaskstream(self.__nativep,whichstream,None,ctypes.cast(None,__stream_cb_type__))
          else:
              self.__stream_func[whichstream] = func
              res = self.__library.MSK_XX_linkfunctotaskstream(self.__nativep,whichstream,None,self.__stream_cb[whichstream])
      else:
          raise TypeError("Invalid stream %s" % whichstream)  

  @accepts(_accept_any,_accept_anyenum(streamtype))
  @synchronized
  def analyzeproblem(self,whichstream_):
    """
    Analyze the data of a task.
  
    analyzeproblem(self,whichstream_)
      whichstream: mosek.streamtype. Index of the stream.
    """
    res = __library__.MSK_XX_analyzeproblem(self.__nativep,whichstream_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(streamtype),_accept_anyenum(nametype))
  @synchronized
  def analyzenames(self,whichstream_,nametype_):
    """
    Analyze the names and issue an error for the first invalid name.
  
    analyzenames(self,whichstream_,nametype_)
      whichstream: mosek.streamtype. Index of the stream.
      nametype: mosek.nametype. The type of names e.g. valid in MPS or LP files.
    """
    res = __library__.MSK_XX_analyzenames(self.__nativep,whichstream_,nametype_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(streamtype),_accept_anyenum(soltype))
  @synchronized
  def analyzesolution(self,whichstream_,whichsol_):
    """
    Print information related to the quality of the solution.
  
    analyzesolution(self,whichstream_,whichsol_)
      whichstream: mosek.streamtype. Index of the stream.
      whichsol: mosek.soltype. Selects a solution.
    """
    res = __library__.MSK_XX_analyzesolution(self.__nativep,whichstream_,whichsol_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_intvector)
  @synchronized
  def initbasissolve(self,basis_):
    """
    Prepare a task for basis solver.
  
    initbasissolve(self,basis_)
      basis: array of int. The array of basis indexes to use.
    """
    _basis_minlength = self.getnumcon()
    if self.getnumcon() > 0 and basis_ is not None and len(basis_) != self.getnumcon():
      raise ValueError("Array argument basis is not long enough: Is %d, expected %d" % (len(basis_),self.getnumcon()))
    if isinstance(basis_,numpy.ndarray) and not basis_.flags.writeable:
      raise ValueError("Argument basis must be writable")
    if isinstance(basis_, numpy.ndarray) and basis_.dtype is numpy.dtype(numpy.int32) and basis_.flags.contiguous:
      _basis_copyarray = False
      _basis_tmp = ctypes.cast(basis_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif basis_ is not None:
      _basis_copyarray = True
      _basis_np_tmp = numpy.zeros(len(basis_),numpy.dtype(numpy.int32))
      _basis_np_tmp[:] = basis_
      assert _basis_np_tmp.flags.contiguous
      _basis_tmp = ctypes.cast(_basis_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _basis_copyarray = False
      _basis_tmp = None
      
    res = __library__.MSK_XX_initbasissolve(self.__nativep,_basis_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _basis_copyarray:
      basis_[:] = _basis_np_tmp
  @accepts(_accept_any,_make_int,_make_int,_accept_intvector,_accept_doublevector)
  @synchronized
  def solvewithbasis(self,transp_,numnz_,sub_,val_):
    """
    Solve a linear equation system involving a basis matrix.
  
    solvewithbasis(self,transp_,numnz_,sub_,val_)
      transp: int. Controls which problem formulation is solved.
      numnz: int. Input (number of non-zeros in right-hand side) and output (number of non-zeros in solution vector).
      sub: array of int. Input (indexes of non-zeros in right-hand side) and output (indexes of non-zeros in solution vector).
      val: array of double. Input (right-hand side values) and output (solution vector values).
    returns: numnz
      numnz: int. Input (number of non-zeros in right-hand side) and output (number of non-zeros in solution vector).
    """
    _numnz_tmp = ctypes.c_int32(numnz_)
    _sub_minlength = self.getnumcon()
    if self.getnumcon() > 0 and sub_ is not None and len(sub_) != self.getnumcon():
      raise ValueError("Array argument sub is not long enough: Is %d, expected %d" % (len(sub_),self.getnumcon()))
    if isinstance(sub_,numpy.ndarray) and not sub_.flags.writeable:
      raise ValueError("Argument sub must be writable")
    if isinstance(sub_, numpy.ndarray) and sub_.dtype is numpy.dtype(numpy.int32) and sub_.flags.contiguous:
      _sub_copyarray = False
      _sub_tmp = ctypes.cast(sub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif sub_ is not None:
      _sub_copyarray = True
      _sub_np_tmp = numpy.zeros(len(sub_),numpy.dtype(numpy.int32))
      _sub_np_tmp[:] = sub_
      assert _sub_np_tmp.flags.contiguous
      _sub_tmp = ctypes.cast(_sub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _sub_copyarray = False
      _sub_tmp = None
      
    _val_minlength = self.getnumcon()
    if self.getnumcon() > 0 and val_ is not None and len(val_) != self.getnumcon():
      raise ValueError("Array argument val is not long enough: Is %d, expected %d" % (len(val_),self.getnumcon()))
    if isinstance(val_,numpy.ndarray) and not val_.flags.writeable:
      raise ValueError("Argument val must be writable")
    if isinstance(val_, numpy.ndarray) and val_.dtype is numpy.dtype(numpy.float64) and val_.flags.contiguous:
      _val_copyarray = False
      _val_tmp = ctypes.cast(val_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif val_ is not None:
      _val_copyarray = True
      _val_np_tmp = numpy.zeros(len(val_),numpy.dtype(numpy.float64))
      _val_np_tmp[:] = val_
      assert _val_np_tmp.flags.contiguous
      _val_tmp = ctypes.cast(_val_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _val_copyarray = False
      _val_tmp = None
      
    res = __library__.MSK_XX_solvewithbasis(self.__nativep,transp_,ctypes.byref(_numnz_tmp),_sub_tmp,_val_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _numnz_return_value = _numnz_tmp.value
    if _sub_copyarray:
      sub_[:] = _sub_np_tmp
    if _val_copyarray:
      val_[:] = _val_np_tmp
    return (_numnz_return_value)
  @accepts(_accept_any)
  @synchronized
  def basiscond(self):
    """
    Computes conditioning information for the basis matrix.
  
    basiscond(self)
    returns: nrmbasis,nrminvbasis
      nrmbasis: double. An estimate for the 1-norm of the basis.
      nrminvbasis: double. An estimate for the 1-norm of the inverse of the basis.
    """
    nrmbasis_ = ctypes.c_double()
    nrminvbasis_ = ctypes.c_double()
    res = __library__.MSK_XX_basiscond(self.__nativep,ctypes.byref(nrmbasis_),ctypes.byref(nrminvbasis_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    nrmbasis_ = nrmbasis_.value
    _nrmbasis_return_value = nrmbasis_
    nrminvbasis_ = nrminvbasis_.value
    _nrminvbasis_return_value = nrminvbasis_
    return (_nrmbasis_return_value,_nrminvbasis_return_value)
  @accepts(_accept_any,_make_int)
  @synchronized
  def appendcons(self,num_):
    """
    Appends a number of constraints to the optimization task.
  
    appendcons(self,num_)
      num: int. Number of constraints which should be appended.
    """
    res = __library__.MSK_XX_appendcons(self.__nativep,num_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int)
  @synchronized
  def appendvars(self,num_):
    """
    Appends a number of variables to the optimization task.
  
    appendvars(self,num_)
      num: int. Number of variables which should be appended.
    """
    res = __library__.MSK_XX_appendvars(self.__nativep,num_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_intvector)
  @synchronized
  def removecons(self,subset_):
    """
    Removes a number of constraints.
  
    removecons(self,subset_)
      subset: array of int. Indexes of constraints which should be removed.
    """
    num_ = None
    if num_ is None:
      num_ = len(subset_)
    elif num_ != len(subset_):
      raise IndexError("Inconsistent length of array subset")
    if subset_ is None:
      raise ValueError("Argument subset cannot be None")
    if subset_ is None:
      raise ValueError("Argument subset may not be None")
    if isinstance(subset_, numpy.ndarray) and subset_.dtype is numpy.dtype(numpy.int32) and subset_.flags.contiguous:
      _subset_copyarray = False
      _subset_tmp = ctypes.cast(subset_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subset_ is not None:
      _subset_copyarray = True
      _subset_np_tmp = numpy.zeros(len(subset_),numpy.dtype(numpy.int32))
      _subset_np_tmp[:] = subset_
      assert _subset_np_tmp.flags.contiguous
      _subset_tmp = ctypes.cast(_subset_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subset_copyarray = False
      _subset_tmp = None
      
    res = __library__.MSK_XX_removecons(self.__nativep,num_,_subset_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_intvector)
  @synchronized
  def removevars(self,subset_):
    """
    Removes a number of variables.
  
    removevars(self,subset_)
      subset: array of int. Indexes of variables which should be removed.
    """
    num_ = None
    if num_ is None:
      num_ = len(subset_)
    elif num_ != len(subset_):
      raise IndexError("Inconsistent length of array subset")
    if subset_ is None:
      raise ValueError("Argument subset cannot be None")
    if subset_ is None:
      raise ValueError("Argument subset may not be None")
    if isinstance(subset_, numpy.ndarray) and subset_.dtype is numpy.dtype(numpy.int32) and subset_.flags.contiguous:
      _subset_copyarray = False
      _subset_tmp = ctypes.cast(subset_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subset_ is not None:
      _subset_copyarray = True
      _subset_np_tmp = numpy.zeros(len(subset_),numpy.dtype(numpy.int32))
      _subset_np_tmp[:] = subset_
      assert _subset_np_tmp.flags.contiguous
      _subset_tmp = ctypes.cast(_subset_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subset_copyarray = False
      _subset_tmp = None
      
    res = __library__.MSK_XX_removevars(self.__nativep,num_,_subset_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_intvector)
  @synchronized
  def removebarvars(self,subset_):
    """
    Removes a number of symmetric matrices.
  
    removebarvars(self,subset_)
      subset: array of int. Indexes of symmetric matrices which should be removed.
    """
    num_ = None
    if num_ is None:
      num_ = len(subset_)
    elif num_ != len(subset_):
      raise IndexError("Inconsistent length of array subset")
    if subset_ is None:
      raise ValueError("Argument subset cannot be None")
    if subset_ is None:
      raise ValueError("Argument subset may not be None")
    if isinstance(subset_, numpy.ndarray) and subset_.dtype is numpy.dtype(numpy.int32) and subset_.flags.contiguous:
      _subset_copyarray = False
      _subset_tmp = ctypes.cast(subset_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subset_ is not None:
      _subset_copyarray = True
      _subset_np_tmp = numpy.zeros(len(subset_),numpy.dtype(numpy.int32))
      _subset_np_tmp[:] = subset_
      assert _subset_np_tmp.flags.contiguous
      _subset_tmp = ctypes.cast(_subset_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subset_copyarray = False
      _subset_tmp = None
      
    res = __library__.MSK_XX_removebarvars(self.__nativep,num_,_subset_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_intvector)
  @synchronized
  def removecones(self,subset_):
    """
    Removes a number of conic constraints from the problem.
  
    removecones(self,subset_)
      subset: array of int. Indexes of cones which should be removed.
    """
    num_ = None
    if num_ is None:
      num_ = len(subset_)
    elif num_ != len(subset_):
      raise IndexError("Inconsistent length of array subset")
    if subset_ is None:
      raise ValueError("Argument subset cannot be None")
    if subset_ is None:
      raise ValueError("Argument subset may not be None")
    if isinstance(subset_, numpy.ndarray) and subset_.dtype is numpy.dtype(numpy.int32) and subset_.flags.contiguous:
      _subset_copyarray = False
      _subset_tmp = ctypes.cast(subset_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subset_ is not None:
      _subset_copyarray = True
      _subset_np_tmp = numpy.zeros(len(subset_),numpy.dtype(numpy.int32))
      _subset_np_tmp[:] = subset_
      assert _subset_np_tmp.flags.contiguous
      _subset_tmp = ctypes.cast(_subset_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subset_copyarray = False
      _subset_tmp = None
      
    res = __library__.MSK_XX_removecones(self.__nativep,num_,_subset_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_intvector)
  @synchronized
  def appendbarvars(self,dim_):
    """
    Appends semidefinite variables to the problem.
  
    appendbarvars(self,dim_)
      dim: array of int. Dimensions of symmetric matrix variables to be added.
    """
    num_ = None
    if num_ is None:
      num_ = len(dim_)
    elif num_ != len(dim_):
      raise IndexError("Inconsistent length of array dim")
    if dim_ is None:
      raise ValueError("Argument dim cannot be None")
    if dim_ is None:
      raise ValueError("Argument dim may not be None")
    if isinstance(dim_, numpy.ndarray) and dim_.dtype is numpy.dtype(numpy.int32) and dim_.flags.contiguous:
      _dim_copyarray = False
      _dim_tmp = ctypes.cast(dim_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif dim_ is not None:
      _dim_copyarray = True
      _dim_np_tmp = numpy.zeros(len(dim_),numpy.dtype(numpy.int32))
      _dim_np_tmp[:] = dim_
      assert _dim_np_tmp.flags.contiguous
      _dim_tmp = ctypes.cast(_dim_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _dim_copyarray = False
      _dim_tmp = None
      
    res = __library__.MSK_XX_appendbarvars(self.__nativep,num_,_dim_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(conetype),_make_double,_make_intvector)
  @synchronized
  def appendcone(self,ct_,conepar_,submem_):
    """
    Appends a new conic constraint to the problem.
  
    appendcone(self,ct_,conepar_,submem_)
      ct: mosek.conetype. Specifies the type of the cone.
      conepar: double. For the power cone it denotes the exponent alpha. For other cone types it is unused and can be set to 0.
      submem: array of int. Variable subscripts of the members in the cone.
    """
    nummem_ = None
    if nummem_ is None:
      nummem_ = len(submem_)
    elif nummem_ != len(submem_):
      raise IndexError("Inconsistent length of array submem")
    if submem_ is None:
      raise ValueError("Argument submem cannot be None")
    if submem_ is None:
      raise ValueError("Argument submem may not be None")
    if isinstance(submem_, numpy.ndarray) and submem_.dtype is numpy.dtype(numpy.int32) and submem_.flags.contiguous:
      _submem_copyarray = False
      _submem_tmp = ctypes.cast(submem_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif submem_ is not None:
      _submem_copyarray = True
      _submem_np_tmp = numpy.zeros(len(submem_),numpy.dtype(numpy.int32))
      _submem_np_tmp[:] = submem_
      assert _submem_np_tmp.flags.contiguous
      _submem_tmp = ctypes.cast(_submem_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _submem_copyarray = False
      _submem_tmp = None
      
    res = __library__.MSK_XX_appendcone(self.__nativep,ct_,conepar_,nummem_,_submem_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(conetype),_make_double,_make_int,_make_int)
  @synchronized
  def appendconeseq(self,ct_,conepar_,nummem_,j_):
    """
    Appends a new conic constraint to the problem.
  
    appendconeseq(self,ct_,conepar_,nummem_,j_)
      ct: mosek.conetype. Specifies the type of the cone.
      conepar: double. For the power cone it denotes the exponent alpha. For other cone types it is unused and can be set to 0.
      nummem: int. Number of member variables in the cone.
      j: int. Index of the first variable in the conic constraint.
    """
    res = __library__.MSK_XX_appendconeseq(self.__nativep,ct_,conepar_,nummem_,j_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_anyenumvector(conetype),_make_doublevector,_make_intvector,_make_int)
  @synchronized
  def appendconesseq(self,ct_,conepar_,nummem_,j_):
    """
    Appends multiple conic constraints to the problem.
  
    appendconesseq(self,ct_,conepar_,nummem_,j_)
      ct: array of mosek.conetype. Specifies the type of the cone.
      conepar: array of double. For the power cone it denotes the exponent alpha. For other cone types it is unused and can be set to 0.
      nummem: array of int. Numbers of member variables in the cones.
      j: int. Index of the first variable in the first cone to be appended.
    """
    num_ = None
    if num_ is None:
      num_ = len(ct_)
    elif num_ != len(ct_):
      raise IndexError("Inconsistent length of array ct")
    if num_ is None:
      num_ = len(conepar_)
    elif num_ != len(conepar_):
      raise IndexError("Inconsistent length of array conepar")
    if num_ is None:
      num_ = len(nummem_)
    elif num_ != len(nummem_):
      raise IndexError("Inconsistent length of array nummem")
    if ct_ is None:
      raise ValueError("Argument ct cannot be None")
    if ct_ is None:
      raise ValueError("Argument ct may not be None")
    if ct_ is not None:
        _ct_tmp = (ctypes.c_int32 * len(ct_))(*ct_)
    else:
        _ct_tmp = None
    if conepar_ is None:
      raise ValueError("Argument conepar cannot be None")
    if conepar_ is None:
      raise ValueError("Argument conepar may not be None")
    if isinstance(conepar_, numpy.ndarray) and conepar_.dtype is numpy.dtype(numpy.float64) and conepar_.flags.contiguous:
      _conepar_copyarray = False
      _conepar_tmp = ctypes.cast(conepar_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif conepar_ is not None:
      _conepar_copyarray = True
      _conepar_np_tmp = numpy.zeros(len(conepar_),numpy.dtype(numpy.float64))
      _conepar_np_tmp[:] = conepar_
      assert _conepar_np_tmp.flags.contiguous
      _conepar_tmp = ctypes.cast(_conepar_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _conepar_copyarray = False
      _conepar_tmp = None
      
    if nummem_ is None:
      raise ValueError("Argument nummem cannot be None")
    if nummem_ is None:
      raise ValueError("Argument nummem may not be None")
    if isinstance(nummem_, numpy.ndarray) and nummem_.dtype is numpy.dtype(numpy.int32) and nummem_.flags.contiguous:
      _nummem_copyarray = False
      _nummem_tmp = ctypes.cast(nummem_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif nummem_ is not None:
      _nummem_copyarray = True
      _nummem_np_tmp = numpy.zeros(len(nummem_),numpy.dtype(numpy.int32))
      _nummem_np_tmp[:] = nummem_
      assert _nummem_np_tmp.flags.contiguous
      _nummem_tmp = ctypes.cast(_nummem_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _nummem_copyarray = False
      _nummem_tmp = None
      
    res = __library__.MSK_XX_appendconesseq(self.__nativep,num_,_ct_tmp,_conepar_tmp,_nummem_tmp,j_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_make_int,_make_int,_make_double)
  @synchronized
  def chgconbound(self,i_,lower_,finite_,value_):
    """
    Changes the bounds for one constraint.
  
    chgconbound(self,i_,lower_,finite_,value_)
      i: int. Index of the constraint for which the bounds should be changed.
      lower: int. If non-zero, then the lower bound is changed, otherwise the upper bound is changed.
      finite: int. If non-zero, then the given value is assumed to be finite.
      value: double. New value for the bound.
    """
    res = __library__.MSK_XX_chgconbound(self.__nativep,i_,lower_,finite_,value_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_make_int,_make_int,_make_double)
  @synchronized
  def chgvarbound(self,j_,lower_,finite_,value_):
    """
    Changes the bounds for one variable.
  
    chgvarbound(self,j_,lower_,finite_,value_)
      j: int. Index of the variable for which the bounds should be changed.
      lower: int. If non-zero, then the lower bound is changed, otherwise the upper bound is changed.
      finite: int. If non-zero, then the given value is assumed to be finite.
      value: double. New value for the bound.
    """
    res = __library__.MSK_XX_chgvarbound(self.__nativep,j_,lower_,finite_,value_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_make_int)
  @synchronized
  def getaij(self,i_,j_):
    """
    Obtains a single coefficient in linear constraint matrix.
  
    getaij(self,i_,j_)
      i: int. Row index of the coefficient to be returned.
      j: int. Column index of the coefficient to be returned.
    returns: aij
      aij: double. Returns the requested coefficient.
    """
    aij_ = ctypes.c_double()
    res = __library__.MSK_XX_getaij(self.__nativep,i_,j_,ctypes.byref(aij_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    aij_ = aij_.value
    _aij_return_value = aij_
    return (_aij_return_value)
  @accepts(_accept_any,_make_int,_make_int,_make_int,_make_int)
  @synchronized
  def getapiecenumnz(self,firsti_,lasti_,firstj_,lastj_):
    """
    Obtains the number non-zeros in a rectangular piece of the linear constraint matrix.
  
    getapiecenumnz(self,firsti_,lasti_,firstj_,lastj_)
      firsti: int. Index of the first row in the rectangular piece.
      lasti: int. Index of the last row plus one in the rectangular piece.
      firstj: int. Index of the first column in the rectangular piece.
      lastj: int. Index of the last column plus one in the rectangular piece.
    returns: numnz
      numnz: int. Number of non-zero elements in the rectangular piece of the linear constraint matrix.
    """
    numnz_ = ctypes.c_int32()
    res = __library__.MSK_XX_getapiecenumnz(self.__nativep,firsti_,lasti_,firstj_,lastj_,ctypes.byref(numnz_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    numnz_ = numnz_.value
    _numnz_return_value = numnz_
    return (_numnz_return_value)
  @accepts(_accept_any,_make_int)
  @synchronized
  def getacolnumnz(self,i_):
    """
    Obtains the number of non-zero elements in one column of the linear constraint matrix
  
    getacolnumnz(self,i_)
      i: int. Index of the column.
    returns: nzj
      nzj: int. Number of non-zeros in the j'th column of (A).
    """
    nzj_ = ctypes.c_int32()
    res = __library__.MSK_XX_getacolnumnz(self.__nativep,i_,ctypes.byref(nzj_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    nzj_ = nzj_.value
    _nzj_return_value = nzj_
    return (_nzj_return_value)
  @accepts(_accept_any,_make_int,_accept_intvector,_accept_doublevector)
  @synchronized
  def getacol(self,j_,subj_,valj_):
    """
    Obtains one column of the linear constraint matrix.
  
    getacol(self,j_,subj_,valj_)
      j: int. Index of the column.
      subj: array of int. Row indices of the non-zeros in the column obtained.
      valj: array of double. Numerical values in the column obtained.
    returns: nzj
      nzj: int. Number of non-zeros in the column obtained.
    """
    nzj_ = ctypes.c_int32()
    _subj_minlength = self.getacolnumnz((j_))
    if self.getacolnumnz((j_)) > 0 and subj_ is not None and len(subj_) != self.getacolnumnz((j_)):
      raise ValueError("Array argument subj is not long enough: Is %d, expected %d" % (len(subj_),self.getacolnumnz((j_))))
    if isinstance(subj_,numpy.ndarray) and not subj_.flags.writeable:
      raise ValueError("Argument subj must be writable")
    if subj_ is None:
      raise ValueError("Argument subj may not be None")
    if isinstance(subj_, numpy.ndarray) and subj_.dtype is numpy.dtype(numpy.int32) and subj_.flags.contiguous:
      _subj_copyarray = False
      _subj_tmp = ctypes.cast(subj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subj_ is not None:
      _subj_copyarray = True
      _subj_np_tmp = numpy.zeros(len(subj_),numpy.dtype(numpy.int32))
      _subj_np_tmp[:] = subj_
      assert _subj_np_tmp.flags.contiguous
      _subj_tmp = ctypes.cast(_subj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subj_copyarray = False
      _subj_tmp = None
      
    _valj_minlength = self.getacolnumnz((j_))
    if self.getacolnumnz((j_)) > 0 and valj_ is not None and len(valj_) != self.getacolnumnz((j_)):
      raise ValueError("Array argument valj is not long enough: Is %d, expected %d" % (len(valj_),self.getacolnumnz((j_))))
    if isinstance(valj_,numpy.ndarray) and not valj_.flags.writeable:
      raise ValueError("Argument valj must be writable")
    if valj_ is None:
      raise ValueError("Argument valj may not be None")
    if isinstance(valj_, numpy.ndarray) and valj_.dtype is numpy.dtype(numpy.float64) and valj_.flags.contiguous:
      _valj_copyarray = False
      _valj_tmp = ctypes.cast(valj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif valj_ is not None:
      _valj_copyarray = True
      _valj_np_tmp = numpy.zeros(len(valj_),numpy.dtype(numpy.float64))
      _valj_np_tmp[:] = valj_
      assert _valj_np_tmp.flags.contiguous
      _valj_tmp = ctypes.cast(_valj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _valj_copyarray = False
      _valj_tmp = None
      
    res = __library__.MSK_XX_getacol(self.__nativep,j_,ctypes.byref(nzj_),_subj_tmp,_valj_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    nzj_ = nzj_.value
    _nzj_return_value = nzj_
    if _subj_copyarray:
      subj_[:] = _subj_np_tmp
    if _valj_copyarray:
      valj_[:] = _valj_np_tmp
    return (_nzj_return_value)
  @accepts(_accept_any,_make_int,_make_int,_accept_longvector,_accept_longvector,_accept_intvector,_accept_doublevector)
  @synchronized
  def getacolslice(self,first_,last_,ptrb_,ptre_,sub_,val_):
    """
    Obtains a sequence of columns from the coefficient matrix.
  
    getacolslice(self,first_,last_,ptrb_,ptre_,sub_,val_)
      first: int. Index of the first column in the sequence.
      last: int. Index of the last column in the sequence plus one.
      ptrb: array of long. Column start pointers.
      ptre: array of long. Column end pointers.
      sub: array of int. Contains the row subscripts.
      val: array of double. Contains the coefficient values.
    """
    maxnumnz_ = self.getacolslicenumnz((first_),(last_))
    _ptrb_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and ptrb_ is not None and len(ptrb_) != ((last_) - (first_)):
      raise ValueError("Array argument ptrb is not long enough: Is %d, expected %d" % (len(ptrb_),((last_) - (first_))))
    if isinstance(ptrb_,numpy.ndarray) and not ptrb_.flags.writeable:
      raise ValueError("Argument ptrb must be writable")
    if ptrb_ is None:
      raise ValueError("Argument ptrb may not be None")
    if isinstance(ptrb_, numpy.ndarray) and ptrb_.dtype is numpy.dtype(numpy.int64) and ptrb_.flags.contiguous:
      _ptrb_copyarray = False
      _ptrb_tmp = ctypes.cast(ptrb_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif ptrb_ is not None:
      _ptrb_copyarray = True
      _ptrb_np_tmp = numpy.zeros(len(ptrb_),numpy.dtype(numpy.int64))
      _ptrb_np_tmp[:] = ptrb_
      assert _ptrb_np_tmp.flags.contiguous
      _ptrb_tmp = ctypes.cast(_ptrb_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _ptrb_copyarray = False
      _ptrb_tmp = None
      
    _ptre_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and ptre_ is not None and len(ptre_) != ((last_) - (first_)):
      raise ValueError("Array argument ptre is not long enough: Is %d, expected %d" % (len(ptre_),((last_) - (first_))))
    if isinstance(ptre_,numpy.ndarray) and not ptre_.flags.writeable:
      raise ValueError("Argument ptre must be writable")
    if ptre_ is None:
      raise ValueError("Argument ptre may not be None")
    if isinstance(ptre_, numpy.ndarray) and ptre_.dtype is numpy.dtype(numpy.int64) and ptre_.flags.contiguous:
      _ptre_copyarray = False
      _ptre_tmp = ctypes.cast(ptre_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif ptre_ is not None:
      _ptre_copyarray = True
      _ptre_np_tmp = numpy.zeros(len(ptre_),numpy.dtype(numpy.int64))
      _ptre_np_tmp[:] = ptre_
      assert _ptre_np_tmp.flags.contiguous
      _ptre_tmp = ctypes.cast(_ptre_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _ptre_copyarray = False
      _ptre_tmp = None
      
    _sub_minlength = (maxnumnz_)
    if (maxnumnz_) > 0 and sub_ is not None and len(sub_) != (maxnumnz_):
      raise ValueError("Array argument sub is not long enough: Is %d, expected %d" % (len(sub_),(maxnumnz_)))
    if isinstance(sub_,numpy.ndarray) and not sub_.flags.writeable:
      raise ValueError("Argument sub must be writable")
    if sub_ is None:
      raise ValueError("Argument sub may not be None")
    if isinstance(sub_, numpy.ndarray) and sub_.dtype is numpy.dtype(numpy.int32) and sub_.flags.contiguous:
      _sub_copyarray = False
      _sub_tmp = ctypes.cast(sub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif sub_ is not None:
      _sub_copyarray = True
      _sub_np_tmp = numpy.zeros(len(sub_),numpy.dtype(numpy.int32))
      _sub_np_tmp[:] = sub_
      assert _sub_np_tmp.flags.contiguous
      _sub_tmp = ctypes.cast(_sub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _sub_copyarray = False
      _sub_tmp = None
      
    _val_minlength = (maxnumnz_)
    if (maxnumnz_) > 0 and val_ is not None and len(val_) != (maxnumnz_):
      raise ValueError("Array argument val is not long enough: Is %d, expected %d" % (len(val_),(maxnumnz_)))
    if isinstance(val_,numpy.ndarray) and not val_.flags.writeable:
      raise ValueError("Argument val must be writable")
    if val_ is None:
      raise ValueError("Argument val may not be None")
    if isinstance(val_, numpy.ndarray) and val_.dtype is numpy.dtype(numpy.float64) and val_.flags.contiguous:
      _val_copyarray = False
      _val_tmp = ctypes.cast(val_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif val_ is not None:
      _val_copyarray = True
      _val_np_tmp = numpy.zeros(len(val_),numpy.dtype(numpy.float64))
      _val_np_tmp[:] = val_
      assert _val_np_tmp.flags.contiguous
      _val_tmp = ctypes.cast(_val_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _val_copyarray = False
      _val_tmp = None
      
    surp_ = ctypes.c_int64(_sub_minlength)
    res = __library__.MSK_XX_getacolslice64(self.__nativep,first_,last_,maxnumnz_,ctypes.byref(surp_),_ptrb_tmp,_ptre_tmp,_sub_tmp,_val_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _ptrb_copyarray:
      ptrb_[:] = _ptrb_np_tmp
    if _ptre_copyarray:
      ptre_[:] = _ptre_np_tmp
    if _sub_copyarray:
      sub_[:] = _sub_np_tmp
    if _val_copyarray:
      val_[:] = _val_np_tmp
  @accepts(_accept_any,_make_int)
  @synchronized
  def getarownumnz(self,i_):
    """
    Obtains the number of non-zero elements in one row of the linear constraint matrix
  
    getarownumnz(self,i_)
      i: int. Index of the row.
    returns: nzi
      nzi: int. Number of non-zeros in the i'th row of `A`.
    """
    nzi_ = ctypes.c_int32()
    res = __library__.MSK_XX_getarownumnz(self.__nativep,i_,ctypes.byref(nzi_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    nzi_ = nzi_.value
    _nzi_return_value = nzi_
    return (_nzi_return_value)
  @accepts(_accept_any,_make_int,_accept_intvector,_accept_doublevector)
  @synchronized
  def getarow(self,i_,subi_,vali_):
    """
    Obtains one row of the linear constraint matrix.
  
    getarow(self,i_,subi_,vali_)
      i: int. Index of the row.
      subi: array of int. Column indices of the non-zeros in the row obtained.
      vali: array of double. Numerical values of the row obtained.
    returns: nzi
      nzi: int. Number of non-zeros in the row obtained.
    """
    nzi_ = ctypes.c_int32()
    _subi_minlength = self.getarownumnz((i_))
    if self.getarownumnz((i_)) > 0 and subi_ is not None and len(subi_) != self.getarownumnz((i_)):
      raise ValueError("Array argument subi is not long enough: Is %d, expected %d" % (len(subi_),self.getarownumnz((i_))))
    if isinstance(subi_,numpy.ndarray) and not subi_.flags.writeable:
      raise ValueError("Argument subi must be writable")
    if subi_ is None:
      raise ValueError("Argument subi may not be None")
    if isinstance(subi_, numpy.ndarray) and subi_.dtype is numpy.dtype(numpy.int32) and subi_.flags.contiguous:
      _subi_copyarray = False
      _subi_tmp = ctypes.cast(subi_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subi_ is not None:
      _subi_copyarray = True
      _subi_np_tmp = numpy.zeros(len(subi_),numpy.dtype(numpy.int32))
      _subi_np_tmp[:] = subi_
      assert _subi_np_tmp.flags.contiguous
      _subi_tmp = ctypes.cast(_subi_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subi_copyarray = False
      _subi_tmp = None
      
    _vali_minlength = self.getarownumnz((i_))
    if self.getarownumnz((i_)) > 0 and vali_ is not None and len(vali_) != self.getarownumnz((i_)):
      raise ValueError("Array argument vali is not long enough: Is %d, expected %d" % (len(vali_),self.getarownumnz((i_))))
    if isinstance(vali_,numpy.ndarray) and not vali_.flags.writeable:
      raise ValueError("Argument vali must be writable")
    if vali_ is None:
      raise ValueError("Argument vali may not be None")
    if isinstance(vali_, numpy.ndarray) and vali_.dtype is numpy.dtype(numpy.float64) and vali_.flags.contiguous:
      _vali_copyarray = False
      _vali_tmp = ctypes.cast(vali_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif vali_ is not None:
      _vali_copyarray = True
      _vali_np_tmp = numpy.zeros(len(vali_),numpy.dtype(numpy.float64))
      _vali_np_tmp[:] = vali_
      assert _vali_np_tmp.flags.contiguous
      _vali_tmp = ctypes.cast(_vali_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _vali_copyarray = False
      _vali_tmp = None
      
    res = __library__.MSK_XX_getarow(self.__nativep,i_,ctypes.byref(nzi_),_subi_tmp,_vali_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    nzi_ = nzi_.value
    _nzi_return_value = nzi_
    if _subi_copyarray:
      subi_[:] = _subi_np_tmp
    if _vali_copyarray:
      vali_[:] = _vali_np_tmp
    return (_nzi_return_value)
  @accepts(_accept_any,_make_int,_make_int)
  @synchronized
  def getacolslicenumnz(self,first_,last_):
    """
    Obtains the number of non-zeros in a slice of columns of the coefficient matrix.
  
    getacolslicenumnz(self,first_,last_)
      first: int. Index of the first column in the sequence.
      last: int. Index of the last column plus one in the sequence.
    returns: numnz
      numnz: long. Number of non-zeros in the slice.
    """
    numnz_ = ctypes.c_int64()
    res = __library__.MSK_XX_getacolslicenumnz64(self.__nativep,first_,last_,ctypes.byref(numnz_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    numnz_ = numnz_.value
    _numnz_return_value = numnz_
    return (_numnz_return_value)
  @accepts(_accept_any,_make_int,_make_int)
  @synchronized
  def getarowslicenumnz(self,first_,last_):
    """
    Obtains the number of non-zeros in a slice of rows of the coefficient matrix.
  
    getarowslicenumnz(self,first_,last_)
      first: int. Index of the first row in the sequence.
      last: int. Index of the last row plus one in the sequence.
    returns: numnz
      numnz: long. Number of non-zeros in the slice.
    """
    numnz_ = ctypes.c_int64()
    res = __library__.MSK_XX_getarowslicenumnz64(self.__nativep,first_,last_,ctypes.byref(numnz_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    numnz_ = numnz_.value
    _numnz_return_value = numnz_
    return (_numnz_return_value)
  @accepts(_accept_any,_make_int,_make_int,_accept_longvector,_accept_longvector,_accept_intvector,_accept_doublevector)
  @synchronized
  def getarowslice(self,first_,last_,ptrb_,ptre_,sub_,val_):
    """
    Obtains a sequence of rows from the coefficient matrix.
  
    getarowslice(self,first_,last_,ptrb_,ptre_,sub_,val_)
      first: int. Index of the first row in the sequence.
      last: int. Index of the last row in the sequence plus one.
      ptrb: array of long. Row start pointers.
      ptre: array of long. Row end pointers.
      sub: array of int. Contains the column subscripts.
      val: array of double. Contains the coefficient values.
    """
    maxnumnz_ = self.getarowslicenumnz((first_),(last_))
    _ptrb_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and ptrb_ is not None and len(ptrb_) != ((last_) - (first_)):
      raise ValueError("Array argument ptrb is not long enough: Is %d, expected %d" % (len(ptrb_),((last_) - (first_))))
    if isinstance(ptrb_,numpy.ndarray) and not ptrb_.flags.writeable:
      raise ValueError("Argument ptrb must be writable")
    if ptrb_ is None:
      raise ValueError("Argument ptrb may not be None")
    if isinstance(ptrb_, numpy.ndarray) and ptrb_.dtype is numpy.dtype(numpy.int64) and ptrb_.flags.contiguous:
      _ptrb_copyarray = False
      _ptrb_tmp = ctypes.cast(ptrb_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif ptrb_ is not None:
      _ptrb_copyarray = True
      _ptrb_np_tmp = numpy.zeros(len(ptrb_),numpy.dtype(numpy.int64))
      _ptrb_np_tmp[:] = ptrb_
      assert _ptrb_np_tmp.flags.contiguous
      _ptrb_tmp = ctypes.cast(_ptrb_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _ptrb_copyarray = False
      _ptrb_tmp = None
      
    _ptre_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and ptre_ is not None and len(ptre_) != ((last_) - (first_)):
      raise ValueError("Array argument ptre is not long enough: Is %d, expected %d" % (len(ptre_),((last_) - (first_))))
    if isinstance(ptre_,numpy.ndarray) and not ptre_.flags.writeable:
      raise ValueError("Argument ptre must be writable")
    if ptre_ is None:
      raise ValueError("Argument ptre may not be None")
    if isinstance(ptre_, numpy.ndarray) and ptre_.dtype is numpy.dtype(numpy.int64) and ptre_.flags.contiguous:
      _ptre_copyarray = False
      _ptre_tmp = ctypes.cast(ptre_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif ptre_ is not None:
      _ptre_copyarray = True
      _ptre_np_tmp = numpy.zeros(len(ptre_),numpy.dtype(numpy.int64))
      _ptre_np_tmp[:] = ptre_
      assert _ptre_np_tmp.flags.contiguous
      _ptre_tmp = ctypes.cast(_ptre_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _ptre_copyarray = False
      _ptre_tmp = None
      
    _sub_minlength = (maxnumnz_)
    if (maxnumnz_) > 0 and sub_ is not None and len(sub_) != (maxnumnz_):
      raise ValueError("Array argument sub is not long enough: Is %d, expected %d" % (len(sub_),(maxnumnz_)))
    if isinstance(sub_,numpy.ndarray) and not sub_.flags.writeable:
      raise ValueError("Argument sub must be writable")
    if sub_ is None:
      raise ValueError("Argument sub may not be None")
    if isinstance(sub_, numpy.ndarray) and sub_.dtype is numpy.dtype(numpy.int32) and sub_.flags.contiguous:
      _sub_copyarray = False
      _sub_tmp = ctypes.cast(sub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif sub_ is not None:
      _sub_copyarray = True
      _sub_np_tmp = numpy.zeros(len(sub_),numpy.dtype(numpy.int32))
      _sub_np_tmp[:] = sub_
      assert _sub_np_tmp.flags.contiguous
      _sub_tmp = ctypes.cast(_sub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _sub_copyarray = False
      _sub_tmp = None
      
    _val_minlength = (maxnumnz_)
    if (maxnumnz_) > 0 and val_ is not None and len(val_) != (maxnumnz_):
      raise ValueError("Array argument val is not long enough: Is %d, expected %d" % (len(val_),(maxnumnz_)))
    if isinstance(val_,numpy.ndarray) and not val_.flags.writeable:
      raise ValueError("Argument val must be writable")
    if val_ is None:
      raise ValueError("Argument val may not be None")
    if isinstance(val_, numpy.ndarray) and val_.dtype is numpy.dtype(numpy.float64) and val_.flags.contiguous:
      _val_copyarray = False
      _val_tmp = ctypes.cast(val_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif val_ is not None:
      _val_copyarray = True
      _val_np_tmp = numpy.zeros(len(val_),numpy.dtype(numpy.float64))
      _val_np_tmp[:] = val_
      assert _val_np_tmp.flags.contiguous
      _val_tmp = ctypes.cast(_val_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _val_copyarray = False
      _val_tmp = None
      
    surp_ = ctypes.c_int64(_sub_minlength)
    res = __library__.MSK_XX_getarowslice64(self.__nativep,first_,last_,maxnumnz_,ctypes.byref(surp_),_ptrb_tmp,_ptre_tmp,_sub_tmp,_val_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _ptrb_copyarray:
      ptrb_[:] = _ptrb_np_tmp
    if _ptre_copyarray:
      ptre_[:] = _ptre_np_tmp
    if _sub_copyarray:
      sub_[:] = _sub_np_tmp
    if _val_copyarray:
      val_[:] = _val_np_tmp
  @accepts(_accept_any,_make_int,_make_int,_accept_intvector,_accept_intvector,_accept_doublevector)
  @synchronized
  def getarowslicetrip(self,first_,last_,subi_,subj_,val_):
    """
    Obtains a sequence of rows from the coefficient matrix in sparse triplet format.
  
    getarowslicetrip(self,first_,last_,subi_,subj_,val_)
      first: int. Index of the first row in the sequence.
      last: int. Index of the last row in the sequence plus one.
      subi: array of int. Constraint subscripts.
      subj: array of int. Column subscripts.
      val: array of double. Values.
    """
    maxnumnz_ = self.getarowslicenumnz((first_),(last_))
    _subi_minlength = (maxnumnz_)
    if (maxnumnz_) > 0 and subi_ is not None and len(subi_) != (maxnumnz_):
      raise ValueError("Array argument subi is not long enough: Is %d, expected %d" % (len(subi_),(maxnumnz_)))
    if isinstance(subi_,numpy.ndarray) and not subi_.flags.writeable:
      raise ValueError("Argument subi must be writable")
    if isinstance(subi_, numpy.ndarray) and subi_.dtype is numpy.dtype(numpy.int32) and subi_.flags.contiguous:
      _subi_copyarray = False
      _subi_tmp = ctypes.cast(subi_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subi_ is not None:
      _subi_copyarray = True
      _subi_np_tmp = numpy.zeros(len(subi_),numpy.dtype(numpy.int32))
      _subi_np_tmp[:] = subi_
      assert _subi_np_tmp.flags.contiguous
      _subi_tmp = ctypes.cast(_subi_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subi_copyarray = False
      _subi_tmp = None
      
    _subj_minlength = (maxnumnz_)
    if (maxnumnz_) > 0 and subj_ is not None and len(subj_) != (maxnumnz_):
      raise ValueError("Array argument subj is not long enough: Is %d, expected %d" % (len(subj_),(maxnumnz_)))
    if isinstance(subj_,numpy.ndarray) and not subj_.flags.writeable:
      raise ValueError("Argument subj must be writable")
    if isinstance(subj_, numpy.ndarray) and subj_.dtype is numpy.dtype(numpy.int32) and subj_.flags.contiguous:
      _subj_copyarray = False
      _subj_tmp = ctypes.cast(subj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subj_ is not None:
      _subj_copyarray = True
      _subj_np_tmp = numpy.zeros(len(subj_),numpy.dtype(numpy.int32))
      _subj_np_tmp[:] = subj_
      assert _subj_np_tmp.flags.contiguous
      _subj_tmp = ctypes.cast(_subj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subj_copyarray = False
      _subj_tmp = None
      
    _val_minlength = (maxnumnz_)
    if (maxnumnz_) > 0 and val_ is not None and len(val_) != (maxnumnz_):
      raise ValueError("Array argument val is not long enough: Is %d, expected %d" % (len(val_),(maxnumnz_)))
    if isinstance(val_,numpy.ndarray) and not val_.flags.writeable:
      raise ValueError("Argument val must be writable")
    if isinstance(val_, numpy.ndarray) and val_.dtype is numpy.dtype(numpy.float64) and val_.flags.contiguous:
      _val_copyarray = False
      _val_tmp = ctypes.cast(val_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif val_ is not None:
      _val_copyarray = True
      _val_np_tmp = numpy.zeros(len(val_),numpy.dtype(numpy.float64))
      _val_np_tmp[:] = val_
      assert _val_np_tmp.flags.contiguous
      _val_tmp = ctypes.cast(_val_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _val_copyarray = False
      _val_tmp = None
      
    surp_ = ctypes.c_int64(_subi_minlength)
    res = __library__.MSK_XX_getarowslicetrip(self.__nativep,first_,last_,maxnumnz_,ctypes.byref(surp_),_subi_tmp,_subj_tmp,_val_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _subi_copyarray:
      subi_[:] = _subi_np_tmp
    if _subj_copyarray:
      subj_[:] = _subj_np_tmp
    if _val_copyarray:
      val_[:] = _val_np_tmp
  @accepts(_accept_any,_make_int,_make_int,_accept_intvector,_accept_intvector,_accept_doublevector)
  @synchronized
  def getacolslicetrip(self,first_,last_,subi_,subj_,val_):
    """
    Obtains a sequence of columns from the coefficient matrix in triplet format.
  
    getacolslicetrip(self,first_,last_,subi_,subj_,val_)
      first: int. Index of the first column in the sequence.
      last: int. Index of the last column in the sequence plus one.
      subi: array of int. Constraint subscripts.
      subj: array of int. Column subscripts.
      val: array of double. Values.
    """
    maxnumnz_ = self.getacolslicenumnz((first_),(last_))
    _subi_minlength = (maxnumnz_)
    if (maxnumnz_) > 0 and subi_ is not None and len(subi_) != (maxnumnz_):
      raise ValueError("Array argument subi is not long enough: Is %d, expected %d" % (len(subi_),(maxnumnz_)))
    if isinstance(subi_,numpy.ndarray) and not subi_.flags.writeable:
      raise ValueError("Argument subi must be writable")
    if isinstance(subi_, numpy.ndarray) and subi_.dtype is numpy.dtype(numpy.int32) and subi_.flags.contiguous:
      _subi_copyarray = False
      _subi_tmp = ctypes.cast(subi_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subi_ is not None:
      _subi_copyarray = True
      _subi_np_tmp = numpy.zeros(len(subi_),numpy.dtype(numpy.int32))
      _subi_np_tmp[:] = subi_
      assert _subi_np_tmp.flags.contiguous
      _subi_tmp = ctypes.cast(_subi_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subi_copyarray = False
      _subi_tmp = None
      
    _subj_minlength = (maxnumnz_)
    if (maxnumnz_) > 0 and subj_ is not None and len(subj_) != (maxnumnz_):
      raise ValueError("Array argument subj is not long enough: Is %d, expected %d" % (len(subj_),(maxnumnz_)))
    if isinstance(subj_,numpy.ndarray) and not subj_.flags.writeable:
      raise ValueError("Argument subj must be writable")
    if isinstance(subj_, numpy.ndarray) and subj_.dtype is numpy.dtype(numpy.int32) and subj_.flags.contiguous:
      _subj_copyarray = False
      _subj_tmp = ctypes.cast(subj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subj_ is not None:
      _subj_copyarray = True
      _subj_np_tmp = numpy.zeros(len(subj_),numpy.dtype(numpy.int32))
      _subj_np_tmp[:] = subj_
      assert _subj_np_tmp.flags.contiguous
      _subj_tmp = ctypes.cast(_subj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subj_copyarray = False
      _subj_tmp = None
      
    _val_minlength = (maxnumnz_)
    if (maxnumnz_) > 0 and val_ is not None and len(val_) != (maxnumnz_):
      raise ValueError("Array argument val is not long enough: Is %d, expected %d" % (len(val_),(maxnumnz_)))
    if isinstance(val_,numpy.ndarray) and not val_.flags.writeable:
      raise ValueError("Argument val must be writable")
    if isinstance(val_, numpy.ndarray) and val_.dtype is numpy.dtype(numpy.float64) and val_.flags.contiguous:
      _val_copyarray = False
      _val_tmp = ctypes.cast(val_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif val_ is not None:
      _val_copyarray = True
      _val_np_tmp = numpy.zeros(len(val_),numpy.dtype(numpy.float64))
      _val_np_tmp[:] = val_
      assert _val_np_tmp.flags.contiguous
      _val_tmp = ctypes.cast(_val_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _val_copyarray = False
      _val_tmp = None
      
    surp_ = ctypes.c_int64(_subi_minlength)
    res = __library__.MSK_XX_getacolslicetrip(self.__nativep,first_,last_,maxnumnz_,ctypes.byref(surp_),_subi_tmp,_subj_tmp,_val_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _subi_copyarray:
      subi_[:] = _subi_np_tmp
    if _subj_copyarray:
      subj_[:] = _subj_np_tmp
    if _val_copyarray:
      val_[:] = _val_np_tmp
  @accepts(_accept_any,_make_int)
  @synchronized
  def getconbound(self,i_):
    """
    Obtains bound information for one constraint.
  
    getconbound(self,i_)
      i: int. Index of the constraint for which the bound information should be obtained.
    returns: bk,bl,bu
      bk: mosek.boundkey. Bound keys.
      bl: double. Values for lower bounds.
      bu: double. Values for upper bounds.
    """
    bk_ = ctypes.c_int32()
    bl_ = ctypes.c_double()
    bu_ = ctypes.c_double()
    res = __library__.MSK_XX_getconbound(self.__nativep,i_,ctypes.byref(bk_),ctypes.byref(bl_),ctypes.byref(bu_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _bk_return_value = boundkey(bk_.value)
    bl_ = bl_.value
    _bl_return_value = bl_
    bu_ = bu_.value
    _bu_return_value = bu_
    return (_bk_return_value,_bl_return_value,_bu_return_value)
  @accepts(_accept_any,_make_int)
  @synchronized
  def getvarbound(self,i_):
    """
    Obtains bound information for one variable.
  
    getvarbound(self,i_)
      i: int. Index of the variable for which the bound information should be obtained.
    returns: bk,bl,bu
      bk: mosek.boundkey. Bound keys.
      bl: double. Values for lower bounds.
      bu: double. Values for upper bounds.
    """
    bk_ = ctypes.c_int32()
    bl_ = ctypes.c_double()
    bu_ = ctypes.c_double()
    res = __library__.MSK_XX_getvarbound(self.__nativep,i_,ctypes.byref(bk_),ctypes.byref(bl_),ctypes.byref(bu_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _bk_return_value = boundkey(bk_.value)
    bl_ = bl_.value
    _bl_return_value = bl_
    bu_ = bu_.value
    _bu_return_value = bu_
    return (_bk_return_value,_bl_return_value,_bu_return_value)
  @accepts(_accept_any,_make_int,_make_int,_accept_any,_accept_doublevector,_accept_doublevector)
  @synchronized
  def getconboundslice(self,first_,last_,bk_,bl_,bu_):
    """
    Obtains bounds information for a slice of the constraints.
  
    getconboundslice(self,first_,last_,bk_,bl_,bu_)
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      bk: array of mosek.boundkey. Bound keys.
      bl: array of double. Values for lower bounds.
      bu: array of double. Values for upper bounds.
    """
    _bk_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and bk_ is not None and len(bk_) != ((last_) - (first_)):
      raise ValueError("Array argument bk is not long enough: Is %d, expected %d" % (len(bk_),((last_) - (first_))))
    if isinstance(bk_,numpy.ndarray) and not bk_.flags.writeable:
      raise ValueError("Argument bk must be writable")
    if bk_ is not None:
        _bk_tmp = (ctypes.c_int32 * len(bk_))()
    else:
        _bk_tmp = None
    _bl_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and bl_ is not None and len(bl_) != ((last_) - (first_)):
      raise ValueError("Array argument bl is not long enough: Is %d, expected %d" % (len(bl_),((last_) - (first_))))
    if isinstance(bl_,numpy.ndarray) and not bl_.flags.writeable:
      raise ValueError("Argument bl must be writable")
    if isinstance(bl_, numpy.ndarray) and bl_.dtype is numpy.dtype(numpy.float64) and bl_.flags.contiguous:
      _bl_copyarray = False
      _bl_tmp = ctypes.cast(bl_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif bl_ is not None:
      _bl_copyarray = True
      _bl_np_tmp = numpy.zeros(len(bl_),numpy.dtype(numpy.float64))
      _bl_np_tmp[:] = bl_
      assert _bl_np_tmp.flags.contiguous
      _bl_tmp = ctypes.cast(_bl_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _bl_copyarray = False
      _bl_tmp = None
      
    _bu_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and bu_ is not None and len(bu_) != ((last_) - (first_)):
      raise ValueError("Array argument bu is not long enough: Is %d, expected %d" % (len(bu_),((last_) - (first_))))
    if isinstance(bu_,numpy.ndarray) and not bu_.flags.writeable:
      raise ValueError("Argument bu must be writable")
    if isinstance(bu_, numpy.ndarray) and bu_.dtype is numpy.dtype(numpy.float64) and bu_.flags.contiguous:
      _bu_copyarray = False
      _bu_tmp = ctypes.cast(bu_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif bu_ is not None:
      _bu_copyarray = True
      _bu_np_tmp = numpy.zeros(len(bu_),numpy.dtype(numpy.float64))
      _bu_np_tmp[:] = bu_
      assert _bu_np_tmp.flags.contiguous
      _bu_tmp = ctypes.cast(_bu_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _bu_copyarray = False
      _bu_tmp = None
      
    res = __library__.MSK_XX_getconboundslice(self.__nativep,first_,last_,_bk_tmp,_bl_tmp,_bu_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if bk_ is not None: bk_[:] = [ boundkey(v) for v in _bk_tmp[0:len(bk_)] ]
    if _bl_copyarray:
      bl_[:] = _bl_np_tmp
    if _bu_copyarray:
      bu_[:] = _bu_np_tmp
  @accepts(_accept_any,_make_int,_make_int,_accept_any,_accept_doublevector,_accept_doublevector)
  @synchronized
  def getvarboundslice(self,first_,last_,bk_,bl_,bu_):
    """
    Obtains bounds information for a slice of the variables.
  
    getvarboundslice(self,first_,last_,bk_,bl_,bu_)
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      bk: array of mosek.boundkey. Bound keys.
      bl: array of double. Values for lower bounds.
      bu: array of double. Values for upper bounds.
    """
    _bk_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and bk_ is not None and len(bk_) != ((last_) - (first_)):
      raise ValueError("Array argument bk is not long enough: Is %d, expected %d" % (len(bk_),((last_) - (first_))))
    if isinstance(bk_,numpy.ndarray) and not bk_.flags.writeable:
      raise ValueError("Argument bk must be writable")
    if bk_ is not None:
        _bk_tmp = (ctypes.c_int32 * len(bk_))()
    else:
        _bk_tmp = None
    _bl_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and bl_ is not None and len(bl_) != ((last_) - (first_)):
      raise ValueError("Array argument bl is not long enough: Is %d, expected %d" % (len(bl_),((last_) - (first_))))
    if isinstance(bl_,numpy.ndarray) and not bl_.flags.writeable:
      raise ValueError("Argument bl must be writable")
    if isinstance(bl_, numpy.ndarray) and bl_.dtype is numpy.dtype(numpy.float64) and bl_.flags.contiguous:
      _bl_copyarray = False
      _bl_tmp = ctypes.cast(bl_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif bl_ is not None:
      _bl_copyarray = True
      _bl_np_tmp = numpy.zeros(len(bl_),numpy.dtype(numpy.float64))
      _bl_np_tmp[:] = bl_
      assert _bl_np_tmp.flags.contiguous
      _bl_tmp = ctypes.cast(_bl_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _bl_copyarray = False
      _bl_tmp = None
      
    _bu_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and bu_ is not None and len(bu_) != ((last_) - (first_)):
      raise ValueError("Array argument bu is not long enough: Is %d, expected %d" % (len(bu_),((last_) - (first_))))
    if isinstance(bu_,numpy.ndarray) and not bu_.flags.writeable:
      raise ValueError("Argument bu must be writable")
    if isinstance(bu_, numpy.ndarray) and bu_.dtype is numpy.dtype(numpy.float64) and bu_.flags.contiguous:
      _bu_copyarray = False
      _bu_tmp = ctypes.cast(bu_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif bu_ is not None:
      _bu_copyarray = True
      _bu_np_tmp = numpy.zeros(len(bu_),numpy.dtype(numpy.float64))
      _bu_np_tmp[:] = bu_
      assert _bu_np_tmp.flags.contiguous
      _bu_tmp = ctypes.cast(_bu_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _bu_copyarray = False
      _bu_tmp = None
      
    res = __library__.MSK_XX_getvarboundslice(self.__nativep,first_,last_,_bk_tmp,_bl_tmp,_bu_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if bk_ is not None: bk_[:] = [ boundkey(v) for v in _bk_tmp[0:len(bk_)] ]
    if _bl_copyarray:
      bl_[:] = _bl_np_tmp
    if _bu_copyarray:
      bu_[:] = _bu_np_tmp
  @accepts(_accept_any,_make_int)
  @synchronized
  def getcj(self,j_):
    """
    Obtains one objective coefficient.
  
    getcj(self,j_)
      j: int. Index of the variable for which the c coefficient should be obtained.
    returns: cj
      cj: double. The c coefficient value.
    """
    cj_ = ctypes.c_double()
    res = __library__.MSK_XX_getcj(self.__nativep,j_,ctypes.byref(cj_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    cj_ = cj_.value
    _cj_return_value = cj_
    return (_cj_return_value)
  @accepts(_accept_any,_accept_doublevector)
  @synchronized
  def getc(self,c_):
    """
    Obtains all objective coefficients.
  
    getc(self,c_)
      c: array of double. Linear terms of the objective as a dense vector. The length is the number of variables.
    """
    _c_minlength = self.getnumvar()
    if self.getnumvar() > 0 and c_ is not None and len(c_) != self.getnumvar():
      raise ValueError("Array argument c is not long enough: Is %d, expected %d" % (len(c_),self.getnumvar()))
    if isinstance(c_,numpy.ndarray) and not c_.flags.writeable:
      raise ValueError("Argument c must be writable")
    if isinstance(c_, numpy.ndarray) and c_.dtype is numpy.dtype(numpy.float64) and c_.flags.contiguous:
      _c_copyarray = False
      _c_tmp = ctypes.cast(c_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif c_ is not None:
      _c_copyarray = True
      _c_np_tmp = numpy.zeros(len(c_),numpy.dtype(numpy.float64))
      _c_np_tmp[:] = c_
      assert _c_np_tmp.flags.contiguous
      _c_tmp = ctypes.cast(_c_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _c_copyarray = False
      _c_tmp = None
      
    res = __library__.MSK_XX_getc(self.__nativep,_c_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _c_copyarray:
      c_[:] = _c_np_tmp
  @accepts(_accept_any)
  @synchronized
  def getcfix(self):
    """
    Obtains the fixed term in the objective.
  
    getcfix(self)
    returns: cfix
      cfix: double. Fixed term in the objective.
    """
    cfix_ = ctypes.c_double()
    res = __library__.MSK_XX_getcfix(self.__nativep,ctypes.byref(cfix_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    cfix_ = cfix_.value
    _cfix_return_value = cfix_
    return (_cfix_return_value)
  @accepts(_accept_any,_make_int,_accept_intvector)
  @synchronized
  def getcone(self,k_,submem_):
    """
    Obtains a cone.
  
    getcone(self,k_,submem_)
      k: int. Index of the cone.
      submem: array of int. Variable subscripts of the members in the cone.
    returns: ct,conepar,nummem
      ct: mosek.conetype. Specifies the type of the cone.
      conepar: double. For the power cone it denotes the exponent alpha. For other cone types it is unused and can be set to 0.
      nummem: int. Number of member variables in the cone.
    """
    ct_ = ctypes.c_int32()
    conepar_ = ctypes.c_double()
    nummem_ = ctypes.c_int32()
    _submem_minlength = self.getconeinfo((k_))[2]
    if self.getconeinfo((k_))[2] > 0 and submem_ is not None and len(submem_) != self.getconeinfo((k_))[2]:
      raise ValueError("Array argument submem is not long enough: Is %d, expected %d" % (len(submem_),self.getconeinfo((k_))[2]))
    if isinstance(submem_,numpy.ndarray) and not submem_.flags.writeable:
      raise ValueError("Argument submem must be writable")
    if isinstance(submem_, numpy.ndarray) and submem_.dtype is numpy.dtype(numpy.int32) and submem_.flags.contiguous:
      _submem_copyarray = False
      _submem_tmp = ctypes.cast(submem_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif submem_ is not None:
      _submem_copyarray = True
      _submem_np_tmp = numpy.zeros(len(submem_),numpy.dtype(numpy.int32))
      _submem_np_tmp[:] = submem_
      assert _submem_np_tmp.flags.contiguous
      _submem_tmp = ctypes.cast(_submem_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _submem_copyarray = False
      _submem_tmp = None
      
    res = __library__.MSK_XX_getcone(self.__nativep,k_,ctypes.byref(ct_),ctypes.byref(conepar_),ctypes.byref(nummem_),_submem_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _ct_return_value = conetype(ct_.value)
    conepar_ = conepar_.value
    _conepar_return_value = conepar_
    nummem_ = nummem_.value
    _nummem_return_value = nummem_
    if _submem_copyarray:
      submem_[:] = _submem_np_tmp
    return (_ct_return_value,_conepar_return_value,_nummem_return_value)
  @accepts(_accept_any,_make_int)
  @synchronized
  def getconeinfo(self,k_):
    """
    Obtains information about a cone.
  
    getconeinfo(self,k_)
      k: int. Index of the cone.
    returns: ct,conepar,nummem
      ct: mosek.conetype. Specifies the type of the cone.
      conepar: double. For the power cone it denotes the exponent alpha. For other cone types it is unused and can be set to 0.
      nummem: int. Number of member variables in the cone.
    """
    ct_ = ctypes.c_int32()
    conepar_ = ctypes.c_double()
    nummem_ = ctypes.c_int32()
    res = __library__.MSK_XX_getconeinfo(self.__nativep,k_,ctypes.byref(ct_),ctypes.byref(conepar_),ctypes.byref(nummem_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _ct_return_value = conetype(ct_.value)
    conepar_ = conepar_.value
    _conepar_return_value = conepar_
    nummem_ = nummem_.value
    _nummem_return_value = nummem_
    return (_ct_return_value,_conepar_return_value,_nummem_return_value)
  @accepts(_accept_any,_make_intvector,_accept_doublevector)
  @synchronized
  def getclist(self,subj_,c_):
    """
    Obtains a sequence of coefficients from the objective.
  
    getclist(self,subj_,c_)
      subj: array of int. A list of variable indexes.
      c: array of double. Linear terms of the requested list of the objective as a dense vector.
    """
    num_ = None
    if num_ is None:
      num_ = len(subj_)
    elif num_ != len(subj_):
      raise IndexError("Inconsistent length of array subj")
    if subj_ is None:
      raise ValueError("Argument subj cannot be None")
    if subj_ is None:
      raise ValueError("Argument subj may not be None")
    if isinstance(subj_, numpy.ndarray) and subj_.dtype is numpy.dtype(numpy.int32) and subj_.flags.contiguous:
      _subj_copyarray = False
      _subj_tmp = ctypes.cast(subj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subj_ is not None:
      _subj_copyarray = True
      _subj_np_tmp = numpy.zeros(len(subj_),numpy.dtype(numpy.int32))
      _subj_np_tmp[:] = subj_
      assert _subj_np_tmp.flags.contiguous
      _subj_tmp = ctypes.cast(_subj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subj_copyarray = False
      _subj_tmp = None
      
    _c_minlength = (num_)
    if (num_) > 0 and c_ is not None and len(c_) != (num_):
      raise ValueError("Array argument c is not long enough: Is %d, expected %d" % (len(c_),(num_)))
    if isinstance(c_,numpy.ndarray) and not c_.flags.writeable:
      raise ValueError("Argument c must be writable")
    if c_ is None:
      raise ValueError("Argument c may not be None")
    if isinstance(c_, numpy.ndarray) and c_.dtype is numpy.dtype(numpy.float64) and c_.flags.contiguous:
      _c_copyarray = False
      _c_tmp = ctypes.cast(c_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif c_ is not None:
      _c_copyarray = True
      _c_np_tmp = numpy.zeros(len(c_),numpy.dtype(numpy.float64))
      _c_np_tmp[:] = c_
      assert _c_np_tmp.flags.contiguous
      _c_tmp = ctypes.cast(_c_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _c_copyarray = False
      _c_tmp = None
      
    res = __library__.MSK_XX_getclist(self.__nativep,num_,_subj_tmp,_c_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _c_copyarray:
      c_[:] = _c_np_tmp
  @accepts(_accept_any,_make_int,_make_int,_accept_doublevector)
  @synchronized
  def getcslice(self,first_,last_,c_):
    """
    Obtains a sequence of coefficients from the objective.
  
    getcslice(self,first_,last_,c_)
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      c: array of double. Linear terms of the requested slice of the objective as a dense vector.
    """
    _c_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and c_ is not None and len(c_) != ((last_) - (first_)):
      raise ValueError("Array argument c is not long enough: Is %d, expected %d" % (len(c_),((last_) - (first_))))
    if isinstance(c_,numpy.ndarray) and not c_.flags.writeable:
      raise ValueError("Argument c must be writable")
    if isinstance(c_, numpy.ndarray) and c_.dtype is numpy.dtype(numpy.float64) and c_.flags.contiguous:
      _c_copyarray = False
      _c_tmp = ctypes.cast(c_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif c_ is not None:
      _c_copyarray = True
      _c_np_tmp = numpy.zeros(len(c_),numpy.dtype(numpy.float64))
      _c_np_tmp[:] = c_
      assert _c_np_tmp.flags.contiguous
      _c_tmp = ctypes.cast(_c_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _c_copyarray = False
      _c_tmp = None
      
    res = __library__.MSK_XX_getcslice(self.__nativep,first_,last_,_c_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _c_copyarray:
      c_[:] = _c_np_tmp
  @accepts(_accept_any,_accept_anyenum(dinfitem))
  @synchronized
  def getdouinf(self,whichdinf_):
    """
    Obtains a double information item.
  
    getdouinf(self,whichdinf_)
      whichdinf: mosek.dinfitem. Specifies a double information item.
    returns: dvalue
      dvalue: double. The value of the required double information item.
    """
    dvalue_ = ctypes.c_double()
    res = __library__.MSK_XX_getdouinf(self.__nativep,whichdinf_,ctypes.byref(dvalue_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    dvalue_ = dvalue_.value
    _dvalue_return_value = dvalue_
    return (_dvalue_return_value)
  @accepts(_accept_any,_accept_anyenum(dparam))
  @synchronized
  def getdouparam(self,param_):
    """
    Obtains a double parameter.
  
    getdouparam(self,param_)
      param: mosek.dparam. Which parameter.
    returns: parvalue
      parvalue: double. Parameter value.
    """
    parvalue_ = ctypes.c_double()
    res = __library__.MSK_XX_getdouparam(self.__nativep,param_,ctypes.byref(parvalue_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    parvalue_ = parvalue_.value
    _parvalue_return_value = parvalue_
    return (_parvalue_return_value)
  @accepts(_accept_any,_accept_anyenum(soltype))
  @synchronized
  def getdualobj(self,whichsol_):
    """
    Computes the dual objective value associated with the solution.
  
    getdualobj(self,whichsol_)
      whichsol: mosek.soltype. Selects a solution.
    returns: dualobj
      dualobj: double. Objective value corresponding to the dual solution.
    """
    dualobj_ = ctypes.c_double()
    res = __library__.MSK_XX_getdualobj(self.__nativep,whichsol_,ctypes.byref(dualobj_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    dualobj_ = dualobj_.value
    _dualobj_return_value = dualobj_
    return (_dualobj_return_value)
  @accepts(_accept_any,_accept_anyenum(iinfitem))
  @synchronized
  def getintinf(self,whichiinf_):
    """
    Obtains an integer information item.
  
    getintinf(self,whichiinf_)
      whichiinf: mosek.iinfitem. Specifies an integer information item.
    returns: ivalue
      ivalue: int. The value of the required integer information item.
    """
    ivalue_ = ctypes.c_int32()
    res = __library__.MSK_XX_getintinf(self.__nativep,whichiinf_,ctypes.byref(ivalue_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    ivalue_ = ivalue_.value
    _ivalue_return_value = ivalue_
    return (_ivalue_return_value)
  @accepts(_accept_any,_accept_anyenum(liinfitem))
  @synchronized
  def getlintinf(self,whichliinf_):
    """
    Obtains a long integer information item.
  
    getlintinf(self,whichliinf_)
      whichliinf: mosek.liinfitem. Specifies a long information item.
    returns: ivalue
      ivalue: long. The value of the required long integer information item.
    """
    ivalue_ = ctypes.c_int64()
    res = __library__.MSK_XX_getlintinf(self.__nativep,whichliinf_,ctypes.byref(ivalue_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    ivalue_ = ivalue_.value
    _ivalue_return_value = ivalue_
    return (_ivalue_return_value)
  @accepts(_accept_any,_accept_anyenum(iparam))
  @synchronized
  def getintparam(self,param_):
    """
    Obtains an integer parameter.
  
    getintparam(self,param_)
      param: mosek.iparam. Which parameter.
    returns: parvalue
      parvalue: int. Parameter value.
    """
    parvalue_ = ctypes.c_int32()
    res = __library__.MSK_XX_getintparam(self.__nativep,param_,ctypes.byref(parvalue_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    parvalue_ = parvalue_.value
    _parvalue_return_value = parvalue_
    return (_parvalue_return_value)
  @accepts(_accept_any)
  @synchronized
  def getmaxnumanz(self):
    """
    Obtains number of preallocated non-zeros in the linear constraint matrix.
  
    getmaxnumanz(self)
    returns: maxnumanz
      maxnumanz: long. Number of preallocated non-zero linear matrix elements.
    """
    maxnumanz_ = ctypes.c_int64()
    res = __library__.MSK_XX_getmaxnumanz64(self.__nativep,ctypes.byref(maxnumanz_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    maxnumanz_ = maxnumanz_.value
    _maxnumanz_return_value = maxnumanz_
    return (_maxnumanz_return_value)
  @accepts(_accept_any)
  @synchronized
  def getmaxnumcon(self):
    """
    Obtains the number of preallocated constraints in the optimization task.
  
    getmaxnumcon(self)
    returns: maxnumcon
      maxnumcon: int. Number of preallocated constraints in the optimization task.
    """
    maxnumcon_ = ctypes.c_int32()
    res = __library__.MSK_XX_getmaxnumcon(self.__nativep,ctypes.byref(maxnumcon_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    maxnumcon_ = maxnumcon_.value
    _maxnumcon_return_value = maxnumcon_
    return (_maxnumcon_return_value)
  @accepts(_accept_any)
  @synchronized
  def getmaxnumvar(self):
    """
    Obtains the maximum number variables allowed.
  
    getmaxnumvar(self)
    returns: maxnumvar
      maxnumvar: int. Number of preallocated variables in the optimization task.
    """
    maxnumvar_ = ctypes.c_int32()
    res = __library__.MSK_XX_getmaxnumvar(self.__nativep,ctypes.byref(maxnumvar_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    maxnumvar_ = maxnumvar_.value
    _maxnumvar_return_value = maxnumvar_
    return (_maxnumvar_return_value)
  @accepts(_accept_any,_make_int)
  @synchronized
  def getbarvarnamelen(self,i_):
    """
    Obtains the length of the name of a semidefinite variable.
  
    getbarvarnamelen(self,i_)
      i: int. Index of the variable.
    returns: len
      len: int. Returns the length of the indicated name.
    """
    len_ = ctypes.c_int32()
    res = __library__.MSK_XX_getbarvarnamelen(self.__nativep,i_,ctypes.byref(len_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    len_ = len_.value
    _len_return_value = len_
    return (_len_return_value)
  @accepts(_accept_any,_make_int)
  @synchronized
  def getbarvarname(self,i_):
    """
    Obtains the name of a semidefinite variable.
  
    getbarvarname(self,i_)
      i: int. Index of the variable.
    returns: name
      name: unicode. The requested name is copied to this buffer.
    """
    sizename_ = (1 + self.getbarvarnamelen((i_)))
    name_ = (ctypes.c_char * (sizename_))()
    res = __library__.MSK_XX_getbarvarname(self.__nativep,i_,sizename_,name_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _name_retval = name_.value.decode("utf-8",errors="replace")
    return (_name_retval)
  @accepts(_accept_any,_accept_str)
  @synchronized
  def getbarvarnameindex(self,somename_):
    """
    Obtains the index of semidefinite variable from its name.
  
    getbarvarnameindex(self,somename_)
      somename: str|unicode. The name of the variable.
    returns: asgn,index
      asgn: int. Non-zero if the name somename is assigned to some semidefinite variable.
      index: int. The index of a semidefinite variable with the name somename (if one exists).
    """
    if isinstance(somename_,unicode):
      somename_ = somename_.encode("utf-8",errors="replace")
    asgn_ = ctypes.c_int32()
    index_ = ctypes.c_int32()
    res = __library__.MSK_XX_getbarvarnameindex(self.__nativep,somename_,ctypes.byref(asgn_),ctypes.byref(index_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    asgn_ = asgn_.value
    _asgn_return_value = asgn_
    index_ = index_.value
    _index_return_value = index_
    return (_asgn_return_value,_index_return_value)
  @accepts(_accept_any,_make_intvector,_accept_str,_make_intvector,_make_longvector)
  @synchronized
  def generatevarnames(self,subj_,fmt_,dims_,sp_):
    """
    Generates systematic names for variables.
  
    generatevarnames(self,subj_,fmt_,dims_,sp_)
      subj: array of int. Indexes of the variables.
      fmt: str|unicode. The variable name formatting string.
      dims: array of int. Dimensions in the shape.
      sp: array of long. Items that should be named.
    """
    num_ = None
    if num_ is None:
      num_ = len(subj_)
    elif num_ != len(subj_):
      raise IndexError("Inconsistent length of array subj")
    if subj_ is None:
      raise ValueError("Argument subj cannot be None")
    if subj_ is None:
      raise ValueError("Argument subj may not be None")
    if isinstance(subj_, numpy.ndarray) and subj_.dtype is numpy.dtype(numpy.int32) and subj_.flags.contiguous:
      _subj_copyarray = False
      _subj_tmp = ctypes.cast(subj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subj_ is not None:
      _subj_copyarray = True
      _subj_np_tmp = numpy.zeros(len(subj_),numpy.dtype(numpy.int32))
      _subj_np_tmp[:] = subj_
      assert _subj_np_tmp.flags.contiguous
      _subj_tmp = ctypes.cast(_subj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subj_copyarray = False
      _subj_tmp = None
      
    if isinstance(fmt_,unicode):
      fmt_ = fmt_.encode("utf-8",errors="replace")
    ndims_ = None
    if ndims_ is None:
      ndims_ = len(dims_)
    elif ndims_ != len(dims_):
      raise IndexError("Inconsistent length of array dims")
    if dims_ is None:
      raise ValueError("Argument dims cannot be None")
    if dims_ is None:
      raise ValueError("Argument dims may not be None")
    if isinstance(dims_, numpy.ndarray) and dims_.dtype is numpy.dtype(numpy.int32) and dims_.flags.contiguous:
      _dims_copyarray = False
      _dims_tmp = ctypes.cast(dims_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif dims_ is not None:
      _dims_copyarray = True
      _dims_np_tmp = numpy.zeros(len(dims_),numpy.dtype(numpy.int32))
      _dims_np_tmp[:] = dims_
      assert _dims_np_tmp.flags.contiguous
      _dims_tmp = ctypes.cast(_dims_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _dims_copyarray = False
      _dims_tmp = None
      
    _sp_minlength = (num_)
    if (num_) > 0 and sp_ is not None and len(sp_) != (num_):
      raise ValueError("Array argument sp is not long enough: Is %d, expected %d" % (len(sp_),(num_)))
    if isinstance(sp_, numpy.ndarray) and sp_.dtype is numpy.dtype(numpy.int64) and sp_.flags.contiguous:
      _sp_copyarray = False
      _sp_tmp = ctypes.cast(sp_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif sp_ is not None:
      _sp_copyarray = True
      _sp_np_tmp = numpy.zeros(len(sp_),numpy.dtype(numpy.int64))
      _sp_np_tmp[:] = sp_
      assert _sp_np_tmp.flags.contiguous
      _sp_tmp = ctypes.cast(_sp_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _sp_copyarray = False
      _sp_tmp = None
      
    res = __library__.MSK_XX_generatevarnames(self.__nativep,num_,_subj_tmp,fmt_,ndims_,_dims_tmp,_sp_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_intvector,_accept_str,_make_intvector,_make_longvector)
  @synchronized
  def generateconnames(self,subi_,fmt_,dims_,sp_):
    """
    Generates systematic names for constraints.
  
    generateconnames(self,subi_,fmt_,dims_,sp_)
      subi: array of int. Indexes of the constraints.
      fmt: str|unicode. The constraint name formatting string.
      dims: array of int. Dimensions in the shape.
      sp: array of long. Items that should be named.
    """
    num_ = None
    if num_ is None:
      num_ = len(subi_)
    elif num_ != len(subi_):
      raise IndexError("Inconsistent length of array subi")
    if subi_ is None:
      raise ValueError("Argument subi cannot be None")
    if subi_ is None:
      raise ValueError("Argument subi may not be None")
    if isinstance(subi_, numpy.ndarray) and subi_.dtype is numpy.dtype(numpy.int32) and subi_.flags.contiguous:
      _subi_copyarray = False
      _subi_tmp = ctypes.cast(subi_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subi_ is not None:
      _subi_copyarray = True
      _subi_np_tmp = numpy.zeros(len(subi_),numpy.dtype(numpy.int32))
      _subi_np_tmp[:] = subi_
      assert _subi_np_tmp.flags.contiguous
      _subi_tmp = ctypes.cast(_subi_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subi_copyarray = False
      _subi_tmp = None
      
    if isinstance(fmt_,unicode):
      fmt_ = fmt_.encode("utf-8",errors="replace")
    ndims_ = None
    if ndims_ is None:
      ndims_ = len(dims_)
    elif ndims_ != len(dims_):
      raise IndexError("Inconsistent length of array dims")
    if dims_ is None:
      raise ValueError("Argument dims cannot be None")
    if dims_ is None:
      raise ValueError("Argument dims may not be None")
    if isinstance(dims_, numpy.ndarray) and dims_.dtype is numpy.dtype(numpy.int32) and dims_.flags.contiguous:
      _dims_copyarray = False
      _dims_tmp = ctypes.cast(dims_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif dims_ is not None:
      _dims_copyarray = True
      _dims_np_tmp = numpy.zeros(len(dims_),numpy.dtype(numpy.int32))
      _dims_np_tmp[:] = dims_
      assert _dims_np_tmp.flags.contiguous
      _dims_tmp = ctypes.cast(_dims_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _dims_copyarray = False
      _dims_tmp = None
      
    _sp_minlength = (num_)
    if (num_) > 0 and sp_ is not None and len(sp_) != (num_):
      raise ValueError("Array argument sp is not long enough: Is %d, expected %d" % (len(sp_),(num_)))
    if isinstance(sp_, numpy.ndarray) and sp_.dtype is numpy.dtype(numpy.int64) and sp_.flags.contiguous:
      _sp_copyarray = False
      _sp_tmp = ctypes.cast(sp_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif sp_ is not None:
      _sp_copyarray = True
      _sp_np_tmp = numpy.zeros(len(sp_),numpy.dtype(numpy.int64))
      _sp_np_tmp[:] = sp_
      assert _sp_np_tmp.flags.contiguous
      _sp_tmp = ctypes.cast(_sp_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _sp_copyarray = False
      _sp_tmp = None
      
    res = __library__.MSK_XX_generateconnames(self.__nativep,num_,_subi_tmp,fmt_,ndims_,_dims_tmp,_sp_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_intvector,_accept_str,_make_intvector,_make_longvector)
  @synchronized
  def generateconenames(self,subk_,fmt_,dims_,sp_):
    """
    Generates systematic names for cone.
  
    generateconenames(self,subk_,fmt_,dims_,sp_)
      subk: array of int. Indexes of the cone.
      fmt: str|unicode. The cone name formatting string.
      dims: array of int. Dimensions in the shape.
      sp: array of long. Items that should be named.
    """
    num_ = None
    if num_ is None:
      num_ = len(subk_)
    elif num_ != len(subk_):
      raise IndexError("Inconsistent length of array subk")
    if subk_ is None:
      raise ValueError("Argument subk cannot be None")
    if subk_ is None:
      raise ValueError("Argument subk may not be None")
    if isinstance(subk_, numpy.ndarray) and subk_.dtype is numpy.dtype(numpy.int32) and subk_.flags.contiguous:
      _subk_copyarray = False
      _subk_tmp = ctypes.cast(subk_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subk_ is not None:
      _subk_copyarray = True
      _subk_np_tmp = numpy.zeros(len(subk_),numpy.dtype(numpy.int32))
      _subk_np_tmp[:] = subk_
      assert _subk_np_tmp.flags.contiguous
      _subk_tmp = ctypes.cast(_subk_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subk_copyarray = False
      _subk_tmp = None
      
    if isinstance(fmt_,unicode):
      fmt_ = fmt_.encode("utf-8",errors="replace")
    ndims_ = None
    if ndims_ is None:
      ndims_ = len(dims_)
    elif ndims_ != len(dims_):
      raise IndexError("Inconsistent length of array dims")
    if dims_ is None:
      raise ValueError("Argument dims cannot be None")
    if dims_ is None:
      raise ValueError("Argument dims may not be None")
    if isinstance(dims_, numpy.ndarray) and dims_.dtype is numpy.dtype(numpy.int32) and dims_.flags.contiguous:
      _dims_copyarray = False
      _dims_tmp = ctypes.cast(dims_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif dims_ is not None:
      _dims_copyarray = True
      _dims_np_tmp = numpy.zeros(len(dims_),numpy.dtype(numpy.int32))
      _dims_np_tmp[:] = dims_
      assert _dims_np_tmp.flags.contiguous
      _dims_tmp = ctypes.cast(_dims_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _dims_copyarray = False
      _dims_tmp = None
      
    _sp_minlength = (num_)
    if (num_) > 0 and sp_ is not None and len(sp_) != (num_):
      raise ValueError("Array argument sp is not long enough: Is %d, expected %d" % (len(sp_),(num_)))
    if isinstance(sp_, numpy.ndarray) and sp_.dtype is numpy.dtype(numpy.int64) and sp_.flags.contiguous:
      _sp_copyarray = False
      _sp_tmp = ctypes.cast(sp_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif sp_ is not None:
      _sp_copyarray = True
      _sp_np_tmp = numpy.zeros(len(sp_),numpy.dtype(numpy.int64))
      _sp_np_tmp[:] = sp_
      assert _sp_np_tmp.flags.contiguous
      _sp_tmp = ctypes.cast(_sp_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _sp_copyarray = False
      _sp_tmp = None
      
    res = __library__.MSK_XX_generateconenames(self.__nativep,num_,_subk_tmp,fmt_,ndims_,_dims_tmp,_sp_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_accept_str)
  @synchronized
  def putconname(self,i_,name_):
    """
    Sets the name of a constraint.
  
    putconname(self,i_,name_)
      i: int. Index of the constraint.
      name: str|unicode. The name of the constraint.
    """
    if isinstance(name_,unicode):
      name_ = name_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_putconname(self.__nativep,i_,name_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_accept_str)
  @synchronized
  def putvarname(self,j_,name_):
    """
    Sets the name of a variable.
  
    putvarname(self,j_,name_)
      j: int. Index of the variable.
      name: str|unicode. The variable name.
    """
    if isinstance(name_,unicode):
      name_ = name_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_putvarname(self.__nativep,j_,name_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_accept_str)
  @synchronized
  def putconename(self,j_,name_):
    """
    Sets the name of a cone.
  
    putconename(self,j_,name_)
      j: int. Index of the cone.
      name: str|unicode. The name of the cone.
    """
    if isinstance(name_,unicode):
      name_ = name_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_putconename(self.__nativep,j_,name_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_accept_str)
  @synchronized
  def putbarvarname(self,j_,name_):
    """
    Sets the name of a semidefinite variable.
  
    putbarvarname(self,j_,name_)
      j: int. Index of the variable.
      name: str|unicode. The variable name.
    """
    if isinstance(name_,unicode):
      name_ = name_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_putbarvarname(self.__nativep,j_,name_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int)
  @synchronized
  def getvarnamelen(self,i_):
    """
    Obtains the length of the name of a variable.
  
    getvarnamelen(self,i_)
      i: int. Index of a variable.
    returns: len
      len: int. Returns the length of the indicated name.
    """
    len_ = ctypes.c_int32()
    res = __library__.MSK_XX_getvarnamelen(self.__nativep,i_,ctypes.byref(len_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    len_ = len_.value
    _len_return_value = len_
    return (_len_return_value)
  @accepts(_accept_any,_make_int)
  @synchronized
  def getvarname(self,j_):
    """
    Obtains the name of a variable.
  
    getvarname(self,j_)
      j: int. Index of a variable.
    returns: name
      name: unicode. Returns the required name.
    """
    sizename_ = (1 + self.getvarnamelen((j_)))
    name_ = (ctypes.c_char * (sizename_))()
    res = __library__.MSK_XX_getvarname(self.__nativep,j_,sizename_,name_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _name_retval = name_.value.decode("utf-8",errors="replace")
    return (_name_retval)
  @accepts(_accept_any,_make_int)
  @synchronized
  def getconnamelen(self,i_):
    """
    Obtains the length of the name of a constraint.
  
    getconnamelen(self,i_)
      i: int. Index of the constraint.
    returns: len
      len: int. Returns the length of the indicated name.
    """
    len_ = ctypes.c_int32()
    res = __library__.MSK_XX_getconnamelen(self.__nativep,i_,ctypes.byref(len_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    len_ = len_.value
    _len_return_value = len_
    return (_len_return_value)
  @accepts(_accept_any,_make_int)
  @synchronized
  def getconname(self,i_):
    """
    Obtains the name of a constraint.
  
    getconname(self,i_)
      i: int. Index of the constraint.
    returns: name
      name: unicode. The required name.
    """
    sizename_ = (1 + self.getconnamelen((i_)))
    name_ = (ctypes.c_char * (sizename_))()
    res = __library__.MSK_XX_getconname(self.__nativep,i_,sizename_,name_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _name_retval = name_.value.decode("utf-8",errors="replace")
    return (_name_retval)
  @accepts(_accept_any,_accept_str)
  @synchronized
  def getconnameindex(self,somename_):
    """
    Checks whether the name has been assigned to any constraint.
  
    getconnameindex(self,somename_)
      somename: str|unicode. The name which should be checked.
    returns: asgn,index
      asgn: int. Is non-zero if the name somename is assigned to some constraint.
      index: int. If the name somename is assigned to a constraint, then return the index of the constraint.
    """
    if isinstance(somename_,unicode):
      somename_ = somename_.encode("utf-8",errors="replace")
    asgn_ = ctypes.c_int32()
    index_ = ctypes.c_int32()
    res = __library__.MSK_XX_getconnameindex(self.__nativep,somename_,ctypes.byref(asgn_),ctypes.byref(index_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    asgn_ = asgn_.value
    _asgn_return_value = asgn_
    index_ = index_.value
    _index_return_value = index_
    return (_asgn_return_value,_index_return_value)
  @accepts(_accept_any,_accept_str)
  @synchronized
  def getvarnameindex(self,somename_):
    """
    Checks whether the name has been assigned to any variable.
  
    getvarnameindex(self,somename_)
      somename: str|unicode. The name which should be checked.
    returns: asgn,index
      asgn: int. Is non-zero if the name somename is assigned to a variable.
      index: int. If the name somename is assigned to a variable, then return the index of the variable.
    """
    if isinstance(somename_,unicode):
      somename_ = somename_.encode("utf-8",errors="replace")
    asgn_ = ctypes.c_int32()
    index_ = ctypes.c_int32()
    res = __library__.MSK_XX_getvarnameindex(self.__nativep,somename_,ctypes.byref(asgn_),ctypes.byref(index_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    asgn_ = asgn_.value
    _asgn_return_value = asgn_
    index_ = index_.value
    _index_return_value = index_
    return (_asgn_return_value,_index_return_value)
  @accepts(_accept_any,_make_int)
  @synchronized
  def getconenamelen(self,i_):
    """
    Obtains the length of the name of a cone.
  
    getconenamelen(self,i_)
      i: int. Index of the cone.
    returns: len
      len: int. Returns the length of the indicated name.
    """
    len_ = ctypes.c_int32()
    res = __library__.MSK_XX_getconenamelen(self.__nativep,i_,ctypes.byref(len_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    len_ = len_.value
    _len_return_value = len_
    return (_len_return_value)
  @accepts(_accept_any,_make_int)
  @synchronized
  def getconename(self,i_):
    """
    Obtains the name of a cone.
  
    getconename(self,i_)
      i: int. Index of the cone.
    returns: name
      name: unicode. The required name.
    """
    sizename_ = (1 + self.getconenamelen((i_)))
    name_ = (ctypes.c_char * (sizename_))()
    res = __library__.MSK_XX_getconename(self.__nativep,i_,sizename_,name_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _name_retval = name_.value.decode("utf-8",errors="replace")
    return (_name_retval)
  @accepts(_accept_any,_accept_str)
  @synchronized
  def getconenameindex(self,somename_):
    """
    Checks whether the name has been assigned to any cone.
  
    getconenameindex(self,somename_)
      somename: str|unicode. The name which should be checked.
    returns: asgn,index
      asgn: int. Is non-zero if the name somename is assigned to some cone.
      index: int. If the name somename is assigned to some cone, this is the index of the cone.
    """
    if isinstance(somename_,unicode):
      somename_ = somename_.encode("utf-8",errors="replace")
    asgn_ = ctypes.c_int32()
    index_ = ctypes.c_int32()
    res = __library__.MSK_XX_getconenameindex(self.__nativep,somename_,ctypes.byref(asgn_),ctypes.byref(index_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    asgn_ = asgn_.value
    _asgn_return_value = asgn_
    index_ = index_.value
    _index_return_value = index_
    return (_asgn_return_value,_index_return_value)
  @accepts(_accept_any)
  @synchronized
  def getnumanz(self):
    """
    Obtains the number of non-zeros in the coefficient matrix.
  
    getnumanz(self)
    returns: numanz
      numanz: int. Number of non-zero elements in the linear constraint matrix.
    """
    numanz_ = ctypes.c_int32()
    res = __library__.MSK_XX_getnumanz(self.__nativep,ctypes.byref(numanz_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    numanz_ = numanz_.value
    _numanz_return_value = numanz_
    return (_numanz_return_value)
  @accepts(_accept_any)
  @synchronized
  def getnumanz64(self):
    """
    Obtains the number of non-zeros in the coefficient matrix.
  
    getnumanz64(self)
    returns: numanz
      numanz: long. Number of non-zero elements in the linear constraint matrix.
    """
    numanz_ = ctypes.c_int64()
    res = __library__.MSK_XX_getnumanz64(self.__nativep,ctypes.byref(numanz_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    numanz_ = numanz_.value
    _numanz_return_value = numanz_
    return (_numanz_return_value)
  @accepts(_accept_any)
  @synchronized
  def getnumcon(self):
    """
    Obtains the number of constraints.
  
    getnumcon(self)
    returns: numcon
      numcon: int. Number of constraints.
    """
    numcon_ = ctypes.c_int32()
    res = __library__.MSK_XX_getnumcon(self.__nativep,ctypes.byref(numcon_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    numcon_ = numcon_.value
    _numcon_return_value = numcon_
    return (_numcon_return_value)
  @accepts(_accept_any)
  @synchronized
  def getnumcone(self):
    """
    Obtains the number of cones.
  
    getnumcone(self)
    returns: numcone
      numcone: int. Number of conic constraints.
    """
    numcone_ = ctypes.c_int32()
    res = __library__.MSK_XX_getnumcone(self.__nativep,ctypes.byref(numcone_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    numcone_ = numcone_.value
    _numcone_return_value = numcone_
    return (_numcone_return_value)
  @accepts(_accept_any,_make_int)
  @synchronized
  def getnumconemem(self,k_):
    """
    Obtains the number of members in a cone.
  
    getnumconemem(self,k_)
      k: int. Index of the cone.
    returns: nummem
      nummem: int. Number of member variables in the cone.
    """
    nummem_ = ctypes.c_int32()
    res = __library__.MSK_XX_getnumconemem(self.__nativep,k_,ctypes.byref(nummem_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    nummem_ = nummem_.value
    _nummem_return_value = nummem_
    return (_nummem_return_value)
  @accepts(_accept_any)
  @synchronized
  def getnumintvar(self):
    """
    Obtains the number of integer-constrained variables.
  
    getnumintvar(self)
    returns: numintvar
      numintvar: int. Number of integer variables.
    """
    numintvar_ = ctypes.c_int32()
    res = __library__.MSK_XX_getnumintvar(self.__nativep,ctypes.byref(numintvar_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    numintvar_ = numintvar_.value
    _numintvar_return_value = numintvar_
    return (_numintvar_return_value)
  @accepts(_accept_any,_accept_anyenum(parametertype))
  @synchronized
  def getnumparam(self,partype_):
    """
    Obtains the number of parameters of a given type.
  
    getnumparam(self,partype_)
      partype: mosek.parametertype. Parameter type.
    returns: numparam
      numparam: int. Returns the number of parameters of the requested type.
    """
    numparam_ = ctypes.c_int32()
    res = __library__.MSK_XX_getnumparam(self.__nativep,partype_,ctypes.byref(numparam_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    numparam_ = numparam_.value
    _numparam_return_value = numparam_
    return (_numparam_return_value)
  @accepts(_accept_any,_make_int)
  @synchronized
  def getnumqconknz(self,k_):
    """
    Obtains the number of non-zero quadratic terms in a constraint.
  
    getnumqconknz(self,k_)
      k: int. Index of the constraint for which the number quadratic terms should be obtained.
    returns: numqcnz
      numqcnz: long. Number of quadratic terms.
    """
    numqcnz_ = ctypes.c_int64()
    res = __library__.MSK_XX_getnumqconknz64(self.__nativep,k_,ctypes.byref(numqcnz_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    numqcnz_ = numqcnz_.value
    _numqcnz_return_value = numqcnz_
    return (_numqcnz_return_value)
  @accepts(_accept_any)
  @synchronized
  def getnumqobjnz(self):
    """
    Obtains the number of non-zero quadratic terms in the objective.
  
    getnumqobjnz(self)
    returns: numqonz
      numqonz: long. Number of non-zero elements in the quadratic objective terms.
    """
    numqonz_ = ctypes.c_int64()
    res = __library__.MSK_XX_getnumqobjnz64(self.__nativep,ctypes.byref(numqonz_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    numqonz_ = numqonz_.value
    _numqonz_return_value = numqonz_
    return (_numqonz_return_value)
  @accepts(_accept_any)
  @synchronized
  def getnumvar(self):
    """
    Obtains the number of variables.
  
    getnumvar(self)
    returns: numvar
      numvar: int. Number of variables.
    """
    numvar_ = ctypes.c_int32()
    res = __library__.MSK_XX_getnumvar(self.__nativep,ctypes.byref(numvar_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    numvar_ = numvar_.value
    _numvar_return_value = numvar_
    return (_numvar_return_value)
  @accepts(_accept_any)
  @synchronized
  def getnumbarvar(self):
    """
    Obtains the number of semidefinite variables.
  
    getnumbarvar(self)
    returns: numbarvar
      numbarvar: int. Number of semidefinite variables in the problem.
    """
    numbarvar_ = ctypes.c_int32()
    res = __library__.MSK_XX_getnumbarvar(self.__nativep,ctypes.byref(numbarvar_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    numbarvar_ = numbarvar_.value
    _numbarvar_return_value = numbarvar_
    return (_numbarvar_return_value)
  @accepts(_accept_any)
  @synchronized
  def getmaxnumbarvar(self):
    """
    Obtains maximum number of symmetric matrix variables for which space is currently preallocated.
  
    getmaxnumbarvar(self)
    returns: maxnumbarvar
      maxnumbarvar: int. Maximum number of symmetric matrix variables for which space is currently preallocated.
    """
    maxnumbarvar_ = ctypes.c_int32()
    res = __library__.MSK_XX_getmaxnumbarvar(self.__nativep,ctypes.byref(maxnumbarvar_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    maxnumbarvar_ = maxnumbarvar_.value
    _maxnumbarvar_return_value = maxnumbarvar_
    return (_maxnumbarvar_return_value)
  @accepts(_accept_any,_make_int)
  @synchronized
  def getdimbarvarj(self,j_):
    """
    Obtains the dimension of a symmetric matrix variable.
  
    getdimbarvarj(self,j_)
      j: int. Index of the semidefinite variable whose dimension is requested.
    returns: dimbarvarj
      dimbarvarj: int. The dimension of the j'th semidefinite variable.
    """
    dimbarvarj_ = ctypes.c_int32()
    res = __library__.MSK_XX_getdimbarvarj(self.__nativep,j_,ctypes.byref(dimbarvarj_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    dimbarvarj_ = dimbarvarj_.value
    _dimbarvarj_return_value = dimbarvarj_
    return (_dimbarvarj_return_value)
  @accepts(_accept_any,_make_int)
  @synchronized
  def getlenbarvarj(self,j_):
    """
    Obtains the length of one semidefinite variable.
  
    getlenbarvarj(self,j_)
      j: int. Index of the semidefinite variable whose length if requested.
    returns: lenbarvarj
      lenbarvarj: long. Number of scalar elements in the lower triangular part of the semidefinite variable.
    """
    lenbarvarj_ = ctypes.c_int64()
    res = __library__.MSK_XX_getlenbarvarj(self.__nativep,j_,ctypes.byref(lenbarvarj_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    lenbarvarj_ = lenbarvarj_.value
    _lenbarvarj_return_value = lenbarvarj_
    return (_lenbarvarj_return_value)
  @accepts(_accept_any)
  @synchronized
  def getobjname(self):
    """
    Obtains the name assigned to the objective function.
  
    getobjname(self)
    returns: objname
      objname: unicode. Assigned the objective name.
    """
    sizeobjname_ = (1 + self.getobjnamelen())
    objname_ = (ctypes.c_char * (sizeobjname_))()
    res = __library__.MSK_XX_getobjname(self.__nativep,sizeobjname_,objname_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _objname_retval = objname_.value.decode("utf-8",errors="replace")
    return (_objname_retval)
  @accepts(_accept_any)
  @synchronized
  def getobjnamelen(self):
    """
    Obtains the length of the name assigned to the objective function.
  
    getobjnamelen(self)
    returns: len
      len: int. Assigned the length of the objective name.
    """
    len_ = ctypes.c_int32()
    res = __library__.MSK_XX_getobjnamelen(self.__nativep,ctypes.byref(len_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    len_ = len_.value
    _len_return_value = len_
    return (_len_return_value)
  @accepts(_accept_any,_accept_anyenum(soltype))
  @synchronized
  def getprimalobj(self,whichsol_):
    """
    Computes the primal objective value for the desired solution.
  
    getprimalobj(self,whichsol_)
      whichsol: mosek.soltype. Selects a solution.
    returns: primalobj
      primalobj: double. Objective value corresponding to the primal solution.
    """
    primalobj_ = ctypes.c_double()
    res = __library__.MSK_XX_getprimalobj(self.__nativep,whichsol_,ctypes.byref(primalobj_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    primalobj_ = primalobj_.value
    _primalobj_return_value = primalobj_
    return (_primalobj_return_value)
  @accepts(_accept_any)
  @synchronized
  def getprobtype(self):
    """
    Obtains the problem type.
  
    getprobtype(self)
    returns: probtype
      probtype: mosek.problemtype. The problem type.
    """
    probtype_ = ctypes.c_int32()
    res = __library__.MSK_XX_getprobtype(self.__nativep,ctypes.byref(probtype_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _probtype_return_value = problemtype(probtype_.value)
    return (_probtype_return_value)
  @accepts(_accept_any,_make_int,_accept_intvector,_accept_intvector,_accept_doublevector)
  @synchronized
  def getqconk(self,k_,qcsubi_,qcsubj_,qcval_):
    """
    Obtains all the quadratic terms in a constraint.
  
    getqconk(self,k_,qcsubi_,qcsubj_,qcval_)
      k: int. Which constraint.
      qcsubi: array of int. Row subscripts for quadratic constraint matrix.
      qcsubj: array of int. Column subscripts for quadratic constraint matrix.
      qcval: array of double. Quadratic constraint coefficient values.
    returns: numqcnz
      numqcnz: long. Number of quadratic terms.
    """
    maxnumqcnz_ = self.getnumqconknz((k_))
    numqcnz_ = ctypes.c_int64()
    _qcsubi_minlength = self.getnumqconknz((k_))
    if self.getnumqconknz((k_)) > 0 and qcsubi_ is not None and len(qcsubi_) != self.getnumqconknz((k_)):
      raise ValueError("Array argument qcsubi is not long enough: Is %d, expected %d" % (len(qcsubi_),self.getnumqconknz((k_))))
    if isinstance(qcsubi_,numpy.ndarray) and not qcsubi_.flags.writeable:
      raise ValueError("Argument qcsubi must be writable")
    if qcsubi_ is None:
      raise ValueError("Argument qcsubi may not be None")
    if isinstance(qcsubi_, numpy.ndarray) and qcsubi_.dtype is numpy.dtype(numpy.int32) and qcsubi_.flags.contiguous:
      _qcsubi_copyarray = False
      _qcsubi_tmp = ctypes.cast(qcsubi_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif qcsubi_ is not None:
      _qcsubi_copyarray = True
      _qcsubi_np_tmp = numpy.zeros(len(qcsubi_),numpy.dtype(numpy.int32))
      _qcsubi_np_tmp[:] = qcsubi_
      assert _qcsubi_np_tmp.flags.contiguous
      _qcsubi_tmp = ctypes.cast(_qcsubi_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _qcsubi_copyarray = False
      _qcsubi_tmp = None
      
    _qcsubj_minlength = self.getnumqconknz((k_))
    if self.getnumqconknz((k_)) > 0 and qcsubj_ is not None and len(qcsubj_) != self.getnumqconknz((k_)):
      raise ValueError("Array argument qcsubj is not long enough: Is %d, expected %d" % (len(qcsubj_),self.getnumqconknz((k_))))
    if isinstance(qcsubj_,numpy.ndarray) and not qcsubj_.flags.writeable:
      raise ValueError("Argument qcsubj must be writable")
    if qcsubj_ is None:
      raise ValueError("Argument qcsubj may not be None")
    if isinstance(qcsubj_, numpy.ndarray) and qcsubj_.dtype is numpy.dtype(numpy.int32) and qcsubj_.flags.contiguous:
      _qcsubj_copyarray = False
      _qcsubj_tmp = ctypes.cast(qcsubj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif qcsubj_ is not None:
      _qcsubj_copyarray = True
      _qcsubj_np_tmp = numpy.zeros(len(qcsubj_),numpy.dtype(numpy.int32))
      _qcsubj_np_tmp[:] = qcsubj_
      assert _qcsubj_np_tmp.flags.contiguous
      _qcsubj_tmp = ctypes.cast(_qcsubj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _qcsubj_copyarray = False
      _qcsubj_tmp = None
      
    _qcval_minlength = self.getnumqconknz((k_))
    if self.getnumqconknz((k_)) > 0 and qcval_ is not None and len(qcval_) != self.getnumqconknz((k_)):
      raise ValueError("Array argument qcval is not long enough: Is %d, expected %d" % (len(qcval_),self.getnumqconknz((k_))))
    if isinstance(qcval_,numpy.ndarray) and not qcval_.flags.writeable:
      raise ValueError("Argument qcval must be writable")
    if qcval_ is None:
      raise ValueError("Argument qcval may not be None")
    if isinstance(qcval_, numpy.ndarray) and qcval_.dtype is numpy.dtype(numpy.float64) and qcval_.flags.contiguous:
      _qcval_copyarray = False
      _qcval_tmp = ctypes.cast(qcval_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif qcval_ is not None:
      _qcval_copyarray = True
      _qcval_np_tmp = numpy.zeros(len(qcval_),numpy.dtype(numpy.float64))
      _qcval_np_tmp[:] = qcval_
      assert _qcval_np_tmp.flags.contiguous
      _qcval_tmp = ctypes.cast(_qcval_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _qcval_copyarray = False
      _qcval_tmp = None
      
    qcsurp_ = ctypes.c_int64(_qcsubi_minlength)
    res = __library__.MSK_XX_getqconk64(self.__nativep,k_,maxnumqcnz_,ctypes.byref(qcsurp_),ctypes.byref(numqcnz_),_qcsubi_tmp,_qcsubj_tmp,_qcval_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    numqcnz_ = numqcnz_.value
    _numqcnz_return_value = numqcnz_
    if _qcsubi_copyarray:
      qcsubi_[:] = _qcsubi_np_tmp
    if _qcsubj_copyarray:
      qcsubj_[:] = _qcsubj_np_tmp
    if _qcval_copyarray:
      qcval_[:] = _qcval_np_tmp
    return (_numqcnz_return_value)
  @accepts(_accept_any,_accept_intvector,_accept_intvector,_accept_doublevector)
  @synchronized
  def getqobj(self,qosubi_,qosubj_,qoval_):
    """
    Obtains all the quadratic terms in the objective.
  
    getqobj(self,qosubi_,qosubj_,qoval_)
      qosubi: array of int. Row subscripts for quadratic objective coefficients.
      qosubj: array of int. Column subscripts for quadratic objective coefficients.
      qoval: array of double. Quadratic objective coefficient values.
    returns: numqonz
      numqonz: long. Number of non-zero elements in the quadratic objective terms.
    """
    maxnumqonz_ = self.getnumqobjnz()
    numqonz_ = ctypes.c_int64()
    _qosubi_minlength = (maxnumqonz_)
    if (maxnumqonz_) > 0 and qosubi_ is not None and len(qosubi_) != (maxnumqonz_):
      raise ValueError("Array argument qosubi is not long enough: Is %d, expected %d" % (len(qosubi_),(maxnumqonz_)))
    if isinstance(qosubi_,numpy.ndarray) and not qosubi_.flags.writeable:
      raise ValueError("Argument qosubi must be writable")
    if qosubi_ is None:
      raise ValueError("Argument qosubi may not be None")
    if isinstance(qosubi_, numpy.ndarray) and qosubi_.dtype is numpy.dtype(numpy.int32) and qosubi_.flags.contiguous:
      _qosubi_copyarray = False
      _qosubi_tmp = ctypes.cast(qosubi_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif qosubi_ is not None:
      _qosubi_copyarray = True
      _qosubi_np_tmp = numpy.zeros(len(qosubi_),numpy.dtype(numpy.int32))
      _qosubi_np_tmp[:] = qosubi_
      assert _qosubi_np_tmp.flags.contiguous
      _qosubi_tmp = ctypes.cast(_qosubi_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _qosubi_copyarray = False
      _qosubi_tmp = None
      
    _qosubj_minlength = (maxnumqonz_)
    if (maxnumqonz_) > 0 and qosubj_ is not None and len(qosubj_) != (maxnumqonz_):
      raise ValueError("Array argument qosubj is not long enough: Is %d, expected %d" % (len(qosubj_),(maxnumqonz_)))
    if isinstance(qosubj_,numpy.ndarray) and not qosubj_.flags.writeable:
      raise ValueError("Argument qosubj must be writable")
    if qosubj_ is None:
      raise ValueError("Argument qosubj may not be None")
    if isinstance(qosubj_, numpy.ndarray) and qosubj_.dtype is numpy.dtype(numpy.int32) and qosubj_.flags.contiguous:
      _qosubj_copyarray = False
      _qosubj_tmp = ctypes.cast(qosubj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif qosubj_ is not None:
      _qosubj_copyarray = True
      _qosubj_np_tmp = numpy.zeros(len(qosubj_),numpy.dtype(numpy.int32))
      _qosubj_np_tmp[:] = qosubj_
      assert _qosubj_np_tmp.flags.contiguous
      _qosubj_tmp = ctypes.cast(_qosubj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _qosubj_copyarray = False
      _qosubj_tmp = None
      
    _qoval_minlength = (maxnumqonz_)
    if (maxnumqonz_) > 0 and qoval_ is not None and len(qoval_) != (maxnumqonz_):
      raise ValueError("Array argument qoval is not long enough: Is %d, expected %d" % (len(qoval_),(maxnumqonz_)))
    if isinstance(qoval_,numpy.ndarray) and not qoval_.flags.writeable:
      raise ValueError("Argument qoval must be writable")
    if qoval_ is None:
      raise ValueError("Argument qoval may not be None")
    if isinstance(qoval_, numpy.ndarray) and qoval_.dtype is numpy.dtype(numpy.float64) and qoval_.flags.contiguous:
      _qoval_copyarray = False
      _qoval_tmp = ctypes.cast(qoval_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif qoval_ is not None:
      _qoval_copyarray = True
      _qoval_np_tmp = numpy.zeros(len(qoval_),numpy.dtype(numpy.float64))
      _qoval_np_tmp[:] = qoval_
      assert _qoval_np_tmp.flags.contiguous
      _qoval_tmp = ctypes.cast(_qoval_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _qoval_copyarray = False
      _qoval_tmp = None
      
    qosurp_ = ctypes.c_int64(_qosubi_minlength)
    res = __library__.MSK_XX_getqobj64(self.__nativep,maxnumqonz_,ctypes.byref(qosurp_),ctypes.byref(numqonz_),_qosubi_tmp,_qosubj_tmp,_qoval_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    numqonz_ = numqonz_.value
    _numqonz_return_value = numqonz_
    if _qosubi_copyarray:
      qosubi_[:] = _qosubi_np_tmp
    if _qosubj_copyarray:
      qosubj_[:] = _qosubj_np_tmp
    if _qoval_copyarray:
      qoval_[:] = _qoval_np_tmp
    return (_numqonz_return_value)
  @accepts(_accept_any,_make_int,_make_int)
  @synchronized
  def getqobjij(self,i_,j_):
    """
    Obtains one coefficient from the quadratic term of the objective
  
    getqobjij(self,i_,j_)
      i: int. Row index of the coefficient.
      j: int. Column index of coefficient.
    returns: qoij
      qoij: double. The required coefficient.
    """
    qoij_ = ctypes.c_double()
    res = __library__.MSK_XX_getqobjij(self.__nativep,i_,j_,ctypes.byref(qoij_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    qoij_ = qoij_.value
    _qoij_return_value = qoij_
    return (_qoij_return_value)
  @accepts(_accept_any,_accept_anyenum(soltype),_accept_any,_accept_any,_accept_any,_accept_doublevector,_accept_doublevector,_accept_doublevector,_accept_doublevector,_accept_doublevector,_accept_doublevector,_accept_doublevector,_accept_doublevector)
  @synchronized
  def getsolution(self,whichsol_,skc_,skx_,skn_,xc_,xx_,y_,slc_,suc_,slx_,sux_,snx_):
    """
    Obtains the complete solution.
  
    getsolution(self,whichsol_,skc_,skx_,skn_,xc_,xx_,y_,slc_,suc_,slx_,sux_,snx_)
      whichsol: mosek.soltype. Selects a solution.
      skc: array of mosek.stakey. Status keys for the constraints.
      skx: array of mosek.stakey. Status keys for the variables.
      skn: array of mosek.stakey. Status keys for the conic constraints.
      xc: array of double. Primal constraint solution.
      xx: array of double. Primal variable solution.
      y: array of double. Vector of dual variables corresponding to the constraints.
      slc: array of double. Dual variables corresponding to the lower bounds on the constraints.
      suc: array of double. Dual variables corresponding to the upper bounds on the constraints.
      slx: array of double. Dual variables corresponding to the lower bounds on the variables.
      sux: array of double. Dual variables corresponding to the upper bounds on the variables.
      snx: array of double. Dual variables corresponding to the conic constraints on the variables.
    returns: prosta,solsta
      prosta: mosek.prosta. Problem status.
      solsta: mosek.solsta. Solution status.
    """
    prosta_ = ctypes.c_int32()
    solsta_ = ctypes.c_int32()
    _skc_minlength = self.getnumcon()
    if self.getnumcon() > 0 and skc_ is not None and len(skc_) != self.getnumcon():
      raise ValueError("Array argument skc is not long enough: Is %d, expected %d" % (len(skc_),self.getnumcon()))
    if isinstance(skc_,numpy.ndarray) and not skc_.flags.writeable:
      raise ValueError("Argument skc must be writable")
    if skc_ is not None:
        _skc_tmp = (ctypes.c_int32 * len(skc_))()
    else:
        _skc_tmp = None
    _skx_minlength = self.getnumvar()
    if self.getnumvar() > 0 and skx_ is not None and len(skx_) != self.getnumvar():
      raise ValueError("Array argument skx is not long enough: Is %d, expected %d" % (len(skx_),self.getnumvar()))
    if isinstance(skx_,numpy.ndarray) and not skx_.flags.writeable:
      raise ValueError("Argument skx must be writable")
    if skx_ is not None:
        _skx_tmp = (ctypes.c_int32 * len(skx_))()
    else:
        _skx_tmp = None
    _skn_minlength = self.getnumcone()
    if self.getnumcone() > 0 and skn_ is not None and len(skn_) != self.getnumcone():
      raise ValueError("Array argument skn is not long enough: Is %d, expected %d" % (len(skn_),self.getnumcone()))
    if isinstance(skn_,numpy.ndarray) and not skn_.flags.writeable:
      raise ValueError("Argument skn must be writable")
    if skn_ is not None:
        _skn_tmp = (ctypes.c_int32 * len(skn_))()
    else:
        _skn_tmp = None
    _xc_minlength = self.getnumcon()
    if self.getnumcon() > 0 and xc_ is not None and len(xc_) != self.getnumcon():
      raise ValueError("Array argument xc is not long enough: Is %d, expected %d" % (len(xc_),self.getnumcon()))
    if isinstance(xc_,numpy.ndarray) and not xc_.flags.writeable:
      raise ValueError("Argument xc must be writable")
    if isinstance(xc_, numpy.ndarray) and xc_.dtype is numpy.dtype(numpy.float64) and xc_.flags.contiguous:
      _xc_copyarray = False
      _xc_tmp = ctypes.cast(xc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif xc_ is not None:
      _xc_copyarray = True
      _xc_np_tmp = numpy.zeros(len(xc_),numpy.dtype(numpy.float64))
      _xc_np_tmp[:] = xc_
      assert _xc_np_tmp.flags.contiguous
      _xc_tmp = ctypes.cast(_xc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _xc_copyarray = False
      _xc_tmp = None
      
    _xx_minlength = self.getnumvar()
    if self.getnumvar() > 0 and xx_ is not None and len(xx_) != self.getnumvar():
      raise ValueError("Array argument xx is not long enough: Is %d, expected %d" % (len(xx_),self.getnumvar()))
    if isinstance(xx_,numpy.ndarray) and not xx_.flags.writeable:
      raise ValueError("Argument xx must be writable")
    if isinstance(xx_, numpy.ndarray) and xx_.dtype is numpy.dtype(numpy.float64) and xx_.flags.contiguous:
      _xx_copyarray = False
      _xx_tmp = ctypes.cast(xx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif xx_ is not None:
      _xx_copyarray = True
      _xx_np_tmp = numpy.zeros(len(xx_),numpy.dtype(numpy.float64))
      _xx_np_tmp[:] = xx_
      assert _xx_np_tmp.flags.contiguous
      _xx_tmp = ctypes.cast(_xx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _xx_copyarray = False
      _xx_tmp = None
      
    _y_minlength = self.getnumcon()
    if self.getnumcon() > 0 and y_ is not None and len(y_) != self.getnumcon():
      raise ValueError("Array argument y is not long enough: Is %d, expected %d" % (len(y_),self.getnumcon()))
    if isinstance(y_,numpy.ndarray) and not y_.flags.writeable:
      raise ValueError("Argument y must be writable")
    if isinstance(y_, numpy.ndarray) and y_.dtype is numpy.dtype(numpy.float64) and y_.flags.contiguous:
      _y_copyarray = False
      _y_tmp = ctypes.cast(y_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif y_ is not None:
      _y_copyarray = True
      _y_np_tmp = numpy.zeros(len(y_),numpy.dtype(numpy.float64))
      _y_np_tmp[:] = y_
      assert _y_np_tmp.flags.contiguous
      _y_tmp = ctypes.cast(_y_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _y_copyarray = False
      _y_tmp = None
      
    _slc_minlength = self.getnumcon()
    if self.getnumcon() > 0 and slc_ is not None and len(slc_) != self.getnumcon():
      raise ValueError("Array argument slc is not long enough: Is %d, expected %d" % (len(slc_),self.getnumcon()))
    if isinstance(slc_,numpy.ndarray) and not slc_.flags.writeable:
      raise ValueError("Argument slc must be writable")
    if isinstance(slc_, numpy.ndarray) and slc_.dtype is numpy.dtype(numpy.float64) and slc_.flags.contiguous:
      _slc_copyarray = False
      _slc_tmp = ctypes.cast(slc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif slc_ is not None:
      _slc_copyarray = True
      _slc_np_tmp = numpy.zeros(len(slc_),numpy.dtype(numpy.float64))
      _slc_np_tmp[:] = slc_
      assert _slc_np_tmp.flags.contiguous
      _slc_tmp = ctypes.cast(_slc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _slc_copyarray = False
      _slc_tmp = None
      
    _suc_minlength = self.getnumcon()
    if self.getnumcon() > 0 and suc_ is not None and len(suc_) != self.getnumcon():
      raise ValueError("Array argument suc is not long enough: Is %d, expected %d" % (len(suc_),self.getnumcon()))
    if isinstance(suc_,numpy.ndarray) and not suc_.flags.writeable:
      raise ValueError("Argument suc must be writable")
    if isinstance(suc_, numpy.ndarray) and suc_.dtype is numpy.dtype(numpy.float64) and suc_.flags.contiguous:
      _suc_copyarray = False
      _suc_tmp = ctypes.cast(suc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif suc_ is not None:
      _suc_copyarray = True
      _suc_np_tmp = numpy.zeros(len(suc_),numpy.dtype(numpy.float64))
      _suc_np_tmp[:] = suc_
      assert _suc_np_tmp.flags.contiguous
      _suc_tmp = ctypes.cast(_suc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _suc_copyarray = False
      _suc_tmp = None
      
    _slx_minlength = self.getnumvar()
    if self.getnumvar() > 0 and slx_ is not None and len(slx_) != self.getnumvar():
      raise ValueError("Array argument slx is not long enough: Is %d, expected %d" % (len(slx_),self.getnumvar()))
    if isinstance(slx_,numpy.ndarray) and not slx_.flags.writeable:
      raise ValueError("Argument slx must be writable")
    if isinstance(slx_, numpy.ndarray) and slx_.dtype is numpy.dtype(numpy.float64) and slx_.flags.contiguous:
      _slx_copyarray = False
      _slx_tmp = ctypes.cast(slx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif slx_ is not None:
      _slx_copyarray = True
      _slx_np_tmp = numpy.zeros(len(slx_),numpy.dtype(numpy.float64))
      _slx_np_tmp[:] = slx_
      assert _slx_np_tmp.flags.contiguous
      _slx_tmp = ctypes.cast(_slx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _slx_copyarray = False
      _slx_tmp = None
      
    _sux_minlength = self.getnumvar()
    if self.getnumvar() > 0 and sux_ is not None and len(sux_) != self.getnumvar():
      raise ValueError("Array argument sux is not long enough: Is %d, expected %d" % (len(sux_),self.getnumvar()))
    if isinstance(sux_,numpy.ndarray) and not sux_.flags.writeable:
      raise ValueError("Argument sux must be writable")
    if isinstance(sux_, numpy.ndarray) and sux_.dtype is numpy.dtype(numpy.float64) and sux_.flags.contiguous:
      _sux_copyarray = False
      _sux_tmp = ctypes.cast(sux_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif sux_ is not None:
      _sux_copyarray = True
      _sux_np_tmp = numpy.zeros(len(sux_),numpy.dtype(numpy.float64))
      _sux_np_tmp[:] = sux_
      assert _sux_np_tmp.flags.contiguous
      _sux_tmp = ctypes.cast(_sux_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _sux_copyarray = False
      _sux_tmp = None
      
    _snx_minlength = self.getnumvar()
    if self.getnumvar() > 0 and snx_ is not None and len(snx_) != self.getnumvar():
      raise ValueError("Array argument snx is not long enough: Is %d, expected %d" % (len(snx_),self.getnumvar()))
    if isinstance(snx_,numpy.ndarray) and not snx_.flags.writeable:
      raise ValueError("Argument snx must be writable")
    if isinstance(snx_, numpy.ndarray) and snx_.dtype is numpy.dtype(numpy.float64) and snx_.flags.contiguous:
      _snx_copyarray = False
      _snx_tmp = ctypes.cast(snx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif snx_ is not None:
      _snx_copyarray = True
      _snx_np_tmp = numpy.zeros(len(snx_),numpy.dtype(numpy.float64))
      _snx_np_tmp[:] = snx_
      assert _snx_np_tmp.flags.contiguous
      _snx_tmp = ctypes.cast(_snx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _snx_copyarray = False
      _snx_tmp = None
      
    res = __library__.MSK_XX_getsolution(self.__nativep,whichsol_,ctypes.byref(prosta_),ctypes.byref(solsta_),_skc_tmp,_skx_tmp,_skn_tmp,_xc_tmp,_xx_tmp,_y_tmp,_slc_tmp,_suc_tmp,_slx_tmp,_sux_tmp,_snx_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _prosta_return_value = prosta(prosta_.value)
    _solsta_return_value = solsta(solsta_.value)
    if skc_ is not None: skc_[:] = [ stakey(v) for v in _skc_tmp[0:len(skc_)] ]
    if skx_ is not None: skx_[:] = [ stakey(v) for v in _skx_tmp[0:len(skx_)] ]
    if skn_ is not None: skn_[:] = [ stakey(v) for v in _skn_tmp[0:len(skn_)] ]
    if _xc_copyarray:
      xc_[:] = _xc_np_tmp
    if _xx_copyarray:
      xx_[:] = _xx_np_tmp
    if _y_copyarray:
      y_[:] = _y_np_tmp
    if _slc_copyarray:
      slc_[:] = _slc_np_tmp
    if _suc_copyarray:
      suc_[:] = _suc_np_tmp
    if _slx_copyarray:
      slx_[:] = _slx_np_tmp
    if _sux_copyarray:
      sux_[:] = _sux_np_tmp
    if _snx_copyarray:
      snx_[:] = _snx_np_tmp
    return (_prosta_return_value,_solsta_return_value)
  @accepts(_accept_any,_accept_anyenum(soltype))
  @synchronized
  def getsolsta(self,whichsol_):
    """
    Obtains the solution status.
  
    getsolsta(self,whichsol_)
      whichsol: mosek.soltype. Selects a solution.
    returns: solsta
      solsta: mosek.solsta. Solution status.
    """
    solsta_ = ctypes.c_int32()
    res = __library__.MSK_XX_getsolsta(self.__nativep,whichsol_,ctypes.byref(solsta_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _solsta_return_value = solsta(solsta_.value)
    return (_solsta_return_value)
  @accepts(_accept_any,_accept_anyenum(soltype))
  @synchronized
  def getprosta(self,whichsol_):
    """
    Obtains the problem status.
  
    getprosta(self,whichsol_)
      whichsol: mosek.soltype. Selects a solution.
    returns: prosta
      prosta: mosek.prosta. Problem status.
    """
    prosta_ = ctypes.c_int32()
    res = __library__.MSK_XX_getprosta(self.__nativep,whichsol_,ctypes.byref(prosta_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _prosta_return_value = prosta(prosta_.value)
    return (_prosta_return_value)
  @accepts(_accept_any,_accept_anyenum(soltype),_accept_any)
  @synchronized
  def getskc(self,whichsol_,skc_):
    """
    Obtains the status keys for the constraints.
  
    getskc(self,whichsol_,skc_)
      whichsol: mosek.soltype. Selects a solution.
      skc: array of mosek.stakey. Status keys for the constraints.
    """
    _skc_minlength = self.getnumcon()
    if self.getnumcon() > 0 and skc_ is not None and len(skc_) != self.getnumcon():
      raise ValueError("Array argument skc is not long enough: Is %d, expected %d" % (len(skc_),self.getnumcon()))
    if isinstance(skc_,numpy.ndarray) and not skc_.flags.writeable:
      raise ValueError("Argument skc must be writable")
    if skc_ is not None:
        _skc_tmp = (ctypes.c_int32 * len(skc_))()
    else:
        _skc_tmp = None
    res = __library__.MSK_XX_getskc(self.__nativep,whichsol_,_skc_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if skc_ is not None: skc_[:] = [ stakey(v) for v in _skc_tmp[0:len(skc_)] ]
  @accepts(_accept_any,_accept_anyenum(soltype),_accept_any)
  @synchronized
  def getskx(self,whichsol_,skx_):
    """
    Obtains the status keys for the scalar variables.
  
    getskx(self,whichsol_,skx_)
      whichsol: mosek.soltype. Selects a solution.
      skx: array of mosek.stakey. Status keys for the variables.
    """
    _skx_minlength = self.getnumvar()
    if self.getnumvar() > 0 and skx_ is not None and len(skx_) != self.getnumvar():
      raise ValueError("Array argument skx is not long enough: Is %d, expected %d" % (len(skx_),self.getnumvar()))
    if isinstance(skx_,numpy.ndarray) and not skx_.flags.writeable:
      raise ValueError("Argument skx must be writable")
    if skx_ is not None:
        _skx_tmp = (ctypes.c_int32 * len(skx_))()
    else:
        _skx_tmp = None
    res = __library__.MSK_XX_getskx(self.__nativep,whichsol_,_skx_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if skx_ is not None: skx_[:] = [ stakey(v) for v in _skx_tmp[0:len(skx_)] ]
  @accepts(_accept_any,_accept_anyenum(soltype),_accept_any)
  @synchronized
  def getskn(self,whichsol_,skn_):
    """
    Obtains the status keys for the conic constraints.
  
    getskn(self,whichsol_,skn_)
      whichsol: mosek.soltype. Selects a solution.
      skn: array of mosek.stakey. Status keys for the conic constraints.
    """
    _skn_minlength = self.getnumcone()
    if self.getnumcone() > 0 and skn_ is not None and len(skn_) != self.getnumcone():
      raise ValueError("Array argument skn is not long enough: Is %d, expected %d" % (len(skn_),self.getnumcone()))
    if isinstance(skn_,numpy.ndarray) and not skn_.flags.writeable:
      raise ValueError("Argument skn must be writable")
    if skn_ is not None:
        _skn_tmp = (ctypes.c_int32 * len(skn_))()
    else:
        _skn_tmp = None
    res = __library__.MSK_XX_getskn(self.__nativep,whichsol_,_skn_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if skn_ is not None: skn_[:] = [ stakey(v) for v in _skn_tmp[0:len(skn_)] ]
  @accepts(_accept_any,_accept_anyenum(soltype),_accept_doublevector)
  @synchronized
  def getxc(self,whichsol_,xc_):
    """
    Obtains the xc vector for a solution.
  
    getxc(self,whichsol_,xc_)
      whichsol: mosek.soltype. Selects a solution.
      xc: array of double. Primal constraint solution.
    """
    _xc_minlength = self.getnumcon()
    if self.getnumcon() > 0 and xc_ is not None and len(xc_) != self.getnumcon():
      raise ValueError("Array argument xc is not long enough: Is %d, expected %d" % (len(xc_),self.getnumcon()))
    if isinstance(xc_,numpy.ndarray) and not xc_.flags.writeable:
      raise ValueError("Argument xc must be writable")
    if xc_ is None:
      raise ValueError("Argument xc may not be None")
    if isinstance(xc_, numpy.ndarray) and xc_.dtype is numpy.dtype(numpy.float64) and xc_.flags.contiguous:
      _xc_copyarray = False
      _xc_tmp = ctypes.cast(xc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif xc_ is not None:
      _xc_copyarray = True
      _xc_np_tmp = numpy.zeros(len(xc_),numpy.dtype(numpy.float64))
      _xc_np_tmp[:] = xc_
      assert _xc_np_tmp.flags.contiguous
      _xc_tmp = ctypes.cast(_xc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _xc_copyarray = False
      _xc_tmp = None
      
    res = __library__.MSK_XX_getxc(self.__nativep,whichsol_,_xc_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _xc_copyarray:
      xc_[:] = _xc_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_accept_doublevector)
  @synchronized
  def getxx(self,whichsol_,xx_):
    """
    Obtains the xx vector for a solution.
  
    getxx(self,whichsol_,xx_)
      whichsol: mosek.soltype. Selects a solution.
      xx: array of double. Primal variable solution.
    """
    _xx_minlength = self.getnumvar()
    if self.getnumvar() > 0 and xx_ is not None and len(xx_) != self.getnumvar():
      raise ValueError("Array argument xx is not long enough: Is %d, expected %d" % (len(xx_),self.getnumvar()))
    if isinstance(xx_,numpy.ndarray) and not xx_.flags.writeable:
      raise ValueError("Argument xx must be writable")
    if xx_ is None:
      raise ValueError("Argument xx may not be None")
    if isinstance(xx_, numpy.ndarray) and xx_.dtype is numpy.dtype(numpy.float64) and xx_.flags.contiguous:
      _xx_copyarray = False
      _xx_tmp = ctypes.cast(xx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif xx_ is not None:
      _xx_copyarray = True
      _xx_np_tmp = numpy.zeros(len(xx_),numpy.dtype(numpy.float64))
      _xx_np_tmp[:] = xx_
      assert _xx_np_tmp.flags.contiguous
      _xx_tmp = ctypes.cast(_xx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _xx_copyarray = False
      _xx_tmp = None
      
    res = __library__.MSK_XX_getxx(self.__nativep,whichsol_,_xx_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _xx_copyarray:
      xx_[:] = _xx_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_accept_doublevector)
  @synchronized
  def gety(self,whichsol_,y_):
    """
    Obtains the y vector for a solution.
  
    gety(self,whichsol_,y_)
      whichsol: mosek.soltype. Selects a solution.
      y: array of double. Vector of dual variables corresponding to the constraints.
    """
    _y_minlength = self.getnumcon()
    if self.getnumcon() > 0 and y_ is not None and len(y_) != self.getnumcon():
      raise ValueError("Array argument y is not long enough: Is %d, expected %d" % (len(y_),self.getnumcon()))
    if isinstance(y_,numpy.ndarray) and not y_.flags.writeable:
      raise ValueError("Argument y must be writable")
    if y_ is None:
      raise ValueError("Argument y may not be None")
    if isinstance(y_, numpy.ndarray) and y_.dtype is numpy.dtype(numpy.float64) and y_.flags.contiguous:
      _y_copyarray = False
      _y_tmp = ctypes.cast(y_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif y_ is not None:
      _y_copyarray = True
      _y_np_tmp = numpy.zeros(len(y_),numpy.dtype(numpy.float64))
      _y_np_tmp[:] = y_
      assert _y_np_tmp.flags.contiguous
      _y_tmp = ctypes.cast(_y_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _y_copyarray = False
      _y_tmp = None
      
    res = __library__.MSK_XX_gety(self.__nativep,whichsol_,_y_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _y_copyarray:
      y_[:] = _y_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_accept_doublevector)
  @synchronized
  def getslc(self,whichsol_,slc_):
    """
    Obtains the slc vector for a solution.
  
    getslc(self,whichsol_,slc_)
      whichsol: mosek.soltype. Selects a solution.
      slc: array of double. Dual variables corresponding to the lower bounds on the constraints.
    """
    _slc_minlength = self.getnumcon()
    if self.getnumcon() > 0 and slc_ is not None and len(slc_) != self.getnumcon():
      raise ValueError("Array argument slc is not long enough: Is %d, expected %d" % (len(slc_),self.getnumcon()))
    if isinstance(slc_,numpy.ndarray) and not slc_.flags.writeable:
      raise ValueError("Argument slc must be writable")
    if slc_ is None:
      raise ValueError("Argument slc may not be None")
    if isinstance(slc_, numpy.ndarray) and slc_.dtype is numpy.dtype(numpy.float64) and slc_.flags.contiguous:
      _slc_copyarray = False
      _slc_tmp = ctypes.cast(slc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif slc_ is not None:
      _slc_copyarray = True
      _slc_np_tmp = numpy.zeros(len(slc_),numpy.dtype(numpy.float64))
      _slc_np_tmp[:] = slc_
      assert _slc_np_tmp.flags.contiguous
      _slc_tmp = ctypes.cast(_slc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _slc_copyarray = False
      _slc_tmp = None
      
    res = __library__.MSK_XX_getslc(self.__nativep,whichsol_,_slc_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _slc_copyarray:
      slc_[:] = _slc_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_accept_doublevector)
  @synchronized
  def getsuc(self,whichsol_,suc_):
    """
    Obtains the suc vector for a solution.
  
    getsuc(self,whichsol_,suc_)
      whichsol: mosek.soltype. Selects a solution.
      suc: array of double. Dual variables corresponding to the upper bounds on the constraints.
    """
    _suc_minlength = self.getnumcon()
    if self.getnumcon() > 0 and suc_ is not None and len(suc_) != self.getnumcon():
      raise ValueError("Array argument suc is not long enough: Is %d, expected %d" % (len(suc_),self.getnumcon()))
    if isinstance(suc_,numpy.ndarray) and not suc_.flags.writeable:
      raise ValueError("Argument suc must be writable")
    if suc_ is None:
      raise ValueError("Argument suc may not be None")
    if isinstance(suc_, numpy.ndarray) and suc_.dtype is numpy.dtype(numpy.float64) and suc_.flags.contiguous:
      _suc_copyarray = False
      _suc_tmp = ctypes.cast(suc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif suc_ is not None:
      _suc_copyarray = True
      _suc_np_tmp = numpy.zeros(len(suc_),numpy.dtype(numpy.float64))
      _suc_np_tmp[:] = suc_
      assert _suc_np_tmp.flags.contiguous
      _suc_tmp = ctypes.cast(_suc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _suc_copyarray = False
      _suc_tmp = None
      
    res = __library__.MSK_XX_getsuc(self.__nativep,whichsol_,_suc_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _suc_copyarray:
      suc_[:] = _suc_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_accept_doublevector)
  @synchronized
  def getslx(self,whichsol_,slx_):
    """
    Obtains the slx vector for a solution.
  
    getslx(self,whichsol_,slx_)
      whichsol: mosek.soltype. Selects a solution.
      slx: array of double. Dual variables corresponding to the lower bounds on the variables.
    """
    _slx_minlength = self.getnumvar()
    if self.getnumvar() > 0 and slx_ is not None and len(slx_) != self.getnumvar():
      raise ValueError("Array argument slx is not long enough: Is %d, expected %d" % (len(slx_),self.getnumvar()))
    if isinstance(slx_,numpy.ndarray) and not slx_.flags.writeable:
      raise ValueError("Argument slx must be writable")
    if slx_ is None:
      raise ValueError("Argument slx may not be None")
    if isinstance(slx_, numpy.ndarray) and slx_.dtype is numpy.dtype(numpy.float64) and slx_.flags.contiguous:
      _slx_copyarray = False
      _slx_tmp = ctypes.cast(slx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif slx_ is not None:
      _slx_copyarray = True
      _slx_np_tmp = numpy.zeros(len(slx_),numpy.dtype(numpy.float64))
      _slx_np_tmp[:] = slx_
      assert _slx_np_tmp.flags.contiguous
      _slx_tmp = ctypes.cast(_slx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _slx_copyarray = False
      _slx_tmp = None
      
    res = __library__.MSK_XX_getslx(self.__nativep,whichsol_,_slx_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _slx_copyarray:
      slx_[:] = _slx_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_accept_doublevector)
  @synchronized
  def getsux(self,whichsol_,sux_):
    """
    Obtains the sux vector for a solution.
  
    getsux(self,whichsol_,sux_)
      whichsol: mosek.soltype. Selects a solution.
      sux: array of double. Dual variables corresponding to the upper bounds on the variables.
    """
    _sux_minlength = self.getnumvar()
    if self.getnumvar() > 0 and sux_ is not None and len(sux_) != self.getnumvar():
      raise ValueError("Array argument sux is not long enough: Is %d, expected %d" % (len(sux_),self.getnumvar()))
    if isinstance(sux_,numpy.ndarray) and not sux_.flags.writeable:
      raise ValueError("Argument sux must be writable")
    if sux_ is None:
      raise ValueError("Argument sux may not be None")
    if isinstance(sux_, numpy.ndarray) and sux_.dtype is numpy.dtype(numpy.float64) and sux_.flags.contiguous:
      _sux_copyarray = False
      _sux_tmp = ctypes.cast(sux_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif sux_ is not None:
      _sux_copyarray = True
      _sux_np_tmp = numpy.zeros(len(sux_),numpy.dtype(numpy.float64))
      _sux_np_tmp[:] = sux_
      assert _sux_np_tmp.flags.contiguous
      _sux_tmp = ctypes.cast(_sux_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _sux_copyarray = False
      _sux_tmp = None
      
    res = __library__.MSK_XX_getsux(self.__nativep,whichsol_,_sux_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _sux_copyarray:
      sux_[:] = _sux_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_accept_doublevector)
  @synchronized
  def getsnx(self,whichsol_,snx_):
    """
    Obtains the snx vector for a solution.
  
    getsnx(self,whichsol_,snx_)
      whichsol: mosek.soltype. Selects a solution.
      snx: array of double. Dual variables corresponding to the conic constraints on the variables.
    """
    _snx_minlength = self.getnumvar()
    if self.getnumvar() > 0 and snx_ is not None and len(snx_) != self.getnumvar():
      raise ValueError("Array argument snx is not long enough: Is %d, expected %d" % (len(snx_),self.getnumvar()))
    if isinstance(snx_,numpy.ndarray) and not snx_.flags.writeable:
      raise ValueError("Argument snx must be writable")
    if snx_ is None:
      raise ValueError("Argument snx may not be None")
    if isinstance(snx_, numpy.ndarray) and snx_.dtype is numpy.dtype(numpy.float64) and snx_.flags.contiguous:
      _snx_copyarray = False
      _snx_tmp = ctypes.cast(snx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif snx_ is not None:
      _snx_copyarray = True
      _snx_np_tmp = numpy.zeros(len(snx_),numpy.dtype(numpy.float64))
      _snx_np_tmp[:] = snx_
      assert _snx_np_tmp.flags.contiguous
      _snx_tmp = ctypes.cast(_snx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _snx_copyarray = False
      _snx_tmp = None
      
    res = __library__.MSK_XX_getsnx(self.__nativep,whichsol_,_snx_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _snx_copyarray:
      snx_[:] = _snx_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_int,_accept_any)
  @synchronized
  def getskcslice(self,whichsol_,first_,last_,skc_):
    """
    Obtains the status keys for a slice of the constraints.
  
    getskcslice(self,whichsol_,first_,last_,skc_)
      whichsol: mosek.soltype. Selects a solution.
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      skc: array of mosek.stakey. Status keys for the constraints.
    """
    _skc_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and skc_ is not None and len(skc_) != ((last_) - (first_)):
      raise ValueError("Array argument skc is not long enough: Is %d, expected %d" % (len(skc_),((last_) - (first_))))
    if isinstance(skc_,numpy.ndarray) and not skc_.flags.writeable:
      raise ValueError("Argument skc must be writable")
    if skc_ is not None:
        _skc_tmp = (ctypes.c_int32 * len(skc_))()
    else:
        _skc_tmp = None
    res = __library__.MSK_XX_getskcslice(self.__nativep,whichsol_,first_,last_,_skc_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if skc_ is not None: skc_[:] = [ stakey(v) for v in _skc_tmp[0:len(skc_)] ]
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_int,_accept_any)
  @synchronized
  def getskxslice(self,whichsol_,first_,last_,skx_):
    """
    Obtains the status keys for a slice of the scalar variables.
  
    getskxslice(self,whichsol_,first_,last_,skx_)
      whichsol: mosek.soltype. Selects a solution.
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      skx: array of mosek.stakey. Status keys for the variables.
    """
    _skx_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and skx_ is not None and len(skx_) != ((last_) - (first_)):
      raise ValueError("Array argument skx is not long enough: Is %d, expected %d" % (len(skx_),((last_) - (first_))))
    if isinstance(skx_,numpy.ndarray) and not skx_.flags.writeable:
      raise ValueError("Argument skx must be writable")
    if skx_ is not None:
        _skx_tmp = (ctypes.c_int32 * len(skx_))()
    else:
        _skx_tmp = None
    res = __library__.MSK_XX_getskxslice(self.__nativep,whichsol_,first_,last_,_skx_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if skx_ is not None: skx_[:] = [ stakey(v) for v in _skx_tmp[0:len(skx_)] ]
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_int,_accept_doublevector)
  @synchronized
  def getxcslice(self,whichsol_,first_,last_,xc_):
    """
    Obtains a slice of the xc vector for a solution.
  
    getxcslice(self,whichsol_,first_,last_,xc_)
      whichsol: mosek.soltype. Selects a solution.
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      xc: array of double. Primal constraint solution.
    """
    _xc_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and xc_ is not None and len(xc_) != ((last_) - (first_)):
      raise ValueError("Array argument xc is not long enough: Is %d, expected %d" % (len(xc_),((last_) - (first_))))
    if isinstance(xc_,numpy.ndarray) and not xc_.flags.writeable:
      raise ValueError("Argument xc must be writable")
    if isinstance(xc_, numpy.ndarray) and xc_.dtype is numpy.dtype(numpy.float64) and xc_.flags.contiguous:
      _xc_copyarray = False
      _xc_tmp = ctypes.cast(xc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif xc_ is not None:
      _xc_copyarray = True
      _xc_np_tmp = numpy.zeros(len(xc_),numpy.dtype(numpy.float64))
      _xc_np_tmp[:] = xc_
      assert _xc_np_tmp.flags.contiguous
      _xc_tmp = ctypes.cast(_xc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _xc_copyarray = False
      _xc_tmp = None
      
    res = __library__.MSK_XX_getxcslice(self.__nativep,whichsol_,first_,last_,_xc_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _xc_copyarray:
      xc_[:] = _xc_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_int,_accept_doublevector)
  @synchronized
  def getxxslice(self,whichsol_,first_,last_,xx_):
    """
    Obtains a slice of the xx vector for a solution.
  
    getxxslice(self,whichsol_,first_,last_,xx_)
      whichsol: mosek.soltype. Selects a solution.
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      xx: array of double. Primal variable solution.
    """
    _xx_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and xx_ is not None and len(xx_) != ((last_) - (first_)):
      raise ValueError("Array argument xx is not long enough: Is %d, expected %d" % (len(xx_),((last_) - (first_))))
    if isinstance(xx_,numpy.ndarray) and not xx_.flags.writeable:
      raise ValueError("Argument xx must be writable")
    if isinstance(xx_, numpy.ndarray) and xx_.dtype is numpy.dtype(numpy.float64) and xx_.flags.contiguous:
      _xx_copyarray = False
      _xx_tmp = ctypes.cast(xx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif xx_ is not None:
      _xx_copyarray = True
      _xx_np_tmp = numpy.zeros(len(xx_),numpy.dtype(numpy.float64))
      _xx_np_tmp[:] = xx_
      assert _xx_np_tmp.flags.contiguous
      _xx_tmp = ctypes.cast(_xx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _xx_copyarray = False
      _xx_tmp = None
      
    res = __library__.MSK_XX_getxxslice(self.__nativep,whichsol_,first_,last_,_xx_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _xx_copyarray:
      xx_[:] = _xx_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_int,_accept_doublevector)
  @synchronized
  def getyslice(self,whichsol_,first_,last_,y_):
    """
    Obtains a slice of the y vector for a solution.
  
    getyslice(self,whichsol_,first_,last_,y_)
      whichsol: mosek.soltype. Selects a solution.
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      y: array of double. Vector of dual variables corresponding to the constraints.
    """
    _y_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and y_ is not None and len(y_) != ((last_) - (first_)):
      raise ValueError("Array argument y is not long enough: Is %d, expected %d" % (len(y_),((last_) - (first_))))
    if isinstance(y_,numpy.ndarray) and not y_.flags.writeable:
      raise ValueError("Argument y must be writable")
    if isinstance(y_, numpy.ndarray) and y_.dtype is numpy.dtype(numpy.float64) and y_.flags.contiguous:
      _y_copyarray = False
      _y_tmp = ctypes.cast(y_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif y_ is not None:
      _y_copyarray = True
      _y_np_tmp = numpy.zeros(len(y_),numpy.dtype(numpy.float64))
      _y_np_tmp[:] = y_
      assert _y_np_tmp.flags.contiguous
      _y_tmp = ctypes.cast(_y_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _y_copyarray = False
      _y_tmp = None
      
    res = __library__.MSK_XX_getyslice(self.__nativep,whichsol_,first_,last_,_y_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _y_copyarray:
      y_[:] = _y_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_int,_accept_doublevector)
  @synchronized
  def getslcslice(self,whichsol_,first_,last_,slc_):
    """
    Obtains a slice of the slc vector for a solution.
  
    getslcslice(self,whichsol_,first_,last_,slc_)
      whichsol: mosek.soltype. Selects a solution.
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      slc: array of double. Dual variables corresponding to the lower bounds on the constraints.
    """
    _slc_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and slc_ is not None and len(slc_) != ((last_) - (first_)):
      raise ValueError("Array argument slc is not long enough: Is %d, expected %d" % (len(slc_),((last_) - (first_))))
    if isinstance(slc_,numpy.ndarray) and not slc_.flags.writeable:
      raise ValueError("Argument slc must be writable")
    if isinstance(slc_, numpy.ndarray) and slc_.dtype is numpy.dtype(numpy.float64) and slc_.flags.contiguous:
      _slc_copyarray = False
      _slc_tmp = ctypes.cast(slc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif slc_ is not None:
      _slc_copyarray = True
      _slc_np_tmp = numpy.zeros(len(slc_),numpy.dtype(numpy.float64))
      _slc_np_tmp[:] = slc_
      assert _slc_np_tmp.flags.contiguous
      _slc_tmp = ctypes.cast(_slc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _slc_copyarray = False
      _slc_tmp = None
      
    res = __library__.MSK_XX_getslcslice(self.__nativep,whichsol_,first_,last_,_slc_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _slc_copyarray:
      slc_[:] = _slc_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_int,_accept_doublevector)
  @synchronized
  def getsucslice(self,whichsol_,first_,last_,suc_):
    """
    Obtains a slice of the suc vector for a solution.
  
    getsucslice(self,whichsol_,first_,last_,suc_)
      whichsol: mosek.soltype. Selects a solution.
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      suc: array of double. Dual variables corresponding to the upper bounds on the constraints.
    """
    _suc_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and suc_ is not None and len(suc_) != ((last_) - (first_)):
      raise ValueError("Array argument suc is not long enough: Is %d, expected %d" % (len(suc_),((last_) - (first_))))
    if isinstance(suc_,numpy.ndarray) and not suc_.flags.writeable:
      raise ValueError("Argument suc must be writable")
    if isinstance(suc_, numpy.ndarray) and suc_.dtype is numpy.dtype(numpy.float64) and suc_.flags.contiguous:
      _suc_copyarray = False
      _suc_tmp = ctypes.cast(suc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif suc_ is not None:
      _suc_copyarray = True
      _suc_np_tmp = numpy.zeros(len(suc_),numpy.dtype(numpy.float64))
      _suc_np_tmp[:] = suc_
      assert _suc_np_tmp.flags.contiguous
      _suc_tmp = ctypes.cast(_suc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _suc_copyarray = False
      _suc_tmp = None
      
    res = __library__.MSK_XX_getsucslice(self.__nativep,whichsol_,first_,last_,_suc_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _suc_copyarray:
      suc_[:] = _suc_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_int,_accept_doublevector)
  @synchronized
  def getslxslice(self,whichsol_,first_,last_,slx_):
    """
    Obtains a slice of the slx vector for a solution.
  
    getslxslice(self,whichsol_,first_,last_,slx_)
      whichsol: mosek.soltype. Selects a solution.
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      slx: array of double. Dual variables corresponding to the lower bounds on the variables.
    """
    _slx_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and slx_ is not None and len(slx_) != ((last_) - (first_)):
      raise ValueError("Array argument slx is not long enough: Is %d, expected %d" % (len(slx_),((last_) - (first_))))
    if isinstance(slx_,numpy.ndarray) and not slx_.flags.writeable:
      raise ValueError("Argument slx must be writable")
    if isinstance(slx_, numpy.ndarray) and slx_.dtype is numpy.dtype(numpy.float64) and slx_.flags.contiguous:
      _slx_copyarray = False
      _slx_tmp = ctypes.cast(slx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif slx_ is not None:
      _slx_copyarray = True
      _slx_np_tmp = numpy.zeros(len(slx_),numpy.dtype(numpy.float64))
      _slx_np_tmp[:] = slx_
      assert _slx_np_tmp.flags.contiguous
      _slx_tmp = ctypes.cast(_slx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _slx_copyarray = False
      _slx_tmp = None
      
    res = __library__.MSK_XX_getslxslice(self.__nativep,whichsol_,first_,last_,_slx_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _slx_copyarray:
      slx_[:] = _slx_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_int,_accept_doublevector)
  @synchronized
  def getsuxslice(self,whichsol_,first_,last_,sux_):
    """
    Obtains a slice of the sux vector for a solution.
  
    getsuxslice(self,whichsol_,first_,last_,sux_)
      whichsol: mosek.soltype. Selects a solution.
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      sux: array of double. Dual variables corresponding to the upper bounds on the variables.
    """
    _sux_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and sux_ is not None and len(sux_) != ((last_) - (first_)):
      raise ValueError("Array argument sux is not long enough: Is %d, expected %d" % (len(sux_),((last_) - (first_))))
    if isinstance(sux_,numpy.ndarray) and not sux_.flags.writeable:
      raise ValueError("Argument sux must be writable")
    if isinstance(sux_, numpy.ndarray) and sux_.dtype is numpy.dtype(numpy.float64) and sux_.flags.contiguous:
      _sux_copyarray = False
      _sux_tmp = ctypes.cast(sux_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif sux_ is not None:
      _sux_copyarray = True
      _sux_np_tmp = numpy.zeros(len(sux_),numpy.dtype(numpy.float64))
      _sux_np_tmp[:] = sux_
      assert _sux_np_tmp.flags.contiguous
      _sux_tmp = ctypes.cast(_sux_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _sux_copyarray = False
      _sux_tmp = None
      
    res = __library__.MSK_XX_getsuxslice(self.__nativep,whichsol_,first_,last_,_sux_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _sux_copyarray:
      sux_[:] = _sux_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_int,_accept_doublevector)
  @synchronized
  def getsnxslice(self,whichsol_,first_,last_,snx_):
    """
    Obtains a slice of the snx vector for a solution.
  
    getsnxslice(self,whichsol_,first_,last_,snx_)
      whichsol: mosek.soltype. Selects a solution.
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      snx: array of double. Dual variables corresponding to the conic constraints on the variables.
    """
    _snx_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and snx_ is not None and len(snx_) != ((last_) - (first_)):
      raise ValueError("Array argument snx is not long enough: Is %d, expected %d" % (len(snx_),((last_) - (first_))))
    if isinstance(snx_,numpy.ndarray) and not snx_.flags.writeable:
      raise ValueError("Argument snx must be writable")
    if isinstance(snx_, numpy.ndarray) and snx_.dtype is numpy.dtype(numpy.float64) and snx_.flags.contiguous:
      _snx_copyarray = False
      _snx_tmp = ctypes.cast(snx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif snx_ is not None:
      _snx_copyarray = True
      _snx_np_tmp = numpy.zeros(len(snx_),numpy.dtype(numpy.float64))
      _snx_np_tmp[:] = snx_
      assert _snx_np_tmp.flags.contiguous
      _snx_tmp = ctypes.cast(_snx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _snx_copyarray = False
      _snx_tmp = None
      
    res = __library__.MSK_XX_getsnxslice(self.__nativep,whichsol_,first_,last_,_snx_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _snx_copyarray:
      snx_[:] = _snx_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_accept_doublevector)
  @synchronized
  def getbarxj(self,whichsol_,j_,barxj_):
    """
    Obtains the primal solution for a semidefinite variable.
  
    getbarxj(self,whichsol_,j_,barxj_)
      whichsol: mosek.soltype. Selects a solution.
      j: int. Index of the semidefinite variable.
      barxj: array of double. Value of the j'th variable of barx.
    """
    _barxj_minlength = self.getlenbarvarj((j_))
    if self.getlenbarvarj((j_)) > 0 and barxj_ is not None and len(barxj_) != self.getlenbarvarj((j_)):
      raise ValueError("Array argument barxj is not long enough: Is %d, expected %d" % (len(barxj_),self.getlenbarvarj((j_))))
    if isinstance(barxj_,numpy.ndarray) and not barxj_.flags.writeable:
      raise ValueError("Argument barxj must be writable")
    if barxj_ is None:
      raise ValueError("Argument barxj may not be None")
    if isinstance(barxj_, numpy.ndarray) and barxj_.dtype is numpy.dtype(numpy.float64) and barxj_.flags.contiguous:
      _barxj_copyarray = False
      _barxj_tmp = ctypes.cast(barxj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif barxj_ is not None:
      _barxj_copyarray = True
      _barxj_np_tmp = numpy.zeros(len(barxj_),numpy.dtype(numpy.float64))
      _barxj_np_tmp[:] = barxj_
      assert _barxj_np_tmp.flags.contiguous
      _barxj_tmp = ctypes.cast(_barxj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _barxj_copyarray = False
      _barxj_tmp = None
      
    res = __library__.MSK_XX_getbarxj(self.__nativep,whichsol_,j_,_barxj_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _barxj_copyarray:
      barxj_[:] = _barxj_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_int,_make_long,_accept_doublevector)
  @synchronized
  def getbarxslice(self,whichsol_,first_,last_,slicesize_,barxslice_):
    """
    Obtains the primal solution for a sequence of semidefinite variables.
  
    getbarxslice(self,whichsol_,first_,last_,slicesize_,barxslice_)
      whichsol: mosek.soltype. Selects a solution.
      first: int. Index of the first semidefinite variable in the slice.
      last: int. Index of the last semidefinite variable in the slice plus one.
      slicesize: long. Denotes the length of the array barxslice.
      barxslice: array of double. Solution values of symmetric matrix variables in the slice, stored sequentially.
    """
    _barxslice_minlength = (slicesize_)
    if (slicesize_) > 0 and barxslice_ is not None and len(barxslice_) != (slicesize_):
      raise ValueError("Array argument barxslice is not long enough: Is %d, expected %d" % (len(barxslice_),(slicesize_)))
    if isinstance(barxslice_,numpy.ndarray) and not barxslice_.flags.writeable:
      raise ValueError("Argument barxslice must be writable")
    if barxslice_ is None:
      raise ValueError("Argument barxslice may not be None")
    if isinstance(barxslice_, numpy.ndarray) and barxslice_.dtype is numpy.dtype(numpy.float64) and barxslice_.flags.contiguous:
      _barxslice_copyarray = False
      _barxslice_tmp = ctypes.cast(barxslice_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif barxslice_ is not None:
      _barxslice_copyarray = True
      _barxslice_np_tmp = numpy.zeros(len(barxslice_),numpy.dtype(numpy.float64))
      _barxslice_np_tmp[:] = barxslice_
      assert _barxslice_np_tmp.flags.contiguous
      _barxslice_tmp = ctypes.cast(_barxslice_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _barxslice_copyarray = False
      _barxslice_tmp = None
      
    res = __library__.MSK_XX_getbarxslice(self.__nativep,whichsol_,first_,last_,slicesize_,_barxslice_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _barxslice_copyarray:
      barxslice_[:] = _barxslice_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_accept_doublevector)
  @synchronized
  def getbarsj(self,whichsol_,j_,barsj_):
    """
    Obtains the dual solution for a semidefinite variable.
  
    getbarsj(self,whichsol_,j_,barsj_)
      whichsol: mosek.soltype. Selects a solution.
      j: int. Index of the semidefinite variable.
      barsj: array of double. Value of the j'th dual variable of barx.
    """
    _barsj_minlength = self.getlenbarvarj((j_))
    if self.getlenbarvarj((j_)) > 0 and barsj_ is not None and len(barsj_) != self.getlenbarvarj((j_)):
      raise ValueError("Array argument barsj is not long enough: Is %d, expected %d" % (len(barsj_),self.getlenbarvarj((j_))))
    if isinstance(barsj_,numpy.ndarray) and not barsj_.flags.writeable:
      raise ValueError("Argument barsj must be writable")
    if barsj_ is None:
      raise ValueError("Argument barsj may not be None")
    if isinstance(barsj_, numpy.ndarray) and barsj_.dtype is numpy.dtype(numpy.float64) and barsj_.flags.contiguous:
      _barsj_copyarray = False
      _barsj_tmp = ctypes.cast(barsj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif barsj_ is not None:
      _barsj_copyarray = True
      _barsj_np_tmp = numpy.zeros(len(barsj_),numpy.dtype(numpy.float64))
      _barsj_np_tmp[:] = barsj_
      assert _barsj_np_tmp.flags.contiguous
      _barsj_tmp = ctypes.cast(_barsj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _barsj_copyarray = False
      _barsj_tmp = None
      
    res = __library__.MSK_XX_getbarsj(self.__nativep,whichsol_,j_,_barsj_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _barsj_copyarray:
      barsj_[:] = _barsj_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_int,_make_long,_accept_doublevector)
  @synchronized
  def getbarsslice(self,whichsol_,first_,last_,slicesize_,barsslice_):
    """
    Obtains the dual solution for a sequence of semidefinite variables.
  
    getbarsslice(self,whichsol_,first_,last_,slicesize_,barsslice_)
      whichsol: mosek.soltype. Selects a solution.
      first: int. Index of the first semidefinite variable in the slice.
      last: int. Index of the last semidefinite variable in the slice plus one.
      slicesize: long. Denotes the length of the array barsslice.
      barsslice: array of double. Dual solution values of symmetric matrix variables in the slice, stored sequentially.
    """
    _barsslice_minlength = (slicesize_)
    if (slicesize_) > 0 and barsslice_ is not None and len(barsslice_) != (slicesize_):
      raise ValueError("Array argument barsslice is not long enough: Is %d, expected %d" % (len(barsslice_),(slicesize_)))
    if isinstance(barsslice_,numpy.ndarray) and not barsslice_.flags.writeable:
      raise ValueError("Argument barsslice must be writable")
    if barsslice_ is None:
      raise ValueError("Argument barsslice may not be None")
    if isinstance(barsslice_, numpy.ndarray) and barsslice_.dtype is numpy.dtype(numpy.float64) and barsslice_.flags.contiguous:
      _barsslice_copyarray = False
      _barsslice_tmp = ctypes.cast(barsslice_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif barsslice_ is not None:
      _barsslice_copyarray = True
      _barsslice_np_tmp = numpy.zeros(len(barsslice_),numpy.dtype(numpy.float64))
      _barsslice_np_tmp[:] = barsslice_
      assert _barsslice_np_tmp.flags.contiguous
      _barsslice_tmp = ctypes.cast(_barsslice_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _barsslice_copyarray = False
      _barsslice_tmp = None
      
    res = __library__.MSK_XX_getbarsslice(self.__nativep,whichsol_,first_,last_,slicesize_,_barsslice_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _barsslice_copyarray:
      barsslice_[:] = _barsslice_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_make_anyenumvector(stakey))
  @synchronized
  def putskc(self,whichsol_,skc_):
    """
    Sets the status keys for the constraints.
  
    putskc(self,whichsol_,skc_)
      whichsol: mosek.soltype. Selects a solution.
      skc: array of mosek.stakey. Status keys for the constraints.
    """
    _skc_minlength = self.getnumcon()
    if self.getnumcon() > 0 and skc_ is not None and len(skc_) != self.getnumcon():
      raise ValueError("Array argument skc is not long enough: Is %d, expected %d" % (len(skc_),self.getnumcon()))
    if skc_ is None:
      raise ValueError("Argument skc cannot be None")
    if skc_ is None:
      raise ValueError("Argument skc may not be None")
    if skc_ is not None:
        _skc_tmp = (ctypes.c_int32 * len(skc_))(*skc_)
    else:
        _skc_tmp = None
    res = __library__.MSK_XX_putskc(self.__nativep,whichsol_,_skc_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype),_make_anyenumvector(stakey))
  @synchronized
  def putskx(self,whichsol_,skx_):
    """
    Sets the status keys for the scalar variables.
  
    putskx(self,whichsol_,skx_)
      whichsol: mosek.soltype. Selects a solution.
      skx: array of mosek.stakey. Status keys for the variables.
    """
    _skx_minlength = self.getnumvar()
    if self.getnumvar() > 0 and skx_ is not None and len(skx_) != self.getnumvar():
      raise ValueError("Array argument skx is not long enough: Is %d, expected %d" % (len(skx_),self.getnumvar()))
    if skx_ is None:
      raise ValueError("Argument skx cannot be None")
    if skx_ is None:
      raise ValueError("Argument skx may not be None")
    if skx_ is not None:
        _skx_tmp = (ctypes.c_int32 * len(skx_))(*skx_)
    else:
        _skx_tmp = None
    res = __library__.MSK_XX_putskx(self.__nativep,whichsol_,_skx_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype),_accept_doublevector)
  @synchronized
  def putxc(self,whichsol_,xc_):
    """
    Sets the xc vector for a solution.
  
    putxc(self,whichsol_,xc_)
      whichsol: mosek.soltype. Selects a solution.
      xc: array of double. Primal constraint solution.
    """
    _xc_minlength = self.getnumcon()
    if self.getnumcon() > 0 and xc_ is not None and len(xc_) != self.getnumcon():
      raise ValueError("Array argument xc is not long enough: Is %d, expected %d" % (len(xc_),self.getnumcon()))
    if isinstance(xc_,numpy.ndarray) and not xc_.flags.writeable:
      raise ValueError("Argument xc must be writable")
    if xc_ is None:
      raise ValueError("Argument xc may not be None")
    if isinstance(xc_, numpy.ndarray) and xc_.dtype is numpy.dtype(numpy.float64) and xc_.flags.contiguous:
      _xc_copyarray = False
      _xc_tmp = ctypes.cast(xc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif xc_ is not None:
      _xc_copyarray = True
      _xc_np_tmp = numpy.zeros(len(xc_),numpy.dtype(numpy.float64))
      _xc_np_tmp[:] = xc_
      assert _xc_np_tmp.flags.contiguous
      _xc_tmp = ctypes.cast(_xc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _xc_copyarray = False
      _xc_tmp = None
      
    res = __library__.MSK_XX_putxc(self.__nativep,whichsol_,_xc_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _xc_copyarray:
      xc_[:] = _xc_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_make_doublevector)
  @synchronized
  def putxx(self,whichsol_,xx_):
    """
    Sets the xx vector for a solution.
  
    putxx(self,whichsol_,xx_)
      whichsol: mosek.soltype. Selects a solution.
      xx: array of double. Primal variable solution.
    """
    _xx_minlength = self.getnumvar()
    if self.getnumvar() > 0 and xx_ is not None and len(xx_) != self.getnumvar():
      raise ValueError("Array argument xx is not long enough: Is %d, expected %d" % (len(xx_),self.getnumvar()))
    if xx_ is None:
      raise ValueError("Argument xx cannot be None")
    if xx_ is None:
      raise ValueError("Argument xx may not be None")
    if isinstance(xx_, numpy.ndarray) and xx_.dtype is numpy.dtype(numpy.float64) and xx_.flags.contiguous:
      _xx_copyarray = False
      _xx_tmp = ctypes.cast(xx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif xx_ is not None:
      _xx_copyarray = True
      _xx_np_tmp = numpy.zeros(len(xx_),numpy.dtype(numpy.float64))
      _xx_np_tmp[:] = xx_
      assert _xx_np_tmp.flags.contiguous
      _xx_tmp = ctypes.cast(_xx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _xx_copyarray = False
      _xx_tmp = None
      
    res = __library__.MSK_XX_putxx(self.__nativep,whichsol_,_xx_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype),_make_doublevector)
  @synchronized
  def puty(self,whichsol_,y_):
    """
    Sets the y vector for a solution.
  
    puty(self,whichsol_,y_)
      whichsol: mosek.soltype. Selects a solution.
      y: array of double. Vector of dual variables corresponding to the constraints.
    """
    _y_minlength = self.getnumcon()
    if self.getnumcon() > 0 and y_ is not None and len(y_) != self.getnumcon():
      raise ValueError("Array argument y is not long enough: Is %d, expected %d" % (len(y_),self.getnumcon()))
    if y_ is None:
      raise ValueError("Argument y cannot be None")
    if y_ is None:
      raise ValueError("Argument y may not be None")
    if isinstance(y_, numpy.ndarray) and y_.dtype is numpy.dtype(numpy.float64) and y_.flags.contiguous:
      _y_copyarray = False
      _y_tmp = ctypes.cast(y_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif y_ is not None:
      _y_copyarray = True
      _y_np_tmp = numpy.zeros(len(y_),numpy.dtype(numpy.float64))
      _y_np_tmp[:] = y_
      assert _y_np_tmp.flags.contiguous
      _y_tmp = ctypes.cast(_y_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _y_copyarray = False
      _y_tmp = None
      
    res = __library__.MSK_XX_puty(self.__nativep,whichsol_,_y_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype),_make_doublevector)
  @synchronized
  def putslc(self,whichsol_,slc_):
    """
    Sets the slc vector for a solution.
  
    putslc(self,whichsol_,slc_)
      whichsol: mosek.soltype. Selects a solution.
      slc: array of double. Dual variables corresponding to the lower bounds on the constraints.
    """
    _slc_minlength = self.getnumcon()
    if self.getnumcon() > 0 and slc_ is not None and len(slc_) != self.getnumcon():
      raise ValueError("Array argument slc is not long enough: Is %d, expected %d" % (len(slc_),self.getnumcon()))
    if slc_ is None:
      raise ValueError("Argument slc cannot be None")
    if slc_ is None:
      raise ValueError("Argument slc may not be None")
    if isinstance(slc_, numpy.ndarray) and slc_.dtype is numpy.dtype(numpy.float64) and slc_.flags.contiguous:
      _slc_copyarray = False
      _slc_tmp = ctypes.cast(slc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif slc_ is not None:
      _slc_copyarray = True
      _slc_np_tmp = numpy.zeros(len(slc_),numpy.dtype(numpy.float64))
      _slc_np_tmp[:] = slc_
      assert _slc_np_tmp.flags.contiguous
      _slc_tmp = ctypes.cast(_slc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _slc_copyarray = False
      _slc_tmp = None
      
    res = __library__.MSK_XX_putslc(self.__nativep,whichsol_,_slc_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype),_make_doublevector)
  @synchronized
  def putsuc(self,whichsol_,suc_):
    """
    Sets the suc vector for a solution.
  
    putsuc(self,whichsol_,suc_)
      whichsol: mosek.soltype. Selects a solution.
      suc: array of double. Dual variables corresponding to the upper bounds on the constraints.
    """
    _suc_minlength = self.getnumcon()
    if self.getnumcon() > 0 and suc_ is not None and len(suc_) != self.getnumcon():
      raise ValueError("Array argument suc is not long enough: Is %d, expected %d" % (len(suc_),self.getnumcon()))
    if suc_ is None:
      raise ValueError("Argument suc cannot be None")
    if suc_ is None:
      raise ValueError("Argument suc may not be None")
    if isinstance(suc_, numpy.ndarray) and suc_.dtype is numpy.dtype(numpy.float64) and suc_.flags.contiguous:
      _suc_copyarray = False
      _suc_tmp = ctypes.cast(suc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif suc_ is not None:
      _suc_copyarray = True
      _suc_np_tmp = numpy.zeros(len(suc_),numpy.dtype(numpy.float64))
      _suc_np_tmp[:] = suc_
      assert _suc_np_tmp.flags.contiguous
      _suc_tmp = ctypes.cast(_suc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _suc_copyarray = False
      _suc_tmp = None
      
    res = __library__.MSK_XX_putsuc(self.__nativep,whichsol_,_suc_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype),_make_doublevector)
  @synchronized
  def putslx(self,whichsol_,slx_):
    """
    Sets the slx vector for a solution.
  
    putslx(self,whichsol_,slx_)
      whichsol: mosek.soltype. Selects a solution.
      slx: array of double. Dual variables corresponding to the lower bounds on the variables.
    """
    _slx_minlength = self.getnumvar()
    if self.getnumvar() > 0 and slx_ is not None and len(slx_) != self.getnumvar():
      raise ValueError("Array argument slx is not long enough: Is %d, expected %d" % (len(slx_),self.getnumvar()))
    if slx_ is None:
      raise ValueError("Argument slx cannot be None")
    if slx_ is None:
      raise ValueError("Argument slx may not be None")
    if isinstance(slx_, numpy.ndarray) and slx_.dtype is numpy.dtype(numpy.float64) and slx_.flags.contiguous:
      _slx_copyarray = False
      _slx_tmp = ctypes.cast(slx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif slx_ is not None:
      _slx_copyarray = True
      _slx_np_tmp = numpy.zeros(len(slx_),numpy.dtype(numpy.float64))
      _slx_np_tmp[:] = slx_
      assert _slx_np_tmp.flags.contiguous
      _slx_tmp = ctypes.cast(_slx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _slx_copyarray = False
      _slx_tmp = None
      
    res = __library__.MSK_XX_putslx(self.__nativep,whichsol_,_slx_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype),_make_doublevector)
  @synchronized
  def putsux(self,whichsol_,sux_):
    """
    Sets the sux vector for a solution.
  
    putsux(self,whichsol_,sux_)
      whichsol: mosek.soltype. Selects a solution.
      sux: array of double. Dual variables corresponding to the upper bounds on the variables.
    """
    _sux_minlength = self.getnumvar()
    if self.getnumvar() > 0 and sux_ is not None and len(sux_) != self.getnumvar():
      raise ValueError("Array argument sux is not long enough: Is %d, expected %d" % (len(sux_),self.getnumvar()))
    if sux_ is None:
      raise ValueError("Argument sux cannot be None")
    if sux_ is None:
      raise ValueError("Argument sux may not be None")
    if isinstance(sux_, numpy.ndarray) and sux_.dtype is numpy.dtype(numpy.float64) and sux_.flags.contiguous:
      _sux_copyarray = False
      _sux_tmp = ctypes.cast(sux_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif sux_ is not None:
      _sux_copyarray = True
      _sux_np_tmp = numpy.zeros(len(sux_),numpy.dtype(numpy.float64))
      _sux_np_tmp[:] = sux_
      assert _sux_np_tmp.flags.contiguous
      _sux_tmp = ctypes.cast(_sux_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _sux_copyarray = False
      _sux_tmp = None
      
    res = __library__.MSK_XX_putsux(self.__nativep,whichsol_,_sux_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype),_make_doublevector)
  @synchronized
  def putsnx(self,whichsol_,sux_):
    """
    Sets the snx vector for a solution.
  
    putsnx(self,whichsol_,sux_)
      whichsol: mosek.soltype. Selects a solution.
      sux: array of double. Dual variables corresponding to the upper bounds on the variables.
    """
    _sux_minlength = self.getnumvar()
    if self.getnumvar() > 0 and sux_ is not None and len(sux_) != self.getnumvar():
      raise ValueError("Array argument sux is not long enough: Is %d, expected %d" % (len(sux_),self.getnumvar()))
    if sux_ is None:
      raise ValueError("Argument sux cannot be None")
    if sux_ is None:
      raise ValueError("Argument sux may not be None")
    if isinstance(sux_, numpy.ndarray) and sux_.dtype is numpy.dtype(numpy.float64) and sux_.flags.contiguous:
      _sux_copyarray = False
      _sux_tmp = ctypes.cast(sux_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif sux_ is not None:
      _sux_copyarray = True
      _sux_np_tmp = numpy.zeros(len(sux_),numpy.dtype(numpy.float64))
      _sux_np_tmp[:] = sux_
      assert _sux_np_tmp.flags.contiguous
      _sux_tmp = ctypes.cast(_sux_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _sux_copyarray = False
      _sux_tmp = None
      
    res = __library__.MSK_XX_putsnx(self.__nativep,whichsol_,_sux_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_int,_make_anyenumvector(stakey))
  @synchronized
  def putskcslice(self,whichsol_,first_,last_,skc_):
    """
    Sets the status keys for a slice of the constraints.
  
    putskcslice(self,whichsol_,first_,last_,skc_)
      whichsol: mosek.soltype. Selects a solution.
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      skc: array of mosek.stakey. Status keys for the constraints.
    """
    _skc_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and skc_ is not None and len(skc_) != ((last_) - (first_)):
      raise ValueError("Array argument skc is not long enough: Is %d, expected %d" % (len(skc_),((last_) - (first_))))
    if skc_ is not None:
        _skc_tmp = (ctypes.c_int32 * len(skc_))(*skc_)
    else:
        _skc_tmp = None
    res = __library__.MSK_XX_putskcslice(self.__nativep,whichsol_,first_,last_,_skc_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_int,_make_anyenumvector(stakey))
  @synchronized
  def putskxslice(self,whichsol_,first_,last_,skx_):
    """
    Sets the status keys for a slice of the variables.
  
    putskxslice(self,whichsol_,first_,last_,skx_)
      whichsol: mosek.soltype. Selects a solution.
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      skx: array of mosek.stakey. Status keys for the variables.
    """
    _skx_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and skx_ is not None and len(skx_) != ((last_) - (first_)):
      raise ValueError("Array argument skx is not long enough: Is %d, expected %d" % (len(skx_),((last_) - (first_))))
    if skx_ is None:
      raise ValueError("Argument skx cannot be None")
    if skx_ is None:
      raise ValueError("Argument skx may not be None")
    if skx_ is not None:
        _skx_tmp = (ctypes.c_int32 * len(skx_))(*skx_)
    else:
        _skx_tmp = None
    res = __library__.MSK_XX_putskxslice(self.__nativep,whichsol_,first_,last_,_skx_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_int,_make_doublevector)
  @synchronized
  def putxcslice(self,whichsol_,first_,last_,xc_):
    """
    Sets a slice of the xc vector for a solution.
  
    putxcslice(self,whichsol_,first_,last_,xc_)
      whichsol: mosek.soltype. Selects a solution.
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      xc: array of double. Primal constraint solution.
    """
    _xc_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and xc_ is not None and len(xc_) != ((last_) - (first_)):
      raise ValueError("Array argument xc is not long enough: Is %d, expected %d" % (len(xc_),((last_) - (first_))))
    if xc_ is None:
      raise ValueError("Argument xc cannot be None")
    if xc_ is None:
      raise ValueError("Argument xc may not be None")
    if isinstance(xc_, numpy.ndarray) and xc_.dtype is numpy.dtype(numpy.float64) and xc_.flags.contiguous:
      _xc_copyarray = False
      _xc_tmp = ctypes.cast(xc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif xc_ is not None:
      _xc_copyarray = True
      _xc_np_tmp = numpy.zeros(len(xc_),numpy.dtype(numpy.float64))
      _xc_np_tmp[:] = xc_
      assert _xc_np_tmp.flags.contiguous
      _xc_tmp = ctypes.cast(_xc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _xc_copyarray = False
      _xc_tmp = None
      
    res = __library__.MSK_XX_putxcslice(self.__nativep,whichsol_,first_,last_,_xc_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_int,_make_doublevector)
  @synchronized
  def putxxslice(self,whichsol_,first_,last_,xx_):
    """
    Sets a slice of the xx vector for a solution.
  
    putxxslice(self,whichsol_,first_,last_,xx_)
      whichsol: mosek.soltype. Selects a solution.
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      xx: array of double. Primal variable solution.
    """
    _xx_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and xx_ is not None and len(xx_) != ((last_) - (first_)):
      raise ValueError("Array argument xx is not long enough: Is %d, expected %d" % (len(xx_),((last_) - (first_))))
    if xx_ is None:
      raise ValueError("Argument xx cannot be None")
    if xx_ is None:
      raise ValueError("Argument xx may not be None")
    if isinstance(xx_, numpy.ndarray) and xx_.dtype is numpy.dtype(numpy.float64) and xx_.flags.contiguous:
      _xx_copyarray = False
      _xx_tmp = ctypes.cast(xx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif xx_ is not None:
      _xx_copyarray = True
      _xx_np_tmp = numpy.zeros(len(xx_),numpy.dtype(numpy.float64))
      _xx_np_tmp[:] = xx_
      assert _xx_np_tmp.flags.contiguous
      _xx_tmp = ctypes.cast(_xx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _xx_copyarray = False
      _xx_tmp = None
      
    res = __library__.MSK_XX_putxxslice(self.__nativep,whichsol_,first_,last_,_xx_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_int,_make_doublevector)
  @synchronized
  def putyslice(self,whichsol_,first_,last_,y_):
    """
    Sets a slice of the y vector for a solution.
  
    putyslice(self,whichsol_,first_,last_,y_)
      whichsol: mosek.soltype. Selects a solution.
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      y: array of double. Vector of dual variables corresponding to the constraints.
    """
    _y_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and y_ is not None and len(y_) != ((last_) - (first_)):
      raise ValueError("Array argument y is not long enough: Is %d, expected %d" % (len(y_),((last_) - (first_))))
    if y_ is None:
      raise ValueError("Argument y cannot be None")
    if y_ is None:
      raise ValueError("Argument y may not be None")
    if isinstance(y_, numpy.ndarray) and y_.dtype is numpy.dtype(numpy.float64) and y_.flags.contiguous:
      _y_copyarray = False
      _y_tmp = ctypes.cast(y_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif y_ is not None:
      _y_copyarray = True
      _y_np_tmp = numpy.zeros(len(y_),numpy.dtype(numpy.float64))
      _y_np_tmp[:] = y_
      assert _y_np_tmp.flags.contiguous
      _y_tmp = ctypes.cast(_y_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _y_copyarray = False
      _y_tmp = None
      
    res = __library__.MSK_XX_putyslice(self.__nativep,whichsol_,first_,last_,_y_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_int,_make_doublevector)
  @synchronized
  def putslcslice(self,whichsol_,first_,last_,slc_):
    """
    Sets a slice of the slc vector for a solution.
  
    putslcslice(self,whichsol_,first_,last_,slc_)
      whichsol: mosek.soltype. Selects a solution.
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      slc: array of double. Dual variables corresponding to the lower bounds on the constraints.
    """
    _slc_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and slc_ is not None and len(slc_) != ((last_) - (first_)):
      raise ValueError("Array argument slc is not long enough: Is %d, expected %d" % (len(slc_),((last_) - (first_))))
    if slc_ is None:
      raise ValueError("Argument slc cannot be None")
    if slc_ is None:
      raise ValueError("Argument slc may not be None")
    if isinstance(slc_, numpy.ndarray) and slc_.dtype is numpy.dtype(numpy.float64) and slc_.flags.contiguous:
      _slc_copyarray = False
      _slc_tmp = ctypes.cast(slc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif slc_ is not None:
      _slc_copyarray = True
      _slc_np_tmp = numpy.zeros(len(slc_),numpy.dtype(numpy.float64))
      _slc_np_tmp[:] = slc_
      assert _slc_np_tmp.flags.contiguous
      _slc_tmp = ctypes.cast(_slc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _slc_copyarray = False
      _slc_tmp = None
      
    res = __library__.MSK_XX_putslcslice(self.__nativep,whichsol_,first_,last_,_slc_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_int,_make_doublevector)
  @synchronized
  def putsucslice(self,whichsol_,first_,last_,suc_):
    """
    Sets a slice of the suc vector for a solution.
  
    putsucslice(self,whichsol_,first_,last_,suc_)
      whichsol: mosek.soltype. Selects a solution.
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      suc: array of double. Dual variables corresponding to the upper bounds on the constraints.
    """
    _suc_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and suc_ is not None and len(suc_) != ((last_) - (first_)):
      raise ValueError("Array argument suc is not long enough: Is %d, expected %d" % (len(suc_),((last_) - (first_))))
    if suc_ is None:
      raise ValueError("Argument suc cannot be None")
    if suc_ is None:
      raise ValueError("Argument suc may not be None")
    if isinstance(suc_, numpy.ndarray) and suc_.dtype is numpy.dtype(numpy.float64) and suc_.flags.contiguous:
      _suc_copyarray = False
      _suc_tmp = ctypes.cast(suc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif suc_ is not None:
      _suc_copyarray = True
      _suc_np_tmp = numpy.zeros(len(suc_),numpy.dtype(numpy.float64))
      _suc_np_tmp[:] = suc_
      assert _suc_np_tmp.flags.contiguous
      _suc_tmp = ctypes.cast(_suc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _suc_copyarray = False
      _suc_tmp = None
      
    res = __library__.MSK_XX_putsucslice(self.__nativep,whichsol_,first_,last_,_suc_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_int,_make_doublevector)
  @synchronized
  def putslxslice(self,whichsol_,first_,last_,slx_):
    """
    Sets a slice of the slx vector for a solution.
  
    putslxslice(self,whichsol_,first_,last_,slx_)
      whichsol: mosek.soltype. Selects a solution.
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      slx: array of double. Dual variables corresponding to the lower bounds on the variables.
    """
    _slx_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and slx_ is not None and len(slx_) != ((last_) - (first_)):
      raise ValueError("Array argument slx is not long enough: Is %d, expected %d" % (len(slx_),((last_) - (first_))))
    if slx_ is None:
      raise ValueError("Argument slx cannot be None")
    if slx_ is None:
      raise ValueError("Argument slx may not be None")
    if isinstance(slx_, numpy.ndarray) and slx_.dtype is numpy.dtype(numpy.float64) and slx_.flags.contiguous:
      _slx_copyarray = False
      _slx_tmp = ctypes.cast(slx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif slx_ is not None:
      _slx_copyarray = True
      _slx_np_tmp = numpy.zeros(len(slx_),numpy.dtype(numpy.float64))
      _slx_np_tmp[:] = slx_
      assert _slx_np_tmp.flags.contiguous
      _slx_tmp = ctypes.cast(_slx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _slx_copyarray = False
      _slx_tmp = None
      
    res = __library__.MSK_XX_putslxslice(self.__nativep,whichsol_,first_,last_,_slx_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_int,_make_doublevector)
  @synchronized
  def putsuxslice(self,whichsol_,first_,last_,sux_):
    """
    Sets a slice of the sux vector for a solution.
  
    putsuxslice(self,whichsol_,first_,last_,sux_)
      whichsol: mosek.soltype. Selects a solution.
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      sux: array of double. Dual variables corresponding to the upper bounds on the variables.
    """
    _sux_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and sux_ is not None and len(sux_) != ((last_) - (first_)):
      raise ValueError("Array argument sux is not long enough: Is %d, expected %d" % (len(sux_),((last_) - (first_))))
    if sux_ is None:
      raise ValueError("Argument sux cannot be None")
    if sux_ is None:
      raise ValueError("Argument sux may not be None")
    if isinstance(sux_, numpy.ndarray) and sux_.dtype is numpy.dtype(numpy.float64) and sux_.flags.contiguous:
      _sux_copyarray = False
      _sux_tmp = ctypes.cast(sux_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif sux_ is not None:
      _sux_copyarray = True
      _sux_np_tmp = numpy.zeros(len(sux_),numpy.dtype(numpy.float64))
      _sux_np_tmp[:] = sux_
      assert _sux_np_tmp.flags.contiguous
      _sux_tmp = ctypes.cast(_sux_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _sux_copyarray = False
      _sux_tmp = None
      
    res = __library__.MSK_XX_putsuxslice(self.__nativep,whichsol_,first_,last_,_sux_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_int,_make_doublevector)
  @synchronized
  def putsnxslice(self,whichsol_,first_,last_,snx_):
    """
    Sets a slice of the snx vector for a solution.
  
    putsnxslice(self,whichsol_,first_,last_,snx_)
      whichsol: mosek.soltype. Selects a solution.
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      snx: array of double. Dual variables corresponding to the conic constraints on the variables.
    """
    _snx_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and snx_ is not None and len(snx_) != ((last_) - (first_)):
      raise ValueError("Array argument snx is not long enough: Is %d, expected %d" % (len(snx_),((last_) - (first_))))
    if snx_ is None:
      raise ValueError("Argument snx cannot be None")
    if snx_ is None:
      raise ValueError("Argument snx may not be None")
    if isinstance(snx_, numpy.ndarray) and snx_.dtype is numpy.dtype(numpy.float64) and snx_.flags.contiguous:
      _snx_copyarray = False
      _snx_tmp = ctypes.cast(snx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif snx_ is not None:
      _snx_copyarray = True
      _snx_np_tmp = numpy.zeros(len(snx_),numpy.dtype(numpy.float64))
      _snx_np_tmp[:] = snx_
      assert _snx_np_tmp.flags.contiguous
      _snx_tmp = ctypes.cast(_snx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _snx_copyarray = False
      _snx_tmp = None
      
    res = __library__.MSK_XX_putsnxslice(self.__nativep,whichsol_,first_,last_,_snx_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_doublevector)
  @synchronized
  def putbarxj(self,whichsol_,j_,barxj_):
    """
    Sets the primal solution for a semidefinite variable.
  
    putbarxj(self,whichsol_,j_,barxj_)
      whichsol: mosek.soltype. Selects a solution.
      j: int. Index of the semidefinite variable.
      barxj: array of double. Value of the j'th variable of barx.
    """
    _barxj_minlength = self.getlenbarvarj((j_))
    if self.getlenbarvarj((j_)) > 0 and barxj_ is not None and len(barxj_) != self.getlenbarvarj((j_)):
      raise ValueError("Array argument barxj is not long enough: Is %d, expected %d" % (len(barxj_),self.getlenbarvarj((j_))))
    if barxj_ is None:
      raise ValueError("Argument barxj cannot be None")
    if barxj_ is None:
      raise ValueError("Argument barxj may not be None")
    if isinstance(barxj_, numpy.ndarray) and barxj_.dtype is numpy.dtype(numpy.float64) and barxj_.flags.contiguous:
      _barxj_copyarray = False
      _barxj_tmp = ctypes.cast(barxj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif barxj_ is not None:
      _barxj_copyarray = True
      _barxj_np_tmp = numpy.zeros(len(barxj_),numpy.dtype(numpy.float64))
      _barxj_np_tmp[:] = barxj_
      assert _barxj_np_tmp.flags.contiguous
      _barxj_tmp = ctypes.cast(_barxj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _barxj_copyarray = False
      _barxj_tmp = None
      
    res = __library__.MSK_XX_putbarxj(self.__nativep,whichsol_,j_,_barxj_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_doublevector)
  @synchronized
  def putbarsj(self,whichsol_,j_,barsj_):
    """
    Sets the dual solution for a semidefinite variable.
  
    putbarsj(self,whichsol_,j_,barsj_)
      whichsol: mosek.soltype. Selects a solution.
      j: int. Index of the semidefinite variable.
      barsj: array of double. Value of the j'th variable of barx.
    """
    _barsj_minlength = self.getlenbarvarj((j_))
    if self.getlenbarvarj((j_)) > 0 and barsj_ is not None and len(barsj_) != self.getlenbarvarj((j_)):
      raise ValueError("Array argument barsj is not long enough: Is %d, expected %d" % (len(barsj_),self.getlenbarvarj((j_))))
    if barsj_ is None:
      raise ValueError("Argument barsj cannot be None")
    if barsj_ is None:
      raise ValueError("Argument barsj may not be None")
    if isinstance(barsj_, numpy.ndarray) and barsj_.dtype is numpy.dtype(numpy.float64) and barsj_.flags.contiguous:
      _barsj_copyarray = False
      _barsj_tmp = ctypes.cast(barsj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif barsj_ is not None:
      _barsj_copyarray = True
      _barsj_np_tmp = numpy.zeros(len(barsj_),numpy.dtype(numpy.float64))
      _barsj_np_tmp[:] = barsj_
      assert _barsj_np_tmp.flags.contiguous
      _barsj_tmp = ctypes.cast(_barsj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _barsj_copyarray = False
      _barsj_tmp = None
      
    res = __library__.MSK_XX_putbarsj(self.__nativep,whichsol_,j_,_barsj_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype),_make_intvector,_accept_doublevector)
  @synchronized
  def getpviolcon(self,whichsol_,sub_,viol_):
    """
    Computes the violation of a primal solution associated to a constraint.
  
    getpviolcon(self,whichsol_,sub_,viol_)
      whichsol: mosek.soltype. Selects a solution.
      sub: array of int. An array of indexes of constraints.
      viol: array of double. List of violations corresponding to sub.
    """
    num_ = None
    if num_ is None:
      num_ = len(sub_)
    elif num_ != len(sub_):
      raise IndexError("Inconsistent length of array sub")
    if sub_ is None:
      raise ValueError("Argument sub cannot be None")
    if sub_ is None:
      raise ValueError("Argument sub may not be None")
    if isinstance(sub_, numpy.ndarray) and sub_.dtype is numpy.dtype(numpy.int32) and sub_.flags.contiguous:
      _sub_copyarray = False
      _sub_tmp = ctypes.cast(sub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif sub_ is not None:
      _sub_copyarray = True
      _sub_np_tmp = numpy.zeros(len(sub_),numpy.dtype(numpy.int32))
      _sub_np_tmp[:] = sub_
      assert _sub_np_tmp.flags.contiguous
      _sub_tmp = ctypes.cast(_sub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _sub_copyarray = False
      _sub_tmp = None
      
    _viol_minlength = (num_)
    if (num_) > 0 and viol_ is not None and len(viol_) != (num_):
      raise ValueError("Array argument viol is not long enough: Is %d, expected %d" % (len(viol_),(num_)))
    if isinstance(viol_,numpy.ndarray) and not viol_.flags.writeable:
      raise ValueError("Argument viol must be writable")
    if viol_ is None:
      raise ValueError("Argument viol may not be None")
    if isinstance(viol_, numpy.ndarray) and viol_.dtype is numpy.dtype(numpy.float64) and viol_.flags.contiguous:
      _viol_copyarray = False
      _viol_tmp = ctypes.cast(viol_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif viol_ is not None:
      _viol_copyarray = True
      _viol_np_tmp = numpy.zeros(len(viol_),numpy.dtype(numpy.float64))
      _viol_np_tmp[:] = viol_
      assert _viol_np_tmp.flags.contiguous
      _viol_tmp = ctypes.cast(_viol_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _viol_copyarray = False
      _viol_tmp = None
      
    res = __library__.MSK_XX_getpviolcon(self.__nativep,whichsol_,num_,_sub_tmp,_viol_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _viol_copyarray:
      viol_[:] = _viol_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_make_intvector,_accept_doublevector)
  @synchronized
  def getpviolvar(self,whichsol_,sub_,viol_):
    """
    Computes the violation of a primal solution for a list of scalar variables.
  
    getpviolvar(self,whichsol_,sub_,viol_)
      whichsol: mosek.soltype. Selects a solution.
      sub: array of int. An array of indexes of x variables.
      viol: array of double. List of violations corresponding to sub.
    """
    num_ = None
    if num_ is None:
      num_ = len(sub_)
    elif num_ != len(sub_):
      raise IndexError("Inconsistent length of array sub")
    if sub_ is None:
      raise ValueError("Argument sub cannot be None")
    if sub_ is None:
      raise ValueError("Argument sub may not be None")
    if isinstance(sub_, numpy.ndarray) and sub_.dtype is numpy.dtype(numpy.int32) and sub_.flags.contiguous:
      _sub_copyarray = False
      _sub_tmp = ctypes.cast(sub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif sub_ is not None:
      _sub_copyarray = True
      _sub_np_tmp = numpy.zeros(len(sub_),numpy.dtype(numpy.int32))
      _sub_np_tmp[:] = sub_
      assert _sub_np_tmp.flags.contiguous
      _sub_tmp = ctypes.cast(_sub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _sub_copyarray = False
      _sub_tmp = None
      
    _viol_minlength = (num_)
    if (num_) > 0 and viol_ is not None and len(viol_) != (num_):
      raise ValueError("Array argument viol is not long enough: Is %d, expected %d" % (len(viol_),(num_)))
    if isinstance(viol_,numpy.ndarray) and not viol_.flags.writeable:
      raise ValueError("Argument viol must be writable")
    if viol_ is None:
      raise ValueError("Argument viol may not be None")
    if isinstance(viol_, numpy.ndarray) and viol_.dtype is numpy.dtype(numpy.float64) and viol_.flags.contiguous:
      _viol_copyarray = False
      _viol_tmp = ctypes.cast(viol_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif viol_ is not None:
      _viol_copyarray = True
      _viol_np_tmp = numpy.zeros(len(viol_),numpy.dtype(numpy.float64))
      _viol_np_tmp[:] = viol_
      assert _viol_np_tmp.flags.contiguous
      _viol_tmp = ctypes.cast(_viol_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _viol_copyarray = False
      _viol_tmp = None
      
    res = __library__.MSK_XX_getpviolvar(self.__nativep,whichsol_,num_,_sub_tmp,_viol_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _viol_copyarray:
      viol_[:] = _viol_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_make_intvector,_accept_doublevector)
  @synchronized
  def getpviolbarvar(self,whichsol_,sub_,viol_):
    """
    Computes the violation of a primal solution for a list of semidefinite variables.
  
    getpviolbarvar(self,whichsol_,sub_,viol_)
      whichsol: mosek.soltype. Selects a solution.
      sub: array of int. An array of indexes of barX variables.
      viol: array of double. List of violations corresponding to sub.
    """
    num_ = None
    if num_ is None:
      num_ = len(sub_)
    elif num_ != len(sub_):
      raise IndexError("Inconsistent length of array sub")
    if sub_ is None:
      raise ValueError("Argument sub cannot be None")
    if sub_ is None:
      raise ValueError("Argument sub may not be None")
    if isinstance(sub_, numpy.ndarray) and sub_.dtype is numpy.dtype(numpy.int32) and sub_.flags.contiguous:
      _sub_copyarray = False
      _sub_tmp = ctypes.cast(sub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif sub_ is not None:
      _sub_copyarray = True
      _sub_np_tmp = numpy.zeros(len(sub_),numpy.dtype(numpy.int32))
      _sub_np_tmp[:] = sub_
      assert _sub_np_tmp.flags.contiguous
      _sub_tmp = ctypes.cast(_sub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _sub_copyarray = False
      _sub_tmp = None
      
    _viol_minlength = (num_)
    if (num_) > 0 and viol_ is not None and len(viol_) != (num_):
      raise ValueError("Array argument viol is not long enough: Is %d, expected %d" % (len(viol_),(num_)))
    if isinstance(viol_,numpy.ndarray) and not viol_.flags.writeable:
      raise ValueError("Argument viol must be writable")
    if viol_ is None:
      raise ValueError("Argument viol may not be None")
    if isinstance(viol_, numpy.ndarray) and viol_.dtype is numpy.dtype(numpy.float64) and viol_.flags.contiguous:
      _viol_copyarray = False
      _viol_tmp = ctypes.cast(viol_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif viol_ is not None:
      _viol_copyarray = True
      _viol_np_tmp = numpy.zeros(len(viol_),numpy.dtype(numpy.float64))
      _viol_np_tmp[:] = viol_
      assert _viol_np_tmp.flags.contiguous
      _viol_tmp = ctypes.cast(_viol_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _viol_copyarray = False
      _viol_tmp = None
      
    res = __library__.MSK_XX_getpviolbarvar(self.__nativep,whichsol_,num_,_sub_tmp,_viol_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _viol_copyarray:
      viol_[:] = _viol_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_make_intvector,_accept_doublevector)
  @synchronized
  def getpviolcones(self,whichsol_,sub_,viol_):
    """
    Computes the violation of a solution for set of conic constraints.
  
    getpviolcones(self,whichsol_,sub_,viol_)
      whichsol: mosek.soltype. Selects a solution.
      sub: array of int. An array of indexes of conic constraints.
      viol: array of double. List of violations corresponding to sub.
    """
    num_ = None
    if num_ is None:
      num_ = len(sub_)
    elif num_ != len(sub_):
      raise IndexError("Inconsistent length of array sub")
    if sub_ is None:
      raise ValueError("Argument sub cannot be None")
    if sub_ is None:
      raise ValueError("Argument sub may not be None")
    if isinstance(sub_, numpy.ndarray) and sub_.dtype is numpy.dtype(numpy.int32) and sub_.flags.contiguous:
      _sub_copyarray = False
      _sub_tmp = ctypes.cast(sub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif sub_ is not None:
      _sub_copyarray = True
      _sub_np_tmp = numpy.zeros(len(sub_),numpy.dtype(numpy.int32))
      _sub_np_tmp[:] = sub_
      assert _sub_np_tmp.flags.contiguous
      _sub_tmp = ctypes.cast(_sub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _sub_copyarray = False
      _sub_tmp = None
      
    _viol_minlength = (num_)
    if (num_) > 0 and viol_ is not None and len(viol_) != (num_):
      raise ValueError("Array argument viol is not long enough: Is %d, expected %d" % (len(viol_),(num_)))
    if isinstance(viol_,numpy.ndarray) and not viol_.flags.writeable:
      raise ValueError("Argument viol must be writable")
    if viol_ is None:
      raise ValueError("Argument viol may not be None")
    if isinstance(viol_, numpy.ndarray) and viol_.dtype is numpy.dtype(numpy.float64) and viol_.flags.contiguous:
      _viol_copyarray = False
      _viol_tmp = ctypes.cast(viol_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif viol_ is not None:
      _viol_copyarray = True
      _viol_np_tmp = numpy.zeros(len(viol_),numpy.dtype(numpy.float64))
      _viol_np_tmp[:] = viol_
      assert _viol_np_tmp.flags.contiguous
      _viol_tmp = ctypes.cast(_viol_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _viol_copyarray = False
      _viol_tmp = None
      
    res = __library__.MSK_XX_getpviolcones(self.__nativep,whichsol_,num_,_sub_tmp,_viol_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _viol_copyarray:
      viol_[:] = _viol_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_make_intvector,_accept_doublevector)
  @synchronized
  def getdviolcon(self,whichsol_,sub_,viol_):
    """
    Computes the violation of a dual solution associated with a set of constraints.
  
    getdviolcon(self,whichsol_,sub_,viol_)
      whichsol: mosek.soltype. Selects a solution.
      sub: array of int. An array of indexes of constraints.
      viol: array of double. List of violations corresponding to sub.
    """
    num_ = None
    if num_ is None:
      num_ = len(sub_)
    elif num_ != len(sub_):
      raise IndexError("Inconsistent length of array sub")
    if sub_ is None:
      raise ValueError("Argument sub cannot be None")
    if sub_ is None:
      raise ValueError("Argument sub may not be None")
    if isinstance(sub_, numpy.ndarray) and sub_.dtype is numpy.dtype(numpy.int32) and sub_.flags.contiguous:
      _sub_copyarray = False
      _sub_tmp = ctypes.cast(sub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif sub_ is not None:
      _sub_copyarray = True
      _sub_np_tmp = numpy.zeros(len(sub_),numpy.dtype(numpy.int32))
      _sub_np_tmp[:] = sub_
      assert _sub_np_tmp.flags.contiguous
      _sub_tmp = ctypes.cast(_sub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _sub_copyarray = False
      _sub_tmp = None
      
    _viol_minlength = (num_)
    if (num_) > 0 and viol_ is not None and len(viol_) != (num_):
      raise ValueError("Array argument viol is not long enough: Is %d, expected %d" % (len(viol_),(num_)))
    if isinstance(viol_,numpy.ndarray) and not viol_.flags.writeable:
      raise ValueError("Argument viol must be writable")
    if viol_ is None:
      raise ValueError("Argument viol may not be None")
    if isinstance(viol_, numpy.ndarray) and viol_.dtype is numpy.dtype(numpy.float64) and viol_.flags.contiguous:
      _viol_copyarray = False
      _viol_tmp = ctypes.cast(viol_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif viol_ is not None:
      _viol_copyarray = True
      _viol_np_tmp = numpy.zeros(len(viol_),numpy.dtype(numpy.float64))
      _viol_np_tmp[:] = viol_
      assert _viol_np_tmp.flags.contiguous
      _viol_tmp = ctypes.cast(_viol_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _viol_copyarray = False
      _viol_tmp = None
      
    res = __library__.MSK_XX_getdviolcon(self.__nativep,whichsol_,num_,_sub_tmp,_viol_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _viol_copyarray:
      viol_[:] = _viol_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_make_intvector,_accept_doublevector)
  @synchronized
  def getdviolvar(self,whichsol_,sub_,viol_):
    """
    Computes the violation of a dual solution associated with a set of scalar variables.
  
    getdviolvar(self,whichsol_,sub_,viol_)
      whichsol: mosek.soltype. Selects a solution.
      sub: array of int. An array of indexes of x variables.
      viol: array of double. List of violations corresponding to sub.
    """
    num_ = None
    if num_ is None:
      num_ = len(sub_)
    elif num_ != len(sub_):
      raise IndexError("Inconsistent length of array sub")
    if sub_ is None:
      raise ValueError("Argument sub cannot be None")
    if sub_ is None:
      raise ValueError("Argument sub may not be None")
    if isinstance(sub_, numpy.ndarray) and sub_.dtype is numpy.dtype(numpy.int32) and sub_.flags.contiguous:
      _sub_copyarray = False
      _sub_tmp = ctypes.cast(sub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif sub_ is not None:
      _sub_copyarray = True
      _sub_np_tmp = numpy.zeros(len(sub_),numpy.dtype(numpy.int32))
      _sub_np_tmp[:] = sub_
      assert _sub_np_tmp.flags.contiguous
      _sub_tmp = ctypes.cast(_sub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _sub_copyarray = False
      _sub_tmp = None
      
    _viol_minlength = (num_)
    if (num_) > 0 and viol_ is not None and len(viol_) != (num_):
      raise ValueError("Array argument viol is not long enough: Is %d, expected %d" % (len(viol_),(num_)))
    if isinstance(viol_,numpy.ndarray) and not viol_.flags.writeable:
      raise ValueError("Argument viol must be writable")
    if viol_ is None:
      raise ValueError("Argument viol may not be None")
    if isinstance(viol_, numpy.ndarray) and viol_.dtype is numpy.dtype(numpy.float64) and viol_.flags.contiguous:
      _viol_copyarray = False
      _viol_tmp = ctypes.cast(viol_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif viol_ is not None:
      _viol_copyarray = True
      _viol_np_tmp = numpy.zeros(len(viol_),numpy.dtype(numpy.float64))
      _viol_np_tmp[:] = viol_
      assert _viol_np_tmp.flags.contiguous
      _viol_tmp = ctypes.cast(_viol_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _viol_copyarray = False
      _viol_tmp = None
      
    res = __library__.MSK_XX_getdviolvar(self.__nativep,whichsol_,num_,_sub_tmp,_viol_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _viol_copyarray:
      viol_[:] = _viol_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_make_intvector,_accept_doublevector)
  @synchronized
  def getdviolbarvar(self,whichsol_,sub_,viol_):
    """
    Computes the violation of dual solution for a set of semidefinite variables.
  
    getdviolbarvar(self,whichsol_,sub_,viol_)
      whichsol: mosek.soltype. Selects a solution.
      sub: array of int. An array of indexes of barx variables.
      viol: array of double. List of violations corresponding to sub.
    """
    num_ = None
    if num_ is None:
      num_ = len(sub_)
    elif num_ != len(sub_):
      raise IndexError("Inconsistent length of array sub")
    if sub_ is None:
      raise ValueError("Argument sub cannot be None")
    if sub_ is None:
      raise ValueError("Argument sub may not be None")
    if isinstance(sub_, numpy.ndarray) and sub_.dtype is numpy.dtype(numpy.int32) and sub_.flags.contiguous:
      _sub_copyarray = False
      _sub_tmp = ctypes.cast(sub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif sub_ is not None:
      _sub_copyarray = True
      _sub_np_tmp = numpy.zeros(len(sub_),numpy.dtype(numpy.int32))
      _sub_np_tmp[:] = sub_
      assert _sub_np_tmp.flags.contiguous
      _sub_tmp = ctypes.cast(_sub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _sub_copyarray = False
      _sub_tmp = None
      
    _viol_minlength = (num_)
    if (num_) > 0 and viol_ is not None and len(viol_) != (num_):
      raise ValueError("Array argument viol is not long enough: Is %d, expected %d" % (len(viol_),(num_)))
    if isinstance(viol_,numpy.ndarray) and not viol_.flags.writeable:
      raise ValueError("Argument viol must be writable")
    if viol_ is None:
      raise ValueError("Argument viol may not be None")
    if isinstance(viol_, numpy.ndarray) and viol_.dtype is numpy.dtype(numpy.float64) and viol_.flags.contiguous:
      _viol_copyarray = False
      _viol_tmp = ctypes.cast(viol_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif viol_ is not None:
      _viol_copyarray = True
      _viol_np_tmp = numpy.zeros(len(viol_),numpy.dtype(numpy.float64))
      _viol_np_tmp[:] = viol_
      assert _viol_np_tmp.flags.contiguous
      _viol_tmp = ctypes.cast(_viol_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _viol_copyarray = False
      _viol_tmp = None
      
    res = __library__.MSK_XX_getdviolbarvar(self.__nativep,whichsol_,num_,_sub_tmp,_viol_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _viol_copyarray:
      viol_[:] = _viol_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_make_intvector,_accept_doublevector)
  @synchronized
  def getdviolcones(self,whichsol_,sub_,viol_):
    """
    Computes the violation of a solution for set of dual conic constraints.
  
    getdviolcones(self,whichsol_,sub_,viol_)
      whichsol: mosek.soltype. Selects a solution.
      sub: array of int. An array of indexes of conic constraints.
      viol: array of double. List of violations corresponding to sub.
    """
    num_ = None
    if num_ is None:
      num_ = len(sub_)
    elif num_ != len(sub_):
      raise IndexError("Inconsistent length of array sub")
    if sub_ is None:
      raise ValueError("Argument sub cannot be None")
    if sub_ is None:
      raise ValueError("Argument sub may not be None")
    if isinstance(sub_, numpy.ndarray) and sub_.dtype is numpy.dtype(numpy.int32) and sub_.flags.contiguous:
      _sub_copyarray = False
      _sub_tmp = ctypes.cast(sub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif sub_ is not None:
      _sub_copyarray = True
      _sub_np_tmp = numpy.zeros(len(sub_),numpy.dtype(numpy.int32))
      _sub_np_tmp[:] = sub_
      assert _sub_np_tmp.flags.contiguous
      _sub_tmp = ctypes.cast(_sub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _sub_copyarray = False
      _sub_tmp = None
      
    _viol_minlength = (num_)
    if (num_) > 0 and viol_ is not None and len(viol_) != (num_):
      raise ValueError("Array argument viol is not long enough: Is %d, expected %d" % (len(viol_),(num_)))
    if isinstance(viol_,numpy.ndarray) and not viol_.flags.writeable:
      raise ValueError("Argument viol must be writable")
    if viol_ is None:
      raise ValueError("Argument viol may not be None")
    if isinstance(viol_, numpy.ndarray) and viol_.dtype is numpy.dtype(numpy.float64) and viol_.flags.contiguous:
      _viol_copyarray = False
      _viol_tmp = ctypes.cast(viol_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif viol_ is not None:
      _viol_copyarray = True
      _viol_np_tmp = numpy.zeros(len(viol_),numpy.dtype(numpy.float64))
      _viol_np_tmp[:] = viol_
      assert _viol_np_tmp.flags.contiguous
      _viol_tmp = ctypes.cast(_viol_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _viol_copyarray = False
      _viol_tmp = None
      
    res = __library__.MSK_XX_getdviolcones(self.__nativep,whichsol_,num_,_sub_tmp,_viol_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _viol_copyarray:
      viol_[:] = _viol_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype))
  @synchronized
  def getsolutioninfo(self,whichsol_):
    """
    Obtains information about of a solution.
  
    getsolutioninfo(self,whichsol_)
      whichsol: mosek.soltype. Selects a solution.
    returns: pobj,pviolcon,pviolvar,pviolbarvar,pviolcone,pviolitg,dobj,dviolcon,dviolvar,dviolbarvar,dviolcone
      pobj: double. The primal objective value.
      pviolcon: double. Maximal primal bound violation for a xc variable.
      pviolvar: double. Maximal primal bound violation for a xx variable.
      pviolbarvar: double. Maximal primal bound violation for a barx variable.
      pviolcone: double. Maximal primal violation of the solution with respect to the conic constraints.
      pviolitg: double. Maximal violation in the integer constraints.
      dobj: double. Dual objective value.
      dviolcon: double. Maximal dual bound violation for a xc variable.
      dviolvar: double. Maximal dual bound violation for a xx variable.
      dviolbarvar: double. Maximal dual bound violation for a bars variable.
      dviolcone: double. Maximum violation of the dual solution in the dual conic constraints.
    """
    pobj_ = ctypes.c_double()
    pviolcon_ = ctypes.c_double()
    pviolvar_ = ctypes.c_double()
    pviolbarvar_ = ctypes.c_double()
    pviolcone_ = ctypes.c_double()
    pviolitg_ = ctypes.c_double()
    dobj_ = ctypes.c_double()
    dviolcon_ = ctypes.c_double()
    dviolvar_ = ctypes.c_double()
    dviolbarvar_ = ctypes.c_double()
    dviolcone_ = ctypes.c_double()
    res = __library__.MSK_XX_getsolutioninfo(self.__nativep,whichsol_,ctypes.byref(pobj_),ctypes.byref(pviolcon_),ctypes.byref(pviolvar_),ctypes.byref(pviolbarvar_),ctypes.byref(pviolcone_),ctypes.byref(pviolitg_),ctypes.byref(dobj_),ctypes.byref(dviolcon_),ctypes.byref(dviolvar_),ctypes.byref(dviolbarvar_),ctypes.byref(dviolcone_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    pobj_ = pobj_.value
    _pobj_return_value = pobj_
    pviolcon_ = pviolcon_.value
    _pviolcon_return_value = pviolcon_
    pviolvar_ = pviolvar_.value
    _pviolvar_return_value = pviolvar_
    pviolbarvar_ = pviolbarvar_.value
    _pviolbarvar_return_value = pviolbarvar_
    pviolcone_ = pviolcone_.value
    _pviolcone_return_value = pviolcone_
    pviolitg_ = pviolitg_.value
    _pviolitg_return_value = pviolitg_
    dobj_ = dobj_.value
    _dobj_return_value = dobj_
    dviolcon_ = dviolcon_.value
    _dviolcon_return_value = dviolcon_
    dviolvar_ = dviolvar_.value
    _dviolvar_return_value = dviolvar_
    dviolbarvar_ = dviolbarvar_.value
    _dviolbarvar_return_value = dviolbarvar_
    dviolcone_ = dviolcone_.value
    _dviolcone_return_value = dviolcone_
    return (_pobj_return_value,_pviolcon_return_value,_pviolvar_return_value,_pviolbarvar_return_value,_pviolcone_return_value,_pviolitg_return_value,_dobj_return_value,_dviolcon_return_value,_dviolvar_return_value,_dviolbarvar_return_value,_dviolcone_return_value)
  @accepts(_accept_any,_accept_anyenum(soltype))
  @synchronized
  def getdualsolutionnorms(self,whichsol_):
    """
    Compute norms of the dual solution.
  
    getdualsolutionnorms(self,whichsol_)
      whichsol: mosek.soltype. Selects a solution.
    returns: nrmy,nrmslc,nrmsuc,nrmslx,nrmsux,nrmsnx,nrmbars
      nrmy: double. The norm of the y vector.
      nrmslc: double. The norm of the slc vector.
      nrmsuc: double. The norm of the suc vector.
      nrmslx: double. The norm of the slx vector.
      nrmsux: double. The norm of the sux vector.
      nrmsnx: double. The norm of the snx vector.
      nrmbars: double. The norm of the bars vector.
    """
    nrmy_ = ctypes.c_double()
    nrmslc_ = ctypes.c_double()
    nrmsuc_ = ctypes.c_double()
    nrmslx_ = ctypes.c_double()
    nrmsux_ = ctypes.c_double()
    nrmsnx_ = ctypes.c_double()
    nrmbars_ = ctypes.c_double()
    res = __library__.MSK_XX_getdualsolutionnorms(self.__nativep,whichsol_,ctypes.byref(nrmy_),ctypes.byref(nrmslc_),ctypes.byref(nrmsuc_),ctypes.byref(nrmslx_),ctypes.byref(nrmsux_),ctypes.byref(nrmsnx_),ctypes.byref(nrmbars_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    nrmy_ = nrmy_.value
    _nrmy_return_value = nrmy_
    nrmslc_ = nrmslc_.value
    _nrmslc_return_value = nrmslc_
    nrmsuc_ = nrmsuc_.value
    _nrmsuc_return_value = nrmsuc_
    nrmslx_ = nrmslx_.value
    _nrmslx_return_value = nrmslx_
    nrmsux_ = nrmsux_.value
    _nrmsux_return_value = nrmsux_
    nrmsnx_ = nrmsnx_.value
    _nrmsnx_return_value = nrmsnx_
    nrmbars_ = nrmbars_.value
    _nrmbars_return_value = nrmbars_
    return (_nrmy_return_value,_nrmslc_return_value,_nrmsuc_return_value,_nrmslx_return_value,_nrmsux_return_value,_nrmsnx_return_value,_nrmbars_return_value)
  @accepts(_accept_any,_accept_anyenum(soltype))
  @synchronized
  def getprimalsolutionnorms(self,whichsol_):
    """
    Compute norms of the primal solution.
  
    getprimalsolutionnorms(self,whichsol_)
      whichsol: mosek.soltype. Selects a solution.
    returns: nrmxc,nrmxx,nrmbarx
      nrmxc: double. The norm of the xc vector.
      nrmxx: double. The norm of the xx vector.
      nrmbarx: double. The norm of the barX vector.
    """
    nrmxc_ = ctypes.c_double()
    nrmxx_ = ctypes.c_double()
    nrmbarx_ = ctypes.c_double()
    res = __library__.MSK_XX_getprimalsolutionnorms(self.__nativep,whichsol_,ctypes.byref(nrmxc_),ctypes.byref(nrmxx_),ctypes.byref(nrmbarx_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    nrmxc_ = nrmxc_.value
    _nrmxc_return_value = nrmxc_
    nrmxx_ = nrmxx_.value
    _nrmxx_return_value = nrmxx_
    nrmbarx_ = nrmbarx_.value
    _nrmbarx_return_value = nrmbarx_
    return (_nrmxc_return_value,_nrmxx_return_value,_nrmbarx_return_value)
  @accepts(_accept_any,_accept_anyenum(soltype),_accept_anyenum(solitem),_make_int,_make_int,_accept_doublevector)
  @synchronized
  def getsolutionslice(self,whichsol_,solitem_,first_,last_,values_):
    """
    Obtains a slice of the solution.
  
    getsolutionslice(self,whichsol_,solitem_,first_,last_,values_)
      whichsol: mosek.soltype. Selects a solution.
      solitem: mosek.solitem. Which part of the solution is required.
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      values: array of double. The values of the requested solution elements.
    """
    _values_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and values_ is not None and len(values_) != ((last_) - (first_)):
      raise ValueError("Array argument values is not long enough: Is %d, expected %d" % (len(values_),((last_) - (first_))))
    if isinstance(values_,numpy.ndarray) and not values_.flags.writeable:
      raise ValueError("Argument values must be writable")
    if isinstance(values_, numpy.ndarray) and values_.dtype is numpy.dtype(numpy.float64) and values_.flags.contiguous:
      _values_copyarray = False
      _values_tmp = ctypes.cast(values_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif values_ is not None:
      _values_copyarray = True
      _values_np_tmp = numpy.zeros(len(values_),numpy.dtype(numpy.float64))
      _values_np_tmp[:] = values_
      assert _values_np_tmp.flags.contiguous
      _values_tmp = ctypes.cast(_values_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _values_copyarray = False
      _values_tmp = None
      
    res = __library__.MSK_XX_getsolutionslice(self.__nativep,whichsol_,solitem_,first_,last_,_values_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _values_copyarray:
      values_[:] = _values_np_tmp
  @accepts(_accept_any,_accept_anyenum(soltype),_make_int,_make_int,_accept_doublevector)
  @synchronized
  def getreducedcosts(self,whichsol_,first_,last_,redcosts_):
    """
    Obtains the reduced costs for a sequence of variables.
  
    getreducedcosts(self,whichsol_,first_,last_,redcosts_)
      whichsol: mosek.soltype. Selects a solution.
      first: int. The index of the first variable in the sequence.
      last: int. The index of the last variable in the sequence plus 1.
      redcosts: array of double. Returns the requested reduced costs.
    """
    _redcosts_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and redcosts_ is not None and len(redcosts_) != ((last_) - (first_)):
      raise ValueError("Array argument redcosts is not long enough: Is %d, expected %d" % (len(redcosts_),((last_) - (first_))))
    if isinstance(redcosts_,numpy.ndarray) and not redcosts_.flags.writeable:
      raise ValueError("Argument redcosts must be writable")
    if isinstance(redcosts_, numpy.ndarray) and redcosts_.dtype is numpy.dtype(numpy.float64) and redcosts_.flags.contiguous:
      _redcosts_copyarray = False
      _redcosts_tmp = ctypes.cast(redcosts_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif redcosts_ is not None:
      _redcosts_copyarray = True
      _redcosts_np_tmp = numpy.zeros(len(redcosts_),numpy.dtype(numpy.float64))
      _redcosts_np_tmp[:] = redcosts_
      assert _redcosts_np_tmp.flags.contiguous
      _redcosts_tmp = ctypes.cast(_redcosts_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _redcosts_copyarray = False
      _redcosts_tmp = None
      
    res = __library__.MSK_XX_getreducedcosts(self.__nativep,whichsol_,first_,last_,_redcosts_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _redcosts_copyarray:
      redcosts_[:] = _redcosts_np_tmp
  @accepts(_accept_any,_accept_anyenum(sparam))
  @synchronized
  def getstrparam(self,param_):
    """
    Obtains the value of a string parameter.
  
    getstrparam(self,param_)
      param: mosek.sparam. Which parameter.
    returns: len,parvalue
      len: int. The length of the parameter value.
      parvalue: unicode. If this is not a null pointer, the parameter value is stored here.
    """
    maxlen_ = (1 + self.getstrparamlen((param_)))
    len_ = ctypes.c_int32()
    parvalue_ = (ctypes.c_char * (maxlen_))()
    res = __library__.MSK_XX_getstrparam(self.__nativep,param_,maxlen_,ctypes.byref(len_),parvalue_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    len_ = len_.value
    _len_return_value = len_
    _parvalue_retval = parvalue_.value.decode("utf-8",errors="replace")
    return (_len_return_value,_parvalue_retval)
  @accepts(_accept_any,_accept_anyenum(sparam))
  @synchronized
  def getstrparamlen(self,param_):
    """
    Obtains the length of a string parameter.
  
    getstrparamlen(self,param_)
      param: mosek.sparam. Which parameter.
    returns: len
      len: int. The length of the parameter value.
    """
    len_ = ctypes.c_int32()
    res = __library__.MSK_XX_getstrparamlen(self.__nativep,param_,ctypes.byref(len_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    len_ = len_.value
    _len_return_value = len_
    return (_len_return_value)
  @accepts(_accept_any)
  @synchronized
  def gettasknamelen(self):
    """
    Obtains the length the task name.
  
    gettasknamelen(self)
    returns: len
      len: int. Returns the length of the task name.
    """
    len_ = ctypes.c_int32()
    res = __library__.MSK_XX_gettasknamelen(self.__nativep,ctypes.byref(len_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    len_ = len_.value
    _len_return_value = len_
    return (_len_return_value)
  @accepts(_accept_any)
  @synchronized
  def gettaskname(self):
    """
    Obtains the task name.
  
    gettaskname(self)
    returns: taskname
      taskname: unicode. Returns the task name.
    """
    sizetaskname_ = (1 + self.gettasknamelen())
    taskname_ = (ctypes.c_char * (sizetaskname_))()
    res = __library__.MSK_XX_gettaskname(self.__nativep,sizetaskname_,taskname_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _taskname_retval = taskname_.value.decode("utf-8",errors="replace")
    return (_taskname_retval)
  @accepts(_accept_any,_make_int)
  @synchronized
  def getvartype(self,j_):
    """
    Gets the variable type of one variable.
  
    getvartype(self,j_)
      j: int. Index of the variable.
    returns: vartype
      vartype: mosek.variabletype. Variable type of variable index j.
    """
    vartype_ = ctypes.c_int32()
    res = __library__.MSK_XX_getvartype(self.__nativep,j_,ctypes.byref(vartype_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _vartype_return_value = variabletype(vartype_.value)
    return (_vartype_return_value)
  @accepts(_accept_any,_make_intvector,_accept_any)
  @synchronized
  def getvartypelist(self,subj_,vartype_):
    """
    Obtains the variable type for one or more variables.
  
    getvartypelist(self,subj_,vartype_)
      subj: array of int. A list of variable indexes.
      vartype: array of mosek.variabletype. Returns the variables types corresponding the variable indexes requested.
    """
    num_ = None
    if num_ is None:
      num_ = len(subj_)
    elif num_ != len(subj_):
      raise IndexError("Inconsistent length of array subj")
    if subj_ is None:
      raise ValueError("Argument subj cannot be None")
    if subj_ is None:
      raise ValueError("Argument subj may not be None")
    if isinstance(subj_, numpy.ndarray) and subj_.dtype is numpy.dtype(numpy.int32) and subj_.flags.contiguous:
      _subj_copyarray = False
      _subj_tmp = ctypes.cast(subj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subj_ is not None:
      _subj_copyarray = True
      _subj_np_tmp = numpy.zeros(len(subj_),numpy.dtype(numpy.int32))
      _subj_np_tmp[:] = subj_
      assert _subj_np_tmp.flags.contiguous
      _subj_tmp = ctypes.cast(_subj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subj_copyarray = False
      _subj_tmp = None
      
    _vartype_minlength = (num_)
    if (num_) > 0 and vartype_ is not None and len(vartype_) != (num_):
      raise ValueError("Array argument vartype is not long enough: Is %d, expected %d" % (len(vartype_),(num_)))
    if isinstance(vartype_,numpy.ndarray) and not vartype_.flags.writeable:
      raise ValueError("Argument vartype must be writable")
    if vartype_ is not None:
        _vartype_tmp = (ctypes.c_int32 * len(vartype_))()
    else:
        _vartype_tmp = None
    res = __library__.MSK_XX_getvartypelist(self.__nativep,num_,_subj_tmp,_vartype_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if vartype_ is not None: vartype_[:] = [ variabletype(v) for v in _vartype_tmp[0:len(vartype_)] ]
  @accepts(_accept_any,_make_int,_make_int,_make_doublevector,_make_double,_make_longvector,_make_longvector,_make_intvector,_make_doublevector,_make_anyenumvector(boundkey),_make_doublevector,_make_doublevector,_make_anyenumvector(boundkey),_make_doublevector,_make_doublevector)
  @synchronized
  def inputdata(self,maxnumcon_,maxnumvar_,c_,cfix_,aptrb_,aptre_,asub_,aval_,bkc_,blc_,buc_,bkx_,blx_,bux_):
    """
    Input the linear part of an optimization task in one function call.
  
    inputdata(self,maxnumcon_,maxnumvar_,c_,cfix_,aptrb_,aptre_,asub_,aval_,bkc_,blc_,buc_,bkx_,blx_,bux_)
      maxnumcon: int. Number of preallocated constraints in the optimization task.
      maxnumvar: int. Number of preallocated variables in the optimization task.
      c: array of double. Linear terms of the objective as a dense vector. The length is the number of variables.
      cfix: double. Fixed term in the objective.
      aptrb: array of long. Row or column start pointers.
      aptre: array of long. Row or column end pointers.
      asub: array of int. Coefficient subscripts.
      aval: array of double. Coefficient values.
      bkc: array of mosek.boundkey. Bound keys for the constraints.
      blc: array of double. Lower bounds for the constraints.
      buc: array of double. Upper bounds for the constraints.
      bkx: array of mosek.boundkey. Bound keys for the variables.
      blx: array of double. Lower bounds for the variables.
      bux: array of double. Upper bounds for the variables.
    """
    numcon_ = None
    if numcon_ is None:
      numcon_ = len(buc_)
    elif numcon_ != len(buc_):
      raise IndexError("Inconsistent length of array buc")
    if numcon_ is None:
      numcon_ = len(blc_)
    elif numcon_ != len(blc_):
      raise IndexError("Inconsistent length of array blc")
    if numcon_ is None:
      numcon_ = len(bkc_)
    elif numcon_ != len(bkc_):
      raise IndexError("Inconsistent length of array bkc")
    numvar_ = None
    if numvar_ is None:
      numvar_ = len(c_)
    elif numvar_ != len(c_):
      raise IndexError("Inconsistent length of array c")
    if numvar_ is None:
      numvar_ = len(bux_)
    elif numvar_ != len(bux_):
      raise IndexError("Inconsistent length of array bux")
    if numvar_ is None:
      numvar_ = len(blx_)
    elif numvar_ != len(blx_):
      raise IndexError("Inconsistent length of array blx")
    if numvar_ is None:
      numvar_ = len(bkx_)
    elif numvar_ != len(bkx_):
      raise IndexError("Inconsistent length of array bkx")
    if numvar_ is None:
      numvar_ = len(aptrb_)
    elif numvar_ != len(aptrb_):
      raise IndexError("Inconsistent length of array aptrb")
    if numvar_ is None:
      numvar_ = len(aptre_)
    elif numvar_ != len(aptre_):
      raise IndexError("Inconsistent length of array aptre")
    if isinstance(c_, numpy.ndarray) and c_.dtype is numpy.dtype(numpy.float64) and c_.flags.contiguous:
      _c_copyarray = False
      _c_tmp = ctypes.cast(c_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif c_ is not None:
      _c_copyarray = True
      _c_np_tmp = numpy.zeros(len(c_),numpy.dtype(numpy.float64))
      _c_np_tmp[:] = c_
      assert _c_np_tmp.flags.contiguous
      _c_tmp = ctypes.cast(_c_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _c_copyarray = False
      _c_tmp = None
      
    if aptrb_ is None:
      raise ValueError("Argument aptrb cannot be None")
    if aptrb_ is None:
      raise ValueError("Argument aptrb may not be None")
    if isinstance(aptrb_, numpy.ndarray) and aptrb_.dtype is numpy.dtype(numpy.int64) and aptrb_.flags.contiguous:
      _aptrb_copyarray = False
      _aptrb_tmp = ctypes.cast(aptrb_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif aptrb_ is not None:
      _aptrb_copyarray = True
      _aptrb_np_tmp = numpy.zeros(len(aptrb_),numpy.dtype(numpy.int64))
      _aptrb_np_tmp[:] = aptrb_
      assert _aptrb_np_tmp.flags.contiguous
      _aptrb_tmp = ctypes.cast(_aptrb_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _aptrb_copyarray = False
      _aptrb_tmp = None
      
    if aptre_ is None:
      raise ValueError("Argument aptre cannot be None")
    if aptre_ is None:
      raise ValueError("Argument aptre may not be None")
    if isinstance(aptre_, numpy.ndarray) and aptre_.dtype is numpy.dtype(numpy.int64) and aptre_.flags.contiguous:
      _aptre_copyarray = False
      _aptre_tmp = ctypes.cast(aptre_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif aptre_ is not None:
      _aptre_copyarray = True
      _aptre_np_tmp = numpy.zeros(len(aptre_),numpy.dtype(numpy.int64))
      _aptre_np_tmp[:] = aptre_
      assert _aptre_np_tmp.flags.contiguous
      _aptre_tmp = ctypes.cast(_aptre_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _aptre_copyarray = False
      _aptre_tmp = None
      
    if asub_ is None:
      raise ValueError("Argument asub cannot be None")
    if asub_ is None:
      raise ValueError("Argument asub may not be None")
    if isinstance(asub_, numpy.ndarray) and asub_.dtype is numpy.dtype(numpy.int32) and asub_.flags.contiguous:
      _asub_copyarray = False
      _asub_tmp = ctypes.cast(asub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif asub_ is not None:
      _asub_copyarray = True
      _asub_np_tmp = numpy.zeros(len(asub_),numpy.dtype(numpy.int32))
      _asub_np_tmp[:] = asub_
      assert _asub_np_tmp.flags.contiguous
      _asub_tmp = ctypes.cast(_asub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _asub_copyarray = False
      _asub_tmp = None
      
    if aval_ is None:
      raise ValueError("Argument aval cannot be None")
    if aval_ is None:
      raise ValueError("Argument aval may not be None")
    if isinstance(aval_, numpy.ndarray) and aval_.dtype is numpy.dtype(numpy.float64) and aval_.flags.contiguous:
      _aval_copyarray = False
      _aval_tmp = ctypes.cast(aval_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif aval_ is not None:
      _aval_copyarray = True
      _aval_np_tmp = numpy.zeros(len(aval_),numpy.dtype(numpy.float64))
      _aval_np_tmp[:] = aval_
      assert _aval_np_tmp.flags.contiguous
      _aval_tmp = ctypes.cast(_aval_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _aval_copyarray = False
      _aval_tmp = None
      
    if bkc_ is None:
      raise ValueError("Argument bkc cannot be None")
    if bkc_ is None:
      raise ValueError("Argument bkc may not be None")
    if bkc_ is not None:
        _bkc_tmp = (ctypes.c_int32 * len(bkc_))(*bkc_)
    else:
        _bkc_tmp = None
    if blc_ is None:
      raise ValueError("Argument blc cannot be None")
    if blc_ is None:
      raise ValueError("Argument blc may not be None")
    if isinstance(blc_, numpy.ndarray) and blc_.dtype is numpy.dtype(numpy.float64) and blc_.flags.contiguous:
      _blc_copyarray = False
      _blc_tmp = ctypes.cast(blc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif blc_ is not None:
      _blc_copyarray = True
      _blc_np_tmp = numpy.zeros(len(blc_),numpy.dtype(numpy.float64))
      _blc_np_tmp[:] = blc_
      assert _blc_np_tmp.flags.contiguous
      _blc_tmp = ctypes.cast(_blc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _blc_copyarray = False
      _blc_tmp = None
      
    if buc_ is None:
      raise ValueError("Argument buc cannot be None")
    if buc_ is None:
      raise ValueError("Argument buc may not be None")
    if isinstance(buc_, numpy.ndarray) and buc_.dtype is numpy.dtype(numpy.float64) and buc_.flags.contiguous:
      _buc_copyarray = False
      _buc_tmp = ctypes.cast(buc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif buc_ is not None:
      _buc_copyarray = True
      _buc_np_tmp = numpy.zeros(len(buc_),numpy.dtype(numpy.float64))
      _buc_np_tmp[:] = buc_
      assert _buc_np_tmp.flags.contiguous
      _buc_tmp = ctypes.cast(_buc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _buc_copyarray = False
      _buc_tmp = None
      
    if bkx_ is None:
      raise ValueError("Argument bkx cannot be None")
    if bkx_ is None:
      raise ValueError("Argument bkx may not be None")
    if bkx_ is not None:
        _bkx_tmp = (ctypes.c_int32 * len(bkx_))(*bkx_)
    else:
        _bkx_tmp = None
    if blx_ is None:
      raise ValueError("Argument blx cannot be None")
    if blx_ is None:
      raise ValueError("Argument blx may not be None")
    if isinstance(blx_, numpy.ndarray) and blx_.dtype is numpy.dtype(numpy.float64) and blx_.flags.contiguous:
      _blx_copyarray = False
      _blx_tmp = ctypes.cast(blx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif blx_ is not None:
      _blx_copyarray = True
      _blx_np_tmp = numpy.zeros(len(blx_),numpy.dtype(numpy.float64))
      _blx_np_tmp[:] = blx_
      assert _blx_np_tmp.flags.contiguous
      _blx_tmp = ctypes.cast(_blx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _blx_copyarray = False
      _blx_tmp = None
      
    if bux_ is None:
      raise ValueError("Argument bux cannot be None")
    if bux_ is None:
      raise ValueError("Argument bux may not be None")
    if isinstance(bux_, numpy.ndarray) and bux_.dtype is numpy.dtype(numpy.float64) and bux_.flags.contiguous:
      _bux_copyarray = False
      _bux_tmp = ctypes.cast(bux_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif bux_ is not None:
      _bux_copyarray = True
      _bux_np_tmp = numpy.zeros(len(bux_),numpy.dtype(numpy.float64))
      _bux_np_tmp[:] = bux_
      assert _bux_np_tmp.flags.contiguous
      _bux_tmp = ctypes.cast(_bux_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _bux_copyarray = False
      _bux_tmp = None
      
    res = __library__.MSK_XX_inputdata64(self.__nativep,maxnumcon_,maxnumvar_,numcon_,numvar_,_c_tmp,cfix_,_aptrb_tmp,_aptre_tmp,_asub_tmp,_aval_tmp,_bkc_tmp,_blc_tmp,_buc_tmp,_bkx_tmp,_blx_tmp,_bux_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_str)
  @synchronized
  def isdouparname(self,parname_):
    """
    Checks a double parameter name.
  
    isdouparname(self,parname_)
      parname: str|unicode. Parameter name.
    returns: param
      param: mosek.dparam. Returns the parameter corresponding to the name, if one exists.
    """
    if isinstance(parname_,unicode):
      parname_ = parname_.encode("utf-8",errors="replace")
    param_ = ctypes.c_int32()
    res = __library__.MSK_XX_isdouparname(self.__nativep,parname_,ctypes.byref(param_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _param_return_value = dparam(param_.value)
    return (_param_return_value)
  @accepts(_accept_any,_accept_str)
  @synchronized
  def isintparname(self,parname_):
    """
    Checks an integer parameter name.
  
    isintparname(self,parname_)
      parname: str|unicode. Parameter name.
    returns: param
      param: mosek.iparam. Returns the parameter corresponding to the name, if one exists.
    """
    if isinstance(parname_,unicode):
      parname_ = parname_.encode("utf-8",errors="replace")
    param_ = ctypes.c_int32()
    res = __library__.MSK_XX_isintparname(self.__nativep,parname_,ctypes.byref(param_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _param_return_value = iparam(param_.value)
    return (_param_return_value)
  @accepts(_accept_any,_accept_str)
  @synchronized
  def isstrparname(self,parname_):
    """
    Checks a string parameter name.
  
    isstrparname(self,parname_)
      parname: str|unicode. Parameter name.
    returns: param
      param: mosek.sparam. Returns the parameter corresponding to the name, if one exists.
    """
    if isinstance(parname_,unicode):
      parname_ = parname_.encode("utf-8",errors="replace")
    param_ = ctypes.c_int32()
    res = __library__.MSK_XX_isstrparname(self.__nativep,parname_,ctypes.byref(param_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _param_return_value = sparam(param_.value)
    return (_param_return_value)
  @accepts(_accept_any,_accept_anyenum(streamtype),_accept_str,_make_int)
  @synchronized
  def linkfiletostream(self,whichstream_,filename_,append_):
    """
    Directs all output from a task stream to a file.
  
    linkfiletostream(self,whichstream_,filename_,append_)
      whichstream: mosek.streamtype. Index of the stream.
      filename: str|unicode. A valid file name.
      append: int. If this argument is 0 the output file will be overwritten, otherwise it will be appended to.
    """
    if isinstance(filename_,unicode):
      filename_ = filename_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_linkfiletotaskstream(self.__nativep,whichstream_,filename_,append_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_doublevector,_make_doublevector,_make_doublevector,_make_doublevector)
  @synchronized
  def primalrepair(self,wlc_,wuc_,wlx_,wux_):
    """
    Repairs a primal infeasible optimization problem by adjusting the bounds on the constraints and variables.
  
    primalrepair(self,wlc_,wuc_,wlx_,wux_)
      wlc: array of double. Weights associated with relaxing lower bounds on the constraints.
      wuc: array of double. Weights associated with relaxing the upper bound on the constraints.
      wlx: array of double. Weights associated with relaxing the lower bounds of the variables.
      wux: array of double. Weights associated with relaxing the upper bounds of variables.
    """
    _wlc_minlength = self.getnumcon()
    if self.getnumcon() > 0 and wlc_ is not None and len(wlc_) != self.getnumcon():
      raise ValueError("Array argument wlc is not long enough: Is %d, expected %d" % (len(wlc_),self.getnumcon()))
    if isinstance(wlc_, numpy.ndarray) and wlc_.dtype is numpy.dtype(numpy.float64) and wlc_.flags.contiguous:
      _wlc_copyarray = False
      _wlc_tmp = ctypes.cast(wlc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif wlc_ is not None:
      _wlc_copyarray = True
      _wlc_np_tmp = numpy.zeros(len(wlc_),numpy.dtype(numpy.float64))
      _wlc_np_tmp[:] = wlc_
      assert _wlc_np_tmp.flags.contiguous
      _wlc_tmp = ctypes.cast(_wlc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _wlc_copyarray = False
      _wlc_tmp = None
      
    _wuc_minlength = self.getnumcon()
    if self.getnumcon() > 0 and wuc_ is not None and len(wuc_) != self.getnumcon():
      raise ValueError("Array argument wuc is not long enough: Is %d, expected %d" % (len(wuc_),self.getnumcon()))
    if isinstance(wuc_, numpy.ndarray) and wuc_.dtype is numpy.dtype(numpy.float64) and wuc_.flags.contiguous:
      _wuc_copyarray = False
      _wuc_tmp = ctypes.cast(wuc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif wuc_ is not None:
      _wuc_copyarray = True
      _wuc_np_tmp = numpy.zeros(len(wuc_),numpy.dtype(numpy.float64))
      _wuc_np_tmp[:] = wuc_
      assert _wuc_np_tmp.flags.contiguous
      _wuc_tmp = ctypes.cast(_wuc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _wuc_copyarray = False
      _wuc_tmp = None
      
    _wlx_minlength = self.getnumvar()
    if self.getnumvar() > 0 and wlx_ is not None and len(wlx_) != self.getnumvar():
      raise ValueError("Array argument wlx is not long enough: Is %d, expected %d" % (len(wlx_),self.getnumvar()))
    if isinstance(wlx_, numpy.ndarray) and wlx_.dtype is numpy.dtype(numpy.float64) and wlx_.flags.contiguous:
      _wlx_copyarray = False
      _wlx_tmp = ctypes.cast(wlx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif wlx_ is not None:
      _wlx_copyarray = True
      _wlx_np_tmp = numpy.zeros(len(wlx_),numpy.dtype(numpy.float64))
      _wlx_np_tmp[:] = wlx_
      assert _wlx_np_tmp.flags.contiguous
      _wlx_tmp = ctypes.cast(_wlx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _wlx_copyarray = False
      _wlx_tmp = None
      
    _wux_minlength = self.getnumvar()
    if self.getnumvar() > 0 and wux_ is not None and len(wux_) != self.getnumvar():
      raise ValueError("Array argument wux is not long enough: Is %d, expected %d" % (len(wux_),self.getnumvar()))
    if isinstance(wux_, numpy.ndarray) and wux_.dtype is numpy.dtype(numpy.float64) and wux_.flags.contiguous:
      _wux_copyarray = False
      _wux_tmp = ctypes.cast(wux_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif wux_ is not None:
      _wux_copyarray = True
      _wux_np_tmp = numpy.zeros(len(wux_),numpy.dtype(numpy.float64))
      _wux_np_tmp[:] = wux_
      assert _wux_np_tmp.flags.contiguous
      _wux_tmp = ctypes.cast(_wux_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _wux_copyarray = False
      _wux_tmp = None
      
    res = __library__.MSK_XX_primalrepair(self.__nativep,_wlc_tmp,_wuc_tmp,_wlx_tmp,_wux_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any)
  @synchronized
  def toconic(self):
    """
    In-place reformulation of a QCQP to a COP
  
    toconic(self)
    """
    res = __library__.MSK_XX_toconic(self.__nativep)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any)
  @synchronized
  def optimize(self):
    """
    Optimizes the problem.
  
    optimize(self)
    returns: trmcode
      trmcode: mosek.rescode. Is either OK or a termination response code.
    """
    trmcode_ = ctypes.c_int32()
    res = __library__.MSK_XX_optimizetrm(self.__nativep,ctypes.byref(trmcode_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _trmcode_return_value = rescode(trmcode_.value)
    return (_trmcode_return_value)
  @accepts(_accept_any)
  @synchronized
  def commitchanges(self):
    """
    Commits all cached problem changes.
  
    commitchanges(self)
    """
    res = __library__.MSK_XX_commitchanges(self.__nativep)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_doublevector)
  @synchronized
  def getatruncatetol(self,tolzero_):
    """
    Gets the current A matrix truncation threshold.
  
    getatruncatetol(self,tolzero_)
      tolzero: array of double. Truncation tolerance.
    """
    _tolzero_minlength = 1
    if 1 > 0 and tolzero_ is not None and len(tolzero_) != 1:
      raise ValueError("Array argument tolzero is not long enough: Is %d, expected %d" % (len(tolzero_),1))
    if isinstance(tolzero_,numpy.ndarray) and not tolzero_.flags.writeable:
      raise ValueError("Argument tolzero must be writable")
    if tolzero_ is None:
      raise ValueError("Argument tolzero may not be None")
    if isinstance(tolzero_, numpy.ndarray) and tolzero_.dtype is numpy.dtype(numpy.float64) and tolzero_.flags.contiguous:
      _tolzero_copyarray = False
      _tolzero_tmp = ctypes.cast(tolzero_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif tolzero_ is not None:
      _tolzero_copyarray = True
      _tolzero_np_tmp = numpy.zeros(len(tolzero_),numpy.dtype(numpy.float64))
      _tolzero_np_tmp[:] = tolzero_
      assert _tolzero_np_tmp.flags.contiguous
      _tolzero_tmp = ctypes.cast(_tolzero_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _tolzero_copyarray = False
      _tolzero_tmp = None
      
    res = __library__.MSK_XX_getatruncatetol(self.__nativep,_tolzero_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _tolzero_copyarray:
      tolzero_[:] = _tolzero_np_tmp
  @accepts(_accept_any,_make_double)
  @synchronized
  def putatruncatetol(self,tolzero_):
    """
    Truncates all elements in A below a certain tolerance to zero.
  
    putatruncatetol(self,tolzero_)
      tolzero: double. Truncation tolerance.
    """
    res = __library__.MSK_XX_putatruncatetol(self.__nativep,tolzero_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_make_int,_make_double)
  @synchronized
  def putaij(self,i_,j_,aij_):
    """
    Changes a single value in the linear coefficient matrix.
  
    putaij(self,i_,j_,aij_)
      i: int. Constraint (row) index.
      j: int. Variable (column) index.
      aij: double. New coefficient.
    """
    res = __library__.MSK_XX_putaij(self.__nativep,i_,j_,aij_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_intvector,_make_intvector,_make_doublevector)
  @synchronized
  def putaijlist(self,subi_,subj_,valij_):
    """
    Changes one or more coefficients in the linear constraint matrix.
  
    putaijlist(self,subi_,subj_,valij_)
      subi: array of int. Constraint (row) indices.
      subj: array of int. Variable (column) indices.
      valij: array of double. New coefficient values.
    """
    num_ = None
    if num_ is None:
      num_ = len(subi_)
    elif num_ != len(subi_):
      raise IndexError("Inconsistent length of array subi")
    if num_ is None:
      num_ = len(subj_)
    elif num_ != len(subj_):
      raise IndexError("Inconsistent length of array subj")
    if num_ is None:
      num_ = len(valij_)
    elif num_ != len(valij_):
      raise IndexError("Inconsistent length of array valij")
    if subi_ is None:
      raise ValueError("Argument subi cannot be None")
    if subi_ is None:
      raise ValueError("Argument subi may not be None")
    if isinstance(subi_, numpy.ndarray) and subi_.dtype is numpy.dtype(numpy.int32) and subi_.flags.contiguous:
      _subi_copyarray = False
      _subi_tmp = ctypes.cast(subi_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subi_ is not None:
      _subi_copyarray = True
      _subi_np_tmp = numpy.zeros(len(subi_),numpy.dtype(numpy.int32))
      _subi_np_tmp[:] = subi_
      assert _subi_np_tmp.flags.contiguous
      _subi_tmp = ctypes.cast(_subi_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subi_copyarray = False
      _subi_tmp = None
      
    if subj_ is None:
      raise ValueError("Argument subj cannot be None")
    if subj_ is None:
      raise ValueError("Argument subj may not be None")
    if isinstance(subj_, numpy.ndarray) and subj_.dtype is numpy.dtype(numpy.int32) and subj_.flags.contiguous:
      _subj_copyarray = False
      _subj_tmp = ctypes.cast(subj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subj_ is not None:
      _subj_copyarray = True
      _subj_np_tmp = numpy.zeros(len(subj_),numpy.dtype(numpy.int32))
      _subj_np_tmp[:] = subj_
      assert _subj_np_tmp.flags.contiguous
      _subj_tmp = ctypes.cast(_subj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subj_copyarray = False
      _subj_tmp = None
      
    if valij_ is None:
      raise ValueError("Argument valij cannot be None")
    if valij_ is None:
      raise ValueError("Argument valij may not be None")
    if isinstance(valij_, numpy.ndarray) and valij_.dtype is numpy.dtype(numpy.float64) and valij_.flags.contiguous:
      _valij_copyarray = False
      _valij_tmp = ctypes.cast(valij_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif valij_ is not None:
      _valij_copyarray = True
      _valij_np_tmp = numpy.zeros(len(valij_),numpy.dtype(numpy.float64))
      _valij_np_tmp[:] = valij_
      assert _valij_np_tmp.flags.contiguous
      _valij_tmp = ctypes.cast(_valij_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _valij_copyarray = False
      _valij_tmp = None
      
    res = __library__.MSK_XX_putaijlist64(self.__nativep,num_,_subi_tmp,_subj_tmp,_valij_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_make_intvector,_make_doublevector)
  @synchronized
  def putacol(self,j_,subj_,valj_):
    """
    Replaces all elements in one column of the linear constraint matrix.
  
    putacol(self,j_,subj_,valj_)
      j: int. Column index.
      subj: array of int. Row indexes of non-zero values in column.
      valj: array of double. New non-zero values of column.
    """
    nzj_ = None
    if nzj_ is None:
      nzj_ = len(subj_)
    elif nzj_ != len(subj_):
      raise IndexError("Inconsistent length of array subj")
    if nzj_ is None:
      nzj_ = len(valj_)
    elif nzj_ != len(valj_):
      raise IndexError("Inconsistent length of array valj")
    if subj_ is None:
      raise ValueError("Argument subj cannot be None")
    if subj_ is None:
      raise ValueError("Argument subj may not be None")
    if isinstance(subj_, numpy.ndarray) and subj_.dtype is numpy.dtype(numpy.int32) and subj_.flags.contiguous:
      _subj_copyarray = False
      _subj_tmp = ctypes.cast(subj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subj_ is not None:
      _subj_copyarray = True
      _subj_np_tmp = numpy.zeros(len(subj_),numpy.dtype(numpy.int32))
      _subj_np_tmp[:] = subj_
      assert _subj_np_tmp.flags.contiguous
      _subj_tmp = ctypes.cast(_subj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subj_copyarray = False
      _subj_tmp = None
      
    if valj_ is None:
      raise ValueError("Argument valj cannot be None")
    if valj_ is None:
      raise ValueError("Argument valj may not be None")
    if isinstance(valj_, numpy.ndarray) and valj_.dtype is numpy.dtype(numpy.float64) and valj_.flags.contiguous:
      _valj_copyarray = False
      _valj_tmp = ctypes.cast(valj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif valj_ is not None:
      _valj_copyarray = True
      _valj_np_tmp = numpy.zeros(len(valj_),numpy.dtype(numpy.float64))
      _valj_np_tmp[:] = valj_
      assert _valj_np_tmp.flags.contiguous
      _valj_tmp = ctypes.cast(_valj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _valj_copyarray = False
      _valj_tmp = None
      
    res = __library__.MSK_XX_putacol(self.__nativep,j_,nzj_,_subj_tmp,_valj_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_make_intvector,_make_doublevector)
  @synchronized
  def putarow(self,i_,subi_,vali_):
    """
    Replaces all elements in one row of the linear constraint matrix.
  
    putarow(self,i_,subi_,vali_)
      i: int. Row index.
      subi: array of int. Column indexes of non-zero values in row.
      vali: array of double. New non-zero values of row.
    """
    nzi_ = None
    if nzi_ is None:
      nzi_ = len(subi_)
    elif nzi_ != len(subi_):
      raise IndexError("Inconsistent length of array subi")
    if nzi_ is None:
      nzi_ = len(vali_)
    elif nzi_ != len(vali_):
      raise IndexError("Inconsistent length of array vali")
    if subi_ is None:
      raise ValueError("Argument subi cannot be None")
    if subi_ is None:
      raise ValueError("Argument subi may not be None")
    if isinstance(subi_, numpy.ndarray) and subi_.dtype is numpy.dtype(numpy.int32) and subi_.flags.contiguous:
      _subi_copyarray = False
      _subi_tmp = ctypes.cast(subi_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subi_ is not None:
      _subi_copyarray = True
      _subi_np_tmp = numpy.zeros(len(subi_),numpy.dtype(numpy.int32))
      _subi_np_tmp[:] = subi_
      assert _subi_np_tmp.flags.contiguous
      _subi_tmp = ctypes.cast(_subi_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subi_copyarray = False
      _subi_tmp = None
      
    if vali_ is None:
      raise ValueError("Argument vali cannot be None")
    if vali_ is None:
      raise ValueError("Argument vali may not be None")
    if isinstance(vali_, numpy.ndarray) and vali_.dtype is numpy.dtype(numpy.float64) and vali_.flags.contiguous:
      _vali_copyarray = False
      _vali_tmp = ctypes.cast(vali_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif vali_ is not None:
      _vali_copyarray = True
      _vali_np_tmp = numpy.zeros(len(vali_),numpy.dtype(numpy.float64))
      _vali_np_tmp[:] = vali_
      assert _vali_np_tmp.flags.contiguous
      _vali_tmp = ctypes.cast(_vali_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _vali_copyarray = False
      _vali_tmp = None
      
    res = __library__.MSK_XX_putarow(self.__nativep,i_,nzi_,_subi_tmp,_vali_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_make_int,_make_longvector,_make_longvector,_make_intvector,_make_doublevector)
  @synchronized
  def putarowslice(self,first_,last_,ptrb_,ptre_,asub_,aval_):
    """
    Replaces all elements in several rows the linear constraint matrix.
  
    putarowslice(self,first_,last_,ptrb_,ptre_,asub_,aval_)
      first: int. First row in the slice.
      last: int. Last row plus one in the slice.
      ptrb: array of long. Array of pointers to the first element in the rows.
      ptre: array of long. Array of pointers to the last element plus one in the rows.
      asub: array of int. Column indexes of new elements.
      aval: array of double. Coefficient values.
    """
    _ptrb_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and ptrb_ is not None and len(ptrb_) != ((last_) - (first_)):
      raise ValueError("Array argument ptrb is not long enough: Is %d, expected %d" % (len(ptrb_),((last_) - (first_))))
    if ptrb_ is None:
      raise ValueError("Argument ptrb cannot be None")
    if ptrb_ is None:
      raise ValueError("Argument ptrb may not be None")
    if isinstance(ptrb_, numpy.ndarray) and ptrb_.dtype is numpy.dtype(numpy.int64) and ptrb_.flags.contiguous:
      _ptrb_copyarray = False
      _ptrb_tmp = ctypes.cast(ptrb_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif ptrb_ is not None:
      _ptrb_copyarray = True
      _ptrb_np_tmp = numpy.zeros(len(ptrb_),numpy.dtype(numpy.int64))
      _ptrb_np_tmp[:] = ptrb_
      assert _ptrb_np_tmp.flags.contiguous
      _ptrb_tmp = ctypes.cast(_ptrb_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _ptrb_copyarray = False
      _ptrb_tmp = None
      
    _ptre_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and ptre_ is not None and len(ptre_) != ((last_) - (first_)):
      raise ValueError("Array argument ptre is not long enough: Is %d, expected %d" % (len(ptre_),((last_) - (first_))))
    if ptre_ is None:
      raise ValueError("Argument ptre cannot be None")
    if ptre_ is None:
      raise ValueError("Argument ptre may not be None")
    if isinstance(ptre_, numpy.ndarray) and ptre_.dtype is numpy.dtype(numpy.int64) and ptre_.flags.contiguous:
      _ptre_copyarray = False
      _ptre_tmp = ctypes.cast(ptre_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif ptre_ is not None:
      _ptre_copyarray = True
      _ptre_np_tmp = numpy.zeros(len(ptre_),numpy.dtype(numpy.int64))
      _ptre_np_tmp[:] = ptre_
      assert _ptre_np_tmp.flags.contiguous
      _ptre_tmp = ctypes.cast(_ptre_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _ptre_copyarray = False
      _ptre_tmp = None
      
    if asub_ is None:
      raise ValueError("Argument asub cannot be None")
    if asub_ is None:
      raise ValueError("Argument asub may not be None")
    if isinstance(asub_, numpy.ndarray) and asub_.dtype is numpy.dtype(numpy.int32) and asub_.flags.contiguous:
      _asub_copyarray = False
      _asub_tmp = ctypes.cast(asub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif asub_ is not None:
      _asub_copyarray = True
      _asub_np_tmp = numpy.zeros(len(asub_),numpy.dtype(numpy.int32))
      _asub_np_tmp[:] = asub_
      assert _asub_np_tmp.flags.contiguous
      _asub_tmp = ctypes.cast(_asub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _asub_copyarray = False
      _asub_tmp = None
      
    if aval_ is None:
      raise ValueError("Argument aval cannot be None")
    if aval_ is None:
      raise ValueError("Argument aval may not be None")
    if isinstance(aval_, numpy.ndarray) and aval_.dtype is numpy.dtype(numpy.float64) and aval_.flags.contiguous:
      _aval_copyarray = False
      _aval_tmp = ctypes.cast(aval_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif aval_ is not None:
      _aval_copyarray = True
      _aval_np_tmp = numpy.zeros(len(aval_),numpy.dtype(numpy.float64))
      _aval_np_tmp[:] = aval_
      assert _aval_np_tmp.flags.contiguous
      _aval_tmp = ctypes.cast(_aval_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _aval_copyarray = False
      _aval_tmp = None
      
    res = __library__.MSK_XX_putarowslice64(self.__nativep,first_,last_,_ptrb_tmp,_ptre_tmp,_asub_tmp,_aval_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_intvector,_make_longvector,_make_longvector,_make_intvector,_make_doublevector)
  @synchronized
  def putarowlist(self,sub_,ptrb_,ptre_,asub_,aval_):
    """
    Replaces all elements in several rows of the linear constraint matrix.
  
    putarowlist(self,sub_,ptrb_,ptre_,asub_,aval_)
      sub: array of int. Indexes of rows or columns that should be replaced.
      ptrb: array of long. Array of pointers to the first element in the rows.
      ptre: array of long. Array of pointers to the last element plus one in the rows.
      asub: array of int. Variable indexes.
      aval: array of double. Coefficient values.
    """
    num_ = None
    if num_ is None:
      num_ = len(sub_)
    elif num_ != len(sub_):
      raise IndexError("Inconsistent length of array sub")
    if num_ is None:
      num_ = len(ptrb_)
    elif num_ != len(ptrb_):
      raise IndexError("Inconsistent length of array ptrb")
    if num_ is None:
      num_ = len(ptre_)
    elif num_ != len(ptre_):
      raise IndexError("Inconsistent length of array ptre")
    if sub_ is None:
      raise ValueError("Argument sub cannot be None")
    if sub_ is None:
      raise ValueError("Argument sub may not be None")
    if isinstance(sub_, numpy.ndarray) and sub_.dtype is numpy.dtype(numpy.int32) and sub_.flags.contiguous:
      _sub_copyarray = False
      _sub_tmp = ctypes.cast(sub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif sub_ is not None:
      _sub_copyarray = True
      _sub_np_tmp = numpy.zeros(len(sub_),numpy.dtype(numpy.int32))
      _sub_np_tmp[:] = sub_
      assert _sub_np_tmp.flags.contiguous
      _sub_tmp = ctypes.cast(_sub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _sub_copyarray = False
      _sub_tmp = None
      
    if ptrb_ is None:
      raise ValueError("Argument ptrb cannot be None")
    if ptrb_ is None:
      raise ValueError("Argument ptrb may not be None")
    if isinstance(ptrb_, numpy.ndarray) and ptrb_.dtype is numpy.dtype(numpy.int64) and ptrb_.flags.contiguous:
      _ptrb_copyarray = False
      _ptrb_tmp = ctypes.cast(ptrb_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif ptrb_ is not None:
      _ptrb_copyarray = True
      _ptrb_np_tmp = numpy.zeros(len(ptrb_),numpy.dtype(numpy.int64))
      _ptrb_np_tmp[:] = ptrb_
      assert _ptrb_np_tmp.flags.contiguous
      _ptrb_tmp = ctypes.cast(_ptrb_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _ptrb_copyarray = False
      _ptrb_tmp = None
      
    if ptre_ is None:
      raise ValueError("Argument ptre cannot be None")
    if ptre_ is None:
      raise ValueError("Argument ptre may not be None")
    if isinstance(ptre_, numpy.ndarray) and ptre_.dtype is numpy.dtype(numpy.int64) and ptre_.flags.contiguous:
      _ptre_copyarray = False
      _ptre_tmp = ctypes.cast(ptre_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif ptre_ is not None:
      _ptre_copyarray = True
      _ptre_np_tmp = numpy.zeros(len(ptre_),numpy.dtype(numpy.int64))
      _ptre_np_tmp[:] = ptre_
      assert _ptre_np_tmp.flags.contiguous
      _ptre_tmp = ctypes.cast(_ptre_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _ptre_copyarray = False
      _ptre_tmp = None
      
    if asub_ is None:
      raise ValueError("Argument asub cannot be None")
    if asub_ is None:
      raise ValueError("Argument asub may not be None")
    if isinstance(asub_, numpy.ndarray) and asub_.dtype is numpy.dtype(numpy.int32) and asub_.flags.contiguous:
      _asub_copyarray = False
      _asub_tmp = ctypes.cast(asub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif asub_ is not None:
      _asub_copyarray = True
      _asub_np_tmp = numpy.zeros(len(asub_),numpy.dtype(numpy.int32))
      _asub_np_tmp[:] = asub_
      assert _asub_np_tmp.flags.contiguous
      _asub_tmp = ctypes.cast(_asub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _asub_copyarray = False
      _asub_tmp = None
      
    if aval_ is None:
      raise ValueError("Argument aval cannot be None")
    if aval_ is None:
      raise ValueError("Argument aval may not be None")
    if isinstance(aval_, numpy.ndarray) and aval_.dtype is numpy.dtype(numpy.float64) and aval_.flags.contiguous:
      _aval_copyarray = False
      _aval_tmp = ctypes.cast(aval_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif aval_ is not None:
      _aval_copyarray = True
      _aval_np_tmp = numpy.zeros(len(aval_),numpy.dtype(numpy.float64))
      _aval_np_tmp[:] = aval_
      assert _aval_np_tmp.flags.contiguous
      _aval_tmp = ctypes.cast(_aval_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _aval_copyarray = False
      _aval_tmp = None
      
    res = __library__.MSK_XX_putarowlist64(self.__nativep,num_,_sub_tmp,_ptrb_tmp,_ptre_tmp,_asub_tmp,_aval_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_make_int,_make_longvector,_make_longvector,_make_intvector,_make_doublevector)
  @synchronized
  def putacolslice(self,first_,last_,ptrb_,ptre_,asub_,aval_):
    """
    Replaces all elements in a sequence of columns the linear constraint matrix.
  
    putacolslice(self,first_,last_,ptrb_,ptre_,asub_,aval_)
      first: int. First column in the slice.
      last: int. Last column plus one in the slice.
      ptrb: array of long. Array of pointers to the first element in the columns.
      ptre: array of long. Array of pointers to the last element plus one in the columns.
      asub: array of int. Row indexes
      aval: array of double. Coefficient values.
    """
    if ptrb_ is None:
      raise ValueError("Argument ptrb cannot be None")
    if ptrb_ is None:
      raise ValueError("Argument ptrb may not be None")
    if isinstance(ptrb_, numpy.ndarray) and ptrb_.dtype is numpy.dtype(numpy.int64) and ptrb_.flags.contiguous:
      _ptrb_copyarray = False
      _ptrb_tmp = ctypes.cast(ptrb_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif ptrb_ is not None:
      _ptrb_copyarray = True
      _ptrb_np_tmp = numpy.zeros(len(ptrb_),numpy.dtype(numpy.int64))
      _ptrb_np_tmp[:] = ptrb_
      assert _ptrb_np_tmp.flags.contiguous
      _ptrb_tmp = ctypes.cast(_ptrb_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _ptrb_copyarray = False
      _ptrb_tmp = None
      
    if ptre_ is None:
      raise ValueError("Argument ptre cannot be None")
    if ptre_ is None:
      raise ValueError("Argument ptre may not be None")
    if isinstance(ptre_, numpy.ndarray) and ptre_.dtype is numpy.dtype(numpy.int64) and ptre_.flags.contiguous:
      _ptre_copyarray = False
      _ptre_tmp = ctypes.cast(ptre_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif ptre_ is not None:
      _ptre_copyarray = True
      _ptre_np_tmp = numpy.zeros(len(ptre_),numpy.dtype(numpy.int64))
      _ptre_np_tmp[:] = ptre_
      assert _ptre_np_tmp.flags.contiguous
      _ptre_tmp = ctypes.cast(_ptre_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _ptre_copyarray = False
      _ptre_tmp = None
      
    if asub_ is None:
      raise ValueError("Argument asub cannot be None")
    if asub_ is None:
      raise ValueError("Argument asub may not be None")
    if isinstance(asub_, numpy.ndarray) and asub_.dtype is numpy.dtype(numpy.int32) and asub_.flags.contiguous:
      _asub_copyarray = False
      _asub_tmp = ctypes.cast(asub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif asub_ is not None:
      _asub_copyarray = True
      _asub_np_tmp = numpy.zeros(len(asub_),numpy.dtype(numpy.int32))
      _asub_np_tmp[:] = asub_
      assert _asub_np_tmp.flags.contiguous
      _asub_tmp = ctypes.cast(_asub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _asub_copyarray = False
      _asub_tmp = None
      
    if aval_ is None:
      raise ValueError("Argument aval cannot be None")
    if aval_ is None:
      raise ValueError("Argument aval may not be None")
    if isinstance(aval_, numpy.ndarray) and aval_.dtype is numpy.dtype(numpy.float64) and aval_.flags.contiguous:
      _aval_copyarray = False
      _aval_tmp = ctypes.cast(aval_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif aval_ is not None:
      _aval_copyarray = True
      _aval_np_tmp = numpy.zeros(len(aval_),numpy.dtype(numpy.float64))
      _aval_np_tmp[:] = aval_
      assert _aval_np_tmp.flags.contiguous
      _aval_tmp = ctypes.cast(_aval_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _aval_copyarray = False
      _aval_tmp = None
      
    res = __library__.MSK_XX_putacolslice64(self.__nativep,first_,last_,_ptrb_tmp,_ptre_tmp,_asub_tmp,_aval_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_intvector,_make_longvector,_make_longvector,_make_intvector,_make_doublevector)
  @synchronized
  def putacollist(self,sub_,ptrb_,ptre_,asub_,aval_):
    """
    Replaces all elements in several columns the linear constraint matrix.
  
    putacollist(self,sub_,ptrb_,ptre_,asub_,aval_)
      sub: array of int. Indexes of columns that should be replaced.
      ptrb: array of long. Array of pointers to the first element in the columns.
      ptre: array of long. Array of pointers to the last element plus one in the columns.
      asub: array of int. Row indexes
      aval: array of double. Coefficient values.
    """
    num_ = None
    if num_ is None:
      num_ = len(sub_)
    elif num_ != len(sub_):
      raise IndexError("Inconsistent length of array sub")
    if num_ is None:
      num_ = len(ptrb_)
    elif num_ != len(ptrb_):
      raise IndexError("Inconsistent length of array ptrb")
    if num_ is None:
      num_ = len(ptre_)
    elif num_ != len(ptre_):
      raise IndexError("Inconsistent length of array ptre")
    if sub_ is None:
      raise ValueError("Argument sub cannot be None")
    if sub_ is None:
      raise ValueError("Argument sub may not be None")
    if isinstance(sub_, numpy.ndarray) and sub_.dtype is numpy.dtype(numpy.int32) and sub_.flags.contiguous:
      _sub_copyarray = False
      _sub_tmp = ctypes.cast(sub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif sub_ is not None:
      _sub_copyarray = True
      _sub_np_tmp = numpy.zeros(len(sub_),numpy.dtype(numpy.int32))
      _sub_np_tmp[:] = sub_
      assert _sub_np_tmp.flags.contiguous
      _sub_tmp = ctypes.cast(_sub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _sub_copyarray = False
      _sub_tmp = None
      
    if ptrb_ is None:
      raise ValueError("Argument ptrb cannot be None")
    if ptrb_ is None:
      raise ValueError("Argument ptrb may not be None")
    if isinstance(ptrb_, numpy.ndarray) and ptrb_.dtype is numpy.dtype(numpy.int64) and ptrb_.flags.contiguous:
      _ptrb_copyarray = False
      _ptrb_tmp = ctypes.cast(ptrb_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif ptrb_ is not None:
      _ptrb_copyarray = True
      _ptrb_np_tmp = numpy.zeros(len(ptrb_),numpy.dtype(numpy.int64))
      _ptrb_np_tmp[:] = ptrb_
      assert _ptrb_np_tmp.flags.contiguous
      _ptrb_tmp = ctypes.cast(_ptrb_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _ptrb_copyarray = False
      _ptrb_tmp = None
      
    if ptre_ is None:
      raise ValueError("Argument ptre cannot be None")
    if ptre_ is None:
      raise ValueError("Argument ptre may not be None")
    if isinstance(ptre_, numpy.ndarray) and ptre_.dtype is numpy.dtype(numpy.int64) and ptre_.flags.contiguous:
      _ptre_copyarray = False
      _ptre_tmp = ctypes.cast(ptre_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif ptre_ is not None:
      _ptre_copyarray = True
      _ptre_np_tmp = numpy.zeros(len(ptre_),numpy.dtype(numpy.int64))
      _ptre_np_tmp[:] = ptre_
      assert _ptre_np_tmp.flags.contiguous
      _ptre_tmp = ctypes.cast(_ptre_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _ptre_copyarray = False
      _ptre_tmp = None
      
    if asub_ is None:
      raise ValueError("Argument asub cannot be None")
    if asub_ is None:
      raise ValueError("Argument asub may not be None")
    if isinstance(asub_, numpy.ndarray) and asub_.dtype is numpy.dtype(numpy.int32) and asub_.flags.contiguous:
      _asub_copyarray = False
      _asub_tmp = ctypes.cast(asub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif asub_ is not None:
      _asub_copyarray = True
      _asub_np_tmp = numpy.zeros(len(asub_),numpy.dtype(numpy.int32))
      _asub_np_tmp[:] = asub_
      assert _asub_np_tmp.flags.contiguous
      _asub_tmp = ctypes.cast(_asub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _asub_copyarray = False
      _asub_tmp = None
      
    if aval_ is None:
      raise ValueError("Argument aval cannot be None")
    if aval_ is None:
      raise ValueError("Argument aval may not be None")
    if isinstance(aval_, numpy.ndarray) and aval_.dtype is numpy.dtype(numpy.float64) and aval_.flags.contiguous:
      _aval_copyarray = False
      _aval_tmp = ctypes.cast(aval_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif aval_ is not None:
      _aval_copyarray = True
      _aval_np_tmp = numpy.zeros(len(aval_),numpy.dtype(numpy.float64))
      _aval_np_tmp[:] = aval_
      assert _aval_np_tmp.flags.contiguous
      _aval_tmp = ctypes.cast(_aval_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _aval_copyarray = False
      _aval_tmp = None
      
    res = __library__.MSK_XX_putacollist64(self.__nativep,num_,_sub_tmp,_ptrb_tmp,_ptre_tmp,_asub_tmp,_aval_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_make_int,_make_longvector,_make_doublevector)
  @synchronized
  def putbaraij(self,i_,j_,sub_,weights_):
    """
    Inputs an element of barA.
  
    putbaraij(self,i_,j_,sub_,weights_)
      i: int. Row index of barA.
      j: int. Column index of barA.
      sub: array of long. Element indexes in matrix storage.
      weights: array of double. Weights in the weighted sum.
    """
    num_ = None
    if num_ is None:
      num_ = len(sub_)
    elif num_ != len(sub_):
      raise IndexError("Inconsistent length of array sub")
    if num_ is None:
      num_ = len(weights_)
    elif num_ != len(weights_):
      raise IndexError("Inconsistent length of array weights")
    if sub_ is None:
      raise ValueError("Argument sub cannot be None")
    if sub_ is None:
      raise ValueError("Argument sub may not be None")
    if isinstance(sub_, numpy.ndarray) and sub_.dtype is numpy.dtype(numpy.int64) and sub_.flags.contiguous:
      _sub_copyarray = False
      _sub_tmp = ctypes.cast(sub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif sub_ is not None:
      _sub_copyarray = True
      _sub_np_tmp = numpy.zeros(len(sub_),numpy.dtype(numpy.int64))
      _sub_np_tmp[:] = sub_
      assert _sub_np_tmp.flags.contiguous
      _sub_tmp = ctypes.cast(_sub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _sub_copyarray = False
      _sub_tmp = None
      
    if weights_ is None:
      raise ValueError("Argument weights cannot be None")
    if weights_ is None:
      raise ValueError("Argument weights may not be None")
    if isinstance(weights_, numpy.ndarray) and weights_.dtype is numpy.dtype(numpy.float64) and weights_.flags.contiguous:
      _weights_copyarray = False
      _weights_tmp = ctypes.cast(weights_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif weights_ is not None:
      _weights_copyarray = True
      _weights_np_tmp = numpy.zeros(len(weights_),numpy.dtype(numpy.float64))
      _weights_np_tmp[:] = weights_
      assert _weights_np_tmp.flags.contiguous
      _weights_tmp = ctypes.cast(_weights_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _weights_copyarray = False
      _weights_tmp = None
      
    res = __library__.MSK_XX_putbaraij(self.__nativep,i_,j_,num_,_sub_tmp,_weights_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_intvector,_make_intvector,_make_longvector,_make_longvector,_make_longvector,_make_doublevector)
  @synchronized
  def putbaraijlist(self,subi_,subj_,alphaptrb_,alphaptre_,matidx_,weights_):
    """
    Inputs list of elements of barA.
  
    putbaraijlist(self,subi_,subj_,alphaptrb_,alphaptre_,matidx_,weights_)
      subi: array of int. Row index of barA.
      subj: array of int. Column index of barA.
      alphaptrb: array of long. Start entries for terms in the weighted sum.
      alphaptre: array of long. End entries for terms in the weighted sum.
      matidx: array of long. Element indexes in matrix storage.
      weights: array of double. Weights in the weighted sum.
    """
    num_ = None
    if num_ is None:
      num_ = len(subi_)
    elif num_ != len(subi_):
      raise IndexError("Inconsistent length of array subi")
    if num_ is None:
      num_ = len(subj_)
    elif num_ != len(subj_):
      raise IndexError("Inconsistent length of array subj")
    if num_ is None:
      num_ = len(alphaptrb_)
    elif num_ != len(alphaptrb_):
      raise IndexError("Inconsistent length of array alphaptrb")
    if num_ is None:
      num_ = len(alphaptre_)
    elif num_ != len(alphaptre_):
      raise IndexError("Inconsistent length of array alphaptre")
    if subi_ is None:
      raise ValueError("Argument subi cannot be None")
    if subi_ is None:
      raise ValueError("Argument subi may not be None")
    if isinstance(subi_, numpy.ndarray) and subi_.dtype is numpy.dtype(numpy.int32) and subi_.flags.contiguous:
      _subi_copyarray = False
      _subi_tmp = ctypes.cast(subi_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subi_ is not None:
      _subi_copyarray = True
      _subi_np_tmp = numpy.zeros(len(subi_),numpy.dtype(numpy.int32))
      _subi_np_tmp[:] = subi_
      assert _subi_np_tmp.flags.contiguous
      _subi_tmp = ctypes.cast(_subi_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subi_copyarray = False
      _subi_tmp = None
      
    if subj_ is None:
      raise ValueError("Argument subj cannot be None")
    if subj_ is None:
      raise ValueError("Argument subj may not be None")
    if isinstance(subj_, numpy.ndarray) and subj_.dtype is numpy.dtype(numpy.int32) and subj_.flags.contiguous:
      _subj_copyarray = False
      _subj_tmp = ctypes.cast(subj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subj_ is not None:
      _subj_copyarray = True
      _subj_np_tmp = numpy.zeros(len(subj_),numpy.dtype(numpy.int32))
      _subj_np_tmp[:] = subj_
      assert _subj_np_tmp.flags.contiguous
      _subj_tmp = ctypes.cast(_subj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subj_copyarray = False
      _subj_tmp = None
      
    if alphaptrb_ is None:
      raise ValueError("Argument alphaptrb cannot be None")
    if alphaptrb_ is None:
      raise ValueError("Argument alphaptrb may not be None")
    if isinstance(alphaptrb_, numpy.ndarray) and alphaptrb_.dtype is numpy.dtype(numpy.int64) and alphaptrb_.flags.contiguous:
      _alphaptrb_copyarray = False
      _alphaptrb_tmp = ctypes.cast(alphaptrb_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif alphaptrb_ is not None:
      _alphaptrb_copyarray = True
      _alphaptrb_np_tmp = numpy.zeros(len(alphaptrb_),numpy.dtype(numpy.int64))
      _alphaptrb_np_tmp[:] = alphaptrb_
      assert _alphaptrb_np_tmp.flags.contiguous
      _alphaptrb_tmp = ctypes.cast(_alphaptrb_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _alphaptrb_copyarray = False
      _alphaptrb_tmp = None
      
    if alphaptre_ is None:
      raise ValueError("Argument alphaptre cannot be None")
    if alphaptre_ is None:
      raise ValueError("Argument alphaptre may not be None")
    if isinstance(alphaptre_, numpy.ndarray) and alphaptre_.dtype is numpy.dtype(numpy.int64) and alphaptre_.flags.contiguous:
      _alphaptre_copyarray = False
      _alphaptre_tmp = ctypes.cast(alphaptre_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif alphaptre_ is not None:
      _alphaptre_copyarray = True
      _alphaptre_np_tmp = numpy.zeros(len(alphaptre_),numpy.dtype(numpy.int64))
      _alphaptre_np_tmp[:] = alphaptre_
      assert _alphaptre_np_tmp.flags.contiguous
      _alphaptre_tmp = ctypes.cast(_alphaptre_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _alphaptre_copyarray = False
      _alphaptre_tmp = None
      
    if matidx_ is None:
      raise ValueError("Argument matidx cannot be None")
    if matidx_ is None:
      raise ValueError("Argument matidx may not be None")
    if isinstance(matidx_, numpy.ndarray) and matidx_.dtype is numpy.dtype(numpy.int64) and matidx_.flags.contiguous:
      _matidx_copyarray = False
      _matidx_tmp = ctypes.cast(matidx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif matidx_ is not None:
      _matidx_copyarray = True
      _matidx_np_tmp = numpy.zeros(len(matidx_),numpy.dtype(numpy.int64))
      _matidx_np_tmp[:] = matidx_
      assert _matidx_np_tmp.flags.contiguous
      _matidx_tmp = ctypes.cast(_matidx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _matidx_copyarray = False
      _matidx_tmp = None
      
    if weights_ is None:
      raise ValueError("Argument weights cannot be None")
    if weights_ is None:
      raise ValueError("Argument weights may not be None")
    if isinstance(weights_, numpy.ndarray) and weights_.dtype is numpy.dtype(numpy.float64) and weights_.flags.contiguous:
      _weights_copyarray = False
      _weights_tmp = ctypes.cast(weights_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif weights_ is not None:
      _weights_copyarray = True
      _weights_np_tmp = numpy.zeros(len(weights_),numpy.dtype(numpy.float64))
      _weights_np_tmp[:] = weights_
      assert _weights_np_tmp.flags.contiguous
      _weights_tmp = ctypes.cast(_weights_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _weights_copyarray = False
      _weights_tmp = None
      
    res = __library__.MSK_XX_putbaraijlist(self.__nativep,num_,_subi_tmp,_subj_tmp,_alphaptrb_tmp,_alphaptre_tmp,_matidx_tmp,_weights_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_intvector,_make_longvector,_make_longvector,_make_intvector,_make_longvector,_make_longvector,_make_doublevector)
  @synchronized
  def putbararowlist(self,subi_,ptrb_,ptre_,subj_,nummat_,matidx_,weights_):
    """
    Replace a set of rows of barA
  
    putbararowlist(self,subi_,ptrb_,ptre_,subj_,nummat_,matidx_,weights_)
      subi: array of int. Row indexes of barA.
      ptrb: array of long. Start of rows in barA.
      ptre: array of long. End of rows in barA.
      subj: array of int. Column index of barA.
      nummat: array of long. Number of entries in weighted sum of matrixes.
      matidx: array of long. Matrix indexes for weighted sum of matrixes.
      weights: array of double. Weights for weighted sum of matrixes.
    """
    num_ = None
    if num_ is None:
      num_ = len(subi_)
    elif num_ != len(subi_):
      raise IndexError("Inconsistent length of array subi")
    if num_ is None:
      num_ = len(ptrb_)
    elif num_ != len(ptrb_):
      raise IndexError("Inconsistent length of array ptrb")
    if num_ is None:
      num_ = len(ptre_)
    elif num_ != len(ptre_):
      raise IndexError("Inconsistent length of array ptre")
    if subi_ is None:
      raise ValueError("Argument subi cannot be None")
    if subi_ is None:
      raise ValueError("Argument subi may not be None")
    if isinstance(subi_, numpy.ndarray) and subi_.dtype is numpy.dtype(numpy.int32) and subi_.flags.contiguous:
      _subi_copyarray = False
      _subi_tmp = ctypes.cast(subi_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subi_ is not None:
      _subi_copyarray = True
      _subi_np_tmp = numpy.zeros(len(subi_),numpy.dtype(numpy.int32))
      _subi_np_tmp[:] = subi_
      assert _subi_np_tmp.flags.contiguous
      _subi_tmp = ctypes.cast(_subi_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subi_copyarray = False
      _subi_tmp = None
      
    if ptrb_ is None:
      raise ValueError("Argument ptrb cannot be None")
    if ptrb_ is None:
      raise ValueError("Argument ptrb may not be None")
    if isinstance(ptrb_, numpy.ndarray) and ptrb_.dtype is numpy.dtype(numpy.int64) and ptrb_.flags.contiguous:
      _ptrb_copyarray = False
      _ptrb_tmp = ctypes.cast(ptrb_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif ptrb_ is not None:
      _ptrb_copyarray = True
      _ptrb_np_tmp = numpy.zeros(len(ptrb_),numpy.dtype(numpy.int64))
      _ptrb_np_tmp[:] = ptrb_
      assert _ptrb_np_tmp.flags.contiguous
      _ptrb_tmp = ctypes.cast(_ptrb_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _ptrb_copyarray = False
      _ptrb_tmp = None
      
    if ptre_ is None:
      raise ValueError("Argument ptre cannot be None")
    if ptre_ is None:
      raise ValueError("Argument ptre may not be None")
    if isinstance(ptre_, numpy.ndarray) and ptre_.dtype is numpy.dtype(numpy.int64) and ptre_.flags.contiguous:
      _ptre_copyarray = False
      _ptre_tmp = ctypes.cast(ptre_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif ptre_ is not None:
      _ptre_copyarray = True
      _ptre_np_tmp = numpy.zeros(len(ptre_),numpy.dtype(numpy.int64))
      _ptre_np_tmp[:] = ptre_
      assert _ptre_np_tmp.flags.contiguous
      _ptre_tmp = ctypes.cast(_ptre_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _ptre_copyarray = False
      _ptre_tmp = None
      
    if subj_ is None:
      raise ValueError("Argument subj cannot be None")
    if subj_ is None:
      raise ValueError("Argument subj may not be None")
    if isinstance(subj_, numpy.ndarray) and subj_.dtype is numpy.dtype(numpy.int32) and subj_.flags.contiguous:
      _subj_copyarray = False
      _subj_tmp = ctypes.cast(subj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subj_ is not None:
      _subj_copyarray = True
      _subj_np_tmp = numpy.zeros(len(subj_),numpy.dtype(numpy.int32))
      _subj_np_tmp[:] = subj_
      assert _subj_np_tmp.flags.contiguous
      _subj_tmp = ctypes.cast(_subj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subj_copyarray = False
      _subj_tmp = None
      
    _nummat_minlength = len((subj_))
    if len((subj_)) > 0 and nummat_ is not None and len(nummat_) != len((subj_)):
      raise ValueError("Array argument nummat is not long enough: Is %d, expected %d" % (len(nummat_),len((subj_))))
    if nummat_ is None:
      raise ValueError("Argument nummat cannot be None")
    if nummat_ is None:
      raise ValueError("Argument nummat may not be None")
    if isinstance(nummat_, numpy.ndarray) and nummat_.dtype is numpy.dtype(numpy.int64) and nummat_.flags.contiguous:
      _nummat_copyarray = False
      _nummat_tmp = ctypes.cast(nummat_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif nummat_ is not None:
      _nummat_copyarray = True
      _nummat_np_tmp = numpy.zeros(len(nummat_),numpy.dtype(numpy.int64))
      _nummat_np_tmp[:] = nummat_
      assert _nummat_np_tmp.flags.contiguous
      _nummat_tmp = ctypes.cast(_nummat_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _nummat_copyarray = False
      _nummat_tmp = None
      
    _matidx_minlength = sum((nummat_))
    if sum((nummat_)) > 0 and matidx_ is not None and len(matidx_) != sum((nummat_)):
      raise ValueError("Array argument matidx is not long enough: Is %d, expected %d" % (len(matidx_),sum((nummat_))))
    if matidx_ is None:
      raise ValueError("Argument matidx cannot be None")
    if matidx_ is None:
      raise ValueError("Argument matidx may not be None")
    if isinstance(matidx_, numpy.ndarray) and matidx_.dtype is numpy.dtype(numpy.int64) and matidx_.flags.contiguous:
      _matidx_copyarray = False
      _matidx_tmp = ctypes.cast(matidx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif matidx_ is not None:
      _matidx_copyarray = True
      _matidx_np_tmp = numpy.zeros(len(matidx_),numpy.dtype(numpy.int64))
      _matidx_np_tmp[:] = matidx_
      assert _matidx_np_tmp.flags.contiguous
      _matidx_tmp = ctypes.cast(_matidx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _matidx_copyarray = False
      _matidx_tmp = None
      
    _weights_minlength = sum((nummat_))
    if sum((nummat_)) > 0 and weights_ is not None and len(weights_) != sum((nummat_)):
      raise ValueError("Array argument weights is not long enough: Is %d, expected %d" % (len(weights_),sum((nummat_))))
    if weights_ is None:
      raise ValueError("Argument weights cannot be None")
    if weights_ is None:
      raise ValueError("Argument weights may not be None")
    if isinstance(weights_, numpy.ndarray) and weights_.dtype is numpy.dtype(numpy.float64) and weights_.flags.contiguous:
      _weights_copyarray = False
      _weights_tmp = ctypes.cast(weights_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif weights_ is not None:
      _weights_copyarray = True
      _weights_np_tmp = numpy.zeros(len(weights_),numpy.dtype(numpy.float64))
      _weights_np_tmp[:] = weights_
      assert _weights_np_tmp.flags.contiguous
      _weights_tmp = ctypes.cast(_weights_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _weights_copyarray = False
      _weights_tmp = None
      
    res = __library__.MSK_XX_putbararowlist(self.__nativep,num_,_subi_tmp,_ptrb_tmp,_ptre_tmp,_subj_tmp,_nummat_tmp,_matidx_tmp,_weights_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any)
  @synchronized
  def getnumbarcnz(self):
    """
    Obtains the number of nonzero elements in barc.
  
    getnumbarcnz(self)
    returns: nz
      nz: long. The number of nonzero elements in barc.
    """
    nz_ = ctypes.c_int64()
    res = __library__.MSK_XX_getnumbarcnz(self.__nativep,ctypes.byref(nz_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    nz_ = nz_.value
    _nz_return_value = nz_
    return (_nz_return_value)
  @accepts(_accept_any)
  @synchronized
  def getnumbaranz(self):
    """
    Get the number of nonzero elements in barA.
  
    getnumbaranz(self)
    returns: nz
      nz: long. The number of nonzero block elements in barA.
    """
    nz_ = ctypes.c_int64()
    res = __library__.MSK_XX_getnumbaranz(self.__nativep,ctypes.byref(nz_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    nz_ = nz_.value
    _nz_return_value = nz_
    return (_nz_return_value)
  @accepts(_accept_any,_accept_longvector)
  @synchronized
  def getbarcsparsity(self,idxj_):
    """
    Get the positions of the nonzero elements in barc.
  
    getbarcsparsity(self,idxj_)
      idxj: array of long. Internal positions of the nonzeros elements in barc.
    returns: numnz
      numnz: long. Number of nonzero elements in barc.
    """
    maxnumnz_ = self.getnumbarcnz()
    numnz_ = ctypes.c_int64()
    _idxj_minlength = (maxnumnz_)
    if (maxnumnz_) > 0 and idxj_ is not None and len(idxj_) != (maxnumnz_):
      raise ValueError("Array argument idxj is not long enough: Is %d, expected %d" % (len(idxj_),(maxnumnz_)))
    if isinstance(idxj_,numpy.ndarray) and not idxj_.flags.writeable:
      raise ValueError("Argument idxj must be writable")
    if idxj_ is None:
      raise ValueError("Argument idxj may not be None")
    if isinstance(idxj_, numpy.ndarray) and idxj_.dtype is numpy.dtype(numpy.int64) and idxj_.flags.contiguous:
      _idxj_copyarray = False
      _idxj_tmp = ctypes.cast(idxj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif idxj_ is not None:
      _idxj_copyarray = True
      _idxj_np_tmp = numpy.zeros(len(idxj_),numpy.dtype(numpy.int64))
      _idxj_np_tmp[:] = idxj_
      assert _idxj_np_tmp.flags.contiguous
      _idxj_tmp = ctypes.cast(_idxj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _idxj_copyarray = False
      _idxj_tmp = None
      
    res = __library__.MSK_XX_getbarcsparsity(self.__nativep,maxnumnz_,ctypes.byref(numnz_),_idxj_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    numnz_ = numnz_.value
    _numnz_return_value = numnz_
    if _idxj_copyarray:
      idxj_[:] = _idxj_np_tmp
    return (_numnz_return_value)
  @accepts(_accept_any,_accept_longvector)
  @synchronized
  def getbarasparsity(self,idxij_):
    """
    Obtains the sparsity pattern of the barA matrix.
  
    getbarasparsity(self,idxij_)
      idxij: array of long. Position of each nonzero element in the vector representation of barA.
    returns: numnz
      numnz: long. Number of nonzero elements in barA.
    """
    maxnumnz_ = self.getnumbaranz()
    numnz_ = ctypes.c_int64()
    _idxij_minlength = (maxnumnz_)
    if (maxnumnz_) > 0 and idxij_ is not None and len(idxij_) != (maxnumnz_):
      raise ValueError("Array argument idxij is not long enough: Is %d, expected %d" % (len(idxij_),(maxnumnz_)))
    if isinstance(idxij_,numpy.ndarray) and not idxij_.flags.writeable:
      raise ValueError("Argument idxij must be writable")
    if idxij_ is None:
      raise ValueError("Argument idxij may not be None")
    if isinstance(idxij_, numpy.ndarray) and idxij_.dtype is numpy.dtype(numpy.int64) and idxij_.flags.contiguous:
      _idxij_copyarray = False
      _idxij_tmp = ctypes.cast(idxij_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif idxij_ is not None:
      _idxij_copyarray = True
      _idxij_np_tmp = numpy.zeros(len(idxij_),numpy.dtype(numpy.int64))
      _idxij_np_tmp[:] = idxij_
      assert _idxij_np_tmp.flags.contiguous
      _idxij_tmp = ctypes.cast(_idxij_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _idxij_copyarray = False
      _idxij_tmp = None
      
    res = __library__.MSK_XX_getbarasparsity(self.__nativep,maxnumnz_,ctypes.byref(numnz_),_idxij_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    numnz_ = numnz_.value
    _numnz_return_value = numnz_
    if _idxij_copyarray:
      idxij_[:] = _idxij_np_tmp
    return (_numnz_return_value)
  @accepts(_accept_any,_make_long)
  @synchronized
  def getbarcidxinfo(self,idx_):
    """
    Obtains information about an element in barc.
  
    getbarcidxinfo(self,idx_)
      idx: long. Index of the element for which information should be obtained. The value is an index of a symmetric sparse variable.
    returns: num
      num: long. Number of terms that appear in the weighted sum that forms the requested element.
    """
    num_ = ctypes.c_int64()
    res = __library__.MSK_XX_getbarcidxinfo(self.__nativep,idx_,ctypes.byref(num_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    num_ = num_.value
    _num_return_value = num_
    return (_num_return_value)
  @accepts(_accept_any,_make_long)
  @synchronized
  def getbarcidxj(self,idx_):
    """
    Obtains the row index of an element in barc.
  
    getbarcidxj(self,idx_)
      idx: long. Index of the element for which information should be obtained.
    returns: j
      j: int. Row index in barc.
    """
    j_ = ctypes.c_int32()
    res = __library__.MSK_XX_getbarcidxj(self.__nativep,idx_,ctypes.byref(j_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    j_ = j_.value
    _j_return_value = j_
    return (_j_return_value)
  @accepts(_accept_any,_make_long,_accept_longvector,_accept_doublevector)
  @synchronized
  def getbarcidx(self,idx_,sub_,weights_):
    """
    Obtains information about an element in barc.
  
    getbarcidx(self,idx_,sub_,weights_)
      idx: long. Index of the element for which information should be obtained.
      sub: array of long. Elements appearing the weighted sum.
      weights: array of double. Weights of terms in the weighted sum.
    returns: j,num
      j: int. Row index in barc.
      num: long. Number of terms in the weighted sum.
    """
    maxnum_ = self.getbarcidxinfo((idx_))
    j_ = ctypes.c_int32()
    num_ = ctypes.c_int64()
    _sub_minlength = (maxnum_)
    if (maxnum_) > 0 and sub_ is not None and len(sub_) != (maxnum_):
      raise ValueError("Array argument sub is not long enough: Is %d, expected %d" % (len(sub_),(maxnum_)))
    if isinstance(sub_,numpy.ndarray) and not sub_.flags.writeable:
      raise ValueError("Argument sub must be writable")
    if sub_ is None:
      raise ValueError("Argument sub may not be None")
    if isinstance(sub_, numpy.ndarray) and sub_.dtype is numpy.dtype(numpy.int64) and sub_.flags.contiguous:
      _sub_copyarray = False
      _sub_tmp = ctypes.cast(sub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif sub_ is not None:
      _sub_copyarray = True
      _sub_np_tmp = numpy.zeros(len(sub_),numpy.dtype(numpy.int64))
      _sub_np_tmp[:] = sub_
      assert _sub_np_tmp.flags.contiguous
      _sub_tmp = ctypes.cast(_sub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _sub_copyarray = False
      _sub_tmp = None
      
    _weights_minlength = (maxnum_)
    if (maxnum_) > 0 and weights_ is not None and len(weights_) != (maxnum_):
      raise ValueError("Array argument weights is not long enough: Is %d, expected %d" % (len(weights_),(maxnum_)))
    if isinstance(weights_,numpy.ndarray) and not weights_.flags.writeable:
      raise ValueError("Argument weights must be writable")
    if weights_ is None:
      raise ValueError("Argument weights may not be None")
    if isinstance(weights_, numpy.ndarray) and weights_.dtype is numpy.dtype(numpy.float64) and weights_.flags.contiguous:
      _weights_copyarray = False
      _weights_tmp = ctypes.cast(weights_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif weights_ is not None:
      _weights_copyarray = True
      _weights_np_tmp = numpy.zeros(len(weights_),numpy.dtype(numpy.float64))
      _weights_np_tmp[:] = weights_
      assert _weights_np_tmp.flags.contiguous
      _weights_tmp = ctypes.cast(_weights_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _weights_copyarray = False
      _weights_tmp = None
      
    res = __library__.MSK_XX_getbarcidx(self.__nativep,idx_,maxnum_,ctypes.byref(j_),ctypes.byref(num_),_sub_tmp,_weights_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    j_ = j_.value
    _j_return_value = j_
    num_ = num_.value
    _num_return_value = num_
    if _sub_copyarray:
      sub_[:] = _sub_np_tmp
    if _weights_copyarray:
      weights_[:] = _weights_np_tmp
    return (_j_return_value,_num_return_value)
  @accepts(_accept_any,_make_long)
  @synchronized
  def getbaraidxinfo(self,idx_):
    """
    Obtains the number of terms in the weighted sum that form a particular element in barA.
  
    getbaraidxinfo(self,idx_)
      idx: long. The internal position of the element for which information should be obtained.
    returns: num
      num: long. Number of terms in the weighted sum that form the specified element in barA.
    """
    num_ = ctypes.c_int64()
    res = __library__.MSK_XX_getbaraidxinfo(self.__nativep,idx_,ctypes.byref(num_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    num_ = num_.value
    _num_return_value = num_
    return (_num_return_value)
  @accepts(_accept_any,_make_long)
  @synchronized
  def getbaraidxij(self,idx_):
    """
    Obtains information about an element in barA.
  
    getbaraidxij(self,idx_)
      idx: long. Position of the element in the vectorized form.
    returns: i,j
      i: int. Row index of the element at position idx.
      j: int. Column index of the element at position idx.
    """
    i_ = ctypes.c_int32()
    j_ = ctypes.c_int32()
    res = __library__.MSK_XX_getbaraidxij(self.__nativep,idx_,ctypes.byref(i_),ctypes.byref(j_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    i_ = i_.value
    _i_return_value = i_
    j_ = j_.value
    _j_return_value = j_
    return (_i_return_value,_j_return_value)
  @accepts(_accept_any,_make_long,_accept_longvector,_accept_doublevector)
  @synchronized
  def getbaraidx(self,idx_,sub_,weights_):
    """
    Obtains information about an element in barA.
  
    getbaraidx(self,idx_,sub_,weights_)
      idx: long. Position of the element in the vectorized form.
      sub: array of long. A list indexes of the elements from symmetric matrix storage that appear in the weighted sum.
      weights: array of double. The weights associated with each term in the weighted sum.
    returns: i,j,num
      i: int. Row index of the element at position idx.
      j: int. Column index of the element at position idx.
      num: long. Number of terms in weighted sum that forms the element.
    """
    maxnum_ = self.getbaraidxinfo((idx_))
    i_ = ctypes.c_int32()
    j_ = ctypes.c_int32()
    num_ = ctypes.c_int64()
    _sub_minlength = (maxnum_)
    if (maxnum_) > 0 and sub_ is not None and len(sub_) != (maxnum_):
      raise ValueError("Array argument sub is not long enough: Is %d, expected %d" % (len(sub_),(maxnum_)))
    if isinstance(sub_,numpy.ndarray) and not sub_.flags.writeable:
      raise ValueError("Argument sub must be writable")
    if sub_ is None:
      raise ValueError("Argument sub may not be None")
    if isinstance(sub_, numpy.ndarray) and sub_.dtype is numpy.dtype(numpy.int64) and sub_.flags.contiguous:
      _sub_copyarray = False
      _sub_tmp = ctypes.cast(sub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif sub_ is not None:
      _sub_copyarray = True
      _sub_np_tmp = numpy.zeros(len(sub_),numpy.dtype(numpy.int64))
      _sub_np_tmp[:] = sub_
      assert _sub_np_tmp.flags.contiguous
      _sub_tmp = ctypes.cast(_sub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _sub_copyarray = False
      _sub_tmp = None
      
    _weights_minlength = (maxnum_)
    if (maxnum_) > 0 and weights_ is not None and len(weights_) != (maxnum_):
      raise ValueError("Array argument weights is not long enough: Is %d, expected %d" % (len(weights_),(maxnum_)))
    if isinstance(weights_,numpy.ndarray) and not weights_.flags.writeable:
      raise ValueError("Argument weights must be writable")
    if weights_ is None:
      raise ValueError("Argument weights may not be None")
    if isinstance(weights_, numpy.ndarray) and weights_.dtype is numpy.dtype(numpy.float64) and weights_.flags.contiguous:
      _weights_copyarray = False
      _weights_tmp = ctypes.cast(weights_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif weights_ is not None:
      _weights_copyarray = True
      _weights_np_tmp = numpy.zeros(len(weights_),numpy.dtype(numpy.float64))
      _weights_np_tmp[:] = weights_
      assert _weights_np_tmp.flags.contiguous
      _weights_tmp = ctypes.cast(_weights_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _weights_copyarray = False
      _weights_tmp = None
      
    res = __library__.MSK_XX_getbaraidx(self.__nativep,idx_,maxnum_,ctypes.byref(i_),ctypes.byref(j_),ctypes.byref(num_),_sub_tmp,_weights_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    i_ = i_.value
    _i_return_value = i_
    j_ = j_.value
    _j_return_value = j_
    num_ = num_.value
    _num_return_value = num_
    if _sub_copyarray:
      sub_[:] = _sub_np_tmp
    if _weights_copyarray:
      weights_[:] = _weights_np_tmp
    return (_i_return_value,_j_return_value,_num_return_value)
  @accepts(_accept_any)
  @synchronized
  def getnumbarcblocktriplets(self):
    """
    Obtains an upper bound on the number of elements in the block triplet form of barc.
  
    getnumbarcblocktriplets(self)
    returns: num
      num: long. An upper bound on the number of elements in the block triplet form of barc.
    """
    num_ = ctypes.c_int64()
    res = __library__.MSK_XX_getnumbarcblocktriplets(self.__nativep,ctypes.byref(num_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    num_ = num_.value
    _num_return_value = num_
    return (_num_return_value)
  @accepts(_accept_any,_make_long,_make_intvector,_make_intvector,_make_intvector,_make_doublevector)
  @synchronized
  def putbarcblocktriplet(self,num_,subj_,subk_,subl_,valjkl_):
    """
    Inputs barC in block triplet form.
  
    putbarcblocktriplet(self,num_,subj_,subk_,subl_,valjkl_)
      num: long. Number of elements in the block triplet form.
      subj: array of int. Symmetric matrix variable index.
      subk: array of int. Block row index.
      subl: array of int. Block column index.
      valjkl: array of double. The numerical value associated with each block triplet.
    """
    _subj_minlength = (num_)
    if (num_) > 0 and subj_ is not None and len(subj_) != (num_):
      raise ValueError("Array argument subj is not long enough: Is %d, expected %d" % (len(subj_),(num_)))
    if subj_ is None:
      raise ValueError("Argument subj cannot be None")
    if subj_ is None:
      raise ValueError("Argument subj may not be None")
    if isinstance(subj_, numpy.ndarray) and subj_.dtype is numpy.dtype(numpy.int32) and subj_.flags.contiguous:
      _subj_copyarray = False
      _subj_tmp = ctypes.cast(subj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subj_ is not None:
      _subj_copyarray = True
      _subj_np_tmp = numpy.zeros(len(subj_),numpy.dtype(numpy.int32))
      _subj_np_tmp[:] = subj_
      assert _subj_np_tmp.flags.contiguous
      _subj_tmp = ctypes.cast(_subj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subj_copyarray = False
      _subj_tmp = None
      
    _subk_minlength = (num_)
    if (num_) > 0 and subk_ is not None and len(subk_) != (num_):
      raise ValueError("Array argument subk is not long enough: Is %d, expected %d" % (len(subk_),(num_)))
    if subk_ is None:
      raise ValueError("Argument subk cannot be None")
    if subk_ is None:
      raise ValueError("Argument subk may not be None")
    if isinstance(subk_, numpy.ndarray) and subk_.dtype is numpy.dtype(numpy.int32) and subk_.flags.contiguous:
      _subk_copyarray = False
      _subk_tmp = ctypes.cast(subk_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subk_ is not None:
      _subk_copyarray = True
      _subk_np_tmp = numpy.zeros(len(subk_),numpy.dtype(numpy.int32))
      _subk_np_tmp[:] = subk_
      assert _subk_np_tmp.flags.contiguous
      _subk_tmp = ctypes.cast(_subk_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subk_copyarray = False
      _subk_tmp = None
      
    _subl_minlength = (num_)
    if (num_) > 0 and subl_ is not None and len(subl_) != (num_):
      raise ValueError("Array argument subl is not long enough: Is %d, expected %d" % (len(subl_),(num_)))
    if subl_ is None:
      raise ValueError("Argument subl cannot be None")
    if subl_ is None:
      raise ValueError("Argument subl may not be None")
    if isinstance(subl_, numpy.ndarray) and subl_.dtype is numpy.dtype(numpy.int32) and subl_.flags.contiguous:
      _subl_copyarray = False
      _subl_tmp = ctypes.cast(subl_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subl_ is not None:
      _subl_copyarray = True
      _subl_np_tmp = numpy.zeros(len(subl_),numpy.dtype(numpy.int32))
      _subl_np_tmp[:] = subl_
      assert _subl_np_tmp.flags.contiguous
      _subl_tmp = ctypes.cast(_subl_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subl_copyarray = False
      _subl_tmp = None
      
    _valjkl_minlength = (num_)
    if (num_) > 0 and valjkl_ is not None and len(valjkl_) != (num_):
      raise ValueError("Array argument valjkl is not long enough: Is %d, expected %d" % (len(valjkl_),(num_)))
    if valjkl_ is None:
      raise ValueError("Argument valjkl cannot be None")
    if valjkl_ is None:
      raise ValueError("Argument valjkl may not be None")
    if isinstance(valjkl_, numpy.ndarray) and valjkl_.dtype is numpy.dtype(numpy.float64) and valjkl_.flags.contiguous:
      _valjkl_copyarray = False
      _valjkl_tmp = ctypes.cast(valjkl_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif valjkl_ is not None:
      _valjkl_copyarray = True
      _valjkl_np_tmp = numpy.zeros(len(valjkl_),numpy.dtype(numpy.float64))
      _valjkl_np_tmp[:] = valjkl_
      assert _valjkl_np_tmp.flags.contiguous
      _valjkl_tmp = ctypes.cast(_valjkl_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _valjkl_copyarray = False
      _valjkl_tmp = None
      
    res = __library__.MSK_XX_putbarcblocktriplet(self.__nativep,num_,_subj_tmp,_subk_tmp,_subl_tmp,_valjkl_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_intvector,_accept_intvector,_accept_intvector,_accept_doublevector)
  @synchronized
  def getbarcblocktriplet(self,subj_,subk_,subl_,valjkl_):
    """
    Obtains barC in block triplet form.
  
    getbarcblocktriplet(self,subj_,subk_,subl_,valjkl_)
      subj: array of int. Symmetric matrix variable index.
      subk: array of int. Block row index.
      subl: array of int. Block column index.
      valjkl: array of double. The numerical value associated with each block triplet.
    returns: num
      num: long. Number of elements in the block triplet form.
    """
    maxnum_ = self.getnumbarcblocktriplets()
    num_ = ctypes.c_int64()
    _subj_minlength = (maxnum_)
    if (maxnum_) > 0 and subj_ is not None and len(subj_) != (maxnum_):
      raise ValueError("Array argument subj is not long enough: Is %d, expected %d" % (len(subj_),(maxnum_)))
    if isinstance(subj_,numpy.ndarray) and not subj_.flags.writeable:
      raise ValueError("Argument subj must be writable")
    if subj_ is None:
      raise ValueError("Argument subj may not be None")
    if isinstance(subj_, numpy.ndarray) and subj_.dtype is numpy.dtype(numpy.int32) and subj_.flags.contiguous:
      _subj_copyarray = False
      _subj_tmp = ctypes.cast(subj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subj_ is not None:
      _subj_copyarray = True
      _subj_np_tmp = numpy.zeros(len(subj_),numpy.dtype(numpy.int32))
      _subj_np_tmp[:] = subj_
      assert _subj_np_tmp.flags.contiguous
      _subj_tmp = ctypes.cast(_subj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subj_copyarray = False
      _subj_tmp = None
      
    _subk_minlength = (maxnum_)
    if (maxnum_) > 0 and subk_ is not None and len(subk_) != (maxnum_):
      raise ValueError("Array argument subk is not long enough: Is %d, expected %d" % (len(subk_),(maxnum_)))
    if isinstance(subk_,numpy.ndarray) and not subk_.flags.writeable:
      raise ValueError("Argument subk must be writable")
    if subk_ is None:
      raise ValueError("Argument subk may not be None")
    if isinstance(subk_, numpy.ndarray) and subk_.dtype is numpy.dtype(numpy.int32) and subk_.flags.contiguous:
      _subk_copyarray = False
      _subk_tmp = ctypes.cast(subk_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subk_ is not None:
      _subk_copyarray = True
      _subk_np_tmp = numpy.zeros(len(subk_),numpy.dtype(numpy.int32))
      _subk_np_tmp[:] = subk_
      assert _subk_np_tmp.flags.contiguous
      _subk_tmp = ctypes.cast(_subk_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subk_copyarray = False
      _subk_tmp = None
      
    _subl_minlength = (maxnum_)
    if (maxnum_) > 0 and subl_ is not None and len(subl_) != (maxnum_):
      raise ValueError("Array argument subl is not long enough: Is %d, expected %d" % (len(subl_),(maxnum_)))
    if isinstance(subl_,numpy.ndarray) and not subl_.flags.writeable:
      raise ValueError("Argument subl must be writable")
    if subl_ is None:
      raise ValueError("Argument subl may not be None")
    if isinstance(subl_, numpy.ndarray) and subl_.dtype is numpy.dtype(numpy.int32) and subl_.flags.contiguous:
      _subl_copyarray = False
      _subl_tmp = ctypes.cast(subl_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subl_ is not None:
      _subl_copyarray = True
      _subl_np_tmp = numpy.zeros(len(subl_),numpy.dtype(numpy.int32))
      _subl_np_tmp[:] = subl_
      assert _subl_np_tmp.flags.contiguous
      _subl_tmp = ctypes.cast(_subl_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subl_copyarray = False
      _subl_tmp = None
      
    _valjkl_minlength = (maxnum_)
    if (maxnum_) > 0 and valjkl_ is not None and len(valjkl_) != (maxnum_):
      raise ValueError("Array argument valjkl is not long enough: Is %d, expected %d" % (len(valjkl_),(maxnum_)))
    if isinstance(valjkl_,numpy.ndarray) and not valjkl_.flags.writeable:
      raise ValueError("Argument valjkl must be writable")
    if valjkl_ is None:
      raise ValueError("Argument valjkl may not be None")
    if isinstance(valjkl_, numpy.ndarray) and valjkl_.dtype is numpy.dtype(numpy.float64) and valjkl_.flags.contiguous:
      _valjkl_copyarray = False
      _valjkl_tmp = ctypes.cast(valjkl_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif valjkl_ is not None:
      _valjkl_copyarray = True
      _valjkl_np_tmp = numpy.zeros(len(valjkl_),numpy.dtype(numpy.float64))
      _valjkl_np_tmp[:] = valjkl_
      assert _valjkl_np_tmp.flags.contiguous
      _valjkl_tmp = ctypes.cast(_valjkl_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _valjkl_copyarray = False
      _valjkl_tmp = None
      
    res = __library__.MSK_XX_getbarcblocktriplet(self.__nativep,maxnum_,ctypes.byref(num_),_subj_tmp,_subk_tmp,_subl_tmp,_valjkl_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    num_ = num_.value
    _num_return_value = num_
    if _subj_copyarray:
      subj_[:] = _subj_np_tmp
    if _subk_copyarray:
      subk_[:] = _subk_np_tmp
    if _subl_copyarray:
      subl_[:] = _subl_np_tmp
    if _valjkl_copyarray:
      valjkl_[:] = _valjkl_np_tmp
    return (_num_return_value)
  @accepts(_accept_any,_make_long,_make_intvector,_make_intvector,_make_intvector,_make_intvector,_make_doublevector)
  @synchronized
  def putbarablocktriplet(self,num_,subi_,subj_,subk_,subl_,valijkl_):
    """
    Inputs barA in block triplet form.
  
    putbarablocktriplet(self,num_,subi_,subj_,subk_,subl_,valijkl_)
      num: long. Number of elements in the block triplet form.
      subi: array of int. Constraint index.
      subj: array of int. Symmetric matrix variable index.
      subk: array of int. Block row index.
      subl: array of int. Block column index.
      valijkl: array of double. The numerical value associated with each block triplet.
    """
    _subi_minlength = (num_)
    if (num_) > 0 and subi_ is not None and len(subi_) != (num_):
      raise ValueError("Array argument subi is not long enough: Is %d, expected %d" % (len(subi_),(num_)))
    if subi_ is None:
      raise ValueError("Argument subi cannot be None")
    if subi_ is None:
      raise ValueError("Argument subi may not be None")
    if isinstance(subi_, numpy.ndarray) and subi_.dtype is numpy.dtype(numpy.int32) and subi_.flags.contiguous:
      _subi_copyarray = False
      _subi_tmp = ctypes.cast(subi_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subi_ is not None:
      _subi_copyarray = True
      _subi_np_tmp = numpy.zeros(len(subi_),numpy.dtype(numpy.int32))
      _subi_np_tmp[:] = subi_
      assert _subi_np_tmp.flags.contiguous
      _subi_tmp = ctypes.cast(_subi_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subi_copyarray = False
      _subi_tmp = None
      
    _subj_minlength = (num_)
    if (num_) > 0 and subj_ is not None and len(subj_) != (num_):
      raise ValueError("Array argument subj is not long enough: Is %d, expected %d" % (len(subj_),(num_)))
    if subj_ is None:
      raise ValueError("Argument subj cannot be None")
    if subj_ is None:
      raise ValueError("Argument subj may not be None")
    if isinstance(subj_, numpy.ndarray) and subj_.dtype is numpy.dtype(numpy.int32) and subj_.flags.contiguous:
      _subj_copyarray = False
      _subj_tmp = ctypes.cast(subj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subj_ is not None:
      _subj_copyarray = True
      _subj_np_tmp = numpy.zeros(len(subj_),numpy.dtype(numpy.int32))
      _subj_np_tmp[:] = subj_
      assert _subj_np_tmp.flags.contiguous
      _subj_tmp = ctypes.cast(_subj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subj_copyarray = False
      _subj_tmp = None
      
    _subk_minlength = (num_)
    if (num_) > 0 and subk_ is not None and len(subk_) != (num_):
      raise ValueError("Array argument subk is not long enough: Is %d, expected %d" % (len(subk_),(num_)))
    if subk_ is None:
      raise ValueError("Argument subk cannot be None")
    if subk_ is None:
      raise ValueError("Argument subk may not be None")
    if isinstance(subk_, numpy.ndarray) and subk_.dtype is numpy.dtype(numpy.int32) and subk_.flags.contiguous:
      _subk_copyarray = False
      _subk_tmp = ctypes.cast(subk_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subk_ is not None:
      _subk_copyarray = True
      _subk_np_tmp = numpy.zeros(len(subk_),numpy.dtype(numpy.int32))
      _subk_np_tmp[:] = subk_
      assert _subk_np_tmp.flags.contiguous
      _subk_tmp = ctypes.cast(_subk_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subk_copyarray = False
      _subk_tmp = None
      
    _subl_minlength = (num_)
    if (num_) > 0 and subl_ is not None and len(subl_) != (num_):
      raise ValueError("Array argument subl is not long enough: Is %d, expected %d" % (len(subl_),(num_)))
    if subl_ is None:
      raise ValueError("Argument subl cannot be None")
    if subl_ is None:
      raise ValueError("Argument subl may not be None")
    if isinstance(subl_, numpy.ndarray) and subl_.dtype is numpy.dtype(numpy.int32) and subl_.flags.contiguous:
      _subl_copyarray = False
      _subl_tmp = ctypes.cast(subl_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subl_ is not None:
      _subl_copyarray = True
      _subl_np_tmp = numpy.zeros(len(subl_),numpy.dtype(numpy.int32))
      _subl_np_tmp[:] = subl_
      assert _subl_np_tmp.flags.contiguous
      _subl_tmp = ctypes.cast(_subl_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subl_copyarray = False
      _subl_tmp = None
      
    _valijkl_minlength = (num_)
    if (num_) > 0 and valijkl_ is not None and len(valijkl_) != (num_):
      raise ValueError("Array argument valijkl is not long enough: Is %d, expected %d" % (len(valijkl_),(num_)))
    if valijkl_ is None:
      raise ValueError("Argument valijkl cannot be None")
    if valijkl_ is None:
      raise ValueError("Argument valijkl may not be None")
    if isinstance(valijkl_, numpy.ndarray) and valijkl_.dtype is numpy.dtype(numpy.float64) and valijkl_.flags.contiguous:
      _valijkl_copyarray = False
      _valijkl_tmp = ctypes.cast(valijkl_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif valijkl_ is not None:
      _valijkl_copyarray = True
      _valijkl_np_tmp = numpy.zeros(len(valijkl_),numpy.dtype(numpy.float64))
      _valijkl_np_tmp[:] = valijkl_
      assert _valijkl_np_tmp.flags.contiguous
      _valijkl_tmp = ctypes.cast(_valijkl_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _valijkl_copyarray = False
      _valijkl_tmp = None
      
    res = __library__.MSK_XX_putbarablocktriplet(self.__nativep,num_,_subi_tmp,_subj_tmp,_subk_tmp,_subl_tmp,_valijkl_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any)
  @synchronized
  def getnumbarablocktriplets(self):
    """
    Obtains an upper bound on the number of scalar elements in the block triplet form of bara.
  
    getnumbarablocktriplets(self)
    returns: num
      num: long. An upper bound on the number of elements in the block triplet form of bara.
    """
    num_ = ctypes.c_int64()
    res = __library__.MSK_XX_getnumbarablocktriplets(self.__nativep,ctypes.byref(num_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    num_ = num_.value
    _num_return_value = num_
    return (_num_return_value)
  @accepts(_accept_any,_accept_intvector,_accept_intvector,_accept_intvector,_accept_intvector,_accept_doublevector)
  @synchronized
  def getbarablocktriplet(self,subi_,subj_,subk_,subl_,valijkl_):
    """
    Obtains barA in block triplet form.
  
    getbarablocktriplet(self,subi_,subj_,subk_,subl_,valijkl_)
      subi: array of int. Constraint index.
      subj: array of int. Symmetric matrix variable index.
      subk: array of int. Block row index.
      subl: array of int. Block column index.
      valijkl: array of double. The numerical value associated with each block triplet.
    returns: num
      num: long. Number of elements in the block triplet form.
    """
    maxnum_ = self.getnumbarablocktriplets()
    num_ = ctypes.c_int64()
    _subi_minlength = (maxnum_)
    if (maxnum_) > 0 and subi_ is not None and len(subi_) != (maxnum_):
      raise ValueError("Array argument subi is not long enough: Is %d, expected %d" % (len(subi_),(maxnum_)))
    if isinstance(subi_,numpy.ndarray) and not subi_.flags.writeable:
      raise ValueError("Argument subi must be writable")
    if subi_ is None:
      raise ValueError("Argument subi may not be None")
    if isinstance(subi_, numpy.ndarray) and subi_.dtype is numpy.dtype(numpy.int32) and subi_.flags.contiguous:
      _subi_copyarray = False
      _subi_tmp = ctypes.cast(subi_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subi_ is not None:
      _subi_copyarray = True
      _subi_np_tmp = numpy.zeros(len(subi_),numpy.dtype(numpy.int32))
      _subi_np_tmp[:] = subi_
      assert _subi_np_tmp.flags.contiguous
      _subi_tmp = ctypes.cast(_subi_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subi_copyarray = False
      _subi_tmp = None
      
    _subj_minlength = (maxnum_)
    if (maxnum_) > 0 and subj_ is not None and len(subj_) != (maxnum_):
      raise ValueError("Array argument subj is not long enough: Is %d, expected %d" % (len(subj_),(maxnum_)))
    if isinstance(subj_,numpy.ndarray) and not subj_.flags.writeable:
      raise ValueError("Argument subj must be writable")
    if subj_ is None:
      raise ValueError("Argument subj may not be None")
    if isinstance(subj_, numpy.ndarray) and subj_.dtype is numpy.dtype(numpy.int32) and subj_.flags.contiguous:
      _subj_copyarray = False
      _subj_tmp = ctypes.cast(subj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subj_ is not None:
      _subj_copyarray = True
      _subj_np_tmp = numpy.zeros(len(subj_),numpy.dtype(numpy.int32))
      _subj_np_tmp[:] = subj_
      assert _subj_np_tmp.flags.contiguous
      _subj_tmp = ctypes.cast(_subj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subj_copyarray = False
      _subj_tmp = None
      
    _subk_minlength = (maxnum_)
    if (maxnum_) > 0 and subk_ is not None and len(subk_) != (maxnum_):
      raise ValueError("Array argument subk is not long enough: Is %d, expected %d" % (len(subk_),(maxnum_)))
    if isinstance(subk_,numpy.ndarray) and not subk_.flags.writeable:
      raise ValueError("Argument subk must be writable")
    if subk_ is None:
      raise ValueError("Argument subk may not be None")
    if isinstance(subk_, numpy.ndarray) and subk_.dtype is numpy.dtype(numpy.int32) and subk_.flags.contiguous:
      _subk_copyarray = False
      _subk_tmp = ctypes.cast(subk_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subk_ is not None:
      _subk_copyarray = True
      _subk_np_tmp = numpy.zeros(len(subk_),numpy.dtype(numpy.int32))
      _subk_np_tmp[:] = subk_
      assert _subk_np_tmp.flags.contiguous
      _subk_tmp = ctypes.cast(_subk_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subk_copyarray = False
      _subk_tmp = None
      
    _subl_minlength = (maxnum_)
    if (maxnum_) > 0 and subl_ is not None and len(subl_) != (maxnum_):
      raise ValueError("Array argument subl is not long enough: Is %d, expected %d" % (len(subl_),(maxnum_)))
    if isinstance(subl_,numpy.ndarray) and not subl_.flags.writeable:
      raise ValueError("Argument subl must be writable")
    if subl_ is None:
      raise ValueError("Argument subl may not be None")
    if isinstance(subl_, numpy.ndarray) and subl_.dtype is numpy.dtype(numpy.int32) and subl_.flags.contiguous:
      _subl_copyarray = False
      _subl_tmp = ctypes.cast(subl_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subl_ is not None:
      _subl_copyarray = True
      _subl_np_tmp = numpy.zeros(len(subl_),numpy.dtype(numpy.int32))
      _subl_np_tmp[:] = subl_
      assert _subl_np_tmp.flags.contiguous
      _subl_tmp = ctypes.cast(_subl_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subl_copyarray = False
      _subl_tmp = None
      
    _valijkl_minlength = (maxnum_)
    if (maxnum_) > 0 and valijkl_ is not None and len(valijkl_) != (maxnum_):
      raise ValueError("Array argument valijkl is not long enough: Is %d, expected %d" % (len(valijkl_),(maxnum_)))
    if isinstance(valijkl_,numpy.ndarray) and not valijkl_.flags.writeable:
      raise ValueError("Argument valijkl must be writable")
    if valijkl_ is None:
      raise ValueError("Argument valijkl may not be None")
    if isinstance(valijkl_, numpy.ndarray) and valijkl_.dtype is numpy.dtype(numpy.float64) and valijkl_.flags.contiguous:
      _valijkl_copyarray = False
      _valijkl_tmp = ctypes.cast(valijkl_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif valijkl_ is not None:
      _valijkl_copyarray = True
      _valijkl_np_tmp = numpy.zeros(len(valijkl_),numpy.dtype(numpy.float64))
      _valijkl_np_tmp[:] = valijkl_
      assert _valijkl_np_tmp.flags.contiguous
      _valijkl_tmp = ctypes.cast(_valijkl_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _valijkl_copyarray = False
      _valijkl_tmp = None
      
    res = __library__.MSK_XX_getbarablocktriplet(self.__nativep,maxnum_,ctypes.byref(num_),_subi_tmp,_subj_tmp,_subk_tmp,_subl_tmp,_valijkl_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    num_ = num_.value
    _num_return_value = num_
    if _subi_copyarray:
      subi_[:] = _subi_np_tmp
    if _subj_copyarray:
      subj_[:] = _subj_np_tmp
    if _subk_copyarray:
      subk_[:] = _subk_np_tmp
    if _subl_copyarray:
      subl_[:] = _subl_np_tmp
    if _valijkl_copyarray:
      valijkl_[:] = _valijkl_np_tmp
    return (_num_return_value)
  @accepts(_accept_any,_make_int,_accept_anyenum(boundkey),_make_double,_make_double)
  @synchronized
  def putconbound(self,i_,bkc_,blc_,buc_):
    """
    Changes the bound for one constraint.
  
    putconbound(self,i_,bkc_,blc_,buc_)
      i: int. Index of the constraint.
      bkc: mosek.boundkey. New bound key.
      blc: double. New lower bound.
      buc: double. New upper bound.
    """
    res = __library__.MSK_XX_putconbound(self.__nativep,i_,bkc_,blc_,buc_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_intvector,_make_anyenumvector(boundkey),_make_doublevector,_make_doublevector)
  @synchronized
  def putconboundlist(self,sub_,bkc_,blc_,buc_):
    """
    Changes the bounds of a list of constraints.
  
    putconboundlist(self,sub_,bkc_,blc_,buc_)
      sub: array of int. List of constraint indexes.
      bkc: array of mosek.boundkey. Bound keys for the constraints.
      blc: array of double. Lower bounds for the constraints.
      buc: array of double. Upper bounds for the constraints.
    """
    num_ = None
    if num_ is None:
      num_ = len(sub_)
    elif num_ != len(sub_):
      raise IndexError("Inconsistent length of array sub")
    if num_ is None:
      num_ = len(bkc_)
    elif num_ != len(bkc_):
      raise IndexError("Inconsistent length of array bkc")
    if num_ is None:
      num_ = len(blc_)
    elif num_ != len(blc_):
      raise IndexError("Inconsistent length of array blc")
    if num_ is None:
      num_ = len(buc_)
    elif num_ != len(buc_):
      raise IndexError("Inconsistent length of array buc")
    if sub_ is None:
      raise ValueError("Argument sub cannot be None")
    if sub_ is None:
      raise ValueError("Argument sub may not be None")
    if isinstance(sub_, numpy.ndarray) and sub_.dtype is numpy.dtype(numpy.int32) and sub_.flags.contiguous:
      _sub_copyarray = False
      _sub_tmp = ctypes.cast(sub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif sub_ is not None:
      _sub_copyarray = True
      _sub_np_tmp = numpy.zeros(len(sub_),numpy.dtype(numpy.int32))
      _sub_np_tmp[:] = sub_
      assert _sub_np_tmp.flags.contiguous
      _sub_tmp = ctypes.cast(_sub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _sub_copyarray = False
      _sub_tmp = None
      
    if bkc_ is None:
      raise ValueError("Argument bkc cannot be None")
    if bkc_ is None:
      raise ValueError("Argument bkc may not be None")
    if bkc_ is not None:
        _bkc_tmp = (ctypes.c_int32 * len(bkc_))(*bkc_)
    else:
        _bkc_tmp = None
    if blc_ is None:
      raise ValueError("Argument blc cannot be None")
    if blc_ is None:
      raise ValueError("Argument blc may not be None")
    if isinstance(blc_, numpy.ndarray) and blc_.dtype is numpy.dtype(numpy.float64) and blc_.flags.contiguous:
      _blc_copyarray = False
      _blc_tmp = ctypes.cast(blc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif blc_ is not None:
      _blc_copyarray = True
      _blc_np_tmp = numpy.zeros(len(blc_),numpy.dtype(numpy.float64))
      _blc_np_tmp[:] = blc_
      assert _blc_np_tmp.flags.contiguous
      _blc_tmp = ctypes.cast(_blc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _blc_copyarray = False
      _blc_tmp = None
      
    if buc_ is None:
      raise ValueError("Argument buc cannot be None")
    if buc_ is None:
      raise ValueError("Argument buc may not be None")
    if isinstance(buc_, numpy.ndarray) and buc_.dtype is numpy.dtype(numpy.float64) and buc_.flags.contiguous:
      _buc_copyarray = False
      _buc_tmp = ctypes.cast(buc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif buc_ is not None:
      _buc_copyarray = True
      _buc_np_tmp = numpy.zeros(len(buc_),numpy.dtype(numpy.float64))
      _buc_np_tmp[:] = buc_
      assert _buc_np_tmp.flags.contiguous
      _buc_tmp = ctypes.cast(_buc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _buc_copyarray = False
      _buc_tmp = None
      
    res = __library__.MSK_XX_putconboundlist(self.__nativep,num_,_sub_tmp,_bkc_tmp,_blc_tmp,_buc_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_intvector,_accept_anyenum(boundkey),_make_double,_make_double)
  @synchronized
  def putconboundlistconst(self,sub_,bkc_,blc_,buc_):
    """
    Changes the bounds of a list of constraints.
  
    putconboundlistconst(self,sub_,bkc_,blc_,buc_)
      sub: array of int. List of constraint indexes.
      bkc: mosek.boundkey. New bound key for all constraints in the list.
      blc: double. New lower bound for all constraints in the list.
      buc: double. New upper bound for all constraints in the list.
    """
    num_ = None
    if num_ is None:
      num_ = len(sub_)
    elif num_ != len(sub_):
      raise IndexError("Inconsistent length of array sub")
    if sub_ is None:
      raise ValueError("Argument sub cannot be None")
    if sub_ is None:
      raise ValueError("Argument sub may not be None")
    if isinstance(sub_, numpy.ndarray) and sub_.dtype is numpy.dtype(numpy.int32) and sub_.flags.contiguous:
      _sub_copyarray = False
      _sub_tmp = ctypes.cast(sub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif sub_ is not None:
      _sub_copyarray = True
      _sub_np_tmp = numpy.zeros(len(sub_),numpy.dtype(numpy.int32))
      _sub_np_tmp[:] = sub_
      assert _sub_np_tmp.flags.contiguous
      _sub_tmp = ctypes.cast(_sub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _sub_copyarray = False
      _sub_tmp = None
      
    res = __library__.MSK_XX_putconboundlistconst(self.__nativep,num_,_sub_tmp,bkc_,blc_,buc_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_make_int,_make_anyenumvector(boundkey),_make_doublevector,_make_doublevector)
  @synchronized
  def putconboundslice(self,first_,last_,bkc_,blc_,buc_):
    """
    Changes the bounds for a slice of the constraints.
  
    putconboundslice(self,first_,last_,bkc_,blc_,buc_)
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      bkc: array of mosek.boundkey. Bound keys for the constraints.
      blc: array of double. Lower bounds for the constraints.
      buc: array of double. Upper bounds for the constraints.
    """
    _bkc_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and bkc_ is not None and len(bkc_) != ((last_) - (first_)):
      raise ValueError("Array argument bkc is not long enough: Is %d, expected %d" % (len(bkc_),((last_) - (first_))))
    if bkc_ is None:
      raise ValueError("Argument bkc cannot be None")
    if bkc_ is None:
      raise ValueError("Argument bkc may not be None")
    if bkc_ is not None:
        _bkc_tmp = (ctypes.c_int32 * len(bkc_))(*bkc_)
    else:
        _bkc_tmp = None
    _blc_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and blc_ is not None and len(blc_) != ((last_) - (first_)):
      raise ValueError("Array argument blc is not long enough: Is %d, expected %d" % (len(blc_),((last_) - (first_))))
    if blc_ is None:
      raise ValueError("Argument blc cannot be None")
    if blc_ is None:
      raise ValueError("Argument blc may not be None")
    if isinstance(blc_, numpy.ndarray) and blc_.dtype is numpy.dtype(numpy.float64) and blc_.flags.contiguous:
      _blc_copyarray = False
      _blc_tmp = ctypes.cast(blc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif blc_ is not None:
      _blc_copyarray = True
      _blc_np_tmp = numpy.zeros(len(blc_),numpy.dtype(numpy.float64))
      _blc_np_tmp[:] = blc_
      assert _blc_np_tmp.flags.contiguous
      _blc_tmp = ctypes.cast(_blc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _blc_copyarray = False
      _blc_tmp = None
      
    _buc_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and buc_ is not None and len(buc_) != ((last_) - (first_)):
      raise ValueError("Array argument buc is not long enough: Is %d, expected %d" % (len(buc_),((last_) - (first_))))
    if buc_ is None:
      raise ValueError("Argument buc cannot be None")
    if buc_ is None:
      raise ValueError("Argument buc may not be None")
    if isinstance(buc_, numpy.ndarray) and buc_.dtype is numpy.dtype(numpy.float64) and buc_.flags.contiguous:
      _buc_copyarray = False
      _buc_tmp = ctypes.cast(buc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif buc_ is not None:
      _buc_copyarray = True
      _buc_np_tmp = numpy.zeros(len(buc_),numpy.dtype(numpy.float64))
      _buc_np_tmp[:] = buc_
      assert _buc_np_tmp.flags.contiguous
      _buc_tmp = ctypes.cast(_buc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _buc_copyarray = False
      _buc_tmp = None
      
    res = __library__.MSK_XX_putconboundslice(self.__nativep,first_,last_,_bkc_tmp,_blc_tmp,_buc_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_make_int,_accept_anyenum(boundkey),_make_double,_make_double)
  @synchronized
  def putconboundsliceconst(self,first_,last_,bkc_,blc_,buc_):
    """
    Changes the bounds for a slice of the constraints.
  
    putconboundsliceconst(self,first_,last_,bkc_,blc_,buc_)
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      bkc: mosek.boundkey. New bound key for all constraints in the slice.
      blc: double. New lower bound for all constraints in the slice.
      buc: double. New upper bound for all constraints in the slice.
    """
    res = __library__.MSK_XX_putconboundsliceconst(self.__nativep,first_,last_,bkc_,blc_,buc_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_accept_anyenum(boundkey),_make_double,_make_double)
  @synchronized
  def putvarbound(self,j_,bkx_,blx_,bux_):
    """
    Changes the bounds for one variable.
  
    putvarbound(self,j_,bkx_,blx_,bux_)
      j: int. Index of the variable.
      bkx: mosek.boundkey. New bound key.
      blx: double. New lower bound.
      bux: double. New upper bound.
    """
    res = __library__.MSK_XX_putvarbound(self.__nativep,j_,bkx_,blx_,bux_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_intvector,_make_anyenumvector(boundkey),_make_doublevector,_make_doublevector)
  @synchronized
  def putvarboundlist(self,sub_,bkx_,blx_,bux_):
    """
    Changes the bounds of a list of variables.
  
    putvarboundlist(self,sub_,bkx_,blx_,bux_)
      sub: array of int. List of variable indexes.
      bkx: array of mosek.boundkey. Bound keys for the variables.
      blx: array of double. Lower bounds for the variables.
      bux: array of double. Upper bounds for the variables.
    """
    num_ = None
    if num_ is None:
      num_ = len(sub_)
    elif num_ != len(sub_):
      raise IndexError("Inconsistent length of array sub")
    if num_ is None:
      num_ = len(bkx_)
    elif num_ != len(bkx_):
      raise IndexError("Inconsistent length of array bkx")
    if num_ is None:
      num_ = len(blx_)
    elif num_ != len(blx_):
      raise IndexError("Inconsistent length of array blx")
    if num_ is None:
      num_ = len(bux_)
    elif num_ != len(bux_):
      raise IndexError("Inconsistent length of array bux")
    if sub_ is None:
      raise ValueError("Argument sub cannot be None")
    if sub_ is None:
      raise ValueError("Argument sub may not be None")
    if isinstance(sub_, numpy.ndarray) and sub_.dtype is numpy.dtype(numpy.int32) and sub_.flags.contiguous:
      _sub_copyarray = False
      _sub_tmp = ctypes.cast(sub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif sub_ is not None:
      _sub_copyarray = True
      _sub_np_tmp = numpy.zeros(len(sub_),numpy.dtype(numpy.int32))
      _sub_np_tmp[:] = sub_
      assert _sub_np_tmp.flags.contiguous
      _sub_tmp = ctypes.cast(_sub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _sub_copyarray = False
      _sub_tmp = None
      
    if bkx_ is None:
      raise ValueError("Argument bkx cannot be None")
    if bkx_ is None:
      raise ValueError("Argument bkx may not be None")
    if bkx_ is not None:
        _bkx_tmp = (ctypes.c_int32 * len(bkx_))(*bkx_)
    else:
        _bkx_tmp = None
    if blx_ is None:
      raise ValueError("Argument blx cannot be None")
    if blx_ is None:
      raise ValueError("Argument blx may not be None")
    if isinstance(blx_, numpy.ndarray) and blx_.dtype is numpy.dtype(numpy.float64) and blx_.flags.contiguous:
      _blx_copyarray = False
      _blx_tmp = ctypes.cast(blx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif blx_ is not None:
      _blx_copyarray = True
      _blx_np_tmp = numpy.zeros(len(blx_),numpy.dtype(numpy.float64))
      _blx_np_tmp[:] = blx_
      assert _blx_np_tmp.flags.contiguous
      _blx_tmp = ctypes.cast(_blx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _blx_copyarray = False
      _blx_tmp = None
      
    if bux_ is None:
      raise ValueError("Argument bux cannot be None")
    if bux_ is None:
      raise ValueError("Argument bux may not be None")
    if isinstance(bux_, numpy.ndarray) and bux_.dtype is numpy.dtype(numpy.float64) and bux_.flags.contiguous:
      _bux_copyarray = False
      _bux_tmp = ctypes.cast(bux_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif bux_ is not None:
      _bux_copyarray = True
      _bux_np_tmp = numpy.zeros(len(bux_),numpy.dtype(numpy.float64))
      _bux_np_tmp[:] = bux_
      assert _bux_np_tmp.flags.contiguous
      _bux_tmp = ctypes.cast(_bux_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _bux_copyarray = False
      _bux_tmp = None
      
    res = __library__.MSK_XX_putvarboundlist(self.__nativep,num_,_sub_tmp,_bkx_tmp,_blx_tmp,_bux_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_intvector,_accept_anyenum(boundkey),_make_double,_make_double)
  @synchronized
  def putvarboundlistconst(self,sub_,bkx_,blx_,bux_):
    """
    Changes the bounds of a list of variables.
  
    putvarboundlistconst(self,sub_,bkx_,blx_,bux_)
      sub: array of int. List of variable indexes.
      bkx: mosek.boundkey. New bound key for all variables in the list.
      blx: double. New lower bound for all variables in the list.
      bux: double. New upper bound for all variables in the list.
    """
    num_ = None
    if num_ is None:
      num_ = len(sub_)
    elif num_ != len(sub_):
      raise IndexError("Inconsistent length of array sub")
    if sub_ is None:
      raise ValueError("Argument sub cannot be None")
    if sub_ is None:
      raise ValueError("Argument sub may not be None")
    if isinstance(sub_, numpy.ndarray) and sub_.dtype is numpy.dtype(numpy.int32) and sub_.flags.contiguous:
      _sub_copyarray = False
      _sub_tmp = ctypes.cast(sub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif sub_ is not None:
      _sub_copyarray = True
      _sub_np_tmp = numpy.zeros(len(sub_),numpy.dtype(numpy.int32))
      _sub_np_tmp[:] = sub_
      assert _sub_np_tmp.flags.contiguous
      _sub_tmp = ctypes.cast(_sub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _sub_copyarray = False
      _sub_tmp = None
      
    res = __library__.MSK_XX_putvarboundlistconst(self.__nativep,num_,_sub_tmp,bkx_,blx_,bux_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_make_int,_make_anyenumvector(boundkey),_make_doublevector,_make_doublevector)
  @synchronized
  def putvarboundslice(self,first_,last_,bkx_,blx_,bux_):
    """
    Changes the bounds for a slice of the variables.
  
    putvarboundslice(self,first_,last_,bkx_,blx_,bux_)
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      bkx: array of mosek.boundkey. Bound keys for the variables.
      blx: array of double. Lower bounds for the variables.
      bux: array of double. Upper bounds for the variables.
    """
    _bkx_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and bkx_ is not None and len(bkx_) != ((last_) - (first_)):
      raise ValueError("Array argument bkx is not long enough: Is %d, expected %d" % (len(bkx_),((last_) - (first_))))
    if bkx_ is None:
      raise ValueError("Argument bkx cannot be None")
    if bkx_ is None:
      raise ValueError("Argument bkx may not be None")
    if bkx_ is not None:
        _bkx_tmp = (ctypes.c_int32 * len(bkx_))(*bkx_)
    else:
        _bkx_tmp = None
    _blx_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and blx_ is not None and len(blx_) != ((last_) - (first_)):
      raise ValueError("Array argument blx is not long enough: Is %d, expected %d" % (len(blx_),((last_) - (first_))))
    if blx_ is None:
      raise ValueError("Argument blx cannot be None")
    if blx_ is None:
      raise ValueError("Argument blx may not be None")
    if isinstance(blx_, numpy.ndarray) and blx_.dtype is numpy.dtype(numpy.float64) and blx_.flags.contiguous:
      _blx_copyarray = False
      _blx_tmp = ctypes.cast(blx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif blx_ is not None:
      _blx_copyarray = True
      _blx_np_tmp = numpy.zeros(len(blx_),numpy.dtype(numpy.float64))
      _blx_np_tmp[:] = blx_
      assert _blx_np_tmp.flags.contiguous
      _blx_tmp = ctypes.cast(_blx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _blx_copyarray = False
      _blx_tmp = None
      
    _bux_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and bux_ is not None and len(bux_) != ((last_) - (first_)):
      raise ValueError("Array argument bux is not long enough: Is %d, expected %d" % (len(bux_),((last_) - (first_))))
    if bux_ is None:
      raise ValueError("Argument bux cannot be None")
    if bux_ is None:
      raise ValueError("Argument bux may not be None")
    if isinstance(bux_, numpy.ndarray) and bux_.dtype is numpy.dtype(numpy.float64) and bux_.flags.contiguous:
      _bux_copyarray = False
      _bux_tmp = ctypes.cast(bux_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif bux_ is not None:
      _bux_copyarray = True
      _bux_np_tmp = numpy.zeros(len(bux_),numpy.dtype(numpy.float64))
      _bux_np_tmp[:] = bux_
      assert _bux_np_tmp.flags.contiguous
      _bux_tmp = ctypes.cast(_bux_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _bux_copyarray = False
      _bux_tmp = None
      
    res = __library__.MSK_XX_putvarboundslice(self.__nativep,first_,last_,_bkx_tmp,_blx_tmp,_bux_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_make_int,_accept_anyenum(boundkey),_make_double,_make_double)
  @synchronized
  def putvarboundsliceconst(self,first_,last_,bkx_,blx_,bux_):
    """
    Changes the bounds for a slice of the variables.
  
    putvarboundsliceconst(self,first_,last_,bkx_,blx_,bux_)
      first: int. First index in the sequence.
      last: int. Last index plus 1 in the sequence.
      bkx: mosek.boundkey. New bound key for all variables in the slice.
      blx: double. New lower bound for all variables in the slice.
      bux: double. New upper bound for all variables in the slice.
    """
    res = __library__.MSK_XX_putvarboundsliceconst(self.__nativep,first_,last_,bkx_,blx_,bux_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_double)
  @synchronized
  def putcfix(self,cfix_):
    """
    Replaces the fixed term in the objective.
  
    putcfix(self,cfix_)
      cfix: double. Fixed term in the objective.
    """
    res = __library__.MSK_XX_putcfix(self.__nativep,cfix_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_make_double)
  @synchronized
  def putcj(self,j_,cj_):
    """
    Modifies one linear coefficient in the objective.
  
    putcj(self,j_,cj_)
      j: int. Index of the variable whose objective coefficient should be changed.
      cj: double. New coefficient value.
    """
    res = __library__.MSK_XX_putcj(self.__nativep,j_,cj_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(objsense))
  @synchronized
  def putobjsense(self,sense_):
    """
    Sets the objective sense.
  
    putobjsense(self,sense_)
      sense: mosek.objsense. The objective sense of the task
    """
    res = __library__.MSK_XX_putobjsense(self.__nativep,sense_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any)
  @synchronized
  def getobjsense(self):
    """
    Gets the objective sense.
  
    getobjsense(self)
    returns: sense
      sense: mosek.objsense. The returned objective sense.
    """
    sense_ = ctypes.c_int32()
    res = __library__.MSK_XX_getobjsense(self.__nativep,ctypes.byref(sense_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _sense_return_value = objsense(sense_.value)
    return (_sense_return_value)
  @accepts(_accept_any,_make_intvector,_make_doublevector)
  @synchronized
  def putclist(self,subj_,val_):
    """
    Modifies a part of the linear objective coefficients.
  
    putclist(self,subj_,val_)
      subj: array of int. Indices of variables for which objective coefficients should be changed.
      val: array of double. New numerical values for the objective coefficients that should be modified.
    """
    num_ = None
    if num_ is None:
      num_ = len(subj_)
    elif num_ != len(subj_):
      raise IndexError("Inconsistent length of array subj")
    if num_ is None:
      num_ = len(val_)
    elif num_ != len(val_):
      raise IndexError("Inconsistent length of array val")
    if subj_ is None:
      raise ValueError("Argument subj cannot be None")
    if subj_ is None:
      raise ValueError("Argument subj may not be None")
    if isinstance(subj_, numpy.ndarray) and subj_.dtype is numpy.dtype(numpy.int32) and subj_.flags.contiguous:
      _subj_copyarray = False
      _subj_tmp = ctypes.cast(subj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subj_ is not None:
      _subj_copyarray = True
      _subj_np_tmp = numpy.zeros(len(subj_),numpy.dtype(numpy.int32))
      _subj_np_tmp[:] = subj_
      assert _subj_np_tmp.flags.contiguous
      _subj_tmp = ctypes.cast(_subj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subj_copyarray = False
      _subj_tmp = None
      
    if val_ is None:
      raise ValueError("Argument val cannot be None")
    if val_ is None:
      raise ValueError("Argument val may not be None")
    if isinstance(val_, numpy.ndarray) and val_.dtype is numpy.dtype(numpy.float64) and val_.flags.contiguous:
      _val_copyarray = False
      _val_tmp = ctypes.cast(val_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif val_ is not None:
      _val_copyarray = True
      _val_np_tmp = numpy.zeros(len(val_),numpy.dtype(numpy.float64))
      _val_np_tmp[:] = val_
      assert _val_np_tmp.flags.contiguous
      _val_tmp = ctypes.cast(_val_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _val_copyarray = False
      _val_tmp = None
      
    res = __library__.MSK_XX_putclist(self.__nativep,num_,_subj_tmp,_val_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_make_int,_make_doublevector)
  @synchronized
  def putcslice(self,first_,last_,slice_):
    """
    Modifies a slice of the linear objective coefficients.
  
    putcslice(self,first_,last_,slice_)
      first: int. First element in the slice of c.
      last: int. Last element plus 1 of the slice in c to be changed.
      slice: array of double. New numerical values for the objective coefficients that should be modified.
    """
    _slice_minlength = ((last_) - (first_))
    if ((last_) - (first_)) > 0 and slice_ is not None and len(slice_) != ((last_) - (first_)):
      raise ValueError("Array argument slice is not long enough: Is %d, expected %d" % (len(slice_),((last_) - (first_))))
    if slice_ is None:
      raise ValueError("Argument slice cannot be None")
    if slice_ is None:
      raise ValueError("Argument slice may not be None")
    if isinstance(slice_, numpy.ndarray) and slice_.dtype is numpy.dtype(numpy.float64) and slice_.flags.contiguous:
      _slice_copyarray = False
      _slice_tmp = ctypes.cast(slice_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif slice_ is not None:
      _slice_copyarray = True
      _slice_np_tmp = numpy.zeros(len(slice_),numpy.dtype(numpy.float64))
      _slice_np_tmp[:] = slice_
      assert _slice_np_tmp.flags.contiguous
      _slice_tmp = ctypes.cast(_slice_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _slice_copyarray = False
      _slice_tmp = None
      
    res = __library__.MSK_XX_putcslice(self.__nativep,first_,last_,_slice_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_make_longvector,_make_doublevector)
  @synchronized
  def putbarcj(self,j_,sub_,weights_):
    """
    Changes one element in barc.
  
    putbarcj(self,j_,sub_,weights_)
      j: int. Index of the element in barc` that should be changed.
      sub: array of long. sub is list of indexes of those symmetric matrices appearing in sum.
      weights: array of double. The weights of the terms in the weighted sum.
    """
    num_ = None
    if num_ is None:
      num_ = len(sub_)
    elif num_ != len(sub_):
      raise IndexError("Inconsistent length of array sub")
    if num_ is None:
      num_ = len(weights_)
    elif num_ != len(weights_):
      raise IndexError("Inconsistent length of array weights")
    if sub_ is None:
      raise ValueError("Argument sub cannot be None")
    if sub_ is None:
      raise ValueError("Argument sub may not be None")
    if isinstance(sub_, numpy.ndarray) and sub_.dtype is numpy.dtype(numpy.int64) and sub_.flags.contiguous:
      _sub_copyarray = False
      _sub_tmp = ctypes.cast(sub_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif sub_ is not None:
      _sub_copyarray = True
      _sub_np_tmp = numpy.zeros(len(sub_),numpy.dtype(numpy.int64))
      _sub_np_tmp[:] = sub_
      assert _sub_np_tmp.flags.contiguous
      _sub_tmp = ctypes.cast(_sub_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _sub_copyarray = False
      _sub_tmp = None
      
    if weights_ is None:
      raise ValueError("Argument weights cannot be None")
    if weights_ is None:
      raise ValueError("Argument weights may not be None")
    if isinstance(weights_, numpy.ndarray) and weights_.dtype is numpy.dtype(numpy.float64) and weights_.flags.contiguous:
      _weights_copyarray = False
      _weights_tmp = ctypes.cast(weights_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif weights_ is not None:
      _weights_copyarray = True
      _weights_np_tmp = numpy.zeros(len(weights_),numpy.dtype(numpy.float64))
      _weights_np_tmp[:] = weights_
      assert _weights_np_tmp.flags.contiguous
      _weights_tmp = ctypes.cast(_weights_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _weights_copyarray = False
      _weights_tmp = None
      
    res = __library__.MSK_XX_putbarcj(self.__nativep,j_,num_,_sub_tmp,_weights_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_accept_anyenum(conetype),_make_double,_make_intvector)
  @synchronized
  def putcone(self,k_,ct_,conepar_,submem_):
    """
    Replaces a conic constraint.
  
    putcone(self,k_,ct_,conepar_,submem_)
      k: int. Index of the cone.
      ct: mosek.conetype. Specifies the type of the cone.
      conepar: double. For the power cone it denotes the exponent alpha. For other cone types it is unused and can be set to 0.
      submem: array of int. Variable subscripts of the members in the cone.
    """
    nummem_ = None
    if nummem_ is None:
      nummem_ = len(submem_)
    elif nummem_ != len(submem_):
      raise IndexError("Inconsistent length of array submem")
    if submem_ is None:
      raise ValueError("Argument submem cannot be None")
    if submem_ is None:
      raise ValueError("Argument submem may not be None")
    if isinstance(submem_, numpy.ndarray) and submem_.dtype is numpy.dtype(numpy.int32) and submem_.flags.contiguous:
      _submem_copyarray = False
      _submem_tmp = ctypes.cast(submem_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif submem_ is not None:
      _submem_copyarray = True
      _submem_np_tmp = numpy.zeros(len(submem_),numpy.dtype(numpy.int32))
      _submem_np_tmp[:] = submem_
      assert _submem_np_tmp.flags.contiguous
      _submem_tmp = ctypes.cast(_submem_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _submem_copyarray = False
      _submem_tmp = None
      
    res = __library__.MSK_XX_putcone(self.__nativep,k_,ct_,conepar_,nummem_,_submem_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_make_intvector,_make_intvector,_make_doublevector)
  @synchronized
  def appendsparsesymmat(self,dim_,subi_,subj_,valij_):
    """
    Appends a general sparse symmetric matrix to the storage of symmetric matrices.
  
    appendsparsesymmat(self,dim_,subi_,subj_,valij_)
      dim: int. Dimension of the symmetric matrix that is appended.
      subi: array of int. Row subscript in the triplets.
      subj: array of int. Column subscripts in the triplets.
      valij: array of double. Values of each triplet.
    returns: idx
      idx: long. Unique index assigned to the inputted matrix.
    """
    nz_ = None
    if nz_ is None:
      nz_ = len(subi_)
    elif nz_ != len(subi_):
      raise IndexError("Inconsistent length of array subi")
    if nz_ is None:
      nz_ = len(subj_)
    elif nz_ != len(subj_):
      raise IndexError("Inconsistent length of array subj")
    if nz_ is None:
      nz_ = len(valij_)
    elif nz_ != len(valij_):
      raise IndexError("Inconsistent length of array valij")
    if subi_ is None:
      raise ValueError("Argument subi cannot be None")
    if subi_ is None:
      raise ValueError("Argument subi may not be None")
    if isinstance(subi_, numpy.ndarray) and subi_.dtype is numpy.dtype(numpy.int32) and subi_.flags.contiguous:
      _subi_copyarray = False
      _subi_tmp = ctypes.cast(subi_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subi_ is not None:
      _subi_copyarray = True
      _subi_np_tmp = numpy.zeros(len(subi_),numpy.dtype(numpy.int32))
      _subi_np_tmp[:] = subi_
      assert _subi_np_tmp.flags.contiguous
      _subi_tmp = ctypes.cast(_subi_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subi_copyarray = False
      _subi_tmp = None
      
    if subj_ is None:
      raise ValueError("Argument subj cannot be None")
    if subj_ is None:
      raise ValueError("Argument subj may not be None")
    if isinstance(subj_, numpy.ndarray) and subj_.dtype is numpy.dtype(numpy.int32) and subj_.flags.contiguous:
      _subj_copyarray = False
      _subj_tmp = ctypes.cast(subj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subj_ is not None:
      _subj_copyarray = True
      _subj_np_tmp = numpy.zeros(len(subj_),numpy.dtype(numpy.int32))
      _subj_np_tmp[:] = subj_
      assert _subj_np_tmp.flags.contiguous
      _subj_tmp = ctypes.cast(_subj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subj_copyarray = False
      _subj_tmp = None
      
    if valij_ is None:
      raise ValueError("Argument valij cannot be None")
    if valij_ is None:
      raise ValueError("Argument valij may not be None")
    if isinstance(valij_, numpy.ndarray) and valij_.dtype is numpy.dtype(numpy.float64) and valij_.flags.contiguous:
      _valij_copyarray = False
      _valij_tmp = ctypes.cast(valij_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif valij_ is not None:
      _valij_copyarray = True
      _valij_np_tmp = numpy.zeros(len(valij_),numpy.dtype(numpy.float64))
      _valij_np_tmp[:] = valij_
      assert _valij_np_tmp.flags.contiguous
      _valij_tmp = ctypes.cast(_valij_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _valij_copyarray = False
      _valij_tmp = None
      
    idx_ = ctypes.c_int64()
    res = __library__.MSK_XX_appendsparsesymmat(self.__nativep,dim_,nz_,_subi_tmp,_subj_tmp,_valij_tmp,ctypes.byref(idx_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    idx_ = idx_.value
    _idx_return_value = idx_
    return (_idx_return_value)
  @accepts(_accept_any,_make_intvector,_make_longvector,_make_intvector,_make_intvector,_make_doublevector,_accept_longvector)
  @synchronized
  def appendsparsesymmatlist(self,dims_,nz_,subi_,subj_,valij_,idx_):
    """
    Appends a general sparse symmetric matrix to the storage of symmetric matrices.
  
    appendsparsesymmatlist(self,dims_,nz_,subi_,subj_,valij_,idx_)
      dims: array of int. Dimensions of the symmetric matrixes.
      nz: array of long. Number of nonzeros for each matrix.
      subi: array of int. Row subscript in the triplets.
      subj: array of int. Column subscripts in the triplets.
      valij: array of double. Values of each triplet.
      idx: array of long. Unique index assigned to the inputted matrix.
    """
    num_ = None
    if num_ is None:
      num_ = len(dims_)
    elif num_ != len(dims_):
      raise IndexError("Inconsistent length of array dims")
    if num_ is None:
      num_ = len(nz_)
    elif num_ != len(nz_):
      raise IndexError("Inconsistent length of array nz")
    if dims_ is None:
      raise ValueError("Argument dims cannot be None")
    if dims_ is None:
      raise ValueError("Argument dims may not be None")
    if isinstance(dims_, numpy.ndarray) and dims_.dtype is numpy.dtype(numpy.int32) and dims_.flags.contiguous:
      _dims_copyarray = False
      _dims_tmp = ctypes.cast(dims_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif dims_ is not None:
      _dims_copyarray = True
      _dims_np_tmp = numpy.zeros(len(dims_),numpy.dtype(numpy.int32))
      _dims_np_tmp[:] = dims_
      assert _dims_np_tmp.flags.contiguous
      _dims_tmp = ctypes.cast(_dims_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _dims_copyarray = False
      _dims_tmp = None
      
    if nz_ is None:
      raise ValueError("Argument nz cannot be None")
    if nz_ is None:
      raise ValueError("Argument nz may not be None")
    if isinstance(nz_, numpy.ndarray) and nz_.dtype is numpy.dtype(numpy.int64) and nz_.flags.contiguous:
      _nz_copyarray = False
      _nz_tmp = ctypes.cast(nz_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif nz_ is not None:
      _nz_copyarray = True
      _nz_np_tmp = numpy.zeros(len(nz_),numpy.dtype(numpy.int64))
      _nz_np_tmp[:] = nz_
      assert _nz_np_tmp.flags.contiguous
      _nz_tmp = ctypes.cast(_nz_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _nz_copyarray = False
      _nz_tmp = None
      
    _subi_minlength = sum((nz_))
    if sum((nz_)) > 0 and subi_ is not None and len(subi_) != sum((nz_)):
      raise ValueError("Array argument subi is not long enough: Is %d, expected %d" % (len(subi_),sum((nz_))))
    if subi_ is None:
      raise ValueError("Argument subi cannot be None")
    if subi_ is None:
      raise ValueError("Argument subi may not be None")
    if isinstance(subi_, numpy.ndarray) and subi_.dtype is numpy.dtype(numpy.int32) and subi_.flags.contiguous:
      _subi_copyarray = False
      _subi_tmp = ctypes.cast(subi_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subi_ is not None:
      _subi_copyarray = True
      _subi_np_tmp = numpy.zeros(len(subi_),numpy.dtype(numpy.int32))
      _subi_np_tmp[:] = subi_
      assert _subi_np_tmp.flags.contiguous
      _subi_tmp = ctypes.cast(_subi_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subi_copyarray = False
      _subi_tmp = None
      
    _subj_minlength = sum((nz_))
    if sum((nz_)) > 0 and subj_ is not None and len(subj_) != sum((nz_)):
      raise ValueError("Array argument subj is not long enough: Is %d, expected %d" % (len(subj_),sum((nz_))))
    if subj_ is None:
      raise ValueError("Argument subj cannot be None")
    if subj_ is None:
      raise ValueError("Argument subj may not be None")
    if isinstance(subj_, numpy.ndarray) and subj_.dtype is numpy.dtype(numpy.int32) and subj_.flags.contiguous:
      _subj_copyarray = False
      _subj_tmp = ctypes.cast(subj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subj_ is not None:
      _subj_copyarray = True
      _subj_np_tmp = numpy.zeros(len(subj_),numpy.dtype(numpy.int32))
      _subj_np_tmp[:] = subj_
      assert _subj_np_tmp.flags.contiguous
      _subj_tmp = ctypes.cast(_subj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subj_copyarray = False
      _subj_tmp = None
      
    _valij_minlength = sum((nz_))
    if sum((nz_)) > 0 and valij_ is not None and len(valij_) != sum((nz_)):
      raise ValueError("Array argument valij is not long enough: Is %d, expected %d" % (len(valij_),sum((nz_))))
    if valij_ is None:
      raise ValueError("Argument valij cannot be None")
    if valij_ is None:
      raise ValueError("Argument valij may not be None")
    if isinstance(valij_, numpy.ndarray) and valij_.dtype is numpy.dtype(numpy.float64) and valij_.flags.contiguous:
      _valij_copyarray = False
      _valij_tmp = ctypes.cast(valij_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif valij_ is not None:
      _valij_copyarray = True
      _valij_np_tmp = numpy.zeros(len(valij_),numpy.dtype(numpy.float64))
      _valij_np_tmp[:] = valij_
      assert _valij_np_tmp.flags.contiguous
      _valij_tmp = ctypes.cast(_valij_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _valij_copyarray = False
      _valij_tmp = None
      
    _idx_minlength = (num_)
    if (num_) > 0 and idx_ is not None and len(idx_) != (num_):
      raise ValueError("Array argument idx is not long enough: Is %d, expected %d" % (len(idx_),(num_)))
    if isinstance(idx_,numpy.ndarray) and not idx_.flags.writeable:
      raise ValueError("Argument idx must be writable")
    if idx_ is None:
      raise ValueError("Argument idx may not be None")
    if isinstance(idx_, numpy.ndarray) and idx_.dtype is numpy.dtype(numpy.int64) and idx_.flags.contiguous:
      _idx_copyarray = False
      _idx_tmp = ctypes.cast(idx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    elif idx_ is not None:
      _idx_copyarray = True
      _idx_np_tmp = numpy.zeros(len(idx_),numpy.dtype(numpy.int64))
      _idx_np_tmp[:] = idx_
      assert _idx_np_tmp.flags.contiguous
      _idx_tmp = ctypes.cast(_idx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int64))
    else:
      _idx_copyarray = False
      _idx_tmp = None
      
    res = __library__.MSK_XX_appendsparsesymmatlist(self.__nativep,num_,_dims_tmp,_nz_tmp,_subi_tmp,_subj_tmp,_valij_tmp,_idx_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _idx_copyarray:
      idx_[:] = _idx_np_tmp
  @accepts(_accept_any,_make_long)
  @synchronized
  def getsymmatinfo(self,idx_):
    """
    Obtains information about a matrix from the symmetric matrix storage.
  
    getsymmatinfo(self,idx_)
      idx: long. Index of the matrix for which information is requested.
    returns: dim,nz,type
      dim: int. Returns the dimension of the requested matrix.
      nz: long. Returns the number of non-zeros in the requested matrix.
      type: mosek.symmattype. Returns the type of the requested matrix.
    """
    dim_ = ctypes.c_int32()
    nz_ = ctypes.c_int64()
    type_ = ctypes.c_int32()
    res = __library__.MSK_XX_getsymmatinfo(self.__nativep,idx_,ctypes.byref(dim_),ctypes.byref(nz_),ctypes.byref(type_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    dim_ = dim_.value
    _dim_return_value = dim_
    nz_ = nz_.value
    _nz_return_value = nz_
    _type_return_value = symmattype(type_.value)
    return (_dim_return_value,_nz_return_value,_type_return_value)
  @accepts(_accept_any)
  @synchronized
  def getnumsymmat(self):
    """
    Obtains the number of symmetric matrices stored.
  
    getnumsymmat(self)
    returns: num
      num: long. The number of symmetric sparse matrices.
    """
    num_ = ctypes.c_int64()
    res = __library__.MSK_XX_getnumsymmat(self.__nativep,ctypes.byref(num_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    num_ = num_.value
    _num_return_value = num_
    return (_num_return_value)
  @accepts(_accept_any,_make_long,_accept_intvector,_accept_intvector,_accept_doublevector)
  @synchronized
  def getsparsesymmat(self,idx_,subi_,subj_,valij_):
    """
    Gets a single symmetric matrix from the matrix store.
  
    getsparsesymmat(self,idx_,subi_,subj_,valij_)
      idx: long. Index of the matrix to retrieve.
      subi: array of int. Row subscripts of the matrix non-zero elements.
      subj: array of int. Column subscripts of the matrix non-zero elements.
      valij: array of double. Coefficients of the matrix non-zero elements.
    """
    maxlen_ = self.getsymmatinfo((idx_))[1]
    _subi_minlength = (maxlen_)
    if (maxlen_) > 0 and subi_ is not None and len(subi_) != (maxlen_):
      raise ValueError("Array argument subi is not long enough: Is %d, expected %d" % (len(subi_),(maxlen_)))
    if isinstance(subi_,numpy.ndarray) and not subi_.flags.writeable:
      raise ValueError("Argument subi must be writable")
    if isinstance(subi_, numpy.ndarray) and subi_.dtype is numpy.dtype(numpy.int32) and subi_.flags.contiguous:
      _subi_copyarray = False
      _subi_tmp = ctypes.cast(subi_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subi_ is not None:
      _subi_copyarray = True
      _subi_np_tmp = numpy.zeros(len(subi_),numpy.dtype(numpy.int32))
      _subi_np_tmp[:] = subi_
      assert _subi_np_tmp.flags.contiguous
      _subi_tmp = ctypes.cast(_subi_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subi_copyarray = False
      _subi_tmp = None
      
    _subj_minlength = (maxlen_)
    if (maxlen_) > 0 and subj_ is not None and len(subj_) != (maxlen_):
      raise ValueError("Array argument subj is not long enough: Is %d, expected %d" % (len(subj_),(maxlen_)))
    if isinstance(subj_,numpy.ndarray) and not subj_.flags.writeable:
      raise ValueError("Argument subj must be writable")
    if isinstance(subj_, numpy.ndarray) and subj_.dtype is numpy.dtype(numpy.int32) and subj_.flags.contiguous:
      _subj_copyarray = False
      _subj_tmp = ctypes.cast(subj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subj_ is not None:
      _subj_copyarray = True
      _subj_np_tmp = numpy.zeros(len(subj_),numpy.dtype(numpy.int32))
      _subj_np_tmp[:] = subj_
      assert _subj_np_tmp.flags.contiguous
      _subj_tmp = ctypes.cast(_subj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subj_copyarray = False
      _subj_tmp = None
      
    _valij_minlength = (maxlen_)
    if (maxlen_) > 0 and valij_ is not None and len(valij_) != (maxlen_):
      raise ValueError("Array argument valij is not long enough: Is %d, expected %d" % (len(valij_),(maxlen_)))
    if isinstance(valij_,numpy.ndarray) and not valij_.flags.writeable:
      raise ValueError("Argument valij must be writable")
    if isinstance(valij_, numpy.ndarray) and valij_.dtype is numpy.dtype(numpy.float64) and valij_.flags.contiguous:
      _valij_copyarray = False
      _valij_tmp = ctypes.cast(valij_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif valij_ is not None:
      _valij_copyarray = True
      _valij_np_tmp = numpy.zeros(len(valij_),numpy.dtype(numpy.float64))
      _valij_np_tmp[:] = valij_
      assert _valij_np_tmp.flags.contiguous
      _valij_tmp = ctypes.cast(_valij_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _valij_copyarray = False
      _valij_tmp = None
      
    res = __library__.MSK_XX_getsparsesymmat(self.__nativep,idx_,maxlen_,_subi_tmp,_subj_tmp,_valij_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _subi_copyarray:
      subi_[:] = _subi_np_tmp
    if _subj_copyarray:
      subj_[:] = _subj_np_tmp
    if _valij_copyarray:
      valij_[:] = _valij_np_tmp
  @accepts(_accept_any,_accept_anyenum(dparam),_make_double)
  @synchronized
  def putdouparam(self,param_,parvalue_):
    """
    Sets a double parameter.
  
    putdouparam(self,param_,parvalue_)
      param: mosek.dparam. Which parameter.
      parvalue: double. Parameter value.
    """
    res = __library__.MSK_XX_putdouparam(self.__nativep,param_,parvalue_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(iparam),_make_int)
  @synchronized
  def putintparam(self,param_,parvalue_):
    """
    Sets an integer parameter.
  
    putintparam(self,param_,parvalue_)
      param: mosek.iparam. Which parameter.
      parvalue: int. Parameter value.
    """
    res = __library__.MSK_XX_putintparam(self.__nativep,param_,parvalue_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int)
  @synchronized
  def putmaxnumcon(self,maxnumcon_):
    """
    Sets the number of preallocated constraints in the optimization task.
  
    putmaxnumcon(self,maxnumcon_)
      maxnumcon: int. Number of preallocated constraints in the optimization task.
    """
    res = __library__.MSK_XX_putmaxnumcon(self.__nativep,maxnumcon_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int)
  @synchronized
  def putmaxnumcone(self,maxnumcone_):
    """
    Sets the number of preallocated conic constraints in the optimization task.
  
    putmaxnumcone(self,maxnumcone_)
      maxnumcone: int. Number of preallocated conic constraints in the optimization task.
    """
    res = __library__.MSK_XX_putmaxnumcone(self.__nativep,maxnumcone_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any)
  @synchronized
  def getmaxnumcone(self):
    """
    Obtains the number of preallocated cones in the optimization task.
  
    getmaxnumcone(self)
    returns: maxnumcone
      maxnumcone: int. Number of preallocated conic constraints in the optimization task.
    """
    maxnumcone_ = ctypes.c_int32()
    res = __library__.MSK_XX_getmaxnumcone(self.__nativep,ctypes.byref(maxnumcone_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    maxnumcone_ = maxnumcone_.value
    _maxnumcone_return_value = maxnumcone_
    return (_maxnumcone_return_value)
  @accepts(_accept_any,_make_int)
  @synchronized
  def putmaxnumvar(self,maxnumvar_):
    """
    Sets the number of preallocated variables in the optimization task.
  
    putmaxnumvar(self,maxnumvar_)
      maxnumvar: int. Number of preallocated variables in the optimization task.
    """
    res = __library__.MSK_XX_putmaxnumvar(self.__nativep,maxnumvar_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int)
  @synchronized
  def putmaxnumbarvar(self,maxnumbarvar_):
    """
    Sets the number of preallocated symmetric matrix variables.
  
    putmaxnumbarvar(self,maxnumbarvar_)
      maxnumbarvar: int. Number of preallocated symmetric matrix variables.
    """
    res = __library__.MSK_XX_putmaxnumbarvar(self.__nativep,maxnumbarvar_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_long)
  @synchronized
  def putmaxnumanz(self,maxnumanz_):
    """
    Sets the number of preallocated non-zero entries in the linear coefficient matrix.
  
    putmaxnumanz(self,maxnumanz_)
      maxnumanz: long. New size of the storage reserved for storing the linear coefficient matrix.
    """
    res = __library__.MSK_XX_putmaxnumanz(self.__nativep,maxnumanz_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_long)
  @synchronized
  def putmaxnumqnz(self,maxnumqnz_):
    """
    Sets the number of preallocated non-zero entries in quadratic terms.
  
    putmaxnumqnz(self,maxnumqnz_)
      maxnumqnz: long. Number of non-zero elements preallocated in quadratic coefficient matrices.
    """
    res = __library__.MSK_XX_putmaxnumqnz(self.__nativep,maxnumqnz_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any)
  @synchronized
  def getmaxnumqnz(self):
    """
    Obtains the number of preallocated non-zeros for all quadratic terms in objective and constraints.
  
    getmaxnumqnz(self)
    returns: maxnumqnz
      maxnumqnz: long. Number of non-zero elements preallocated in quadratic coefficient matrices.
    """
    maxnumqnz_ = ctypes.c_int64()
    res = __library__.MSK_XX_getmaxnumqnz64(self.__nativep,ctypes.byref(maxnumqnz_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    maxnumqnz_ = maxnumqnz_.value
    _maxnumqnz_return_value = maxnumqnz_
    return (_maxnumqnz_return_value)
  @accepts(_accept_any,_accept_str,_make_double)
  @synchronized
  def putnadouparam(self,paramname_,parvalue_):
    """
    Sets a double parameter.
  
    putnadouparam(self,paramname_,parvalue_)
      paramname: str|unicode. Name of a parameter.
      parvalue: double. Parameter value.
    """
    if isinstance(paramname_,unicode):
      paramname_ = paramname_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_putnadouparam(self.__nativep,paramname_,parvalue_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_str,_make_int)
  @synchronized
  def putnaintparam(self,paramname_,parvalue_):
    """
    Sets an integer parameter.
  
    putnaintparam(self,paramname_,parvalue_)
      paramname: str|unicode. Name of a parameter.
      parvalue: int. Parameter value.
    """
    if isinstance(paramname_,unicode):
      paramname_ = paramname_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_putnaintparam(self.__nativep,paramname_,parvalue_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_str,_accept_str)
  @synchronized
  def putnastrparam(self,paramname_,parvalue_):
    """
    Sets a string parameter.
  
    putnastrparam(self,paramname_,parvalue_)
      paramname: str|unicode. Name of a parameter.
      parvalue: str|unicode. Parameter value.
    """
    if isinstance(paramname_,unicode):
      paramname_ = paramname_.encode("utf-8",errors="replace")
    if isinstance(parvalue_,unicode):
      parvalue_ = parvalue_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_putnastrparam(self.__nativep,paramname_,parvalue_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_str)
  @synchronized
  def putobjname(self,objname_):
    """
    Assigns a new name to the objective.
  
    putobjname(self,objname_)
      objname: str|unicode. Name of the objective.
    """
    if isinstance(objname_,unicode):
      objname_ = objname_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_putobjname(self.__nativep,objname_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_str,_accept_str)
  @synchronized
  def putparam(self,parname_,parvalue_):
    """
    Modifies the value of parameter.
  
    putparam(self,parname_,parvalue_)
      parname: str|unicode. Parameter name.
      parvalue: str|unicode. Parameter value.
    """
    if isinstance(parname_,unicode):
      parname_ = parname_.encode("utf-8",errors="replace")
    if isinstance(parvalue_,unicode):
      parvalue_ = parvalue_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_putparam(self.__nativep,parname_,parvalue_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_intvector,_make_intvector,_make_intvector,_make_doublevector)
  @synchronized
  def putqcon(self,qcsubk_,qcsubi_,qcsubj_,qcval_):
    """
    Replaces all quadratic terms in constraints.
  
    putqcon(self,qcsubk_,qcsubi_,qcsubj_,qcval_)
      qcsubk: array of int. Constraint subscripts for quadratic coefficients.
      qcsubi: array of int. Row subscripts for quadratic constraint matrix.
      qcsubj: array of int. Column subscripts for quadratic constraint matrix.
      qcval: array of double. Quadratic constraint coefficient values.
    """
    numqcnz_ = None
    if numqcnz_ is None:
      numqcnz_ = len(qcsubi_)
    elif numqcnz_ != len(qcsubi_):
      raise IndexError("Inconsistent length of array qcsubi")
    if numqcnz_ is None:
      numqcnz_ = len(qcsubj_)
    elif numqcnz_ != len(qcsubj_):
      raise IndexError("Inconsistent length of array qcsubj")
    if numqcnz_ is None:
      numqcnz_ = len(qcval_)
    elif numqcnz_ != len(qcval_):
      raise IndexError("Inconsistent length of array qcval")
    if qcsubk_ is None:
      raise ValueError("Argument qcsubk cannot be None")
    if qcsubk_ is None:
      raise ValueError("Argument qcsubk may not be None")
    if isinstance(qcsubk_, numpy.ndarray) and qcsubk_.dtype is numpy.dtype(numpy.int32) and qcsubk_.flags.contiguous:
      _qcsubk_copyarray = False
      _qcsubk_tmp = ctypes.cast(qcsubk_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif qcsubk_ is not None:
      _qcsubk_copyarray = True
      _qcsubk_np_tmp = numpy.zeros(len(qcsubk_),numpy.dtype(numpy.int32))
      _qcsubk_np_tmp[:] = qcsubk_
      assert _qcsubk_np_tmp.flags.contiguous
      _qcsubk_tmp = ctypes.cast(_qcsubk_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _qcsubk_copyarray = False
      _qcsubk_tmp = None
      
    if qcsubi_ is None:
      raise ValueError("Argument qcsubi cannot be None")
    if qcsubi_ is None:
      raise ValueError("Argument qcsubi may not be None")
    if isinstance(qcsubi_, numpy.ndarray) and qcsubi_.dtype is numpy.dtype(numpy.int32) and qcsubi_.flags.contiguous:
      _qcsubi_copyarray = False
      _qcsubi_tmp = ctypes.cast(qcsubi_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif qcsubi_ is not None:
      _qcsubi_copyarray = True
      _qcsubi_np_tmp = numpy.zeros(len(qcsubi_),numpy.dtype(numpy.int32))
      _qcsubi_np_tmp[:] = qcsubi_
      assert _qcsubi_np_tmp.flags.contiguous
      _qcsubi_tmp = ctypes.cast(_qcsubi_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _qcsubi_copyarray = False
      _qcsubi_tmp = None
      
    if qcsubj_ is None:
      raise ValueError("Argument qcsubj cannot be None")
    if qcsubj_ is None:
      raise ValueError("Argument qcsubj may not be None")
    if isinstance(qcsubj_, numpy.ndarray) and qcsubj_.dtype is numpy.dtype(numpy.int32) and qcsubj_.flags.contiguous:
      _qcsubj_copyarray = False
      _qcsubj_tmp = ctypes.cast(qcsubj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif qcsubj_ is not None:
      _qcsubj_copyarray = True
      _qcsubj_np_tmp = numpy.zeros(len(qcsubj_),numpy.dtype(numpy.int32))
      _qcsubj_np_tmp[:] = qcsubj_
      assert _qcsubj_np_tmp.flags.contiguous
      _qcsubj_tmp = ctypes.cast(_qcsubj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _qcsubj_copyarray = False
      _qcsubj_tmp = None
      
    if qcval_ is None:
      raise ValueError("Argument qcval cannot be None")
    if qcval_ is None:
      raise ValueError("Argument qcval may not be None")
    if isinstance(qcval_, numpy.ndarray) and qcval_.dtype is numpy.dtype(numpy.float64) and qcval_.flags.contiguous:
      _qcval_copyarray = False
      _qcval_tmp = ctypes.cast(qcval_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif qcval_ is not None:
      _qcval_copyarray = True
      _qcval_np_tmp = numpy.zeros(len(qcval_),numpy.dtype(numpy.float64))
      _qcval_np_tmp[:] = qcval_
      assert _qcval_np_tmp.flags.contiguous
      _qcval_tmp = ctypes.cast(_qcval_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _qcval_copyarray = False
      _qcval_tmp = None
      
    res = __library__.MSK_XX_putqcon(self.__nativep,numqcnz_,_qcsubk_tmp,_qcsubi_tmp,_qcsubj_tmp,_qcval_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_make_intvector,_make_intvector,_make_doublevector)
  @synchronized
  def putqconk(self,k_,qcsubi_,qcsubj_,qcval_):
    """
    Replaces all quadratic terms in a single constraint.
  
    putqconk(self,k_,qcsubi_,qcsubj_,qcval_)
      k: int. The constraint in which the new quadratic elements are inserted.
      qcsubi: array of int. Row subscripts for quadratic constraint matrix.
      qcsubj: array of int. Column subscripts for quadratic constraint matrix.
      qcval: array of double. Quadratic constraint coefficient values.
    """
    numqcnz_ = None
    if numqcnz_ is None:
      numqcnz_ = len(qcsubi_)
    elif numqcnz_ != len(qcsubi_):
      raise IndexError("Inconsistent length of array qcsubi")
    if numqcnz_ is None:
      numqcnz_ = len(qcsubj_)
    elif numqcnz_ != len(qcsubj_):
      raise IndexError("Inconsistent length of array qcsubj")
    if numqcnz_ is None:
      numqcnz_ = len(qcval_)
    elif numqcnz_ != len(qcval_):
      raise IndexError("Inconsistent length of array qcval")
    if qcsubi_ is None:
      raise ValueError("Argument qcsubi cannot be None")
    if qcsubi_ is None:
      raise ValueError("Argument qcsubi may not be None")
    if isinstance(qcsubi_, numpy.ndarray) and qcsubi_.dtype is numpy.dtype(numpy.int32) and qcsubi_.flags.contiguous:
      _qcsubi_copyarray = False
      _qcsubi_tmp = ctypes.cast(qcsubi_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif qcsubi_ is not None:
      _qcsubi_copyarray = True
      _qcsubi_np_tmp = numpy.zeros(len(qcsubi_),numpy.dtype(numpy.int32))
      _qcsubi_np_tmp[:] = qcsubi_
      assert _qcsubi_np_tmp.flags.contiguous
      _qcsubi_tmp = ctypes.cast(_qcsubi_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _qcsubi_copyarray = False
      _qcsubi_tmp = None
      
    if qcsubj_ is None:
      raise ValueError("Argument qcsubj cannot be None")
    if qcsubj_ is None:
      raise ValueError("Argument qcsubj may not be None")
    if isinstance(qcsubj_, numpy.ndarray) and qcsubj_.dtype is numpy.dtype(numpy.int32) and qcsubj_.flags.contiguous:
      _qcsubj_copyarray = False
      _qcsubj_tmp = ctypes.cast(qcsubj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif qcsubj_ is not None:
      _qcsubj_copyarray = True
      _qcsubj_np_tmp = numpy.zeros(len(qcsubj_),numpy.dtype(numpy.int32))
      _qcsubj_np_tmp[:] = qcsubj_
      assert _qcsubj_np_tmp.flags.contiguous
      _qcsubj_tmp = ctypes.cast(_qcsubj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _qcsubj_copyarray = False
      _qcsubj_tmp = None
      
    if qcval_ is None:
      raise ValueError("Argument qcval cannot be None")
    if qcval_ is None:
      raise ValueError("Argument qcval may not be None")
    if isinstance(qcval_, numpy.ndarray) and qcval_.dtype is numpy.dtype(numpy.float64) and qcval_.flags.contiguous:
      _qcval_copyarray = False
      _qcval_tmp = ctypes.cast(qcval_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif qcval_ is not None:
      _qcval_copyarray = True
      _qcval_np_tmp = numpy.zeros(len(qcval_),numpy.dtype(numpy.float64))
      _qcval_np_tmp[:] = qcval_
      assert _qcval_np_tmp.flags.contiguous
      _qcval_tmp = ctypes.cast(_qcval_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _qcval_copyarray = False
      _qcval_tmp = None
      
    res = __library__.MSK_XX_putqconk(self.__nativep,k_,numqcnz_,_qcsubi_tmp,_qcsubj_tmp,_qcval_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_intvector,_make_intvector,_make_doublevector)
  @synchronized
  def putqobj(self,qosubi_,qosubj_,qoval_):
    """
    Replaces all quadratic terms in the objective.
  
    putqobj(self,qosubi_,qosubj_,qoval_)
      qosubi: array of int. Row subscripts for quadratic objective coefficients.
      qosubj: array of int. Column subscripts for quadratic objective coefficients.
      qoval: array of double. Quadratic objective coefficient values.
    """
    numqonz_ = None
    if numqonz_ is None:
      numqonz_ = len(qosubi_)
    elif numqonz_ != len(qosubi_):
      raise IndexError("Inconsistent length of array qosubi")
    if numqonz_ is None:
      numqonz_ = len(qosubj_)
    elif numqonz_ != len(qosubj_):
      raise IndexError("Inconsistent length of array qosubj")
    if numqonz_ is None:
      numqonz_ = len(qoval_)
    elif numqonz_ != len(qoval_):
      raise IndexError("Inconsistent length of array qoval")
    if qosubi_ is None:
      raise ValueError("Argument qosubi cannot be None")
    if qosubi_ is None:
      raise ValueError("Argument qosubi may not be None")
    if isinstance(qosubi_, numpy.ndarray) and qosubi_.dtype is numpy.dtype(numpy.int32) and qosubi_.flags.contiguous:
      _qosubi_copyarray = False
      _qosubi_tmp = ctypes.cast(qosubi_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif qosubi_ is not None:
      _qosubi_copyarray = True
      _qosubi_np_tmp = numpy.zeros(len(qosubi_),numpy.dtype(numpy.int32))
      _qosubi_np_tmp[:] = qosubi_
      assert _qosubi_np_tmp.flags.contiguous
      _qosubi_tmp = ctypes.cast(_qosubi_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _qosubi_copyarray = False
      _qosubi_tmp = None
      
    if qosubj_ is None:
      raise ValueError("Argument qosubj cannot be None")
    if qosubj_ is None:
      raise ValueError("Argument qosubj may not be None")
    if isinstance(qosubj_, numpy.ndarray) and qosubj_.dtype is numpy.dtype(numpy.int32) and qosubj_.flags.contiguous:
      _qosubj_copyarray = False
      _qosubj_tmp = ctypes.cast(qosubj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif qosubj_ is not None:
      _qosubj_copyarray = True
      _qosubj_np_tmp = numpy.zeros(len(qosubj_),numpy.dtype(numpy.int32))
      _qosubj_np_tmp[:] = qosubj_
      assert _qosubj_np_tmp.flags.contiguous
      _qosubj_tmp = ctypes.cast(_qosubj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _qosubj_copyarray = False
      _qosubj_tmp = None
      
    if qoval_ is None:
      raise ValueError("Argument qoval cannot be None")
    if qoval_ is None:
      raise ValueError("Argument qoval may not be None")
    if isinstance(qoval_, numpy.ndarray) and qoval_.dtype is numpy.dtype(numpy.float64) and qoval_.flags.contiguous:
      _qoval_copyarray = False
      _qoval_tmp = ctypes.cast(qoval_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif qoval_ is not None:
      _qoval_copyarray = True
      _qoval_np_tmp = numpy.zeros(len(qoval_),numpy.dtype(numpy.float64))
      _qoval_np_tmp[:] = qoval_
      assert _qoval_np_tmp.flags.contiguous
      _qoval_tmp = ctypes.cast(_qoval_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _qoval_copyarray = False
      _qoval_tmp = None
      
    res = __library__.MSK_XX_putqobj(self.__nativep,numqonz_,_qosubi_tmp,_qosubj_tmp,_qoval_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_make_int,_make_double)
  @synchronized
  def putqobjij(self,i_,j_,qoij_):
    """
    Replaces one coefficient in the quadratic term in the objective.
  
    putqobjij(self,i_,j_,qoij_)
      i: int. Row index for the coefficient to be replaced.
      j: int. Column index for the coefficient to be replaced.
      qoij: double. The new coefficient value.
    """
    res = __library__.MSK_XX_putqobjij(self.__nativep,i_,j_,qoij_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype),_make_anyenumvector(stakey),_make_anyenumvector(stakey),_make_anyenumvector(stakey),_make_doublevector,_make_doublevector,_make_doublevector,_make_doublevector,_make_doublevector,_make_doublevector,_make_doublevector,_make_doublevector)
  @synchronized
  def putsolution(self,whichsol_,skc_,skx_,skn_,xc_,xx_,y_,slc_,suc_,slx_,sux_,snx_):
    """
    Inserts a solution.
  
    putsolution(self,whichsol_,skc_,skx_,skn_,xc_,xx_,y_,slc_,suc_,slx_,sux_,snx_)
      whichsol: mosek.soltype. Selects a solution.
      skc: array of mosek.stakey. Status keys for the constraints.
      skx: array of mosek.stakey. Status keys for the variables.
      skn: array of mosek.stakey. Status keys for the conic constraints.
      xc: array of double. Primal constraint solution.
      xx: array of double. Primal variable solution.
      y: array of double. Vector of dual variables corresponding to the constraints.
      slc: array of double. Dual variables corresponding to the lower bounds on the constraints.
      suc: array of double. Dual variables corresponding to the upper bounds on the constraints.
      slx: array of double. Dual variables corresponding to the lower bounds on the variables.
      sux: array of double. Dual variables corresponding to the upper bounds on the variables.
      snx: array of double. Dual variables corresponding to the conic constraints on the variables.
    """
    if skc_ is not None:
        _skc_tmp = (ctypes.c_int32 * len(skc_))(*skc_)
    else:
        _skc_tmp = None
    if skx_ is not None:
        _skx_tmp = (ctypes.c_int32 * len(skx_))(*skx_)
    else:
        _skx_tmp = None
    if skn_ is not None:
        _skn_tmp = (ctypes.c_int32 * len(skn_))(*skn_)
    else:
        _skn_tmp = None
    if isinstance(xc_, numpy.ndarray) and xc_.dtype is numpy.dtype(numpy.float64) and xc_.flags.contiguous:
      _xc_copyarray = False
      _xc_tmp = ctypes.cast(xc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif xc_ is not None:
      _xc_copyarray = True
      _xc_np_tmp = numpy.zeros(len(xc_),numpy.dtype(numpy.float64))
      _xc_np_tmp[:] = xc_
      assert _xc_np_tmp.flags.contiguous
      _xc_tmp = ctypes.cast(_xc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _xc_copyarray = False
      _xc_tmp = None
      
    if isinstance(xx_, numpy.ndarray) and xx_.dtype is numpy.dtype(numpy.float64) and xx_.flags.contiguous:
      _xx_copyarray = False
      _xx_tmp = ctypes.cast(xx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif xx_ is not None:
      _xx_copyarray = True
      _xx_np_tmp = numpy.zeros(len(xx_),numpy.dtype(numpy.float64))
      _xx_np_tmp[:] = xx_
      assert _xx_np_tmp.flags.contiguous
      _xx_tmp = ctypes.cast(_xx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _xx_copyarray = False
      _xx_tmp = None
      
    if isinstance(y_, numpy.ndarray) and y_.dtype is numpy.dtype(numpy.float64) and y_.flags.contiguous:
      _y_copyarray = False
      _y_tmp = ctypes.cast(y_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif y_ is not None:
      _y_copyarray = True
      _y_np_tmp = numpy.zeros(len(y_),numpy.dtype(numpy.float64))
      _y_np_tmp[:] = y_
      assert _y_np_tmp.flags.contiguous
      _y_tmp = ctypes.cast(_y_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _y_copyarray = False
      _y_tmp = None
      
    if isinstance(slc_, numpy.ndarray) and slc_.dtype is numpy.dtype(numpy.float64) and slc_.flags.contiguous:
      _slc_copyarray = False
      _slc_tmp = ctypes.cast(slc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif slc_ is not None:
      _slc_copyarray = True
      _slc_np_tmp = numpy.zeros(len(slc_),numpy.dtype(numpy.float64))
      _slc_np_tmp[:] = slc_
      assert _slc_np_tmp.flags.contiguous
      _slc_tmp = ctypes.cast(_slc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _slc_copyarray = False
      _slc_tmp = None
      
    if isinstance(suc_, numpy.ndarray) and suc_.dtype is numpy.dtype(numpy.float64) and suc_.flags.contiguous:
      _suc_copyarray = False
      _suc_tmp = ctypes.cast(suc_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif suc_ is not None:
      _suc_copyarray = True
      _suc_np_tmp = numpy.zeros(len(suc_),numpy.dtype(numpy.float64))
      _suc_np_tmp[:] = suc_
      assert _suc_np_tmp.flags.contiguous
      _suc_tmp = ctypes.cast(_suc_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _suc_copyarray = False
      _suc_tmp = None
      
    if isinstance(slx_, numpy.ndarray) and slx_.dtype is numpy.dtype(numpy.float64) and slx_.flags.contiguous:
      _slx_copyarray = False
      _slx_tmp = ctypes.cast(slx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif slx_ is not None:
      _slx_copyarray = True
      _slx_np_tmp = numpy.zeros(len(slx_),numpy.dtype(numpy.float64))
      _slx_np_tmp[:] = slx_
      assert _slx_np_tmp.flags.contiguous
      _slx_tmp = ctypes.cast(_slx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _slx_copyarray = False
      _slx_tmp = None
      
    if isinstance(sux_, numpy.ndarray) and sux_.dtype is numpy.dtype(numpy.float64) and sux_.flags.contiguous:
      _sux_copyarray = False
      _sux_tmp = ctypes.cast(sux_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif sux_ is not None:
      _sux_copyarray = True
      _sux_np_tmp = numpy.zeros(len(sux_),numpy.dtype(numpy.float64))
      _sux_np_tmp[:] = sux_
      assert _sux_np_tmp.flags.contiguous
      _sux_tmp = ctypes.cast(_sux_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _sux_copyarray = False
      _sux_tmp = None
      
    if isinstance(snx_, numpy.ndarray) and snx_.dtype is numpy.dtype(numpy.float64) and snx_.flags.contiguous:
      _snx_copyarray = False
      _snx_tmp = ctypes.cast(snx_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif snx_ is not None:
      _snx_copyarray = True
      _snx_np_tmp = numpy.zeros(len(snx_),numpy.dtype(numpy.float64))
      _snx_np_tmp[:] = snx_
      assert _snx_np_tmp.flags.contiguous
      _snx_tmp = ctypes.cast(_snx_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _snx_copyarray = False
      _snx_tmp = None
      
    res = __library__.MSK_XX_putsolution(self.__nativep,whichsol_,_skc_tmp,_skx_tmp,_skn_tmp,_xc_tmp,_xx_tmp,_y_tmp,_slc_tmp,_suc_tmp,_slx_tmp,_sux_tmp,_snx_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_accept_anyenum(soltype),_accept_anyenum(stakey),_make_double,_make_double,_make_double)
  @synchronized
  def putconsolutioni(self,i_,whichsol_,sk_,x_,sl_,su_):
    """
    Sets the primal and dual solution information for a single constraint.
  
    putconsolutioni(self,i_,whichsol_,sk_,x_,sl_,su_)
      i: int. Index of the constraint.
      whichsol: mosek.soltype. Selects a solution.
      sk: mosek.stakey. Status key of the constraint.
      x: double. Primal solution value of the constraint.
      sl: double. Solution value of the dual variable associated with the lower bound.
      su: double. Solution value of the dual variable associated with the upper bound.
    """
    res = __library__.MSK_XX_putconsolutioni(self.__nativep,i_,whichsol_,sk_,x_,sl_,su_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_accept_anyenum(soltype),_accept_anyenum(stakey),_make_double,_make_double,_make_double,_make_double)
  @synchronized
  def putvarsolutionj(self,j_,whichsol_,sk_,x_,sl_,su_,sn_):
    """
    Sets the primal and dual solution information for a single variable.
  
    putvarsolutionj(self,j_,whichsol_,sk_,x_,sl_,su_,sn_)
      j: int. Index of the variable.
      whichsol: mosek.soltype. Selects a solution.
      sk: mosek.stakey. Status key of the variable.
      x: double. Primal solution value of the variable.
      sl: double. Solution value of the dual variable associated with the lower bound.
      su: double. Solution value of the dual variable associated with the upper bound.
      sn: double. Solution value of the dual variable associated with the conic constraint.
    """
    res = __library__.MSK_XX_putvarsolutionj(self.__nativep,j_,whichsol_,sk_,x_,sl_,su_,sn_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_accept_anyenum(soltype),_make_double)
  @synchronized
  def putsolutionyi(self,i_,whichsol_,y_):
    """
    Inputs the dual variable of a solution.
  
    putsolutionyi(self,i_,whichsol_,y_)
      i: int. Index of the dual variable.
      whichsol: mosek.soltype. Selects a solution.
      y: double. Solution value of the dual variable.
    """
    res = __library__.MSK_XX_putsolutionyi(self.__nativep,i_,whichsol_,y_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(sparam),_accept_str)
  @synchronized
  def putstrparam(self,param_,parvalue_):
    """
    Sets a string parameter.
  
    putstrparam(self,param_,parvalue_)
      param: mosek.sparam. Which parameter.
      parvalue: str|unicode. Parameter value.
    """
    if isinstance(parvalue_,unicode):
      parvalue_ = parvalue_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_putstrparam(self.__nativep,param_,parvalue_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_str)
  @synchronized
  def puttaskname(self,taskname_):
    """
    Assigns a new name to the task.
  
    puttaskname(self,taskname_)
      taskname: str|unicode. Name assigned to the task.
    """
    if isinstance(taskname_,unicode):
      taskname_ = taskname_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_puttaskname(self.__nativep,taskname_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_accept_anyenum(variabletype))
  @synchronized
  def putvartype(self,j_,vartype_):
    """
    Sets the variable type of one variable.
  
    putvartype(self,j_,vartype_)
      j: int. Index of the variable.
      vartype: mosek.variabletype. The new variable type.
    """
    res = __library__.MSK_XX_putvartype(self.__nativep,j_,vartype_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_intvector,_make_anyenumvector(variabletype))
  @synchronized
  def putvartypelist(self,subj_,vartype_):
    """
    Sets the variable type for one or more variables.
  
    putvartypelist(self,subj_,vartype_)
      subj: array of int. A list of variable indexes for which the variable type should be changed.
      vartype: array of mosek.variabletype. A list of variable types.
    """
    num_ = None
    if num_ is None:
      num_ = len(subj_)
    elif num_ != len(subj_):
      raise IndexError("Inconsistent length of array subj")
    if num_ is None:
      num_ = len(vartype_)
    elif num_ != len(vartype_):
      raise IndexError("Inconsistent length of array vartype")
    if subj_ is None:
      raise ValueError("Argument subj cannot be None")
    if subj_ is None:
      raise ValueError("Argument subj may not be None")
    if isinstance(subj_, numpy.ndarray) and subj_.dtype is numpy.dtype(numpy.int32) and subj_.flags.contiguous:
      _subj_copyarray = False
      _subj_tmp = ctypes.cast(subj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subj_ is not None:
      _subj_copyarray = True
      _subj_np_tmp = numpy.zeros(len(subj_),numpy.dtype(numpy.int32))
      _subj_np_tmp[:] = subj_
      assert _subj_np_tmp.flags.contiguous
      _subj_tmp = ctypes.cast(_subj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subj_copyarray = False
      _subj_tmp = None
      
    if vartype_ is None:
      raise ValueError("Argument vartype cannot be None")
    if vartype_ is None:
      raise ValueError("Argument vartype may not be None")
    if vartype_ is not None:
        _vartype_tmp = (ctypes.c_int32 * len(vartype_))(*vartype_)
    else:
        _vartype_tmp = None
    res = __library__.MSK_XX_putvartypelist(self.__nativep,num_,_subj_tmp,_vartype_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_str,_accept_anyenum(dataformat),_accept_anyenum(compresstype))
  @synchronized
  def readdataformat(self,filename_,format_,compress_):
    """
    Reads problem data from a file.
  
    readdataformat(self,filename_,format_,compress_)
      filename: str|unicode. A valid file name.
      format: mosek.dataformat. File data format.
      compress: mosek.compresstype. File compression type.
    """
    if isinstance(filename_,unicode):
      filename_ = filename_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_readdataformat(self.__nativep,filename_,format_,compress_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_str)
  @synchronized
  def readdata(self,filename_):
    """
    Reads problem data from a file.
  
    readdata(self,filename_)
      filename: str|unicode. A valid file name.
    """
    if isinstance(filename_,unicode):
      filename_ = filename_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_readdataautoformat(self.__nativep,filename_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_str)
  @synchronized
  def readparamfile(self,filename_):
    """
    Reads a parameter file.
  
    readparamfile(self,filename_)
      filename: str|unicode. A valid file name.
    """
    if isinstance(filename_,unicode):
      filename_ = filename_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_readparamfile(self.__nativep,filename_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype),_accept_str)
  @synchronized
  def readsolution(self,whichsol_,filename_):
    """
    Reads a solution from a file.
  
    readsolution(self,whichsol_,filename_)
      whichsol: mosek.soltype. Selects a solution.
      filename: str|unicode. A valid file name.
    """
    if isinstance(filename_,unicode):
      filename_ = filename_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_readsolution(self.__nativep,whichsol_,filename_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(streamtype))
  @synchronized
  def readsummary(self,whichstream_):
    """
    Prints information about last file read.
  
    readsummary(self,whichstream_)
      whichstream: mosek.streamtype. Index of the stream.
    """
    res = __library__.MSK_XX_readsummary(self.__nativep,whichstream_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_int,_make_int,_make_int,_make_long,_make_long)
  @synchronized
  def resizetask(self,maxnumcon_,maxnumvar_,maxnumcone_,maxnumanz_,maxnumqnz_):
    """
    Resizes an optimization task.
  
    resizetask(self,maxnumcon_,maxnumvar_,maxnumcone_,maxnumanz_,maxnumqnz_)
      maxnumcon: int. New maximum number of constraints.
      maxnumvar: int. New maximum number of variables.
      maxnumcone: int. New maximum number of cones.
      maxnumanz: long. New maximum number of linear non-zero elements.
      maxnumqnz: long. New maximum number of quadratic non-zeros elements.
    """
    res = __library__.MSK_XX_resizetask(self.__nativep,maxnumcon_,maxnumvar_,maxnumcone_,maxnumanz_,maxnumqnz_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_str,_make_int)
  @synchronized
  def checkmem(self,file_,line_):
    """
    Checks the memory allocated by the task.
  
    checkmem(self,file_,line_)
      file: str|unicode. File from which the function is called.
      line: int. Line in the file from which the function is called.
    """
    if isinstance(file_,unicode):
      file_ = file_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_checkmemtask(self.__nativep,file_,line_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any)
  @synchronized
  def getmemusage(self):
    """
    Obtains information about the amount of memory used by a task.
  
    getmemusage(self)
    returns: meminuse,maxmemuse
      meminuse: long. Amount of memory currently used by the task.
      maxmemuse: long. Maximum amount of memory used by the task until now.
    """
    meminuse_ = ctypes.c_int64()
    maxmemuse_ = ctypes.c_int64()
    res = __library__.MSK_XX_getmemusagetask(self.__nativep,ctypes.byref(meminuse_),ctypes.byref(maxmemuse_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    meminuse_ = meminuse_.value
    _meminuse_return_value = meminuse_
    maxmemuse_ = maxmemuse_.value
    _maxmemuse_return_value = maxmemuse_
    return (_meminuse_return_value,_maxmemuse_return_value)
  @accepts(_accept_any)
  @synchronized
  def setdefaults(self):
    """
    Resets all parameter values.
  
    setdefaults(self)
    """
    res = __library__.MSK_XX_setdefaults(self.__nativep)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype))
  @synchronized
  def solutiondef(self,whichsol_):
    """
    Checks whether a solution is defined.
  
    solutiondef(self,whichsol_)
      whichsol: mosek.soltype. Selects a solution.
    returns: isdef
      isdef: int. Is non-zero if the requested solution is defined.
    """
    isdef_ = ctypes.c_int32()
    res = __library__.MSK_XX_solutiondef(self.__nativep,whichsol_,ctypes.byref(isdef_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    isdef_ = isdef_.value
    _isdef_return_value = isdef_
    return (_isdef_return_value)
  @accepts(_accept_any,_accept_anyenum(soltype))
  @synchronized
  def deletesolution(self,whichsol_):
    """
    Undefine a solution and free the memory it uses.
  
    deletesolution(self,whichsol_)
      whichsol: mosek.soltype. Selects a solution.
    """
    res = __library__.MSK_XX_deletesolution(self.__nativep,whichsol_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(streamtype),_accept_anyenum(soltype))
  @synchronized
  def onesolutionsummary(self,whichstream_,whichsol_):
    """
    Prints a short summary of a specified solution.
  
    onesolutionsummary(self,whichstream_,whichsol_)
      whichstream: mosek.streamtype. Index of the stream.
      whichsol: mosek.soltype. Selects a solution.
    """
    res = __library__.MSK_XX_onesolutionsummary(self.__nativep,whichstream_,whichsol_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(streamtype))
  @synchronized
  def solutionsummary(self,whichstream_):
    """
    Prints a short summary of the current solutions.
  
    solutionsummary(self,whichstream_)
      whichstream: mosek.streamtype. Index of the stream.
    """
    res = __library__.MSK_XX_solutionsummary(self.__nativep,whichstream_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype))
  @synchronized
  def updatesolutioninfo(self,whichsol_):
    """
    Update the information items related to the solution.
  
    updatesolutioninfo(self,whichsol_)
      whichsol: mosek.soltype. Selects a solution.
    """
    res = __library__.MSK_XX_updatesolutioninfo(self.__nativep,whichsol_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(streamtype))
  @synchronized
  def optimizersummary(self,whichstream_):
    """
    Prints a short summary with optimizer statistics from last optimization.
  
    optimizersummary(self,whichstream_)
      whichstream: mosek.streamtype. Index of the stream.
    """
    res = __library__.MSK_XX_optimizersummary(self.__nativep,whichstream_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_str)
  @synchronized
  def strtoconetype(self,str_):
    """
    Obtains a cone type code.
  
    strtoconetype(self,str_)
      str: str|unicode. String corresponding to the cone type code.
    returns: conetype
      conetype: mosek.conetype. The cone type corresponding to str.
    """
    if isinstance(str_,unicode):
      str_ = str_.encode("utf-8",errors="replace")
    conetype_ = ctypes.c_int32()
    res = __library__.MSK_XX_strtoconetype(self.__nativep,str_,ctypes.byref(conetype_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _conetype_return_value = conetype(conetype_.value)
    return (_conetype_return_value)
  @accepts(_accept_any,_accept_str)
  @synchronized
  def strtosk(self,str_):
    """
    Obtains a status key.
  
    strtosk(self,str_)
      str: str|unicode. A status key abbreviation string.
    returns: sk
      sk: mosek.stakey. Status key corresponding to the string.
    """
    if isinstance(str_,unicode):
      str_ = str_.encode("utf-8",errors="replace")
    sk_ = ctypes.c_int32()
    res = __library__.MSK_XX_strtosk(self.__nativep,str_,ctypes.byref(sk_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _sk_return_value = stakey(sk_.value)
    return (_sk_return_value)
  @accepts(_accept_any,_accept_str)
  @synchronized
  def writedata(self,filename_):
    """
    Writes problem data to a file.
  
    writedata(self,filename_)
      filename: str|unicode. A valid file name.
    """
    if isinstance(filename_,unicode):
      filename_ = filename_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_writedata(self.__nativep,filename_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_str)
  @synchronized
  def writetask(self,filename_):
    """
    Write a complete binary dump of the task data.
  
    writetask(self,filename_)
      filename: str|unicode. A valid file name.
    """
    if isinstance(filename_,unicode):
      filename_ = filename_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_writetask(self.__nativep,filename_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_str)
  @synchronized
  def readtask(self,filename_):
    """
    Load task data from a file.
  
    readtask(self,filename_)
      filename: str|unicode. A valid file name.
    """
    if isinstance(filename_,unicode):
      filename_ = filename_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_readtask(self.__nativep,filename_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_str)
  @synchronized
  def readopfstring(self,data_):
    """
    Load task data from a string in OPF format.
  
    readopfstring(self,data_)
      data: str|unicode. Problem data in text format.
    """
    if isinstance(data_,unicode):
      data_ = data_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_readopfstring(self.__nativep,data_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_str)
  @synchronized
  def readlpstring(self,data_):
    """
    Load task data from a string in LP format.
  
    readlpstring(self,data_)
      data: str|unicode. Problem data in text format.
    """
    if isinstance(data_,unicode):
      data_ = data_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_readlpstring(self.__nativep,data_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_str)
  @synchronized
  def readjsonstring(self,data_):
    """
    Load task data from a string in JSON format.
  
    readjsonstring(self,data_)
      data: str|unicode. Problem data in text format.
    """
    if isinstance(data_,unicode):
      data_ = data_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_readjsonstring(self.__nativep,data_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_str)
  @synchronized
  def readptfstring(self,data_):
    """
    Load task data from a string in PTF format.
  
    readptfstring(self,data_)
      data: str|unicode. Problem data in text format.
    """
    if isinstance(data_,unicode):
      data_ = data_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_readptfstring(self.__nativep,data_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_str)
  @synchronized
  def writeparamfile(self,filename_):
    """
    Writes all the parameters to a parameter file.
  
    writeparamfile(self,filename_)
      filename: str|unicode. A valid file name.
    """
    if isinstance(filename_,unicode):
      filename_ = filename_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_writeparamfile(self.__nativep,filename_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_anyenum(soltype))
  @synchronized
  def getinfeasiblesubproblem(self,whichsol_):
    """
    Obtains an infeasible subproblem.
  
    getinfeasiblesubproblem(self,whichsol_)
      whichsol: mosek.soltype. Which solution to use when determining the infeasible subproblem.
    returns: inftask
      inftask: Task. A new task containing the infeasible subproblem.
    """
    inftask_ = ctypes.c_void_p()
    res = __library__.MSK_XX_getinfeasiblesubproblem(self.__nativep,whichsol_,ctypes.byref(inftask_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _inftask_return_value = Task(nativep = inftask_)
    return (_inftask_return_value)
  @accepts(_accept_any,_accept_anyenum(soltype),_accept_str)
  @synchronized
  def writesolution(self,whichsol_,filename_):
    """
    Write a solution to a file.
  
    writesolution(self,whichsol_,filename_)
      whichsol: mosek.soltype. Selects a solution.
      filename: str|unicode. A valid file name.
    """
    if isinstance(filename_,unicode):
      filename_ = filename_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_writesolution(self.__nativep,whichsol_,filename_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_str)
  @synchronized
  def writejsonsol(self,filename_):
    """
    Writes a solution to a JSON file.
  
    writejsonsol(self,filename_)
      filename: str|unicode. A valid file name.
    """
    if isinstance(filename_,unicode):
      filename_ = filename_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_writejsonsol(self.__nativep,filename_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_intvector,_make_anyenumvector(mark),_make_intvector,_make_anyenumvector(mark),_accept_doublevector,_accept_doublevector,_accept_doublevector,_accept_doublevector,_accept_doublevector,_accept_doublevector,_accept_doublevector,_accept_doublevector)
  @synchronized
  def primalsensitivity(self,subi_,marki_,subj_,markj_,leftpricei_,rightpricei_,leftrangei_,rightrangei_,leftpricej_,rightpricej_,leftrangej_,rightrangej_):
    """
    Perform sensitivity analysis on bounds.
  
    primalsensitivity(self,subi_,marki_,subj_,markj_,leftpricei_,rightpricei_,leftrangei_,rightrangei_,leftpricej_,rightpricej_,leftrangej_,rightrangej_)
      subi: array of int. Indexes of constraints to analyze.
      marki: array of mosek.mark. Mark which constraint bounds to analyze.
      subj: array of int. Indexes of variables to analyze.
      markj: array of mosek.mark. Mark which variable bounds to analyze.
      leftpricei: array of double. Left shadow price for constraints.
      rightpricei: array of double. Right shadow price for constraints.
      leftrangei: array of double. Left range for constraints.
      rightrangei: array of double. Right range for constraints.
      leftpricej: array of double. Left shadow price for variables.
      rightpricej: array of double. Right shadow price for variables.
      leftrangej: array of double. Left range for variables.
      rightrangej: array of double. Right range for variables.
    """
    numi_ = None
    if numi_ is None:
      numi_ = len(subi_)
    elif numi_ != len(subi_):
      raise IndexError("Inconsistent length of array subi")
    if numi_ is None:
      numi_ = len(marki_)
    elif numi_ != len(marki_):
      raise IndexError("Inconsistent length of array marki")
    if subi_ is None:
      raise ValueError("Argument subi cannot be None")
    if subi_ is None:
      raise ValueError("Argument subi may not be None")
    if isinstance(subi_, numpy.ndarray) and subi_.dtype is numpy.dtype(numpy.int32) and subi_.flags.contiguous:
      _subi_copyarray = False
      _subi_tmp = ctypes.cast(subi_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subi_ is not None:
      _subi_copyarray = True
      _subi_np_tmp = numpy.zeros(len(subi_),numpy.dtype(numpy.int32))
      _subi_np_tmp[:] = subi_
      assert _subi_np_tmp.flags.contiguous
      _subi_tmp = ctypes.cast(_subi_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subi_copyarray = False
      _subi_tmp = None
      
    if marki_ is None:
      raise ValueError("Argument marki cannot be None")
    if marki_ is None:
      raise ValueError("Argument marki may not be None")
    if marki_ is not None:
        _marki_tmp = (ctypes.c_int32 * len(marki_))(*marki_)
    else:
        _marki_tmp = None
    numj_ = None
    if numj_ is None:
      numj_ = len(subj_)
    elif numj_ != len(subj_):
      raise IndexError("Inconsistent length of array subj")
    if numj_ is None:
      numj_ = len(markj_)
    elif numj_ != len(markj_):
      raise IndexError("Inconsistent length of array markj")
    if subj_ is None:
      raise ValueError("Argument subj cannot be None")
    if subj_ is None:
      raise ValueError("Argument subj may not be None")
    if isinstance(subj_, numpy.ndarray) and subj_.dtype is numpy.dtype(numpy.int32) and subj_.flags.contiguous:
      _subj_copyarray = False
      _subj_tmp = ctypes.cast(subj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subj_ is not None:
      _subj_copyarray = True
      _subj_np_tmp = numpy.zeros(len(subj_),numpy.dtype(numpy.int32))
      _subj_np_tmp[:] = subj_
      assert _subj_np_tmp.flags.contiguous
      _subj_tmp = ctypes.cast(_subj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subj_copyarray = False
      _subj_tmp = None
      
    if markj_ is None:
      raise ValueError("Argument markj cannot be None")
    if markj_ is None:
      raise ValueError("Argument markj may not be None")
    if markj_ is not None:
        _markj_tmp = (ctypes.c_int32 * len(markj_))(*markj_)
    else:
        _markj_tmp = None
    _leftpricei_minlength = (numi_)
    if (numi_) > 0 and leftpricei_ is not None and len(leftpricei_) != (numi_):
      raise ValueError("Array argument leftpricei is not long enough: Is %d, expected %d" % (len(leftpricei_),(numi_)))
    if isinstance(leftpricei_,numpy.ndarray) and not leftpricei_.flags.writeable:
      raise ValueError("Argument leftpricei must be writable")
    if isinstance(leftpricei_, numpy.ndarray) and leftpricei_.dtype is numpy.dtype(numpy.float64) and leftpricei_.flags.contiguous:
      _leftpricei_copyarray = False
      _leftpricei_tmp = ctypes.cast(leftpricei_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif leftpricei_ is not None:
      _leftpricei_copyarray = True
      _leftpricei_np_tmp = numpy.zeros(len(leftpricei_),numpy.dtype(numpy.float64))
      _leftpricei_np_tmp[:] = leftpricei_
      assert _leftpricei_np_tmp.flags.contiguous
      _leftpricei_tmp = ctypes.cast(_leftpricei_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _leftpricei_copyarray = False
      _leftpricei_tmp = None
      
    _rightpricei_minlength = (numi_)
    if (numi_) > 0 and rightpricei_ is not None and len(rightpricei_) != (numi_):
      raise ValueError("Array argument rightpricei is not long enough: Is %d, expected %d" % (len(rightpricei_),(numi_)))
    if isinstance(rightpricei_,numpy.ndarray) and not rightpricei_.flags.writeable:
      raise ValueError("Argument rightpricei must be writable")
    if isinstance(rightpricei_, numpy.ndarray) and rightpricei_.dtype is numpy.dtype(numpy.float64) and rightpricei_.flags.contiguous:
      _rightpricei_copyarray = False
      _rightpricei_tmp = ctypes.cast(rightpricei_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif rightpricei_ is not None:
      _rightpricei_copyarray = True
      _rightpricei_np_tmp = numpy.zeros(len(rightpricei_),numpy.dtype(numpy.float64))
      _rightpricei_np_tmp[:] = rightpricei_
      assert _rightpricei_np_tmp.flags.contiguous
      _rightpricei_tmp = ctypes.cast(_rightpricei_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _rightpricei_copyarray = False
      _rightpricei_tmp = None
      
    _leftrangei_minlength = (numi_)
    if (numi_) > 0 and leftrangei_ is not None and len(leftrangei_) != (numi_):
      raise ValueError("Array argument leftrangei is not long enough: Is %d, expected %d" % (len(leftrangei_),(numi_)))
    if isinstance(leftrangei_,numpy.ndarray) and not leftrangei_.flags.writeable:
      raise ValueError("Argument leftrangei must be writable")
    if isinstance(leftrangei_, numpy.ndarray) and leftrangei_.dtype is numpy.dtype(numpy.float64) and leftrangei_.flags.contiguous:
      _leftrangei_copyarray = False
      _leftrangei_tmp = ctypes.cast(leftrangei_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif leftrangei_ is not None:
      _leftrangei_copyarray = True
      _leftrangei_np_tmp = numpy.zeros(len(leftrangei_),numpy.dtype(numpy.float64))
      _leftrangei_np_tmp[:] = leftrangei_
      assert _leftrangei_np_tmp.flags.contiguous
      _leftrangei_tmp = ctypes.cast(_leftrangei_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _leftrangei_copyarray = False
      _leftrangei_tmp = None
      
    _rightrangei_minlength = (numi_)
    if (numi_) > 0 and rightrangei_ is not None and len(rightrangei_) != (numi_):
      raise ValueError("Array argument rightrangei is not long enough: Is %d, expected %d" % (len(rightrangei_),(numi_)))
    if isinstance(rightrangei_,numpy.ndarray) and not rightrangei_.flags.writeable:
      raise ValueError("Argument rightrangei must be writable")
    if isinstance(rightrangei_, numpy.ndarray) and rightrangei_.dtype is numpy.dtype(numpy.float64) and rightrangei_.flags.contiguous:
      _rightrangei_copyarray = False
      _rightrangei_tmp = ctypes.cast(rightrangei_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif rightrangei_ is not None:
      _rightrangei_copyarray = True
      _rightrangei_np_tmp = numpy.zeros(len(rightrangei_),numpy.dtype(numpy.float64))
      _rightrangei_np_tmp[:] = rightrangei_
      assert _rightrangei_np_tmp.flags.contiguous
      _rightrangei_tmp = ctypes.cast(_rightrangei_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _rightrangei_copyarray = False
      _rightrangei_tmp = None
      
    _leftpricej_minlength = (numj_)
    if (numj_) > 0 and leftpricej_ is not None and len(leftpricej_) != (numj_):
      raise ValueError("Array argument leftpricej is not long enough: Is %d, expected %d" % (len(leftpricej_),(numj_)))
    if isinstance(leftpricej_,numpy.ndarray) and not leftpricej_.flags.writeable:
      raise ValueError("Argument leftpricej must be writable")
    if isinstance(leftpricej_, numpy.ndarray) and leftpricej_.dtype is numpy.dtype(numpy.float64) and leftpricej_.flags.contiguous:
      _leftpricej_copyarray = False
      _leftpricej_tmp = ctypes.cast(leftpricej_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif leftpricej_ is not None:
      _leftpricej_copyarray = True
      _leftpricej_np_tmp = numpy.zeros(len(leftpricej_),numpy.dtype(numpy.float64))
      _leftpricej_np_tmp[:] = leftpricej_
      assert _leftpricej_np_tmp.flags.contiguous
      _leftpricej_tmp = ctypes.cast(_leftpricej_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _leftpricej_copyarray = False
      _leftpricej_tmp = None
      
    _rightpricej_minlength = (numj_)
    if (numj_) > 0 and rightpricej_ is not None and len(rightpricej_) != (numj_):
      raise ValueError("Array argument rightpricej is not long enough: Is %d, expected %d" % (len(rightpricej_),(numj_)))
    if isinstance(rightpricej_,numpy.ndarray) and not rightpricej_.flags.writeable:
      raise ValueError("Argument rightpricej must be writable")
    if isinstance(rightpricej_, numpy.ndarray) and rightpricej_.dtype is numpy.dtype(numpy.float64) and rightpricej_.flags.contiguous:
      _rightpricej_copyarray = False
      _rightpricej_tmp = ctypes.cast(rightpricej_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif rightpricej_ is not None:
      _rightpricej_copyarray = True
      _rightpricej_np_tmp = numpy.zeros(len(rightpricej_),numpy.dtype(numpy.float64))
      _rightpricej_np_tmp[:] = rightpricej_
      assert _rightpricej_np_tmp.flags.contiguous
      _rightpricej_tmp = ctypes.cast(_rightpricej_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _rightpricej_copyarray = False
      _rightpricej_tmp = None
      
    _leftrangej_minlength = (numj_)
    if (numj_) > 0 and leftrangej_ is not None and len(leftrangej_) != (numj_):
      raise ValueError("Array argument leftrangej is not long enough: Is %d, expected %d" % (len(leftrangej_),(numj_)))
    if isinstance(leftrangej_,numpy.ndarray) and not leftrangej_.flags.writeable:
      raise ValueError("Argument leftrangej must be writable")
    if isinstance(leftrangej_, numpy.ndarray) and leftrangej_.dtype is numpy.dtype(numpy.float64) and leftrangej_.flags.contiguous:
      _leftrangej_copyarray = False
      _leftrangej_tmp = ctypes.cast(leftrangej_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif leftrangej_ is not None:
      _leftrangej_copyarray = True
      _leftrangej_np_tmp = numpy.zeros(len(leftrangej_),numpy.dtype(numpy.float64))
      _leftrangej_np_tmp[:] = leftrangej_
      assert _leftrangej_np_tmp.flags.contiguous
      _leftrangej_tmp = ctypes.cast(_leftrangej_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _leftrangej_copyarray = False
      _leftrangej_tmp = None
      
    _rightrangej_minlength = (numj_)
    if (numj_) > 0 and rightrangej_ is not None and len(rightrangej_) != (numj_):
      raise ValueError("Array argument rightrangej is not long enough: Is %d, expected %d" % (len(rightrangej_),(numj_)))
    if isinstance(rightrangej_,numpy.ndarray) and not rightrangej_.flags.writeable:
      raise ValueError("Argument rightrangej must be writable")
    if isinstance(rightrangej_, numpy.ndarray) and rightrangej_.dtype is numpy.dtype(numpy.float64) and rightrangej_.flags.contiguous:
      _rightrangej_copyarray = False
      _rightrangej_tmp = ctypes.cast(rightrangej_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif rightrangej_ is not None:
      _rightrangej_copyarray = True
      _rightrangej_np_tmp = numpy.zeros(len(rightrangej_),numpy.dtype(numpy.float64))
      _rightrangej_np_tmp[:] = rightrangej_
      assert _rightrangej_np_tmp.flags.contiguous
      _rightrangej_tmp = ctypes.cast(_rightrangej_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _rightrangej_copyarray = False
      _rightrangej_tmp = None
      
    res = __library__.MSK_XX_primalsensitivity(self.__nativep,numi_,_subi_tmp,_marki_tmp,numj_,_subj_tmp,_markj_tmp,_leftpricei_tmp,_rightpricei_tmp,_leftrangei_tmp,_rightrangei_tmp,_leftpricej_tmp,_rightpricej_tmp,_leftrangej_tmp,_rightrangej_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _leftpricei_copyarray:
      leftpricei_[:] = _leftpricei_np_tmp
    if _rightpricei_copyarray:
      rightpricei_[:] = _rightpricei_np_tmp
    if _leftrangei_copyarray:
      leftrangei_[:] = _leftrangei_np_tmp
    if _rightrangei_copyarray:
      rightrangei_[:] = _rightrangei_np_tmp
    if _leftpricej_copyarray:
      leftpricej_[:] = _leftpricej_np_tmp
    if _rightpricej_copyarray:
      rightpricej_[:] = _rightpricej_np_tmp
    if _leftrangej_copyarray:
      leftrangej_[:] = _leftrangej_np_tmp
    if _rightrangej_copyarray:
      rightrangej_[:] = _rightrangej_np_tmp
  @accepts(_accept_any,_accept_anyenum(streamtype))
  @synchronized
  def sensitivityreport(self,whichstream_):
    """
    Creates a sensitivity report.
  
    sensitivityreport(self,whichstream_)
      whichstream: mosek.streamtype. Index of the stream.
    """
    res = __library__.MSK_XX_sensitivityreport(self.__nativep,whichstream_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_make_intvector,_accept_doublevector,_accept_doublevector,_accept_doublevector,_accept_doublevector)
  @synchronized
  def dualsensitivity(self,subj_,leftpricej_,rightpricej_,leftrangej_,rightrangej_):
    """
    Performs sensitivity analysis on objective coefficients.
  
    dualsensitivity(self,subj_,leftpricej_,rightpricej_,leftrangej_,rightrangej_)
      subj: array of int. Indexes of objective coefficients to analyze.
      leftpricej: array of double. Left shadow prices for requested coefficients.
      rightpricej: array of double. Right shadow prices for requested coefficients.
      leftrangej: array of double. Left range for requested coefficients.
      rightrangej: array of double. Right range for requested coefficients.
    """
    numj_ = None
    if numj_ is None:
      numj_ = len(subj_)
    elif numj_ != len(subj_):
      raise IndexError("Inconsistent length of array subj")
    if subj_ is None:
      raise ValueError("Argument subj cannot be None")
    if subj_ is None:
      raise ValueError("Argument subj may not be None")
    if isinstance(subj_, numpy.ndarray) and subj_.dtype is numpy.dtype(numpy.int32) and subj_.flags.contiguous:
      _subj_copyarray = False
      _subj_tmp = ctypes.cast(subj_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    elif subj_ is not None:
      _subj_copyarray = True
      _subj_np_tmp = numpy.zeros(len(subj_),numpy.dtype(numpy.int32))
      _subj_np_tmp[:] = subj_
      assert _subj_np_tmp.flags.contiguous
      _subj_tmp = ctypes.cast(_subj_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_int32))
    else:
      _subj_copyarray = False
      _subj_tmp = None
      
    _leftpricej_minlength = (numj_)
    if (numj_) > 0 and leftpricej_ is not None and len(leftpricej_) != (numj_):
      raise ValueError("Array argument leftpricej is not long enough: Is %d, expected %d" % (len(leftpricej_),(numj_)))
    if isinstance(leftpricej_,numpy.ndarray) and not leftpricej_.flags.writeable:
      raise ValueError("Argument leftpricej must be writable")
    if isinstance(leftpricej_, numpy.ndarray) and leftpricej_.dtype is numpy.dtype(numpy.float64) and leftpricej_.flags.contiguous:
      _leftpricej_copyarray = False
      _leftpricej_tmp = ctypes.cast(leftpricej_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif leftpricej_ is not None:
      _leftpricej_copyarray = True
      _leftpricej_np_tmp = numpy.zeros(len(leftpricej_),numpy.dtype(numpy.float64))
      _leftpricej_np_tmp[:] = leftpricej_
      assert _leftpricej_np_tmp.flags.contiguous
      _leftpricej_tmp = ctypes.cast(_leftpricej_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _leftpricej_copyarray = False
      _leftpricej_tmp = None
      
    _rightpricej_minlength = (numj_)
    if (numj_) > 0 and rightpricej_ is not None and len(rightpricej_) != (numj_):
      raise ValueError("Array argument rightpricej is not long enough: Is %d, expected %d" % (len(rightpricej_),(numj_)))
    if isinstance(rightpricej_,numpy.ndarray) and not rightpricej_.flags.writeable:
      raise ValueError("Argument rightpricej must be writable")
    if isinstance(rightpricej_, numpy.ndarray) and rightpricej_.dtype is numpy.dtype(numpy.float64) and rightpricej_.flags.contiguous:
      _rightpricej_copyarray = False
      _rightpricej_tmp = ctypes.cast(rightpricej_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif rightpricej_ is not None:
      _rightpricej_copyarray = True
      _rightpricej_np_tmp = numpy.zeros(len(rightpricej_),numpy.dtype(numpy.float64))
      _rightpricej_np_tmp[:] = rightpricej_
      assert _rightpricej_np_tmp.flags.contiguous
      _rightpricej_tmp = ctypes.cast(_rightpricej_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _rightpricej_copyarray = False
      _rightpricej_tmp = None
      
    _leftrangej_minlength = (numj_)
    if (numj_) > 0 and leftrangej_ is not None and len(leftrangej_) != (numj_):
      raise ValueError("Array argument leftrangej is not long enough: Is %d, expected %d" % (len(leftrangej_),(numj_)))
    if isinstance(leftrangej_,numpy.ndarray) and not leftrangej_.flags.writeable:
      raise ValueError("Argument leftrangej must be writable")
    if isinstance(leftrangej_, numpy.ndarray) and leftrangej_.dtype is numpy.dtype(numpy.float64) and leftrangej_.flags.contiguous:
      _leftrangej_copyarray = False
      _leftrangej_tmp = ctypes.cast(leftrangej_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif leftrangej_ is not None:
      _leftrangej_copyarray = True
      _leftrangej_np_tmp = numpy.zeros(len(leftrangej_),numpy.dtype(numpy.float64))
      _leftrangej_np_tmp[:] = leftrangej_
      assert _leftrangej_np_tmp.flags.contiguous
      _leftrangej_tmp = ctypes.cast(_leftrangej_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _leftrangej_copyarray = False
      _leftrangej_tmp = None
      
    _rightrangej_minlength = (numj_)
    if (numj_) > 0 and rightrangej_ is not None and len(rightrangej_) != (numj_):
      raise ValueError("Array argument rightrangej is not long enough: Is %d, expected %d" % (len(rightrangej_),(numj_)))
    if isinstance(rightrangej_,numpy.ndarray) and not rightrangej_.flags.writeable:
      raise ValueError("Argument rightrangej must be writable")
    if isinstance(rightrangej_, numpy.ndarray) and rightrangej_.dtype is numpy.dtype(numpy.float64) and rightrangej_.flags.contiguous:
      _rightrangej_copyarray = False
      _rightrangej_tmp = ctypes.cast(rightrangej_.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    elif rightrangej_ is not None:
      _rightrangej_copyarray = True
      _rightrangej_np_tmp = numpy.zeros(len(rightrangej_),numpy.dtype(numpy.float64))
      _rightrangej_np_tmp[:] = rightrangej_
      assert _rightrangej_np_tmp.flags.contiguous
      _rightrangej_tmp = ctypes.cast(_rightrangej_np_tmp.ctypes._as_parameter_,ctypes.POINTER(ctypes.c_double))
    else:
      _rightrangej_copyarray = False
      _rightrangej_tmp = None
      
    res = __library__.MSK_XX_dualsensitivity(self.__nativep,numj_,_subj_tmp,_leftpricej_tmp,_rightpricej_tmp,_leftrangej_tmp,_rightrangej_tmp)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    if _leftpricej_copyarray:
      leftpricej_[:] = _leftpricej_np_tmp
    if _rightpricej_copyarray:
      rightpricej_[:] = _rightpricej_np_tmp
    if _leftrangej_copyarray:
      leftrangej_[:] = _leftrangej_np_tmp
    if _rightrangej_copyarray:
      rightrangej_[:] = _rightrangej_np_tmp
  @accepts(_accept_any,_accept_str,_accept_str)
  @synchronized
  def optimizermt(self,server_,port_):
    """
    Offload the optimization task to a solver server.
  
    optimizermt(self,server_,port_)
      server: str|unicode. Name or IP address of the solver server.
      port: str|unicode. Network port of the solver server.
    returns: trmcode
      trmcode: mosek.rescode. Is either OK or a termination response code.
    """
    if isinstance(server_,unicode):
      server_ = server_.encode("utf-8",errors="replace")
    if isinstance(port_,unicode):
      port_ = port_.encode("utf-8",errors="replace")
    trmcode_ = ctypes.c_int32()
    res = __library__.MSK_XX_optimizermt(self.__nativep,server_,port_,ctypes.byref(trmcode_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _trmcode_return_value = rescode(trmcode_.value)
    return (_trmcode_return_value)
  @accepts(_accept_any,_accept_str,_accept_str)
  @synchronized
  def asyncoptimize(self,server_,port_):
    """
    Offload the optimization task to a solver server.
  
    asyncoptimize(self,server_,port_)
      server: str|unicode. Name or IP address of the solver server
      port: str|unicode. Network port of the solver service
    returns: token
      token: unicode. Returns the task token
    """
    if isinstance(server_,unicode):
      server_ = server_.encode("utf-8",errors="replace")
    if isinstance(port_,unicode):
      port_ = port_.encode("utf-8",errors="replace")
    token_ = (ctypes.c_char * 33)()
    res = __library__.MSK_XX_asyncoptimize(self.__nativep,server_,port_,token_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    _token_retval = token_.value.decode("utf-8",errors="replace")
    return (_token_retval)
  @accepts(_accept_any,_accept_str,_accept_str,_accept_str)
  @synchronized
  def asyncstop(self,server_,port_,token_):
    """
    Request that the job identified by the token is terminated.
  
    asyncstop(self,server_,port_,token_)
      server: str|unicode. Name or IP address of the solver server
      port: str|unicode. Network port of the solver service
      token: str|unicode. The task token
    """
    if isinstance(server_,unicode):
      server_ = server_.encode("utf-8",errors="replace")
    if isinstance(port_,unicode):
      port_ = port_.encode("utf-8",errors="replace")
    if isinstance(token_,unicode):
      token_ = token_.encode("utf-8",errors="replace")
    res = __library__.MSK_XX_asyncstop(self.__nativep,server_,port_,token_)
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
  @accepts(_accept_any,_accept_str,_accept_str,_accept_str)
  @synchronized
  def asyncpoll(self,server_,port_,token_):
    """
    Requests information about the status of the remote job.
  
    asyncpoll(self,server_,port_,token_)
      server: str|unicode. Name or IP address of the solver server
      port: str|unicode. Network port of the solver service
      token: str|unicode. The task token
    returns: respavailable,resp,trm
      respavailable: int. Indicates if a remote response is available.
      resp: mosek.rescode. Is the response code from the remote solver.
      trm: mosek.rescode. Is either OK or a termination response code.
    """
    if isinstance(server_,unicode):
      server_ = server_.encode("utf-8",errors="replace")
    if isinstance(port_,unicode):
      port_ = port_.encode("utf-8",errors="replace")
    if isinstance(token_,unicode):
      token_ = token_.encode("utf-8",errors="replace")
    respavailable_ = ctypes.c_int32()
    resp_ = ctypes.c_int32()
    trm_ = ctypes.c_int32()
    res = __library__.MSK_XX_asyncpoll(self.__nativep,server_,port_,token_,ctypes.byref(respavailable_),ctypes.byref(resp_),ctypes.byref(trm_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    respavailable_ = respavailable_.value
    _respavailable_return_value = respavailable_
    _resp_return_value = rescode(resp_.value)
    _trm_return_value = rescode(trm_.value)
    return (_respavailable_return_value,_resp_return_value,_trm_return_value)
  @accepts(_accept_any,_accept_str,_accept_str,_accept_str)
  @synchronized
  def asyncgetresult(self,server_,port_,token_):
    """
    Request a response from a remote job.
  
    asyncgetresult(self,server_,port_,token_)
      server: str|unicode. Name or IP address of the solver server.
      port: str|unicode. Network port of the solver service.
      token: str|unicode. The task token.
    returns: respavailable,resp,trm
      respavailable: int. Indicates if a remote response is available.
      resp: mosek.rescode. Is the response code from the remote solver.
      trm: mosek.rescode. Is either OK or a termination response code.
    """
    if isinstance(server_,unicode):
      server_ = server_.encode("utf-8",errors="replace")
    if isinstance(port_,unicode):
      port_ = port_.encode("utf-8",errors="replace")
    if isinstance(token_,unicode):
      token_ = token_.encode("utf-8",errors="replace")
    respavailable_ = ctypes.c_int32()
    resp_ = ctypes.c_int32()
    trm_ = ctypes.c_int32()
    res = __library__.MSK_XX_asyncgetresult(self.__nativep,server_,port_,token_,ctypes.byref(respavailable_),ctypes.byref(resp_),ctypes.byref(trm_))
    if res != 0:
      _,msg = self.__getlasterror(res)
      raise Error(rescode(res),msg)
    respavailable_ = respavailable_.value
    _respavailable_return_value = respavailable_
    _resp_return_value = rescode(resp_.value)
    _trm_return_value = rescode(trm_.value)
    return (_respavailable_return_value,_resp_return_value,_trm_return_value)

class LinAlg:
  __env = Env()

  axpy = __env.axpy
  dot  = __env.dot
  gemv = __env.gemv
  gemm = __env.gemm
  syrk = __env.syrk
  syeig = __env.syeig
  syevd = __env.syevd
  potrf = __env.potrf
