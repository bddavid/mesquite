# Change this in Makefile.customize so that it points to your
# installation of cppunit
#CPPUNIT_DIR = ../../../cppunit


# The rest of these items probably won't need to be messed with
#CPPUNIT_INCLUDE = -I${CPPUNIT_DIR}/include
MSQ_BASE_DIR = ../..
MESQUITE_INCLUDE = ${TSTT_INCLUDE} -I$(MSQ_BASE_DIR)/include -I$(MSQ_BASE_DIR)/includeLinks

MESQUITE_LINK = -L$(MSQ_BASE_DIR)/lib -lmesquite ${TSTT_LINK}

PREFIX = @

default: msq_test

include $(MSQ_BASE_DIR)/Makefile.customize
include Makefile.inc
MSQ_BASE_DIR = ../..

msq_test : $(TEST_OBJS) msq_test_main.o $(MSQ_BASE_DIR)/lib/libmesquite.a
	@echo "Linking $@"
	$(PREFIX)$(LINKER) $(LDFLAGS) \
        $(TEST_OBJS) msq_test_main.o \
        -o $@ \
        -L$(MSQ_BASE_DIR)/lib -lmesquite \
         $(CONFIG_LDFLAGS) $(CPPUNIT_LNK) \
        $(TSTTM_SERVER_LIBS) $(TSTTB_SERVER_LIBS)

qt_test: $(TEST_OBJS) qt_test_main.o $(MSQ_BASE_DIR)/lib/libmesquite.a
	$(PREFIX)$(LINKER) $(LDFLAGS) \
	$(TEST_OBJS) qt_test_main.o \
	-o $@ \
        -L$(MSQ_BASE_DIR)/lib -lmesquite \
         $(CONFIG_LDFLAGS) $(CPPUNIT_LNK) \
	-lqttestrunner -L$(QTDIR)/lib -lqt-mt \
        $(TSTTM_SERVER_LIBS) $(TSTTB_SERVER_LIBS)

clean:
	@rm -f *.o msq_test
	@rm -f *.o msq_test_mdb
	@rm -f *.o msq_test_aomd
	@rm -rf SunWS_cache
	@rm -rf cxx_repository

.SUFFIXES : .cpp .o
