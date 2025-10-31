#include <iostream>
#include <windows.h>
#include <mmsystem.h> // windows multimedia library (winmm)// Required for joyGetPosEx and midiOut... functions
#include <cmath>      // Required for floor()
#include <string>     // Required for wstring
#include <locale>    // Needed for std::codecvt (and often wstring_convert)
#include <codecvt>   // For std::wstring_convert and std::codecvt_utf8
#include <cwchar>     // Required for wcscmp

using namespace std;

class ControllerToMidi{
    HMIDIOUT midiOutput = NULL;   
    const wstring TARGET_MIDI_PORT;// The specific port name we are looking for
    const int CHANNEL;

    const DWORD MIDI_NOTE_ON = 0x90;//NOTE ON
    const DWORD MIDI_NOTE_OFF = 0x80;//NOTE OFF
    const DWORD MIDI_CC = 0xB0;//Control Change

    public:
    JOYINFOEX controllerInfo;
    // --- State Tracking (Prevent MIDI Flooding) ---
    // Initialize states with values that will force an update on first poll
    DWORD prevCC[128] = { 0 };
    bool prevNoteState[64] = { false };//128 notes in the MIDI spec, but a windows controller only has up to 32 buttons (but we want to send those buttons to 2 separate deck so *2)

    // --- FUNCTION DEFINITIONS ---
    ControllerToMidi(string midiOutputName,int channel);
    ~ControllerToMidi();

    /**
         * @brief Sends a 3-byte MIDI short message (Status, Data1, Data2).
         * @param type NOTE_ON,NOTE_OFF,control change(CC)
         * @param channel channel from 0 to 15
         * @param number note number or cc number
         * @param value note value or cc value
    */
    void sendMidiEvent(DWORD type, DWORD channel, DWORD number, DWORD value);
    /**
         * @brief Maps an axis value (0-65535) to a MIDI CC value (0-127) and sends if changed.
         * @param axisValue The raw axis position.
         * @param ccNumber The MIDI Control Change number to use.
    */
    void bindValueToCC(DWORD value, DWORD ccNumber);
    /**
         * @brief Maps a button state to a MIDI Note On/Off message and sends if changed.
         * @param buttonState True if pressed, false otherwise.
         * @param buttonIndex The 0-based index of the button (0 for button 1, etc.)
         * @param noteNumber The MIDI Note number to trigger.
    */
    void bindBooleanToNote(bool boolValue, DWORD note);
    // Function to check if a joystick is present
    bool isControllerConnected();
    //get button state (button 0 to 31)
    bool getButtonState(int button);
    //temporary button state override
    void setButtonState(int button, bool state);
};