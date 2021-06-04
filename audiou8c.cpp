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
//Sinusoidal waveform
uint8_t sine(double t, int wl, uint h){ 
	return (sin(2*3.14159265*t/wl)+1)/2*h;
}
//Triangular waveform
uint8_t triangle(double t, uint8_t wl, uint8_t h){ 
	t+=3*wl/4;
	return 2*abs( ((t/wl - floor(t/wl)))-0.5 )*h;
}
//Ramp waveform
uint8_t saw(double t, int wl, uint h){ 
	t+=wl/2;
	return qmod(t/wl,1)*h;
}
//Square waveform
uint8_t square(double t, int wl, uint h){ 
	return floor( 2*((t/wl - floor(t/wl)) ) )*h;
}

//BOF b();
void b( void ){

	uint samples = 1024*2048;
	int bufs = 8; //count of channel allocations

	for(int i=0;i<bufs;i++){ //allocate channels
		coia.set[i].scnt = samples;
		coia.set[i].reserve();
	}
	
	double x=0;
	double m=1;

	double g=0.13;
	double d=7.6;

	coia.set[16].scnt = samples;
	coia.set[16].reserve();

	//random chords
	int z=0;
	double zm=1;
	int al=0;
	int bl=0;
	int lct=0;
	for(int b=0;b<samples;b+=1){
		uint h=255;//amplitude
		uint l=255*2;//wave length

		if(b%24==0){
			x=sine(z,l+al*32-bl*32,100)+sine(z,l*0.5+al*32-bl*32,55);
		}
		if(b%(1024*8)==0){
			al=rand()%7+rand()%7;
			bl=rand()%5+rand()%5;
		}		

		if(  b%(1024*(32+al))	<	1024*(16+bl)	){
			coia.set[16].s[b]=x;
		}else{
			int mx=al*8-bl*8+1;
			coia.set[16].s[b]=rand()%( mx )+127-mx/2 ;
		}
		z+=zm;

	}
	
	//base
	samples=1024*32;
	coia.set[29].scnt = samples;
	coia.set[29].reserve();
	x=samples;
	z=1;
	for(int b=samples;b>0;b-=1){
		uint h=255;
		uint l=255*3;
	
		coia.set[29].s[b]=sine(b, l * ( 0.25+0.75 * ( b*(3.0/2.0) )/samples ) ,h);

		if(b>1024*24){
			coia.set[29].s[b]=0;
		}

	}

	//hithat
	samples=1024*16;
	coia.set[28].scnt = samples;
	coia.set[28].reserve();
	for(int b=samples;b>0;b-=1){
		x=b;
	
		coia.set[28].s[b]= rand()%127+64;

		if(
			b>1024*2 && 
			b<1024*8 ||
			b>1024*10 && 
			b<1024*16 
		){
			coia.set[28].s[b]=127;
		}

	}



	coia.set[29].loopWave(70);
	coia.set[28].loopWave(120);

	coia.modWave(0,1);
	
	coia.macp(0,30);
	coia.set[30].invy();
	coia.modWave(2,30);

	coia.mixWave(0,2);
	
	coia.macp(16,0);
	coia.macp(16,1);
	
	coia.mixWave(29,28);

	coia.macp(29,2);
	coia.macp(29,3);

}//EOF b();

int main(){
	
	//reset random and the file
	srand(time(NULL));
	rst(fileName);

	//sound function
	b();
	
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