#!/bin/bash

qemu_dir="/mnt/c/Program Files/qemu"
build_dir="/mnt/c/edk2/Build"

loop_device="/dev/loop4"
mount_point="/mnt/drive"

qemu_bios_filename="BIOS.BIN"
loader_efi_filename="BOOTX64.EFI"
drive_img_filename="DRIVE.IMG"

bios_fd_file="$build_dir/OvmfX64/RELEASE_VS2019/FV/OVMF.fd"
loader_efi_file="$build_dir/MdeModule/RELEASE_VS2019/X64/Loader.efi"

mute()
{
  "$@" > /dev/null 2>&1
}

if [[ "$1" == "setup" ]]; then

	mute git clone https://github.com/tianocore/edk2
	mute git submodule update --init
	mute git checkout tags/edk2-stable202405

	echo "Done"
fi

if [[ "$1" == "mkdrive" ]]; then

	mute "$qemu_dir/qemu-img.exe" create -f raw "$drive_img_filename" 10G
	mute losetup -P "$loop_device" "$drive_img_filename"
	mute parted -s "$loop_device" mklabel gpt
	mute parted -s "$loop_device" mkpart primary fat32 1MiB 100%
	mute mkfs.msdos -F 32 "$loop_device"p1
	mute losetup -d "$loop_device"

	echo "Done"

fi

if [[ "$1" == "mount" ]]; then

	mute losetup -P "$loop_device" "$drive_img_filename"
	mute mount "$loop_device"p1 "$mount_point"

	echo "Done"

fi

if [[ "$1" == "umount" ]]; then

	if [[ -z "$loop_device" ]]; then
		echo "Missing --loop-dev"
		exit 1
	fi

	mute umount "$mount_point"
	mute losetup -d "$loop_device"

	echo "Done"

fi

if [[ "$1" == "update" ]]; then

	mute mkdir -p "$mount_point/EFI/BOOT"
	mute cp "$bios_fd_file" "$qemu_bios_filename"
	mute cp "$loader_efi_file" "$loader_efi_filename"
	mute cp "$loader_efi_file" "$mount_point/EFI/BOOT/$loader_efi_filename"

	echo "Done"

fi

if [[ "$1" == "boot" ]]; then

	mute "$qemu_dir/qemu-system-x86_64.exe" \
		-drive file="$drive_img_filename",format=raw,if=virtio \
		-m 2G \
		-bios "$qemu_bios_filename" \
		-netdev user,id=net0 -device virtio-net-pci,netdev=net0

	echo "Done"

fi

if [[ "$1" == "help" ]]; then
	echo
	echo "Usage:"
	echo
	echo " BootKit [command]"
	echo
	echo "Commands:"
	echo " setup          Setup EDK2 repository"
	echo " mkdrive        Make a hard drive image"
	echo " mount          Mount the hard drive image"
	echo " umount         Unmount the hard drive image"
	echo " update         Update BIOS and loader EFI file"
	echo " boot           Boot QEMU"
	echo " help           Display this help message"
	echo
fi