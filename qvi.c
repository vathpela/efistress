#include <efi.h>
#include <efilib.h>

EFI_STATUS
efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *systab)
{
	EFI_STATUS status;
	UINT64 MaximumVariableStorageSize,
		RemainingVariableStorageSize,
		MaximumVariableSize;

	InitializeLib(image_handle, systab);

	status = systab->RuntimeServices->QueryVariableInfo(
				EFI_VARIABLE_NON_VOLATILE |
				EFI_VARIABLE_BOOTSERVICE_ACCESS |
				EFI_VARIABLE_RUNTIME_ACCESS,
				&MaximumVariableStorageSize,
				&RemainingVariableStorageSize,
				&MaximumVariableSize);
	Print(L"Maximum Variable Storage Size: %lld\r\n",
		MaximumVariableStorageSize);
	Print(L"Remaining Variable Storage Size: %lld\r\n",
		RemainingVariableStorageSize);
	Print(L"Maximum Variable Size: %lld\r\n",
		MaximumVariableSize);

	return status;
}
