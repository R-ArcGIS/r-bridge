@@echo off
@@set target=%1
@@set isbld=%2
@@if -%1-==-- set target=both
@@if -%2-==-- set isbld=false
@@findstr/v "^@@.*" "%~f0" > "%TMP%\b0.ps1" & powershell -ExecutionPolicy ByPass "%TMP%\b0.ps1" -isbld %isbld% -target %target% -curdir %~dp0 & del "%TMP%\b0.ps1" & pause & goto:eof
param($curdir, $target, $isbld)

#### correct if needed
$R_PATH="${env:programfiles}\R\R-3.2.0"
$R_TOOLS="C:\Rtools\bin"

if (-Not (Test-Path $R_PATH))
{
  Write-Host "Cannot find R 3.2.0" -foregroundcolor Red
  Write-Host "If your have R installed in defferent location change build.bar variable:" -foregroundcolor Yell
  Write-Host '$R_PATH='$R_PATH -foregroundcolor Magenta
  Exit
}
if (-Not (Test-Path $R_TOOLS))
{
  Write-Host "Cannot find RTools" -foregroundcolor Red
  Write-Host "If your have RTools installed in defferent location change build.bar variable:" -foregroundcolor Yell
  Write-Host '$R_TOOLS='$R_TOOLS -foregroundcolor Magenta
  Exit
}
$env:Path= "${R_TOOLS};${env:Path}"

$msbuild_path = "${env:programfiles(x86)}\MSBuild\14.0\bin"
if (Test-Path $msbuild_path)
{
  $env:Path= "${msbuild_path};${env:Path}"
}
else
{
  $msbuild_path = "${env:programfiles}\MSBuild\14.0\bin"
  if (Test-Path $msbuild_path)
  {
    $env:Path= "${msbuild_path};${env:Path}"
  }
  else
  {
    $msbuild_path = ""
  }
}

$Rscript="${R_PATH}\bin\x64\Rscript.exe"
$pkg_name="arcgisbinding"

if ($target -notin "compile", "pro", "desktop", "both" -or $isbld -notin "compile", "false")
{
  Write-Host "Incorrect parameters. Use: build.bat [both|pro|desktop] [compile]" -foregroundcolor Red
  Exit
}

$isbuild=$isbld -in ,"compile"
if ($target -in ,"compile")
{
  $isbuild=$true
  $target="both"
}

$PKG_MAIN_VER="1.0.0."

# use relative path from package folder
push-location -path $curdir

# add build number to the package version - first line
$build_n = ((Get-Content .\buildnum.h) | select-string -pattern 'BUILD_NUM+ (\d+)').Matches.Groups[1].Value
Write-Host "Begin building ${PKG_NAME} ${PKG_MAIN_VER}${BUILD_N}" -foregroundcolor Green

$descFile = "package/DESCRIPTION"
$desc = Get-Content $descFile

$new = $desc -replace '^Version:.+$', ('Version: ' + $PKG_MAIN_VER + $build_n)
$new = $new -replace '^Date:.+$', ('Date: '+(Get-Date -uformat "%Y-%m-%d"))

$compile_flag = "--compile-both"
$Rcmd="${R_PATH}\bin\x64\Rcmd.exe"

$build=""
$arch =""

if ($target -in "desktop")
{
  $isbuild = (-Not (Test-Path ../lib/x86/rarcproxy.dll)) -or $isbuild
  $Rcmd="${R_PATH}\bin\i386\Rcmd.exe"
  $build="proxy"
  $arch_msg = "ArcGIS Desktop"
  $arch = "i386, x64"
  if ($isbuild){ $build="build_proxy proxy"}
}
elseif ($target -eq "pro")
{
  $isbuild = (-Not (Test-Path ../lib/x64/rarcproxy_pro.dll)) -or $isbuild
  $build="proxy_pro"
  $arch = "x64"
  $compile_flag = "--no-multiarch"
  $arch_msg = "ArcGIS Pro"
  if ($isbuild){ $build="build_proxy_pro proxy_pro" }
}
elseif ($target -eq "both")
{
  $isbuild = (-Not ((Test-Path ../lib/x64/rarcproxy_pro.dll) -and
                    (Test-Path ../lib/x64/rarcproxy.dll) -and
                    (Test-Path ../lib/x86/rarcproxy.dll))
             ) -or $isbuild
  $build="proxy proxy_pro"
  $arch = "i386, x64"
  $arch_msg = "ArcGIS Desktop and ArcGIS Pro"
  if ($isbuild){ $build="build_proxy proxy build_proxy_pro proxy_pro"}
}

if ($isbuild -and $msbuild_path -eq "")
{
  Write-Host "Cannot find MS VisualStudio 2015" -foregroundcolor Red
  Exit
}

$env:BUILD_TARGET=$build

$new = $new -replace '^Archs:.+$', ('Archs: '+$arch)
$updateRD = $isbuild
if (Compare-Object $new $desc)
{
  Write-Host "Update DESCRIPTION file" -foregroundcolor Yell
  Set-Content $descFile $new
  $updateRD = $true
}

if ($updateRD)
{
  Write-Host "Re-create '.RD' files" -foregroundcolor Green
  & $Rscript --vanilla -e "roxygen2::roxygenize('package')"
  if ($lastExitCode -ne 0)
  {
    Write-Host "Skip creating new '.DR' files. Check if roxygen2 is installed" -foregroundcolor Red
  }
}

if ($isbuild -or -Not (Test-Path package/inst/doc/$pkg_name.pdf))
{
  Write-Host "Creating PDF" -foregroundcolor Green
  & $Rcmd Rd2pdf --batch --no-preview --force --pdf package -o package/inst/doc/$pkg_name.pdf

  if ($lastExitCode -ne 0)
  {
    Write-Host "Skip creating package PDF" -foregroundcolor Red
  }
}

Write-Host "Building package" -foregroundcolor Green
& $Rcmd INSTALL --build --clean $compile_flag --html --library=../ package

if ($lastExitCode -ne 0)
{
  Remove-Item -Path package/src/*.dll
  Write-Host "Failed" -foregroundcolor Red
  Exit
}
Remove-Item -Path package/src/*.dll
$pkg_zip=$pkg_name+"_1.0.0."+$build_n+".zip"

Write-Host "Copy" $pkg_zip -foregroundcolor Green
Move-Item -force -Path $pkg_zip -Destination ../
if ($lastExitCode -ne 0)
{
  Write-Host "Failed" -foregroundcolor Red
  Exit
}
Write-Host "Package supported: ${ARCH_MSG} (${ARCH})" -foregroundcolor Yell
Write-Host "Done." -foregroundcolor Green
