
fs = 64000.0
waveFormMax = 255.0

C_f = 32.7032
Csh_f = 34.6479

D_f = 36.7081
Dsh_f = 38.8909



#C 
N = fs / C_f
N = round(N,0)
N = int(N)

#SAW
sawTable="const unsigned char SAW_TABLE_C["+str(N)+"] PROGMEM ={"
for i in range(0,N):
	value = (waveFormMax*i)/N
	value = round(value,0)
	value = int(value)
	sawTable+=str(value)
	if i!=(N-1):
		sawTable+=","
	
sawTable+="};"	

print(sawTable)