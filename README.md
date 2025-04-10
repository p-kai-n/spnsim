# spnsim
A simulator of ".spn" files for FV-1.<br>
![0_tremolo](img/0_tremolo.png)<br>
![1_chorus](img/1_chorus.png)<br>
![2_pots](img/2_pots.png)<br>
![3_font](img/3_font.png)<br><br>

## Features(Simulator)
### In/Out Gain
Avoid clipping in spn instructions.<br><br>

### Bypass/Mute Button
The Bypass Button works for compare to the wet and dry sound.<br>
The Mute Button stops sound.<br><br>

### Waveform Viewers
The Waveform Viewers display the register value ​​and allows you to select which registers to display.<br><br>

### 3+1 POTs
The values ​​of POT0-2 are automatically transferred to the registers at before calculation of a sample.<br>
The POT3 has 2 functions and allows you to select.<br>
"Dry Mix Mode" ... The POT3 adjusts the dry signal level. POT0-2 can be assigned other functions.<br>
"Register Assignment Mode" ... The values ​​of POT3 is transferred to the REG0-31 like POT0-2.<br><br>

### 2/3 Resampling
Decimate the samples to 2/3 and interpolate samples after spn processing.<br>
This function works for simulate 32kHz samplerate on 48kHz systems. The cutoff frequency and delay time behave like 32768Hz.<br><br>


## Features(Editor)
### Syntax Highlighting
The Memoly Declarations, Memoly Numbers, REG and CONST Equatements, Registers, Flags, LFOs, SKP lines and labels, and Comments, are highlighted.<br><br>

### Short Codes
Typing the 2-caracter at lowercase in line start, it makes the full instructions (and typical arguments).<br>
[Learn more...](ShortCodes.md)<br><br>

### Font Familly & Size & Line Height Customize<br>
The code editor is highly customizable.<br><br>

### Dark UI
Switchable to the dark UI.<br><br>
