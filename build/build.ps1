param(
  $target,
  $isbuild,
  $config,
  $set_date,
  $increment_build,
  $check_package,
  $build_html
)
[bool]$isbuild = $isbuild -eq "true"
[bool]$set_date = $set_date -eq "true"
[bool]$increment_build = $increment_build -eq "true"
[bool]$check_package = $check_package -eq "true"
[bool]$build_html = $build_html -eq "true"
$curdir = Split-Path $MyInvocation.MyCommand.Path
$projects_path = Resolve-Path "${curdir}/../src/msprojects"

#Write-Host "target=${target}, isbuild=${isbuild}, set_date=${set_date}, config=${config}"
#$tmp=(get-childitem env:) | Out-String
#Write-Host $increment_build

if (-Not (Test-Path $env:R_PATH))
{
  Write-Host "Cannot find R in ${env:R_PATH}" -foregroundcolor Red
  Write-Host "Install R for Windows from https://cran.r-project.org/bin/windows/base/" -foregroundcolor Yell
  Write-Host "If your have R installed in defferent location change 'R_PATH' in build.bat" -foregroundcolor Yell
  Exit
}
if (-Not (Test-Path $env:R_TOOLS))
{
  Write-Host "Cannot find RTools in ${env:R_TOOLS}" -foregroundcolor Red
  Write-Host "Install RTools from https://cran.r-project.org/bin/windows/Rtools/" -foregroundcolor Yell
  Write-Host "If your have RTools installed in defferent location change 'R_TOOLS' in build.bat" -foregroundcolor Yell
  Exit
}
$env:Path="${env:R_TOOLS};${env:Path}"

$descFile = "../DESCRIPTION.in"

# use relative path from package folder
push-location -path $curdir

$desc = Get-Content $descFile

$pkg_name=($desc | select-string -pattern "^Package: ([a-z]+)").Matches.Groups[1].Value
$pkg_main_ver=($desc | select-string -pattern "^Version: (\d{1}\.\d{1}.\d{1}\.)").Matches.Groups[1].Value

$output_package_path = "./${pkg_name}"
#Write-Host "${output_package_path} shoud be a folder" -foregroundcolor Red
if (Test-Path $output_package_path -PathType Leaf)
{
  Write-Host "${output_package_path} shoud be a folder" -foregroundcolor Red
  Remove-Item $output_package_path
}
# add build number to the package version - first line
$build_n = (select-string -Path $projects_path/buildnum.h -pattern 'BUILD_NUM+ (\d+)').Matches.Groups[1].Value
$build_n = [int]$build_n

if ($increment_build)
{
  $set_date = $true
  if ($isbuild)
  {
    $build_n++
    Write-Host "Incrementing build number $build_n" -foregroundcolor Magenta
    $f = Get-Content $projects_path/buildnum.h
    $f = $f -replace 'BUILD_NUM+ (\d+)', "BUILD_NUM $build_n"
    Set-Content $projects_path/buildnum.h $f
  }
  else
  {
    Write-Host "Ignore incrementing build number. Please set 'release|debug' parameter " -foregroundcolor Red
  }
}

$Rcmd="${env:R_PATH}\bin\x64\R.exe"
$RcmdVer=((Get-Item $Rcmd).VersionInfo.FileVersion  -split ' ')[0]
Write-Host "R.exe version: $RcmdVer"

$arc_flag=0
$targets=@()
if ($target -match "desktop")
{
  $arc_flag = $arc_flag+1
  $targets += "${config}-Desktop,platform=Win32"
  $targets += "${config}-Desktop,platform=x64"
}
if ($target -match "pro")
{
  $arc_flag = $arc_flag+2
  $targets += "${config}-Pro,platform=x64"
}

$new = $desc -replace '^Version:.+$', ('Version: ' + $pkg_main_ver + $build_n)
$dependsR = "Depends: R (>= $RcmdVer)"
$new = $new -replace '^Depends:.+$', ($dependsR)
if ($set_date)
{
  $new = $new -replace '^Date:.+$', ('Date: '+(Get-Date -uformat "%Y-%m-%d"))
  Set-Content $descFile $new
}

