# Build the app which supports ALSA, using the HAL

include_directories(include)
file(GLOB MY_SOURCES "src/*.c")
add_executable(beat_box ${MY_SOURCES})

# Make use of the HAL library
target_link_libraries(beat_box LINK_PRIVATE gpiod)
target_link_libraries(beat_box LINK_PRIVATE hal)
target_link_libraries(beat_box LINK_PRIVATE lcd)
target_link_libraries(beat_box LINK_PRIVATE lgpio)

# ---------------- NEW! ----------------
# ALSA support
find_package(ALSA REQUIRED)
target_link_libraries(beat_box LINK_PRIVATE asound)

# PTHREAD support
find_package(Threads REQUIRED)
target_link_libraries(beat_box LINK_PRIVATE Threads::Threads)

# Copy executable to final location (change `beat_box` to project name as needed)
add_custom_command(TARGET beat_box POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:beat_box>"
     "~/cmpt433/public/myApps/beatbox" 
  COMMENT "Copying ARM executable to public NFS directory")


# ---------------- NEW! ----------------
# Copy the WAV folder to NFS
add_custom_command(TARGET beat_box POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy_directory
     "${CMAKE_SOURCE_DIR}/beatbox-wav-files"
     "~/cmpt433/public/myApps/beatbox-wave-files" 
  COMMENT "Copying WAVE files to public NFS directory")

#Copy the server folder to NFS
add_custom_command(TARGET beat_box POST_BUILD 
COMMAND "${CMAKE_COMMAND}" -E copy_directory
    "${CMAKE_SOURCE_DIR}/beatbox-server-copy"
    "~/cmpt433/public/myApps/beatbox-server-copy" 
COMMENT "Copying WAVE files to public NFS directory")

find_library(GPIOD_LIBRARY gpiod)       
target_link_libraries(beat_box LINK_PRIVATE gpiod)
