#definitions
fs = 64000.0 #Sample rate 64Khz
waveFormMax = 572*16
waveFormName = "SAW"
#______________


notesFreqs = {"C":32.7032,"Cs":34.6479,"D":36.7081,"Ds":38.8909,"E":41.2035,"F":43.6536,"Fs":46.2493,"G":48.9995,"Gs":51.9130,"A":55.0000,"As":58.2705,"B":61.7354}

f = open("tables.c", "w")
f.close()

f = open("tables.c", "a")
sizes = ""
sawTable = ""	
for noteName, freq in notesFreqs.iteritems():

	N = fs / freq
	N = round(N,0)
	N = int(N)

	#SAW
	lenName = waveFormName+"_TABLE_"+noteName+"_LEN"
	sizes+= "#define "+lenName+" "+str(N)+"\n"
	sawTable+="volatile const unsigned short "+waveFormName+"_TABLE_"+noteName+"["+lenName+"] PROGMEM ={"
	for i in range(0,N):
		value = (waveFormMax*i)/N
		value = round(value,0)
		value = int(value)
		sawTable+=str(value)
		if i!=(N-1):
			sawTable+=","	
	sawTable+="};\n"	

	
f.write(sizes)
f.write(sawTable)
f.write("\n")
f.close()