# btop (Terminality clone)

A Windows re-implementation of [btop](https://github.com/aristocratos/btop) built with the
Terminality TUI framework. The original btop 1.4.7 sources are in `../btop-1.4.7/` and were
used as the reference for the layout, keybindings, graph rendering, and the Default theme.

## What is implemented

| btop feature | Status |
|---|---|
| CPU box: total + per-core usage, history graphs, frequency, uptime | ✔ |
| MEM box: used/available/cached/free + swap rows with sparklines | ✔ |
| Disks: per-volume usage meters (fixed disks) | ✔ |
| NET box: download/upload graphs, speed/total/top, interface cycling | ✔ |
| PROC box: sortable process table (pid, program, threads, user, mem, cpu%) | ✔ |
| Braille/block graph symbols, theme gradients, `floating_humanizer` | ✔ |
| Help / main menu / options / filter overlays | ✔ |
| Kill process (with confirmation) | ✔ |
| Box toggling (`1`–`4`), update timer, auto-scaling net graphs | ✔ |
| Battery indicator in the header | ✔ |
| Tree view / per-process details / GPU boxes / temperatures | ✘ (no portable Windows API used) |

## Keybindings

- `q` quit
- `m`/`esc` menu
- `h`/`?`/`F1` help
- `1`–`4` toggle boxes
- `+`/`-` update timer
- `b`/`n` net interface
- `z` zero net totals
- `a` net graph auto-scale
- `↑↓`/`PgUp/PgDn`/`Home/End` select process
- `←→` sort column
- `r` reverse
- `f` filter
- `u` pause
- `k` kill
- `shift+5` MEMB bytes/percent

## Data sources (Windows)

| btop (Linux) | This sample (Windows) |
|---|---|
| `/proc/stat` | `NtQuerySystemInformation(SystemProcessorPerformanceInformation)` |
| `/proc/meminfo` | `GlobalMemoryStatusEx` + `GetPerformanceInfo` |
| `/proc/[0-9]/*` | `CreateToolhelp32Snapshot` + `GetProcessTimes`/`GetProcessMemoryInfo` |
| `/sys/class/net/*` | `GetIfTable`/`GetIfEntry` (32-bit counters with wrap accumulation) |
| `statvfs` | `GetLogicalDriveStringsW` + `GetDiskFreeSpaceExW` |
| `/sys/class/power_supply` | `GetSystemPowerStatus` |
