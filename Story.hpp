#include "oops.hpp"

struct Story {
    State game_over;
    State bad_endding[5];
    State good_endding, real_endding;
    State north_room_1F, west_room_1F, east_room_1F, south_room_1F;
    State boss_room_2F;
    State middle_state_1F;
    State start_state;

    State id_state{
        "id"
    };
    void message(std::string mes){
        std::cout<<mes<<"\n\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    State id_message(std::string mes){
        message(mes);
        return id_state;
    }


    void state_init();

};