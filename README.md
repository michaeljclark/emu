# emu

Tiny x86 emulator example using the Windows Hypervisor Platform API.

![glcube](/images/emu.png)

## build

- requires Visual Studio 2019 on 64-bit Windows.
- requires Windows Hypervisor Platform Feature.
- requires WSL 2 Ubuntu image with gcc, binutils and make.
- requires CMake and Python interpreter.

```
cmake -B build -G "Visual Studio 16 2019" -A x64
cmake --build build --config RelWithDebInfo
build\RelWithDebInfo\emu.exe --dump-reg ^
  --kernel build\out\x86\tests\0000_kernel\system.elf
```
