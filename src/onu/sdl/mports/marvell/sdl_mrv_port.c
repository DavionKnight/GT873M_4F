/****************************************************************************
            Software License for Customer Use of Cortina Software
                          Grant Terms and Conditions

IMPORTANT NOTICE - READ CAREFULLY: This Software License for Customer Use
of Cortina Software ("LICENSE") is the agreement which governs use of
software of Cortina Systems, Inc. and its subsidiaries ("CORTINA"),
including computer software (source code and object code) and associated
printed materials ("SOFTWARE").  The SOFTWARE is protected by copyright laws
and international copyright treaties, as well as other intellectual property
laws and treaties.  The SOFTWARE is not sold, and instead is only licensed
for use, strictly in accordance with this document.  Any hardware sold by
CORTINA is protected by various patents, and is sold but this LICENSE does
not cover that sale, since it may not necessarily be sold as a package with
the SOFTWARE.  This LICENSE sets forth the terms and conditions of the
SOFTWARE LICENSE only.  By downloading, installing, copying, or otherwise
using the SOFTWARE, you agree to be bound by the terms of this LICENSE.
If you do not agree to the terms of this LICENSE, then do not download the
SOFTWARE.

DEFINITIONS:  "DEVICE" means the Cortina Systems? Daytona SDK product.
"You" or "CUSTOMER" means the entity or individual that uses the SOFTWARE.
"SOFTWARE" means the Cortina Systems? SDK software.

GRANT OF LICENSE:  Subject to the restrictions below, CORTINA hereby grants
CUSTOMER a non-exclusive, non-assignable, non-transferable, royalty-free,
perpetual copyright license to (1) install and use the SOFTWARE for
reference only with the DEVICE; and (2) copy the SOFTWARE for your internal
use only for use with the DEVICE.

RESTRICTIONS:  The SOFTWARE must be used solely in conjunction with the
DEVICE and solely with Your own products that incorporate the DEVICE.  You
may not distribute the SOFTWARE to any third party.  You may not modify
the SOFTWARE or make derivatives of the SOFTWARE without assigning any and
all rights in such modifications and derivatives to CORTINA.  You shall not
through incorporation, modification or distribution of the SOFTWARE cause
it to become subject to any open source licenses.  You may not
reverse-assemble, reverse-compile, or otherwise reverse-engineer any
SOFTWARE provided in binary or machine readable form.  You may not
distribute the SOFTWARE to your customers without written permission
from CORTINA.

OWNERSHIP OF SOFTWARE AND COPYRIGHTS. All title and copyrights in and the
SOFTWARE and any accompanying printed materials, and copies of the SOFTWARE,
are owned by CORTINA. The SOFTWARE protected by the copyright laws of the
United States and other countries, and international treaty provisions.
You may not remove any copyright notices from the SOFTWARE.  Except as
otherwise expressly provided, CORTINA grants no express or implied right
under CORTINA patents, copyrights, trademarks, or other intellectual
property rights.

DISCLAIMER OF WARRANTIES. THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY
EXPRESS OR IMPLIED WARRANTY OF ANY KIND, INCLUDING ANY IMPLIED WARRANTIES
OF MERCHANTABILITY, NONINFRINGEMENT, OR FITNESS FOR A PARTICULAR PURPOSE,
TITLE, AND NON-INFRINGEMENT.  CORTINA does not warrant or assume
responsibility for the accuracy or completeness of any information, text,
graphics, links or other items contained within the SOFTWARE.  Without
limiting the foregoing, you are solely responsible for determining and
verifying that the SOFTWARE that you obtain and install is the appropriate
version for your purpose.

LIMITATION OF LIABILITY. IN NO EVENT SHALL CORTINA OR ITS SUPPLIERS BE
LIABLE FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, LOST
PROFITS, BUSINESS INTERRUPTION, OR LOST INFORMATION) OR ANY LOSS ARISING OUT
OF THE USE OF OR INABILITY TO USE OF OR INABILITY TO USE THE SOFTWARE, EVEN
IF CORTINA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
TERMINATION OF THIS LICENSE. This LICENSE will automatically terminate if
You fail to comply with any of the terms and conditions hereof. Upon
termination, You will immediately cease use of the SOFTWARE and destroy all
copies of the SOFTWARE or return all copies of the SOFTWARE in your control
to CORTINA.  IF you commence or participate in any legal proceeding against
CORTINA, then CORTINA may, in its sole discretion, suspend or terminate all
license grants and any other rights provided under this LICENSE during the
pendency of such legal proceedings.
APPLICABLE LAWS. Claims arising under this LICENSE shall be governed by the
laws of the State of California, excluding its principles of conflict of
laws.  The United Nations Convention on Contracts for the International Sale
of Goods is specifically disclaimed.  You shall not export the SOFTWARE
without first obtaining any required export license or other approval from
the applicable governmental entity, if required.  This is the entire
agreement and understanding between You and CORTINA relating to this subject
matter.
GOVERNMENT RESTRICTED RIGHTS. The SOFTWARE is provided with "RESTRICTED
RIGHTS." Use, duplication, or disclosure by the Government is subject to
restrictions as set forth in FAR52.227-14 and DFAR252.227-7013 et seq. or
its successor. Use of the SOFTWARE by the Government constitutes
acknowledgment of CORTINA's proprietary rights therein. Contractor or
Manufacturer is CORTINA.

Copyright (c) 2010 by Cortina Systems Incorporated
****************************************************************************/
#include "plat_common.h"
#include "sdl_port.h"
#include "sdl_qos_cmn.h"
#include "sdl_init.h"
#include "aal_pon.h"
#include "aal_flow.h"
#include "aal_uni.h"
#include "aal_l2.h"
#include "sdl_int.h"
#include "sdl.h"
#include "sdl_event_cmn.h"
#include "make_file.h"

#include "MARVELL_BSP_expo.h"
#include "switch_expo.h"
#include "gtDrvSwRegs.h"
#include "switch_drv.h"

#define __SDL_PORT_AUTO_NEGO_FAIL              0
#define __SDL_PORT_AUTO_NEGO_SUCCESS           1
#define __SDL_AUTO_NEG_FAIL_CHECK_INTVL        500        /*500 ms*/
#define __SDL_AUTO_NEG_FAIL_CHECK_START_INTVL  1000       /*1s*/
#define __SDL_PORT_MTU_MAX                     1552       
#define __PHY_REGISTER_19                      19

#define MIN_BURST_SIZE              10
#define MAX_BURST_SIZE              500
#define SWITCH_UPLINK_PORT_ID       CS_UPLINK_PHY_PORT
#define MIRROR_MAX_PORT_MASK        ((1<<(SWITCH_UPLINK_PORT_ID))-1)
#define PORT_STORM_METER_ID_START   10
#define PORT_STORM_METER_ID_END     14
#define MAX_PORT_NUMBER              4
#define MIN_STORM_RATE              8
#define STORM_GRANULARITY           8

#define BURST_CONTROL_REG 0x1203

extern void cs_polling_handler_reg(void (*handler)());

/*Port mask map from mirror port mask to switch port mask*/
#define MSK_MIRROR_TO_SWITCH(mirror_msk, switch_msk)  do{           \
    if(mirror_msk & 0x10){                                          \
        switch_msk = mirror_msk & (~(0x10));                        \
        switch_msk |= 0x40;                                         \
    }                                                               \
    else{                                                           \
        switch_msk = mirror_msk;                                    \
    }                                                               \
                                                                    \
}while(0)


/*Port mask map from switch port mask to mirror port mask*/
#define MSK_SWITCH_TO_MIRROR(switch_msk, mirror_msk)                \
do{                                                                 \
    if(switch_msk & 0x40 ){                                         \
        mirror_msk = switch_msk & (~(0x40));                        \
        mirror_msk |= 0x10;                                         \
    }                                                               \
    else{                                                           \
        mirror_msk = switch_msk;                                    \
    }                                                               \
                                                                    \
}while(0)


cs_uint32 __pir = 0;
cs_sdl_policy_t __us_rate_limit[UNI_PORT_MAX];
cs_sdl_policy_t __ds_rate_limit[UNI_PORT_MAX];
cs_sdl_policy_t __sc_rate_limit[UNI_PORT_MAX];

cs_boolean __port_isolation = 0;
cs_sdl_storm_ctrl_e __storm_type[UNI_PORT_MAX] = {SDL_STORM_TYPE_DISABLE, SDL_STORM_TYPE_DISABLE, SDL_STORM_TYPE_DISABLE, SDL_STORM_TYPE_DISABLE};

cs_sdl_port_admin_t       __pon_admin;
//cs_pri_remark_t __dot1p_remark[4];
//cs_uint8 __acl_occupy[4][8] = {{0,},};
//cs_uint32 __acl_avl = 0;

#define MAX_PORT_TRAFFIC_RATE sdl_int_cfg.max_uni_rate

cs_sdl_port_loopback_t __pon_loopback_mode = SDL_PORT_LOOPBACK_NONE;
cs_sdl_port_loopback_t __uni_loopback_mode[UNI_PORT_MAX] = {SDL_PORT_LOOPBACK_NONE, SDL_PORT_LOOPBACK_NONE, SDL_PORT_LOOPBACK_NONE, SDL_PORT_LOOPBACK_NONE};
//cs_sdl_uni_loopback_depth_t __uni_loopback_depth = SDL_UNI_MAC_LOOPBACK;

typedef struct {
    cs_sdl_port_speed_cfg_t port_cfg;
    cs_sdl_port_admin_t     uni_admin;
    cs_boolean              flow_ctrl_en; 
}__sdl_port_config_t;


__sdl_port_config_t __port_cfg[UNI_PORT_MAX];

cs_sdl_port_link_status_t __uni_link_ststus[UNI_PORT_MAX] = {SDL_PORT_LINK_STATUS_DOWN, SDL_PORT_LINK_STATUS_DOWN, SDL_PORT_LINK_STATUS_DOWN, SDL_PORT_LINK_STATUS_DOWN};

extern sdl_init_cfg_t sdl_int_cfg;


