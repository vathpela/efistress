#include <efi.h>
#include <efilib.h>

static UINTN number_of_variables = 32768;
static EFI_SYSTEM_TABLE *systab = NULL;

#define MIN(x,y) ((x) < (y) ? (x) : (y))

EFI_STATUS
clear_variables(CHAR16 *root, EFI_GUID guid)
{
	EFI_STATUS status = EFI_SUCCESS;
	UINTN variable_name_size = 1;
	EFI_GUID vendor_guid;

	struct {
		CHAR16 *name;
		EFI_GUID guid;
	} variables_to_remove[number_of_variables];
	UINTN n = 0;
	CHAR16 *variable_name;
	UINTN old_variable_name_size = 1;

	variable_name = AllocateZeroPool(old_variable_name_size);
	variable_name[0] = L'\0';

	while (status == EFI_SUCCESS) {
		status = systab->RuntimeServices->GetNextVariableName(
						&variable_name_size,
						variable_name,
						&vendor_guid);
		Print(L"Status: %d\r\n", status);
		if (status == EFI_NOT_FOUND) {
			Print(L"End of variable list\r\n");
			break;
		}
		if (status == EFI_BUFFER_TOO_SMALL) {
			old_variable_name_size = variable_name_size;
			CHAR16 *new = AllocatePool(old_variable_name_size);
			StrCpy(new, variable_name);
			FreePool(variable_name);
			variable_name = new;
			status = EFI_SUCCESS;
			continue;
		}

		if (!StrnCmp(variable_name, root, MIN(variable_name_size,
							StrLen(root)))) {
#if 1
			Print(L"Found variable \"%s\": ", variable_name);
			Print(L"adding to removal list\r\n");
#endif
			variables_to_remove[n].name = StrDuplicate(variable_name);
			CopyMem(&variables_to_remove[n].guid, &vendor_guid,
							sizeof (vendor_guid));
			n++;
#if 0
		} else {
			Print(L"Found variable \"%s\": ", variable_name);
			Print(L"skipping\r\n");
#endif
		}
		status = EFI_SUCCESS;
	}
	variables_to_remove[n].name = NULL;
	
	for (n = 0; variables_to_remove[n].name != NULL; n++) {
		Print(L"Clearing %s\r\n", variables_to_remove[n].name);
		systab->RuntimeServices->SetVariable(
				variables_to_remove[n].name,
				&variables_to_remove[n].guid,
				EFI_VARIABLE_NON_VOLATILE |
				EFI_VARIABLE_BOOTSERVICE_ACCESS |
				EFI_VARIABLE_RUNTIME_ACCESS,
				0, NULL);
	}
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

	Print(L"Clearing variables\r\n");
	status = clear_variables(L"FlashTest", guid);
	if (EFI_ERROR(status))
		Print(L"clear_variables(): %d\r\n");

	return 0;
}
