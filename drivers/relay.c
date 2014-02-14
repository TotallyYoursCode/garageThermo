#include "relay.h"
void relay_lines_init(void){
   switch_off(Relay1);
   switch_off(Relay2);
   switch_off(Relay3);
   switch_off(Relay4);
   
   set_as_out(Relay1);
   set_as_out(Relay2);
   set_as_out(Relay3);
   set_as_out(Relay4);
}