/*****************************************************************************/
/*                    Local Functions                                        */
/*****************************************************************************/
#ifdef HAVE_SDL_CTC
static void __sdl_port_auto_neg_polling_handler(void)
{
    sdl_event_port_auto_nego_t auto_nego_event;
    GT_U16 phy_data;
    static cs_uint8 counter = 20;
    GT_LPORT port;
    GT_STATUS ret = 0;
    
    if(counter>0){
        counter--;
        return;
    }

    for(port=0; port<UNI_PORT_MAX; port++) {
    	GT_32 unit, hwport;

    	gt_getswitchunitbylport(port, &unit, &hwport);

    	ret = gprtGetSwitchReg(QD_DEV_PTR, hwport, QD_PHY_INT_STATUS_REG, &phy_data);
        if(GT_OK != ret)
           return;
           
        if (phy_data & GT_AUTO_NEG_COMPLETED){
            //cs_printf("port %d, auto nego failed! Reg value: 0x%x\n", P2L_PORT(port), phy_data);
            auto_nego_event.port = P2L_PORT(port);
            auto_nego_event.status = __SDL_PORT_AUTO_NEGO_FAIL;
            sdl_event_send(EPON_EVENT_PORT_AUTO_NEGO_STATUS, sizeof(sdl_event_port_auto_nego_t), &auto_nego_event);
        }
    }
    
    counter = 20;
    
    return;

}
#endif
void __sdl_port_int_process(void)
{
    cs_sdl_port_link_status_t link_status;
    GT_LPORT port;
    sdl_event_port_link_t link_event;
    GT_U16 phy_data;
    GT_U16 portvec = 0, inttype;
    
#ifdef HAVE_SDL_CTC
    sdl_event_port_auto_nego_t auto_nego_event;
#endif

    FOR_UNIT_START(GT_32, unit)

    if(gprtGetPhyIntPortSummary(QD_DEV_PTR,&portvec) == GT_OK &&
    		portvec )
    {
        for (port=0; port <UNI_PORT_MAX; port++) {

        	GT_32 unit, hwport;

        	gt_getswitchunitbylport(port, &unit, &hwport);

        	if(!(portvec & (1<<port)))
        		continue;

        	if(GT_OK != gprtGetPhyIntStatus(QD_DEV_PTR, hwport, &inttype))
        		continue;

        	if(!(inttype & GT_LINK_STATUS_CHANGED))
        		continue;

        	gprtGetSwitchReg(QD_DEV_PTR, hwport, QD_PHY_INT_STATUS_REG, &phy_data);
            
#ifdef HAVE_SDL_CTC
            if (phy_data & GT_LINK_STATUS_CHANGED){
                //cs_printf("port %d, auto nego success! Reg value: 0x%x\n", P2L_PORT(port), phy_data);
                auto_nego_event.port = P2L_PORT(port);
                auto_nego_event.status = __SDL_PORT_AUTO_NEGO_SUCCESS;
                sdl_event_send(EPON_EVENT_PORT_AUTO_NEGO_STATUS, sizeof(sdl_event_port_auto_nego_t), &auto_nego_event);
            }
            else
            {
                //cs_printf("port %d, auto nego fail! Reg value: 0x%x\n", P2L_PORT(port), phy_data);
                auto_nego_event.port = P2L_PORT(port);
                auto_nego_event.status = __SDL_PORT_AUTO_NEGO_FAIL;
                sdl_event_send(EPON_EVENT_PORT_AUTO_NEGO_STATUS, sizeof(sdl_event_port_auto_nego_t), &auto_nego_event);
            }
#endif

            gprtGetLinkState(QD_DEV_PTR, hwport, (GT_BOOL*)&link_status);
            if (link_status == __uni_link_ststus[port]) {
                continue;
            }
            
            __uni_link_ststus[port] = link_status;
            //SDL_MIN_LOG("port %d, %s\n", port + CS_UNI_PORT_ID1, (link_statu == PORT_LINKUP) ? "link up" : "link down");
            link_event.link = link_status;
            link_event.port = P2L_PORT(port);
            sdl_event_send(EPON_EVENT_PORT_LINK_CHANGE, sizeof(sdl_event_port_link_t), &link_event);
        }
    }

    FOR_UNIT_END

    return;

}

/****************************************************************/

cs_status epon_request_onu_port_mtu_get(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_port_id_t              port_id,
    CS_OUT cs_uint16                 *mtu
)
{
    cs_aal_pon_mac_cfg_t cfg;
    cs_uint32            uni_mtu;
    GT_STATUS        ret = 0;
    GT_BOOL			mode;

    if (NULL == mtu) {
        SDL_MIN_LOG("MTU is NULL pointer!\n");
        return CS_E_PARAM;
    }
    
    if (port_id > UNI_PORT_MAX){
        SDL_MIN_LOG("Port ID is Invalid\n");
        return CS_E_PARAM;
    }
    
    if (port_id == CS_PON_PORT_ID) {
            aal_pon_mac_cfg_get(&cfg);
            *mtu = cfg.mtu;
    }
    else{
    	ret = gsysGetMaxFrameSize(QD_MASTER_DEV_PTR,&mode);
        uni_mtu = (mode)?1522:1632;
        if(GT_OK != ret){
            SDL_MIN_LOG("gsysGetMaxFrameSize return %d\n", ret);
            return  CS_E_ERROR;
        }
        
        *mtu = uni_mtu & 0xffff;
    }
        
    return CS_E_OK;

}

cs_status epon_request_onu_port_mtu_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              port_id,
    CS_IN cs_uint16                 mtu
)
{
    cs_aal_pon_mac_cfg_t    cfg;
    cs_aal_pon_mac_msk_t    mask;
    GT_STATUS           ret = 0;
    cs_status               rt = CS_E_OK;
    GT_BOOL				mode;
    
    if (mtu > __SDL_PORT_MTU_MAX) {
        SDL_MIN_LOG(" mtu %d is not supported!\n",mtu);
        return CS_E_PARAM;
    }
    
    if (port_id > CS_UNI_PORT_ID4){
        SDL_MIN_LOG("Port ID is Invalid\n");
        return CS_E_PARAM;
    }
    
    if (port_id == CS_PON_PORT_ID) {
        mask.u32 = 0;
        mask.s.mtu = 1;
        mask.s.tx_max_size = 1;
        mask.s.mtu_tag_en = 1;

        cfg.mtu = mtu;
        cfg.tx_max_size = mtu;
        cfg.mtu_tag_en = sdl_int_cfg.mtu_tag_exclude;
        rt = aal_pon_mac_cfg_set(mask, &cfg);
        if (rt) {
            SDL_MIN_LOG("aal_pon_mac_cfg_set return  %d\n", rt);
            return rt;
        }
    }
    else{
        if (mtu <= 1522) {
            mode = GT_TRUE;
        } 
        else {
            mode = GT_FALSE;
        }

        FOR_UNIT_START(GT_32, unit)
        ret = gsysSetMaxFrameSize(QD_DEV_PTR, mode);
        if (GT_OK != ret) {
            SDL_MIN_LOG("gsysSetMaxFrameSize return %dd\n", ret);
            return CS_E_ERROR;
        }
        FOR_UNIT_END
    }
        
    return CS_E_OK;
    
}


cs_status epon_request_onu_port_speed_get(
    CS_IN cs_callback_context_t         context,
    CS_IN cs_int32                      device_id,
    CS_IN cs_int32                      llidport,
    CS_IN cs_port_id_t                  port_id,
    CS_OUT cs_sdl_port_ether_speed_t    *speed
)
{
    cs_status rc = CS_E_OK;
    GT_BOOL 		status;
    GT_LPORT              port;
    GT_STATUS          ret = 0;
    GT_PORT_SPEED_MODE			lspeed;
    
    GT_32 unit, hwport;

    if (NULL == speed) {
        SDL_MIN_LOG("Speed is NULL pointer!\n");
        return CS_E_PARAM;
    }

    UNI_PORT_CHECK(port_id);
    port = L2P_PORT(port_id);



    /** PHY link status */
    gt_getswitchunitbylport(port, &unit, &hwport);
    ret = gprtGetLinkState(QD_DEV_PTR, hwport, &status);
    if (GT_OK != ret) {
        SDL_MIN_LOG("gprtGetLinkState return %d\n", ret);
        rc=CS_E_ERROR;
        goto end; 
    }

    /** While the PHY link is down, no meaning of the link speed */
    if (! status) {
            *speed = SDL_PORT_SPEED_10;
        goto end;
    }
    
    if(GT_OK == gprtGetSpeedMode(QD_DEV_PTR, hwport, &lspeed))
    {
    	switch(lspeed)
    	{
    	case PORT_SPEED_1000_MBPS:
    		*speed = SDL_PORT_SPEED_1000;
    		break;
    	case PORT_SPEED_100_MBPS:
    		*speed = SDL_PORT_SPEED_100;
    		break;
    	default:
    		*speed = SDL_PORT_SPEED_10;
    		break;
    	}
    }
    else
    	rc = CS_E_ERROR;
    
end:
    
    return rc;
    
}

cs_status epon_request_onu_port_duplex_get(
    CS_IN cs_callback_context_t         context,
    CS_IN cs_int32                      device_id,
    CS_IN cs_int32                      llidport,
    CS_IN cs_port_id_t                  port_id,
    CS_OUT cs_sdl_port_ether_duplex_t   *duplex
)
{
    GT_LPORT             port;
    GT_BOOL				lduplex;
    GT_STATUS          ret  = 0;
    
    GT_32 unit, hwport;

    if (NULL == duplex) {
        SDL_MIN_LOG("duplex is NULL pointer\n" );
        return CS_E_PARAM;
    }
    
    UNI_PORT_CHECK(port_id);
    
    port = L2P_PORT(port_id);

    /** PHY link status */
    gt_getswitchunitbylport(port, &unit, &hwport);
    ret = gprtGetDuplex(QD_DEV_PTR, hwport, &lduplex);
    if (GT_OK != ret) {
        SDL_MIN_LOG("gprtGetC_Duplex return %d\n", ret);
        return CS_E_ERROR;
    }

    if(lduplex)
        *duplex = SDL_PORT_DUPLEX_FULL;
    else
        *duplex = SDL_PORT_DUPLEX_HALF;

    return CS_E_OK;
    
}

cs_status epon_request_onu_port_auto_neg_get(
    CS_IN cs_callback_context_t         context,
    CS_IN cs_int32                      device_id,
    CS_IN cs_int32                      llidport,
    CS_IN cs_port_id_t                  port_id,
    CS_OUT cs_sdl_port_autoneg_status_t *auto_neg
)

{
    cs_sdl_port_speed_cfg_t port_cfg;

    if (NULL == auto_neg) {
        SDL_MIN_LOG("auto_neg is NULL pointer!\n" );
        return CS_E_PARAM;
    }

    UNI_PORT_CHECK(port_id);
    
    port_cfg = __port_cfg[port_id -1].port_cfg;

    if ((port_cfg == SDL_PORT_AUTO_10_100)
        || (port_cfg == SDL_PORT_AUTO_10_100_1000) 
        || (port_cfg == SDL_PORT_1000_FULL)){
        *auto_neg = SDL_PORT_AUTONEG_UP;
    } 
    else {
        *auto_neg = SDL_PORT_AUTONEG_DOWN;
    }

    return CS_E_OK;
}

cs_status epon_request_onu_port_autoneg_restart(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              port_id
)
{
    GT_LPORT      port;
    GT_STATUS   ret  = 0;
    cs_status       rt = CS_E_OK;
    GT_32 unit,hwport;
    
    UNI_PORT_CHECK(port_id);

    port = L2P_PORT(port_id);
    
    gt_getswitchunitbylport(port, &unit, &hwport);
    ret = gprtPortRestartAutoNeg(QD_DEV_PTR, hwport);
    
    if(ret != GT_OK)
    	rt = CS_E_ERROR;

    return rt;
}


cs_status epon_request_onu_port_mdi_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              port_id,
    CS_IN cs_sdl_port_mdi_t         mdi
)
{

    return CS_E_NOT_SUPPORT;
}

