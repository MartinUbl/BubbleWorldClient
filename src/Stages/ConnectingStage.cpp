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
#include "Colors.h"
#include "Drawing.h"
#include "UI/LabelWidget.h"
#include "UI/ButtonWidget.h"
#include "UI/TextFieldWidget.h"
#include "UI/SplashMessageWidget.h"
#include "Stages.h"
#include "Gameplay.h"
#include "Application.h"

void ConnectingStage::OnEnter()
{
    // just add "Loading..." text to the center of screen
    LabelWidget* lw = LabelWidget::Create(0, 0, FONT_CAPTION, "Loading...");
    lw->SetPositionXLambda([lw](int32_t win_w, int32_t) {
        return win_w / 2 - lw->GetWidth() / 2;
    });
    lw->SetPositionYLambda([lw](int32_t, int32_t win_h) {
        return win_h / 2 - lw->GetHeight() / 2;
    });
    sDrawing->AddUIWidget(lw);
}

void ConnectingStage::OnDraw()
{
    //
}

void ConnectingStage::OnUIAction(uint32_t elementId, UIActionType actionId)
{
    //
}

void ConnectingStage::OnGlobalAction(GlobalActionIDs actionId, void* actionParam)
{
    //
}
