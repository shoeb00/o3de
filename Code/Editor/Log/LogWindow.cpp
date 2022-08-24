/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "EditorDefs.h"

#include "LogWindow.h"

// Qt
#include <QMessageBox>

// AzCore
//#include <AzCore/Asset/AssetManager.h>
#include <AzCore/UserSettings/UserSettingsComponent.h>
#include <AzCore/Utils/Utils.h>

// AzToolsFramework
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzToolsFramework/API/ViewPaneOptions.h>

// Editor
#include "LyViewPaneNames.h"

AZ_PUSH_DISABLE_DLL_EXPORT_MEMBER_WARNING
#include <Log/ui_LogWindow.h>
AZ_POP_DISABLE_DLL_EXPORT_MEMBER_WARNING

namespace LogUtils
{
   /* LogWindow* CreateLogWithAsset(const AZ::Data::Asset<AZ::Data::AssetData> assetRef)
    {
        LogWindow* LogWindow = new LogWindow();
        if (auto assetsWindowsToRestore = AZ::UserSettings::CreateFind<AzToolsFramework::Log::LogWindowSettings>(AZ::Crc32(AzToolsFramework::Log::LogWindowSettings::s_name), AZ::UserSettings::CT_GLOBAL))
        {
            assetsWindowsToRestore->m_openAssets.emplace(assetRef);
        }

        AZ::UserSettingsComponentRequestBus::Broadcast(&AZ::UserSettingsComponentRequestBus::Events::Save);

        auto&& loadedAsset = AZ::Data::AssetManager::Instance().GetAsset(assetRef.GetId(), assetRef.GetType(), assetRef.GetAutoLoadBehavior());
        loadedAsset.BlockUntilLoadComplete();
        LogWindow->OpenAsset(loadedAsset);
        return LogWindow;
    }*/
}

LogWindow::LogWindow(QWidget* parent)
    : QWidget(parent)
    , m_ui(new Ui::LogWindowClass())
{
    //using namespace AzToolsFramework::Log;

    m_ui->setupUi(this);

    //connect(m_ui->m_LogWidget, &LogWidget::OnAssetSaveFailedSignal, this, &LogWindow::OnAssetSaveFailed);
    //connect(m_ui->m_LogWidget, &LogWidget::OnAssetOpenedSignal, this, &LogWindow::OnAssetOpened);

    //BusConnect();
}

LogWindow::~LogWindow()
{
    //using namespace AzToolsFramework::Log;
    //BusDisconnect();
}

//void LogWindow::RegisterViewClass(const AZ::Data::Asset<AZ::Data::AssetData>& asset)
//{
//    AzToolsFramework::ViewPaneOptions options;
//    options.showInMenu = false;
//    auto& assetName = asset.GetHint();
//    const char* paneName = assetName.c_str();
//    AzToolsFramework::RegisterViewPane<LogWindow>(paneName, LyViewPane::CategoryTools, options, [asset](QWidget*) {return LogUtils::CreateLogWithAsset(asset); });
//}

//void LogWindow::CreateAsset(const AZ::Data::AssetType& assetType)
//{
//    m_ui->m_LogWidget->CreateAsset(assetType);
//}
//
//void LogWindow::OpenAsset(const AZ::Data::Asset<AZ::Data::AssetData>& asset)
//{
//    m_ui->m_LogWidget->OpenAsset(asset);
//}
//
//void LogWindow::OpenAssetById(const AZ::Data::AssetId assetId)
//{
//    AZ::Data::Asset<AZ::Data::AssetData> asset = AZ::Data::AssetManager::Instance().GetAsset<AZ::Data::AssetData>(assetId, AZ::Data::AssetLoadBehavior::NoLoad);
//    OpenAsset(asset);
//}
//
//void LogWindow::SaveAssetAs(const AZStd::string_view assetPath)
//{
//    if (assetPath.empty())
//    {
//        AZ_Warning("Asset Editor", false, "Could not save asset to empty path.");
//        return;
//    }
//
//    auto absoluteAssetPath = AZ::IO::FixedMaxPath(AZ::Utils::GetEnginePath()) / assetPath;
//
//    if (!m_ui->m_LogWidget->SaveAssetToPath(absoluteAssetPath.Native()))
//    {
//        AZ_Warning("Asset Editor", false, "File was not saved correctly via SaveAssetAs.");
//    }
//}

void LogWindow::RegisterViewClass()
{
    AzToolsFramework::ViewPaneOptions options;
    options.preferedDockingArea = Qt::LeftDockWidgetArea;
    options.showOnToolsToolbar = true;
    options.toolbarIcon = ":/Menu/asset_editor.svg";
    AzToolsFramework::RegisterViewPane<LogWindow>(LyViewPane::LogPanel, LyViewPane::CategoryTools, options);
}

//void LogWindow::OnAssetOpened(const AZ::Data::Asset<AZ::Data::AssetData>& asset)
//{
//    if (asset)
//    {
//        AZStd::string assetPath;
//        AZStd::string assetName;
//        AZStd::string extension;
//        AZ::Data::AssetCatalogRequestBus::BroadcastResult(assetPath, &AZ::Data::AssetCatalogRequests::GetAssetPathById, asset.GetId());
//        AzFramework::StringFunc::Path::Split(assetPath.c_str(), nullptr, nullptr, &assetName, &extension);
////        AZStd::string windowTitle = AZStd::string::format("Edit Asset: %s", (assetName + extension).c_str());
//
//        AZStd::string windowTitle = asset.GetHint();
//
//        qobject_cast<QWidget*>(parent())->setWindowTitle(tr(windowTitle.c_str()));
//    }
//    else
//    {
//        qobject_cast<QWidget*>(parent())->setWindowTitle(tr("Asset Editor"));
//    }
//}

void LogWindow::closeEvent(QCloseEvent* /*event*/)
{
    //if (m_ui->m_LogWidget->WaitingToSave())
    //{
    //    // Don't need to ask to save, as a save is already queued.
    //    m_ui->m_LogWidget->SetCloseAfterSave();
    //    event->ignore();
    //    return;
    //}

    //if (m_ui->m_LogWidget->TrySave([this]() {  qobject_cast<QWidget*>(parent())->close(); }))
    //{
    //    event->ignore();
    //}
}

//void LogWindow::OnAssetSaveFailed(const AZStd::string& error)
//{
//    QMessageBox::warning(this, tr("Unable to Save Asset"), tr(error.c_str()), QMessageBox::Ok, QMessageBox::Ok);
//}

#include <Log/moc_LogWindow.cpp>
