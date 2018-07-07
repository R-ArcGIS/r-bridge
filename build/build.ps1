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

$Rscript="${env:R_PATH}\bin\x64\Rscript.exe"

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

$Rcmd="${env:R_PATH}\bin\x64\Rcmd.exe"

$arc_flag=0
$targets=@()
if ($target -match "desktop")
{
  #$Rcmd="${env:R_PATH}\bin\i386\Rcmd.exe"
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
    cmd /c "`"${env:VS_TOOLS}`" && msbuild ${projects_path}\R-bridge.sln /m /p:configuration=${cfg},PostBuildEventUseInBuild=false /t:Rebuild /nologo /clp:NoSummary /verbosity:minimal"
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

if ((dir ../libs/*.dll -recurse).Count -eq 0)
{
  Write-Host "Cannot find proxy dll(s) in ../libs/*.dll" -ForegroundColor Red
  Write-Host "To build windows dll call './build.bat pro release'" -ForegroundColor Yell
  Exit
}

if ($check_package)
{
  $env:_R_CHECK_FORCE_SUGGESTS_="false"
  #push-location -path build/
  Write-Host "Prepare source package..." -foregroundcolor Green
  & $Rcmd build --force --no-build-vignettes ..
  if ($lastExitCode -ne 0)
  {
    Write-Host "Rcmd build FAILED" -foregroundcolor Red
    Exit
  }

  $pkg_tag_gz=$pkg_name+"_"+$pkg_main_ver+$build_n+".tar.gz"
  Write-Host "Checking ${pkg_tag_gz}" -foregroundcolor Green
  & $Rcmd check --no-multiarch --check-subdirs=yes --no-vignettes --library=./ $pkg_tag_gz
  Exit
}

Write-Host "Prepare $pkg_name $pkg_main_ver$build_n" -foregroundcolor Green

$env:BUILD_TARGET=@("-","proxy", "proxy_pro", "proxy proxy_pro")[$arc_flag]
$arch = @("-","i386, x64", "x64", "i386, x64")[$arc_flag]
$compile_flag = @("-","--compile-both", "--no-multiarch", "--compile-both")[$arc_flag]

$new = $new -replace '^Archs:.+$', ('Archs: '+$arch)
$updateRD = $isbuild
Set-Content ../DESCRIPTION $new

Function Get-StringHash([String] $String,$HashName="MD5")
{
  $StringBuilder=New-Object System.Text.StringBuilder
  [System.Security.Cryptography.HashAlgorithm]::Create($HashName).ComputeHash([System.Text.Encoding]::UTF8.GetBytes($String))|%{
  [Void]$StringBuilder.Append($_.ToString("x2"))}
  $StringBuilder.ToString()
}

$check_sum_now = Get-ChildItem @("../R/*.R","../man/*.Rd") | Get-FileHash
$check_sum_now = $check_sum_now| Out-String
$md5 = Get-StringHash $check_sum_now
$check_sum_file = [System.IO.Path]::GetTempPath() + "${md5}.bld-chksm"

if (-Not $updateRD)
{
  $updateRD = -Not (Test-Path $check_sum_file)
}

if ($updateRD)
{
  Write-Host "New MD5 = $md5" -foregroundcolor Green
  $tmp = [System.IO.Path]::GetTempPath()
  Remove-Item -Include *.bld-chksm -Path $tmp/*
  Set-Content $check_sum_file $Null
}

if ($updateRD -or -Not (Test-Path ../inst/doc/$pkg_name.pdf))
{
  Write-Host "Creating PDF" -foregroundcolor Green
  if (-Not (Test-Path ../inst/doc/))
  {
    New-Item ../inst/doc/ -type directory | Out-Null
  }
  & $Rcmd Rd2pdf --batch --no-preview --force --pdf ../ -o ../inst/doc/$pkg_name.pdf

  if ($lastExitCode -ne 0)
  {
    Write-Host "Skip creating package PDF" -foregroundcolor Red
  }
}

$build_html_flag = @("--no-html","--html")[$build_html]
#push-location -path ../
& $Rcmd INSTALL --build --clean --preclean $compile_flag $build_html_flag --byte-compile --no-test-load --library=./ ../
$lec = $lastExitCode
if ($lec -ne 0)
{
  Write-Host "Build failed" -foregroundcolor Red
  Exit
}

$pkg_zip=$pkg_name+"_"+$pkg_main_ver+$build_n+".zip"
$arc_msg = @("-","Desktop", "Pro", "Desktop, Pro")[$arc_flag]
Write-Host "Binary package for: ArcGIS ${arc_msg} (${arch}) is in build/${pkg_zip}" -foregroundcolor Yell
Write-Host "Done" -foregroundcolor Green