cs_status epon_request_onu_port_mdi_get(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              port_id,
    CS_OUT cs_sdl_port_mdi_t        *mdi
)
{
    return CS_E_NOT_SUPPORT;
}
cs_status epon_request_onu_port_lpbk_set(
    CS_IN cs_callback_context_t             context,
    CS_IN cs_int32                          device_id,
    CS_IN cs_int32                          llidport,
    CS_IN cs_port_id_t                      port_id,
    CS_IN cs_sdl_port_loopback_t            loopback
)
{
    GT_LPORT                     port;
    GT_U16                     phyData;
    static cs_sdl_port_speed_cfg_t port_cfg[UNI_PORT_MAX];
    GT_STATUS                  ret  = 0;
    cs_status                      rt = CS_E_OK;
    
    GT_32 unit, hwport;

    if (port_id > UNI_PORT_MAX){
        SDL_MIN_LOG("Port ID is Invalid\n");
        return CS_E_PARAM;
    }
    
    if (port_id == CS_PON_PORT_ID) {
        if (__pon_loopback_mode == loopback) {
            rt =  CS_E_OK;
            goto END;
        }

        switch (loopback) {
            case SDL_PORT_LOOPBACK_NONE: {
                rt = aal_pon_loopback_set(EPON_FALSE);
                if (rt) {
                    SDL_MIN_LOG("aal_pon_loopback_set return %d\n", rt);
                    goto END;
                }

                rt = epon_request_onu_traffic_block(context, device_id, llidport, CS_UP_STREAM, FALSE);
                if (rt) {
                    SDL_MIN_LOG("epon_request_onu_traffic_block return %d\n", rt);
                    goto END;
                }
            }
            break;

            case SDL_PORT_LOOPBACK_TX2RX: {
                rt = aal_pon_loopback_set(EPON_TRUE);
                if (rt) {
                    SDL_MIN_LOG("aal_pon_loopback_set fail!return %d\n", rt);
                    goto END;
                }

                rt = epon_request_onu_traffic_block(context, device_id, llidport, CS_UP_STREAM, TRUE);
                if (rt) {
                    SDL_MIN_LOG("epon_request_onu_traffic_block return %d\n", rt);
                    goto END;
                }
            }
            break;

            case SDL_PORT_LOOPBACK_RX2TX:
            default: {
                SDL_MIN_LOG("loopback %d is not supported\n", loopback);
                return CS_E_PARAM;
            }
        }

        __pon_loopback_mode = loopback;

    } 
    else {
        port = L2P_PORT(port_id);
        if (__uni_loopback_mode[port] == loopback) {
            rt =  CS_E_OK;
            goto END;
        }

        gt_getswitchunitbylport(port, &unit, &hwport);

        switch (loopback) {
            case SDL_PORT_LOOPBACK_NONE: {
                rt = epon_request_onu_port_status_set(context, device_id, llidport, port_id, port_cfg[port]);
                if (rt) {
                    SDL_MIN_LOG("epon_request_onu_port_status_set return %d\n", rt);
                    goto END;
                }
                
                ret = gprtGetSwitchReg(QD_DEV_PTR, hwport, QD_PHY_CONTROL_REG, &phyData);
                if(GT_OK != ret){
                    SDL_MIN_LOG("gprtGetSwitchReg return %d\n", ret);
                    rt = CS_E_ERROR;
                    goto END;
                }
                
                phyData = phyData & (~QD_PHY_LOOPBACK);
                ret = gprtSetSwitchReg(QD_DEV_PTR, hwport, QD_PHY_CONTROL_REG, phyData);
                if(GT_OK != ret){
                    SDL_MIN_LOG("gprtSetSwitchReg return %d\n", ret);
                    rt = CS_E_ERROR;
                    goto END;
                }
            }
            break;

            case SDL_PORT_LOOPBACK_TX2RX: {
                port_cfg[port] = __port_cfg[port].port_cfg;
                rt = epon_request_onu_port_status_set(context, device_id, llidport, port_id, SDL_PORT_100_FULL);
                if (rt) {
                    SDL_MIN_LOG("epon_request_onu_port_status_set return %d!\n", rt);
                    goto END;
                }
                
                ret = gprtGetSwitchReg(QD_DEV_PTR, hwport, QD_PHY_CONTROL_REG, &phyData);
                if(GT_OK != ret){
                    SDL_MIN_LOG("gprtGetSwitchReg return %d\n", ret);
                    rt = CS_E_ERROR;
                    goto END;
                }

                phyData = phyData | QD_PHY_LOOPBACK;
                
                ret = gprtSetSwitchReg(QD_DEV_PTR, hwport, QD_PHY_CONTROL_REG, phyData);
                if(GT_OK != ret){
                    SDL_MIN_LOG("gprtSetSwitchReg return %d\n", ret);
                    rt = CS_E_ERROR;
                    goto END;
                }

            }
            break;

            case SDL_PORT_LOOPBACK_RX2TX:
            default: {
                SDL_MIN_LOG(" loopback %d is not supported\n", loopback);
                rt = CS_E_PARAM;
                goto END;
            }
        }

        __uni_loopback_mode[port] = loopback;
    }

END:
    return rt;
}

cs_status epon_request_onu_port_lpbk_get(
    CS_IN cs_callback_context_t             context,
    CS_IN cs_int32                          device_id,
    CS_IN cs_int32                          llidport,
    CS_IN cs_port_id_t                      port_id,
    CS_OUT cs_sdl_port_loopback_t           *loopback
)
{

    if (NULL == loopback) {
        SDL_MIN_LOG("loopback is NULL pointer\n");
        return CS_E_PARAM;
    }

    if (port_id >CS_UNI_PORT_ID4){
        SDL_MIN_LOG("Port ID is Invalid\n");
        return CS_E_PARAM;
    }
    
    if (port_id == CS_PON_PORT_ID) {
        *loopback = __pon_loopback_mode;
    }
    else{
        *loopback = __uni_loopback_mode[port_id-1];
    } 

    return CS_E_OK;
}

cs_status epon_request_onu_port_status_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              port_id,
    CS_IN cs_sdl_port_speed_cfg_t   speed_cfg
)
{
    GT_LPORT             port;
//    GT_STATUS          ret  = 0;
    cs_status              rt = CS_E_OK;

    
    UNI_PORT_CHECK(port_id);
    
    port = L2P_PORT(port_id);
    if (__port_cfg[port].port_cfg == speed_cfg) {
        goto END;
    }

    if ((MAX_PORT_TRAFFIC_RATE == 100000) && (speed_cfg == SDL_PORT_AUTO_10_100_1000)) {
        speed_cfg = SDL_PORT_AUTO_10_100;
    }


    __port_cfg[port].port_cfg = speed_cfg;

END:
    return rt;
}

cs_status epon_request_onu_port_status_get(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_port_id_t              port_id,
    CS_OUT cs_sdl_port_speed_cfg_t   *speed_cfg
)
{
    if (NULL == speed_cfg) {
        SDL_MIN_LOG("speed_cfg is a NULL pointer\n" );
        return CS_E_PARAM;
    }

    UNI_PORT_CHECK(port_id);

    *speed_cfg = __port_cfg[port_id -1].port_cfg;

    return CS_E_OK;
}

cs_status epon_request_onu_port_admin_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              port_id,
    CS_IN cs_sdl_port_admin_t       admin
)
{
    GT_LPORT      port;
    GT_BOOL    state;
    GT_STATUS   ret  = 0;
    cs_status       rt = CS_E_OK;
    GT_32 unit, hwport;
    
    UNI_PORT_CHECK(port_id);
    
    port = L2P_PORT(port_id);
    if (__port_cfg[port].uni_admin == admin) {
        goto END;
    }

    if (admin == SDL_PORT_ADMIN_DOWN) {
        state = GT_TRUE;
    } else {
        state = GT_FALSE;
    }
    
    gt_getswitchunitbylport(port, &unit, &hwport);
    ret = gprtPortPowerDown(QD_DEV_PTR, hwport, state);
    if(GT_OK != ret){
        SDL_MIN_LOG("gstpSetPortState return %d\n", ret);
        rt = CS_E_ERROR;
        goto END;
    }

    __port_cfg[port].uni_admin = admin;

END:
    return rt;
}


cs_status epon_request_onu_port_admin_get(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              port_id,
    CS_OUT cs_sdl_port_admin_t      *admin
)
{
    if (NULL == admin) {
        SDL_MIN_LOG("admin is a NULL pointer\n" );
        return CS_E_PARAM;
    }
    
    UNI_PORT_CHECK(port_id);
    
    *admin = __port_cfg[port_id-1].uni_admin;

    return CS_E_OK;
}

