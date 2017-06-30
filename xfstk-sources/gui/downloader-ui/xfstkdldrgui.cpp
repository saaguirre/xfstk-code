#include "xfstkdldrgui.h"
#include "ui_xfstkdldrgui.h"
#ifndef DOWNLOADER_VERSION
#define DOWNLOADER_VERSION "0.0.0"
#endif

/*
    Copyright (C) 2014  Intel Corporation

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
	
XfstkDldrGui::XfstkDldrGui(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::XfstkDldrGui)
{
    ui->setupUi(this);
    this->XfstkVersion = DOWNLOADER_VERSION;
    this->setWindowTitle("xFSTK Downloader " + this->XfstkVersion);
    this->SettingsDialog.setWindowTitle("xFSTK Downloader " + this->XfstkVersion + " Settings");
    this->AboutDialog = NULL;
    this->AboutDialog = new xfstkaboutdialog();
    this->InitActionsSignalsSlots();
    this->LoadDldrPlugins();
    this->LoadUserInterfaces();
    this->LoadOptionsInterfaces();
    this->PluginAbstractor.RestoreAllPluginInterfaceState();
    this->LoadGenericAppState();
#if defined XFSTK_OS_WIN
#else
    this->ui->actionModify_Settings->setIconVisibleInMenu(true);
    this->ui->actionAbout->setIconVisibleInMenu(true);
    this->ui->actionDocumentation->setIconVisibleInMenu(true);
#endif
}

XfstkDldrGui::~XfstkDldrGui()
{
    this->PluginAbstractor.SaveAllPluginInterfaceState();
    this->SaveGenericAppState();
    delete ui;
}

void XfstkDldrGui::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

bool XfstkDldrGui::InitActionsSignalsSlots()
{
    connect(this->ui->actionModify_Settings, SIGNAL(triggered()), this, SLOT(WhenModifySettingsMenuItemTriggers()));
    connect(this->ui->UserInterfaceTabDock, SIGNAL(currentChanged(int)), &(this->SettingsDialog), SLOT(CurrentTabChanged(int)));
    connect(this->ui->actionDocumentation, SIGNAL(triggered()), this, SLOT(WhenDocumentsMenuItemTriggers()));
    connect(this->ui->actionAbout,SIGNAL(triggered()),this,SLOT(WhenAboutMenuItemTriggers()));

    return true;
}

bool XfstkDldrGui::LoadUserInterfaces()
{
    return this->PluginAbstractor.AddAllPluginUserInterfacesToMainUi(this->ui->UserInterfaceTabDock);
}

bool XfstkDldrGui::LoadOptionsInterfaces()
{
    return this->PluginAbstractor.AddAllPluginOptionInterfacesToMainUi(this->SettingsDialog.GetOptionsInterfaceTabDock());
}

bool XfstkDldrGui::LoadDldrPlugins()
{
#if defined XFSTK_OS_WIN
    QString PluginPath = QString("%1/%2").arg(qApp->applicationDirPath(),QString("xfstkdldrplugins"));
#else
    QString PluginPath = QString("/usr/lib/xfstk/xfstkdldrplugins");
#endif
    return this->PluginAbstractor.EnumeratePlugins(PluginPath);
}

void XfstkDldrGui::WhenModifySettingsMenuItemTriggers()
{
    this->SettingsDialog.setModal(true);
    this->SettingsDialog.show();
}

void XfstkDldrGui::WhenDocumentsMenuItemTriggers()
{
#if defined XFSTK_OS_WIN
    QString help = QString("%1/xfstk-doc/html/index.html").arg(QDir::currentPath());
#else
    QString help = QString("/usr/share/doc/xfstk-downloader/html/index.html");
#endif
    QString urlhelp = QString("file:///").append(help);
    QDesktopServices::openUrl (QUrl(urlhelp, QUrl::TolerantMode));
}

void XfstkDldrGui::WhenAboutMenuItemTriggers()
{
    if(this->AboutDialog) {
        delete this->AboutDialog;
    }
    this->AboutDialog = new xfstkaboutdialog();
    this->AboutDialog->setWindowTitle("About xFSTK Downloader " + this->XfstkVersion);
    this->AboutDialog->show();
}

void XfstkDldrGui::LoadGenericAppState()
{
    QString Vendor = "Intel";
    QString Product = "XFSTK";
    QSettings settings(Vendor, Product);
    QDesktopWidget dt;
    QRect DesktopRect = dt.availableGeometry(dt.primaryScreen());
    QSize AppSize;
    QPoint AppUpperLeftCorner;
    settings.beginGroup("General");
    AppSize = settings.value("size", QSize(600, 400)).toSize();
    AppUpperLeftCorner = settings.value("pos", QPoint(0, 0)).toPoint();
    settings.endGroup();

    if(AppUpperLeftCorner == QPoint(0,0)) {
        int AppRectWidth = size().rwidth();
        int DesktopWidth = DesktopRect.width();
        int DesktopHight = DesktopRect.height();
        int AppUpperLeftX = (DesktopWidth/2) - (AppRectWidth/2);
        int AppUpperLeftY = (DesktopHight/8);
        move(QPoint(AppUpperLeftX,AppUpperLeftY));
    }
    else {
        resize(AppSize);
        move(AppUpperLeftCorner);
    }
}

void XfstkDldrGui::SaveGenericAppState()
{
    QString Vendor = "Intel";
    QString Product = "XFSTK";
    QSettings settings(Vendor, Product);
    settings.beginGroup("General");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();
}
