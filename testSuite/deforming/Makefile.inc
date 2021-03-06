all_tests: $(testdir)/deforming/main


$(testdir)/deforming/main: $(testdir)/deforming/main.cpp $(locallibdir)/libmesquite.a
	@echo "Linking Mesquite driver application ($@)..."
	${LINKER} $(CXXFLAGS) ${LDFLAGS} $(CONFIG_CFLAGS) \
	-o $@ $< -L$(locallibdir) -lmesquite \
        $(CONFIG_LDFLAGS) -lm

$(testdir)/deforming/main2: $(testdir)/deforming/main2.cpp $(locallibdir)/libmesquite.a
	@echo "Linking Mesquite driver application ($@)..."
	${LINKER} $(CXXFLAGS) ${LDFLAGS} $(CONFIG_CFLAGS) \
	-o $@ $< -L$(locallibdir) -lmesquite \
        $(CONFIG_LDFLAGS) -lm
