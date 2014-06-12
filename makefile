#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Directories.
export BINDIR  = $(CURDIR)/bin

# Builds all kernels for Intel x86.
all-x86:
	cd x86 && $(MAKE) all BINDIR=$(BINDIR)

# Builds all kernels for MPPA-256.
all-mppa256: 
	cd mppa256 && $(MAKE) all BINDIR=$(BINDIR)


# Cleans compilation files.
clean:
	cd x86 && $(MAKE) clean
	cd mppa256 && $(MAKE) clean
