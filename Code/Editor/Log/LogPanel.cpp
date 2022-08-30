/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "LogPanel.h"
//#include <native/utilities/ThreadHelper.h>

namespace Logging
{
    static constexpr const char* s_logName = "Console";

    LogPanel::LogPanel(QWidget* parent)
        : AzToolsFramework::LogPanel::StyledTracePrintFLogPanel(parent)
    {

        //AzToolsFramework::LogPanel::TabSettings settings(s_logName,
        //    "",
        //    "",
        //    true,
        //    true,
        //    true,
        //    true);

        //AddLogTab(settings);

        AddLogTab(AzToolsFramework::LogPanel::TabSettings("All", "", "", true, true, true, true));
        AddLogTab(AzToolsFramework::LogPanel::TabSettings("Debug", "", "", false, false, false, true));
        AddLogTab(AzToolsFramework::LogPanel::TabSettings("Messages", "", "", true, false, false, false));
        AddLogTab(AzToolsFramework::LogPanel::TabSettings("Errors", "", "", false, false, true, false));
        AddLogTab(AzToolsFramework::LogPanel::TabSettings("Warnings", "", "", false, true, false, false));

    }

    QWidget* LogPanel::CreateTab(const AzToolsFramework::LogPanel::TabSettings& settings)
    {
        LogTab* logTab = aznew LogTab(settings, this);
        logTab->AddInitialLogMessage();
        return logTab;
    }

    LogTab::LogTab(const AzToolsFramework::LogPanel::TabSettings& settings, QWidget* parent)
        : AzToolsFramework::LogPanel::StyledTracePrintFLogTab(settings, parent)
    {
    }

    void LogTab::AddInitialLogMessage()
    {
        //LogTraceMessage(AzToolsFramework::Logging::LogLine::TYPE_MESSAGE, s_logName, "Started recording logs. To check previous logs please navigate to the logs folder.", true);
    }

    bool LogTab::OnAssert(const char* message)
    {
        //if (AssetProcessor::GetThreadLocalJobId())
        //{
            //return false; // we are in a job thread
        //}

        return AzToolsFramework::LogPanel::StyledTracePrintFLogTab::OnAssert(message);
    }
    bool LogTab::OnException(const char* message)
    {
        //if (AssetProcessor::GetThreadLocalJobId())
       // {
        //    return false; // we are in a job thread
       // }

        return AzToolsFramework::LogPanel::StyledTracePrintFLogTab::OnException(message);
    }

    bool LogTab::OnPrintf(const char* window, const char* message)
    {
        //if (AssetProcessor::GetThreadLocalJobId())
        //{
        //    return false; // we are in a job thread
        //}

        return AzToolsFramework::LogPanel::StyledTracePrintFLogTab::OnPrintf(window, message);
    }

    bool LogTab::OnOutput(const char* window, const char* message)
    {
        return AzToolsFramework::LogPanel::StyledTracePrintFLogTab::OnOutput(window, message);
    }

    bool LogTab::OnError(const char* window, const char* message)
    {
        //if (AssetProcessor::GetThreadLocalJobId())
        //{
        //    return false; // we are in a job thread
        //}

        return AzToolsFramework::LogPanel::StyledTracePrintFLogTab::OnError(window, message);
    }

    bool LogTab::OnWarning(const char* window, const char* message)
    {
        //if (AssetProcessor::GetThreadLocalJobId())
        //{
        //    return false; // we are in a job thread
       // }

        return AzToolsFramework::LogPanel::StyledTracePrintFLogTab::OnWarning(window, message);
    }

}
