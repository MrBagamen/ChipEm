#include "../include/Chip8.hpp"

void Init(int scale);
int main(int argc, char** argv)
{
	Chip8 emu;

	Init(emu.scale);
	emu.Init();
	emu.Load("/home/luka/Desktop/chip8roms/PUZZLE");

	SDL_Event event;
	bool is_running = true;

	while(is_running)
	{
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
					is_running = false;
				break;
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym)
					{
						case SDLK_w:
							emu.keys[0x1] = 1;
						break;
						case SDLK_2:
							emu.keys[0x2] = 1;
						break;
						case SDLK_0:
							emu.keys[0x3] = 1;
						break;
						case SDLK_UP:
							emu.keys[0xC] = 1;
						break;
						case SDLK_s:
							emu.keys[0x4] = 1;
						break;
						case SDLK_1:
							emu.keys[0x5] = 1;
						break;
						case SDLK_o:
							emu.keys[0x6] = 1;
						break;
						case SDLK_DOWN:
							emu.keys[0xD] = 1;
						break;
						case SDLK_a:
							emu.keys[0x7] = 1;
						break;
						case SDLK_q:
							emu.keys[0x8] = 1;
						break;
						case SDLK_k:
	                    	emu.keys[0x9] = 1;
	                    break;
		                case SDLK_l:
		                   	emu.keys[0xE] = 1;
		                break;
		                case SDLK_z:
		                    emu.keys[0xA] = 1;
		                break;
		                case SDLK_x:
		                    emu.keys[0x0] = 1;
		                break;
		                case SDLK_n:
		                    emu.keys[0xB] = 1;
		                break;
		                case SDLK_m:
		                    emu.keys[0xF] = 1;
		                break;
		                default:;
					}
				break;
				case SDL_KEYUP:
					switch(event.key.keysym.sym)
					{
						case SDLK_w:
							emu.keys[0x1] = 0;
						break;
						case SDLK_2:
							emu.keys[0x2] = 0;
						break;
						case SDLK_0:
							emu.keys[0x3] = 0;
						break;
						case SDLK_UP:
							emu.keys[0xC] = 0;
						break;
						case SDLK_s:
							emu.keys[0x4] = 0;
						break;
						case SDLK_1:
							emu.keys[0x5] = 0;
						break;
						case SDLK_o:
							emu.keys[0x6] = 0;
						break;
						case SDLK_DOWN:
							emu.keys[0xD] = 0;
						break;
						case SDLK_a:
							emu.keys[0x7] = 0;
						break;
						case SDLK_q:
							emu.keys[0x8] = 0;
						break;
						case SDLK_k:
	                    	emu.keys[0x9] = 0;
	                    break;
		                case SDLK_l:
		                   	emu.keys[0xE] = 0;
		                break;
		                case SDLK_z:
		                    emu.keys[0xA] = 0;
		                break;
		                case SDLK_x:
		                    emu.keys[0x0] = 0;
		                break;
		                case SDLK_n:
		                    emu.keys[0xB] = 0;
		                break;
		                case SDLK_m:
		                    emu.keys[0xF] = 0;
		                break;
		                default:;
					}
				break;
				default:;
			}
		}
		emu.Cycle();
	}

	SDL_Quit();
	return 0;
}

void Init(int scale)
{
	SDL_SetVideoMode(64*scale, 32*scale, 32, SDL_HWSURFACE | SDL_OPENGL);
	SDL_WM_SetCaption("ChipEm", nullptr);

	//Set Projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 64*scale, 32*scale, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glEnableClientState(GL_VERTEX_ARRAY);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}