$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw 'CMake est introuvable. Installer CMake et les outils C++ de Visual Studio, puis relancer.'
}
$commit = '51a8a6d7aeae7326956d747737ccf1575e61e209' # JUCE 8.0.6
$deps = Join-Path $PSScriptRoot '.deps'
$juce = Join-Path $deps ('JUCE-' + $commit)
if (-not (Test-Path (Join-Path $juce 'CMakeLists.txt'))) {
    New-Item -ItemType Directory -Force -Path $deps | Out-Null
    $archive = Join-Path $deps 'JUCE_8_0_6.zip'
    Write-Host 'Telechargement des sources officielles JUCE 8.0.6...'
    Invoke-WebRequest -Uri ('https://github.com/juce-framework/JUCE/archive/' + $commit + '.zip') -OutFile $archive
    Expand-Archive -Path $archive -DestinationPath $deps -Force
}
cmake -S . -B build-win -A x64 -DANKS_BUILD_PLUGIN=ON "-DANKS_JUCE_PATH=$juce"
if ($LASTEXITCODE -ne 0) { throw 'Configuration echouee. Verifier Visual Studio C++ et le SDK Windows.' }
cmake --build build-win --config Release --target test_six_slots test_core test_bypass_clicks test_realtime bench_audio test_fm test_creative test_effects test_rack test_sequencer test_audio_io test_plugin Modular_VST3 Modular_Standalone
if ($LASTEXITCODE -ne 0) { throw 'Compilation echouee.' }
ctest --test-dir build-win -C Release --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'Les tests ont echoue.' }
& (Join-Path $PSScriptRoot 'build-win/Release/bench_audio.exe') --full | Out-File -Encoding utf8 (Join-Path $PSScriptRoot 'build-win/performance.csv')
if ($LASTEXITCODE -ne 0) { throw 'Le banc audio a echoue.' }
$plugin = Get-ChildItem -Path 'build-win/Modular_artefacts/Release/VST3' -Directory -Filter '*.vst3' | Select-Object -First 1
if (-not $plugin) { throw 'Compilation terminee mais dossier VST3 introuvable.' }
Write-Host ('VST3 cree : ' + $plugin.FullName)
Write-Host 'Le script ne copie rien dans les dossiers Ableton ou systeme.'

$previewApp = Get-ChildItem -Path 'build-win/test_plugin_artefacts/Release' -Filter '*.exe' | Select-Object -First 1
$env:ANKS_UI_PREVIEW = Join-Path $PSScriptRoot 'build-win/Modular-preview.png'
& $previewApp.FullName
if ($LASTEXITCODE -ne 0) { throw 'Verification interface echouee.' }
$env:ANKS_UI_MINIMUM = '1'
$env:ANKS_UI_PREVIEW = Join-Path $PSScriptRoot 'build-win/Modular-preview-minimum.png'
& $previewApp.FullName
if ($LASTEXITCODE -ne 0) { throw 'Verification taille minimum echouee.' }
Remove-Item Env:ANKS_UI_PREVIEW
Remove-Item Env:ANKS_UI_MINIMUM
