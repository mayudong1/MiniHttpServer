for /r %%i in (debug release ipch) do rd /s /q "%%i"
del *.ncb /s
del *.user /s
del *.sdf /s
del *.suo /s /a
del *.o /s
del *.exe /s
pause
