"""
This setup.py file is a mish-mash of hacks that should make it work
under most circumstances. The core problems of the installation
process are

- Installation across different platforms
- Installation in user-defined locations
- Handling shared library dependencies

The extension modules depend on a set of dynamic libraries, namely
MOSEK and Cilk. These must be placed so they can be seen from the
extension module binaries.

==Windows==
On windows, a `A.dll` can locate `B.dll` if
- `B.dll` is in the `PATH`, or
- `B.dll` is in the current working directory

==OS X==
On OS X a program `P` that links to `A.dylib`, which in turn links to `B.dylib` can load `B.dylib` if
- `P` defines an RPATH and `A.dylib` is linked with `B.dylib` as `@rpath/B.dylib`, or
- `A.dylib` is linked with an absolute path to `B.dylib`.

==Linux==
On Linux a program `P` that links to `A.so`, which in turn links to `B.so` can load `B.so` if
- `P` defines an RPATH where `B.so` is located, including a relative path using `$ORIGIN`, or
- `B.so` is in the `LD_LIBRARY_PATH`.

"""

from distutils.core import setup
from distutils.core import Extension
import distutils.command.build_ext
import distutils.command.install
from distutils import log
import platform,sys,shutil
import os,os.path
import subprocess
import ctypes
import sysconfig

class InstallationError(Exception): pass

major,minor,_,_,_ = sys.version_info
if major != 2 or minor < 5:
    print "Python 2.5+ required, got %d.%d" % (major,minor)

instdir = os.path.abspath(os.path.join(__file__,'..'))
mosekinstdir = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..','bin'))
ver = ('9','0')

libd = {
    'osx64x86'   : [ 'libmosek64.%s.%s.dylib'%ver, 'libcilkrts.5.dylib', ],
    'linux64x86' : [ 'libmosek64.so.%s.%s'   %ver, 'libcilkrts.so.5',    ],
    'win64x86'   : [ 'mosek64_%s_%s.dll'     %ver, 'cilkrts20.dll',      ],
    'win32x86'   : [ 'mosek%s_%s.dll'        %ver, 'cilkrts20.dll',      ]
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


def osxfixlibs(libfile, libs, prefix=''):
    """
    Replace path in the dynamic library reference in the DYLIB
    `libfile` for all libraries listed in `libs` (disregarding their
    current paths in the `libfile`). 

    To see current library references, use `otool -L LIBNAME`. 

    Example: If `prefix` is `@rpath/` the DYLIB `libfile` contains a
    reference to `/Users/jdoe/libs/libtest.dylib`, and `libtest.dylib`
    is listed in libs, the reference will be changed to
    `@rpath/libtest.dylib`.
    """
    L = [ l.strip().split(' ')[0] for l in subprocess.check_output(['otool','-L',libfile]).split('\n')[1:] ]
    d = { os.path.basename(f) : f for f in L }

    args = []
    for l in libs:
        if l in d:
            args.extend([ '-change', d[l],prefix+l])
    if len(args) > 0:
        cmd = [ 'install_name_tool' ]
        cmd.extend(args)
        cmd.append(libfile)
        subprocess.call(cmd)

def patchelf(libfile,rpath):
    """
    Replace the `RPATH` entry in the SharedObject `libfile`.
    """
    subprocess.call(['patchelf','--set-rpath',rpath,libfile])

class install(distutils.command.install.install):
    """
    Extend the default install command, adding an additional operation
    that installs the dynamic MOSEK libraries.
    """
    def install_libs(self):
        mskdir = os.path.join(self.install_lib,'mosek')

        with open(os.path.join(mskdir,'mosekorigin.py'),'wt') as f:
            f.write('__mosekinstpath__ = {0}\n'.format(repr(mosekinstdir)))

        if platform.system() == 'Darwin':
            for dirpath,dirnames,filenames in os.walk(self.install_lib):
                for fname in filenames:
                    if fname.endswith('.so'):
                        osxfixlibs(os.path.join(dirpath,fname),['libmosek64.%s.%s.dylib'%ver,'libcilkrts.5.dylib'], mskdir + '/')
            for lib in libd[sysid]:
                log.info("copying %s -> %s/" % (os.path.join(mosekinstdir,lib),mskdir))
                shutil.copy(os.path.join(mosekinstdir,lib),mskdir)
                osxfixlibs(os.path.join(mskdir,lib),['libmosek64.%s.%s.dylib'%ver,'libcilkrts.5.dylib'], mskdir + '/')
        elif platform.system() == 'Linux':
            # patchelf may not be available on all platforms, so instead we make a preloader
            #for dirpath,dirnames,filenames in os.walk(self.install_lib):
            #    for fname in filenames:
            #        if fname.endswith('.so'):
            #            patchelf(os.path.join(dirpath,fname),rpath=os.path.join('$ORIGIN',os.path.relpath(mskdir,dirpath)))
            for lib in libd[sysid]:
                log.info("copying %s -> %s/" % (os.path.join(mosekinstdir,lib),mskdir))
                shutil.copy(os.path.join(mosekinstdir,lib),mskdir)
            with open(os.path.join(mskdir,'_mskpreload.py'),'wt') as f:
                f.write('import ctypes,os.path\n')
                f.write('DLLS = map(ctypes.CDLL,[ \n\t%s ])\n' % ',\n\t'.join([ "os.path.join(os.path.dirname(__file__),'%s')" % l for l in libd[sysid] ]))
        else:
            for lib in libd[sysid]:
                log.info("copying %s -> %s/" % (os.path.join(mosekinstdir,lib),mskdir))
                shutil.copy(os.path.join(mosekinstdir,lib),mskdir)

    def run(self):
        distutils.command.install.install.run(self)
        self.execute(self.install_libs,      (), msg="Fixing library paths")
        #self.execute(self.install_preloader, (), msg="Installing preloader module")

class build_ext(distutils.command.build_ext.build_ext):
    """
    Extend the default `build_ext` command replacing the extension
    building functionality with one that simply copies a pre-built
    extension module.
    """
    def build_extension(self,ext):
        tgtdir = os.path.join(self.build_lib,*ext.name.split('.')[:-1])

        try: os.makedirs(tgtdir)
        except OSError: pass
        for s in ext.sources:
            log.info("copying %s -> %s" % (s,tgtdir))
            shutil.copy(s,tgtdir)

if platform.system() == 'Windows':
    pyextsuffix = '.pyd'
else:
    pyextsuffix = '.so'
    #sysconfig.get_config_var('SO' if platform.system() == 'Windows' else "SHLIB_SUFFIX")

msksources       = [ 'mosek/_msk'+pyextsuffix ]
fragmentssources = ['mosek/fusion/impl/fragments'+pyextsuffix ]

def runsetup():
    setup( name             = 'Mosek', 
           version          = '9.0.97',
           #install_requires = ['numpy'],
           packages         = [ 'mosek', 'mosek.fusion','mosek.fusion.impl' ],
           ext_modules      = [ Extension('mosek._msk',
                                          sources = msksources),
                                Extension('mosek.fusion.impl.fragments',
                                          sources = fragmentssources) ],
           cmdclass         = { 'install' : install,
                                'build_ext' : build_ext },
           description      = 'Mosek/Python APIs',
           long_description = 'Interface for MOSEK',
           author           = 'Mosek ApS',
           author_email     = "support@mosek.com",
           license          = "See license.pdf in the MOSEK distribution",
           url              = 'http://www.mosek.com',
    )
    
if __name__ == '__main__':
    os.chdir(os.path.abspath(os.path.dirname(__file__)))
    runsetup()
