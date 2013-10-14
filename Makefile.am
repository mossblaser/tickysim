# Build recursively in these subdirs
SUBDIRS = src . tests

# Extra files to include in the distribution
EXTRA_DIST = README.md configs

# Delete any random swapfiles/dotfiles etc. which get pulled in with the configs
# directory.
dist-hook:
	rm -f $(distdir)/configs/.*.swp
