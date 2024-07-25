# BootKit

	EDK2 Setup:
	------------------------------------------------------------

	git clone https://github.com/tianocore/edk2
	git submodule update --init
	git checkout tags/edk2-stable202405
	Launch Developer Command Prompt for VS20XX
	edksetup.bat
	Build BaseTools
		cd BaseTools
		nmake
	Build OVMF BIOS
		Configure Conf\target.txt
			ACTIVE_PLATFORM = OvmfPkg/OvmfPkgX64.dsc
			TARGET_ARCH = X64
			TOOL_CHAIN_TAG = VS20XX
		edksetup.bat
		build
	Build Custom My Custom Package
		Configure Conf\target.txt
			ACTIVE_PLATFORM = MyModulePkg/MyModulePkg.dsc
			TARGET_ARCH = X64
			TOOL_CHAIN_TAG = VS20XX
		edksetup.bat
		build

	Create Filesystem:
	------------------------------------------------------------

	qemu-img create -f raw drive.img 10G
	losetup -f -P drive.img
	parted /dev/loopX mklabel gpt
	parted /dev/loopX mkpart primary fat32 1MiB 100%
	mkfs.msdos -F 32 -f /dev/loopXp1
	mount /dev/loopXp1 /mnt
	mkdir -p /mnt/EFI/BOOT
	cp MyModule.efi /mnt/EFI/BOOT/BOOTX64.EFI

	Launch QEMU:
	------------------------------------------------------------

	qemu-system-x86_64 `
		-drive file=drive.img,format=raw,if=virtio `
		-m 2G `
		-bios OVMF.fd `
		-netdev user,id=net0 -device virtio-net-pci,netdev=net0 `
		-net none