/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#if !defined(Q_MOC_RUN)
#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/UserSettings/UserSettings.h>
//#include <AzToolsFramework/Log/LogBus.h>
#include <AzToolsFramework/UI/Logging/LogLine.h>
#include <QWidget>
#endif

#pragma optimize("", off)

namespace Ui
{
    class LogWindowClass;
}

class LogCache
    : protected AZ::Debug::TraceMessageBus::Handler
{
public:

    struct LogEntry
    {
        AzToolsFramework::Logging::LogLine::LogType m_messageType;
        AZStd::string m_window;
        AZStd::string m_message;

        LogEntry(AzToolsFramework::Logging::LogLine::LogType messageType, const char* window, const char* message)
            : m_messageType(messageType)
            , m_window(window)
            , m_message(message)
        {}
    };

    LogCache()
    {
        AZ::Debug::TraceMessageBus::Handler::BusConnect();
    }

    ~LogCache()
    {
        AZ::Debug::TraceMessageBus::Handler::BusDisconnect();
    }

    //////////////////////////////////////////////////////////////////////////
    // TraceMessagesBus
    bool OnAssert(const char* message) override
    {
        m_cache.push(LogEntry(AzToolsFramework::Logging::LogLine::LogType::TYPE_ERROR, "ASSERT", message));
        return false;
    }

    bool OnException(const char* message) override
    {
        m_cache.push(LogEntry(AzToolsFramework::Logging::LogLine::LogType::TYPE_ERROR, "EXCEPTION", message));
        return false;
    }

    bool Enqueue(AzToolsFramework::Logging::LogLine::LogType messageType, const char* window, const char* message)
    {
        m_cache.push(LogEntry(messageType, window, message));
        return false;
    }

    bool OnError(const char* window, const char* message) override
    {
        return Enqueue(AzToolsFramework::Logging::LogLine::LogType::TYPE_ERROR, window, message);
    }

    bool OnWarning(const char* window, const char* message) override
    {
        return Enqueue(AzToolsFramework::Logging::LogLine::LogType::TYPE_WARNING, window, message);
    }

    bool OnPrintf(const char* window, const char* message) override
    {
        return Enqueue(AzToolsFramework::Logging::LogLine::LogType::TYPE_MESSAGE, window, message);
    }

    bool OnOutput(const char* window, const char* message) override
    {
        return Enqueue(AzToolsFramework::Logging::LogLine::LogType::TYPE_MESSAGE, window, message);
    }

    bool OnPreWarning(const char* window, const char* /*fileName*/, int /*line*/, const char* /*func*/, const char* message) override
    {
        return Enqueue(AzToolsFramework::Logging::LogLine::LogType::TYPE_WARNING, window, message);
    }

    bool OnPreError(const char* window, const char* /*fileName*/, int /*line*/, const char* /*func*/, const char* message) override
    {
        return Enqueue(AzToolsFramework::Logging::LogLine::LogType::TYPE_ERROR, window, message);
    }
    /////////////////////////////////////////////////////////////////////////

    AZStd::queue<LogEntry> m_cache;
};

/**
* Window pane wrapper for the Asset Editor widget.
*/
class LogWindow
    : public QWidget
    //, AzToolsFramework::Log::LogWidgetRequestsBus::Handler
{
    Q_OBJECT

public:
    AZ_CLASS_ALLOCATOR(LogWindow, AZ::SystemAllocator, 0);

    explicit LogWindow(QWidget* parent = nullptr);
    ~LogWindow() override;

    //////////////////////////////////////////////////////////////////////////
    // LogWindow
    //////////////////////////////////////////////////////////////////////////
    //void CreateAsset(const AZ::Data::AssetType& assetType) override;
    //void OpenAsset(const AZ::Data::Asset<AZ::Data::AssetData>& asset) override;
    //void OpenAssetById(const AZ::Data::AssetId assetId) override;
    //void SaveAssetAs(const AZStd::string_view assetPath) override;

    static void RegisterViewClass();
    static LogCache& HookUpCache()
    {
        static LogCache logCache;
        return logCache;
    }
    //static void RegisterViewClass(const AZ::Data::Asset<AZ::Data::AssetData>& asset);

    void UnrollCache();

    AZStd::unique_ptr<Ui::LogWindowClass> m_ui;


protected Q_SLOTS:
    //void OnAssetSaveFailed(const AZStd::string& error);
    //void OnAssetOpened(const AZ::Data::Asset<AZ::Data::AssetData>& asset);

    void closeEvent(QCloseEvent* event) override;


};


#pragma optimize("", on)
