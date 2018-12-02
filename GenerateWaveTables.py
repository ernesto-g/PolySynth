import wave,struct

def readWavFile(filename,maxOut,nSamplesMaxC3):
	wav_r = wave.open(filename, 'r')
	print("File SAMPLES:"+str(wav_r.getnframes()))
	
	ret = []
	while wav_r.tell() < wav_r.getnframes():
		decoded = struct.unpack("<h", wav_r.readframes(1))
		ret.append(decoded)
	
	r = ret
	max=-99999999
	min=99999999
	for sample in r:
		sample = sample[0]
		if sample>max:
			max=sample
		if sample<min:
			min=sample
			
	max = max - min

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
		return None
		
	return out
	
def samplesFunction(samples,index,N,waveFormMax):
	return samples[  int((index*len(samples))/N) ]

	
#INPUT FILE INFO (C3 sample)
inputFilePeriods = 1	#Amount of periods inside input file
inputFileFs = 96000		#Sample rate of input file
#inputFileName = "C3_MELLOTRON_MA300_96Khz.wav"
#inputFileName = "C3_MELLOTRON_MB300_96Khz.wav"
#inputFileName = "C3_MELLOTRON_CELLO_96Khz.wav"
#inputFileName = "C3_CASIOMT600_96Khz.wav"
inputFileName = "C3_KORG_M3R_96Khz.wav"
#_______________________________________________________________________________________________

#OUTPUT FILE INFO
fs = 64000.0 			#Output sample rate 64Khz
waveFormMax = 572*16	#Output max unsigned level
#waveFormName = "MELLO_M300A"
#waveFormName = "MELLO_M300B"
#waveFormName = "MELLO_CELLO"
#waveFormName = "CASIO_MT600"
waveFormName = "KORG_M3R"
#_____________________________________________________________________________________________


#START SCRIPT

nSamplesMaxC3 = int(round(inputFileFs/130.813)*inputFilePeriods) #C3:130.813Hz
print("Samples required at "+str(inputFileFs)+"Hz for C3: "+str(nSamplesMaxC3)+" ("+str(inputFilePeriods)+" cycle[s])")

samples = readWavFile(inputFileName,waveFormMax,nSamplesMaxC3)
if samples==None:
	exit()
	
sizes = ""
sawTable = ""	
notesFreqs = {"C":32.7032,"Cs":34.6479,"D":36.7081,"Ds":38.8909,"E":41.2035,"F":43.6536,"Fs":46.2493,"G":48.9995,"Gs":51.9130,"A":55.0000,"As":58.2705,"B":61.7354}
for noteName, freq in notesFreqs.iteritems():

	N = fs / freq
	N = round(N,0)
	N = int(N)
	N = N * inputFilePeriods

	#SAW
	lenName = waveFormName+"_TABLE_"+noteName+"_LEN"
	sizes+= "#define "+lenName+" "+str(N)+"\n"
	sawTable+="volatile const unsigned short "+waveFormName+"_TABLE_"+noteName+"["+lenName+"] PROGMEM ={"
	for i in range(0,N):
		value = samplesFunction(samples,i,N,waveFormMax)
		value = round(value,0)
		value = int(value)
		sawTable+=str(value)
		if i!=(N-1):
			sawTable+=","	
	sawTable+="};\n"	

f = open("tables.c", "w")	
f.write(sizes)
f.write(sawTable)
f.write("\n")
f.close()
print("File generated")