# LodjurDS V0.1.4

<img align="right" width="220" src="./logo.png" alt="The Lynx logo"/>

An Atari Lynx emulator for Nintendo DS(i)/3DS.

## How to use

1. Create a folder named "lodjurds" in either the root of your flash card or in
 the data folder. This is where settings and save files end up.
2. Now put game/bios files into a folder where you have (Lynx) roms, max
 768 games per folder, filenames must not be longer than 127 chars. You can use
 zip-files (as long as they use the deflate compression).
3. Depending on your flashcart you might have to DLDI patch the emulator.

Note! You need a bios to be able to run games.

When the emulator starts, you can either press L+R or tap on the screen to open
up the menu.
Now you can use the cross or touchscreen to navigate the menus, A or double tap
to select an option, B or the top of the screen to go back a step.

To select between the tabs use R & L or the touchscreen.

## Menu

### File

* Load Game: Select a game to load.
* Load State: Load a previously saved state of the currently running game.
* Save State: Save a state of the currently running game.
* Save Settings: Save the current settings.
* Reset Console: Reset the console.
* Quit Emulator: (If supported.)

### Options

* Controller:
  * B Autofire: Select if you want autofire on button B.
  * A Autofire: Select if you want autofire on button A.
  * Swap A-B: Swap which NDS button is mapped to which Lynx button.
* Display:
  * Screen: Select rotation & zoom.
  * Gamma: Lets you change the gamma ("brightness").
  * Contrast: Lets you change the contrast.
  * Border: Change between console image and black.
* Machine:
  * Machine: Select the emulated machine.
  * Cpu Speed Hacks: Allow speed hacks.
  * Select Bios: Load a Lynx Bios, needed.
  * Sound: Enable/disable sound.
* Settings:
  * Speed: Switch between speed modes.
    * Normal: Game runs at it's normal speed.
    * 200%: Game runs at double speed.
    * Max: Games can run up to 4 times normal speed (might change).
    * 50%: Game runs at half speed.
  * Allow Refresh Change: Allow the Lynx to change NDS refresh rate.
  * Autoload State: Toggle Savestate autoloading. Automagically load the savestate associated with the current game.
  * Autosave Settings: This will save settings when leaving menu if any changes are made.
  * Autopause Game: Toggle if the game should pause when opening the menu.
  * Powersave 2nd Screen: If graphics/light should be turned off for the GUI screen when menu is not active.
  * Emulator on Bottom: Select if top or bottom screen should be used for emulator, when menu is active emulator screen is allways on top.
  * Autosleep: Doesn't work.
* Debug:
  * Debug Output: Show FPS and logged text.
  * Step Frame: Emulate one frame.

### About

Some dumb info about the game and emulator...

## Controls

* NDS B & A buttons are mapped to Lynx B & A.
* NDS X is mapped to Lynx Option I.
* NDS Y is mapped to Lynx Option II.
* NDS Start is mapped to Lynx Pause.
* NDS d-pad is mapped to Lynx d-pad.

## Games

* Baseball Heroes. Doesn't start'
* Gates of zendocon. Hangs if intro is not skipped.

## Credits

```text
Thanks to:
Keith Wilkins for Handy source and info.
42Bastian for bug reports, test files & bll.enc.
```

Fredrik Ahlström

<https://bsky.app/profile/therealflubba.bsky.social>

<https://www.github.com/FluBBaOfWard>

X/Twitter @TheRealFluBBa
