
Since V.0.8.0 ScummVM requires the SKY.CPT file for running Beneath a Steel Sky.
This file contains data that was originally included in the SKY.EXE and was also included
in the ScummVM executable before 0.8.0.
Including all this data in the executable has the disadvantage that it wastes memory.
Even when playing a completely different game (like, e.g. Day of the Tentacle) ScummVM would
have the Beneath a Steel Sky data lying around in memory. This doesn't make much of a difference
on a 1GB RAM windows system, but for platforms like PALM and PocketPC it's a real problem,
so this data was moved into the SKY.CPT file and is loaded at runtime when BASS is started.

This program creates the SKY.CPT file.

If you want to create your own file, *PLEASE* simply forget about it.
It's a long and annoying procedure and won't help you with anything.
This program was only included in ScummVM's source tree because the Debian license
forces us to.
Instead download the file from http://www.scummvm.org/SKY.CPT
Also, please be aware that if you create your own CPT file (if it isn't exactly the same as the
one we offer for download at www.scummvm.org), it will be incompatible and the savegames produced
using the file will be incompatible with ScummVM using the normal CPT file.
The incompatibility will not be detected by ScummVM, it will most probably simply crash.

If you still want to waste your time by creating this file:
1) compile the tool
2) run it once, it'll complain that it's missing 7 files, the RESET.* files but
	it creates an incomplete COMPACT.DBG file anyways, which you rename to SKY.CPT.
3) download the 7 different Beneath a Steel Sky versions:
	v.0.00288: The first floppy version that was released
	v.0.00303: Another floppy version
	v.0.00331: Another floppy version
	v.0.00348: Another floppy version
	v.0.00365: CD Demo
	v.0.00368: Full CD version
	v.0.00372: Final CD version
4) Change scummvm/sky/logic.cpp, function fnSkipIntroCode
	so that it calls _skyControl->doLoadSavePanel()
5) Compile the patched ScummVM.
6) Start each of the BASS versions, enjoy the intro over and over again.
	Afterwards, it'll automatically show the load/save dialog where you save the game.
7) Rename the Savegame files you created to "RESET.*", depending on the version.
	e.g. RESET.288 for v.0.00288
8) Copy the files into the skycpt tool directory, execute the tool again.
	It'll create a new COMPACT.DBG.
9) Rename this file to SKY.CPT.
10) DELETE IT BECAUSE IT'S PROBABLY BROKEN, NOT WORTH BOTHERING WITH ANYWAYS
	AND DOWNLOAD THE SKY.CPT FILE FROM THE URL ABOVE!!
	
Oh, I almost forgot.
The program only works on little endian systems and probably isn't alignment safe either.
It may also leak memory or accidentially reformat your harddisk. Who knows.
You've been warned.
