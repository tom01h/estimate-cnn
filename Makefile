SHELL = /bin/bash

SIM_DIR = sim

VERILATOR = verilator

VERILATOR_OPTS = \
	-Wall \
	-Wno-WIDTH \
	-Wno-UNUSED \
	-Wno-BLKSEQ \
	--cc \
	+1364-2001ext+v \
	-Wno-fatal \
	--Mdir sim \
	--trace \

VERILATOR_MAKE_OPTS = OPT_FAST="-O3"

DESIGN_SRCS = \
estimate.sv \

VERILATOR_CPP_TB = estimate_tb.cpp

default: $(SIM_DIR)/Vestimate

sim: $(SIM_DIR)/Vestimate

$(SIM_DIR)/Vestimate: $(DESIGN_SRCS) $(VERILATOR_CPP_TB)
	$(VERILATOR) $(VERILATOR_OPTS) $(DESIGN_SRCS) --exe ../$(VERILATOR_CPP_TB)
	cd sim; make $(VERILATOR_MAKE_OPTS) -f Vestimate.mk Vestimate__ALL.a
	cd sim; make $(VERILATOR_MAKE_OPTS) -f Vestimate.mk Vestimate

clean:
	rm -rf sim/ tmp.vcd

.PHONY:
