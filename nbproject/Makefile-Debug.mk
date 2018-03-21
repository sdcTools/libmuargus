#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=MinGW-Windows
CND_DLIB_EXT=dll
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/ChSafeVarInfo.o \
	${OBJECTDIR}/Household.o \
	${OBJECTDIR}/MuArgCtrl.o \
	${OBJECTDIR}/MuArgCtrl_wrap.o \
	${OBJECTDIR}/Table.o \
	${OBJECTDIR}/Variable.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblibmuargus.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblibmuargus.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblibmuargus.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -Wl,--kill-at -shared

${OBJECTDIR}/ChSafeVarInfo.o: ChSafeVarInfo.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/C/Program\ Files\ \(x86\)/Java/jdk1.8.0_111/include -I/C/Program\ Files\ \(x86\)/Java/jdk1.8.0_111/include/win32  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ChSafeVarInfo.o ChSafeVarInfo.cpp

${OBJECTDIR}/Household.o: Household.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/C/Program\ Files\ \(x86\)/Java/jdk1.8.0_111/include -I/C/Program\ Files\ \(x86\)/Java/jdk1.8.0_111/include/win32  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Household.o Household.cpp

${OBJECTDIR}/MuArgCtrl.o: MuArgCtrl.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/C/Program\ Files\ \(x86\)/Java/jdk1.8.0_111/include -I/C/Program\ Files\ \(x86\)/Java/jdk1.8.0_111/include/win32  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MuArgCtrl.o MuArgCtrl.cpp

.NO_PARALLEL:MuArgCtrl_wrap.cpp MuArgCtrl_wrap.h
MuArgCtrl_wrap.cpp MuArgCtrl_wrap.h: MuArgCtrl.swg
	@echo Performing Custom Build Step
	C:\swigwin-3.0.10\swig.exe -c++ -java -package muargus.extern.dataengine -outdir ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM} -o MuArgCtrl_wrap.cpp MuArgCtrl.swg

${OBJECTDIR}/MuArgCtrl_wrap.o: MuArgCtrl_wrap.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/C/Program\ Files\ \(x86\)/Java/jdk1.8.0_111/include -I/C/Program\ Files\ \(x86\)/Java/jdk1.8.0_111/include/win32  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MuArgCtrl_wrap.o MuArgCtrl_wrap.cpp

${OBJECTDIR}/Table.o: Table.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/C/Program\ Files\ \(x86\)/Java/jdk1.8.0_111/include -I/C/Program\ Files\ \(x86\)/Java/jdk1.8.0_111/include/win32  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Table.o Table.cpp

${OBJECTDIR}/Variable.o: Variable.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/C/Program\ Files\ \(x86\)/Java/jdk1.8.0_111/include -I/C/Program\ Files\ \(x86\)/Java/jdk1.8.0_111/include/win32  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Variable.o Variable.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} MuArgCtrl_wrap.cpp MuArgCtrl_wrap.h

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
