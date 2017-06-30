#ifndef XFSTKDLDRPLUGINABSTRACTOR_H
#define XFSTKDLDRPLUGINABSTRACTOR_H

#include <QObject>
#include <QDir>
#include <QPluginLoader>
#include "XfstkDldrPluginInterface.h"

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

class XfstkDldrPluginAbstractor : public QObject
{
Q_OBJECT
public:
    explicit XfstkDldrPluginAbstractor(QObject *parent = 0);
    bool EnumeratePlugins(QString PluginPath);
    bool InitializeAllPluginInterfaces();
    bool RestoreAllPluginInterfaceState();
    bool SaveAllPluginInterfaceState();
    bool AddAllPluginUserInterfacesToMainUi(QTabWidget *MainUserInterface);
    bool AddAllPluginOptionInterfacesToMainUi(QTabWidget *MainOptionInterface);
    QList<XfstkDldrPluginInfo *> AvailablePlugins;
    XfstkDldrPluginInterface *CurrentPlugin;
    ~XfstkDldrPluginAbstractor();

signals:

public slots:

};

#endif // XFSTKDLDRPLUGINABSTRACTOR_H
