#include <conio.h>// console input output // Required for _kbhit() and _getch()
#include <chrono>
#include <thread>
#include "src/controllerToMidi.h"
#include "src/buttonsName.h"
#include "src/actionsName.h"

using namespace std::chrono;

//IN THE COMPILATION COMMAND ADD "-lwinmm"
//it will allow the code to use winmm functions and types directly
//TO RUN USE 
//g++ -std=c++23 main.cpp src/controllerToMidi.cpp -lwinmm -o controllerToMidi

/*
    In virtualDJ, midi commands are displayed as following:
        -Control Change: <CHANNEL>-SLIDER<CC_NUMBER>
        -Note: <CHANNEL>-BUTTON<NOTE>
*/
DWORD allAxis[6+2*NB_CUSTOM_CONTROL]={32767,//left joystick X
                    32767,//left joystick Y
                    32767,//right joystick X
                    32767,//right joystick Y
                    0,//left trigger (tested on dualshock 4)
                    0,//right trigger (tested on dualshock 4)

                    32767,//left low
                    32767,//left mid
                    32767,//left high
                    32767,//left volume
                    32767,//left pitch
                    32767,//left filter
                    32767,//left flanger
                    32767,//left cut

                    32767,//right low
                    32767,//right mid
                    32767,//right high
                    32767,//right volume
                    32767,//right pitch
                    32767,//right filter
                    32767,//right flanger
                    32767//right cut
                    };


char controllerAxisToButtonBind[6]={ -1,DPAD_UP,-1,DPAD_UP,-1,-1 };//what button is currently assigned to each controller axis (by index) //-1 is not set //controller axis in the same order as in allAxis
//in buttonToAxisBindings only redirect to the first set (the "left")of custom axis, the code handles picking the second set for right joystick 
char buttonToAxisBindings[32]={FILTER,EQ_LOW,EQ_MID,EQ_HIGH,-1,-1,-1,-1,//so we can easily change bindings //-1 is not set //index 0 is button 1
                            FILTER,CUT,-1,-1,-1,FLANGER,-1,-1,//what custom axis to assign to joysticks depending on what button pressed after in edit bindings mode
                            -1,-1,-1,-1,-1,-1,-1,-1,//rows of 8
                            -1,-1,-1,-1,PITCH,-1,VOLUME,-1}; 

bool isRightDeck[6]={ false,false,true,true,false,true };//if true the axis modifies control of the right deck //so we can have both joystick controlling a single deck
bool isActiveDeckRight=false;//to which deck are the buttons mapped (e.g. so we can play/pause on either deck)
const double deadzonePercent = 0.1;
const DWORD DEADZONE=static_cast<DWORD>(round(deadzonePercent*32767));//32767 is half the max axis value 65535 (16 bits) 
const DWORD MIN_DEADZONE=32767-DEADZONE;
const DWORD MAX_DEADZONE=32767+DEADZONE;
const double JOYSTICK_SENSITIVITY=0.2;

//virtual MIDI cable created with https://www.tobias-erichsen.de/software/loopmidi.html
ControllerToMidi ctm("loopMIDI Port",0);//MIDI spec has 16 channels, you can use separate channels for separate instruments

