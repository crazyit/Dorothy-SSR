/* Copyright (c) 2016 Jin Li, http://www.luvfight.me

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "Const/Header.h"
#include "Basic/Application.h"
#include "bx/timer.h"

NS_DOROTHY_BEGIN

Application::Application():
_width(800),
_height(600),
_deltaTime(0),
_updateTime(0),
_frequency(double(bx::getHPFrequency()))
{
	_lastTime = bx::getHPCounter() / _frequency;
}

int Application::getWidth() const
{
	return _width;
}

int Application::getHeight() const
{
	return _height;
}

// This function runs in main thread, and do render work
int Application::run()
{
	if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0)
	{
		Log("SDL fail to initialize! %s", SDL_GetError());
		return 1;
	}

	Uint32 windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
#if BX_PLATFORM_IOS || BX_PLATFORM_ANDROID
	windowFlags |= SDL_WINDOW_FULLSCREEN;
#endif

	SDL_Window* window = SDL_CreateWindow("Dorothy-SSR",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		_width, _height, windowFlags);
	if (!window)
	{
		Log("SDL fail to create window!");
		return 1;
	}

	Application::setSdlWindow(window);

	// call this function here to disable default render threads creation of bgfx
	bgfx::renderFrame();

	// start running logic thread
	_logicThread.init(Application::mainLogic, this);

	SDL_Event event;
	bool running = true;
	while (running)
	{
		// do render staff and swap buffers
		bgfx::renderFrame();

		// handle SDL event in this main thread only
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
    			running = false;
    			break;
			default:
				break;
			}
			_logicEvent.post("SDLEvent", event);
		}

		// poll events from logic thread
		for (Own<QEvent> event = _renderEvent.poll();
			event != nullptr;
			event = _renderEvent.poll())
		{
			switch (Switch::hash(event->getName()))
			{
				case "Quit"_hash:
				{
					SDL_Event ev;
					ev.quit.type = SDL_QUIT;
					SDL_PushEvent(&ev);
					break;
				}
				default:
					break;
			}
		}
	}

	// wait for render process to stop
	while (bgfx::RenderFrame::NoContext != bgfx::renderFrame());
	_logicThread.shutdown();

	SDL_DestroyWindow(window);
	SDL_Quit();

	return _logicThread.getExitCode();
}

void Application::updateDeltaTime()
{
	double currentTime = bx::getHPCounter() / _frequency;
	_deltaTime = currentTime - _lastTime;
	// in case of system timer api error
	if (_deltaTime < 0)
	{
		_deltaTime = 0;
		_lastTime = currentTime;
	}
	// only accept frames drop to 30 FPS
	_deltaTime = min(_deltaTime, 1.0/30);
}

double Application::getEclapsedTime() const
{
	double currentTime = bx::getHPCounter() / _frequency;
	return max(currentTime - _lastTime, 0.0);
}

double Application::getLastTime() const
{
	return _lastTime;
}

double Application::getDeltaTime() const
{
	return _deltaTime;
}

double Application::getUpdateTime() const
{
	return _updateTime;
}

void Application::makeTimeNow()
{
	_lastTime = bx::getHPCounter() / _frequency;
}

void Application::shutdown()
{
	_renderEvent.post("Quit");
}

int Application::mainLogic(void* userData)
{
	Application* app = r_cast<Application*>(userData);
	
	if (!bgfx::init())
	{
		Log("bgfx fail to initialize!");
		return 1;
	}

	SharedPoolManager.push();
	if (!SharedDirector.init())
	{
		Log("Director fail to initialize!");
		return 1;
	}
	SharedPoolManager.pop();

	// Update and invoke render apis
	app->updateDeltaTime();
	bool running = true;
	while (running)
	{
		SharedPoolManager.push();
		// poll events from render thread
		for (Own<QEvent> event = app->_logicEvent.poll();
			event != nullptr;
			event = app->_logicEvent.poll())
		{
			switch (Switch::hash(event->getName()))
			{
				case "SDLEvent"_hash:
				{
					SDL_Event sdlEvent;
					EventQueue::retrieve(event, sdlEvent);
					switch (sdlEvent.type)
					{
						case SDL_QUIT:
							running = false;
							break;
						default:
							break;
					}
					SharedDirector.handleSDLEvent(sdlEvent);
					break;
				}
				default:
					break;
			}
		}
		SharedDirector.mainLoop();
		SharedPoolManager.pop();

		app->_updateTime = app->getEclapsedTime();

		// Advance to next frame. Rendering thread will be kicked to
		// process submitted rendering primitives.
		bgfx::frame();

		// limit for 60 FPS
		do {
			app->updateDeltaTime();
		} while (app->getDeltaTime() < 1.0/60);
		app->makeTimeNow();
	}

	bgfx::shutdown();
	return 0;
}

TargetPlatform Application::getPlatform() const
{
#if BX_PLATFORM_WINDOWS
	return TargetPlatform::Windows;
#elif BX_PLATFORM_ANDROID
	return TargetPlatform::Android;
#elif BX_PLATFORM_OSX
	return TargetPlatform::macOS;
#elif BX_PLATFORM_IOS
	return TargetPlatform::iOS;
#else
	return TargetPlatform::Unknown;
#endif
}

#if !BX_PLATFORM_IOS
void Application::setSdlWindow(SDL_Window* window)
{
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);
	SDL_GetWindowWMInfo(window, &wmi);
	bgfx::PlatformData pd;
#if BX_PLATFORM_OSX
	pd.ndt = nullptr;
	pd.nwh = wmi.info.cocoa.window;
#elif BX_PLATFORM_WINDOWS
	pd.ndt = nullptr;
	pd.nwh = wmi.info.win.window;
#elif BX_PLATFORM_ANDROID
	pd.ndt = nullptr;
	pd.nwh = wmi.info.android.window;
	SDL_GL_GetDrawableSize(window, &_width, &_height);
#endif
	pd.context = nullptr;
	pd.backBuffer = nullptr;
	pd.backBufferDS = nullptr;
	bgfx::setPlatformData(pd);
}
#endif

NS_DOROTHY_END

// Entry functions needed by SDL2
#if BX_PLATFORM_OSX || BX_PLATFORM_ANDROID || BX_PLATFORM_IOS
int main(int argc, char *argv[])
{
	return SharedApplication.run();
}
#endif // BX_PLATFORM_OSX || BX_PLATFORM_ANDROID || BX_PLATFORM_IOS

#if BX_PLATFORM_WINDOWS
int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nCmdShow)
{
#ifndef NDEBUG
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
#endif

	int result = SharedApplication.run();

#ifndef NDEBUG
	FreeConsole();
#endif
	return result;
}
#endif // BX_PLATFORM_WINDOWS
