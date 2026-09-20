# Neo-Gage_Tennis
A Symbian S60v1 'Pong' clone

# Supported devices
This game should work on all Symbian Series 60 v1.x and 2.x devices, although I've only tested on Nokia N-Gage (NEM-4). If you have other devices, please let me know whether everything functions normally.

# Installation instructions
Go to https://github.com/AlexKocian/Neo-Gage_Tennis/releases and download.
.sis files can be installed by sending them to the device over Bluetooth, installing through the Nokia PC Connectivity Suite, through WAP (save me lord) or by executing the installer via a file explorer (e.g. FExlpore or X-Plore).

For .zip releases, copy and replace the 'System' folder onto the target drive (likely E: in the case of an MMC).

This game is small enough (unpacked size: 62kB) that it should be safe to install directly to the C: drive.
IMPORTANT: make sure you have enough free space on your device! At least a few hundred kilobytes (best practise is at least .5 megabytes).

# Compilation instructions
TBA. I'm too tired right now and I feel like this will need an entire instructional video.
The short version is this: WinXP or older (32 bit), Symbian S60v1.2 SDK (or older, but why?), ActivePerl 5.6, Microsoft Visual C++ 6.0 and JRE 2.

# Saving
Saving options is done automatically when leaving the Options screen, high scores are saved when leaving the game to go back to the main menu (recorded as "USR" on the scoreboard).

# Controls
Command button area (stuff like Options, Exit and Back) - left/right softkey<br>
Accept - center of D-Pad/'Tick' button on QD/'5'<br>
Menu control - D-Pad<br>
Game controls - left on the D-Pad/'4' to move left, right on the D-Pad/'6' to move right, right softkey pauses

# Screens
<img width="176" height="208" alt="NGTS1" src="https://github.com/user-attachments/assets/1e4d33cd-ad96-4821-a53e-1803eaf2f28e" />
<img width="176" height="208" alt="NGTS4" src="https://github.com/user-attachments/assets/f9b8bcab-a247-40d6-bb8f-03b29e2c2805" />
<img width="176" height="208" alt="NGTS3" src="https://github.com/user-attachments/assets/19a26ddc-0e0f-4faa-8251-b00b74fab23f" />
<img width="176" height="208" alt="NGTS2" src="https://github.com/user-attachments/assets/8b37214d-c76b-4920-8bd4-b0dc8c174004" />

# Known bugs
When suspending game, no other sounds play, because it's hogging the sound server (whoops!)

# Usage
You're free to use this for any project (open-source or otherwise, just please no AI garbo)

# Resources
Thanks to EMCCSoft (defunct) for writing the book "Developing Series 60 Applications: A Guide for Symbian OS C++ Developers", ISBN: 0-321-22722-0. This is a great resource that covers all of the basics of Symbian S60 C++ programming, including some common issues, tips and S60v1/S60v2 differences. They also wrote a bunch of example applications, from which I re-wrote some of the basic UI application functionality.

I used Cool Edit Pro 2.1 for sound and GraphicsGale Free Edition for bitmaps/sprites. The only use of AI was for debugging (screw Symbian debugging!)
