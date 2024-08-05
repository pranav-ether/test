/**
 * @file        lte.c
 * @author      Pranav (pranav@ethersys.ai)
 * @version     0.1
 * @date        2024-05-14
 *
 * @copyright Copyright (c) EtherSys 2024
 *
 */

#include "lte.h"

LOG_MODULE_REGISTER(LTE, LOG_LEVEL_INF);
K_SEM_DEFINE(lte_connected, 0, 1);

void lte_handler(const struct lte_lc_evt *const evt)
{
    switch (evt->type)
    {
    case LTE_LC_EVT_NW_REG_STATUS:
        if ((evt->nw_reg_status != LTE_LC_NW_REG_REGISTERED_HOME) &&
            (evt->nw_reg_status != LTE_LC_NW_REG_REGISTERED_ROAMING))
        {
            break;
        }
        LOG_INF("Network registration status: %s",
                evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_HOME ? "Connected - home network" : "Connected - roaming");
        k_sem_give(&lte_connected);
        break;
    default:
        break;
    }
}

int modem_configure(void)
{
    int err;

    LOG_INF("Connecting to LTE network");
    err = lte_lc_system_mode_set(LTE_LC_SYSTEM_MODE_NBIOT, LTE_LC_SYSTEM_MODE_PREFER_NBIOT_PLMN_PRIO);
    LOG_INF("lte_lc_system_mode_set: %d", err);
    if (err)
    {
        LOG_ERR("Failed to activate LTE, error: %d", err);
        return 0;
    }
    err = lte_lc_connect_async(lte_handler);
    LOG_INF("lte_lc_connect_async: %d", err);
    if (err)
    {
        LOG_ERR("Error in lte_lc_connect_async, error: %d", err);
        return err;
    }

    k_sem_take(&lte_connected, K_FOREVER);

    LOG_INF("Connected to LTE network");

    return err;
}

int init_modem(void)
{
    int err;
    err = lte_lc_func_mode_set(LTE_LC_FUNC_MODE_ACTIVATE_GNSS);
    if (err)
    {
        LOG_ERR("Failed to activate LTE, error: %d", err);
        return 0;
    }

    LOG_INF("Initializing modem library");
    err = nrf_modem_lib_init();
    if (err)
    {
        LOG_ERR("Failed to initialize the modem library, error: %d", err);
    }
    return err;
}