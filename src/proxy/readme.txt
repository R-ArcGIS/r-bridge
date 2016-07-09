Build all:
> build.bat compile

----------------------------
update R.def file

>dumpbin /exports R.dll > R.def

edit R.def like this:
LIBRARY R
EXPORTS
  ATTRIB
  AllDevicesKilled
...
