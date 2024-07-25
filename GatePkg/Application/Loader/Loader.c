#include <Uefi.h>

#include <Pi/PiDxeCis.h>

#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>

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

STATIC
EFI_STATUS
EFIAPI
SetHighestAvailableTextMode(
	VOID
)
{
	if (gST->ConOut == NULL)
	{
		return EFI_NOT_READY;
	}

	INT32 MaxModeNum = 0;
	UINTN Cols, Rows, MaxWeightedColsXRows = 0;
	EFI_STATUS Status = EFI_SUCCESS;

	for (INT32 ModeNum = 0; ModeNum < gST->ConOut->Mode->MaxMode; ModeNum++)
	{
		Status = gST->ConOut->QueryMode(gST->ConOut, ModeNum, &Cols, &Rows);

		if (EFI_ERROR(Status))
		{
			continue;
		}

		CONST UINTN WeightedColsXRows = (16 * Rows) * (10 * Cols);
		
		if (WeightedColsXRows >= MaxWeightedColsXRows)
		{
			MaxWeightedColsXRows = WeightedColsXRows;
			MaxModeNum = ModeNum;
		}
	}

	if (gST->ConOut->Mode->Mode != MaxModeNum)
	{
		Status = gST->ConOut->SetMode(gST->ConOut, MaxModeNum);
	}

	gST->ConOut->ClearScreen(gST->ConOut);
	gST->ConOut->EnableCursor(gST->ConOut, TRUE);

	return Status;
}

STATIC
EFI_STATUS
EFIAPI
StartBootKit(
	IN BOOLEAN InteractiveConfiguration
)
{
	//
	// Check if the driver is loaded 
	//

	EFIGUARD_DRIVER_PROTOCOL *EfiGateDriverProtocol;

	EFI_STATUS Status = gBS->LocateProtocol(&gEfiGateDriverProtocolGuid, NULL, (VOID **)&EfiGateDriverProtocol);
}

EFI_STATUS
EFIAPI
UefiMain(
	IN EFI_HANDLE ImageHandle,
	IN EFI_SYSTEM_TABLE *SystemTable
)
{
	//
	// Connect all drivers to all controllers
	//
	EfiBootManagerConnectAll();

	//
	// Set the highest available console mode and clear the screen
	//

	SetHighestAvailableTextMode();

	//
	// Turn off the watchdog timer
	//

	gBS->SetWatchdogTimer(0, 0, 0, NULL);

	//
	// Query the console input handle for the EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL
	//
	
	gBS->HandleProtocol(gST->ConsoleInHandle, &gEfiSimpleTextInputExProtocolGuid, (VOID **)&mTextInputEx);

	//
	// Allow user to configure the driver
	//

	Print(L"Press <F1> to configure BootKit...\r\n");

	CONST BOOLEAN InteractiveConfiguration  = WaitForKeyWithTimeout(1500) == SCAN_F1;

	//
	// Locate, load, start and configure the driver
	//

	CONST EFI_STATUS DriverStatus = StartBootKit(InteractiveConfiguration);

	if (EFI_ERROR(DriverStatus))
	{
		Print(L"ERROR: driver load failed with status %llX\r\n", DriverStatus);

		WaitForKey();

		gBS->Exit(gImageHandle, EFI_SUCCESS, 0, NULL);

		return DriverStatus;
	}

	//
	// We should never reach this unless something is seriously wrong
	//

	WaitForKey();

	gBS->Exit(gImageHandle, EFI_SUCCESS, 0, NULL);

	return EFI_SUCCESS;
}