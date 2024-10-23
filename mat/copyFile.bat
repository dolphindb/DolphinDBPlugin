@echo off  
set "parent_folder=%~1"  src
set "target_folder=%~2" dst 
set "list_file=%~3"     
  
for /f "delims=" %%i in (%list_file%) do (  
    for %%f in ("%parent_folder%\%%i") do (  
        if exist "%%f" (  
            xcopy /Y "%%f" "%target_folder%"  
        ) else (  
            echo file %%f %%i is not found. 
        )  
    )  
)  
  
echo scuccessfully copied all files.