#ifdef USE_GPS
#ifndef CATS_FW_GPS_H
#define CATS_FW_GPS_H

typedef struct gps_position {
    double lat;
    double lon;
    float alt;
    
} gps_position_t;

void gps_init();
void gps_tick();

#endif // CATS_FW_GPS_H
#endif // USE_GPS