#include <efi.h>
#include <efilib.h>

static UINTN number_of_variables = 0;
static EFI_SYSTEM_TABLE *systab = NULL;

EFI_STATUS
fill_variables(CHAR16 *root, EFI_GUID guid, int max)
{
	UINTN size = max;
	EFI_STATUS status = EFI_SUCCESS;
	UINTN rootlen = StrLen(root);
	CHAR16 hex[] = L"0123456789abcdef";
	int n = 0;

	while((status == EFI_SUCCESS && size >= 1) ||
			(status == EFI_OUT_OF_RESOURCES && size >= 64)) {
		UINT8 buffer[size];
		CHAR16 name[rootlen + 5];

		StrCpy(name, root);
		name[rootlen+0] = hex[(n & 0xf000) >> 12];
		name[rootlen+1] = hex[(n & 0x0f00) >> 8];
		name[rootlen+2] = hex[(n & 0x00f0) >> 4];
		name[rootlen+3] = hex[(n & 0x000f) >> 0];
		name[rootlen+4] = L'\0';
		Print(L"Making variable %s with size %d\r\n", name, size);

		status = systab->RuntimeServices->SetVariable(name, &guid,
					EFI_VARIABLE_NON_VOLATILE |
					EFI_VARIABLE_BOOTSERVICE_ACCESS |
					EFI_VARIABLE_RUNTIME_ACCESS,
					size, buffer);
		switch (status) {
		case EFI_INVALID_PARAMETER:
			status = EFI_SUCCESS;
			n++;
		case EFI_OUT_OF_RESOURCES:
			status = EFI_SUCCESS;
			size /= 2;
			break;
		case EFI_ACCESS_DENIED:
			status = EFI_SUCCESS;
			n++;
			break;
		case EFI_NOT_FOUND:
			Print(L"Platform erroneously reports EFI_NOT_FOUND\r\n");
			status = EFI_OUT_OF_RESOURCES;
			break;
		case EFI_SUCCESS:
			Print(L"Succeeded in making variable %s with "
				L"size %d\r\n", name, size);
			n++;
			break;
		default:
			Print(L"Error: %d\r\n", status);
			return status;
		}
	}
	number_of_variables = n;
	return EFI_SUCCESS;
}

EFI_STATUS
efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *passed_systab)
{
	EFI_GUID guid = {0xe9c7d2b3, 0x14fb, 0x4b50,
				{0xbb,0x50,0x3e,0x85,0xd6,0x32,0xd5,0x4b}};
	EFI_STATUS status;
	UINT64 MaximumVariableStorageSize,
		RemainingVariableStorageSize,
		MaximumVariableSize;

	systab = passed_systab;
	
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

	systab->BootServices->Stall(2000000);

	Print(L"Filling variables\r\n");
	status = fill_variables(L"FlashTest", guid, MaximumVariableSize);
	if (EFI_ERROR(status) && status != EFI_OUT_OF_RESOURCES)
		Print(L"fill_variables(): %d\r\n", status);
	
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

	return 0;
}
