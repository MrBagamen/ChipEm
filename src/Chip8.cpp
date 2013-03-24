#include "../include/Chip8.hpp"

void Chip8::Init()
{
	// initialize memory and registers
	memset(memory, 0, 4096);
	memset(vram, 0, 64*32);
	memset(V, 0, 16);

	pc = 0x200;				// pc starts at 0x200
	I = opcode = sp = 0;

	// load fontset
	for(int i = 0; i < 80; i++)
	{
		memory[i] = fontset[i];
	}

	// initialize timers
	delay_timer = 0;
	sound_timer = 0;

	srand(static_cast<unsigned int>(time(nullptr)));

	//Pixel
	pixel[0] = 0;pixel[1] = 0;
	pixel[2] = scale;pixel[3] = 0;
	pixel[4] = scale;pixel[5] = scale;
	pixel[6] = 0;pixel[7] = scale;
}

void Chip8::Load(const char* fileName)
{
	FILE *file;
	long long fSize;
	char *buffer;

	// open file
	file = fopen(fileName, "rb");
	if(file == nullptr){printf("Error loading %s\n", fileName);exit(1);}

	// get file size
	fseek(file, 0, SEEK_END);
	fSize = ftell(file);
	rewind(file);

	// allocate enough memory to store entire file
	buffer = (char*)malloc(sizeof(char)*fSize);

	// copy file into the buffer
	fread(buffer, 1, fSize, file);

	// read rom into memory
	for(int i = 0; i < fSize; i++)
	{
		memory[i+512] = buffer[i];
	}
	printf("Loaded: %s. it's %lld bytes large.\n", fileName, fSize);

	fclose(file);
	free(buffer);
}

void Chip8::Cycle()
{
	// fetch opcode
	opcode = memory[pc] << 8 | memory[pc+1];

	drawFlag = false;

	// decode opcode
	switch(opcode & 0xF000)
	{
		case 0xD000: //DXYN : Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. Each row of 8 pixels is read as bit-coded (with the most significant bit of each byte displayed on the left) starting from memory location I; I value doesn't change after the execution of this instruction. As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn't happen.
		{
			unsigned char x = V[(opcode & 0x0F00) >> 8];
	        unsigned char y = V[(opcode & 0x00F0) >> 4];
	        unsigned char h = opcode & 0x000F;
	        V[0xF] = 0;

	        for (int yy = 0; yy < h; ++yy)
	        {
	            unsigned char px = memory[I + yy];

	            for (int xx = 0; xx < 8; ++xx)
	            {
	                if (px & ((0x80) >> xx))
	                {
	                    unsigned short pos = x + xx + ((y + yy) * 64);

	                    if (pos < (64*32))
	                    {
	                        if(vram[pos])
	                        {
	                            V[0xF] = 1;
	                        }
	                        vram[pos] ^= 1;
	                    }
	                }
	            }
	        }
			drawFlag = true;
			pc += 2;
			break;
		}
		case 0x3000: // 3XNN : Skips the next instruction if VX equals NN.
			if(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
			{
				pc += 4;
			}
			else
			{
				pc += 2;
			}
		break;
		case 0xC000: // CXNN: Sets VX to a random number and NN
			V[(opcode & 0x0F00) >> 8] = rand() % 255 & (opcode & 0x00FF);
			pc += 2;
		break;
		case 0xA000: // ANNN: Sets I to the address NNN
			I = opcode & 0x0FFF;
			pc += 2;
		break;
		case 0x1000: // 1NNN: Jumps to address NNN
			pc = (opcode & 0x0FFF);
		break;
		case 0x7000: // 7XNN: Adds NN to VX
			V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
			pc += 2;
		break;
		case 0x6000: // 6XNN: Sets VX to NN
			V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
			pc += 2;
		break;
		default:
			printf("Unknocn opcode: 0x%X\n", opcode);
	}

	if(delay_timer > 0)
	{
		--delay_timer;
	}
	if(sound_timer > 0)
	{
		--sound_timer;
	}

	Draw();
}

void Chip8::Draw()
{
	if(drawFlag)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glLoadIdentity();

		for(int i = 0, x = 0, y = 0; i < (64*32); i++)
		{
			if(vram[i])
			{
				glPushMatrix();
				glTranslatef(x*scale, y*scale, 0.0f);
				glVertexPointer(2, GL_INT, 0, pixel);
				glDrawArrays(GL_QUADS, 0, 4);
				glPopMatrix();
			}
			++x;
			if(x && (x % 64 == 0))
			{
				x = 0;
				++y;
			}
		}

		SDL_GL_SwapBuffers();
	}
}