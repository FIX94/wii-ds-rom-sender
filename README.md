# wii-ds-rom-sender
This application will allow you to send NDS ROMs (and emulated NES/GB/GBC ROMs) via a Wii to a DS using Download Play.  
Please note that if your DS is not patched with FlashMe and you dont use HaxxStation then you can only send over Official Demo ROMs.  
To make use of HaxxStation, get the US version of "DS Download Station - Volume 1" renamed to "haxxstation.nds" on the root of your sd card.  
All you have to do is put your .nds/.srl/.nes/.gb/.gbc files into a "srl" folder on your sd card, 
get the current version of this program from the "releases" tab and start it via the homebrew channel.  
After it loaded up just select the file you want, download it on your DS and play it!  
If you happen to have trouble getting the file sent over then change the delay timing to a higher value.  
Note that .nes files are emulated using nesDS and .gb/.gbc files are emulated using GameYob.    

This Application uses/includes:  
Parts of the HaxxStation code from https://github.com/Gericom/dspatch  
GameYob Binary with 2MB Dummy ROM from https://github.com/FIX94/GameYob  
nesDS Binary with 2MB Dummy ROM from https://github.com/ApacheThunder/NesDS/tree/master/NesDS_Singles  
DOL Application compressed with dolxz from https://github.com/FIX94/dolxz  
NDS Files compressed with the LZO Library by Markus F.X.J. Oberhumer  