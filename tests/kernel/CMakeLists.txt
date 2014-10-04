include_directories(${FREETYPE_INCLUDE_DIRS} ${DIRECTFB_INCLUDE_DIRS})
link_directories(${FREETYPE_LIBRARY_DIRS} ${DIRECTFB_LIBRARY_DIRS})

add_executable(semaphore-locking-test semaphore-locking-test.c)
target_link_libraries(semaphore-locking-test root)

add_executable(semaphore-test semaphore-test.c)
target_link_libraries(semaphore-test root)

add_executable(thread-exit-test thread-exit-test.c)
target_link_libraries(thread-exit-test root)

add_executable(thread-suspend-test thread-suspend-test.cpp)
target_link_libraries(thread-suspend-test root be)

add_executable(thread-test thread-test.c)
target_link_libraries(thread-test root)

add_executable(time-test time-test.c)
target_link_libraries(time-test root)