cs_status epon_request_onu_port_stats_get(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              port_id,
    CS_IN cs_boolean                read_clear,
    CS_OUT cs_sdl_port_uni_stats_t  *uni_stats
)
{

    GT_LPORT           port;
    cs_uint32            val[2] = {0, 0};
    GT_STATS_COUNTER_SET3 sts;
    GT_32 unit, hwport;
    GT_STATUS   ret = GT_ERROR;
    cs_status            rt = CS_E_OK;
    
    if (NULL == uni_stats) {
        SDL_MIN_LOG("uni_stats is a NULL pointer!!\n");
        rt = CS_E_PARAM;
        goto END;
    }
    
    if ((port_id < CS_UNI_PORT_ID1) || (port_id > CS_UPLINK_PORT)) {
        SDL_MIN_LOG("port_id %d is not supported\n", port_id);
        rt = CS_E_PARAM;
        goto END;
    }

    memset((cs_uint8 *)uni_stats, 0, sizeof(cs_sdl_port_uni_stats_t));
    memset((cs_uint8*)&sts, 0, sizeof(GT_STATS_COUNTER_SET3));
//    memset((cs_uint8*)&port_cnt, 0, sizeof(port_cnt));

    if (port_id != CS_DOWNLINK_PORT) {
        port = L2P_PORT(port_id);

        gt_getswitchunitbylport(port, &unit, &hwport);

        ret = gstatsGetPortAllCounterGwd(QD_DEV_PTR, hwport, &sts );
        if(ret != GT_OK)
        {
        	rt = CS_ERROR;
        	goto END;
        }

        uni_stats->rxucfrm_cnt = sts.InUnicasts;
        uni_stats->rxbcfrm_cnt = sts.InBroadcasts;
        uni_stats->rxmcfrm_cnt = sts.InMulticasts;
        uni_stats->rxpausefrm_cnt = sts.InPause;
        uni_stats->rxoamfrm_cnt = 0;
        uni_stats->rxcrcerrfrm_cnt = sts.InFCSErr;
        uni_stats->fcs_errors = sts.InFCSErr;
        uni_stats->rxunknownopfrm_cnt = 0;
        uni_stats->rxundersizefrm_cnt = sts.Undersize;
        uni_stats->rxoversizefrm_cnt = sts.Oversize;
        uni_stats->rxjabberfrm_cnt = sts.Jabber;
        uni_stats->rxbyte_cnt = sts.InGoodOctetsLo;
        uni_stats->txbyte_cnt = sts.OutOctetsLo;
        uni_stats->txucfrm_cnt = sts.OutUnicasts;
        uni_stats->txmcfrm_cnt = sts.OutMulticasts;
        uni_stats->txbcfrm_cnt = sts.OutBroadcasts;
        uni_stats->txoamfrm_cnt = 0;
        uni_stats->txpausefrm_cnt = sts.OutPause;
        uni_stats->txsinglecolfrm_cnt = sts.Single;
        uni_stats->txmulticolfrm_cnt = sts.Multiple;
        uni_stats->txlatecolfrm_cnt = sts.Late;
        uni_stats->txexesscolfrm_cnt = sts.Excessive;
		uni_stats->rxstatsfrm64_cnt = sts.Octets64;
		uni_stats->rxstatsfrm65_127_cnt = sts.Octets127;
		uni_stats->rxstatsfrm128_255_cnt = sts.Octets255;
		uni_stats->rxstatsfrm256_511_cnt = sts.Octets511;
		uni_stats->rxstatsfrm512_1023_cnt = sts.Octets1023;
		uni_stats->rxfrm_cnt = uni_stats->rxucfrm_cnt + uni_stats->rxmcfrm_cnt + uni_stats->rxbcfrm_cnt;

		if(read_clear)
			gstatsFlushPort(QD_DEV_PTR, hwport);

    } 
    else {
        aal_uni_mib_get(AAL_MIB_RxUCPktCnt, read_clear, &val[1], &val[0]);
        uni_stats->rxucfrm_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_RxMCFrmCnt, read_clear, &val[1], &val[0]);
        uni_stats->rxmcfrm_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_RxBCFrmCnt, read_clear, &val[1], &val[0]);
        uni_stats->rxbcfrm_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_RxOAMFrmCnt, read_clear, &val[1], &val[0]);
        uni_stats->rxoamfrm_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_RxPauseFrmCnt, read_clear, &val[1], &val[0]);
        uni_stats->rxpausefrm_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_RxUnKnownOCFrmCnt, read_clear, &val[1], &val[0]);
        uni_stats->rxunknownopfrm_cnt = val[0];
        
        aal_uni_mib_get(AAL_MIB_RxCrcErrFrmCnt, read_clear, &val[1], &val[0]);
        uni_stats->rxcrcerrfrm_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_RxUndersizeFrmCnt, read_clear, &val[1], &val[0]);
        uni_stats->rxundersizefrm_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_RxRuntFrmCnt, read_clear, &val[1], &val[0]);
        uni_stats->rxruntfrm_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_RxOvSizeFrmCnt, read_clear, &val[1], &val[0]);
        uni_stats->rxoversizefrm_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_RxJabberFrmCnt, read_clear, &val[1], &val[0]);
        uni_stats->rxjabberfrm_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_RxStatsFrm64Oct, read_clear, &val[1], &val[0]);
        uni_stats->rxstatsfrm64_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_RxStatsFrm65to127Oct, read_clear, &val[1], &val[0]);
        uni_stats->rxstatsfrm65_127_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_RxStatsFrm128to255Oct, read_clear, &val[1], &val[0]);
        uni_stats->rxstatsfrm128_255_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_RxStatsFrm256to511Oct, read_clear, &val[1], &val[0]);
        uni_stats->rxstatsfrm256_511_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_RxStatsFrm512to1023Oct, read_clear, &val[1], &val[0]);
        uni_stats->rxstatsfrm512_1023_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_RxStatsFrm1024to1518Oct, read_clear, &val[1], &val[0]);
        uni_stats->rxstatsfrm1024_1518_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_RxStatsFrm1519toMaxOct, read_clear, &val[1], &val[0]);
        uni_stats->rxstatsfrm1519_Max_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_RxByteCount, read_clear, &val[1], &val[0]);
        uni_stats->rxbyte_cnt = *(cs_uint64 *)val; // 64 bits

        aal_uni_mib_get(AAL_MIB_RxFrmCount, read_clear, &val[1], &val[0]);
        uni_stats->rxfrm_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_TxUCPktCnt, read_clear, &val[1], &val[0]);
        uni_stats->txucfrm_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_TxMCFrmCnt, read_clear, &val[1], &val[0]);
        uni_stats->txmcfrm_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_TxBCFrmCnt, read_clear, &val[1], &val[0]);
        uni_stats->txbcfrm_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_TxOAMFrmCnt, read_clear, &val[1], &val[0]);
        uni_stats->txoamfrm_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_TxPauseFrmCnt, read_clear, &val[1], &val[0]);
        uni_stats->txpausefrm_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_TxCrcErrFrmCnt, read_clear, &val[1], &val[0]);
        uni_stats->txcrcerrfrm_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_TxOvSizeFrmCnt, read_clear, &val[1], &val[0]);
        uni_stats->txoversizefrm_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_TxSingleColFrm, read_clear, &val[1], &val[0]);
        uni_stats->txsinglecolfrm_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_TxMultiColFrm, read_clear, &val[1], &val[0]);
        uni_stats->txmulticolfrm_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_TxLateColFrm, read_clear, &val[1], &val[0]);
        uni_stats->txlatecolfrm_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_TxExessColFrm, read_clear, &val[1], &val[0]);
        uni_stats->txexesscolfrm_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_TxEStatsFrm64Oct, read_clear, &val[1], &val[0]);
        uni_stats->txstatsfrm64_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_TxEStatsFrm65to127Oct, read_clear, &val[1], &val[0]);
        uni_stats->txstatsfrm65_127_cnt = val[0];
        
        aal_uni_mib_get(AAL_MIB_TxEStatsFrm128to255Oct, read_clear, &val[1], &val[0]);
        uni_stats->txstatsfrm128_255_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_TxEStatsFrm256to511Oct, read_clear, &val[1], &val[0]);
        uni_stats->txstatsfrm256_511_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_TxEStatsFrm512to1023Oct, read_clear, &val[1], &val[0]);
        uni_stats->txstatsfrm512_1023_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_TxEStatsFrm1024to1518Oct, read_clear, &val[1], &val[0]);
        uni_stats->txstatsfrm1024_1518_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_TxEStatsFrm1519toMaxOct, read_clear, &val[1], &val[0]);
        uni_stats->txstatsfrm1519_Max_cnt = val[0];

        aal_uni_mib_get(AAL_MIB_TxByteCount, read_clear, &val[1], &val[0]);
        uni_stats->txbyte_cnt = *(cs_uint64 *)val; // 64 bits

        aal_uni_mib_get(AAL_MIB_TxFrmCount, read_clear, &val[1], &val[0]);
        uni_stats->txfrm_cnt = val[0];

    }
    
END:
    return rt;
}

cs_status epon_request_onu_port_stats_clr(
    CS_IN cs_callback_context_t             context,
    CS_IN cs_int32                          device_id,
    CS_IN cs_int32                          llidport,
    CS_IN cs_port_id_t                      port_id
)
{
    cs_status rt = CS_E_OK;
    GT_LPORT port;
    GT_STATUS   ret  = 0;
    cs_aal_uni_cfg_t uni_cfg;
    cs_aal_uni_cfg_mask_t uni_msk;

    if ((port_id < CS_UNI_PORT_ID1) || (port_id > CS_UPLINK_PORT)) {
        SDL_MIN_LOG("port_id %d is not supported\n", port_id);
        rt = CS_E_PARAM;
        goto END;
    }

    if (port_id == CS_DOWNLINK_PORT) {
        uni_msk.u32 = 0;
        uni_msk.s.mib_clr_all = 1;
        uni_cfg.mib_clr_all = 1;
        rt = aal_uni_mac_cfg_set(uni_msk, uni_cfg);
        if (rt) {
            SDL_MIN_LOG("aal_uni_mac_cfg_set return %d\n", rt);
            goto END;
        }
    } 
    else {
        port = L2P_PORT(port_id);
//        mtodo added codes for port statistic reset
//        ret = rtk_stat_port_reset(port);
        if (GT_OK != ret) {
            SDL_MIN_LOG("rtk_stat_port_reset return %d\n", ret);
            rt = CS_E_ERROR;
            goto END;
        }
    }
    
END:
    return rt;
}

#if 1
cs_status epon_request_onu_port_link_status_get(
    CS_IN cs_callback_context_t         context,
    CS_IN cs_int32                      device_id,
    CS_IN cs_int32                      llidport,
    CS_IN cs_port_id_t                  port_id,
    CS_OUT cs_sdl_port_link_status_t    *link_status
)
{
    GT_LPORT     port;
    GT_STATUS  ret  = 0;
    GT_BOOL		link;
    GT_32 unit, hwport;
    
    if (NULL == link_status) {
        SDL_MIN_LOG("link_status is NULL pointer\n");
		//cs_printf("link_status port %d == NULL\n",port_id);
        return CS_E_PARAM;
    }

    UNI_PORT_CHECK(port_id);
    port = L2P_PORT(port_id);

    gt_getswitchunitbylport(port, &unit, &hwport);
    ret = gprtGetLinkState(QD_DEV_PTR, hwport, &link);
    if (GT_OK != ret) {
        SDL_MIN_LOG("gprtGetLinkState return %d\n", ret);
		//cs_printf("phy port %d status error\n",port_id);
        return CS_E_ERROR;
    }
    
    if (link) {
	//	cs_printf("uni port %d link up\n",port_id);
        *link_status = SDL_PORT_LINK_STATUS_UP;
    } 
    else {
		//cs_printf("uni port %d link down\n",port_id);
        *link_status = SDL_PORT_LINK_STATUS_DOWN;
    }
    
    return CS_E_OK;
}
#endif
/* This func is to set ONU GE port's RX flow-control and switch's uplink port's flow-control.
 * TX flow-control is not touched.
 */
cs_status uni_ge_rx_flow_ctrl_set(cs_boolean enable)
{
    cs_status rc = CS_E_OK;
    GT_LPORT port;
    GT_32 unit, hwport;

    //set CS8030 UNI rx port flow control
    rc = aal_rx_flow_control_set(enable);
    if (rc) {
        SDL_MIN_LOG("In function:%s,line:%d invoke aal_rx_flow_control_set fail!\n", __FUNCTION__, __LINE__);
        goto end;
    }

    // set uplink port flow control
    port = L2P_PORT(CS_UPLINK_PORT);
    gt_getswitchunitbylport(port, &unit, &hwport);

    rc = gprtSetPause(QD_DEV_PTR, hwport, GT_PHY_BOTH_PAUSE);
    if (rc) {
        SDL_MIN_LOG("In function:%s,line:%d invoke gprtSetPause fail!\n", __FUNCTION__, __LINE__);
        goto end;
    }

end:
    return rc;

}

cs_status uni_ge_rx_flow_ctrl_get(cs_boolean *enable)
{
    if (NULL == enable) {
        SDL_MIN_LOG("In %s, enable is NULL pointer\n", __FUNCTION__);
        return CS_E_PARAM;
    }

    aal_rx_flow_control_get(enable);

    return CS_E_OK;

}


