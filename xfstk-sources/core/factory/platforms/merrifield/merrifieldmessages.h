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
#ifndef MERRIFIELDMESSAGES_H
#define MERRIFIELDMESSAGES_H

#define MAX_ERR_MSG                 200
#define MFDLIB_VERSION              0x0107
#define MAX_DEVPATH_LENGTH          256
#define MAX_PIPES                   16
#define MAX_DATA_LEN                3996
#define RETRY_ATTEMPTS              2
#define MAX_PKT_SIZE                0x200
#define ONE_K                      (1024)
#define TWO_K                      (1024 * 2)
#define ONE28_K                    (1024 * 128)
#define FOUR_MB                    (4 * 1024 * 1024)
#define NINETY_SIX_KB              (1024 * 96)
#define ONE40_KB                   (1024 * 140)
#define ONE41_KB                   (1024 * 141)
#define TWO_HUNDRED_KB             (1024 * 200)
#define SIXTY_FIVE_KB              (1024 * 65)
#define SEVENTY_TWO_KB             (1024 * 72)
#define FIVE_KB                    (1024 * 5)
#define SIXTEEN_KB                 (1024 * 16)
#define ONE_HUNDRED_KB             (1024 * 100)
#define FW_USB_IMAGE_SIZE          0x40800
#define DNX_FW_SIZE_HDR_SIZE       0x18
#define MERRIFIELD_FW_UPDATE_PROFILE_HDR_SIZE    0x90
#define MERRIFIELD_B0_FW_UPDATE_PROFILE_HDR_SIZE 0xA4
#define D0_FW_UPDATE_PROFILE_HDR_SIZE            0x24
#define C0_FW_UPDATE_PROFILE_HDR_SIZE            0x20
#define FW_UPDATE_PROFILE_OLD_HDR_SIZE_MRFD      0x1C
#define VRL_HEADER_SIZE                          0x2D8
#define CDPH_HEADER_SIZE                         0x200
#define CDPH_HEADER_OFFSET                       (1024 * 170)

#define PSFW1_SIZE_OFFSET     0x0C
#define PSFW2_SIZE_OFFSET     0x10
#define SSFW_SIZE_OFFSET      0x14
#define ROM_PATCH_SIZE_OFFSET 0x18

#define PREAMBLE_RETRY_TIMEOUT 86400000 	//24hours default timeout, might be variable in SMIP header later

// opp codes
#define OPP_FW                  0
#define OPP_OS                  1

// FW/OS Preambles
#define PREAMBLE_DNER      0x52456E44ULL      // FW/OS      'DnER'
#define PREAMBLE_IDRQ      0x51524449ULL      // FW/OS      'IDRQ'
// New BCH preamble
#define PREAMBLE_BMRQ      0x51524D42ULL      // BMRQ       'BMRQ'


// BATI battery status preambles
#define PREAMBLE_DBDS      0x53444244ULL      // BATI       'DBDS'
#define PREAMBLE_RRBD      0x44425252ULL      // BATI       'RRBD

// BATI signature code
#define BATI_SIGNATURE     0x42415449ULL      // BATI       'BATI'

// Serial Start
#define SERIAL_START       0x536F5478ULL      // Serial     'SoTx'

// EMMC
#define BULK_ACK_RDY$               0x52445924ULL        // 'RDY$'
#define EMMC_DUMP_ACK               0x2441434BULL        // '$ACK'
#define EMMC_DUMP_NACK              0x4E41434BULL        // 'NACK'
#define EMMC_DUMP_ECSD              0x45435344ULL        // 'ECSD'
#define EMMC_DUMP_SPR$              0x53505224ULL        // 'SPR$'
#define EMMC_DUMP_REQB              0x52455142ULL        // 'REQB'
#define EMMC_DUMP_EOIO              0x454F494FULL        // 'EOIO'
#define EMMC_DUMP_NOSN              0x4E4F534EULL        // 'NOSN'


// FW Upgrade Ack values
#define BULK_ACK_DFRM               0x4446524DULL        // 'DFRM'
#define BULK_ACK_DxxM               0x4478784DULL        // 'DxxM'
#define BULK_ACK_DXBL               0x4458424CULL        // 'DXBL'	//Used by OS Recovery too
//#define BULK_ACK_CH00		        0x43483030ULL        // 'CH00'	//MRD CHFI00
#define BULK_ACK_DCFI00             0x444346493030ULL    // 'DCFI00'	//MRD CHFI00
#define BULK_ACK_READY_UPH_SIZE     0x5255504853ULL      // 'RUPHS'
#define BULK_ACK_READY_UPH          0x52555048ULL        // 'RUPH'
#define BULK_ACK_GPP_RESET          0x5245534554ULL      // 'RESET'
//#define BULK_ACK_DMIP		        0x444D4950ULL        // 'DMIP'
#define BULK_ACK_UPDATE_SUCESSFUL   0x484C5424ULL        // 'HLT$'
#define BULK_ACK_MFLD               0x4D464C44ULL        // 'MFLD'
#define BULK_ACK_CLVT               0x434C5654ULL        // 'CLVT'
#define BULK_ACK_RTBD               0x52544244ULL        // 'RTBD'
#define BULK_ACK_DIFWI              0x4449465749ULL      // 'DIFWI

