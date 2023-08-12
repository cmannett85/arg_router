### Copyright (C) 2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

# If you see 'vcpkg_package_updater.ps1 cannot be loaded because running scripts is disabled on this system',
# set the following in Powershell first:
# Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass

# If you see 'gpg: skipped "xxxxxxx": No secret key', it is because their is no GPG program
# associated with Git.  Add this:
# git config --global gpg.program "c:/Program Files (x86)/GnuPG/bin/gpg.exe"

# If a REF hash is provided, check that it is valid
param([string]$ref_hash = '')
if ($ref_hash -and $ref_hash -notmatch '^[0-9a-f]{40}$') {
    Write-Host 'Invalid REF hash'
    exit 1
}

$root_dir = "$PSScriptRoot\.."

# Use the version in the Doxyfile
Select-String -Path $root_dir\docs\Doxyfile -Pattern '^PROJECT_NUMBER         = (.*)$' `
| ForEach-Object { Set-Variable -name 'version' -Value $_.matches[0].groups[1].value }

# Create a new branch at vcpkg HEAD.  Wipe out any previous changes
Set-Location $root_dir\external\vcpkg
git checkout master
git reset --hard origin/master
git fetch
git checkout -b vcpkg_$version

# Update the versions
(Get-Content -ReadCount 0 ports/arg-router/vcpkg.json) `
    -replace '^  "version": "[0-9]+\.[0-9]+\.[0-9]+",$', "  `"version`": `"$version`"," `
| Set-Content ports/arg-router/vcpkg.json
.\vcpkg format-manifest ports/arg-router/vcpkg.json
git add ports/arg-router/vcpkg.json
git commit -m "[arg-router] Update to v$version"
.\vcpkg x-add-version arg-router

# If this run is for a test build, then update the REF entry to the specified hash
if ($ref_hash) {
    (Get-Content -ReadCount 0 ports/arg-router/portfile.cmake) `
        -replace '^    REF v.*$', "    REF $ref_hash" `
    | Set-Content ports/arg-router/portfile.cmake
}

# Manually build the test package, this will fail due to an incorrect SHA but we need it to for
# vcpkg to give us the correct one
Set-Location $root_dir\ci\vcpkg_test_project
New-Item -Path . -Name 'build' -ItemType 'directory'
Set-Location .\build
[array] $sha_output = cmake -G 'Ninja' `
    -DVCPKG_TARGET_TRIPLET=x64-windows-static `
    -DCMAKE_CXX_STANDARD=20 `
    -B . -S .. 2>&1 `
| Out-String

# Update the SHA value in the portfile, and the ref if this run is for a test build
Set-Location $root_dir\external\vcpkg
Select-String -Pattern 'Actual hash: (.*)' -InputObject $sha_output `
| ForEach-Object { Set-Variable -name 'new_sha' -Value $_.matches[0].groups[1].value.Trim() }
(Get-Content -ReadCount 0 ports/arg-router/portfile.cmake) `
    -replace '^    SHA512 [0-9a-f]{128}$', "    SHA512 $new_sha" `
| Set-Content ports/arg-router/portfile.cmake

# Re-run the build to confirm
Write-Host "Testing project against updated vcpkg..."
Set-Location $root_dir\ci\vcpkg_test_project\build
cmake -G 'Ninja' `
    -DVCPKG_TARGET_TRIPLET=x64-windows-static `
    -DCMAKE_CXX_STANDARD=20 `
    -B . -S ..
cmake --build .

Set-Location ..
Remove-Item -Path .\build -Recurse -Force

# Update the version hash for the SHA update
Set-Location $root_dir\external\vcpkg
git add ports\arg-router\portfile.cmake
git commit -m "WIP"
.\vcpkg x-add-version arg-router --overwrite-version
