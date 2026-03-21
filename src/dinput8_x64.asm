.code
extern dinput8_original_functions:QWORD
DirectInput8Create_wrapper proc
	jmp dinput8_original_functions[0*8]
DirectInput8Create_wrapper endp
DllCanUnloadNow_wrapper proc
	jmp dinput8_original_functions[1*8]
DllCanUnloadNow_wrapper endp
DllGetClassObject_wrapper proc
	jmp dinput8_original_functions[2*8]
DllGetClassObject_wrapper endp
DllRegisterServer_wrapper proc
	jmp dinput8_original_functions[3*8]
DllRegisterServer_wrapper endp
DllUnregisterServer_wrapper proc
	jmp dinput8_original_functions[4*8]
DllUnregisterServer_wrapper endp
GetdfDIJoystick_wrapper proc
	jmp dinput8_original_functions[5*8]
GetdfDIJoystick_wrapper endp
end