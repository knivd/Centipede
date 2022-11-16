#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/Centipede.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/Centipede.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

ifdef SUB_IMAGE_ADDRESS

else
SUB_IMAGE_ADDRESS_COMMAND=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../platform.c ../../../asmdis.c ../../../main.c ../../../monitor.c ../../../vm.c ../../../xmodem/xmodem.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/1472/platform.o ${OBJECTDIR}/_ext/2124829536/asmdis.o ${OBJECTDIR}/_ext/2124829536/main.o ${OBJECTDIR}/_ext/2124829536/monitor.o ${OBJECTDIR}/_ext/2124829536/vm.o ${OBJECTDIR}/_ext/1507166017/xmodem.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/1472/platform.o.d ${OBJECTDIR}/_ext/2124829536/asmdis.o.d ${OBJECTDIR}/_ext/2124829536/main.o.d ${OBJECTDIR}/_ext/2124829536/monitor.o.d ${OBJECTDIR}/_ext/2124829536/vm.o.d ${OBJECTDIR}/_ext/1507166017/xmodem.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1472/platform.o ${OBJECTDIR}/_ext/2124829536/asmdis.o ${OBJECTDIR}/_ext/2124829536/main.o ${OBJECTDIR}/_ext/2124829536/monitor.o ${OBJECTDIR}/_ext/2124829536/vm.o ${OBJECTDIR}/_ext/1507166017/xmodem.o

# Source Files
SOURCEFILES=../platform.c ../../../asmdis.c ../../../main.c ../../../monitor.c ../../../vm.c ../../../xmodem/xmodem.c



CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/Centipede.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=32MX170F256B
MP_LINKER_FILE_OPTION=
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/1472/platform.o: ../platform.c  .generated_files/flags/default/4820b8990218ef54e0ed96367740a23661f7cafb .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/platform.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/platform.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ftoplevel-reorder -Os -funroll-loops -fomit-frame-pointer -fschedule-insns -fschedule-insns2 -fcommon -DPIC32MX1 -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1472/platform.o.d" -o ${OBJECTDIR}/_ext/1472/platform.o ../platform.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/2124829536/asmdis.o: ../../../asmdis.c  .generated_files/flags/default/84fd351d5fdc250a48982efb72efd979a6c04e64 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/2124829536" 
	@${RM} ${OBJECTDIR}/_ext/2124829536/asmdis.o.d 
	@${RM} ${OBJECTDIR}/_ext/2124829536/asmdis.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ftoplevel-reorder -Os -funroll-loops -fomit-frame-pointer -fschedule-insns -fschedule-insns2 -fcommon -DPIC32MX1 -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/2124829536/asmdis.o.d" -o ${OBJECTDIR}/_ext/2124829536/asmdis.o ../../../asmdis.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/2124829536/main.o: ../../../main.c  .generated_files/flags/default/ff831a49f486ba03a0c15c53089ce5e0428b6ac7 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/2124829536" 
	@${RM} ${OBJECTDIR}/_ext/2124829536/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/2124829536/main.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ftoplevel-reorder -Os -funroll-loops -fomit-frame-pointer -fschedule-insns -fschedule-insns2 -fcommon -DPIC32MX1 -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/2124829536/main.o.d" -o ${OBJECTDIR}/_ext/2124829536/main.o ../../../main.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/2124829536/monitor.o: ../../../monitor.c  .generated_files/flags/default/c0554c629e0811174da9a0390f29e7fbc0d48d8f .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/2124829536" 
	@${RM} ${OBJECTDIR}/_ext/2124829536/monitor.o.d 
	@${RM} ${OBJECTDIR}/_ext/2124829536/monitor.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ftoplevel-reorder -Os -funroll-loops -fomit-frame-pointer -fschedule-insns -fschedule-insns2 -fcommon -DPIC32MX1 -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/2124829536/monitor.o.d" -o ${OBJECTDIR}/_ext/2124829536/monitor.o ../../../monitor.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/2124829536/vm.o: ../../../vm.c  .generated_files/flags/default/5a5158f8bd917f5ae5cf91b08b52b1629c8a1f32 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/2124829536" 
	@${RM} ${OBJECTDIR}/_ext/2124829536/vm.o.d 
	@${RM} ${OBJECTDIR}/_ext/2124829536/vm.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ftoplevel-reorder -O3 -funroll-loops -fomit-frame-pointer -fschedule-insns -fschedule-insns2 -fcommon -DPIC32MX1 -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/2124829536/vm.o.d" -o ${OBJECTDIR}/_ext/2124829536/vm.o ../../../vm.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/1507166017/xmodem.o: ../../../xmodem/xmodem.c  .generated_files/flags/default/6b1032307f96321b8f5910f064d74ee3af0cdc05 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1507166017" 
	@${RM} ${OBJECTDIR}/_ext/1507166017/xmodem.o.d 
	@${RM} ${OBJECTDIR}/_ext/1507166017/xmodem.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ftoplevel-reorder -Os -funroll-loops -fomit-frame-pointer -fschedule-insns -fschedule-insns2 -fcommon -DPIC32MX1 -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1507166017/xmodem.o.d" -o ${OBJECTDIR}/_ext/1507166017/xmodem.o ../../../xmodem/xmodem.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)    
	
