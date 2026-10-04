# Stack Realisation

Protected stack realisation in C.

The repository has 3 parts:

- `stack_realisation` - demo project for PC.
- `C_StackLib` - one-file C library version.
- `ESP_Arduino_StackLib` - Arduino/ESP version.

## Dependency

The PC demo version uses my helper library:

https://github.com/Winston-Arkihit/Usefull-Short-Functions

It is used for `My_assert()`.

Expected local folder layout:

```text
MyCiProjects/
|
|-- My_stacks/
|   |-- README.md
|   |-- C_StackLib/
|   |-- ESP_Arduino_StackLib/
|   |-- stack_realisation/
|       |-- stack.c
|       |-- stack.h
|       |-- code.c
|       |-- Start.bat
|
|-- my_lib_func/
    |-- Header_Only.h
```

In the GitHub version, this means that the helper library should be cloned or copied near the stack project before building the PC demo.

## Protection Modes

The stack has 3 compile modes:

```bat
-DSTACK_NO_PROTECTION
-DSTACK_CANARY_PROTECTION
-DSTACK_FULL_PROTECTION
```

If no mode is selected, full protection is used by default.

## PC Build

Run:

```bat
cd stack_realisation
Start.bat
```

Current build command:

```bat
g++ -DSTACK_FULL_PROTECTION -I "." -I "..\..\my_lib_func" -x c "code.c" -x c "stack.c" -o "code.exe"
```

## Arduino / ESP Note

`My_assert()` from `Usefull-Short-Functions` is made for normal PC C programs.

It is not a good idea to use the current PC helper header directly on Arduino/ESP, because it can contain PC-specific things like:

- `conio.h`;
- `_getch()`;
- Windows-related includes;
- ordinary `printf`/`abort` behavior.

For Arduino/ESP, use:

```text
ESP_Arduino_StackLib/Arduino_ESP_Stack.h
```

That version avoids normal `printf` in stack logic and uses Arduino-style debug output through `Serial` / `Print`.