cs_status epon_request_onu_port_flow_ctrl_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              port_id,
    CS_IN cs_boolean                enable
)
{
    cs_aal_flow_control_t cfg[4];
    cs_status rc = CS_E_OK;
    GT_LPORT port;
    cs_boolean internal_enable;
    GT_32 unit, hwport;

    if (enable > 1) {
        SDL_MIN_LOG("In %s, port_id %d  enable(%d) is not supported\n", __FUNCTION__, port_id, enable);
        rc = CS_E_PARAM;
        goto end;
    }

    if (port_id == CS_UPLINK_PORT || port_id == CS_DOWNLINK_PORT) {
        rc = uni_ge_rx_flow_ctrl_set(enable);
        goto end;
    }

    UNI_PORT_CHECK(port_id);

    memset(cfg, 0, sizeof(cs_aal_flow_control_t)*4);

    port = L2P_PORT(port_id);
    
    gt_getswitchunitbylport(port, &unit, &hwport);

    if (__port_cfg[port].flow_ctrl_en == enable) {
        SDL_MIN_LOG("set flow control(port: %d): is same( %d) as before\n", port, enable);
        goto end;
    }
    
    __port_cfg[port].flow_ctrl_en = enable;

    /* mtodo cortina cpu��bmģ���֧��5�����ж��н�������ʹ�ܣ���ǰ��ѡ��ǰ4���˿ڵ�ʹ��ӳ�䵽bm*/
    if (__port_cfg[0].flow_ctrl_en || __port_cfg[1].flow_ctrl_en ||
            __port_cfg[2].flow_ctrl_en || __port_cfg[3].flow_ctrl_en) {
        cfg[0].mem_fc.tx_en = 1;
    } else {
        cfg[0].mem_fc.tx_en = 0;
    }

    /*set bm flow control threshold, turn on CS8030 UNI port flow control*/
    cfg[0].fc_msk.u = 0;
    cfg[0].fc_msk.s.mem = 1;

  
    uni_ge_rx_flow_ctrl_get(&internal_enable);
    
    /**/
    cfg[0].mem_fc.rx_en = enable;
    /*cfg[0].mem_fc.tx_en = enable; */        /*tx flow control work in RTL MAC */



    rc = aal_flow_control_set(cfg);
    if (rc) {
        SDL_MIN_LOG("In function:%s,line:%d invoke aal_flow_control_set fail!\n", __FUNCTION__, __LINE__);
        goto end;
    }

    rc = gprtSetPause(QD_DEV_PTR, hwport, GT_PHY_PAUSE); //6096�ƺ���֧��GT_PHY_PAUSE
    if(rc)
    {
    	SDL_MAJ_LOG("In function:%s, line:%d invoke  gprtSetPause fail!(%d)", __FUNCTION__, __LINE__, rc);
    	goto end;
    }

#if 0
    rc = rtk_port_phyAutoNegoAbility_get(port, &advertisement);
    if (rc) {
        SDL_MIN_LOG("In function:%s,line:%d invoke rtk_port_macForceLinkExt0_get fail!\n", __FUNCTION__, __LINE__);
        goto end;
    }
    //SDL_MIN_LOG("set flow control %d enable %d\n", port, enable);

    advertisement.FC = enable;

    rc = rtk_port_phyAutoNegoAbility_set(port, &advertisement);
    if (rc) {
        SDL_MIN_LOG("In function:%s,line:%d invoke rtk_port_phyAutoNegoAbility_set fail!\n", __FUNCTION__, __LINE__);
        goto end;
    }
    rc = rtk_rate_igrBandwidthCtrlRate_get(port, &us_rate, &pIfg_include, &pFc_enable);
    if (rc) {
        SDL_MIN_LOG("In function:%s,line:%d invoke rtk_rate_igrBandwidthCtrlRate_get fail!\n", __FUNCTION__, __LINE__);
        goto end;
    }
    pFc_enable = enable;

    rc = rtk_rate_igrBandwidthCtrlRate_set(port, us_rate, pIfg_include, pFc_enable);
    if (rc) {
        SDL_MIN_LOG("In function:%s,line:%d invoke rtk_rate_igrBandwidthCtrlRate_set fail!\n", __FUNCTION__, __LINE__);
        goto end;
    }
#endif

end:
    return rc;

}


cs_status epon_request_onu_port_flow_ctrl_get(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              port_id,
    CS_OUT cs_boolean                *enable
)
{
    cs_status rc = CS_E_OK;
    GT_LPORT port;

    if (NULL == enable) {
        rc = CS_E_PARAM;
        goto end;
    }

    if (port_id == CS_UPLINK_PORT || port_id == CS_DOWNLINK_PORT) {
        rc = uni_ge_rx_flow_ctrl_get(enable);
        goto end;
    }

    UNI_PORT_CHECK(port_id);

    port = port_id - 1;
    *enable = __port_cfg[port].flow_ctrl_en;

end:
    return rc;
}

cs_status epon_request_onu_port_ds_rate_limit_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              port_id,
    CS_IN cs_sdl_policy_t           *rate
)
{
    cs_status rc = CS_E_OK;
    GT_LPORT port;
    cs_aal_rate_limit_t shp;
    GT_32 unit, hwport;

    if (NULL == rate) {
        SDL_MIN_LOG("In %s, rate is NULL pointer\n", __FUNCTION__);
        return CS_E_PARAM;
    }
//diag_printf("port id = 0x%x\n",port_id);
    if (port_id != CS_DOWNLINK_PORT) {
        UNI_PORT_CHECK(port_id);
        
        if (rate->rate > MAX_PORT_TRAFFIC_RATE) {
            SDL_MIN_LOG("In %s, rate->rate is error!\n", __FUNCTION__);
            return CS_E_PARAM;
        }   
        
        port = L2P_PORT(port_id);

        gt_getswitchunitbylport(port, &unit, &hwport);

        rc = grcSetEgressRateInKbps(QD_DEV_PTR, hwport, (GT_ERATE_TYPE*)&rate->rate);

        if(rc == GT_OK)
        {
			__ds_rate_limit[port].enable = rate->enable;
			__ds_rate_limit[port].rate = rate->rate;
			__ds_rate_limit[port].cbs = rate->cbs;
			__ds_rate_limit[port].ebs = rate->ebs;
			rc = CS_E_OK;
        }
        else
        	rc = CS_E_ERROR;
    }else{
        memset(&shp, 0, sizeof(shp));

        shp.ctrl.s.enable = rate->enable;
        // diag_printf(" rate %d, CBS %d\n", rate->rate, rate->cbs);

        shp.rate = rate->rate;
        shp.cbs = rate->cbs / 1000;

        if (shp.cbs < MIN_BURST_SIZE) {
            shp.cbs = MIN_BURST_SIZE;
        }

        if (shp.cbs > MAX_BURST_SIZE) {
            shp.cbs = MAX_BURST_SIZE;
        }

        if (aal_flow_shaper_set(AAL_PORT_ID_GE, &shp)) {
            rc = CS_E_PARAM;
            goto end;
        }

        __pir = rate->cbs;

    }
    

end:
    return rc;
}

cs_status epon_request_onu_port_ds_rate_limit_get(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              port_id,
    CS_OUT cs_sdl_policy_t           *rate
)
{
    GT_LPORT port;
    cs_aal_rate_limit_t shp;
    if (NULL == rate) {
        SDL_MIN_LOG("In %s, rate is NULL pointer\n", __FUNCTION__);
        return CS_E_PARAM;
    }

    if (port_id != CS_DOWNLINK_PORT) {
        UNI_PORT_CHECK(port_id);

        port = (GT_LPORT)(port_id - 1);

        rate->enable = __ds_rate_limit[port].enable ;
        rate->rate = __ds_rate_limit[port].rate;
        rate->cbs = __ds_rate_limit[port].cbs;
    } else {
        aal_flow_shaper_get(AAL_PORT_ID_GE, &shp);
        rate->enable = shp.ctrl.s.enable;
        rate->rate = shp.rate;
        rate->cbs = __pir;
    }

    return CS_E_OK;

}

cs_status epon_request_onu_port_policy_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              port_id,
    CS_IN cs_sdl_policy_t           *policy
)
{
    cs_status rc = CS_E_OK;
    GT_LPORT port;
    GT_32 unit, hwport;

    port = L2P_PORT(port_id);
    gt_getswitchunitbylport(port, &unit, &hwport);

    rc = grcSetPri0RateInKbps(QD_DEV_PTR, hwport, policy->rate);

    if(rc == GT_OK)
    {
		__us_rate_limit[port].enable = policy->enable;
		__us_rate_limit[port].rate = policy->rate;
		__us_rate_limit[port].cbs = policy->cbs;
		__us_rate_limit[port].ebs = policy->ebs;

		rc = CS_E_OK;
    }
    else
    	rc = CS_E_ERROR;

    return rc;
}
cs_status epon_request_onu_port_policy_get(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              port_id,
    CS_OUT cs_sdl_policy_t          *policy
)
{
    GT_LPORT port;

    if (NULL == policy) {
        SDL_MIN_LOG("In %s, NULL pointer!\n", __FUNCTION__);
        return CS_E_PARAM;
    }

    UNI_PORT_CHECK(port_id);    

    port = (GT_LPORT)(port_id - 1);

    policy->enable = __us_rate_limit[port].enable;
    policy->rate = __us_rate_limit[port].rate;
    policy->cbs  = __us_rate_limit[port].cbs;
    policy->ebs = __us_rate_limit[port].ebs;


    return CS_E_OK;

}




cs_status uni_min_pkt_size_set(
    CS_IN cs_uint16 min
)
{
    cs_status rc = CS_E_OK;
    cs_aal_uni_cfg_t uni_cfg;
    cs_aal_uni_cfg_mask_t uni_msk;

    if (min > 127 || min < 64)
        return CS_E_PARAM;

    uni_msk.u32 = 0;
    uni_msk.s.min_size = 1;
    uni_cfg.min_size = min;

    rc = aal_uni_mac_cfg_set(uni_msk, uni_cfg);

    return rc;
}

cs_status uni_min_pkt_size_get(
    CS_OUT cs_uint16 *min
)
{
    cs_status rc = CS_E_OK;
    cs_aal_uni_cfg_t uni_cfg;

    if (NULL == min) {
        SDL_MIN_LOG("In %s, NULL pointer!\n", __FUNCTION__);
        return CS_E_PARAM;
    }

    aal_uni_mac_cfg_get(&uni_cfg);
    *min = uni_cfg.min_size;

    return rc;
}
void storm_rate_init()
{
	#if 1
	cs_callback_context_t     context;
	cs_sdl_rl_mod_e mode;
	cs_sdl_storm_ctrl_e type;
	cs_sdl_policy_t rate;
	cs_port_id_t portid;
	cs_status sdl_rt;
	#endif
	#if 1
	mode = SDL_RL_MOD_BPS;
	type = SDL_STORM_TYPE_BC;
	rate.enable = 1;
	rate.rate = 8;
	#endif
	#if 1
	//cs_status status;
	
    for(portid = 1; portid <=UNI_PORT_MAX; portid++)
    	{
    		sdl_rt = epon_request_onu_port_storm_ctrl_set(context,  0, 0, portid, mode, type, &rate);
			if(sdl_rt){
		        cs_printf("epon_request_onu_port_storm_ctrl_set failed! \n");          
		    }
    	}
	cs_printf("Broadcase storm enable\n");
#endif
}

