all:
	$(MAKE) -C patcher
	$(MAKE) -C loader

clean:
	$(MAKE) -C patcher clean
	$(MAKE) -C loader clean

