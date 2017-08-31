CS120B-Summer-2017

Video Link
https://www.youtube.com/watch?v=h71nCW1uf6Y

Pin Diagram:
http://imgur.com/a/mZDki

Taylor Che
Tche001@ucr.edu
Custom Lab
Project: JukeBox
Objective:
The goal of our project is to utilize the ATMEGA1284 to build a jukebox. The player can pause or skip
through the stored songs loaded into the jukebox. The songs will be accessible by cycling through a list of
songs displayed on the LCD screen with buttons affixed to the breadboard.


Component Specifications:

• ATMEGA 1284

• AVR Studio 7

Functionality Specifications:

Inputs: PA0, PA1, PA2, PA3, PA4

Outputs: LCD Screen, Speaker

• JukeBox with preloaded songs

o Play, Pause, Skip Song, Back Song, Stop Song


JukeBox Details:

• PA0 will function as a PLAY button and enables functionality of PA1 and PA2

• PA1 will function as a PAUSE button to pause the song from wherever it is currently playing.
    o Pressing PA1 again will STOP the song completely and return to the select menu

• PA2 will be a RESET which starts the song over from the beginning

• PA4 will iterate through the song list or skip the currently playing song to the beginning of the
next once pressed

• PA5 will reverse through the song list or skip the currently playing song to the beginning of the
previous song once pressed.

• Songs will be stored as frequency arrays which are accessed by the Pulse Width Modifier and
transmitted from PB6 to the speaker.
