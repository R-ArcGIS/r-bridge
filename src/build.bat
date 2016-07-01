@@echo off
@@set target=%1
@@set isbld=%2
@@if -%1-==-- set target=both
@@if -%2-==-- set isbld=false
@@findstr/v "^@@.*" "%~f0" > "%TMP%\b0.ps1" & powershell -ExecutionPolicy ByPass "%TMP%\b0.ps1" -isbld %isbld% -target %target% -curdir %~dp0 & del "%TMP%\b0.ps1" & goto:eof

#### PowerShell

param($curdir, $target, $isbld)

#### correct if needed
$R_PATH="C:\Program Files\R\R-3.2.0"
$msbuild_path ="C:\Program Files (x86)\MSBuild\14.0\Bin"
$r_tools="C:\Rtools\bin;C:\Rtools\gcc-4.6.3\bin"

$PKG_MAIN_VER="1.0.0."
$Rscript="$R_PATH\bin\x64\Rscript.exe"
$pkg_name="arcgisbinding"


if (($isbld -ne "compile" -and $isbld -ne "false") -or ($target -ne "pro" -and $target -ne "desktop" -and $target -ne "both"))
{
  Write-Host "Incorrect parameters. Use: build.bat [both|pro|desktop] [compile]" -foregroundcolor Red
  Exit
}

$isbuild=$isbld -eq "compile"

$env:Path= $r_tools + ";" + $msbuild_path + ";" + $env:Path

# use relative path from package folder
Set-Location $curdir\package

# add build number to the package version - first line
$build_n = ((Get-Content ..\buildnum.h) | select-string -pattern 'BUILD_NUM+ (\d+)').Matches.Groups[1].Value
Write-Host "Begin building $PKG_NAME $PKG_MAIN_VER$BUILD_N" -foregroundcolor Green

$descFile = "arc\DESCRIPTION"
$desc = Get-Content $descFile

$new = $desc -replace '^Version:.+$', ('Version: ' + $PKG_MAIN_VER + $build_n)
$new = $new -replace '^Date:.+$', ('Date: '+(Get-Date -uformat "%Y-%m-%d"))

$compile_flag = "--compile-both"
$Rcmd="$R_PATH\bin\x64\Rcmd.exe"

$build=""
$arch =""

if ($target -eq "desktop")
{
  $isbuild = (-Not (Test-Path ..\..\bin32\rarcproxy.dll)) -or $isbuild
  $Rcmd="$R_PATH\bin\i386\Rcmd.exe"
  $build="proxy"
  $arch_msg = "ArcGIS Desktop"
  $arch = "i386, x64"
  if ($isbuild){ $build="build_proxy proxy"}
}
if ($target -eq "pro")
{
  $isbuild = (-Not (Test-Path ..\..\bin64\rarcproxy_pro.dll)) -or $isbuild
  $build="proxy_pro"
  $arch = "x64"
  $compile_flag = "--no-multiarch"
  $arch_msg = "ArcGIS Pro"
  if ($isbuild){ $build="build_proxy_pro proxy_pro" }
}
if ($target -eq "both")
{
  $isbuild = (-Not ((Test-Path ..\..\bin64\rarcproxy_pro.dll) -and (Test-Path ..\..\bin64\rarcproxy.dll) -and (Test-Path ..\..\bin32\rarcproxy.dll))) -or $isbuild
  $build="proxy proxy_pro"
  $arch = "i386, x64"
  $arch_msg = "ArcGIS Desktop and ArcGIS Pro"
  if ($isbuild){ $build="build_proxy proxy build_proxy_pro proxy_pro"}
}

$env:BUILD_TARGET=$build

$new = $new -replace '^Archs:.+$', ('Archs: '+$arch)

if (Compare-Object $new $desc)
{
  Write-Host "Update DESCRIPTION file" -foregroundcolor Yell
  Set-Content $descFile $new
}

Write-Host "Creating .RD files" -foregroundcolor Green
& $Rscript --vanilla --verbose -e "roxygen2::roxygenize('arc')"
if ($lastExitCode -ne 0)
{
  Write-Host "Ignore. Propably roxygen2 is not installed" -foregroundcolor Red
}

Write-Host "Creating PDF" -foregroundcolor Green
& $Rcmd Rd2pdf --batch --no-preview --force --pdf arc -o arc/inst/doc/$pkg_name.pdf

if ($lastExitCode -ne 0)
{
  #Write-Host "Failed" -foregroundcolor Red
  Write-Host "Ignore building PDF" -foregroundcolor Red
}

Write-Host "Building package" -foregroundcolor Green
& $Rcmd INSTALL --build --clean $compile_flag --html --library=../../ arc

if ($lastExitCode -ne 0)
{
  Write-Host "Failed" -foregroundcolor Red
  Exit
}
Remove-Item -Path arc\src\*.dll
$pkg_zip=$pkg_name+"_1.0.0."+$build_n+".zip"

Write-Host "Copy" $pkg_zip -foregroundcolor Green
Move-Item -force -Path $pkg_zip -Destination ..\..\
if ($lastExitCode -ne 0)
{
  Write-Host "Failed" -foregroundcolor Red
  Exit
}
Write-Host "Package supported: $ARCH_MSG ($ARCH)" -foregroundcolor Yell
Write-Host "Done." -foregroundcolor Green
