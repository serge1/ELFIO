#!/bin/sh
# find out where the build script is located
tdir=`echo "$0" | sed 's%[\\/][^\\/][^\\/]*$%%'`
test "x$tdir" = "x$0" && tdir=.
scriptdir=`cd $tdir; pwd`
# find src directory.
# If scriptdir ends in SPECS, then topdir is $scriptdir/..
# If scriptdir ends in CYGWIN-PATCHES, then topdir is $scriptdir/../..
# Otherwise, we assume that topdir = scriptdir
topdir1=`echo ${scriptdir} | sed 's%/SPECS$%%'`
topdir2=`echo ${scriptdir} | sed 's%/CYGWIN-PATCHES$%%'`
if [ "x$topdir1" != "x$scriptdir" ] ; then # SPECS
  topdir=`cd ${scriptdir}/..; pwd`
else
  if [ "x$topdir2" != "x$scriptdir" ] ; then # CYGWIN-PATCHES
    topdir=`cd ${scriptdir}/../..; pwd`
  else
    topdir=`cd ${scriptdir}; pwd`
  fi
fi

tscriptname=`basename $0 .sh`
export PKG=`echo $tscriptname | sed -e 's/\-[^\-]*\-[^\-]*$//'`
export VER=`echo $tscriptname | sed -e 's/^[^\-]*\-//' -e 's/\-[^\-]*$//'`
export REL=`echo $tscriptname | sed -e 's/^[^\-]*\-[^\-]*\-//'`
export FULLPKG=${PKG}-${VER}-${REL}
# if the orig src package is bzip2'ed, remember to
# change 'z' to 'j' in the 'tar xvzf' commands in the
# prep) and mkpatch) sections
export src_orig_pkg_name=${PKG}-${VER}.tar.gz
export src_pkg_name=${FULLPKG}-src.tar.bz2
export src_patch_name=${FULLPKG}.patch
export bin_pkg_name=${FULLPKG}.tar.bz2

export src_orig_pkg=${topdir}/${src_orig_pkg_name}
export src_pkg=${topdir}/${src_pkg_name}
export src_patch=${topdir}/${src_patch_name}
export bin_pkg=${topdir}/${bin_pkg_name}
export srcdir=${topdir}/${PKG}-${VER}
export objdir=${srcdir}/.build
export instdir=${srcdir}/.inst
export srcinstdir=${srcdir}/.sinst
export checkfile=${topdir}/${FULLPKG}.check
# run on
host=i686-pc-cygwin
# if this package creates binaries, they run on
target=i686-pc-cygwin
prefix=/usr
sysconfdir=/etc
MY_CFLAGS="-O2"
MY_CXXFLAGS="-O2"
MY_LDFLAGS=

