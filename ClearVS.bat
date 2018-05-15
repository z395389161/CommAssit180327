
@echo off
for /d /r f:\VC++\VS2008\CommAssit %%b in (Debug) do rd /s /q "%%b"


del *.ncb /s
del *.pdb /s
del *.htm /s
del *.obj /s
del *.pch /s
del *.manifest /s
del *.tlh /s
del *.tli /s
del *.dep /s
del *.idb /s

