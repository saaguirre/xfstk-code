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
#include "xfstkfactory.h"

IDownloader* XfstkFactory::CreateDownloader(unsigned long ProductId, unsigned long Device)
{
    IDownloader* TargetDownloaderInterface = NULL;
    switch(ProductId) {
    case MOORESTOWN:
        TargetDownloaderInterface = NULL;
        break;
    case MEDFIELD:
        TargetDownloaderInterface = (IDownloader*) (new MedfieldDownloader());
        break;
    case CLOVERVIEW:
        TargetDownloaderInterface = (IDownloader*) (new CloverviewDownloader());
        break;
    case CLOVERVIEWPLUS:
        TargetDownloaderInterface = (IDownloader*) (new CloverviewPlusDownloader());
        break;
    case MERRIFIELD:
        TargetDownloaderInterface = (IDownloader*) (new MerrifieldDownloader());
        break;
    case HASHVERIFY:
        TargetDownloaderInterface = (IDownloader*) (new HashVerifyDownloader(Device));
        break;
    case EMMC_DUMP:
        TargetDownloaderInterface = (IDownloader*) (new EmmcDownloader(Device));
        break;
    case MOOREFIELD:
        TargetDownloaderInterface = (IDownloader*) (new MerrifieldDownloader());
        break;
    case BAYTRAIL:
        TargetDownloaderInterface = (IDownloader*) (new BaytrailDownloader());
        break;
    case CARBONCANYON:
        TargetDownloaderInterface = (IDownloader*) (new MerrifieldDownloader());
        break;
    case XFSTK_NODEVICE:
        TargetDownloaderInterface = NULL;
        break;
    default:
        break;
    }

    return TargetDownloaderInterface;
}

IOptions* XfstkFactory::CreateDownloaderOptions(unsigned long ProductId)
{
    IOptions* TargetOptionsInterface = NULL;
    switch(ProductId) {
    case MOORESTOWN:
        TargetOptionsInterface = NULL;
        break;
    case MEDFIELD:
        TargetOptionsInterface = (IOptions*) (new MedfieldOptions());
        break;
    case CLOVERVIEW:
        TargetOptionsInterface = (IOptions*) (new CloverviewOptions());
        break;
    case CLOVERVIEWPLUS:
        TargetOptionsInterface = (IOptions*) (new CloverviewPlusOptions());
        break;
    case MERRIFIELD:
        TargetOptionsInterface = (IOptions*) (new MerrifieldOptions());
        break;
    case HASHVERIFY:
        TargetOptionsInterface = (IOptions*) (new HashVerifyOptions());
        break;
    case EMMC_DUMP:
        TargetOptionsInterface = (IOptions*) (new EmmcOptions());
        break;
    case MOOREFIELD:
        TargetOptionsInterface = (IOptions*) (new MerrifieldOptions());
        break;
    case BAYTRAIL:
       TargetOptionsInterface = (IOptions*) (new BaytrailOptions());
       break;
    case CARBONCANYON:
        TargetOptionsInterface = (IOptions*) (new MerrifieldOptions());
        break;
    case XFSTK_NODEVICE:
        TargetOptionsInterface = NULL;
        break;
    default:
        break;
    }

    return TargetOptionsInterface;
}

IGenericDevice* XfstkFactory::CreateDevice(unsigned long ProductId)
{
    IGenericDevice* TargetDeviceInterface = NULL;
    switch(ProductId) {
    case MOORESTOWN:
        TargetDeviceInterface = NULL;
        break;
    case MEDFIELD:
        TargetDeviceInterface = (IGenericDevice*) (new MedfieldDevice());
        break;
    case CLOVERVIEW:
        TargetDeviceInterface = (IGenericDevice*) (new CloverviewDevice());
        break;
    case CLOVERVIEWPLUS:
        TargetDeviceInterface = (IGenericDevice*) (new CloverviewPlusDevice());
        break;
    case MERRIFIELD:
        TargetDeviceInterface = (IGenericDevice*) (new MerrifieldDevice(ProductId));
        break;
    case MOOREFIELD:
        TargetDeviceInterface = (IGenericDevice*) (new MerrifieldDevice(ProductId));
        break;
    case BAYTRAIL:
       TargetDeviceInterface = (IGenericDevice*) (new BaytrailDevice());
       break;
    case CARBONCANYON:
        TargetDeviceInterface = (IGenericDevice*) (new MerrifieldDevice(ProductId));
        break;
    case XFSTK_NODEVICE:
        TargetDeviceInterface = NULL;
        break;
    default:
        break;
    }
    return TargetDeviceInterface;
}
