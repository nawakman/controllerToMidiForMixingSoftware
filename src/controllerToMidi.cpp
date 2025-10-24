#include "controllerToMidi.h"

ControllerToMidi::ControllerToMidi(string midiOutputName,int channel):TARGET_MIDI_PORT(wstring_convert<codecvt_utf8<wchar_t>>().from_bytes(midiOutputName)),CHANNEL(channel){//wstring_convert applies the conversion codecvt_utf8<wchar_t> from utf8 char of string to wchar
    UINT numDevices = midiOutGetNumDevs();
    if (numDevices == 0) {
        throw runtime_error("Error: No MIDI output devices found");
    }

    MIDIOUTCAPS moc; //data structure from winmm, contains data about a MIDI port //https://learn.microsoft.com/en-us/previous-versions/dd798467(v=vs.85)
    UINT targetDeviceId = (UINT)-1;
    
    for (UINT i = 0; i < numDevices; ++i) {//Iterate through all MIDI devices to find the one named "loopMIDI Port"
        if (midiOutGetDevCaps(i, &moc, sizeof(MIDIOUTCAPS)) == MMSYSERR_NOERROR) {//midiOutGetDevCaps() retrieves info about a MIDI port and put it in a MIDIOUTCAPS
            
            const int MAX_NAME_CHARS = 32;// Determine the maximum size for the name array (usually 32 chars for MIDIOUTCAPS)
            wchar_t wideName[MAX_NAME_CHARS];
            
            // Convert the ANSI/CHAR string (moc.szPname) to wide character (wchar_t)
            // CP_ACP specifies the system default ANSI code page.
            MultiByteToWideChar(CP_ACP, 0, moc.szPname, -1, wideName, MAX_NAME_CHARS);

            if (wcscmp(TARGET_MIDI_PORT.c_str(), wideName) == 0) {//if is the one we searched for
                targetDeviceId = i;
                break;
            }
        }
    }

    if (targetDeviceId == (UINT)-1) {
        throw runtime_error("Error: Could not find MIDI output port named "+midiOutputName);
    }

    MMRESULT result = midiOutOpen(&midiOutput, targetDeviceId, 0, 0, CALLBACK_NULL);//MMRESULT is litteraly just a UINT //https://tyleo.github.io/sharedlib/doc/winapi/mmsystem/type.MMRESULT.html

    if (result == MMSYSERR_NOERROR) {
        cout << "MIDI Output successfully initialized to " << midiOutputName << endl;
    } else {
       throw runtime_error("Error: Could not open "+midiOutputName);
        
    }
}

ControllerToMidi::~ControllerToMidi(){
    if (midiOutput != NULL) {//if not already closed for some reason
        midiOutClose(midiOutput);
        cout << "MIDI Output closed" << endl;
    }
}

void ControllerToMidi::sendMidiEvent(DWORD type, DWORD channel, DWORD number, DWORD value) {
    if (midiOutput != NULL) {
        DWORD status = type | channel;
        DWORD message = (value << 16) | (number << 8) | status;//Pack the 3 bytes into a single DWORD: 0x00(Data2)(Data1)(Status)
        midiOutShortMsg(midiOutput, message);
    }
}

void ControllerToMidi::bindValueToCC(DWORD value, DWORD ccNumber) {
    int midiValue = static_cast<DWORD>( floor( (static_cast<double>(value)/65535.0) * 127.0) );//Convert 16-bit axis (0-65535) to 7-bit MIDI CC (0-127)

    if (midiValue != prevCC[ccNumber]) {//Only send if the value has changed
        sendMidiEvent(MIDI_CC,CHANNEL, ccNumber, midiValue);
        prevCC[ccNumber] = midiValue;
    }
}

void ControllerToMidi::bindBooleanToNote(bool boolValue, DWORD note){
    if (boolValue != prevNoteState[note]) {//Only send if the state has changed
        if (boolValue) {
            sendMidiEvent(MIDI_NOTE_ON, CHANNEL, note, 100);// Button pressed (Note On, velocity 100)
        } else {
            sendMidiEvent(MIDI_NOTE_OFF, CHANNEL, note, 0);// Button released (Note Off, velocity 0)
        }
        prevNoteState[note] = boolValue;
    }
}

bool ControllerToMidi::isControllerConnected() {
    JOYCAPS jc;
    if (joyGetDevCaps(JOYSTICKID1, &jc, sizeof(jc)) == JOYERR_NOERROR) {//is any controller connected//takes the first controller of the connected controller list
        cout << "Found Controller: " << jc.szPname << endl;
        return true;
    } else {
        cout << "Controller not detected" << endl;
        return false;
    }
}

bool ControllerToMidi::getButtonState(int button){
    if(button>31){
        throw runtime_error("Error: button "+to_string(button)+" does not exist, buttons are from 1 to 32");
    }
    if(button>=28){//the d pad is detected as an axis but we want it as buttons //let's say they are the last 4 buttons 29,30,31,32
        return 2*(button-28)==(controllerInfo.dwPOV/4500);//only detects 4 postions not the 8 positions range
    }
    return controllerInfo.dwButtons & (1 << button);//1<<button is 2^button
}