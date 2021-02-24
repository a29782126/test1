#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include "mxio.h"
#include <string.h>
#include <pthread.h>

#pragma once

struct flags
{
    bool start_is_pushed;

    bool stop_is_pushed;
    bool emeg_is_pushed;
    bool chag_is_enable;
    bool chag_is_finish;
    bool result_sending;

    bool Charger_Malfunction;
    bool Vehicle_Malfunction;

    ///////////////////////////////////////////////////////////
    //lin

    bool f10240_Battery_Battery_overvoltage;
    bool f10241_Battery_undervoltage;
    bool f10242_Battery_current_deviation_error;
    bool f10243_High_battery_temperature;
    bool f10244_Battery_voltage_deviation_error;

    bool f10250_Vehicle_charging_enable;
    bool f10251_Vehicle_shift_position;
    bool f10252_Charging_system_error;
    bool f10253_vehicle_staus;
    bool f10254_Normal_stop_request_before_charging;

    bool f10800_Welding_detection;

    bool f10950_Charger_status;
    bool f10951_Charger_error;
    bool f10952_Energizing_state;
    bool f10953_Battery_incompatibility;
    bool f10954_Charging_system_error;
    bool f10955_Charging_stop_control;

    bool f11000_Dynamic_control;
    bool f11001_High_current_control;
    bool f11002_High_voltage_control;
    bool f11003_Reserved;

    bool f11800_Dynamic_control;
    bool f11801_High_current_control;
    bool f11802_High_voltage_control;
    bool f11803_Reserved;

    bool f11850_Operating_condition;
    bool f11851_Cooling_function_for_charging_cable;
    bool f11852_Current_limiting_function_for_charging_cable;
    bool f11853_Cooling_function_for_charging_connector;
    bool f11854_Current_limiting_function_for_charging_connector;
    bool f11855_Over_temperature_protection_for_charging_connector;
    bool f11856_Reliability_design_temperature_monitoring_function;

    bool f11860_Permission_to_reset_the_maximum_charging_time;

    ///////////////////////////////////////////////////////////
    bool TOT_01_enable;
    bool TOT_04_enable;
    bool TOT_07_enable;
    bool TOT_08_enable;
    bool TOT_14_enable;
    bool TOT_15_enable;

    bool TOT_01_check;
    bool TOT_04_check;
    bool TOT_07_check;
    bool TOT_08_check;
    bool TOT_14_check;
    bool TOT_15_check;

    bool Enable_Dynamic_Control; //From V1.1
    bool Enable_High_Current;    //From V1.2
    bool Enable_High_Voltage;    //From V2.0

    bool CH_SUB_10_enable;

    flags()
    {

        stop_is_pushed = false;
        emeg_is_pushed = false;
        chag_is_enable = false;
        chag_is_finish = false;
        result_sending = false;

        Vehicle_Malfunction = false;
        Charger_Malfunction = false;
        ////////////////////////lin
        f10240_Battery_Battery_overvoltage = false;
        f10241_Battery_undervoltage = false;
        f10242_Battery_current_deviation_error = false;
        f10243_High_battery_temperature = false;
        f10244_Battery_voltage_deviation_error = false;

        f10250_Vehicle_charging_enable = false;
        f10251_Vehicle_shift_position = false;
        f10252_Charging_system_error = false;
        f10253_vehicle_staus = false;
        f10254_Normal_stop_request_before_charging = false;

        f10800_Welding_detection = false;

        f10950_Charger_status = false;
        f10951_Charger_error = false;
        f10952_Energizing_state = false;
        f10953_Battery_incompatibility = false;
        f10954_Charging_system_error = false;
        f10955_Charging_stop_control = false;

        f11000_Dynamic_control = false;
        f11001_High_current_control = false;
        f11002_High_voltage_control = false;
        f11003_Reserved = false;

        f11800_Dynamic_control = false;
        f11801_High_current_control = false;
        f11802_High_voltage_control = false;
        f11803_Reserved = false;

        f11850_Operating_condition = false;
        f11851_Cooling_function_for_charging_cable = false;
        f11852_Current_limiting_function_for_charging_cable = false;
        f11853_Cooling_function_for_charging_connector = false;
        f11854_Current_limiting_function_for_charging_connector = false;
        f11855_Over_temperature_protection_for_charging_connector = false;
        f11856_Reliability_design_temperature_monitoring_function = false;

        f11860_Permission_to_reset_the_maximum_charging_time = false;

        /////////////////////////////

        TOT_01_enable = false;
        TOT_04_enable = false;
        TOT_07_enable = false;
        TOT_08_enable = false;
        TOT_14_enable = false;
        TOT_15_enable = false;

        TOT_01_check = false;
        TOT_04_check = false;
        TOT_07_check = false;
        TOT_08_check = false;
        TOT_14_check = false;
        TOT_15_check = false;

        CH_SUB_10_enable = true;
    }
    ~flags() {}
};
typedef struct _CAN_PARA
{
    bool CAN0_read_enable;
    bool CAN1_read_enable;
    bool CAN0_write_enable;
    bool CAN1_write_enable;
    int CANPort_0, CANPort_1;
    int time_tmp;
} CAN_PARA, *PCAN_PARA;
