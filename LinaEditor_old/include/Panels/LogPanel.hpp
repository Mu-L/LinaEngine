/*
Class: LogPanel

Responsible for displaying logs generated via macros in Log.hpp.

Timestamp: 6/7/2020 8:56:39 PM
*/
#pragma once

#ifndef LogPanel_HPP
#define LogPanel_HPP

#include "EventSystem/ApplicationEvents.hpp"
#include "Log/Log.hpp"
#include "Panels/EditorPanel.hpp"
#include "imgui/imgui.h"

#include "Data/Deque.hpp"

namespace Lina::Editor
{
#define LOGPANEL_ICONSENABLED      false
#define LOGPANEL_COLOR_ICONDEFAULT ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
#define LOGPANEL_COLOR_ICONHOVERED ImVec4(1.0f, 1.0f, 1.0f, 0.8f);
#define LOGPANEL_COLOR_ICONPRESSED ImVec4(1.0f, 1.0f, 1.0f, .4f);

#define LOGPANEL_COLOR_DEBUG_DEFAULT ImVec4(0.0f, 0.6f, 0.0f, 1.0f)
#define LOGPANEL_COLOR_DEBUG_HOVERED ImVec4(0.0f, 0.8f, 0.0f, 1.0f)
#define LOGPANEL_COLOR_DEBUG_PRESSED ImVec4(0.0f, 0.4f, 0.0f, 1.0f)

#define LOGPANEL_COLOR_INFO_DEFAULT ImVec4(0.8f, 0.8f, 0.8f, 1.0f)
#define LOGPANEL_COLOR_INFO_HOVERED ImVec4(0.9f, 0.9f, 0.9f, 1.0f)
#define LOGPANEL_COLOR_INFO_PRESSED ImVec4(1.0f, 1.0f, 1.0f, 1.0f)

#define LOGPANEL_COLOR_TRACE_DEFAULT ImVec4(0.6f, 0.6f, 0.8f, 1.0f)
#define LOGPANEL_COLOR_TRACE_HOVERED ImVec4(0.0f, 0.6f, 1.0f, 1.0f)
#define LOGPANEL_COLOR_TRACE_PRESSED ImVec4(0.0f, 0.2f, 0.4f, 1.0f)

#define LOGPANEL_COLOR_WARN_DEFAULT ImVec4(0.6f, 0.6f, 0.0f, 1.0f)
#define LOGPANEL_COLOR_WARN_HOVERED ImVec4(0.8f, 0.8f, 0.0f, 1.0f)
#define LOGPANEL_COLOR_WARN_PRESSED ImVec4(0.4f, 0.4f, 0.0f, 1.0f)

#define LOGPANEL_COLOR_ERR_DEFAULT ImVec4(0.8f, 0.0f, 0.0f, 1.0f)
#define LOGPANEL_COLOR_ERR_HOVERED ImVec4(1.0f, 0.0f, 0.0f, 1.0f)
#define LOGPANEL_COLOR_ERR_PRESSED ImVec4(0.6f, 0.0f, 0.0f, 1.0f)

#define LOGPANEL_COLOR_CRIT_DEFAULT ImVec4(0.0f, 0.0f, 0.0f, 1.0f)
#define LOGPANEL_COLOR_CRIT_HOVERED ImVec4(0.3f, 0.3f, 0.3f, 1.0f)
#define LOGPANEL_COLOR_CRIT_PRESSED ImVec4(0.0f, 0.0f, 0.0f, 1.0f)

    class LogLevelIconButton
    {
    public:
        LogLevelIconButton(const char* id, const char* tooltip, const char* icon, unsigned int targetLevel, ImVec4 colorDefault, ImVec4 colorHovered, ImVec4 colorPressed)
            : m_id(id), m_tooltip(tooltip), m_icon(icon), m_targetLevel(targetLevel), m_colorDefault(colorDefault), m_colorHovered(colorHovered), m_colorPressed(colorPressed){};

        void UpdateColors(unsigned int* flags);
        void DrawButton(unsigned int* flags);

    private:
        const char*  m_id               = "";
        const char*  m_tooltip          = "";
        const char*  m_icon             = "";
        unsigned int m_targetLevel      = 0;
        ImVec4       m_colorDefault     = ImVec4(0, 0, 0, 0);
        ImVec4       m_colorHovered     = ImVec4(0, 0, 0, 0);
        ImVec4       m_colorPressed     = ImVec4(0, 0, 0, 0);
        ImVec4       m_usedColorDefault = LOGPANEL_COLOR_ICONDEFAULT;
        ImVec4       m_usedColorHovered = LOGPANEL_COLOR_ICONHOVERED;
        ImVec4       m_usedColorPressed = LOGPANEL_COLOR_ICONPRESSED;
    };

    class LogPanel : public EditorPanel
    {
        // Wrapper for displaying log dumps
        struct LogDumpEntry
        {
            LogDumpEntry(const Event::ELog& dump, int count) : m_dump(dump), m_count(count){};
            Event::ELog m_dump;
            int         m_count = 1;
        };

    public:
        LogPanel() = default;
        virtual ~LogPanel();

        virtual void Initialize(const char* id, const char* icon) override;
        virtual void Draw() override;
        void         OnLog(const Event::ELog& dump);

    private:
        unsigned int                    m_logLevelFlags = 0;
        Vector<LogLevelIconButton> m_logLevelIconButtons;
        Deque<LogDumpEntry>        m_logDeque;
    };
} // namespace Lina::Editor

#endif
