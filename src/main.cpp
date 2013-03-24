#include "../include/Chip8.hpp"

void Init(int scale);
int main(int argc, char** argv)
{
	//sage
	Chip8 emu;
	emu.Init();
	emu.Load("/home/luka/Desktop/chip8roms/MAZE");

	Init(emu.scale);
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
				default:;
			}
		}
		//glClear(GL_COLOR_BUFFER_BIT);
		//glLoadIdentity();

		emu.Cycle();

		SDL_GL_SwapBuffers();
		SDL_Delay(16);
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