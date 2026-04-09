## c7

c7 is a mod for TEKKEN 7 that redesigns movesets for every character.

Features:

* Zero configuration. Just run alongside TEKKEN 7.
* Works online if both players use it and should work with most other mods that don’t modify movesets.
* Adds a tag to your display name to make coordinating with other players easier.
* Reports all online FT3 matches between players running the mod to [Wavu Wank7](https://wank7.wavu.wiki/latest)

For detailed list of moveset changes, see https://cthor.me/c7.

## Windows Defender Exclusion

Windows Defender will sometimes flag `c7.exe` as malware because it uses DLL injection to mod the game and network access to talk to the game server. When this happens, the DLL will remain injected and modified movesets will continue to load as expected. However, match reporting to the game server will stop, and the executable will be deleted (so you can't run it again). To avoid this, add an exclusion to Windows Defender for the c7 folder:

1. Press **Start**, type `Windows Security`, and open it.
2. Click **Virus & threat protection** in the left sidebar.
3. Under **Virus & threat protection settings**, click **Manage settings**.
4. Scroll down to **Exclusions** and click **Add or remove exclusions**. Accept the UAC prompt if one appears.
5. Click **Add an exclusion** → **Folder**.
6. Browse to the folder containing `c7.exe` and click **Select Folder**.

After this, Defender will leave anything in that folder alone. For this to apply to future updates of `c7.exe` as well, create a blank folder (e.g. `c7`) to exclude and always extract releases into that folder.

**If Defender has already removed `c7.exe`**, you have two options:

- **Restore it from quarantine.** In Windows Security → Virus & threat protection → **Protection history**, find the `Behavior:Win32/DefenseEvasion` entry for `c7.exe`, click **Actions**, and choose **Allow on device**. This both restores the file and whitelists that exact copy. Then add the folder exclusion above so it doesn't get removed again after the next update.
- **Re-download it.** Add the folder exclusion first (to an empty folder if you like), then extract or copy the new `c7.exe` into that folder. If you put it somewhere else first, Defender will delete it again before you can run it.

## Credits

This program is a hard fork of [kiloutre's TKMovesets](https://github.com/Kiloutre/TKMovesets) and would not have been possible without this work and the countless work of others in reverse engineering movesets, animation formats, and annotating moveset data structures.

## Building

This project uses CMake, so if you're new to this, your best way to get started is to install **Visual Studio Community**, and making sure to install the **C++CMake modules**. If you already have Visual Studio but without those modules, you can still install them.

### Dependencies

This project requires external libraries to be downloaded & built to function.

#### [vcpkg](https://vcpkg.io/en/getting-started.html)-obtainable dependencies

Some libraries can be quickly obtained by simply using vcpkg, a package manager that can be tied to Visual Studio and download & build vcpkg dependencies automatically for you.
This is the recommended way of building this project.
- Download vcpkg by running `git clone https://github.com/Microsoft/vcpkg.git`, or simply [downloading the .zip file](https://github.com/microsoft/vcpkg/archive/refs/heads/master.zip)
- Place the folder anywhere you want in your system, decide on a good place now because moving it later will break things.
- Run `bootstrap-vcpkg.bat` off the vcpkg folder
- Add the path of vcpkg to your PATH system environment variable
- Also create a system environment variable named `VCPKG_DEFAULT_TRIPLET` and set it to `x64-windows`
- **If you have visual studio installed**: As an admin, open a command prompt and execute `vcpkg integrate install`.
  - The command prompt will show you a `DCMAKE_TOOLCHAIN_FILE=xxx` value.
  - Take that `xxx` value and create a new system environment variable named `CMAKE_TOOLCHAIN_FILE` (yes, without the D at the start), and set it to that `xxx` value
- If you have visual studio running, restart it now.
- **If you do not have visual studio installed:** Go into the project folder and run `vcpkg install` to install the dependencies

vcpkg is now installed.

#### Submodules dependencies

Some dependencies have to be obtained from other git repos. To get them, make sure to also clone the submodules contained within this project.

If cloning with git from the command line, use this command to also download the required submodules:

`git clone --recurse-submodules https://github.com/Kiloutre/TKMovesets.git`

If you already have the repository cloned and want to bring the submodules over, execute those two commands inside of TKMovesets's repository:

`git submodule init ; git submodule update`

If downloading the project manually (.zip or such), you will also have to manually download the dependencies.
If using a GUI program to download this project, there will most likely be an option to bring the submodules over.

#### Steamworks dependency

The steamworks API's headers, which i do use in various parts of this software, does not have a license that would allow me to package it with my own code.
You may obtain the steamworks files using two possible ways:

- From the official steamworks website (that will require creating a steamworks account): https://partner.steamgames.com/downloads/list . Do make sure to download the version that came right before the Tekken build you'll be using the software on, for minimum problems.
- From this link which may go down at any time (v1.56) : https://mega.nz/folder/htpVHSCa#O24Zz0PfxcyejV5ChqDAgw

Make sure the folder is extracted at the root of the project (inside the TKMovesets folder), and that it is named `steamapi sdk`

### Building

- **If you have visual studio installed**: Make sure to close Visual Studio and delete the folders `.vs` and `out` before trying your first build, they may contain bad cache if you attempted building the project with an incomplete environment
- Open the project in visual studio, and if prompted to **Generate** the cmake config, say yes. It probably already started doing it on its own.
- In the list of targets at the top of Visual Studio (next to the green arrow icon), select `TKMovesets2.exe` to build the main tool