mkdirs() {
  (cd ${topdir} && \
  mkdir -p ${objdir} && \
  mkdir -p ${instdir} && \
  mkdir -p ${srcinstdir} )
}
prep() {
  (cd ${topdir} && \
  tar xvzf ${src_orig_pkg} ; \
  cd ${topdir} && \
  patch -p0 < ${src_patch}
  && mkdirs )
}
conf() {
  (cd ${objdir} && \
  CFLAGS="${MY_CFLAGS}" LDFLAGS="${MY_LDFLAGS}" \
  CXXFLAGS="${MY_CXXFLAGS}" LDFLAGS="${MY_LDFLAGS}" \
  ${srcdir}/configure --host=${host} --target=${target} \
  --srcdir=${srcdir} --prefix=${prefix} \
  --exec-prefix=${prefix} --sysconfdir=${sysconfdir} \
  --libdir=/lib --includedir=${prefix}/include \
  --libexecdir='${sbindir}' --localstatedir=/var \
  --datadir='${prefix}/share'
)
}
build() {
  (cd ${objdir} && \
  CFLAGS="${MY_CFLAGS}" make )
}
check() {
  (cd ${objdir} && \
  make test | tee ${checkfile} 2>&1 )
}
clean() {
  (cd ${objdir} && \
  make clean )
}
install() {
  (cd ${objdir} && \
  make install DESTDIR=${instdir}
  if [ -f ${instdir}${prefix}/info/dir ] ; then \
    rm ${instdir}${prefix}/info/dir ; \
  fi && \
  if [ ! -d ${instdir}${prefix}/doc/${PKG}-${VER} ]; then \
    mkdir -p ${instdir}${prefix}/doc/${PKG}-${VER} ; \
  fi && \
  if [ ! -d ${instdir}${prefix}/doc/Cygwin ]; then \
    mkdir -p ${instdir}${prefix}/doc/Cygwin ; \
  fi && \
  templist=""; \
  for f in ${srcdir}/ANNOUNCE ${srcdir}/CHANGES ${srcdir}/INSTALL \
           ${srcdir}/KNOWNBUG ${srcdir}/LICENSE ${srcdir}/README \
           ${srcdir}/AUTHORS  ${srcdir}/KNOWNBUG ${srcdir}/COPYING \
           ${srcdir}/doc/tutorial.pdf \
           ${srcdir}/Examples/tutorial/tutorial.cpp \
	   ${srcdir}/TODO ; do \
    if [ -f $f ] ; then \
      templist="$templist $f"; \
    fi ; \
  done && \
  if [ ! "x$templist" = "x" ]; then \
    /usr/bin/install -m 644 $templist \
         ${instdir}${prefix}/doc/${PKG}-${VER} ;
  fi && \
  if [ -f ${srcdir}/CYGWIN-PATCHES/${PKG}-${VER}.README ]; then \
    /usr/bin/install -m 644 ${srcdir}/CYGWIN-PATCHES/${PKG}-${VER}.README \
      ${instdir}${prefix}/doc/Cygwin/${PKG}-${VER}.README ; \
  else \
    if [ -f ${srcdir}/CYGWIN-PATCHES/README ]; then \
      /usr/bin/install -m 644 ${srcdir}/CYGWIN-PATCHES/README \
        ${instdir}${prefix}/doc/Cygwin/${PKG}-${VER}.README ; \
    fi ;\
  fi ; )
}
strip() {
  (cd ${instdir} && \
  find . -name "*.dll" | xargs strip > /dev/null 2>&1
  find . -name "*.exe" | xargs strip > /dev/null 2>&1 )
}
pkg() {
  (cd ${instdir} && \
  tar cvjf ${bin_pkg} * )
}
mkpatch() {
  (cd ${srcdir} && \
  tar xvzf ${src_orig_pkg} ;\
  mv ${PKG}-${VER} ../${PKG}-${VER}-orig && \
  cd ${topdir} && \
  diff -urN -x '.build' -x '.inst' -x '.sinst' \
    ${PKG}-${VER}-orig ${PKG}-${VER} > \
    ${srcinstdir}/${src_patch_name} ; \
  rm -rf ${PKG}-${VER}-orig )
}
spkg() {
  (mkpatch && \
  cp ${src_orig_pkg} ${srcinstdir}/${src_orig_pkg_name} && \
  cp $0 ${srcinstdir}/`basename $0` && \
  cd ${srcinstdir} && \
  tar cvjf ${src_pkg} * )
}
finish() {
  rm -rf ${srcdir}
}
case $1 in
  prep)	prep ; STATUS=$? ;;
  mkdirs)	mkdirs; STATUS=$? ;;
  conf)	conf ; STATUS=$? ;;
  build)	build ; STATUS=$? ;;
  check)	check ; STATUS=$? ;;
  clean)	clean ; STATUS=$? ;;
  install)	install ; STATUS=$? ;;
  strip)	strip ; STATUS=$? ;;
  package)	pkg ; STATUS=$? ;;
  pkg)	pkg ; STATUS=$? ;;
  mkpatch)	mkpatch ; STATUS=$? ;;
  src-package)	spkg ; STATUS=$? ;;
  spkg)	spkg ; STATUS=$? ;;
  finish) finish ; STATUS=$? ;;
  all) prep && conf && build && install && \
     strip && pkg && spkg && finish ; \
	  STATUS=$? ;;
  *) echo "Error: bad arguments" ; exit 1 ;;
esac
exit ${STATUS}


