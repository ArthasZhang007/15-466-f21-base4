#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <thread>
#include <chrono>

using namespace std;

typedef std::map<std::string, std::string> atr_type;

class Player{
    public:
        //item -> attribute
        atr_type atr;
};
class State{
public:
    std::string description;
    class Choices{
    public:
        std::string description = "";
        function<State(atr_type &)> trans = nullptr;
    };
    std::vector<Choices> choices;
    size_t current_choice = 0;

    //last choice 
    void pushup(){current_choice = (current_choice + choices.size() - 1) % choices.size();}
    //next choice
    void pushdown(){current_choice = (current_choice + 1) % choices.size();}
    //actually choose (current)
    State choose(atr_type &atr){
        if(choices[current_choice].trans == nullptr)
        {
            return *this;
        }
        return choices[current_choice].trans(atr);
    }
    //actually choose (indexing)
    State choose(atr_type &atr, size_t i){
        //assert(i >= 0);
        //assert(i < choices.size());
        if(choices[i].trans == nullptr)
        {
            return *this;
        }
        return choices[i].trans(atr);
    }
};

// ostream &operator <<(ostream &os, State S){
//     os << "Q : " << S.description << std::endl;
//     os << std::endl;
//     for(size_t i = 0; i<S.choices.size();i++){
//         os << i + 1 << " : " << S.choices[i].description << std::endl;
//     }
//     os << "Current Choice :" << S.current_choice + 1 <<std::endl;
//     return os;
// }

typedef State::Choices Choices;

// State id_state{
//     "id"
// };
// void message(std::string mes){
//     std::cout<<mes<<"\n\n";
//     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
// }
// State id_message(std::string mes){
//     message(mes);
//     return id_state;
// }

