add_executable(btop
        src/main.cpp
        src/MainWindow.cpp

        src/Controls/BoxesGrid.cpp
        src/Controls/CoreGrid.cpp
        src/Controls/CpuWidget.cpp
        src/Controls/DiskList.cpp
        src/Controls/Graphing.cpp
        src/Controls/MemWidget.cpp
        src/Controls/NetWidget.cpp
        src/Controls/ProcWidget.cpp
        src/Controls/SearchBox.cpp
        src/Controls/StatRows.cpp

        src/Services/Collector.cpp
        src/Services/CpuService.cpp
        src/Services/DiskService.cpp
        src/Services/MemService.cpp
        src/Services/NetService.cpp
        src/Services/PowerService.cpp
        src/Services/ProcService.cpp
        src/Services/SnapshotStore.cpp
)

target_include_directories(btop PRIVATE
    src
)

target_compile_definitions(btop PRIVATE
    _WIN32_WINNT=0x0A00
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

target_link_libraries(btop PRIVATE
    terminality
    iphlpapi
    psapi
    ws2_32
)
