all: xplaneterm

clean: clean_xplaneterm

##############################
#######  TARGET: xplaneterm
##############################
TOP_xplaneterm=../..
WD_xplaneterm=$(shell cd ${TOP_xplaneterm};echo `pwd`)
c_SRC_xplaneterm+=${WD_xplaneterm}/SourceCode/xplane-term.cpp

OBJS_xplaneterm+=$(c_SRC_xplaneterm:.cpp=.cpp.o)

CFLAGS_xplaneterm+= -I./\
 -I../../SDK/CHeaders/XPLM\
 -I../../SDK/CHeaders/Widgets\
 -I../../SDK/CHeaders/Wrappers\
 -iquote -I/usr/include/\
  -DIBM=0 -DAPL=0 -DLIN=1 -DXPLM200=1

DBG=-g

CFLAGS_xplaneterm+=-O0 -x c++ -ansi

clean_xplaneterm:
	rm -f ${OBJS_xplaneterm}

xplaneterm:
	$(MAKE) -f xplane-term.mk xplaneterm.xpl TARGET=xplaneterm.xpl CC="g++"  LD="g++"  AR="ar -crs"  SIZE="size" LIBS+="-lGL -lGLU"

xplaneterm.xpl: ${OBJS_xplaneterm}
	$(CC) -shared ${LDFLAGS} -o Release/Resources/plugins/xplane-term.xpl ${OBJS_xplaneterm} ${LIBS}

%.cpp.o: %.cpp
	$(CC) -c -fPIC ${CFLAGS_xplaneterm} $< -o $@ -MMD
include $(c_SRC_xplaneterm:.cpp=.d)

%.d: %.cpp
	set -e; $(CC) -M $(CFLAGS_xplaneterm) $< \
 | sed 's!\($(*F)\)\.o[ :]*!$(*D)/\1.o $@ : !g' > $@; \
 [ -s $@ ] || rm -f $@

# end Makefile
