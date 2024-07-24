/*

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

*/

#include <Uefi.h>

#include <Pi/PiDxeCis.h>

#include <Protocol/EfiGuard.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/LegacyBios.h>

#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/ReportStatusCodeLib.h>
#include <Library/DevicePathLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiBootManagerLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

STATIC EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *mTextInputEx = NULL;

STATIC
VOID
EFIAPI
ResetTextInput(
	VOID
)
{
	if (mTextInputEx != NULL)
	{
		mTextInputEx->Reset(mTextInputEx, FALSE);
	}
	else
	{
		gST->ConIn->Reset(gST->ConIn, FALSE);
	}
}

STATIC
UINT16
EFIAPI
WaitForKey(
	VOID
)
{
	EFI_KEY_DATA KeyData = { 0 };
	UINTN Index = 0;

	if (mTextInputEx != NULL)
	{
		gBS->WaitForEvent(1, &mTextInputEx->WaitForKeyEx, &Index);
		mTextInputEx->ReadKeyStrokeEx(mTextInputEx, &KeyData);
	}
	else
	{
		gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &Index);
		gST->ConIn->ReadKeyStroke(gST->ConIn, &KeyData.Key);
	}

	return KeyData.Key.ScanCode;
}

STATIC
UINT16
EFIAPI
WaitForKeyWithTimeout(
	IN UINTN Milliseconds
)
{
	ResetTextInput();

	gBS->Stall(Milliseconds * 1000);

	EFI_KEY_DATA KeyData = { 0 };

	if (mTextInputEx != NULL)
	{
		mTextInputEx->ReadKeyStrokeEx(mTextInputEx, &KeyData);
	}
	else
	{
		gST->ConIn->ReadKeyStroke(gST->ConIn, &KeyData.Key);
	}

	ResetTextInput();

	return KeyData.Key.ScanCode;
}

EFI_STATUS
EFIAPI
UefiMain(
	IN EFI_HANDLE ImageHandle,
	IN EFI_SYSTEM_TABLE *SystemTable
)
{
	return EFI_SUCCESS;
}