#define BULK_ACK_INVALID_PING       0x45523030ULL        // 'ER00'
#define BULK_ACK_HLT0               0x484C5430ULL        // 'HLT0'
#define BULK_ACK_ER01               0x45523031ULL        // 'ER01'
#define BULK_ACK_ER02               0x45523032ULL        // 'ER02'
#define BULK_ACK_ER03               0x45523033ULL        // 'ER03'
#define BULK_ACK_ER04               0x45523034ULL        // 'ER04'
#define BULK_ACK_ER06               0x45523036ULL        // 'ER06'
#define BULK_ACK_ER07               0x45523037ULL        // 'ER07'
#define BULK_ACK_ER10               0x45523130ULL        // 'ER10'
#define BULK_ACK_ER11               0x45523131ULL        // 'ER11'
#define BULK_ACK_ER12               0x45523132ULL        // 'ER12'
#define BULK_ACK_ER13               0x45523133ULL        // 'ER13'
#define BULK_ACK_ER15               0x45523135ULL        // 'ER15'
#define BULK_ACK_ER16               0x45523136ULL        // 'ER16'
#define BULK_ACK_ER17               0x45523137ULL        // 'ER17'
#define BULK_ACK_ER18               0x45523138ULL        // 'ER18'
#define BULK_ACK_ER20               0x45523230ULL        // 'ER20'
#define BULK_ACK_ER21               0x45523231ULL        // 'ER21'
#define BULK_ACK_ER22               0x45523232ULL        // 'ER22'
#define BULK_ACK_ER25               0x45523235ULL        // 'ER25'
#define BULK_ACK_ER26               0x45523236ULL        // 'ER26'
#define BULK_ACK_ER40               0x45523430ULL        //ER40 � Initialize eMMC HW and Card failed
#define BULK_ACK_ER41               0x45523431ULL        //ER41 � Error response to �REQB� in case failure reading from eMMC or invalid block size
#define BULK_ACK_ER42               0x45523432ULL        //ER42 � Error response to �SPR$� in case partition access is not allowed
#define BULK_ACK_ER43               0x45523433ULL        //ER43 � Error response to �SPR$� in case access token mismatched
#define BULK_ACK_ER44               0x45523434ULL        //ER44 � Error response to �SPR$� in case access token has expired
#define BULK_ACK_ER45               0x45523435ULL        //ER45 � Error response to �SPR$� in case failure during eMMC partition switching
#define BULK_ACK_ER46               0x45523436ULL        //ER46 � Error response to �ECSD� in case failure reading ext CSD from eMMC

//PMIC Provisioning errors and messages
#define BULK_ACK_ER90               0x45523930ULL        //ER90
#define BULK_ACK_ER91               0x45523931ULL        //ER91
#define BULK_ACK_ER92               0x45523932ULL        //ER92
#define BULK_ACK_ER93               0x45523933ULL        //ER93
#define BULK_ACK_ER94               0x45523934ULL        //ER94
#define BULK_ACK_ER95               0x45523935ULL        //ER95
#define BULK_ACK_ER96               0x45523936ULL        //ER96
#define BULK_ACK_ER97               0x45523937ULL        //ER97
#define BULK_ACK_ER98               0x45523938ULL        //ER98
#define BULK_ACK_ER99               0x45523939ULL        //ER99
#define BULK_ACK_ER9A               0x45523941ULL        //ER9A
#define BULK_ACK_ER9B               0x45523942ULL        //ER9B
#define BULK_ACK_PP90               0x50503930ULL        //PP90
#define BULK_ACK_PP91               0x50503931ULL        //PP91
#define BULK_ACK_PPOK               0x50504F4BULL        //PPOK


#define BULK_ACK_ERRR               0x45525252ULL        // 'ERRR'
#define BULK_ACK_ERB0               0x45524230ULL        // 'ERB0'
#define BULK_ACK_ERB1               0x45524231ULL        // 'ERB1'

// OS Recovery Ack values
#define BULK_ACK_DORM               0x444F524DULL        // 'DORM'
#define BULK_ACK_OSIPSZ             0x4F53495020537AULL  // 'OSIP Sz'
#define BULK_ACK_ROSIP              0x524F534950ULL      // 'ROSIP'
#define BULK_ACK_DONE               0x444F4E45ULL        // 'DONE'
#define BULK_ACK_RIMG               0x52494D47ULL        // 'RIMG'
#define BULK_ACK_EOIU               0x454F4955ULL        // 'EOIU'

#define BULK_ACK_DCSDB              0x4443534442ULL
#define BULK_ACK_UCSDB              0x5543534442ULL

#define MAX_ACK_CODE_MERRIFIIELD    56

#define MAX_ERROR_CODE_MERRIFIIELD  59
#define OSIP_PARTITIONTABLE_SIZE    0x200

// OS Recovery HOST Commands
#define H_SEND_OSIP_SIZE    0x01    // -- Sending OSIP Size
#define H_REQ_XFER_SIZE     0x02    // -- Requesting size of transfer
#define H_XFER_COMPLETE     0x03    // --  Transfer Complete

// OSIP size offset
#define OSIP_SIZE_OFFSET    0x04
// OSIP HEADER SIZE FILED WIDTH
#define OSIP_HDR_FLD_WTH    0x01
// OSIP Number of Pointers filed offset
#define OSIP_NUM_POINTERS_OFFSET    0x08
// OS Size offsets
#define GET_OS_N_SIZE_OFFSET(n) ((n * 0x18) + 0x30)
// host side usb error when reading in pipe
#define ERROR_READING_FOR_ACK   0x21
// cmd/status pkt size
#define CMD_STAT_PKT_SIZE   0x08
//DFRM op code
#define DFRM 0x1000
//DORM Op Code
#define DxxM 0x2000
//DxxM Op Code
#define DORM 0x3000
//ERRR Op Code
#define ERRR 0x4000
#endif // MERRIFIELDMESSAGES_H
