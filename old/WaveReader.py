import wave,struct

def read_whole(filename,maxOut):
	wav_r = wave.open(filename, 'r')
	ret = []
	while wav_r.tell() < wav_r.getnframes():
		decoded = struct.unpack("<h", wav_r.readframes(1))
		ret.append(decoded)
	
	r = ret
	print("cantidad:"+str(len(r)))
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
	print("max:"+str(max))

	out=[]
	for sample in r:
		sample = sample[0]
		sample-=min
		out.append(int(((sample*maxOut)/max)))
		
	return out

r = read_whole("C3_PERIODO_his.wav",572*16)
print(r)