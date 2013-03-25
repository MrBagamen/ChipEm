#include "../include/Chip8.hpp"

void Chip8::Init()
{
	// initialize memory and registers
	memset(memory, 0, 4096);
	memset(keys, 0, 16);
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
		case 0x4000: //4XNN: Skips the next instruction if VX doesn't equal NN
			if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
			{
				pc += 2;
			}
			pc += 2;
		break;
		case 0x7000: // 7XNN: Adds NN to VX
			V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
			pc += 2;
		break;
		case 0x6000: // 6XNN: Sets VX to NN
			V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
			pc += 2;
		break;
		case 0x5000: // 5XY0: Skips the next instruction if VX equals VY
			if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
			{
				pc += 2;
			}
			pc += 2;
		break;
		case 0x2000: // 2NNN: Calls subroutine at NNN
			stack[sp] = pc;
			++sp;
			pc = (opcode & 0x0FFF);
		break;
		case 0x0000:
			switch(opcode & 0x0FFF)
			{
				case 0x00EE: //00EE: Returns from a subroutine
					--sp;
					pc = stack[sp];
					pc += 2;
				break;
				case 0x00E0: // 00E0: Clears the screen
				memset(vram, 0, 64*32);
				pc += 2;
				break;
				default:
					printf("Unknown opcode: 0x%X\n", opcode);
			}
		break;
		case 0x9000: // 9XY0: Skips the next instruction if VX doesn't equal VY
			if(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
			{
				pc += 2;
			}
			pc += 2;
		break;
		case 0x8000:
			switch(opcode & 0x000F)
			{
				case 0x0000: // 8XY0: Sets VX to the value of VY
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;
				case 0x0002: // 8XY2: Sets VX to VX and VY
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4];
					pc += 2; 
				break;
				case 0x0004: // 8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
					if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
		            {
		                V[0xF] = 1;
		            }
		            else
		            {
		                V[0xF] = 0;
		            }
		            V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;
				case 0x0005: // 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
					if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
		            {
		                V[0xF] = 0;
		            }
		            else
		            {
		                V[0xF] = 1;
		            }
		            V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;
				case 0x0003: // 8XY3: Sets VX to VX xor VY
					V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;
				case 0x000E: // 8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift.
					V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] << 1;
					pc += 2;
				break;
				default:
					printf("Unknown opcode: 0x%X\n", opcode);
			}
		break;
		case 0xE000:
			switch(opcode & 0x00FF)
			{
				case 0x00A1: // EXA1: Skips the next instruction if the key stored in VX isn't pressed
					if(!keys[V[(opcode & 0x0F00) >> 8]])
		            {
		                pc += 2;
		            }
		            pc += 2;
				break;
				case 0x009E: // EX9E: Skips the next instruction if the key stored in VX is pressed
					if(keys[V[(opcode & 0x0F00) >> 8]])
					{
						pc += 2;
					}
					pc += 2;
				break;
				default:
					printf("Unknown opcode: 0x%X\n", opcode);
			}
		break;
		case 0xF000:
			switch(opcode & 0x00FF)
			{
				case 0x0033: // FX33: Stores the Binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2. (In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.)
					memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
					memory[I+1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
					memory[I+2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
					pc += 2;
				break;
				case 0x0065: // FX65: Fills V0 to VX with values from memory starting at address I.
					for(int i = 0; i <= (opcode & 0x0F00) >> 8; i++)
					{
						V[i] = memory[I+i];
					}
					pc += 2;
				break;
				case 0x0029: // FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
					I = V[(opcode & 0x0F00) >> 8] * 0x5;
					pc += 2;
				break;
				case 0x0015: // FX15: Sets the delay timer to VX
					delay_timer = V[(opcode & 0x0F00) >> 8];
					pc += 2;
				break;
				case 0x0007: // FX07: Sets VX to the value of the delay timer.
					V[(opcode & 0x0F00) >> 8] = delay_timer;
					pc += 2;
				break;
				case 0x0018: // FX18: Sets the sound timer to VX
					sound_timer = V[(opcode & 0x0F00) >> 8];
					pc += 2;
				break;
				case 0x001E: // FX1E: Adds VX to I
					I += V[(opcode & 0x0F00)];
					pc += 2;
				break;
				case 0x0055: // FX55: Stores V0 to VX in memory starting at address I
					for(int i = 0; i <= (opcode & 0x0F00) >> 8; i++)
					{
						memory[I+i] = V[i];
					}
					pc += 2;
				break;
				case 0x000A: // FX0A: A key press is awaited, and then stored in VX
					for(int i = 0; i < 16; i++)
					{
						if(keys[i])
						{
							V[(opcode & 0x0F00) >> 8] = keys[i];
						}
					}
					pc += 2;
				break;
				default:
					printf("Unknown opcode: 0x%X\n", opcode);
			}
		break;
		default:
			printf("Unknown opcode: 0x%X\n", opcode);
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