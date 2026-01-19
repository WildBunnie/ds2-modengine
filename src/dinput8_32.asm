.model flat, C
.code
extern dinput8_original_functions:DWORD
DirectInput8Create_wrapper proc
	jmp dinput8_original_functions[0*4]
DirectInput8Create_wrapper endp
DllCanUnloadNow_wrapper proc
	jmp dinput8_original_functions[1*4]
DllCanUnloadNow_wrapper endp
DllGetClassObject_wrapper proc
	jmp dinput8_original_functions[2*4]
DllGetClassObject_wrapper endp
DllRegisterServer_wrapper proc
	jmp dinput8_original_functions[3*4]
DllRegisterServer_wrapper endp
DllUnregisterServer_wrapper proc
	jmp dinput8_original_functions[4*4]
DllUnregisterServer_wrapper endp
GetdfDIJoystick_wrapper proc
	jmp dinput8_original_functions[5*4]
GetdfDIJoystick_wrapper endp
end