// State game_over;
// State bad_endding[5];
// State good_endding, real_endding;
// State north_room_1F, west_room_1F, east_room_1F, south_room_1F;
// State boss_room_2F;
// State middle_state_1F;
// State start_state;
/*
void state_init(){

good_endding = {
    "FREEDOM! YOU WIN!"
};
game_over = {
    "Game is over, try again!",
    {{
        "Restart!",
        [](atr_type &atr){
            atr.clear();
            return start_state;
        }
    }}
};
bad_endding[1] = {
    "After trembling for hours in the freezing unknown place...... you are frozen to death."
    ,
    {{
        "Game Over!",
        [](atr_type &atr){return game_over;}
    }}
};
bad_endding[2] = {
    "A sharp sword comes from the mirror and cut through your throat!"
    ,
    {{
        "Game Over!",
        [](atr_type &atr){return game_over;}
    }}
};
bad_endding[3] = {
    "The chest opens, and it swallows you!"
    ,
    {{
        "Game Over!",
        [](atr_type &atr){return game_over;}
    }}
};
bad_endding[4] = {
    "The chef locks your hand forcibly, 'IT IS A NICE HAND INDEED!', you hand is ripped off and you are bleed to death."
    ,
    {{
        "Game Over!",
        [](atr_type &atr){return game_over;}
    }}
};
start_state = {
    "You wake up from a long dream, it feels like a million year has passed. "
    "Everything around you is pure darkness"
    "Now you feel a warm thing in your hand..... it seems like a button, do you want to push it?"
    ,
    {
        {
            "Yes",
            [](atr_type &atr){return middle_state_1F;}
        }
        ,
        {
            "No",
            [](atr_type &atr){return bad_endding[1];}
        }
    }
};
middle_state_1F = {
    "Suddenly, multiple electric lights are on.... actually, wait. These are just kerosene lamps."
    "That's why the room is still so dim. Is this a mansion? prison? paradise? British classroom?"
    "You have absolutely no clue. The only thing you can see is that there are 8 doors around you, in 0', 45', 90' 135' 180' 225' "
    "270' 315' degree respectively, which one do you want to go?"
    ,
    {
        {
            "0",
            [](atr_type &atr){
                if(atr["east_room"] == "open")
                {
                    return east_room_1F;
                }
                else{
                    return id_message("East room is locked. You can not open it now");
                }
            }
        },
        {
            "45",
            [](atr_type &atr){
                return id_message("This is a fake room");
            }
        },
        {
            "90",
            [](atr_type &atr){
                return north_room_1F;
            }
        },
        {
            "135",
            [](atr_type &atr){
                return id_message("This is a fake room");
            }
        },
        {
            "180",
            [](atr_type &atr){
                if(atr["west_room"] == "open"){
                    return west_room_1F;
                }
                return id_message("West room is locked. You can not open it now");
            }
        },
        {
            "225",
            [](atr_type &atr){
                return id_message("This is a fake room");
            }
        },
        {
            "270",
            [](atr_type &atr){
                return south_room_1F;
            }
        },
        {
            "315",
            [](atr_type &atr){
                return id_message("This is a fake room");
            }
        }
    }
};

north_room_1F = {
    "You pushed the door and found it ...... broken and crashed into fragments on the floor. "
    "A nice artistic portrait of some ...... king is in front of you. He is on a horse, using a long sword cutting through the throat of evil enemy. "
    "His soldiers are motivated by this greatly and start to fight as well, even if the enemies are strong. "
    "Now on the left side of the portrait, there is a mirror, on the right side there is another one. The left is decorated with diamonds and gold "
    "with a cross on it, the right one is made by simple wood and has a demon symbol on it, what do you want to do next? ",
    {
        {
            "look at the left mirror",
            [](atr_type &atr){
                return bad_endding[2];
            }
        },
        {
            "look at the right mirror",
            [](atr_type &atr){
                atr["east_room"] = "open";
                message("You see a handsome boy with a great cowboy hat in the mirror, isn't it cool?");
                return id_message("You hear a crack sound outside... from east?");
            } 
        },
        {
            "exit the room",
            [](atr_type &atr){
                return middle_state_1F;
            } 
        }
    }
};

west_room_1F = {
    "It is so weird that door opens automatically, while just few minutes ago the door is as hard as a rock. "
    "There is a white board on the table, and a hand-written math formula, neat as printed. "
    "There is a chest with a coded lock of 4 digits on the side and you are a mathematic genius, how can this go wrong?"
    "   |x^2 - 15471 x + 77330 = 0|"
    ,
    {
        {
            "15213",
            [](atr_type &atr){
                return bad_endding[3];
            }
        },
        {
            "15312",
            [](atr_type &atr){
                return bad_endding[3];
            }
        },
        {
            "15462",
            [](atr_type &atr){
                return bad_endding[3];
            }
        },
        {
            "15466",
            [](atr_type &atr){
                atr["teddy bear"] = "yes";
                message(
                    "The chest is open and there is a cute teddy bear in it! It is soft, cool, and comfortable, "
                    "without even a tiny particle of dust on it. You put it into your bag. "
                );
                west_room_1F.description = "There is nothing in the room except the table and an empty chest";
                west_room_1F.choices.erase(west_room_1F.choices.begin(),west_room_1F.choices.begin()+4);
                east_room_1F.choices.emplace_back(
                    Choices
                    {
                        "how about this hand of my cute little teddy bear?",
                        [](atr_type &atr){
                            message(
                                "The chef cuts the hand of your teddy bear, "
                                "he laughs loudly, so loud that your eardrum is nearly broken. "
                                "Then the knife just drops to the ground, there is blood coming out of the arm of teddy bear. "
                            );
                            message(
                                "You hear multiple crashing sounds outside of the room."
                            );
                            middle_state_1F.description = "You see a convoluted stairway which you have not seen before";
                            middle_state_1F.choices.emplace_back(
                                Choices{
                                    "Go upstairs",
                                    [](atr_type &atr){
                                        return boss_room_2F;
                                    }
                                }
                            );
                            return middle_state_1F;
                        }
                    }
                );
                return west_room_1F;
            }
        },
        {
            "exit the room",
            [](atr_type &atr){
                return middle_state_1F;
            }
        }
    }
};

east_room_1F = {
    "A knife is cutting through raw meat in the kitchen, and the smell of cheesesteak around the corner is so good! Wait a minute, "
    "the knife is moving by itself! Is this magic, or just an invisible chef? While you are wondering, the chef speaks, \n"
    " |  CAN YOU GIVE ME A HAND? | ",
    {
        {
            "(The smell is so damn good!) I would love to !",
            [](atr_type &atr){
                return bad_endding[4];
            }
            
        },
        {
            "No thanks, my sir",
            [](atr_type &atr){
                message("You hear a crack sound outside... from west?");
                atr["west_room"] = "open";
                return middle_state_1F;
            }
        }
    }
};

south_room_1F = {
    "In front of you is a heavy bronze door that you can not open. "
    "However, there is a teardrop-shaped groove right in the middle of the door",
    {
        {
            "Go back to the middle hobby",
            [](atr_type &atr){
                return middle_state_1F;
            }
        },
        {
            "Try to put something in the groove",
            [](atr_type &atr){
                if(atr["teardrop crystal"] == "yes"){
                    message(
                        "the boundary matches exactly and the door opens and you see the light outside."
                        
                    );
                    return good_endding;
                }
                else{
                    message("You put your hand into it and nothing happen");
                    return id_state;
                }
            }
        }
    }
};

boss_room_2F = {
    "A huge fire dragon appears!",
    {
        {
            "Just beat it!",
            [](atr_type &atr){
                message("You are so powerful that you beat the dragon to death and it spills out a large teardrop crystal. ");
                
                message("Get item : teardrop crystal");
                atr["teardrop crystal"] = "yes";
                return middle_state_1F;
            }
        }
    }
};
}
*/

/*
攻略:
1. 选择90(北房间右边的普通镜子)
2. 出来后选择东边房间(0)，不给厨师手
3. 西边房间(180), 选择15466，获得泰迪玩偶
4. 东边房间把泰迪玩偶的手给隐形厨师切
5. 回到主大厅，上楼击败火龙，获得泪滴水晶钥匙
6. 用钥匙开南门，获得胜利
*/


//demo
// void init(){
//     State istate;
//     atr_type m;
//     istate = start_state;
//     m["power"] = "powerful";
//     int i = -1;
//     while(true)
//     {
//         std::cout<<istate<<std::endl;
//         if(istate.choices.empty())break;
//         std::cin>>i;
//         State next_state = istate.choose(m, i - 1);
//         if(next_state.description !="id"){
//             istate = next_state;
//         }
//     }
    
// }
// int main(){
//     state_init();
//     // init();
//     return 0;
// }