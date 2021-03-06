#include "engine.h"

// VC++ Weirdness
#define NOMINMAX

// std includes
#include <iostream>
#include <thread>
#include <chrono>
#include <exception>
#include <stdexcept>

// lib includes
#include <GLFW/glfw3.h>

// mirage includes
#include "config.h"
#include "macros.h"
#include "graphics/window.h"
#include "graphics/gfxengine.h"
#include "game/game.h"
#include "core/input.h"

namespace mirage
{

	CoreEngine::CoreEngine(
		const std::string & cfgFilePath
	) :
		m_config(cfgFilePath),
		m_runState(ERS_UNINTIALIZED),
		m_window(nullptr),
		m_graphicsEngine(nullptr),
		m_game(nullptr)
	{
		// Create window
		m_window = new Window(
			m_config.getString("[gfx]", "windowT", "MirageENGiNE"),
			m_config.getInt("[gfx]", "windowW", 1280),
			m_config.getInt("[gfx]", "windowH", 720),
			m_config.getBool("[gfx]", "fullscreen", false)
		);

		// Create graphics engine
		m_graphicsEngine = new GraphicsEngine(this);

		MLOG_INFO("CoreEngine::CoreEngine, created. Config file: %s", m_config.getFilePath().c_str());
	}

	CoreEngine::~CoreEngine()
	{
		MDELETES(m_game);
		MDELETES(m_graphicsEngine);
		MDELETES(m_window);

		MLOG_INFO("CoreEngine::~CoreEngine, destroyed.");
	}

	void CoreEngine::glfwWindowSizeCallback(GLFWwindow * window, int width, int height)
	{
		glfwSetWindowSize(window, width, height);

		MLOG_DEBUG("CoreEngine::glfwWindowSizeCallback, called. Width: %d, height: %d", width, height);
	}

	void CoreEngine::glfwKeyCallback(GLFWwindow * window, int key, int scancode, int action, int mods)
	{
		input::KB_KEYS[key] = static_cast<input::KBK_State_t>(action);

		//MLOG_DEBUG("CoreEngine::glfwKeyCallback, called. Key: %d, scancode: %d, action: %d, mods: %d", key, scancode, action, mods);
	}

	void CoreEngine::glfwMouseButtonCallback(GLFWwindow * window, int button, int action, int mods)
	{
		input::MS_BUTTONS[button] = (1 & action) != 0;

		//MLOG_DEBUG("CoreEngine::glfwMouseButtonCallback, called. Button: %d, action: %d, mods: %d", button, action, mods);
	}

	void CoreEngine::glfwCursorPosCallback(GLFWwindow * window, double x, double y)
	{
		input::MS_POS_X = static_cast<float>(x);
		input::MS_POS_Y = static_cast<float>(y);

		//MLOG_DEBUG("CoreEngine::glfwCursorPosCallback, called. x: %.2f, y: %.2f", x, y);
	}

	void CoreEngine::run()
	{
		MLOG_INFO("CoreEngine::run(), called. Initializing and entering main loop...");

		// Throw if engine run state is not correct
		if (m_runState != ERS_UNINTIALIZED)
		{
			throw std::runtime_error("CoreEngine::run, m_runState != ERS_UNITIALIZED! Engine run state:" + m_runState);
		}

		// Initialize graphics engine
		m_graphicsEngine->initialize();

		// Throw if game is pointing to null
		if (m_game == nullptr)
		{
			throw std::runtime_error("CoreEngine::run, m_game == nullptr!");
		}

		// Initialize current game instance
		m_game->initialize();

		// Register GLFW callback functions
		glfwSetWindowSizeCallback(m_window->getHandle(), glfwWindowSizeCallback);
		glfwSetKeyCallback(m_window->getHandle(), glfwKeyCallback);
		glfwSetMouseButtonCallback(m_window->getHandle(), glfwMouseButtonCallback);
		glfwSetCursorPosCallback(m_window->getHandle(), glfwCursorPosCallback);

		// Enter the engine's main loop
		m_runState = ERS_RUNNING;
		while (glfwWindowShouldClose(m_window->getHandle()) == GL_FALSE && m_runState == ERS_RUNNING)
		{
			// Poll for any glfw-related events
			glfwPollEvents();

			// Update / render game & execute all rendering tasks in graphicsEngine
			m_game->update(1.0f);
			m_game->render(m_graphicsEngine);
			m_graphicsEngine->render();

			// Sleep for a bit, because why not?
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		MLOG_INFO("CoreEngine::run, exiting main loop. Engine run state: %d", m_runState);
	}

	const IniFile & CoreEngine::getConfig() const
	{
		return m_config;
	}

	const EngineRunState & CoreEngine::getRunState() const
	{
		return m_runState;
	}

	Window * const CoreEngine::getWindow() const
	{
		return m_window;
	}

	GraphicsEngine * const CoreEngine::getGraphicsEngine() const
	{
		return m_graphicsEngine;
	}

	void CoreEngine::setGame(Game * const game)
	{
		if (game == nullptr)
		{
			throw std::runtime_error("CoreEngine::setGame, game == nullptr!");
		}

		m_game = game;
		m_game->setEngine(this);

		MLOG_INFO("Game::setGame, called. Game *: %p", (void *)game);
	}

	Game * const CoreEngine::getGame() const
	{
		return m_game;
	}

}