else
${OBJECTDIR}/_ext/1472/platform.o: ../platform.c  .generated_files/flags/default/e7bdaf2486a3efb3f000da59a572c19271cc2183 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/platform.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/platform.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ftoplevel-reorder -Os -funroll-loops -fomit-frame-pointer -fschedule-insns -fschedule-insns2 -fcommon -DPIC32MX1 -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1472/platform.o.d" -o ${OBJECTDIR}/_ext/1472/platform.o ../platform.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/2124829536/asmdis.o: ../../../asmdis.c  .generated_files/flags/default/43a24089801d893a51feee63a739bdc6dcbf46e4 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/2124829536" 
	@${RM} ${OBJECTDIR}/_ext/2124829536/asmdis.o.d 
	@${RM} ${OBJECTDIR}/_ext/2124829536/asmdis.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ftoplevel-reorder -Os -funroll-loops -fomit-frame-pointer -fschedule-insns -fschedule-insns2 -fcommon -DPIC32MX1 -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/2124829536/asmdis.o.d" -o ${OBJECTDIR}/_ext/2124829536/asmdis.o ../../../asmdis.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/2124829536/main.o: ../../../main.c  .generated_files/flags/default/c47bcb0e39aac69d2205c768bb93536689256cc8 .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/2124829536" 
	@${RM} ${OBJECTDIR}/_ext/2124829536/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/2124829536/main.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ftoplevel-reorder -Os -funroll-loops -fomit-frame-pointer -fschedule-insns -fschedule-insns2 -fcommon -DPIC32MX1 -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/2124829536/main.o.d" -o ${OBJECTDIR}/_ext/2124829536/main.o ../../../main.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/2124829536/monitor.o: ../../../monitor.c  .generated_files/flags/default/67ca3a588abef88e63159c3483edf124f1b2677b .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/2124829536" 
	@${RM} ${OBJECTDIR}/_ext/2124829536/monitor.o.d 
	@${RM} ${OBJECTDIR}/_ext/2124829536/monitor.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ftoplevel-reorder -Os -funroll-loops -fomit-frame-pointer -fschedule-insns -fschedule-insns2 -fcommon -DPIC32MX1 -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/2124829536/monitor.o.d" -o ${OBJECTDIR}/_ext/2124829536/monitor.o ../../../monitor.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/2124829536/vm.o: ../../../vm.c  .generated_files/flags/default/2d088b31822d3a3a5cb617c1ad427da76c48b44f .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/2124829536" 
	@${RM} ${OBJECTDIR}/_ext/2124829536/vm.o.d 
	@${RM} ${OBJECTDIR}/_ext/2124829536/vm.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ftoplevel-reorder -O3 -funroll-loops -fomit-frame-pointer -fschedule-insns -fschedule-insns2 -fcommon -DPIC32MX1 -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/2124829536/vm.o.d" -o ${OBJECTDIR}/_ext/2124829536/vm.o ../../../vm.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/1507166017/xmodem.o: ../../../xmodem/xmodem.c  .generated_files/flags/default/82e1c9cd09fe98ee88469892549ebfb3ddd450be .generated_files/flags/default/87483c345429186c5999dcd95309ab66555f8ca8
	@${MKDIR} "${OBJECTDIR}/_ext/1507166017" 
	@${RM} ${OBJECTDIR}/_ext/1507166017/xmodem.o.d 
	@${RM} ${OBJECTDIR}/_ext/1507166017/xmodem.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ftoplevel-reorder -Os -funroll-loops -fomit-frame-pointer -fschedule-insns -fschedule-insns2 -fcommon -DPIC32MX1 -Wall -MP -MMD -MF "${OBJECTDIR}/_ext/1507166017/xmodem.o.d" -o ${OBJECTDIR}/_ext/1507166017/xmodem.o ../../../xmodem/xmodem.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)    
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compileCPP
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${DISTDIR}/Centipede.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -g   -mprocessor=$(MP_PROCESSOR_OPTION) -O3 --fill=0xFF -o ${DISTDIR}/Centipede.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)   -mreserve=data@0x0:0x1FC -mreserve=boot@0x1FC00490:0x1FC00BEF  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D=__DEBUG_D,--defsym=_min_heap_size=128,--defsym=_min_stack_size=128,--no-code-in-dinit,--no-dinit-in-serial-mem,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--cref,--memorysummary,${DISTDIR}/memoryfile.xml 
	
else
${DISTDIR}/Centipede.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mprocessor=$(MP_PROCESSOR_OPTION) -O3 --fill=0xFF -o ${DISTDIR}/Centipede.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=_min_heap_size=128,--defsym=_min_stack_size=128,--no-code-in-dinit,--no-dinit-in-serial-mem,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--cref,--memorysummary,${DISTDIR}/memoryfile.xml 
	${MP_CC_DIR}\\xc32-bin2hex ${DISTDIR}/Centipede.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} 
	@echo Normalizing hex file
	@"C:/Program Files/Microchip/MPLABX/v6.00/mplab_platform/platform/../mplab_ide/modules/../../bin/hexmate" --edf="C:/Program Files/Microchip/MPLABX/v6.00/mplab_platform/platform/../mplab_ide/modules/../../dat/en_msgs.txt" ${DISTDIR}/Centipede.X.${IMAGE_TYPE}.hex -o${DISTDIR}/Centipede.X.${IMAGE_TYPE}.hex

endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${OBJECTDIR}
	${RM} -r ${DISTDIR}

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
