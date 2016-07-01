Building proxy dll (one or both configuration)
for ArcGISPro (optional):
> msbuild rarcproxy.vcxproj /p:configuration=Release-Pro  /p:platform=x64

for ArcGIS Desktop 10.x (optional):
> msbuild rarcproxy.vcxproj /p:configuration=Release-Desktop /p:platform=x64
> msbuild rarcproxy.vcxproj /p:configuration=Release-Desktop /p:platform=win32

Build R arrcgisbinding package:
> cd package/
> build.bat

----------------------------
update R.def file

>dumpbin /exports R.dll > R.def

edit R.def like this:
LIBRARY R
EXPORTS
  ATTRIB
  AllDevicesKilled
...