if ($isbuild)
{
  function bld
  {
    param($cfg)
    Write-Host "Compiling: ${cfg}" -foregroundcolor Green
    Write-Host "R_PATH=$env:R_PATH"
    if ($target -match "pro")
    {
      Write-Host "ARCGIS_PRO_PATH=$env:ARCGIS_PRO_PATH"
    }
    else
    {
      Write-Host "ARCGIS_DESKTOP_KIT_PATH=$env:ARCGIS_DESKTOP_KIT_PATH"
    }
    & cmd.exe /c """$($env:VS_TOOLS)"" && msbuild ${projects_path}\R-bridge.sln /m /p:configuration=${cfg},PostBuildEventUseInBuild=false /t:Rebuild /nologo /clp:NoSummary /verbosity:minimal"
    if ($lastExitCode -ne 0)
    {
      Write-Host "Build Failed" -foregroundcolor Red
      Exit
    }
  }
  if (-Not (Test-Path $env:VS_TOOLS))
  {
    Write-Host "Failed to find Visual Studio installation." -ForegroundColor Red
    Exit
  }
  #if not defined VisualStudioVersion call "%VS140COMNTOOLS%\..\..\vc\vcvarsall.bat"
  foreach ($e in $targets) {bld $e}
}

if ((Get-ChildItem -Path ../libs/ -Include *.dll -Recurse -Name).Count -eq 0)
{
  Write-Host "Cannot find proxy dll(s) in ../libs/*.dll" -ForegroundColor Red
  Write-Host "To build windows dll call './build.bat pro release'" -ForegroundColor Yell
  Exit
}

if ($check_package)
{
  $env:_R_CHECK_FORCE_SUGGESTS_="false"
  #push-location -path build/
  $compile_flag = @("-","--no-multiarch", "--no-multiarch", "--multiarch")[$arc_flag]
  $arch = @("-","i386", "x64", "i386, x64")[$arc_flag]
  if ($arc_flag -eq 1)
  {
    $Rcmd="${env:R_PATH}\bin\i386\R.exe"
  }
  $pkg_tag_gz=$pkg_name+"_"+$pkg_main_ver+$build_n+".tar.gz"

  Write-Host "Prepare source package '$pkg_tag_gz' Archs: $arch ..." -foregroundcolor Green
  $new = $new -replace '^Archs:.+$', ('Archs: '+$arch)
  Set-Content ../DESCRIPTION $new
  & $Rcmd CMD build --force --no-build-vignettes ..
  if ($lastExitCode -ne 0)
  {
    Write-Host "Rcmd build FAILED" -foregroundcolor Red
    Exit
  }

  Write-Host "Checking ${pkg_tag_gz}" -foregroundcolor Green
  & $Rcmd CMD check $compile_flag --check-subdirs=yes --no-vignettes --library=./ $pkg_tag_gz
  Exit
}

$env:BUILD_TARGET=@("-","proxy", "proxy_pro", "proxy proxy_pro")[$arc_flag]
$arch = @("-","i386, x64", "x64", "i386, x64")[$arc_flag]
$compile_flag = @("-","--compile-both", "--no-multiarch", "--compile-both")[$arc_flag]

Write-Host "Prepare $pkg_name $pkg_main_ver$build_n" -foregroundcolor Green

$new = $new -replace '^Archs:.+$', ('Archs: '+$arch)
$updateRD = $isbuild
Set-Content ../DESCRIPTION $new

Function Get-StringHash([String] $String,$HashName="MD5")
{
  $ret=""
  $hashByteArray = [System.Security.Cryptography.HashAlgorithm]::Create($HashName).ComputeHash([System.Text.Encoding]::UTF8.GetBytes($String))
  foreach($byte in $hashByteArray)
  {
    $ret += $byte.ToString("x2")
  }
  $ret
}

$check_sum_now = Get-ChildItem @("../R/*.R","../man/*.Rd") | Get-FileHash
$check_sum_now = $check_sum_now| Out-String
$md5 = Get-StringHash $check_sum_now
$check_sum_file = [System.IO.Path]::GetTempPath() + "{${md5}}.bld-log"

if (-Not $updateRD)
{
  $updateRD = -Not (Test-Path $check_sum_file)
}

if ($updateRD)
{
  Write-Host "New MD5 = $md5" -foregroundcolor Green
  $tmp = [System.IO.Path]::GetTempPath()
  Remove-Item -Include *.bld-log -Path $tmp/*
  Set-Content $check_sum_file $Null
}

if ($updateRD -or -Not (Test-Path ../inst/doc/$pkg_name.pdf))
{
  Write-Host "Creating PDF" -foregroundcolor Green
  if (-Not (Test-Path ../inst/doc/))
  {
    New-Item ../inst/doc/ -type directory | Out-Null
  }
  & $Rcmd CMD Rd2pdf --batch --no-preview --force --pdf ../ -o ../inst/doc/$pkg_name.pdf

  if ($lastExitCode -ne 0)
  {
    Write-Host "Skip creating package PDF" -foregroundcolor Red
  }
}
$is35 = $env:R_PATH.contains("\R-3.");
$namespaceFile = "../NAMESPACE"
if ($is35)
{
  Write-Host "Fixing R 3.5 NAMESPACE"
  $namespace = Get-Content $namespaceFile
  Move-Item $namespaceFile "$namespaceFile.bk" -Force
  $namespace = $namespace -replace '^S3method\(leaflet::.+$', ('##')
  Set-Content $namespaceFile $namespace
}

$build_html_flag = @("--no-html","--html")[$build_html]
#push-location -path ../
& $Rcmd CMD INSTALL --build --clean --preclean $compile_flag $build_html_flag --byte-compile --no-test-load --library=./ ../
$lec = $lastExitCode

if ($is35)
{
  Remove-Item $namespaceFile
  Move-Item "$namespaceFile.bk" $namespaceFile
}
if ($lec -ne 0)
{
  Write-Host "Build failed" -foregroundcolor Red
  Exit
}

$pkg_zip=$pkg_name+"_"+$pkg_main_ver+$build_n+".zip"
$arc_msg = @("-","Desktop", "Pro", "Desktop, Pro")[$arc_flag]
Write-Host "Binary package for: ArcGIS ${arc_msg} (${arch}) is in build/${pkg_zip}" -foregroundcolor Yell
Write-Host "Done" -foregroundcolor Green
