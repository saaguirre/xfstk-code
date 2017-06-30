#ifndef XFSTKDLDRGUI_H
#define XFSTKDLDRGUI_H

#include <QMainWindow>
#include <QDir>
#include <QPluginLoader>
#include <QDesktopServices>
#include <QUrl>
#include "XfstkDldrPluginInterface.h"
#include "xfstkdldrpluginabstractor.h"
#include "xfstksettingsdialog.h"
#include "xfstkaboutdialog.h"
#include <QSettings>
#include <QDesktopWidget>

/*
    Copyright (C) 2015  Intel Corporation

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

namespace Ui {
    class XfstkDldrGui;
}

class XfstkDldrGui : public QMainWindow {
    Q_OBJECT
public:
    XfstkDldrGui(QWidget *parent = 0);
    ~XfstkDldrGui();
    Ui::XfstkDldrGui *ui;

public slots:
    void WhenModifySettingsMenuItemTriggers();
    void WhenDocumentsMenuItemTriggers();
    void WhenAboutMenuItemTriggers();

protected:
    void changeEvent(QEvent *e);

private:
    bool LoadOptionsInterfaces();
    bool LoadUserInterfaces();
    bool LoadDldrPlugins();
    bool InitActionsSignalsSlots();
    void LoadGenericAppState();
    void SaveGenericAppState();
    XfstkDldrPluginAbstractor PluginAbstractor;
    XfstkSettingsDialog SettingsDialog;
    xfstkaboutdialog *AboutDialog;
    QString XfstkVersion;

};

#endif // XFSTKDLDRGUI_H
