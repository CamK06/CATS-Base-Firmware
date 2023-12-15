#include "version.h"
#include "config.h"

#include "gpio.h"
#include "serial.h"
#include "radio.h"
#include "settings.h"
#include "shell.h"
#include "util.h"
#include "cats/cats.h"
#include <string.h>

// TODO: USE FREERTOS

int main() {
    // Initialization
    //settings_load();
    serial_init(115200);
    
    // GPIO Setup
    gpio_setup(USB_LED_PIN);
    gpio_setup(TX_LED_PIN);
    gpio_setup(RX_LED_PIN);
    gpio_set_mode(USB_LED_PIN, GPIO_OUTPUT);
    gpio_set_mode(TX_LED_PIN, GPIO_OUTPUT);
    gpio_set_mode(RX_LED_PIN, GPIO_OUTPUT);

    radio_init();
    radio_set_channel(20); // 430.5MHz with current config   TODO: Make this generic so it works with any radio as channels may differ
    while(true) {
        radio_tx((uint8_t*)"Hejoigf;lsdfgjsdfgoijdsf;jogijidosfgj;iidsfgijodsfoijgsdfighisdufhgiosdfhgohdsfgouihdsfguidshfgiouhdsfgiuhdsfgiuohdsgiohdsiofhgidfuhgidfoshuosghiufdohodgufgihfdousgfhiougfshiouhgidfosudsfhugsiudfhge8r7uiudnfsogserufdfs8g7rueiufdhsg87ser9yghdfshg8sre7godfsghserygdhofsgiuohdsfgiuhodsfgsidfuhgsdfgisfdiguhdsguoeirsghrghesoiurghesrgusoierghsreioughseorighesuoirghseruighseriugohserioughesrioghuhserioghiuolloiidsfgijodsfoijgsdfighisdufhgiosdfhgohdsfgouihdsfguidshfgiouhdsfgiuhdsfgiuohdsgiohdsiofhgidfuhgidfoshuosghiufdohodgufgihfdousgfhiougfshiouhgidfosudsfhugsiudfhge8r7uiudnfsogserufdfs8g7rueiufdhsg87ser9yghdfshg8sre7godfsghserygdhofsgiuohdsfgiuhodsfgsidfuhgsdfgisfdiguhdsguoeirsghrghesoiurghesrgusoierghsreioughseorighesuoirghseruighseriugohserioughesrioghuhserioghiuolloiidsfgijodsfoijgsdfighisdufhgiosdfhgohdsfgouihdsfguidshfgiouhdsfgiuhdsfgiuohdsgiohdsiofhgidfuhgidfoshuosghiufdohodgufgihfdousgfhiougfshiouhgidfosudsfhugsiudfhge8r7uiudnfsogserufdfs8g7rueiufdhsg87ser9yghdfshg8sre7godfsghserygdhofsgiuohdsfgiuhodsfgsidfuhgsdfgisfdiguhdsguoeirsghrghesoiurghesrgusoierghsreioughseorighesuoirghseruighseriugohserioughesrioghuhserioghiuolloiidsfgijodsfoijgsdfighisdufhgiosdfhgohdsfgouihdsfguidshfgiouhdsfgiuhdsfgiuohdsgiohdsiofhgidfuhgidfoshuosghiufdohodgufgihfdousgfhiougfshiouhgidfosudsfhugsiudfhge8r7uiudnfsogserufdfs8g7rueiufdhsg87ser9yghdfshg8sre7godfsghserygdhofsgiuohdsfgiuhodsfgsidfuhgsdfgisfdiguhdsguoeirsghrghesoiurghesrgusoierghsreioughseorighesuoirghseruighseriugohserioughesrioghuhserioghiuolloiidsfgijodsfoijgsdfighisdufhgiosdfhgohdsfgouihdsfguidshfgiouhdsfgiuhdsfgiuohdsgiohdsiofhgidfuhgidfoshuosghiufdohodgufgihfdousgfhiougfshiouhgidfosudsfhugsiudfhge8r7uiudnfsogserufdfs8g7rueiufdhsg87ser9yghdfshg8sre7godfsghserygdhofsgiuohdsfgiuhodsfgsidfuhgsdfgisfdiguhdsguoeirsghrghesoiurghesrgusoierghsreioughseorighesuoirghseruighseriugohserioughesrioghuhserioghiuolloiidsfgijodsfoijgsdfighisdufhgiosdfhgohdsfgouihdsfguidshfgiouhdsfgiuhdsfgiuohdsgiohdsiofhgidfuhgidfoshuosghiufdohodgufgihfdousgfhiougfshiouhgidfosudsfhugsiudfhge8r7uiudnfsogserufdfs8g7rueiufdhsg87ser9yghdfshg8sre7godfsghserygdhofsgiuohdsfgiuhodsfgsidfuhgsdfgisfdiguhdsguoeirsghrghesoiurghesrgusoierghsreioughseorighesuoirghseruighseriugohserioughesrioghuhserioghiuolloiidsfgijodsfoijgsdfighisdufhgiosdfhgohdsfgouihdsfguidshfgiouhdsfgiuhdsfgiuohdsgiohdsiofhgidfuhgidfoshuosghiufdohodgufgihfdousgfhiougfshiouhgidfosudsfhugsiudfhge8r7uiudnfsogserufdfs8g7rueiufdhsg87ser9yghdfshg8sre7godfsghserygdhofsgiuohdsfgiuhodsfgsidfuhgsdfgisfdiguhdsguoeirsghrghesoiurghesrgusoierghsreioughseorighesuoirghseruighseriugohserioughesrioghuhserioghiuolloiidsfgijodsfoijgsdfighisdufhgiosdfhgohdsfgouihdsfguidshfgiouhdsfgiuhdsfgiuohdsgiohdsiofhgidfuhgidfoshuosghiufdohodgufgihfdousgfhiougfshiouhgidfosudsfhugsiudfhge8r7uiudnfsogserufdfs8g7rueiufdhsg87ser9yghdfshg8sre7godfsghserygdhofsgiuohdsfgiuhodsfgsidfuhgsdfgisfdiguhdsguoeirsghrghesoiurghesrgusoierghsreioughseorighesuoirghseruighseriugohserioughesrioghuhserioghiuolloiidsfgijodsfoijgsdfighisdufhgiosdfhgohdsfgouihdsfguidshfgiouhdsfgiuhdsfgiuohdsgiohdsiofhgidfuhgidfoshuosghiufdohodgufgihfdousgfhiougfshiouhgidfosudsfhugsiudfhge8r7uiudnfsogserufdfs8g7rueiufdhsg87ser9yghdfshg8sre7godfsghserygdhofsgiuohdsfgiuhodsfgsidfuhgsdfgisfdiguhdsguoeirsghrghesoiurghesrgusoierghsreioughseorighesuoirghseruighseriugohserioughesrioghuhserioghiuolloiidsfgijodsfoijgsdfighisdufhgiosdfhgohdsfgouihdsfguidshfgiouhdsfgiuhdsfgiuohdsgiohdsiofhgidfuhgidfoshuosghiufdohodgufgihfdousgfhiougfshiouhgidfosudsfhugsiudfhge8r7uiudnfsogserufdfs8g7rueiufdhsg87ser9yghdfshg8sre7godfsghserygdhofsgiuohdsfgiuhodsfgsidfuhgsdfgisfdiguhdsguoeirsghrghesoiurghesrgusoierghsreioughseorighesuoirghseruighseriugohserioughesrioghuhserioghiuolloiidsfgijodsfoijgsdfighisdufhgiosdfhgohdsfgouihdsfguidshfgiouhdsfgiuhdsfgiuohdsgiohdsiofhgidfuhgidfoshuosghiufdohodgufgihfdousgfhiougfshiouhgidfosudsfhugsiudfhge8r7uiudnfsogserufdfs8g7rueiufdhsg87ser9yghdfshg8sre7godfsghserygdhofsgiuohdsfgiuhodsfgsidfuhgsdfgisfdiguhdsguoeirsghrghesoiurghesrgusoierghsreioughseorighesuoirghseruighseriugohserioughesrioghuhserioghiuolloiidsfgijodsfoijgsdfighisdufhgiosdfhgohdsfgouihdsfguidshfgiouhdsfgiuhdsfgiuohdsgiohdsiofhgidfuhgidfoshuosghiufdohodgufgihfdousgfhiougfshiouhgidfosudsfhugsiudfhge8r7uiudnfsogserufdfs8g7rueiufdhsg87ser9yghdfshg8sre7godfsghserygdhofsgiuohdsfgiuhodsfgsidfuhgsdfgisfdiguhdsguoeirsghrghesoiurghesrgusoierghsreioughseorighesuoirghseruighseriugohserioughesrioghuhserioghiuollo World!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!", 1000);
        mcu_sleep(2500);
    }

    // Main Loop
    // TODO: Use interrupts instead of polling?
    bool usbConnected = false;
    while(true) {
        if(serial_connected() && !usbConnected) {
            usbConnected = true;
            serial_write(DEVICE_NAME "\n");
            serial_write("Firmware Version: " VERSION "\n");
            serial_write("Build: " BUILD_STR "\n");
            //shell_init();
            gpio_write(USB_LED_PIN, usbConnected);
            continue;
        } else if(!serial_connected() && usbConnected) {
            usbConnected = false;
            shell_terminate();
            gpio_write(USB_LED_PIN, usbConnected);
            continue;
        }
        //if(usbConnected)
        //    shell();
        mcu_sleep(1);
    }
}