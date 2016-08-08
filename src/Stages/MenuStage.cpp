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

enum MenuStageElementIDs
{
    ELEMENT_LOGIN_BTN = 1000,
    ELEMENT_USERNAME_FIELD = 1001,
    ELEMENT_PASSWORD_FIELD = 1002
};

void MenuStage::OnEnter()
{
    // game name to the top
    LabelWidget* lw = LabelWidget::Create(0, 20, FONT_BIGGER, L"BubbleWorld", BWCOLOR_BUZNABLUE);
    lw->SetPositionXLambda([lw](int32_t win_w, int32_t) {
        return win_w / 2 - lw->GetWidth() / 2;
    });
    sDrawing->AddUIWidget(lw);

    // username label
    lw = LabelWidget::Create(0, 0, FONT_MAIN, L"Username");
    lw->SetPositionXLambda([lw](int32_t win_w, int32_t) {
        return win_w / 2 - lw->GetWidth() / 2;
    });
    lw->SetPositionYLambda([lw](int32_t, int32_t win_h) {
        return win_h / 2 - 100;
    });
    sDrawing->AddUIWidget(lw);

    // username input field
    TextFieldWidget* wg = TextFieldWidget::Create(20, 130, 400, FONT_MAIN, L"");
    wg->SetID(ELEMENT_USERNAME_FIELD);
    wg->SetPositionXLambda([wg](int32_t win_w, int32_t) {
        return win_w / 2 - wg->GetWidth() / 2;
    });
    wg->SetPositionYLambda([wg](int32_t, int32_t win_h) {
        return win_h / 2 - 80;
    });
    sDrawing->AddUIWidget(wg);

    // password label
    lw = LabelWidget::Create(0, 200, FONT_MAIN, L"Password");
    lw->SetPositionXLambda([lw](int32_t win_w, int32_t) {
        return win_w / 2 - lw->GetWidth() / 2;
    });
    lw->SetPositionYLambda([lw](int32_t, int32_t win_h) {
        return win_h / 2 - 20;
    });
    sDrawing->AddUIWidget(lw);

    // password input field
    wg = TextFieldWidget::Create(0, 230, 400, FONT_MAIN, L"");
    wg->SetID(ELEMENT_PASSWORD_FIELD);
    wg->SetMaskCharacter('*');
    wg->SetPositionXLambda([wg](int32_t win_w, int32_t) {
        return win_w / 2 - wg->GetWidth() / 2;
    });
    wg->SetPositionYLambda([wg](int32_t, int32_t win_h) {
        return win_h / 2 + 0;
    });
    sDrawing->AddUIWidget(wg);

    // login button
    ButtonWidget* bw = ButtonWidget::Create(0, 280, FONT_MAIN, L"LOGIN");
    bw->SetID(ELEMENT_LOGIN_BTN);
    bw->SetPositionXLambda([bw](int32_t win_w, int32_t) {
        return win_w / 2 - bw->GetWidth() / 2;
    });
    bw->SetPositionYLambda([bw](int32_t, int32_t win_h) {
        return win_h / 2 + 70;
    });
    sDrawing->AddUIWidget(bw);
}

void MenuStage::OnDraw()
{
    //
}

void MenuStage::OnUIAction(uint32_t elementId, UIActionType actionId)
{
    // clicked on login button
    if (elementId == ELEMENT_LOGIN_BTN && actionId == UIACTION_MOUSE_CLICK_LEFT)
    {
        TextFieldWidget* tfw;
        
        // retrieve username and password
        tfw = (TextFieldWidget*)sDrawing->FindUIWidgetById(ELEMENT_USERNAME_FIELD);
        const wchar_t* username = tfw->GetText();
        tfw = (TextFieldWidget*)sDrawing->FindUIWidgetById(ELEMENT_PASSWORD_FIELD);
        const wchar_t* password = tfw->GetText();

        // secure length
        if (wcslen(username) == 0)
        {
            sDrawing->AddUIWidget(SplashMessageWidget::Create(SPLASH_TYPE_ERROR, 500, FONT_MAIN, L"Enter your username!"));
            return;
        }
        if (wcslen(password) == 0)
        {
            sDrawing->AddUIWidget(SplashMessageWidget::Create(SPLASH_TYPE_ERROR, 500, FONT_MAIN, L"Enter your password!"));
            return;
        }

        // connect to server and wait before sending any data
        sGameplay->ConnectToServer();
    }
}

void MenuStage::OnGlobalAction(GlobalActionIDs actionId, void* actionParam)
{
    switch (actionId)
    {
        case GA_CONNECTION_START:
            sDrawing->AddUIWidget(SplashMessageWidget::Create(SPLASH_TYPE_NORMAL, 500, FONT_MAIN, L"Connecting...", false));
            break;
        case GA_CONNECTION_CONNECTED:
        {
            sDrawing->AddUIWidget(SplashMessageWidget::Create(SPLASH_TYPE_NORMAL, 500, FONT_MAIN, L"Authenticating...", false));

            TextFieldWidget* tfw;

            tfw = (TextFieldWidget*)sDrawing->FindUIWidgetById(ELEMENT_USERNAME_FIELD);
            std::string username = WStringToUTF8(tfw->GetText());
            tfw = (TextFieldWidget*)sDrawing->FindUIWidgetById(ELEMENT_PASSWORD_FIELD);
            std::string password = WStringToUTF8(tfw->GetText());

            // send login packet after connection succeeded
            sGameplay->Login(username.c_str(), password.c_str());
            break;
        }
        case GA_CONNECTION_FETCHING:
            sDrawing->AddUIWidget(SplashMessageWidget::Create(SPLASH_TYPE_NORMAL, 500, FONT_MAIN, L"Fetching data...", false));
            // request character list and move to lobby stage
            sGameplay->RequestCharacterList();
            sApplication->SetStageType(STAGE_LOBBY);
            break;
        case GA_CONNECTION_SUCCESS:
            // ...
            break;
        case GA_CONNECTION_UNABLE_TO_CONNECT:
            sDrawing->AddUIWidget(SplashMessageWidget::Create(SPLASH_TYPE_ERROR, 500, FONT_MAIN, L"Unable to connect to server! Please, check your internet connection and try again."));
            break;
        case GA_CONNECTION_INVALID_USER:
            sDrawing->AddUIWidget(SplashMessageWidget::Create(SPLASH_TYPE_ERROR, 500, FONT_MAIN, L"Invalid username"));
            break;
        case GA_CONNECTION_INVALID_PASSWORD:
            sDrawing->AddUIWidget(SplashMessageWidget::Create(SPLASH_TYPE_ERROR, 500, FONT_MAIN, L"Invalid password"));
            break;
        case GA_CONNECTION_INCOMPATIBLE_VERSION:
            sDrawing->AddUIWidget(SplashMessageWidget::Create(SPLASH_TYPE_ERROR, 500, FONT_MAIN, L"Incompatible client version, please update"));
            break;
        case GA_CONNECTION_BANNED:
            sDrawing->AddUIWidget(SplashMessageWidget::Create(SPLASH_TYPE_ERROR, 500, FONT_MAIN, L"Your account has been banned!"));
            break;
    }
}
