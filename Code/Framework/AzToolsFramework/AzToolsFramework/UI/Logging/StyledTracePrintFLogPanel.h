/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#if !defined(Q_MOC_RUN)
#include "StyledLogPanel.h"
#include "LogLine.h"

#include <AzCore/std/containers/queue.h>
#include <AzCore/Debug/TraceMessageBus.h>
#endif

namespace AzToolsFramework
{
    namespace LogPanel
    {
        class LogTraceRequests : public AZ::EBusTraits
        {
        public:

            using MutexType = AZStd::recursive_mutex;

            virtual void LogTraceMessage(Logging::LogLine::LogType type, const char* window, const char* message, bool alwaysShowMessage = false) = 0;

            using Bus = AZ::EBus<LogTraceRequests>;
        };

        class StyledTracePrintFLogPanel
            : public StyledLogPanel
        {
            Q_OBJECT;
        public:
            AZ_CLASS_ALLOCATOR(StyledTracePrintFLogPanel, AZ::SystemAllocator, 0);

            StyledTracePrintFLogPanel(QWidget* pParent = nullptr);

        private:
            QWidget* CreateTab(const TabSettings& settings) override;
        };

        class StyledTracePrintFLogTab
            : public StyledLogTab
            , protected AZ::Debug::TraceMessageBus::Handler
            , protected LogTraceRequests::Bus::Handler
        {
            Q_OBJECT;
        public:
            AZ_CLASS_ALLOCATOR(StyledTracePrintFLogTab, AZ::SystemAllocator, 0);
            StyledTracePrintFLogTab(const TabSettings& in_settings, QWidget* parent = nullptr);
            virtual ~StyledTracePrintFLogTab();

            //////////////////////////////////////////////////////////////////////////
            // TraceMessagesBus
            bool OnAssert(const char* message) override;
            bool OnException(const char* message) override;
            bool OnError(const char* window, const char* message) override;
            bool OnWarning(const char* window, const char* message) override;
            bool OnPrintf(const char* window, const char* message) override;
            bool OnOutput(const char* window, const char* message) override;
            //bool OnPreError(const char* /*window*/, const char* /*fileName*/, int /*line*/, const char* /*func*/, const char* /*message*/) override;
            //bool OnPreWarning(const char* /*window*/, const char* /*fileName*/, int /*line*/, const char* /*func*/, const char* /*message*/) override;

            //////////////////////////////////////////////////////////////////////////

        protected:

            // Log a message received from the TraceMessageBus
            void LogTraceMessage(Logging::LogLine::LogType type, const char* window, const char* message, bool alwaysShowMessage = false) override;

            // note that we actually buffer the lines up since we could receive them at any time from this bus, even on another thread
            // so we dont push them to the GUI immediately.  Instead we connect to the tickbus and drain the queue on tick.

            AZStd::queue<Logging::LogLine> m_bufferedLines;
            AZStd::atomic_bool m_alreadyQueuedDrainMessage;  // we also only drain the queue at the end so that we do bulk inserts instead of one at a time.
            AZStd::mutex m_bufferedLinesMutex; // protects m_bufferedLines from draining and adding entries into it from different threads at the same time 

        private Q_SLOTS:
            void DrainMessages();
            void ScheduleDrainMessages();

        public slots:
            virtual void Clear();
        };

    } // namespace LogPanel
} // namespace AzToolsFramework
