# Jiangshi 2

Experimental Windows recompilation and English-presentation project for
Kyonshiizu 2, using NESRecomp.

## Project status

The project includes English dialogue, location and item catalogs, together
with UI presentation patches. Translation and gameplay testing are ongoing.
Some text and UI details remain unfinished.

The maintained source and catalogs already contain the installed changes.
Historical one-time translation installers are not required.

## Requirements

- Windows
- CMake 3.20 or newer
- A compatible MinGW-w64 GCC/G++ toolchain and mingw32-make on PATH
- A compatible NESRecomp checkout and its built NESRecomp.exe
- A user-supplied copy of the supported game ROM

Tested NESRecomp dependency:

- Repository: https://github.com/mstan/nesrecomp
- Commit: fc0bf3fa46a710464960c0a1f2d3b83b4d345600
- Local checkout reported no changes during the initial build verification.
- The engine's license is preserved in docs/NESRecomp-LICENSE.txt.
- Engine source and binaries are not bundled in this repository.

## Supported ROM

The ROM is not included.

Place your supported ROM at:

    roms/game.nes

Required full-file SHA-256:

    4fbf02411b66f8e4cdff16219612fc8aa804ec12f2e2ce19235afbccdaeaf4a2

Other revisions, modified ROMs and alternate headers are not currently verified.
CMake checks the ROM hash during configuration.

## Build

From the project root, in a PowerShell session with the MinGW tools on PATH:

    cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DNESRECOMP_ROOT="C:/path/to/nesrecomp"
    cmake --build build --parallel 2

By default, the recompiler is expected at:

    <NESRECOMP_ROOT>/build/recompiler/NESRecomp.exe

If necessary, supply its location during configuration with:

    -DNESRECOMP_HOST_COMPILER="C:/path/to/NESRecomp.exe"

## Run

From PowerShell:

    .\game.bat

The launcher sets the working directory to the project root.
The current launcher expects the ROM at roms/game.nes.

## Translation catalogs

- dialogue_translations.tsv
- item_translations.tsv
- location_translations.tsv

Keep these files beside game.bat. Restart the game after catalog changes.

## Repository contents

The original ROM, build outputs, generated game code, local saves, diagnostic
captures and development backups are excluded from version control.

## Rights and licensing

This is an unofficial project. Original game rights belong to their respective
owners. Users must supply their own supported ROM.

NESRecomp is a separate dependency with its own licensing terms.
A project-wide license has not yet been selected; licensing review is pending.
