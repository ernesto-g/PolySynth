import wave,struct


	
def sawFunction(i,N,waveFormMax):
	return (waveFormMax*i)/N

def readWavFile(filename,maxOut,nSamplesMaxC3):
	wav_r = wave.open(filename, 'r')
	print("File SAMPLES:"+str(wav_r.getnframes()))
	
	ret = []
	while wav_r.tell() < wav_r.getnframes():
		decoded = struct.unpack("<h", wav_r.readframes(1))
		ret.append(decoded)
	
	r = ret
	#print("cantidad:"+str(len(r)))
	max=-99999999
	min=99999999
	for sample in r:
		sample = sample[0]
		if sample>max:
			max=sample
		if sample<min:
			min=sample
			
	#print("min:"+str(min))
	max = max - min
	#print("max:"+str(max))

	out=[]
	n=0
	for sample in r:
		sample = sample[0]
		sample-=min
		out.append(int(((sample*maxOut)/max)))
		n+=1
		if n>=nSamplesMaxC3:
			break
			
	print("Out SAMPLES:"+str(len(out)))
	if len(out)<nSamplesMaxC3:
		print("WARNING: File information does not contains enough samples for C3 frequency!")
	return out
	
	
	
#definitions
inputFileFs = 96000
nSamplesMaxC3 = round(inputFileFs/130.813)
print("Samples required at "+str(inputFileFs)+"Hz : "+str(nSamplesMaxC3))
notesFreqs = {"C":32.7032,"Cs":34.6479,"D":36.7081,"Ds":38.8909,"E":41.2035,"F":43.6536,"Fs":46.2493,"G":48.9995,"Gs":51.9130,"A":55.0000,"As":58.2705,"B":61.7354}

fs = 64000.0 #Output sample rate 64Khz
waveFormMax = 572*16
#MELLOTRON MA300
#waveFormName = "MELLO_M300A"
#samples = readWavFile("C3_MELLOTRON_MA300_96Khz.wav",waveFormMax,nSamplesMaxC3)

#MELLOTRON MB300
#waveFormName = "MELLO_M300B"
#samples = readWavFile("C3_MELLOTRON_MB300_96Khz.wav",waveFormMax,nSamplesMaxC3)

#MELLOTRON CELLO
#waveFormName = "MELLO_CELLO"
#samples = readWavFile("C3_MELLOTRON_CELLO_96Khz.wav",waveFormMax,nSamplesMaxC3)

#CASIO MT600
#waveFormName = "CASIO_MT600"
#samples = readWavFile("C3_CASIOMT600_96Khz.wav",waveFormMax,nSamplesMaxC3)

#KORG M3R
waveFormName = "KORG_M3R"
samples = readWavFile("C3_KORG_M3R_96Khz.wav",waveFormMax,nSamplesMaxC3)





#waveFunction = sawFunction

def samplesFunction(index,N,waveFormMax):
	return samples[  int((index*len(samples))/N) ]

waveFunction = samplesFunction
#______________





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
		value = waveFunction(i,N,waveFormMax) #(waveFormMax*i)/N
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