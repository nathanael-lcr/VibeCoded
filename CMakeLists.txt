cmake_minimum_required(VERSION 3.26)
project(MonProjet C)

set(CMAKE_C_STANDARD 23)

# Spécifier où se trouve SDL3
set(SDL3_DIR "C:/Users/lecro/Documents/SDL3/cmake")

# SDL3_ttf
set(SDL3_ttf_DIR "C:/SDL3_ttf/cmake")
find_package(SDL3_ttf REQUIRED CONFIG REQUIRED COMPONENTS SDL3_ttf-shared)

# Trouver SDL3
find_package(SDL3 REQUIRED CONFIG)
include_directories(include)



# Créer l'exécutable
add_executable(MonProjet src/main.c
        src/audio.c
        src/waveform.c
        src/synth.c
        src/textarea.c
        src/input.c
        include/audio.h
        include/input.h
        include/synth.h
        include/waveform.h
        include/textarea.h
        include/oscillator.h
        src/oscillator.c
        include/command_parser.h
        src/command_parser.c
        include/lfo.h
        src/lfo.c
        include/project.h
        include/filter.h
        src/filter.c)

# Lier SDL3
target_link_libraries(MonProjet PRIVATE SDL3::SDL3 SDL3_ttf::SDL3_ttf-shared)

# Copier SDL3.dll à côté de l'exécutable
add_custom_command(TARGET MonProjet POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3-shared>
        $<TARGET_FILE:SDL3_ttf::SDL3_ttf-shared>
        $<TARGET_FILE_DIR:MonProjet>
)