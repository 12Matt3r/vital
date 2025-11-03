#pragma once

#include <memory>
#include <string>
#include <functional>
#include <queue>
#include <thread>

namespace vital {
namespace ui {
namespace accessibility {

/**
 * @brief Text-to-speech engine for accessibility
 * 
 * Provides text-to-speech functionality with:
 * - Voice selection
 * - Speech rate control
 * - Queue management
 * - Event callbacks
 */
class TextToSpeechEngine {
public:
    enum class VoiceGender {
        Male,
        Female,
        Neutral
    };

    struct Voice {
        std::string id;
        std::string name;
        std::string language;
        VoiceGender gender = VoiceGender::Neutral;
        float rate = 1.0f;
        float pitch = 1.0f;
        float volume = 1.0f;
    };

    struct SpeechRequest {
        std::string text;
        std::string voiceId;
        float rate = 1.0f;
        float pitch = 1.0f;
        float volume = 1.0f;
        bool interrupt = false;
    };

    using SpeechCallback = std::function<void(const std::string& requestId, bool completed)>;
    using VoiceChangedCallback = std::function<void(const Voice& voice)>;

    TextToSpeechEngine();
    ~TextToSpeechEngine();

    // Speech control
    void speak(const std::string& text);
    void speak(const std::string& text, const std::string& voiceId);
    void speak(const SpeechRequest& request);
    void stop();
    void pause();
    void resume();
    
    // Queue management
    void clearQueue();
    int getQueueSize() const;
    void setQueueEnabled(bool enabled);
    
    // Voice management
    std::vector<Voice> getAvailableVoices() const;
    void setCurrentVoice(const std::string& voiceId);
    std::string getCurrentVoiceId() const;
    void setDefaultRate(float rate);
    void setDefaultPitch(float pitch);
    void setDefaultVolume(float volume);
    
    // Settings
    void setEnabled(bool enabled);
    bool isEnabled() const { return enabled_; }
    void setInterruptionEnabled(bool enabled);
    
    // Callbacks
    void setSpeechStartedCallback(SpeechCallback callback);
    void setSpeechCompletedCallback(SpeechCallback callback);
    void setVoiceChangedCallback(VoiceChangedCallback callback);
    
    // Status
    bool isSpeaking() const;
    bool isPaused() const;

private:
    std::vector<Voice> voices_;
    std::string currentVoiceId_;
    
    std::queue<SpeechRequest> speechQueue_;
    std::thread speechThread_;
    std::atomic<bool> isRunning_{false};
    std::atomic<bool> isPaused_{false};
    std::atomic<bool> stopRequested_{false};
    
    bool enabled_ = true;
    bool interruptionEnabled_ = true;
    
    // Default settings
    float defaultRate_ = 1.0f;
    float defaultPitch_ = 1.0f;
    float defaultVolume_ = 1.0f;
    
    // Callbacks
    SpeechCallback speechStartedCallback_;
    SpeechCallback speechCompletedCallback_;
    VoiceChangedCallback voiceChangedCallback_;
    
    void processQueue();
    void speakRequest(const SpeechRequest& request);
    void loadVoices();
    
    // Platform-specific
    void platformSpeak(const SpeechRequest& request);
    void platformStop();
    void platformPause();
    void platformResume();
    void platformLoadVoices();
};

} // namespace accessibility
} // namespace ui
} // namespace vital