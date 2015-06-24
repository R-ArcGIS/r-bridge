Building rarcproxy.dll

prepare R.lib file
---------------------
open MSVC Developer Command Prompt
Use the folowing commands:

>dumpbin /exports R.dll > R.def
edit R.def like this:

LIBRARY R
EXPORTS
  ATTRIB
  AllDevicesKilled
...

>lib /def:R.def /MACHINE:x86 /OUT:R.lib


//build package 
>Rcmd.exe INSTALL --build --library=D:\ArcGIS\help\gp\R arc