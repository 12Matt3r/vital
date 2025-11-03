#pragma once

#include <vector>
#include <functional>
#include <deque>

namespace vital {

struct MidiMessage {
    int status;
    int data1;
    int data2;
    double timestamp;
    int channel;
};

enum class MidiChannel {
    All = -1,
    Channel1 = 0,
    Channel2 = 1,
    Channel3 = 2,
    Channel4 = 3,
    Channel5 = 4,
    Channel6 = 5,
    Channel7 = 6,
    Channel8 = 7,
    Channel9 = 8,
    Channel10 = 9,
    Channel11 = 10,
    Channel12 = 11,
    Channel13 = 12,
    Channel14 = 13,
    Channel15 = 14,
    Channel16 = 15
};

enum class MidiStatus {
    NoteOff = 0x80,
    NoteOn = 0x90,
    PolyphonicAfterTouch = 0xA0,
    ControlChange = 0xB0,
    ProgramChange = 0xC0,
    ChannelAfterTouch = 0xD0,
    PitchBend = 0xE0,
    SystemExclusive = 0xF0,
    TimeCodeQuarterFrame = 0xF1,
    SongPositionPointer = 0xF2,
    SongSelect = 0xF3,
    TuneRequest = 0xF6,
    TimingClock = 0xF8,
    Start = 0xFA,
    Continue = 0xFB,
    Stop = 0xFC,
    ActiveSensing = 0xFE,
    SystemReset = 0xFF
};

enum class MidiControl {
    ModulationWheel = 1,
    BreathControl = 2,
    FootControl = 4,
    PortamentoTime = 5,
    MainVolume = 7,
    Balance = 8,
    Pan = 10,
    Expression = 11,
    DamperPedal = 64,
    Portamento = 65,
    Sostenuto = 66,
    SoftPedal = 67,
    HarmonyHQ = 98,
    HarmonyLQ = 99
};

class MidiInput {
public:
    MidiInput();
    ~MidiInput();

    void initialize();
    void shutdown();

    // Message processing
    void processByte(int byte);
    void processMessage(int status, int data1, int data2);
    void processMessage(const MidiMessage& message);
    
    // MIDI output (for MIDI THRU)
    void setMidiOutput(MidiOutput* output) { midi_output_ = output; }
    void setMidiThru(bool enabled) { midi_thru_enabled_ = enabled; }
    bool getMidiThru() const { return midi_thru_enabled_; }
    
    // Channel filtering
    void setChannelFilter(int channel);
    int getChannelFilter() const { return channel_filter_; }
    void setChannelEnabled(int channel, bool enabled);
    bool getChannelEnabled(int channel) const;
    
    // System real-time messages
    void processSystemRealTime(MidiStatus status);
    void startClock();
    void stopClock();
    void resetClock();
    
    // Callbacks for received messages
    std::function<void(int channel, int note, int velocity)> onNoteOn;
    std::function<void(int channel, int note)> onNoteOff;
    std::function<void(int channel, int note, int pressure)> onPolyAfterTouch;
    std::function<void(int channel, int controller, int value)> onControlChange;
    std::function<void(int channel, int program)> onProgramChange;
    std::function<void(int channel, int pressure)> onChannelAfterTouch;
    std::function<void(int channel, int value)> onPitchBend;
    std::function<void(MidiStatus status)> onSystemMessage;
    
    // Utilities
    MidiChannel getChannelFromStatus(int status) const;
    MidiStatus getStatusFromChannel(MidiChannel channel, int status_type) const;
    bool isChannelMessage(int status) const;
    bool isSystemMessage(int status) const;
    
    // MIDI clock
    void setTempo(double bpm) { tempo_bpm_ = bpm; }
    double getTempo() const { return tempo_bpm_; }
    void setClockEnabled(bool enabled) { clock_enabled_ = enabled; }
    bool getClockEnabled() const { return clock_enabled_; }

private:
    void parseSystemExclusive();
    void handleNoteOn(int channel, int note, int velocity);
    void handleNoteOff(int channel, int note);
    void handleControlChange(int channel, int controller, int value);
    void handlePitchBend(int channel, int value);
    void handleAfterTouch(int channel, int data);
    
    void sendMessageThrough(const MidiMessage& message);
    
    // Parsing state
    int current_status_;
    bool expecting_data1_;
    bool expecting_data2_;
    std::vector<int> system_exclusive_data_;
    bool in_sysex_;
    
    // Channel configuration
    int channel_filter_; // -1 for all channels
    bool channel_enabled_[16];
    
    // MIDI output
    MidiOutput* midi_output_;
    bool midi_thru_enabled_;
    
    // Timing
    double tempo_bpm_;
    bool clock_enabled_;
    int clock_count_;
    double clock_timer_;
    
    // Callback state
    bool notes_[16][128];
    int controller_values_[16][128];
    int pitch_bend_[16];
    int channel_pressure_[16];
    int program_[16];
};

} // namespace vital