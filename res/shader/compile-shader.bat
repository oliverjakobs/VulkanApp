for /r %%i in (*.frag, *.vert) do %VULKAN_SDK%/Bin/glslangValidator.exe -o %%i.spv -V %%i
pause