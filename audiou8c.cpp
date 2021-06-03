///*******************
//Created by Kaelygon
//Do What The Fuck You Want To Public License (WTFPL)
//I'm not responsible of anything if you use this code, like if your computer blows up or whatever. Memory likely pissing everywhere
//You need stuff like ffmpeg to run this. Might not compile on Windows without modification
//********************/

#include <fstream>
#include <bits/stdc++.h>

using namespace std; 

//constants
const uint8_t sets = 32; //Count of integer arrays
const uint8_t steps = pow(2,8)-1; //bit depth
const uint16_t samplehz = pow(2,16)-1; //sampling rate
const uint8_t cnlcount = 4; //channel count, 1-mono, 2-stereo etc
const char fileName[] = "audio";
const char rawFormat[] = "pcm";
const char encFormat[] = "aiff";

#include "./intset.cpp"

Intcol coia; //Collection of integer arrays

//decimal modulus
double qmod(double a, double b){	
	return a-b*floor(a/b);
}

//sign
double qsign(double n){
	return n < 0 ?  -1 : 1;
}

//System command functions
void convAudio(string aname){ //ffmpeg raw to fileName.encFormat
	string cmd ="ffmpeg -y -hide_banner -loglevel error -f u8 -ar ";
	cmd += to_string(samplehz);
	cmd +=" -ac ";
	cmd += to_string(cnlcount);
	cmd += " -i ./";
	cmd += (string)fileName;
	cmd += ".";
	cmd += rawFormat;
	cmd += " -acodec pcm_u8 ";
	cmd += " -filter:a volume=1 "+aname+"."+encFormat;
	cout << "convert audio " << cmd <<"\n\n";

	system(cmd.c_str());
}
void playAudio(void){ //ffplay
	string cmd ="ffplay -hide_banner -loglevel error -f u8 -ar ";
	cmd += to_string(samplehz);
	cmd +=" -ac ";
	cmd += to_string(cnlcount);
	cmd += " -volume 25 -autoexit -i "+(string)fileName+"."+rawFormat;
	cout << "play audio " << cmd <<"\n\n";

	system(cmd.c_str());
}

//write functions
//write raw file
void write(string file, Intset data){
	file += "."+(string)rawFormat; 
	ofstream binFile(file, ios::out | ios::binary | std::ios::app);
	if (binFile.is_open()){
		binFile.write((char*)&data.s[0], coia.alen() );
	}
}
//reset
void rst(string file){
	file += "."+(string)rawFormat; 
	ofstream binFile(file, ios::out | ios::binary);
	if (binFile.is_open()){
		binFile.write("", 0);
	}
}


//Single sample functions
uint8_t sine(double t, uint wl, uint h){ //Sinusoidal waveform
	return (sin(2*3.14159265*t/wl)+1)/2*h;
}

uint8_t triangle(double t, uint8_t wl, uint8_t h){ //Triangular waveform
	t+=3*wl/4;
	return 2*abs( ((t/wl - floor(t/wl)))-0.5 )*h;
}

uint8_t saw(double t, uint wl, uint h){ //Ramp waveform
	t+=wl/2;
	return qmod(t/wl,1)*h;
}

uint8_t square(double t, uint wl, uint h){ //Square waveform
	return floor( 2*((t/wl - floor(t/wl)) ) )*h;
}

//generate sound
void a( void ){

	uint samples = 1024*64;
	int bufs = 6; //count of channel allocations
	for(int i=0;i<bufs;i++){ //allocate channels
		coia.set[i].scnt = samples;
		coia.set[i].reserve();
	}
	
	for(int b=0;b<samples;b+=1){
		uint l=255;//wave length
		uint h=255;//amplitude

		coia.set[0].s[b]=rand()%255; //some randomness
		coia.set[1].s[b]=sine(1.25*b,l,h);
		coia.set[2].s[b]=square(1.5*b,l,h);
		coia.set[3].s[b]=triangle(2.0*b,l,h);

	}

	coia.set[0].loopWave(2); //loop set 0 twice
	coia.set[0].invy(); //invert waveform
	coia.macon(0,1); //concatenate 1 at the end of 0
	coia.mixWave(3,0); //average sets 3 and 0
	coia.modWave(1,2); //multiply set 1 by set 2
	coia.set[0].invx(); //reverse track
	coia.mixWave(2,3); //average sets 2 and 3 
	coia.macp(0,1); //copy set 0 to 1 

}

int main(){
	
	//reset random and the file
	srand(time(NULL));
	rst(fileName);

	//sound function
	a();
	
	//write channels to a file
	write(fileName, coia.mixChannels() );
		
	//free memory
	coia.free();

	//ffmpeg
	convAudio(fileName);

	//ffplay
	playAudio();

	return 0;
}
