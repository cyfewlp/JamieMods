Write-Output "Hello, World!"

cmake --build build\debug -DOUTPUT_FOLDER="${env:MO2_MODS_PATH}\SimpleIME" --target