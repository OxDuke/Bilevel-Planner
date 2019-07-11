from distutils.core import setup,Extension
from distutils.command.install import INSTALL_SCHEMES
import distutils.cmd
import platform,sys,shutil
import os,os.path
import subprocess
import ctypes



major,minor,_,_,_ = sys.version_info
if major != 3 or minor < 3:
    print("Python 3.3+ required, got %d.%d" % (major,minor))
    sys.exit(1)

setup( name             = 'Mosek',
       version          = '$(version)',
       description      = 'Mosek/Python APIs',
       long_description = 'Interface for MOSEK',
       author           = 'Mosek ApS',
       author_email     = "support@mosek.com",
       license          = "See license.pdf in the MOSEK distribution",
       url              = 'http://www.mosek.com',
       packages         = [ 'mosek', 'mosek.fusion','mosek.fusion.impl' ],
       ext_modules      = [ Extension('mosek._msk',
                                      include_dirs = ["src/",'../../h'],
                                      libraries = ['mosek64'],
                                      library_dirs = ['../../bin'],
                                      sources = ['src/_msk.c']),
                            Extension('mosek.fusion.impl.fragments',
                                      sources = ['src/mosek-py.c',
                                                 'src/Tools.c']) ])