cs_status sdl_port_init(
    CS_IN const sdl_init_cfg_t   *cfg
)
{
    cs_callback_context_t     context;
    cs_sdl_port_speed_cfg_t   speed_cfg;
    cs_aal_uni_cfg_t          uni_cfg;
    cs_aal_uni_cfg_mask_t     uni_msk;
    cs_aal_uni_cfg_ext_t      uni_mac_ext;
    cs_aal_uni_cfg_ext_mask_t uni_mac_ext_mask;
    GT_BOOL						switch_mtu = GT_TRUE; //mtu set to 1522B
    GT_STATUS             ret = 0;
    cs_status                 rt = CS_E_OK;
    cs_uint8                  index;

    GT_LPORT 				port;
    GT_32 unit, hwport;

    
    if (NULL == cfg) {
        SDL_MIN_LOG(" NULL pointer!\n" );
        return CS_E_PARAM;
    }
    
    /*config Lynxd uni port MTU to __SDL_PORT_MTU_MAX*/
    uni_msk.u32 = 0;
    uni_msk.s.max_size = 1;
    uni_cfg.max_size = __SDL_PORT_MTU_MAX;

    rt = aal_uni_mac_cfg_set(uni_msk, uni_cfg);
    if (rt) {
        SDL_MIN_LOG("aal_uni_mac_cfg_set return %d\n", rt);
        goto END;
    }
    
    /*config switch MTU to __SDL_PORT_MTU_MAX*/
    FOR_UNIT_START(GT_32, unit)

    ret = gsysSetMaxFrameSize(QD_DEV_PTR, switch_mtu);
    if (GT_OK != ret) {
        SDL_MIN_LOG("gsysSetMaxFrameSize return %d\n", rt);
        rt = CS_E_ERROR;
        goto END;
    }

    FOR_UNIT_END

    speed_cfg = SDL_PORT_AUTO_10_100;

    for (index=0; index<UNI_PORT_MAX; index++) {
        __port_cfg[index].port_cfg = speed_cfg;
        __port_cfg[index].uni_admin = SDL_PORT_ADMIN_UP;
        __port_cfg[index].flow_ctrl_en = 0;
        __us_rate_limit[index].enable = 0;
        __us_rate_limit[index].rate = MAX_PORT_TRAFFIC_RATE;
        __us_rate_limit[index].cbs = 10000;
        __us_rate_limit[index].ebs = 1000;
        __ds_rate_limit[index].enable = 0;
        __ds_rate_limit[index].rate = MAX_PORT_TRAFFIC_RATE;
        __ds_rate_limit[index].cbs = 10000;
        __ds_rate_limit[index].ebs = 1000;
        __sc_rate_limit[index].enable = 0;
        __sc_rate_limit[index].rate = MAX_PORT_TRAFFIC_RATE;
        __sc_rate_limit[index].cbs = 10000;
        __sc_rate_limit[index].ebs = 1000;

        gt_getswitchunitbylport(index, &unit, &hwport);
        ret = gprtSetPortAutoMode(QD_DEV_PTR, hwport, SPEED_AUTO_DUPLEX_AUTO);
        if(GT_OK != ret){
            SDL_MIN_LOG("gprtSetPortAutoMode return %d\n", ret);
            rt =  CS_E_ERROR;
            goto END;
        }


        //epon_request_onu_port_status_set(context, 0, 0, CS_UNI_PORT_ID1+index, speed_cfg);

        /*By default, flow control is disable*/
        rt = epon_request_onu_port_flow_ctrl_set(context, 0, 0, 1 + index, 0);
        if(rt){
            SDL_MIN_LOG("epon_request_onu_port_flow_ctrl_set return %d\n", rt);
            goto END;
        }
        
        /*By default, loopback is disable*/
        rt = epon_request_onu_port_lpbk_set(context, 0, 0, 1 + index, 0);
        if(rt){
            SDL_MIN_LOG("epon_request_onu_port_lpbk_set return %d\n", rt);
            goto END;
        }
        
    }

    /*Port Isolation is enabled by default*/
    rt = epon_request_onu_port_isolation_set(context, 0, 0, 1);
    if(rt){
        SDL_MIN_LOG("epon_request_onu_port_isolation_set return %d\n", rt);
        goto END;
    }

    /*By default, PON port MTU is 1536*/
    rt = epon_request_onu_port_mtu_set(context, 0, 0, CS_PON_PORT_ID, 1536);
    if(rt){
        SDL_MIN_LOG("epon_request_onu_port_mtu_set return %d\n", rt);
        goto END;
    }
     /*********if storm control is disable, flooding packet from all 4 UNI ports will be limited by this rate limiter**/
     /*********it should be bigger than 400Mbps************************************************************************/   
    /* mtodo add storm control api calls
    ret = rtk_rate_shareMeter_set(PORT_STORM_METER_ID_END, 1000000, DISABLED);
    if(GT_OK != ret){
        SDL_MIN_LOG("rtk_rate_shareMeter_set return %d\n", ret);
        rt =  CS_E_ERROR;
        goto END;
    }
	*/
    uni_mac_ext_mask.u32 = 0;
    uni_mac_ext.txfifo_thrshld = sdl_int_cfg.uni_tx_fifo_threshould;
    uni_mac_ext.fifo_thrshld_low = sdl_int_cfg.uni_tx_fifo_ready_low;
    uni_mac_ext.fifo_thrshld_high = sdl_int_cfg.uni_tx_fifo_ready_high;
    uni_mac_ext.phy_mode = sdl_int_cfg.uni_phy_mod;
    uni_mac_ext.tag_exclude = sdl_int_cfg.mtu_tag_exclude;

    uni_mac_ext_mask.s.txfifo_thrshld = TRUE;
    uni_mac_ext_mask.s.fifo_thrshld_low = TRUE;
    uni_mac_ext_mask.s.fifo_thrshld_high = TRUE;
    uni_mac_ext_mask.s.phy_mode = TRUE;
    uni_mac_ext_mask.s.tag_exclude = TRUE;
    
    rt = aal_uni_mac_cfg_ext_set(uni_mac_ext_mask, uni_mac_ext);
    if(rt){
        SDL_MIN_LOG("aal_uni_mac_cfg_ext_set return %d\n", rt);
        goto END;
    }
    
    if (sdl_int_cfg.uni_phy_mod) {
    	/*mtodo added process for UNI as master*/
#if 0
        mod = MODE_EXT_RGMII;
        mac_abi.forcemode = 1;
        mac_abi.speed = 2;
        mac_abi.duplex = 1;
        mac_abi.link = 1;
        mac_abi.nway = 1;
        mac_abi.txpause = 0;
        mac_abi.rxpause = 0;
        
        ret = rtk_port_macForceLinkExt0_set(mod, &mac_abi);
        if(GT_OK != ret){
            SDL_MIN_LOG("rtk_port_macForceLinkExt0_set return %d\n", ret);
            rt =  CS_E_ERROR;
            goto END;
        }
        if(RTK_CHIP_8305 == rtk_chip && MAKE_UART)
        	{
        		ret = rtk_led_mode_set(LED_MODE_2);
				if(GT_OK != ret){
			            SDL_MIN_LOG("rtk_led_mode_set return %d\n", ret);
			            rt =  CS_E_ERROR;
			            goto END;
			        }
				ret = rtk_led_groupConfig_set(LED_GROUP_2, LED_CONFIG_LINK_ACT);
				if(GT_OK != ret){
		            SDL_MIN_LOG("rtk_led_groupConfig_set return %d\n", ret);
		            rt =  CS_E_ERROR;
		            goto END;
		        }
        	}
		else
			{
				if(RTK_CHIP_8305 == rtk_chip)
					{
						ret = rtk_led_mode_set(LED_MODE_1);
						if(GT_OK != ret){
					            SDL_MIN_LOG("rtk_led_mode_set return %d\n", ret);
					            rt =  CS_E_ERROR;
					            goto END;
					        }
						ret = rtk_led_groupConfig_set(LED_GROUP_1, LED_CONFIG_LINK_ACT);
						if(GT_OK != ret){
				            SDL_MIN_LOG("rtk_led_groupConfig_set return %d\n", ret);
				            rt =  CS_E_ERROR;
				            goto END;
				        }
					}
			}
		if(RTK_CHIP_8365 == rtk_chip)
			{
				ret = rtk_led_mode_set(LED_MODE_2);
				if(GT_OK != ret){
		            SDL_MIN_LOG("rtk_led_mode_set return %d\n", ret);
		            rt =  CS_E_ERROR;
		            goto END;
		        }
				ret = rtk_led_groupConfig_set(LED_GROUP_2, LED_CONFIG_LINK_ACT);
				if(GT_OK != ret){
		            SDL_MIN_LOG("rtk_led_groupConfig_set return %d\n", ret);
		            rt =  CS_E_ERROR;
		            goto END;
		        }
			}

        ret = rtk_port_rgmiiDelayExt0_set(0, 5);
        if(GT_OK != ret){
            SDL_MIN_LOG("rtk_port_rgmiiDelayExt0_set return %d\n", ret);
            rt =  CS_E_ERROR;
            goto END;
        }
#endif
    }

    for(port=0; port<UNI_PORT_MAX; port++)
    {
    	gt_getswitchunitbylport(port, &unit, &hwport);
    	if(GT_OK != gprtPhyIntEnable(QD_DEV_PTR, hwport, GT_LINK_STATUS_CHANGED, GT_TRUE))
    	{

			SDL_MIN_LOG("gprtPhyIntEnable type 0x%x, code %d return %d\n", GT_LINK_STATUS_CHANGED, GT_TRUE, ret);
			goto END;
		}
    }
    
    sdl_int_handler_reg(SDL_INT_UNI_LINK_CHANGE, __sdl_port_int_process);
    
#ifdef HAVE_SDL_CTC
    cs_polling_handler_reg(__sdl_port_auto_neg_polling_handler);
#endif

END:
    return rt;
    
}

