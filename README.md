<h1 align="center">CoD-Black Ops rtx-remix compatibility mod</h1>

<div align="center" markdown="1"> 

This client modification is made to make the game compatible with nvidia's [rtx-remix](https://github.com/NVIDIAGameWorks/rtx-remix).  
How? By manually reimplementing fixed function rendering :) 

It does __not__ come with a 'rtx mod' -> meaning no custom models nor materials.  

<br>

</div>

<div align="center" markdown="1">
	
![img](img/01.jpg)
	
only compatible with sp (including [game_mod](https://github.com/Nukem9/LinkerMod))

</div>

# Usage

#### A. Easy way but might not feature the latest and greatest:
  1) Download the latest [release](https://github.com/xoxor4d/t5-rtx/releases) and extract the contents into your CoD-BO1 root directory.  
  
  2) Download the latest release of [asi-loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases) (non-x64)  
  and extract the dll that comes with it (probably `dinput8.dll`) into your CoD-BO1 root directory. 
  
  3) Rename the extracted dll file (probably `dinput8.dll`) to `DSOUND.dll`.

  4) Start the game. The in-game console or the game window itself should show `t5-rtx-version >` if the asi was loaded correctly

  <br>

  5) Read the `Dvars / Console Commands` and `Current issues` sections

<br>

#### B. Manual way but always up-to-date:

1) Install [asi-loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases) (__A. Step 2__)

1) Install the latest full rtx-remix release (0.4.0 at this time)   
https://github.com/NVIDIAGameWorks/rtx-remix/tags

<br>

3) (Optional) Install the latest `github action` builds of:  
remix bridge - https://github.com/NVIDIAGameWorks/bridge-remix/actions  
remix runtime - https://github.com/NVIDIAGameWorks/dxvk-remix/actions  

<br>

4) Download the latest t5-rtx `github actions` build:  
  Release-binaries-t5-rtx - https://github.com/xoxor4d/t5-rtx/actions 
  - Drop `t5rtx.asi` and the contents of the `assets` folder into your CoD-BO1 root directory and start the game.  
  The in-game console or the game window itself should show `t5-rtx-version >` if the asi was loaded correctly.

<br>

5) Read the `Dvars / Console Commands` and `Current issues` sections

<br>
<br>

## ⚠️ Current issues:
- changing resolution or anything else that requires the game to restart the renderer WILL currently crash the game
- the main menu will be darker after a map was loaded (reason unkown)

## Compiling yourself:
1. setup `CODBO1_ROOT` environment variable with path to your CoD-BO1 directory (optional)
2. run `generate-buildfiles_vs22.bat` to generate build files (build folder)
3. compile and copy `t5rtx.asi` to your CoD-BO1 root folder  
   - ^ automatically happens if you setup `CODBO1_ROOT`

<br>

## Questions? 
- join the [rtx-remix showcase](https://discord.gg/j6sh7JD3v9) discord and check out the CoD Black Ops thread within the `remix-projects` channel.
- join the [iw3xo](https://discord.gg/t5jRGbj) discord if you have questions related to this modification
- rtx-remix: https://github.com/NVIDIAGameWorks/rtx-remix  


<br>

## Credits
- [people of the showcase discord](https://discord.gg/j6sh7JD3v9) (for testing, feedback and ideas)
- [Nukem - LinkerMod](https://github.com/Nukem9/LinkerMod)
- [nvidia - rtx-remix](https://github.com/NVIDIAGameWorks/rtx-remix)