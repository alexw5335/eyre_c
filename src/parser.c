#include "eyre.h"



// ASCII bitmap of valid digit chars (0-9, _)
u8 digitMap[32] = {
	0,0,0,0,0,0,
	0b11111111, // 48
	0b00000001, // 56
	0,0,0,
	0b10000000, // 88
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};



// Bitmap of valid identifier chars (a-z, A-Z, 0-9, _)
u8 idMap[32] = {
	0,0,0,0,0,0,
	0b11111111, // 48
	0b00000001, // 56
	0b11111110, // 64
	0b11111111, // 72
	0b11111111, // 80
	0b10000111, // 88
	0b11111110, // 96
	0b11111111, // 104
	0b11111111, // 112
	0b00000111, // 120
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};



int numTokens;

#define tokenCapacity 1 << 16

u32 tokens[tokenCapacity];

u8 tokenTypes[tokenCapacity];

u8 terminators[tokenCapacity >> 3];



void eyreLex(SrcFile* srcFile) {
	char* data = srcFile->data;
	int pos    = 0;
	int size   = srcFile->size;
	numTokens  = 0;

	while(pos < size) {
		switch(data[pos]) {
			case '\n': {
				terminators[pos >> 3] &= (1 << (pos & 7));
				break;
			}

			case '\t':
			case '\r':
			case ' ' : { pos++; break; }

			case '0': {
				char base = data[pos + 1];
				if(base == 'b' || base == 'x' || base == 'd' || base == 'o')
					pos++;
			} // fallthrough

			case '1'...'9': {
				pos++;
				while(1) {
					char c = data[pos];
					if(digitMap[c >> 3] & (1 << (c & 7)))
						pos++;
					else
						break;
				}
				numTokens++;
				break;
			}

			case '_':
			case 'a'...'z':
			case 'A'...'Z': {
				while(1) {
					char c = data[pos];
					if(idMap[c >> 3] & (1 << (c & 7)))
						pos++;
					else
						break;
				}
				numTokens++;
				break;
			}

			case '!': {
				numTokens++;
				pos++;
				if(data[pos] == '=') pos++;
				break;
			}

			case '#': {
				pos++;
				numTokens++;
				break;
			}

			default:
				eyreLogError("Invalid ascii char codepoint: %d", data[pos]);
		}
	}
}