/****************************************************************************/
/* $rtn_hdr_start  epon_request_onu_port_storm_control_set                  */
/* CATEGORY   : Device                                                      */
/* ACCESS     : Public                                                      */
/* BLOCK      : SAL                                                         */
/* CHIP       : 8030                                                        */
cs_status epon_request_onu_port_storm_ctrl_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              port_id,
    CS_IN cs_sdl_rl_mod_e           storm_mod,
    CS_IN cs_sdl_storm_ctrl_e       storm_type,
    CS_IN cs_sdl_policy_t           *rate
)
{
    cs_status rc = CS_E_OK;
    cs_aal_rate_limit_t storm;
    cs_aal_rate_limit_msk_t st_msk;
    GT_LPORT port;
//    rtk_rate_t rtk_rate;

    if (NULL == rate) {
        SDL_MIN_LOG("In %s, rate is NULL pointer\n", __FUNCTION__);
        rc = CS_E_PARAM;
        goto end;
    }

    if (!((storm_mod == SDL_RL_MOD_BPS) || (storm_mod == SDL_RL_MOD_PPS))) {
        SDL_MIN_LOG("In %s, storm_mod %d is not supported\n", __FUNCTION__, storm_mod);
		cs_printf("not support strom_mod bps\n");
        rc = CS_E_PARAM;
        goto end;
    }

    if ((storm_type < SDL_STORM_TYPE_BC) || (storm_type > SDL_STORM_TYPE_DISABLE)) {
        SDL_MIN_LOG("In %s, storm_type %d is not supported\n", __FUNCTION__, storm_type);
        rc = CS_E_PARAM;
        goto end;
    }
    if (port_id != CS_DOWNLINK_PORT) {
        UNI_PORT_CHECK(port_id); 
        
        port = (GT_LPORT)(port_id - 1);
        /* mtodo replace rtk api by mrv api
        rtk_rate = ((rtk_rate_t)rate->rate) / STORM_GRANULARITY * STORM_GRANULARITY;
        if(rtk_rate < MIN_STORM_RATE){
            rtk_rate = MIN_STORM_RATE;
        }

        if ( rate->enable==0) {
            storm_type=SDL_STORM_TYPE_DISABLE;
        }
        rc = rtk_rate_shareMeter_set(PORT_STORM_METER_ID_START + port, rtk_rate, ENABLED);
        if (rc) {
            goto end;
        }
        switch (storm_type) {
        case SDL_STORM_TYPE_DISABLE:
            rc = rtk_storm_controlRate_set(port, STORM_GROUP_BROADCAST, PORT_STORM_METER_ID_END, 1, MODE1);
            if (rc) {
                goto end;
            }
            rc = rtk_storm_controlRate_set(port, STORM_GROUP_UNKNOWN_MULTICAST, PORT_STORM_METER_ID_END, 1, MODE1);
            if (rc) {
                goto end;
            }
            rc = rtk_storm_controlRate_set(port, STORM_GROUP_UNKNOWN_UNICAST, PORT_STORM_METER_ID_END, 1, MODE1);
            if (rc) {
                goto end;
            }
            break;
        case SDL_STORM_TYPE_BC:
            rc = rtk_storm_controlRate_set(port, STORM_GROUP_BROADCAST, PORT_STORM_METER_ID_START + port, 1, MODE1);
            if (rc) {
                goto end;
            }
            rc = rtk_storm_controlRate_set(port, STORM_GROUP_UNKNOWN_MULTICAST, PORT_STORM_METER_ID_END, 1, MODE1);
            if (rc) {
                goto end;
            }
            rc = rtk_storm_controlRate_set(port, STORM_GROUP_UNKNOWN_UNICAST,PORT_STORM_METER_ID_END, 1, MODE1);
            if (rc) {
                goto end;
            }
            break;

        case SDL_STORM_TYPE_BC_UMC:

            rc = rtk_storm_controlRate_set(port, STORM_GROUP_BROADCAST, PORT_STORM_METER_ID_START + port, 1, MODE1);
            if (rc) {
                goto end;
            }
            rc = rtk_storm_controlRate_set(port, STORM_GROUP_UNKNOWN_MULTICAST, PORT_STORM_METER_ID_START + port, 1, MODE1);
            if (rc) {
                goto end;
            }
            rc = rtk_storm_controlRate_set(port, STORM_GROUP_UNKNOWN_UNICAST, PORT_STORM_METER_ID_END, 1, MODE1);
            break;
        case SDL_STORM_TYPE_BC_UMC_UUC:
            rc = rtk_storm_controlRate_set(port, STORM_GROUP_BROADCAST, PORT_STORM_METER_ID_START + port, 1, MODE1);
            if (rc) {
                goto end;
            }
            rc = rtk_storm_controlRate_set(port, STORM_GROUP_UNKNOWN_MULTICAST, PORT_STORM_METER_ID_START + port, 1, MODE1);
            if (rc) {
                goto end;
            }
            rc = rtk_storm_controlRate_set(port, STORM_GROUP_UNKNOWN_UNICAST, PORT_STORM_METER_ID_START + port, 1, MODE1);
            if (rc) {
                goto end;
            }
            break;
        default:
            break;
        }
        */
        __storm_type[port] = storm_type;
        __sc_rate_limit[port].rate = rate->rate;
    } else {
        memset(&storm, 0, sizeof(storm));
        st_msk.u = 0;
        storm.ctrl.s.enable = rate->enable;
        storm.ctrl.s.rlmode = storm_mod;
        storm.rate = rate->rate;
        storm.cbs = MAX_BURST_SIZE;
        st_msk.s.rlmode = 1;
        st_msk.s.rate = 1;
        st_msk.s.cbs = 1;
        st_msk.s.enable = 1;

        rc = aal_flow_rate_limit_set(AAL_RATE_LIMIT_GE_UMC, &st_msk, &storm);
        if (rc) {
            SDL_MIN_LOG("In function:%s,line:%d invoke aal_flow_rate_limit_set fail!\n", __FUNCTION__, __LINE__);
            goto end;
        }

        rc = aal_flow_storm_ctrl_set(AAL_PORT_ID_GE, (cs_aal_storm_ctrl_mod_t)storm_type);
        if (rc) {
            SDL_MIN_LOG("In function:%s,line:%d invoke aal_flow_storm_ctrl_set fail!\n", __FUNCTION__, __LINE__);
            goto end;
        }
    }
end:
    return rc;
}

/****************************************************************************/
/* $rtn_hdr_start  epon_request_onu_port_storm_control_get                  */
/* CATEGORY   : Device                                                      */
/* ACCESS     : Public                                                      */
/* BLOCK      : SAL                                                         */
/* CHIP       : 8030                                                        */
cs_status epon_request_onu_port_storm_ctrl_get(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              port_id,
    CS_OUT cs_sdl_rl_mod_e          *storm_mod,
    CS_OUT cs_sdl_storm_ctrl_e      *storm_type,
    CS_OUT cs_sdl_policy_t          *rate
)
{
    cs_aal_rate_limit_t storm;
    cs_aal_storm_ctrl_mod_t type = AAL_STORM_CTRL_MODE_DIS;
    cs_aal_rate_limit_msk_t rl_msk;
    cs_status rc = CS_E_OK;


    if (!storm_mod || !storm_type || !storm_type) {
        SDL_MIN_LOG("In %s,  Input parameter is NULL pointer\n", __FUNCTION__);
        rc = CS_E_PARAM;
        goto end;
    }

    if (port_id != CS_DOWNLINK_PORT) {
        UNI_PORT_CHECK(port_id);    

        rate->rate = __sc_rate_limit[port_id -1].rate;
	
        *storm_type = __storm_type[port_id -1];
        *storm_mod = SDL_RL_MOD_BPS;

    } else {
        rl_msk.u = 0;
        rl_msk.s.rate = 1;

        rc = aal_flow_rate_limit_get(AAL_RATE_LIMIT_GE_UMC, &rl_msk, &storm);
        if (rc) {
            goto end;
        }
        rc = aal_flow_storm_ctrl_get(AAL_PORT_ID_GE, &type);
        if (rc) {
            goto end;
        }

        *storm_mod = (cs_sdl_rl_mod_e)storm.ctrl.s.rlmode;
        *storm_type = (cs_sdl_storm_ctrl_e)type;

        rate->enable = storm.ctrl.s.enable;
        rate->rate = storm.rate;
    }
end:
    return rc;
}



cs_status epon_request_onu_port_isolation_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_boolean                enable
)
{
//    rtk_portmask_t portmask;
    GT_STATUS  ret  = 0;
    cs_status      rt = CS_E_OK;
    
    if (enable > 1) {
        SDL_MIN_LOG("error param\n");
        rt = CS_E_PARAM;
        goto END;
    }


    FOR_UNIT_START(GT_32, unit)

    ret = gvlnSetPortIsolate(QD_DEV_PTR, enable);

    FOR_UNIT_END

    __port_isolation = enable;

END:
    return rt;
}

cs_status epon_request_onu_port_isolation_get(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_OUT cs_boolean               *enable
)
{
    if (NULL == enable) {
        SDL_MIN_LOG(" enable is a NULL pointer\n");
        return CS_E_PARAM;
    }

    *enable = __port_isolation;

    return CS_E_OK;
}

cs_status epon_request_onu_port_hello_mac_set(
    CS_IN cs_callback_context_t      context,
    CS_IN cs_int32                   device_id,
    CS_IN cs_int32                   llidport,
    CS_IN cs_port_id_t               port_id,
    CS_IN cs_mac_t             *mac
)
{
    cs_aal_pkt_spec_udf_msk_t msk;
    cs_aal_pkt_spec_udf_t     cfg;
    cs_status                 ret = CS_E_OK;

    if (NULL == mac) {
        SDL_MIN_LOG("In %s, mac is NULL pointer\n", __FUNCTION__);
        return CS_E_PARAM;
    }

    if (port_id > CS_VOIP_PORT_ID) {
        SDL_MIN_LOG("In %s, port_id %d is not supported\n", __FUNCTION__, port_id);
        return CS_E_PARAM;
    }

    memset(&cfg, 0, sizeof(cs_aal_pkt_spec_udf_t));
    msk.u32 = 0;
    msk.s.hello_da = 1;
    cfg.hello_da.addr[0] = mac->addr[0];
    cfg.hello_da.addr[1] = mac->addr[1];
    cfg.hello_da.addr[2] = mac->addr[2];
    cfg.hello_da.addr[3] = mac->addr[3];
    cfg.hello_da.addr[4] = mac->addr[4];
    cfg.hello_da.addr[5] = mac->addr[5];

    switch (port_id) {
    case CS_PON_PORT_ID:
        ret = aal_pkt_special_udf_set(AAL_PORT_ID_PON, msk, &cfg);
        break;

    case CS_UNI_PORT_ID1:
        ret = aal_pkt_special_udf_set(AAL_PORT_ID_GE, msk, &cfg);
        break;

    case CS_MGMT_PORT_ID:
        ret = aal_pkt_special_udf_set(AAL_PORT_ID_MII0, msk, &cfg);
        break;

    default:
        SDL_MIN_LOG("In %s, port_id %d is not supported\n", __FUNCTION__, port_id);
        ret = CS_E_PARAM;
    }

    return ret;
}

#if 0

/****************************************************************/
/* $rtn_hdr_start  epon_request_onu_dot1p_remark_set        */
/* CATEGORY   : Device                                          */
/* ACCESS     : Public                                          */
/* BLOCK      : SAL                                             */
/* CHIP       : 8030                                            */
cs_status epon_request_onu_dot1p_remark_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              port_id,
    CS_IN cs_pri_remark_t          *remark
)

