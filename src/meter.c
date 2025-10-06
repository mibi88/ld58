#include <meter.h>

#include <string.h>

int meter_init(Meter *meter, char *name, size_t size, size_t value) {
    static const char meter_start[] = "~\n";
    static const char meter_end[] = "~\n \n";

    size_t i;

    char c;

    meter->size = size;

    strcpy(meter->text, meter_start);

    meter_set_value(meter, value);

    strcpy(meter->text+METER_START+size*2, meter_end);

    for(i=METER_START+METER_END+size*2;(c = *name++);i+=2){
        meter->text[i] = c;
        meter->text[i+1] = '\n';
    }
    meter->text[i] = '\0';

    return 0;
}

void meter_set_value(Meter *meter, size_t value) {
    size_t i;

    for(i=METER_START;i<METER_START+meter->size*2;i+=2){
        meter->text[i] = meter->size-i/2 < value ? '#' : '.';
        meter->text[i+1] = '\n';
    }
}

void meter_free(Meter *meter) {
    /**/

    (void)meter;
}

