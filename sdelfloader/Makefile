
all:
	@make -C loader
	@bin2s loader/loader.dol > miniloader/source/loader.S
	@make -C miniloader
	@cp miniloader/miniloader.dol trucha-disc-loader.dol
	@cp loader/loader.elf tp-hack-loader.elf
	@cp miniloader/miniloader.elf homebrew-channel-loader.elf

clean:
	make -C loader clean
	make -C miniloader clean
	rm -f miniloader/source/loader.S
