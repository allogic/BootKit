#!/bin/bash

cmd=$1

mute()
{
  "$@" > /dev/null 2>&1
}

while [[ $# -gt 0 ]]; do
	case $1 in
		--bios-file)
			if [[ -n "$2" && "$2" != --* ]]; then
				bios_file="$2"
				shift
			fi
			;;
		--drive-file)
			if [[ -n "$2" && "$2" != --* ]]; then
				drive_file="$2"
				shift
			fi
			;;
		--loop-dev)
			if [[ -n "$2" && "$2" != --* ]]; then
				loop_dev="$2"
				shift
			fi
			;;
		--mnt-point)
			if [[ -n "$2" && "$2" != --* ]]; then
				mnt_point="$2"
				shift
			fi
			;;
		*)
		;;
	esac
  	shift
done

if [[ "$cmd" == "mkdrive" ]]; then

	if [[ -z "$drive_file" ]]; then
		echo "Missing drive file"
		exit 1
	fi

	if [[ -z "$loop_dev" ]]; then
		echo "Missing loopback device"
		exit 1
	fi

	mute "/mnt/c/Program Files/qemu/qemu-img.exe" create -f raw "$drive_file" 10G
	mute losetup -P "$loop_dev" "$drive_file"
	mute parted -s "$loop_dev" mklabel gpt
	mute parted -s "$loop_dev" mkpart primary fat32 1MiB 100%
	mute mkfs.msdos -F 32 "$loop_dev"p1
	mute losetup -d "$loop_dev"

	echo "Done"

fi

if [[ "$cmd" == "mount" ]]; then

	if [[ -z "$drive_file" ]]; then
		echo "Missing drive file"
		exit 1
	fi

	if [[ -z "$loop_dev" ]]; then
		echo "Missing loopback device"
		exit 1
	fi

	if [[ -z "$mnt_point" ]]; then
		echo "Missing mount point"
		exit 1
	fi

	mute losetup -P "$loop_dev" "$drive_file"
	mute mount "$loop_dev"p1 "$mnt_point"

	echo "Done"

fi

if [[ "$cmd" == "umount" ]]; then

	if [[ -z "$loop_dev" ]]; then
		echo "Missing loopback device"
		exit 1
	fi

	if [[ -z "$mnt_point" ]]; then
		echo "Missing mount point"
		exit 1
	fi

	mute umount "$mnt_point"
	mute losetup -d "$loop_dev"

	echo "Done"

fi

if [[ "$cmd" == "boot" ]]; then

	if [[ -z "$bios_file" ]]; then
		echo "Missing bios file"
		exit 1
	fi

	if [[ -z "$drive_file" ]]; then
		echo "Missing drive file"
		exit 1
	fi

	mute "/mnt/c/Program Files/qemu/qemu-system-x86_64.exe" \
		-drive file="$drive_file",format=raw,if=virtio \
		-m 2G \
		-bios "$bios_file" \
		-netdev user,id=net0 -device virtio-net-pci,netdev=net0 \
		-net none

	echo "Done"

fi

if [[ "$cmd" == "help" ]] || [[ "$cmd" == "h" ]]; then
	echo
	echo "Usage:"
	echo
	echo " BootKit [command] [options]"
	echo
	echo "Commands:"
	echo " mkdrive             Make a hard drive image"
	echo " mount               Mount the hard drive image"
	echo " umount              Unmount the hard drive image"
	echo " boot                Boot QEMU for development"
	echo " help                Display this help message"
	echo
	echo "Options:"
	echo " --bios-file         Specify the "OVMF.fd" BIOS file for QEMU"
	echo " --drive-file        Raw hard drive image for QEMU to boot from"
	echo " --loop-dev          Loopback device for the hard drive image"
	echo " --mnt-point         Mount point for the hard drive image"
	echo
	echo "Examples:"
	echo
	echo " BootKit mkdrive --drive-file drive.img --loop-dev /dev/loopX"
	echo " BootKit mount --drive-file drive.img --loop-dev /dev/loopX --mnt-point /mnt/drive"
	echo " BootKit umount --loop-dev /dev/loopX --mnt-point /mnt/drive"
	echo " BootKit boot --bios-file OVMF.fd --drive-file drive.img"
	echo
fi