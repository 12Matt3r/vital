#pragma once

namespace vital {

class Envelope {
public:
    enum class Stage {
        Idle,
        Attack,
        Decay,
        Sustain,
        Release,
        Shutdown
    };

    struct Parameters {
        float attack_time;    // seconds (0.001 to 10.0)
        float decay_time;     // seconds (0.001 to 10.0)
        float sustain_level;  // 0.0 to 1.0
        float release_time;   // seconds (0.01 to 10.0)
        float velocity_sensitivity; // 0.0 to 1.0
    };

    Envelope();
    ~Envelope();

    void setParameters(const Parameters& params);
    void setParameter(Stage stage, float value);
    void setParameters(float attack, float decay, float sustain, float release);
    
    // Processing
    void process(float* output, int frames);
    void trigger();
    void release();
    void stop();
    
    // State management
    Stage getCurrentStage() const { return current_stage_; }
    float getCurrentValue() const { return current_value_; }
    bool isActive() const { return current_stage_ != Stage::Idle; }
    bool isPlaying() const { return is_playing_; }
    
    // Modulation
    void setVelocity(float velocity); // 0.0 to 1.0
    
private:
    void updateStage(Stage new_stage);
    void processStage(float* output, int frames);
    void processAttack(float* output, int frames);
    void processDecay(float* output, int frames);
    void processSustain(float* output, int frames);
    void processRelease(float* output, int frames);
    
    float calculateCurve(float start, float end, float progress, bool exponential = true);
    
    Parameters params_;
    Stage current_stage_;
    Stage target_stage_;
    float current_value_;
    float velocity_multiplier_;
    bool is_playing_;
    bool is_releasing_;
    
    // Timing
    float sample_rate_;
    double stage_elapsed_time_;
    
    // Slope calculations
    float attack_slope_;
    float decay_slope_;
    float release_slope_;
};

} // namespace vital