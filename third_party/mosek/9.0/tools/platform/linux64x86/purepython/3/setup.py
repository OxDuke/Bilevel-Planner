from distutils.core import setup
import distutils.command.install
import platform,sys
import os,os.path
import subprocess
import ctypes

class InstallationError(Exception): pass

major,minor,_,_,_ = sys.version_info
if major != 3:
    print ("Python 3.0+ required, got %d.%d" % (major,minor))

instdir = os.path.abspath(os.path.join(__file__,'..'))
mosekinstdir = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..','bin'))

ver = ('|MSKMAJORVER|','|MSKMINORVER|')

libd = {
    'osx64x86'   : [ 'libmosek64.%s.%s.dylib'%ver, 'libmosekxx%s_%s.dylib'%ver,'libcilkrts.5.dylib', ],
    'linux64x86' : [ 'libmosek64.so.%s.%s'   %ver, 'libmosekxx%s_%s.so'%ver,   'libcilkrts.so.5',    ],
    'win64x86'   : [ 'mosek64_%s_%s.dll'     %ver, 'mosekxx%s_%s.dll'%ver,     'cilkrts20.so',       ],
    'win32x86'   : [ 'mosek%s_%s.dll'        %ver, 'mosekxx%s_%s.dll'%ver,     'cilkrts20.so',       ]
}

sysid = None
if platform.system() == 'Darwin':           sysid = 'osx64x86'
elif platform.system() == 'Linux':
    if ctypes.sizeof(ctypes.c_void_p) == 8: sysid = 'linux64x86'
    else:                                   sysid = 'linux32x86'
elif platform.system() == 'Windows':
    if ctypes.sizeof(ctypes.c_void_p) == 8: sysid = 'win64x86'
    else:                                   sysid = 'win32x86'
if sysid is None:
    raise InstallationError("Unsupported system")

os.chdir(os.path.abspath(os.path.dirname(__file__)))

def _post_install(sitedir):
    mskdir = os.path.join(sitedir,'mosek')
    with open(os.path.join(mskdir,'mosekorigin.py'),'wt',encoding='ascii') as f:
        f.write('__mosekinstpath__ = {0}\n'.format(repr(mosekinstdir)))

class install(distutils.command.install.install):
    def run(self):
        distutils.command.install.install.run(self)
        self.execute(_post_install,
                     (self.install_lib,),
                     msg="Fixing library paths")

setup( name             = 'Mosek',
       cmdclass         = { 'install' : install },
       version          = '9.0.97',
       description      = 'Mosek/Python APIs',
       long_description = 'Interface for MOSEK',
       author           = 'Mosek ApS',
       author_email     = "support@mosek.com",
       license          = "See license.pdf in the MOSEK distribution",
       url              = 'http://www.mosek.com',
       packages         = [ 'mosek', 'mosek.fusion','mosek.fusion.impl' ],
       )



