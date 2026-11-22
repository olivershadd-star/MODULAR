cmake_minimum_required(VERSION 3.22)
project(Modular VERSION 1.4.0 LANGUAGES C CXX)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(test_core tests/test_engine.cpp)
target_include_directories(test_core PRIVATE src)
enable_testing()
find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
 add_test(NAME factory_bank COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_factory_bank.py")
endif()
add_test(NAME core COMMAND test_core)
add_executable(test_sequencer tests/test_sequencer.cpp)
target_include_directories(test_sequencer PRIVATE src)
add_test(NAME sequencer COMMAND test_sequencer)
add_executable(test_rack tests/test_rack.cpp)
target_include_directories(test_rack PRIVATE src)
add_test(NAME rack COMMAND test_rack)
add_executable(test_effects tests/test_effects.cpp)
target_include_directories(test_effects PRIVATE src)
add_test(NAME effects COMMAND test_effects)
add_executable(test_creative tests/test_creative.cpp)
target_include_directories(test_creative PRIVATE src)
add_test(NAME creative COMMAND test_creative)
add_executable(test_fm tests/test_fm.cpp)
target_include_directories(test_fm PRIVATE src)
add_test(NAME fm COMMAND test_fm)
add_executable(test_realtime tests/test_realtime.cpp)
target_include_directories(test_realtime PRIVATE src)
add_test(NAME realtime COMMAND test_realtime)
add_executable(test_bypass_clicks tests/test_bypass_clicks.cpp)
target_include_directories(test_bypass_clicks PRIVATE src)
add_test(NAME bypass_clicks COMMAND test_bypass_clicks)
add_executable(test_six_slots tests/test_six_slots.cpp)
target_include_directories(test_six_slots PRIVATE src)
add_test(NAME six_slots COMMAND test_six_slots)
add_executable(bench_audio tests/bench_audio.cpp)
target_include_directories(bench_audio PRIVATE src)
set(ANKS_REFERENCE_SOURCE "" CACHE PATH "Reference checkout for optional paired DSP benchmark")
if(ANKS_REFERENCE_SOURCE)
 add_executable(bench_compare tests/bench_compare.cpp)
 target_include_directories(bench_compare PRIVATE src)
 target_compile_definitions(bench_compare PRIVATE "ANKS_REFERENCE_ENGINE=\"${ANKS_REFERENCE_SOURCE}/src/Engine.h\"")
endif()
option(ANKS_BUILD_PLUGIN "Build the experimental JUCE wrapper" OFF)
set(ANKS_JUCE_PATH "" CACHE PATH "Path to a local JUCE source checkout")
if(ANKS_BUILD_PLUGIN)
 if(ANKS_JUCE_CONFIG_PATH)
  find_package(JUCE CONFIG REQUIRED PATHS "${ANKS_JUCE_CONFIG_PATH}" NO_DEFAULT_PATH)
 else()
 if(NOT EXISTS "${ANKS_JUCE_PATH}/CMakeLists.txt")
  message(FATAL_ERROR "Set ANKS_JUCE_PATH to a JUCE source folder")
 endif()
 add_subdirectory("${ANKS_JUCE_PATH}" "${CMAKE_BINARY_DIR}/JUCE")
 endif()
 juce_add_binary_data(ANKSFactory NAMESPACE ANKSFactory HEADER_NAME FactoryBinary.h SOURCES
  Excitations/Clap_Triplet.wav
  Excitations/FM_C2.wav
  Excitations/Friction_Rugueuse.wav
  Excitations/Grain_Numerique.wav
  Excitations/Impact_Sec.wav
  Excitations/Kick_Pulse.wav
  Excitations/Lame_110Hz.wav
  Excitations/Metal_Cluster.wav
  Excitations/Snare_Poudre.wav
  Excitations/Souffle_Continu.wav
  Excitations/Verre_Aigu.wav
  Excitations/Voyelle_Synthetique.wav
  Excitations/Bois_Creux.wav
  Excitations/Corde_Pincee.wav
  Excitations/Kalimba_Douce.wav
  Excitations/Marimba_Fibre.wav
  Excitations/Corde_Frottee.wav
  Excitations/Bambou_Pulse.wav
  Excitations/Cloche_Felee.wav
  Excitations/Plaque_Vibrante.wav
  Excitations/Ressort_Metallique.wav
  Excitations/Bol_Chantant.wav
  Excitations/Carillon_Mobile.wav
  Excitations/Lamelle_Rouillee.wav
  Excitations/Papier_Froisse.wav
  Excitations/Sable_Roulant.wav
  Excitations/Brosse_Granuleuse.wav
  Excitations/Gouttes_Resonantes.wav
  Excitations/Souffle_Anche.wav
  Excitations/Crepitement_Doux.wav
  Excitations/FM_Evolutive.wav
  Excitations/Impulsions_Irisees.wav
  Excitations/Spectre_Mouvant.wav
  Excitations/Voyelle_Morphing.wav
  Excitations/Pixels_Fondants.wav
  Excitations/Nuage_Harmonique.wav
)
 add_library(ANKSFactoryAdapter STATIC src/FactorySources.cpp)
 set_target_properties(ANKSFactory ANKSFactoryAdapter PROPERTIES POSITION_INDEPENDENT_CODE ON)
 target_include_directories(ANKSFactoryAdapter PRIVATE src)
 target_link_libraries(ANKSFactoryAdapter PRIVATE ANKSFactory)
 juce_add_console_app(test_audio_io PRODUCT_NAME "ANKS Audio IO Tests")
 target_sources(test_audio_io PRIVATE tests/test_audio_io.cpp)
 target_include_directories(test_audio_io PRIVATE src)
 target_compile_definitions(test_audio_io PRIVATE JUCE_USE_CURL=0)
 target_link_libraries(test_audio_io PRIVATE juce::juce_audio_formats)
 add_test(NAME audio_io COMMAND test_audio_io)
 juce_add_console_app(test_plugin PRODUCT_NAME "ANKS Plugin Tests")
 target_sources(test_plugin PRIVATE tests/test_plugin.cpp plugin/PluginProcessor.cpp)
 target_include_directories(test_plugin PRIVATE src plugin)
 target_compile_definitions(test_plugin PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
 target_link_libraries(test_plugin PRIVATE juce::juce_audio_utils ANKSFactoryAdapter)
 add_test(NAME plugin COMMAND test_plugin)
 juce_add_plugin(Modular COMPANY_NAME "ANKS" IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE NEEDS_MIDI_OUTPUT FALSE IS_MIDI_EFFECT FALSE
  PLUGIN_MANUFACTURER_CODE Anks PLUGIN_CODE Arlc FORMATS VST3 Standalone
  PRODUCT_NAME "Modular" COPY_PLUGIN_AFTER_BUILD FALSE)
 target_sources(Modular PRIVATE plugin/PluginProcessor.cpp plugin/PluginProcessor.h src/Sequencer.h)
 target_include_directories(Modular PRIVATE src)
 target_compile_definitions(Modular PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
 target_link_libraries(Modular PRIVATE juce::juce_audio_utils ANKSFactoryAdapter PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
endif()
