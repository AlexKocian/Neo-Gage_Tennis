# Neo-Gage Tennis
A Symbian S60v1 'Pong' clone. This project is not affiliated with Nokia or Atari :)

# Supported devices
This game should work on all Symbian Series 60 v1.x and 2.x devices, although I've only tested on Nokia N-Gage (NEM-4). If you have other devices, please let me know whether everything functions normally.

# Installation instructions
Go to https://github.com/AlexKocian/Neo-Gage_Tennis/releases and download.
.sis files can be installed by sending them to the device over Bluetooth, installing through the Nokia PC Connectivity Suite, through WAP (save me lord) or by executing the installer via a file explorer (e.g. FExlpore or X-Plore).<br>

For .zip releases, copy and replace the 'System' folder onto the target drive (likely E: in the case of an MMC).<br>

This game is small enough (unpacked size: 62kB) that it should be safe to install directly to the C: drive.<br>
IMPORTANT: make sure you have enough free space on your device! At least a few hundred kilobytes (best practise is at least .5 megabytes).

# Compilation instructions
Watch this great guide for the setup: <a href="https://www.youtube.com/watch?v=CYOjrQPTV1Q" >
How-to: Symbian C++ programming (Nokia 9210 Communicator)</a><br>
Just get the Symbiand Series 60 SDK v1.2 instead, because the communicator uses Series 80<br>
The short version is this: WinXP or older (32 bit), Symbian S60v1.2 SDK, ActivePerl 5.6, Microsoft Visual C++ 6.0 and JRE 2.

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
When suspending the game and then bringing it back to the foreground, the sound may skip forward a few ms (I haven't noticed this myself, but it should happen). This is because the queue of sound data chunks to be played by the sound server are flushed when stopping playback (done when suspending the game in order for other applications - or the system itself - to be able to use sound). These chunks never end up being played as playback picks up after the last chunk loaded into the queue. The fix for this would probably require some insane Symbian wizardry on my part as the location and size of the queue are abstracted by the API and therefore practically inaccessible. I have to say, this operating system is very cool (no sarcasm)! I'm having fun with it :D

# Usage
You're free to use this for any project (open-source or otherwise, just please no AI garbo)

# Resources
Thanks to EMCCSoft (defunct) for writing the book "Developing Series 60 Applications: A Guide for Symbian OS C++ Developers", ISBN: 0-321-22722-0. This is a great resource that covers all of the basics of Symbian S60 C++ programming, including some common issues, tips and S60v1/S60v2 differences. They also wrote a bunch of example applications, from which I re-wrote some of the basic UI application functionality.

I used Cool Edit Pro 2.1 for sound and GraphicsGale Free Edition for bitmaps/sprites. The only use of AI was for debugging (screw Symbian debugging!)
