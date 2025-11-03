#pragma once

#include "oscillator.h"

namespace vital {

class LFO {
public:
    enum class SyncMode {
        Free,
        TempoSync,
        KeySync
    };

    struct Parameters {
        float frequency_hz;
        SyncMode sync_mode;
        float tempo_sync_ratio; // for tempo sync (e.g., 1.0, 0.5, 0.25, etc.)
        float depth; // modulation depth 0.0 to 1.0
        bool phase_invert;
        bool retrigger;
    };

    LFO();
    ~LFO();

    void setParameters(const Parameters& params);
    void setFrequency(float frequency_hz);
    void setSyncMode(SyncMode sync_mode);
    void setTempoSyncRatio(float ratio);
    void setDepth(float depth);
    void setPhaseInvert(bool invert);
    void setRetrigger(bool retrigger);
    
    // Processing
    void process(float* output, int frames);
    void trigger();
    void syncToBeat();
    
    // Utility
    float getFrequency() const { return params_.frequency_hz; }
    float getDepth() const { return params_.depth; }
    SyncMode getSyncMode() const { return params_.sync_mode; }
    Parameters getParameters() const { return params_; }

private:
    void updateFrequency();
    void processFreeMode(float* output, int frames);
    void processTempoSyncMode(float* output, int frames);
    void processKeySyncMode(float* output, int frames);
    
    Parameters params_;
    Oscillator oscillator_;
    
    // Sync timing
    float sample_rate_;
    double current_phase_;
    int beat_counter_;
    
    // Tempo sync
    float tempo_bpm_;
    int samples_per_beat_;
    
    // Phase management
    bool is_sync_;
    bool has_triggered_;
};

} // namespace vital