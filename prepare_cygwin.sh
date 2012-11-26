ELFIOVER='ELFIO-1.0.3'
BUILDVER='1'
FULLVER=${ELFIOVER}-${BUILDVER}

mkdir $1
cp ${ELFIOVER}.tar.gz $1
cd $1
tar -xzf ${ELFIOVER}.tar.gz
cp ../cygwin/${FULLVER}.sh .
mkdir ${ELFIOVER}/CYGWIN-PATCHES
cp ../cygwin/CYGWIN-PATCHES/* ${ELFIOVER}/CYGWIN-PATCHES
./${FULLVER}.sh mkdirs
./${FULLVER}.sh spkg

cd ..
mkdir $2
cp $1/${FULLVER}-src.tar.bz2 $2
cd $2
tar -xjf ${FULLVER}-src.tar.bz2
./${FULLVER}.sh all
