#include <stdafx.h>
#include "inline.h"
#include <Windows.h>

#if defined(DISABLE_DETOURS)
#include <MinHook.h>
#else
#include <detours.h>
#endif

namespace hook {
	struct hook_info {
		uintptr_t* pointer_ptr;
		uintptr_t  detour;
	};
	bool install(uintptr_t* pointer_ptr, uintptr_t detour, hook_t* ph)
	{
		LONG status;
		if ((status = DetourTransactionBegin()) == NO_ERROR) {
			if ((status = DetourUpdateThread(::GetCurrentThread())) == NO_ERROR) {
				if ((status = DetourAttach((PVOID*)pointer_ptr, (PVOID)detour)) == NO_ERROR) {
					if ((status = DetourTransactionCommit()) == NO_ERROR) {
						if (ph) {
							hook_info* hi = new hook_info;
							hi->detour = detour;
							hi->pointer_ptr = pointer_ptr;
							*ph = hi;
						}
						return true;
					}
				}
			}
			DetourTransactionAbort();
		}
		::SetLastError(status);
		return false;
	}

	bool uninstall(hook_t* ph)
	{
		if (!ph || !*ph) {
			::SetLastError(ERROR_INVALID_HANDLE);
			return false;
		}
		hook_info* hi = *(hook_info**)ph;
		LONG status;
		if ((status = DetourTransactionBegin()) == NO_ERROR) {
			if ((status = DetourUpdateThread(::GetCurrentThread())) == NO_ERROR) {
				if ((status = DetourDetach((PVOID*)hi->pointer_ptr, (PVOID)hi->detour)) == NO_ERROR) {
					if ((status = DetourTransactionCommit()) == NO_ERROR) {
						delete hi;
						*ph = 0;
						return true;
					}
				}
			}
			DetourTransactionAbort();
		}
		::SetLastError(status);
		return false;
	}

}
