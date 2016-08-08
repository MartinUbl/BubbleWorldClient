/**
 * Copyright (C) 2016 Martin Ubl <http://kennny.cz>
 *
 * This file is part of BubbleWorld MMORPG engine
 *
 * BubbleWorld is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BubbleWorld is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BubbleWorld. If not, see <http://www.gnu.org/licenses/>.
 **/

#include "General.h"
#include "Application.h"
#include "Drawing.h"
#include "NetworkManager.h"
#include "Log.h"
#include "StorageManager.h"
#include "Gameplay.h"
#include "FramerateLimiter.h"

#include "CRC32.h"

#include <assert.h>
#include <thread>

Application::Application()
{
    m_stage = nullptr;
    m_stageType = STAGE_NONE;
    m_pendingStageType = STAGE_NONE;

    memset(&m_keyMod, 0, sizeof(m_keyMod));
}

Application::~Application()
{
    //
}

bool Application::Init()
{
    // log some info
    sLog->Info("BubbleWorld " APP_VERSION_STR);

#ifdef WIN32
    std::locale::global(std::locale("czech"));
    setlocale(LC_ALL, "czech");
    _wsetlocale(LC_ALL, L"czech");
#else
    std::locale::global(std::locale("cs_CZ"));
    setlocale(LC_ALL, "cs_CZ");
#endif

    // init SDL library
    if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
    {
        sLog->Error("Could not initialize SDL subsystem");
        return false;
    }

    // init SDL_image library
    if (!IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF))
    {
        sLog->Error("Could not initialize SDL_image subsystem");
        return false;
    }

    // init drawing class
    sDrawing->Init();

    // init local file storages
    if (!sStorageManager->Init())
    {
        sLog->Error("Could not initialize storage manager");
        return false;
    }

    // init networking
    if (!sNetwork->Init())
    {
        sLog->Error("Could not initialize networking");
        return false;
    }

    // move stage to menu
    SetStageType(STAGE_MENU);

    // initialize FPS limiter
    // TODO: load FPS limit from config
    sFramerateLimiter->Initialize(200);

    return true;
}

int Application::Run()
{
    SDL_Event ev;

    int quit = 0;
    // main application loop
    while (!quit)
    {
        while (SDL_PollEvent(&ev))
        {
            switch (ev.type)
            {
                // quit signal (close window, alt+f4, ..)
                case SDL_QUIT:
                    quit = true;
                    break;
                // mouse move - store coordinates
                case SDL_MOUSEMOTION:
                    m_mouseX = ev.motion.x;
                    m_mouseY = ev.motion.y;
                    sDrawing->OnMouseMove(m_mouseX, m_mouseY);
                    sGameplay->CheckHoverObject();
                    break;
                // mouse click - signal UI and stage
                case SDL_MOUSEBUTTONDOWN:
                    if (m_stage)
                    {
                        // left button
                        if (ev.button.button == 1)
                            m_stage->OnMouseClick(true, true);
                        // right button
                        else if (ev.button.button == 2)
                            m_stage->OnMouseClick(false, true);
                    }
                    sDrawing->OnMouseClick(ev.button.button == 1, true);
                    break;
                // mouse button up - signal UI and stage
                case SDL_MOUSEBUTTONUP:
                    if (m_stage)
                    {
                        // left button
                        if (ev.button.button == 1)
                            m_stage->OnMouseClick(true, false);
                        // right button
                        else if (ev.button.button == 2)
                            m_stage->OnMouseClick(false, false);
                    }
                    sDrawing->OnMouseClick(ev.button.button == 1, false);
                    break;
                // key pressed event - signal UI and stage
                case SDL_KEYDOWN:
                    StoreKeyMod(ev.key.keysym.mod);
                    if (!sDrawing->OnKeyPress(ev.key.keysym.sym, true))
                    {
                        if (m_stage)
                            m_stage->OnKeyPress(ev.key.keysym.sym, true);
                    }
                    break;
                // key released event - signal UI and stage
                case SDL_KEYUP:
                    StoreKeyMod(ev.key.keysym.mod);
                    if (!sDrawing->OnKeyPress(ev.key.keysym.sym, false))
                    {
                        if (m_stage)
                            m_stage->OnKeyPress(ev.key.keysym.sym, false);
                    }
                    break;
                // text input event - when focused to textfield widget
                case SDL_TEXTINPUT:
                    sDrawing->OnTextInput(ev.text.text);
                    break;
            }
        }

        // process pending packets
        sNetwork->ProcessPending();

        // perform "before draw" method on stage
        if (m_stage)
            m_stage->OnBeforeDraw();

        // update gameplay
        sGameplay->Update();

        // update drawing class and draw if needed
        sDrawing->Update();

        // perform "after draw" method on stage
        if (m_stage)
            m_stage->OnDraw();

        // if there is some stage change waiting to be done, switch stages
        SetStage_internal();

        // invoke framerate limiter
        sFramerateLimiter->Limit();
    }

    SDL_Quit();

    return 0;
}

void Application::StoreKeyMod(uint16_t mod)
{
    m_keyMod[KEYMOD_SHIFT] = (mod & KMOD_SHIFT) != 0;
    m_keyMod[KEYMOD_CTRL] = (mod & KMOD_CTRL) != 0;
    m_keyMod[KEYMOD_ALT] = (mod & KMOD_ALT) != 0;
    m_keyMod[KEYMOD_CAPS] = (mod & KMOD_CAPS) != 0;
    m_keyMod[KEYMOD_NUM] = (mod & KMOD_NUM) != 0;
}

void Application::SetStageType(StageType type)
{
    // if we are in the state right now, nothing to change
    if (m_stageType != type)
        m_pendingStageType = type;
}

StageType Application::GetStageType()
{
    return m_stageType;
}

void Application::SetStage_internal()
{
    // no pending state? return
    if (m_pendingStageType == STAGE_NONE)
        return;

    StageTemplate* st = nullptr;

    // create new state from pending type
    switch (m_pendingStageType)
    {
        case STAGE_MENU:
            st = new MenuStage();
            break;
        case STAGE_LOBBY:
            st = new LobbyStage();
            break;
        case STAGE_CONNECTING:
            st = new ConnectingStage();
            break;
        case STAGE_GAME:
            st = new GameStage();
            break;
        default:
            break;
    }

    // stage has to be created at this point
    if (!st)
    {
        sLog->Error("Unknown stage supplied or stage not created for some reason");
        assert(false);
        abort();
    }

    // if there is current stage, leave it and destroy UI
    if (m_stage)
    {
        m_stage->OnLeave();
        sDrawing->DestroyUI();
        delete m_stage;
    }

    // call OnEnter method
    st->OnEnter();
    m_stage = st;

    // switch internal types
    m_stageType = m_pendingStageType;
    m_pendingStageType = STAGE_NONE;
}

void Application::SignalUIEvent(uint32_t elementId, UIActionType actionId)
{
    if (m_stage)
        m_stage->OnUIAction(elementId, actionId);
}

void Application::SignalGlobalEvent(GlobalActionIDs actionId, void* actionParam)
{
    if (m_stage)
        m_stage->OnGlobalAction(actionId, actionParam);
}

void Application::SetTextInputModeState(bool enabled)
{
    // allow only state change (ternary condition due to compiler complaining about types)
    if ((enabled ^ (SDL_IsTextInputActive() ? 1 : 0)) == 0)
        return;

    if (enabled)
        SDL_StartTextInput();
    else if (!enabled)
        SDL_StopTextInput();
}
