## Environment Varibles  
  
`MO2_MODS_PATH`: The [ModOrganizer2](https://www.modorganizer.org/) mods folder. The `dll`, `pdb` and other required filed will auto copy to `MO2_MODS_PATH/{PLUGIN_NAME}` folder when build successful if seup this env varible; 
## Build  
```shell  
cmake --preset debug-clangcl-ninjia-vcpkgcmake --build build\debug-clangcl-ninjia-vcpkg --target SimpleIMEcd build\debug-clangcl-ninjia-vcpkgcpack  
```  
## Test  
  
```shell  
cmake --build build\debug-clangcl-ninjia-vcpkg --target SimpleIMETestcd build\debug-clangcl-ninjia-vcpkg\SimpleIME && ctest  
```