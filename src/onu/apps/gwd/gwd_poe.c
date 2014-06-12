#include "gwd_poe.h"
#include "sdl_peri_util.h"

#if (RPU_MODULE_POE == MODULE_YES)

unsigned int gwd_poe_thread_id = 0;
#define GWD_POE_THREAD_NAME "gwd_poe"
#define GWD_POE_THREAD_STACKSIZE               (2 * 4096)

unsigned int gulPoeEnable = 0;
unsigned char gucPoeDisablePerPort[NUM_PORTS_PER_SYSTEM - 1] = {0};
unsigned char gucPoedefaultconfig[NUM_PORTS_PER_SYSTEM - 1] = {0};

extern int cmd_onu_cpld_reg_cfg_set(struct cli_def *cli, char *command, char *argv[], int argc);
extern int gw_cli_interface_debug_terminal(struct cli_def *cli, char *command, char *argv[], int argc);

epon_return_code_t Gwd_onu_poe_exist_stat_set(unsigned int poe_stat)
{
    gulPoeEnable = poe_stat;
    return EPON_RETURN_SUCCESS;
}



int gwd_poe_cpld_read(unsigned int type,unsigned char* val)
{
    cs_callback_context_t    context;
    unsigned int ret = EPON_RETURN_SUCCESS;	

	ret += cs_plat_ssp_cpld_write(context, 0, 0, CPLD_OP_READ);
    ret += cs_plat_ssp_cpld_write(context,0,0,type);
    ret += cs_plat_ssp_cpld_read(context,0,0,val);
	return ret;
}
int gwd_poe_cpld_write(unsigned int type,unsigned char val)
{
    cs_callback_context_t    context;
    unsigned int ret = EPON_RETURN_SUCCESS;	
	
	ret += cs_plat_ssp_cpld_write(context, 0, 0, CPLD_OP_WRITE);
    ret += cs_plat_ssp_cpld_write(context,0,0,type);
    ret += cs_plat_ssp_cpld_write(context,0,0,val);
	return ret;
}


int onu_cpld_read_register(unsigned int type,unsigned char* val)
{

    unsigned int reg = 0;
    unsigned int ret = EPON_RETURN_SUCCESS;

    if(!val)
    {
        return EPON_RETURN_FAIL;
    }
    
    switch(type)
    {
        case GWD_CPLD_VERSION_REG:
        case GWD_CPLD_RESERVED_REG:
        case GWD_CPLD_POWER3_REG:
        case GWD_CPLD_RESET_REG:
        case GWD_CPLD_POWER2_REG:
        case GWD_CPLD_POWER_ALARM_REG:
        case GWD_CPLD_POWER1_REG:
        case GWD_CPLD_PROTECT_REG:
            reg = type;
            if(gwd_poe_cpld_read(reg,val) != EPON_RETURN_SUCCESS)
            {
                gw_log(GW_LOG_LEVEL_MINOR,"read cpld register(0x%02x) fail\n",reg);
            }
            
            break;
           default:
              ret = EPON_RETURN_FAIL;
                break;
    }
    if(ret)
    {
        gw_log(GW_LOG_LEVEL_MINOR,"read cpld register type error\n");
        return EPON_RETURN_FAIL;
    }
    return EPON_RETURN_SUCCESS;
}

int onu_cpld_write_register(unsigned int type,unsigned int val)
{
    unsigned int reg = 0;
    int ret = EPON_RETURN_SUCCESS;

    switch(type)
    {
        case GWD_CPLD_VERSION_REG:
        case GWD_CPLD_RESERVED_REG:
        case GWD_CPLD_POWER3_REG:
        case GWD_CPLD_RESET_REG:
        case GWD_CPLD_POWER2_REG:
        case GWD_CPLD_POWER_ALARM_REG:
        case GWD_CPLD_POWER1_REG:
            reg = type;
            
            if((ret += gwd_poe_cpld_write(GWD_CPLD_PROTECT_REG,UNLOCK_CPLD)) != EPON_RETURN_SUCCESS)
            {
                gw_log(GW_LOG_LEVEL_MINOR,"UNLOCK CPLD fail\n");
            }
            if((ret += gwd_poe_cpld_write(reg,val)) != EPON_RETURN_SUCCESS)
            {
                gw_log(GW_LOG_LEVEL_MINOR,"write cpld register(0x%02x) fail\n",reg);
            }
            if((ret += gwd_poe_cpld_write(GWD_CPLD_PROTECT_REG,LOCK_CPLD)) != EPON_RETURN_SUCCESS)
            {
                gw_log(GW_LOG_LEVEL_MINOR,"LOCK CPLD fail\n");
            }
            break;
           default:
              ret = EPON_RETURN_FAIL;
                break;
    }
    if(ret)
    {
        gw_log(GW_LOG_LEVEL_MINOR,"write cpld register type error\n");
        return EPON_RETURN_FAIL;
    }
    return EPON_RETURN_SUCCESS;
}




