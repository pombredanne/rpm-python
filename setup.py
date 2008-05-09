#!/usr/bin/env python

from distutils.core import setup, Extension
import commands
from glob import glob
import os

srcs = glob('src/*.c')

def pkgconfig(*packages, **kw):
    flag_map = {'-I': 'include_dirs', '-L': 'library_dirs', '-l': 'libraries'}
    for token in commands.getoutput("pkg-config --libs --cflags %s" % ' '.join(packages)).split():
        kw.setdefault(flag_map.get(token[:2]), []).append(token[2:])
    return kw

def getversion():
    ver = "0.1"
    if os.access('.git', os.F_OK):
        gitout = os.popen('git-log --pretty=oneline|wc -l').readlines()
        gitver = gitout[0].strip()
        ver = "%s.git%s" % (ver, gitver)
    return ver

rpmmod = Extension('rpmng._rpmng',
                   sources = srcs,
                   **pkgconfig('rpm')
                  )

setup(name='rpmng',
      version=getversion(),
      description='RPM Python NG',
      author='Panu Matilainen',
      author_email='pmatilai@redhat.com',
      url='http://www.rpm.org/',
      packages = ['rpmng'],
      ext_modules= [rpmmod]
     )