{
    GT_LPORT port;
    GT_STATUS rt;
    cs_uint8 index, index1;
    cs_uint8 filter_id;
    cs_uint8 port_msk = 0;
    cs_uint8 temp_pri;

    cs_status ret = CS_E_OK;
    if (!remark) {
        SDL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d", __FILE__, __LINE__);
        ret = CS_E_PARAM;
        goto end;
    }
    if (port_id < CS_UNI_PORT_ID1 || port_id > CS_UNI_PORT_ID4) {
        SDL_MIN_LOG("In %s, port_id %d is not supported!\n", __FUNCTION__, port_id);
        return CS_E_PARAM;
    }
    port = (GT_LPORT)(port_id - 1);

    for (index = 0; index < 8; index++) {
        filter_id = __acl_occupy[port][index];
        if (filter_id != 0) {
            rt = rtk_filter_igrAcl_cfg_del(filter_id);
            if (rt) {
                ret = (cs_status)rt;
                goto end;
            }
            __acl_occupy[port][index] = 0;
            for (index1 = 0; index1 < 4; index1++) {
                if (__acl_occupy[index1][index] == filter_id) {
                    port_msk |= (1 << index1);
                    temp_pri = __dot1p_remark[index1].priority[index];
                }
            }
            if (port_msk != 0) {
                ret = __add_pri_remark_acl(filter_id, port_msk, index, temp_pri);
                if (ret) {
                    goto end;
                }
            } else {
                __acl_avl &= ~(1 << filter_id);

            }
        }

    }

    if (remark->en == 1) {

        for (index = 0; index < 8; index++) {

            if ((index&7) == remark->priority[index&7]) {
                continue;
            }
            port_msk = 0;
            for (index1 = 0; index1 < 4; index1++) {

                //cs_printf("index1 = %d,index = %d, filter_id = %d\n", index1, index,filter_id);
                if (__acl_occupy[index1][index&7] != 0) {
                    filter_id = __acl_occupy[index1][index&7];
                    // cs_printf("__dot1p_remark[%d].priority[%d] = %d\n",index1, index&7,__dot1p_remark[index1].priority[index&7]);
                    //cs_printf("remark->priority[%d] = %d\n",index&7,remark->priority[index&7]);

                    if (__dot1p_remark[index1].priority[index&7] == remark->priority[index&7]) {
                        rt = rtk_filter_igrAcl_cfg_del(filter_id);
                        if (rt) {
                            ret = (cs_status)rt;
                            goto end;
                        }
                        port_msk |= (1 << index1);
                    }
                }
            }
            if (port_msk == 0) {
                filter_id = __check_free_id();
            }
            port_msk |= (1 << port);
            //__acl_avl = 0;
            temp_pri = remark->priority[index&7];
            ret = __add_pri_remark_acl(filter_id, port_msk, index, temp_pri);
            if (ret) {
                goto end;
            }
            __acl_avl |= 1 << filter_id;

            __acl_occupy[port][index&7] = filter_id;
        }
    }

    __dot1p_remark[port].en = remark->en;

    // cs_printf("set remark->en = %d\n",remark->en);
    for (index = 0; index < 8; index++) {
        __dot1p_remark[port].priority[index] = remark->priority[index];
    }
end:
    return ret;
}


/****************************************************************/
/* $rtn_hdr_start  epon_request_onu_dot1p_remark_get        */
/* CATEGORY   : Device                                          */
/* ACCESS     : Public                                          */
/* BLOCK      : SAL                                             */
/* CHIP       : 8030                                            */
cs_status epon_request_onu_dot1p_remark_get(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                device_id,
    CS_IN cs_int32                llidport,
    CS_IN cs_port_id_t              port_id,
    CS_OUT cs_pri_remark_t         *remark
)

{

    GT_LPORT port;
    cs_uint8 index;
    cs_status ret = CS_E_OK;

    if (!remark) {
        SDL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d", __FILE__, __LINE__);
        ret = CS_E_PARAM;
        goto end;
    }
    if (port_id < CS_UNI_PORT_ID1 || port_id > CS_UNI_PORT_ID4) {
        SDL_MIN_LOG("In %s, port_id %d is not supported!\n", __FUNCTION__, port_id);
        ret = CS_E_PARAM;
        goto end;
    }
    port = (GT_LPORT)(port_id - 1);

    remark->en = __dot1p_remark[port].en;
// cs_printf("get remark->en = %d\n",remark->en);
    for (index = 0; index < 8; index++) {
        remark->priority[index] = __dot1p_remark[port].priority[index];
    }
end:
    return ret;

}
#endif

cs_status epon_request_onu_port_mirror_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              mirror_port,
    CS_IN cs_uint32                 rx_port_msk,
    CS_IN cs_uint32                 tx_port_msk
)
{
    GT_LPORT     port, i;
    cs_status      rt = CS_E_OK;
    
	GT_32 unit, hwport;

    if ((mirror_port < CS_UNI_PORT_ID1) ||
            ((mirror_port > CS_UNI_PORT_ID4) && (mirror_port != CS_UPLINK_PORT))) {
        SDL_MIN_LOG("port_id %d is not supported!\n", mirror_port);
		cs_printf("mirror port error");
        rt = CS_E_PARAM;
        goto END;
    }

    if (rx_port_msk > MIRROR_MAX_PORT_MASK) {
        SDL_MIN_LOG("rx_port_msk: %d is not supported!\n", rx_port_msk);
		cs_printf("rx_port_msk:0x%02x\n",rx_port_msk);
        rt = CS_E_PARAM;
        goto END;
    }

    if (tx_port_msk > MIRROR_MAX_PORT_MASK) {
        SDL_MIN_LOG("tx_port_msk %d is not supported!\n", tx_port_msk);
		cs_printf("tx_port_msk:0x%02x\n",tx_port_msk);
        rt = CS_E_PARAM;
        goto END;
    }

    /*rx_port_msk should equal to tx_port_msk if neither of them is 0*/
    if ((rx_port_msk != tx_port_msk) && (tx_port_msk != 0) && (rx_port_msk != 0)) {
        SDL_MIN_LOG("port_mask is set uncorrectly!\n");
        rt = CS_E_PARAM;
        goto END;
    }

    /*mirror port != source port*/
    port = L2P_PORT(mirror_port);
    if (mirror_port == CS_UPLINK_PORT) {
        if (((tx_port_msk &(1 << UNI_PORT_MAX)) > 0) || ((rx_port_msk &(1 << UNI_PORT_MAX)) > 0)) {
            SDL_MIN_LOG("mirror_port %d should not be included into source port!\n", mirror_port);
            rt = CS_E_PARAM;
            goto END;
        }
    }
    else{
        if (((tx_port_msk &(1 << port)) > 0) || ((rx_port_msk &(1 << port)) > 0)) {
            SDL_MIN_LOG("mirror_port %d should not be included into source port!\n", mirror_port);
            rt = CS_E_PARAM;
            goto END;
        }
    }

	gt_getswitchunitbylport(port, &unit, &hwport);
	if(gsysSetIngressMonitorDest(QD_DEV_PTR, hwport) != GT_OK)
	{
		rt = CS_E_ERROR;
		goto END;
	}
	if(gsysSetEgressMonitorDest(QD_DEV_PTR, hwport) != GT_OK)
	{
		rt = CS_E_ERROR;
		goto END;
	}


    i=0;
    while(rx_port_msk)
    {
    	if(rx_port_msk & 0x1)
    	{
    		gt_getswitchunitbylport(i, &unit, &hwport);
    		if( GT_OK != gprtSetIngressMonitorSource(QD_DEV_PTR, hwport, GT_TRUE))
    		{
    			rt = CS_E_ERROR;
    			goto END;
    		}
    	}
    	rx_port_msk >>= 1;
    	i++;
    }

    i=0;
    while(tx_port_msk)
    {
    	if(tx_port_msk & 0x1)
    	{
    		gt_getswitchunitbylport(i, &unit, &hwport);
    		if( GT_OK != gprtSetEgressMonitorSource(QD_DEV_PTR, hwport, GT_TRUE))
    		{
    			rt = CS_E_ERROR;
    			goto END;
    		}
    	}
    	tx_port_msk >>= 1;
    	i++;
    }
END:
    return rt;

}

cs_status epon_request_onu_port_mirror_get(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_OUT cs_port_id_t              *mirror_port,
    CS_OUT cs_uint32                 *rx_port_msk,
    CS_OUT cs_uint32                 *tx_port_msk
)
{
    GT_LPORT     port;
    cs_status rt = CS_E_OK;
    GT_BOOL 	mode;
    
    if ((NULL == mirror_port) || (NULL == rx_port_msk) || (NULL == tx_port_msk)) {
        SDL_MIN_LOG("NULL pointer\n");
        rt = CS_E_PARAM;
        goto END;
    }

    *rx_port_msk = 0;
    *tx_port_msk = 0;

    for(port = 0; port < UNI_PORT_MAX; port++)
    {
    	GT_32 unit, hwport;

    	gt_getswitchunitbylport(port, &unit, &hwport);
    	if((GT_OK == gprtGetIngressMonitorSource(QD_DEV_PTR, hwport, &mode)) &&
    			(mode == GT_TRUE))
    	{
    		*rx_port_msk |= 1<<port;
    	}

    	if((GT_OK == gprtGetEgressMonitorSource(QD_DEV_PTR, hwport, &mode)) &&
    			(mode == GT_TRUE))
    	{
    		*tx_port_msk |= 1<<port;
    	}
    }

    FOR_UNIT_START(GT_32, unit)
    if(gsysGetIngressMonitorDest(QD_DEV_PTR, &port) == GT_OK)
    {
    	*mirror_port = P2L_PORT(port);
    }

    FOR_UNIT_END

END:
    return rt;
}

cs_status epon_request_onu_big_burst_size_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_boolean                enable
)
{
    cs_status ret = CS_E_OK;

    if (enable == TRUE) {
        ret = (cs_status)cs_plat_ssp_switch_write(context, 0, 0, BURST_CONTROL_REG, 0x12);
    } else if (enable == FALSE) {
        ret = (cs_status)cs_plat_ssp_switch_write(context, 0, 0, BURST_CONTROL_REG, 0xed);
    } else {
        ret = CS_E_PARAM;
    }
    return ret;
}

cs_status epon_request_onu_big_burst_size_get(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_boolean                *enable
)
{
    cs_status ret = CS_E_OK;
    cs_uint32 valu32;

    if (!enable) {
        ret =  CS_E_PARAM;
        goto end;
    }
    ret = (cs_status)cs_plat_ssp_switch_read(context, 0, 0, BURST_CONTROL_REG, &valu32);
    if (ret) {
        goto end;
    }
    if (valu32 == 0xed) {
        *enable = FALSE;
    } else {
        *enable = TRUE;
    }
end:
    return ret;
}

#if 1
extern cs_status port_storm_limit_set(cs_port_id_t port, cs_sdl_rl_mod_e mode, cs_sdl_storm_ctrl_e type, cs_sdl_policy_t rate)
{
#if 0
	cs_printf("in port_storm_limit_set\n");
#endif
	cs_status ret = CS_E_OK;
	cs_callback_context_t     context;
	cs_int32                  device_id = 0;
	cs_int32                  llidport = 0;;
	cs_port_id_t              port_id;
	cs_sdl_rl_mod_e           storm_mod;
	cs_sdl_storm_ctrl_e       storm_type;
	cs_sdl_policy_t           sdl_rate;

	port_id = port;
	storm_mod = mode;
	storm_type = type;
	sdl_rate = rate;
	ret = epon_request_onu_port_storm_ctrl_set(context, device_id, llidport, port_id, storm_mod, storm_type, &sdl_rate);
	return ret;
}
#endif

