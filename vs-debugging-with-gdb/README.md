# VS Debugging with GDB

* The libraries in `C:\msys64\mingw64\bin` must be on the path when you execute a program built
  with MinGW. You can temporarily add to the path in powershell with:
  ```
  $env:Path = 'C:\msys64\mingw64\bin;' + $env:Path
  ```
* You can view the powershell path with:
  ```
  $env:Path
  ```
* Always compile with `-g` option to get debug symbols.