int main(){
    if (!ctm.isControllerConnected()) {
        cout << "Exiting..." << endl;
        Sleep(3000);//let time for user to read
        return 0;
    }


    ctm.controllerInfo.dwSize = sizeof(JOYINFOEX);//mandatory else joyGetPosEx might return JOYERR_PARMS
    ctm.controllerInfo.dwFlags = JOY_RETURNALL;//https://learn.microsoft.com/en-us/windows/win32/api/joystickapi/ns-joystickapi-joyinfoex

    cout << "--- Generic Controller MIDI Sender (WinMM API) ---" << endl;
    cout << "Press any key to stop sending MIDI and exit..." << endl;

    bool editLeftJoystickYBind=false;
    bool editRightJoystickYBind=false; 

    // Game loop / Polling loop
    const milliseconds deltaTime(33);//represents a duration
    high_resolution_clock::time_point nextUpdate = high_resolution_clock::now();//represents a point in time

    while (!_kbhit()) {//if no key pressed
        MMRESULT result = joyGetPosEx(JOYSTICKID1, &ctm.controllerInfo);//get state of controller 1 //https://stackoverflow.com/questions/4416994/joystick-key-capture
        if (result == JOYERR_NOERROR) {
            //##### UPDATE CONTROLLER AXIS #####
            allAxis[0] = ctm.controllerInfo.dwXpos;
            allAxis[1] = ctm.controllerInfo.dwYpos;
            allAxis[2] = ctm.controllerInfo.dwZpos;
            allAxis[3] = ctm.controllerInfo.dwRpos;
            allAxis[4] = ctm.controllerInfo.dwVpos;
            allAxis[5] = ctm.controllerInfo.dwUpos;

            //##### DEADZONE JOYSTICKS #####
            for (int i = 0; i < 4; i++) {//triggers (5 & 6) don't need deadzones
                allAxis[i]=(allAxis[i]>MIN_DEADZONE && allAxis[i]<MAX_DEADZONE) ? 32767 : allAxis[i];
            }

            //##### EDIT BINDINGS MODE ##### 
            editLeftJoystickYBind=editLeftJoystickYBind || ctm.getButtonState(LEFT_TOP_TRIGGER);//once pressed stays true until manualy set to false
            editRightJoystickYBind=editRightJoystickYBind || ctm.getButtonState(RIGHT_TOP_TRIGGER);

            if(editLeftJoystickYBind){
                for(int button=0;button<32;button++){
                    if (button!=LEFT_TOP_TRIGGER && ctm.getButtonState(button)){//avoid the button we used to get in this mode
                        if(button==DPAD_LEFT){isRightDeck[1]=false;}else if(button==DPAD_RIGHT){isRightDeck[1]=true;}else{//deck selection
                            controllerAxisToButtonBind[1]=button;//what buttons is pressed first after pressing left top trigger
                            editLeftJoystickYBind=false;
                            ctm.setButtonState(button,false);//release button so it is only used for binding purposes and not sent afterward as regular input
                            Sleep(200);//let time for user to remove its finger from the button
                        }
                    }
                }
            }
            if(editRightJoystickYBind){
                for(int button=0;button<32;button++){
                    if (button!=RIGHT_TOP_TRIGGER && ctm.getButtonState(button)){//avoid the button we used to get in this mode
                        if(button==DPAD_LEFT){isRightDeck[3]=false;}else if(button==DPAD_RIGHT){isRightDeck[3]=true;}else{//deck selection
                            controllerAxisToButtonBind[3]=button;//what buttons is pressed first after pressing left top trigger
                            editRightJoystickYBind=false;
                            ctm.setButtonState(button,false);//release button so it is only used for binding purposes and not sent afterward as regular input
                            Sleep(200);//let time for user to remove its finger from the button
                        }
                    }
                }
            }

            //##### DECK SELECTION ##### 
            if(!(editLeftJoystickYBind || editRightJoystickYBind)){//if we press those buttons outside of binding edition mode
                if(ctm.getButtonState(DPAD_LEFT)){
                    isActiveDeckRight=false;
                }else if(ctm.getButtonState(DPAD_RIGHT)){
                    isActiveDeckRight=true;
                }
            }
            
            //##### USE JOYSTICK VALUES TO INCREMENT/DECREMENT CUSTOM AXIS #####
            for(int controllerAxis=0;controllerAxis<4;controllerAxis++){//for each joystick axis
                if (controllerAxisToButtonBind[controllerAxis]!=-1){//if controller axis is bound to a button
                    if(buttonToAxisBindings[controllerAxisToButtonBind[controllerAxis]]!=-1){//if the button is bound to a custom axis
                        char offsetIfRightDeck=isRightDeck[controllerAxis] ? NB_CUSTOM_CONTROL : 0;
                        DWORD &valueRef=allAxis[buttonToAxisBindings[controllerAxisToButtonBind[controllerAxis]]+offsetIfRightDeck];//reference to array elem
                        double offset=-1*(static_cast<double>(allAxis[controllerAxis])-32767.0);//double because it can be negative //-1 because i want it the other way
                        offset=round(copysign(pow(abs(offset/32767.0),3.0)*32767.0,offset))*JOYSTICK_SENSITIVITY;//so joystick value are non linear, the more distance to 0, the more growth 
                        valueRef=static_cast<DWORD>(clamp(static_cast<double>(valueRef)+offset,0.0,65535.0));//convert value to double for the addition (because we can add negative number but DWORD is unsingned) then back to DWORD
                    }else{
                        cout<<"binding does not exist: button "<<static_cast<int>(controllerAxisToButtonBind[controllerAxis]+1)<<" is not asociated to a custom axis"<<endl;
                    }
                }else{
                    //cout<<"binding does not exist: controller axis "<<controllerAxis<<endl;
                }
            }
            //##### SEND BUTTONS TO MIDI OUTPUT #####
            if(!(editLeftJoystickYBind || editRightJoystickYBind)){
                for(int buttonIndex=0;buttonIndex<32;buttonIndex++){
                    char offsetIfRightDeck=isActiveDeckRight ? 32 : 0;
                    ctm.bindBooleanToNote(ctm.getButtonState(buttonIndex),buttonIndex+offsetIfRightDeck );//from 0-BUTTON0 to 0-BUTTON31
                }
            }

            //##### SEND CUSTOM AXIS TO MIDI OUTPUT #####
            for(int customChannel=6;customChannel<6+2*NB_CUSTOM_CONTROL;customChannel++){
                ctm.bindValueToCC(allAxis[customChannel],customChannel);
                //cout<<customChannel<<": "<<allAxis[customChannel]<<", ";
            }
            //cout<<endl;

            //##### TIME MANAGEMENT FOR A SPECIFIC REFRESH RATE #####
            nextUpdate+=deltaTime;
            this_thread::sleep_until(nextUpdate);//more precise than windows Sleep()







        } else if (result == JOYERR_UNPLUGGED) {
            cout << "Controller disconnected. Exiting..." << endl;
            Sleep(3000);//let time for user to read
            break;
        }
    }
    //cleanly end execution
    if (_kbhit()) {//if any key pressed
        _getch();//reset the "key pressed" flag 
    }
    cout << "Exiting..." << endl;
    return 0;
}