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
#ifndef SOFTFUSESMESSAGES_H
#define SOFTFUSESMESSAGES_H

#define D0_FW_UPDATE_PROFILE_HDR_SIZE           0x24
#define C0_FW_UPDATE_PROFILE_HDR_SIZE           0x20
#define FW_UPDATE_PROFILE_OLD_HDR_SIZE_SF          0x10

#define BULK_ACK_READY_SFUSE_UPH_SIZE      0x525355504853ULL           // 'RSUPHS'
#define BULK_ACK_READY_SFUSE_UPH           0x5253555048ULL             // 'RSUPH'
#define BULK_ACK_DSKF                      0x44534B46ULL               // 'DSKF'

#define BULK_ACK_HLT0		        0x484C5430ULL        // 'HLT0'
#define BULK_ACK_ER30		        0x45523330ULL        // 'Signature Error'
#define BULK_ACK_ER32		        0x45523332ULL        // 'Sum Mismatch Error'
#define BULK_ACK_ER14               0x45523134ULL        // 'Bulk Unknown Hash Verification Failure

#define CDPH_HEADER_SIZE                        0x200
#define KEY_2_HASH                              0x20
#define KEY_3_HASH                              0x20
#define KEY_4_HASH                              0x20
#define SFUSE_HEADER_SIZE                       0x120
#define VRL_HEADER_SIZE_SF                      0x1E0

#endif // SOFTFUSESMESSAGES_H
