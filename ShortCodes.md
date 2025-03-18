# Short Codes for spnsim
### NOP
```
has no short codes
```

### CLR
```
cr -> CLR
cR -> CLR + "\n"
```

### LDAX
```
lx -> LDAX	
lX -> LDAX	REG
lL -> LDAX	ADCL
lR -> LDAX	ADCR
l0 -> LDAX	POT0
l1 -> LDAX	POT1
l2 -> LDAX	POT2
```

### RDAX
```
rx -> RDAX	
rX -> RDAX	REG, 1.0
rL -> RDAX	ADCL, 1.0
rR -> RDAX	ADCR, 1.0
rC -> RDAX	ADCL, 0.5 + "\n" + 
      RDAX	ADCR, 0.5
r0 -> RDAX	POT0, 1.0
r1 -> RDAX	POT1, 1.0
r2 -> RDAX	POT2, 1.0
```

### WRAX
```
wx -> WRAX	
wX -> WRAX	REG, 0.0
wL -> WRAX	ADCL, 0.0
wR -> WRAX	ADCR, 0.0
wC -> WRAX	ADCL, 1.0 + "\n" + 
      WRAX	ADCR, 0.0
```

### RDFX
```
fx -> RDFX	
fX -> RDFX	RDFX	REG, 1.0
```

### WRHX
```
fh -> WRHX	
fH -> WRHX	REG, -1.0
```

### WRLX
```
fl -> WRLX	
fL -> WRLX	REG, -1.0
```

### MAXX
```
ma -> MAXX	
mA -> MAXX	REG, 1.0
```

### MULX
```
mu -> MULX	
mL -> MULX	DACL
mR -> MULX	DACR
m0 -> MULX	POT0
m1 -> MULX	POT1
m2 -> MULX	POT2
```

### SOF
```
sf -> SOF	
sF -> SOF	1.0, 0.0
s0 -> SOF	0.0, 0.0
s5 -> SOF	0.5, 0.5
s2 -> SOF	-2.0, 0.0
s" -> SOF	-2.0, 0.0 + "\n" + 
      SOF	-2.0, 0.0
```

### LOG
```
lg -> LOG	
lG -> LOG	1.0, 0.0
```

### EXP
```
ep -> EXP	
eP -> EXP	1.0, 0.0
```

### ABSA
```
aa -> ABSA	
aA -> ABSA + "\n"
```

### AND
```
ad -> AND	
aD -> AND	%00000000_00000000_00000000
a0 -> AND	%00000000_00000000_00000000
a1 -> AND	%11111111_11111111_11111111
a6 -> AND	$000000
aF -> AND	$FFFFFF
```

### OR
```
or -> OR	
oR -> AND	%00000000_00000000_00000000
o0 -> AND	%00000000_00000000_00000000
o1 -> AND	%11111111_11111111_11111111
o6 -> AND	$000000
oF -> AND	$FFFFFF
```

### XOR
```
xr -> OR	
xR -> AND	%00000000_00000000_00000000
x0 -> AND	%00000000_00000000_00000000
x1 -> AND	%11111111_11111111_11111111
x6 -> AND	$000000
xF -> AND	$FFFFFF
```

### NOT
```
nt -> NOT
nT -> NOT + "\n"
```

### SKP
```
sp -> SKP	
sN -> SKP	NEG, 1
sG -> SKP	GEZ, 1
sZ -> SKP	ZRO, 1
sC -> SKP	ZRC, 1
sR -> SKP	RUN, 1
```

### RDA
```
ra -> RDA	
rA -> RDA	addr, 1.0
r5 -> RDA	addr, -0.5
r% -> RDA	addr, 0.5
```

### WRA
```
wa -> WRA	
wA -> WRA	addr, 0.0
```

### RMPA
```
rp -> RMPA	
rP -> RMPA	1.0
```

### WRAP
```
wp -> WRAP	
wP -> WRAP	addr, 0.5
w5 -> WRAP	addr, 0.5
w% -> WRAP	addr, -0.5
```

### WLDS
```
ds -> WLDS	
dS -> WLDS	SIN0, 512, 32767
```

### WLDR
```
dr -> WLDR	
dR -> WLDR	RMP0, 32767, 4096
```

### JAM
```
jm -> JAM	
jM -> JAM + "\n"
```

### CHO
```
co -> CHO	
cO -> CHO	RDAL, SIN0

cL -> CHO	RDAL, SIN0

cf -> CHO	SOF, RMP0, NA | COMPC, 0.0
cF -> CHO	SOF, RMP0, NA | COMPC, 0.0 + "\n" + 
      CHO	RDA, RMP0, NA, addr

cA -> CHO	RDA, SIN0, SIN | REG | COMPC, addr
cs -> CHO	RDA, SIN0, SIN | REG | COMPC, addr + "\n" + 
      CHO	RDA, SIN0, SIN, addr+1
cS -> CHO	RDA, SIN0, SIN | REG | COMPA, addr + "\n" + 
      CHO	RDA, SIN0, SIN | COMPC | COMPA, addr+1
cc -> CHO	RDA, SIN0, COS | REG | COMPC, addr + "\n" + 
      CHO	RDA, SIN0, COS, addr+1
cC -> CHO	RDA, SIN0, COS | REG | COMPA, addr + "\n" + 
      CHO	RDA, SIN0, COS | COMPC | COMPA, addr+1

cp -> CHO	RDA, RMP0, REG | COMPC, addr
      CHO	RDA, RMP0, 0, addr+1
cP -> CHO	RDA, RMP0, COMPC | RPTR2, addr
      CHO	RDA, RMP0, RPTR2, addr+1
```
