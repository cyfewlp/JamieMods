<#
    A script to quickly switch differente profile for Skyrim Special Edition

    IMPORTANT: Must backup all profile files by youself that you want to switch.
#>
param (
    # height of largest column without top bar
    [string]$profile = "debug",
    [string]$SkseProfileDir = "$env:SkseProfileDir",
    [string]$SkyrimPath = "$env:Skyrim64Path"
)

## validate params

if (($null -eq $SkseProfileDir ) -or -not (Test-Path -Path $SkseProfileDir)){
    Write-Error "Invalid param SkseProfileDir, is empty or not a valid path"
    exit
}

if (($null -eq $SkyrimPath ) -or -not (Test-Path -Path $SkyrimPath)){
    Write-Error "Invalid param SkyrimPath, is empty or not a valid path"
    exit
}

$baseFiles = @(
    "SkyrimSE.exe"
    "skse64_1_6_1170.dll"
    "skse64_loader.exe"
)
$debugFiles=@("skse64_1_6_1170.pdb")
$enb_files=@("d3d11.dll", "d3dcompiler_46e.dll")
$enb_plugin=@("enblocal.ini", "enbseries.ini", "enbseries/") #endwith / as a folder

$profileConfig=@{}
$profileConfig.add("base", $baseFiles)
$profileConfig.add("debug", $debugFiles + $baseFiles)
$profileConfig.add("release", $baseFiles)
$profileConfig.add("enb", $enb_files)
$profileConfig.add("enb_plugin", $enb_plugin)

function Switch-Profile {
    
    param (
        [Parameter(Mandatory)]
        [string]$ProfileName,
        [Parameter(Mandatory)]
        [string]$SkyrimSEPath
    )

    ## Validate params
    if (-not (Test-Path -Path $SkyrimSEPath)){
        Write-Error -Message "Invalid param SkyrimSEPath: $SkyrimSEPath"
        return
    }
    $expectedProfiles=$ProfileName -split ","
    $expectedHash=@{}
    
    foreach ($expectedProfile in $expectedProfiles) {
        if (-not (Test-Path -Path "$SkseProfileDir\$expectedProfile")){
            Write-Error -Message "Invalid param ProfileName: $ProfileName"
            return
        }
        $expectedHash.add($expectedProfile, $profileConfig.$expectedProfile)
    }
    $expectedHash

    ## validate expected profile files
    $expectedHash.GetEnumerator() | ForEach-Object{
        $profile = $_.key
        foreach ($fileToCopy in $_.value) {
            if (-not (Test-Path -Path "$SkseProfileDir\$profile\$fileToCopy")) {
                Write-Error "Profile [$profile] missing file $SkseProfileDir\$profile\$fileToCopy" -Category InvalidArgument
                exit
            }
        }
    }

    $currentProfile=Get-Content -Path .\game-profile
    $currentProfile=$currentProfile -split "\n"
    Write-Host "============================"
    Write-Host "Current profile config: $currentProfile"
    foreach ($profile in $currentProfile){
        if ($profileConfig.$profile -eq $null) {
            Write-Error -Message "Invalid profile in config file: $profile"
            $profile -eq "enb"
            exit
        }
        if ($expectedHash.$profile -eq $null) {
            # remove the profile files
            Write-Host "Profile [$profile] files will be remove because not speciafied in params"
            foreach ($fileToRemove in $profileConfig.$profile) {
                Write-Host "Remove profile [$profile] file $SkyrimPath\$fileToRemove"
                Remove-Item -Path "$SkyrimPath\$fileToRemove"
            }
        } else {
            $expectedHash.remove($profile)
        }
    }
    $expectedHash.GetEnumerator() | ForEach-Object{
        $message = 'profile {0} is {1}' -f $_.key, $_.value
        Write-Output $message
        $profile = $_.key
        foreach ($fileToCopy in $_.value) {
            Write-Host "Copy profile [$profile] file $fileToCopy to $SkyrimSEPath\$fileToCopy"
            Copy-Item -Path "$SkseProfileDir\$profile\$fileToCopy" -Destination "$SkyrimSEPath\$fileToCopy" -Force
        }
    }

    Write-Host "Active profiles [$expectedProfiles] successed."
    Write-Host "Updating game-profile"
    Clear-Content .\game-profile
    foreach ($writedProfile in $expectedProfiles) {
        Add-Content -Path .\game-profile -Value $writedProfile
    }
}

