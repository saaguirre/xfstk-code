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
#ifndef XFSTKFACTORY_H
#define XFSTKFACTORY_H
#include "./interfaces/ifactory.h"


#include "./platforms/baytrail/baytraildevice.h"
#include "./platforms/baytrail/baytrailoptions.h"
#include "./platforms/baytrail/baytraildownloader.h"

#include "./platforms/medfield/medfielddevice.h"
#include "./platforms/medfield/medfieldemmcdevice.h"
#include "./platforms/medfield/medfieldoptions.h"
#include "./platforms/medfield/medfielddownloader.h"

#include "./platforms/merrifield/merrifielddevice.h"
#include "./platforms/merrifield/merrifieldoptions.h"
#include "./platforms/merrifield/merrifielddownloader.h"

#include "./platforms/cloverview/cloverviewdevice.h"
#include "./platforms/cloverview/cloverviewdownloader.h"
#include "./platforms/cloverview/cloverviewoptions.h"

#include "./platforms/cloverviewplus/cloverviewplusdevice.h"
#include "./platforms/cloverviewplus/cloverviewplusemmcdevice.h"
#include "./platforms/cloverviewplus/cloverviewplusdownloader.h"
#include "./platforms/cloverviewplus/cloverviewplusoptions.h"

#include "./platforms/hashverify/hashverifydownloader.h"
#include "./platforms/hashverify/hashverifyoptions.h"
#include "./platforms/emmc/emmcoptions.h"
#include "./platforms/emmc/emmcdownloader.h"

#define MOORESTOWN          0x1
#define MEDFIELD            0x2
#define CLOVERVIEW          0x3
#define CLOVERVIEWPLUS      0x4
#define MERRIFIELD          0x5
#define EMMC_DUMP           0x6
#define HASHVERIFY          0x7
#define MOOREFIELD          0x8
#define BAYTRAIL            0x9
#define CARBONCANYON        0xA
#define XFSTK_MAXDEVICE     0xB
#define XFSTK_NODEVICE      0x666

class XfstkFactory : public IFactory
{
public:
    IGenericDevice* CreateDevice(unsigned long ProductId);
    IDownloader* CreateDownloader(unsigned long ProductId, unsigned long Device=0);
    IOptions* CreateDownloaderOptions(unsigned long ProductId);
};

#endif // XFSTKFACTORY_H
