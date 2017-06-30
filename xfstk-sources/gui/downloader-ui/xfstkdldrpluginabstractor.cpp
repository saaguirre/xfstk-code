#include "xfstkdldrpluginabstractor.h"

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

XfstkDldrPluginAbstractor::XfstkDldrPluginAbstractor(QObject *parent) :
    QObject(parent)
{
    this->CurrentPlugin = NULL;
    this->AvailablePlugins.clear();
}
XfstkDldrPluginAbstractor::~XfstkDldrPluginAbstractor()
{
    this->AvailablePlugins.clear();
}
bool XfstkDldrPluginAbstractor::InitializeAllPluginInterfaces()
{
    return true;
}

bool XfstkDldrPluginAbstractor::RestoreAllPluginInterfaceState()
{
    XfstkDldrPluginInterface *CurrentPluginToLoad = NULL;
    for(int i = 0; i < this->AvailablePlugins.length(); i++) {
        CurrentPluginToLoad = NULL;
        CurrentPluginToLoad = this->AvailablePlugins.at(i)->PluginInterface;
        if(CurrentPluginToLoad) {
            CurrentPluginToLoad->RestoreTabInterfaceSettings();
        }
    }
    return true;
}
        
bool XfstkDldrPluginAbstractor::SaveAllPluginInterfaceState()
{
    XfstkDldrPluginInterface *CurrentPluginToLoad = NULL;
    for(int i = 0; i < this->AvailablePlugins.length(); i++) {
        CurrentPluginToLoad = NULL;
        CurrentPluginToLoad = this->AvailablePlugins.at(i)->PluginInterface;
        if(CurrentPluginToLoad) {
            CurrentPluginToLoad->SaveTabInterfaceSettings();
        }
    }
    return true;
}

bool XfstkDldrPluginAbstractor::AddAllPluginUserInterfacesToMainUi(QTabWidget *MainUserInterface)
{
    XfstkDldrPluginInterface *CurrentPluginToLoad = NULL;
    for(int i = 0; i < this->AvailablePlugins.length(); i++) {
        CurrentPluginToLoad = NULL;
        CurrentPluginToLoad = this->AvailablePlugins.at(i)->PluginInterface;
        if(CurrentPluginToLoad) {
            QWidget *DynamicTab = CurrentPluginToLoad->GetUserTabInterface()->currentWidget();
            int DynamicTabCurrentIndex = CurrentPluginToLoad->GetUserTabInterface()->currentIndex();
            QString DynamicTabText = CurrentPluginToLoad->GetUserTabInterface()->tabText(DynamicTabCurrentIndex);
            MainUserInterface->addTab(DynamicTab,DynamicTabText);
        }
    }

    return true;
}

bool XfstkDldrPluginAbstractor::AddAllPluginOptionInterfacesToMainUi(QTabWidget *MainOptionInterface)
{
    XfstkDldrPluginInterface *CurrentPluginToLoad = NULL;
    for(int i = 0; i < this->AvailablePlugins.length(); i++) {
        CurrentPluginToLoad = NULL;
        CurrentPluginToLoad = this->AvailablePlugins.at(i)->PluginInterface;
        if(CurrentPluginToLoad) {
            QWidget *DynamicTab = CurrentPluginToLoad->GetOptionsTabInterface()->currentWidget();
            int DynamicTabCurrentIndex = CurrentPluginToLoad->GetOptionsTabInterface()->currentIndex();
            QString DynamicTabText = CurrentPluginToLoad->GetOptionsTabInterface()->tabText(DynamicTabCurrentIndex);
            MainOptionInterface->addTab(DynamicTab,DynamicTabText);
        }
    }

    if(MainOptionInterface->count() > 0) {
        MainOptionInterface->setCurrentIndex(0);
        for(int j = 1; j < MainOptionInterface->count();j++) {
                MainOptionInterface->setTabEnabled(j,false);
        }
    }
    return true;
}


bool XfstkDldrPluginAbstractor::EnumeratePlugins(QString PluginPath)
{
    XfstkDldrPluginInfo *TmpPluginInfo = NULL;
    XfstkDldrPluginInterface *CurrentPluginToLoad = NULL;
    bool returnval = true;
    QDir pluginsDir(PluginPath);

    if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
        pluginsDir.cdUp();

    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();

        if (plugin) {
            CurrentPluginToLoad = qobject_cast<XfstkDldrPluginInterface *>(plugin);
            if (CurrentPluginToLoad) {
                TmpPluginInfo = NULL;
                TmpPluginInfo = CurrentPluginToLoad->GetPluginInfo();
                if(TmpPluginInfo) {
                    returnval = returnval;
                    this->AvailablePlugins.insert(TmpPluginInfo->PluginTabOrder, TmpPluginInfo);
                }
                else {
                    returnval = false;
                }
            }
            else {
                returnval = false;
            }
        }
        else {
            returnval = false;
        }
    }
    return returnval;
}
