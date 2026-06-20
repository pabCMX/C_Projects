param(
    [ValidateRange(0, 18)]
    [int]$MaxPower = 9,

    [ValidateRange(0, 18)]
    [int]$MinPower = 0,

    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

if ($MinPower -gt $MaxPower) {
    throw "MinPower must be less than or equal to MaxPower."
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$sieveExe = Join-Path $scriptDir "build\primeSearch.exe"
$lhExe = Join-Path $scriptDir "build\LHPrimeSum.exe"

if (-not $SkipBuild) {
    & make -C $scriptDir
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed."
    }
}

foreach ($exe in @($sieveExe, $lhExe)) {
    if (-not (Test-Path $exe)) {
        throw "Missing executable: $exe"
    }
}

[uint64]$n = 1
for ($power = 0; $power -le $MaxPower; ++$power) {
    if ($power -ge $MinPower) {
        Write-Host "Checking 10^$power ($n)... " -NoNewline

        $sieveSum = (& $sieveExe $n --sum-only)
        if ($LASTEXITCODE -ne 0) {
            throw "primeSearch failed for n=$n."
        }

        $lhSum = (& $lhExe $n --sum-only)
        if ($LASTEXITCODE -ne 0) {
            throw "LHPrimeSum failed for n=$n."
        }

        $sieveSum = ($sieveSum | Out-String).Trim()
        $lhSum = ($lhSum | Out-String).Trim()

        if ($sieveSum -ne $lhSum) {
            Write-Host "FAILED"
            Write-Host "primeSearch: $sieveSum"
            Write-Host "LHPrimeSum:  $lhSum"
            exit 1
        }

        Write-Host "OK ($lhSum)"
    }

    if ($power -lt $MaxPower) {
        $n *= 10
    }
}

Write-Host "All checked prime sums matched."
