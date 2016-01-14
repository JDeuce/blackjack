--------------------------------------------
WHAT IS THIS
--------------------------------------------
My final project for Real time systems.

Runs blackjack with the touchscreen on an AVR xplain

--------------------------------------------
BUILDING:
--------------------------------------------
I build using avr-gcc and make under cygwin on Windows 7.

The included makefile builds the system by simply running `make'.
The binary will be stored in the root of the project directory and is called 'project.hex'.

--------------------------------------------
RUNNING:
--------------------------------------------
Just flash the hex file onto the xplain board and go.


--------------------------------------------
INPUTS:
--------------------------------------------
There are buttons displayed on the LCD, you can use your finger or a stylus to activate them.
The only other input I use is SWITCH7, which can be pressed at any time to toggle the backlight level between a set of 5 predefined states.

--------------------------------------------
EEPROM:
--------------------------------------------
The game successfully detects invalid EEPROM state & initializes it on the fly.

The game stores the backlight level in eeprom.

The game additionally stores the game state (chip balance & current bet) in EEPROM after each hand.
This happens every 5 seconds when running the game, so I implemented basic wear leveling so it doesn't just hammer the same bytes over and over. By writing to the EEPROM in a logical sequential pattern, I can determine where the last successful write was on startup and continue from there. See game.c for more details.

--------------------------------------------
MODULE DESCRIPTIONS:
--------------------------------------------
Clock  	- sets up 32mhz clock rate, handles delays, and keeps a heartbeat
Button	- handles the buttons that are displayed on the lcd
Cards   - handles logic for cards & decks
Eeprom  - handles simple read & writes to eeprom
Events  - exposes some simple functions to disable non-timer interrupts
Font    - handles drawing fonts on the screen. characters are just glyphs so           the methods here just figure out which glyph to draw where
Game    - handles all of the game logic, including wear leveled state storage
Glyph   - handles drawing arbitrary bitmasks with given forecolor/backcolor
Hud	- handles drawing the current bet & balance during some game screens
LCD	- handles the meat of the lcd logic
Lcd_low - handles the more low-level hardware oriented aspects of the LCD
Leds	- handles simple toggling of the leds
Main	- bootstraps the game, handles logic to adjust backlight, 
	  and runs the main loop
Map 	- range_map implementation
Rand    - a linear congruential pseudorandom number generator
Screen  - exposes an interface for game screens to implement
	  screens have 4 methods: activate, deactivate, update and draw
Screen_bet - the betting screen
Screen_broke - the screen when you lose the game by running out of money
Screen_hand - the screen when actually playing a blackjack hand
Screen_start - the 'tap to start' screen
Suits   - handles drawing glyphs of the suit characters
Switches - maintains hysterisis state of the switches, and declares switch transition 		events through callbacks
Touch - the low level touch screen driver

--------------------------------------------
NOTES ON THE GENERATORS:
--------------------------------------------
I use these to generate the glyphs (fonts and suit sprites that are displayed on the screen. 
Basically just takes a black and white png and outputs a bitmask for it.

They're not fully automatic or anything, I just ran them and pasted the output into my c files.

