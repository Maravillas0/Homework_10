$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$vivadoRoot = if ($env:XILINX_VIVADO) { $env:XILINX_VIVADO } else { 'C:\Xilinx\2025.1\Vivado' }
$updatemem = Join-Path $vivadoRoot 'bin\updatemem.bat'
$memInfo = Join-Path $projectRoot 'LED_AXI_MB.sim\sim_1\behav\xsim\LED_AXI_MB.smi'
$elf = Join-Path $PSScriptRoot 'sim_build\app_component.elf'

if (-not (Test-Path -LiteralPath $updatemem)) { throw "updatemem not found: $updatemem" }
if (-not (Test-Path -LiteralPath $memInfo)) { throw "Simulation .smi file not found: $memInfo. Generate simulation output products first." }
if (-not (Test-Path -LiteralPath $elf)) { throw "Fast simulation ELF not found: $elf" }

& $updatemem -meminfo $memInfo -data $elf -proc 'dut/design_1_i/microblaze_0' -force
if ($LASTEXITCODE -ne 0) { throw "updatemem failed with exit code $LASTEXITCODE" }

Write-Host 'Fast MicroBlaze ELF loaded into the simulation BRAM image.'