int gwd_poe_cpld_check()
{
    unsigned char val = 0;
    int ret = 0;
    ret = EPON_RETURN_EXIST_OK;
	
    if(onu_cpld_read_register(GWD_CPLD_VERSION_REG,&val) != EPON_RETURN_SUCCESS)
    {
        return EPON_RETURN_FAIL;
    }

    if(val < GWD_CPLD_VERSION_1)  
    {
        ret = EPON_RETURN_EXIST_ERROR;
    }
	    
    return ret;
}

void gwd_onu_poe_thread_hander()
{
    unsigned int poeexiststat =0;
    unsigned int uni_port_num = 0;
    unsigned int ulport =0;
    unsigned int port_stat = 0;
    unsigned int Pstate = 0;
    unsigned int ctl_state = 0;
        
    Gwd_onu_poe_exist_stat_get(&poeexiststat);
    if(poeexiststat)
    {        
        Gwd_onu_poe_cpld_init();
    }
    else
    {
        gw_log(GW_LOG_LEVEL_MINOR,"poe thread exit\n");
        return ;
    }

    uni_port_num = gw_onu_read_port_num();
        
    while(TRUE)
    {
        for(ulport = 1;ulport <= uni_port_num; ulport++)
        {
            if(call_gwdonu_if_api(LIB_IF_PORT_ADMIN_GET, 2, ulport, &port_stat) != EPON_RETURN_SUCCESS)
            {
                continue;
            }

            if(port_stat == PORT_ADMIN_UP)
            {
                if(Gwd_onu_port_poe_controle_stat_get(ulport,&ctl_state) != EPON_RETURN_SUCCESS)
                {
                    gw_log(GW_LOG_LEVEL_MINOR,"get port:%d  admin fail:\n",ulport);
                    continue;
                }
                else
                {
                    if(ctl_state == POE_PORT_ENABLE)
                    {
                        if(Gwd_onu_port_power_detect_get(ulport,&Pstate) != EPON_RETURN_SUCCESS)
                        {
                            gw_log(GW_LOG_LEVEL_MINOR,"get port:%d  power fail:\n",ulport);
                            continue;
                        }

                        if(Pstate == POE_POWER_DISABLE)
                        {
                            Gwd_onu_port_poe_operation_stat_set(ulport,PORT_ADMIN_DOWN);
                        }
                    }
                }
                                    
            }
            else
            {
                if(Gwd_onu_port_poe_controle_stat_get(ulport,&ctl_state) != EPON_RETURN_SUCCESS)
                {
                    continue;
                }
                else
                {
                    if(ctl_state == POE_PORT_ENABLE)
                    {
                        if(Gwd_onu_port_power_detect_get(ulport,&Pstate) != EPON_RETURN_SUCCESS)
                        {
                            gw_log(GW_LOG_LEVEL_MINOR,"get port:%d power fail\n",ulport);
                            continue;
                        }
                        if(Pstate == POE_POWER_ENABLE)
                        {
                             Gwd_onu_port_poe_operation_stat_set(ulport,PORT_ADMIN_UP);
                        }
                    }
                    else
                    {
                        Gwd_onu_port_poe_operation_stat_set(ulport,PORT_ADMIN_UP);
                    }
                    
                }
            }
        }

        gw_thread_delay(100);
    }
}

void gwd_poe_init()
{
	int ret;
	
	ret == gwd_poe_cpld_check();
	cs_printf("GWD CPLD VERSION 0x%x , %s !", val, ret==EPON_RETURN_EXIST_OK?"VALID":"INVALID");
	if(EPON_RETURN_EXIST_OK == ret)
	{
		Gwd_onu_poe_exist_stat_set(1);
	    cs_thread_create(&gwd_poe_thread_id, GWD_POE_THREAD_NAME, gwd_onu_poe_thread_hander, NULL, GWD_POE_THREAD_STACKSIZE, PORT_STATS_THREAD_PRIORITY, 0);
	}
	else
	{
		Gwd_onu_poe_exist_stat_set(0);
		return;
	}
}


#endif

