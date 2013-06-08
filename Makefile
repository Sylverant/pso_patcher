all:
	$(MAKE) -C patcher
	$(MAKE) -C loader

prpatcher:
	$(MAKE) -C patcher prpatcher
	$(MAKE) -C loader

clean:
	$(MAKE) -C patcher clean
	$(MAKE) -C loader clean

