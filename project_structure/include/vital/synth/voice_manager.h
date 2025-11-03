#pragma once

#include <vector>
#include <array>
#include "voice.h"
#include "note_processor.h"

namespace vital {

class VoiceManager {
public:
    VoiceManager(int max_voices = 32);
    ~VoiceManager();

    void setMaxVoices(int max_voices);
    int getMaxVoices() const { return max_voices_; }
    int getActiveVoices() const { return active_voice_count_; }
    int getPolyphony() const { return current_polyphony_; }

    // Voice management
    Voice* allocateVoice(int note, int velocity);
    void releaseVoice(int note);
    void releaseAllVoices();
    Voice* getVoice(int note) const;
    
    // Processing
    void process(float* left_output, float* right_output, int frames);
    void processMidi(int status, int data1, int data2);

    // Voice stealing algorithms
    void setVoiceStealingMode(VoiceStealingMode mode) { stealing_mode_ = mode; }
    VoiceStealingMode getVoiceStealingMode() const { return stealing_mode_; }

    // Utility
    bool hasAvailableVoice() const;
    int getOldestVoice() const;
    int getQuietestVoice() const;
    int getFirstVoice() const;

private:
    void initializeVoices();
    void updateVoicePriorities();
    void stealVoice(int note);
    void processKeyPressure(int note, int pressure);

    std::vector<std::unique_ptr<Voice>> voices_;
    std::vector<bool> voice_active_;
    std::array<int, 128> note_to_voice_map_;
    
    int max_voices_;
    int active_voice_count_;
    int current_polyphony_;
    
    VoiceStealingMode stealing_mode_;
    
    NoteProcessor note_processor_;
    
    // Voice performance tracking
    std::vector<float> voice_priorities_;
    double total_cpu_usage_;
};

} // namespace vital