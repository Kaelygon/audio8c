#include <fstream>
#include <bits/stdc++.h> 

extern const uint8_t cnlcount;
extern const uint8_t steps;
extern const uint8_t sets;

using namespace std; 

//Integer set, handles integers in the arrays
class Intset { 
	public:
		uint8_t* s;
		bool isalloc = 0;
		uint scnt = 0;

//mallocs
	void free(void){
		if(isalloc==1){
			delete[] s;
			isalloc=0;
		}
	}
	//allocate memory
	void reserve( void ){ 
		free();
		s=new uint8_t[ (uint)ceil( scnt*1.0625 ) ];
		if(s == NULL){
			throw "Failed to allocate!";
		}else{
			isalloc=1;
		}
	}

//wave functions
	//invert horizontally
	void invx( void ){  
		uint i = 0;
		while(i<=scnt/2){
			uint8_t tmp = s[i];
			s[i]=s[scnt-i];
			s[scnt-i]=tmp;
			i++;
		}
	}
	//Invert vertically
	void invy( void ){ 
		uint i = 0;
		while(i<scnt){
			s[i]=steps-s[i];
			i+=1;
		}
	}
	//repeats itself c times
	void loopWave(uint c ){ 
	
		uint8_t* buffer=new uint8_t[ (uint)ceil( c*scnt*1.0625 ) ];

		for(int a=0;a<scnt;a++){
			for(int b=0;b<c;b++){
				buffer[b*(scnt)+a]=s[a];
			}
		}
		scnt=c*scnt;
		reserve();
		
		for(int a=0;a<scnt;a++){
			this->s[a]=buffer[a];	
		}

		delete[] buffer;
	}
	//echo, kinda sucks
	void echoWave(uint delay=0, float amp=0){ 
		for(uint a=delay;a<scnt;a+=1){
			s[a]=s[a]*(1-amp)+s[a-delay]*(amp);
		}
	}

};

//Int array collection, handles arrays
class Intcol : public Intset { 
	public:
		Intset fbuf; //function buffer
		Intset* set = new Intset[sets]; //internal integer sets


	//clear tmp function buffer
	void clearbuf(){
		fbuf.free();
	}
	//free sets
	void free( void ){
		clearbuf();
		for(int i=0;i<sets;i++){
			set[i].free();
		}
		delete[] set;
	}	
	//combined channels size
	uint alen(void){ 
		uint asize = 0;
		for(uint8_t a=0;a<cnlcount;a++){
			asize+=set[a].scnt;
		}
		return asize;
	}

//Functions for "internal integer sets" - return nothing
	//average of samples
	void mixWave(uint8_t st, uint8_t nd){ 
		uint stlen = set[st].scnt;
		uint ndlen = set[nd].scnt;

		uint lena = ndlen;
		
		if(stlen>ndlen){
			lena=stlen;
		}

		uint8_t* buffer=new uint8_t[ (uint)ceil( lena*1.0625 ) ];

		int i=0; //avg to buffer
		while(i<=lena){
			if(i>stlen){
				buffer[i]=set[nd].s[i];
			}else if(i>ndlen){
				buffer[i]=set[st].s[i];
			}else{
				buffer[i]=set[st].s[i]/2+set[nd].s[i]/2;
			}
			i++;
		}

		set[st].free(); //clear st and allocate lena size 
		set[st].scnt=lena;
		set[st].reserve();

		i=0;
		while(i<=lena){ //buffer to st
			set[st].s[i]=buffer[i];
			i++;
		}

		delete[] buffer;		
	}
	
	//add together samples
	void addWave(uint8_t st, uint8_t nd){ 
		uint lena = set[nd].scnt;
		
		if(set[st].scnt>set[nd].scnt){
			lena=set[st].scnt;
		}

		for(uint a=0;a<lena;a+=1){
			set[st].s[a] = set[st].s[a]+set[nd].s[a];
		}
	}
	//add together samples
	void modWave(uint8_t st, uint8_t nd){ 
		uint lena = set[nd].scnt;
		uint max = steps;
		if(set[st].scnt>set[nd].scnt){
			lena=set[st].scnt;
		}

		for(uint a=0;a<lena;a+=1){
			double multi = set[nd].s[a]/max ;
			set[st].s[a] = set[st].s[a]*multi;
		}
	}
	//copy st to nd
	void macp( uint8_t st, uint8_t nd ){
		set[nd].free();
		set[nd].scnt=set[st].scnt;
		set[nd].reserve();

		for(uint a=0;a<set[nd].scnt;a+=1){
			set[nd].s[a] = set[st].s[a];
		}
	}
	//concatenate nd to end of st
	void macon( uint8_t st, uint8_t nd ){  

		uint stlen = set[st].scnt;
		uint ndlen = set[nd].scnt;
		uint totlen = stlen+ndlen;

		uint8_t* buffer=new uint8_t[ (uint)ceil( stlen*1.0625 ) ];

		int i=0; //st to buffer
		while(i<=ndlen){
			buffer[i]=set[st].s[i];
			i++;
		}

		set[st].free(); //clear st and allocate st+nd size 
		set[st].scnt=totlen;
		set[st].reserve();

		i=0;
		while(i<=ndlen){ //concat buffer and nd to st
			set[st].s[i]=buffer[i];
			set[st].s[i+stlen]=set[nd].s[i];
			i++;
		}

		delete[] buffer;
	}



//Functions for external integer sets - return Intset
	//combine channels, cnl[0]=000 cnl[1]=123, return 010203. This is at least how pcm reads the channels
	Intset mixChannels(){ 
		fbuf.scnt=alen();
		fbuf.reserve();
		uint8_t c;
		for(int a=0;a< alen()/cnlcount ;a+=1){
			for(int b=0;b<cnlcount;b+=1){
				if( a > set[b].scnt ){
					c=0;
				}else{
					c = set[b].s[a];
				}
				fbuf.s[ a*cnlcount+b ] = c;
			}
		}
		return fbuf;
	}



};