function CopyDebugFile {
    [CmdletBinding()]
    param (
        [Parameter(Mandatory)]
        [string]$DebugSkyrimFileDir,
        [Parameter(Mandatory)]
        [string]$SkyrimSEPath
    )
    if (-not (Test-Path -Path $DebugSkyrimFileDir)){
        Write-Error -Message "Invalid param DebugSkyrimFileDir: $DebugSkyrimFileDir"
        return
    }
    if (-not (Test-Path -Path $SkyrimSEPath)){
        Write-Error -Message "Invalid param SkyrimSEPath: $SkyrimSEPath"
        return
    }
    
    $debugSkyrimFile="SkyrimSE.exe"
    $debugSkse1170File="skse64_1_6_1170.dll"
    $debugSkse1170PdbFile="skse64_1_6_1170.pdb"
    $debugSkseLoaderFile="skse64_loader.exe"

    if ((Test-Path -Path "$DebugSkyrimFileDir\$debugSkyrimFile") -and 
        (Test-Path -Path "$DebugSkyrimFileDir\$debugSkse1170File") -and 
        (Test-Path -Path "$DebugSkyrimFileDir\$debugSkse1170PdbFile") -and 
        (Test-Path -Path "$DebugSkyrimFileDir\$debugSkseLoaderFile") ) {
        Write-Information -MessageData "Found Skyrim & SKSE file for debug mode $debugSkyrimFile" -InformationAction Continue
        Copy-Item -Path "$DebugSkyrimFileDir\$debugSkyrimFile" -Destination "$SkyrimSEPath\$debugSkyrimFile" -Force
        Copy-Item -Path "$DebugSkyrimFileDir\$debugSkse1170File" -Destination "$SkyrimSEPath\$debugSkse1170File" -Force
        Copy-Item -Path "$DebugSkyrimFileDir\$debugSkse1170PdbFile" -Destination "$SkyrimSEPath\$debugSkse1170PdbFile"
        Copy-Item -Path "$DebugSkyrimFileDir\$debugSkseLoaderFile" -Destination "$SkyrimSEPath\$debugSkseLoaderFile" -Force
        Write-Host "[Debug]Coyied Successed."
    }
}

function CopyReleaseFile {
    [CmdletBinding()]
    param (
        [Parameter(Mandatory)]
        [string]$ReleaseSkyrimFileDir,
        [Parameter(Mandatory)]
        [string]$SkyrimSEPath
    )
    if (-not (Test-Path -Path $ReleaseSkyrimFileDir)){
        Write-Error -Message "Invalid param ReleaseSkyrimFileDir: $ReleaseSkyrimFileDir"
        return
    }
    if (-not (Test-Path -Path $SkyrimSEPath)){
        Write-Error -Message "Invalid param SkyrimSEPath: $SkyrimSEPath"
        return
    }
    
    $releaseSkyrimFile="SkyrimSE.exe"
    $releaseSkse1170File="skse64_1_6_1170.dll"
    $releaseSkseLoaderFile="skse64_loader.exe"

    if ((Test-Path -Path "$ReleaseSkyrimFileDir\$releaseSkyrimFile") -and 
        (Test-Path -Path "$ReleaseSkyrimFileDir\$releaseSkse1170File") -and 
        (Test-Path -Path "$ReleaseSkyrimFileDir\$releaseSkseLoaderFile") ) {
        Write-Information -MessageData "Found Skyrim & SKSE file for release mode $releaseSkyrimFile" -InformationAction Continue
        Copy-Item -Path "$ReleaseSkyrimFileDir\$releaseSkyrimFile" -Destination "$SkyrimSEPath\$releaseSkyrimFile" -Force
        Copy-Item -Path "$ReleaseSkyrimFileDir\$releaseSkse1170File" -Destination "$SkyrimSEPath\$releaseSkse1170File" -Force
        Copy-Item -Path "$ReleaseSkyrimFileDir\$releaseSkseLoaderFile" -Destination "$SkyrimSEPath\$releaseSkseLoaderFile" -Force
        Write-Host "[Release]Coyied Successed."
    }
}

#if ($profile -eq "debug") {
#    Write-Host "Copy from dir [$SkseProfileDir\debug] to [$SkyrimPath]"
#    CopyDebugFile -DebugSkyrimFileDir "$SkseProfileDir\debug" -SkyrimSEPath "$SkyrimPath"
#    exit
#}
#
#if ($profile -eq "release") {
#    Write-Host "Copy from dir [$SkseProfileDir\debug] to [$SkyrimPath]"
#    CopyReleaseFile -ReleaseSkyrimFileDir "$SkseProfileDir\release" -SkyrimSEPath "$SkyrimPath"
#    exit
#}

Switch-Profile -ProfileName "$profile" -SkyrimSEPath "$SkyrimPath"