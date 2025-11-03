#include "vital_voice_control.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <thread>

namespace vital {
namespace voice_control {

/**
 * @brief MFCC feature extraction for speech recognition
 */
class MFCCExtractor {
public:
    MFCCExtractor(int sample_rate = 16000, int num_mfcc = 13, int num_filters = 26)
        : sample_rate_(sample_rate)
        , num_mfcc_(num_mfcc)
        , num_filters_(num_filters)
        , frame_size_(512)
        , hop_length_(256) {
        
        initializeMelFilters();
    }

    std::vector<float> extractMFCC(const std::vector<float>& audio_data) {
        // Pre-emphasis filter
        std::vector<float> preemphasized = preemphasize(audio_data);
        
        // Windowing and framing
        auto frames = frameSignal(preemphasized);
        
        // Apply window function and compute FFT
        std::vector<std::vector<std::complex<float>>> fft_frames;
        for (const auto& frame : frames) {
            auto windowed = applyHannWindow(frame);
            fft_frames.push_back(computeFFT(windowed));
        }
        
        // Apply mel filters and compute MFCC
        std::vector<float> mfcc_features;
        for (const auto& fft_frame : fft_frames) {
            auto mel_spectrum = applyMelFilters(fft_frame);
            auto mfcc_frame = computeDCT(mel_spectrum);
            mfcc_features.insert(mfcc_features.end(), mfcc_frame.begin(), mfcc_frame.end());
        }
        
        return mfcc_features;
    }

private:
    std::vector<float> preemphasize(const std::vector<float>& audio) {
        std::vector<float> result(audio.size());
        result[0] = audio[0];
        
        for (size_t i = 1; i < audio.size(); i++) {
            result[i] = audio[i] - 0.97f * audio[i-1];
        }
        
        return result;
    }

    std::vector<std::vector<float>> frameSignal(const std::vector<float>& signal) {
        std::vector<std::vector<float>> frames;
        
        for (int i = 0; i + frame_size_ <= signal.size(); i += hop_length_) {
            std::vector<float> frame(signal.begin() + i, signal.begin() + i + frame_size_);
            frames.push_back(frame);
        }
        
        return frames;
    }

    std::vector<float> applyHannWindow(const std::vector<float>& frame) {
        std::vector<float> windowed(frame.size());
        
        for (size_t i = 0; i < frame.size(); i++) {
            float window = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (frame.size() - 1)));
            windowed[i] = frame[i] * window;
        }
        
        return windowed;
    }

    std::vector<std::complex<float>> computeFFT(const std::vector<float>& signal) {
        // Simple FFT implementation (for production, use FFTW or similar)
        std::vector<std::complex<float>> result(signal.size() / 2 + 1);
        
        for (size_t k = 0; k < result.size(); k++) {
            std::complex<float> sum(0.0f, 0.0f);
            
            for (size_t n = 0; n < signal.size(); n++) {
                float angle = -2.0f * M_PI * k * n / signal.size();
                std::complex<float> twiddle(std::cos(angle), std::sin(angle));
                sum += signal[n] * twiddle;
            }
            
            result[k] = sum;
        }
        
        return result;
    }

    void initializeMelFilters() {
        mel_filters_.resize(num_filters_);
        
        float mel_min = 0.0f;
        float mel_max = 2595.0f * std::log10(1.0f + sample_rate_ / 2.0f / 700.0f);
        
        std::vector<float> mel_points(num_filters_ + 2);
        for (int i = 0; i < num_filters_ + 2; i++) {
            mel_points[i] = mel_min + (mel_max - mel_min) * i / (num_filters_ + 1);
        }
        
        // Convert mel points to frequency
        for (int i = 0; i < num_filters_ + 2; i++) {
            mel_points[i] = 700.0f * (std::pow(10.0f, mel_points[i] / 2595.0f) - 1.0f);
        }
        
        // Convert frequency to FFT bin
        std::vector<int> fft_bins(num_filters_ + 2);
        for (int i = 0; i < num_filters_ + 2; i++) {
            fft_bins[i] = static_cast<int>((frame_size_ + 1) * mel_points[i] / sample_rate_);
        }
        
        // Create triangular filters
        for (int m = 1; m <= num_filters_; m++) {
            mel_filters_[m-1].resize(frame_size_ / 2 + 1, 0.0f);
            
            int left = fft_bins[m-1];
            int center = fft_bins[m];
            int right = fft_bins[m+1];
            
            // Rising slope
            for (int k = left; k < center; k++) {
                mel_filters_[m-1][k] = static_cast<float>(k - left) / (center - left);
            }
            
            // Falling slope
            for (int k = center; k < right; k++) {
                mel_filters_[m-1][k] = static_cast<float>(right - k) / (right - center);
            }
        }
    }

    std::vector<float> applyMelFilters(const std::vector<std::complex<float>>& fft_frame) {
        std::vector<float> mel_spectrum(num_filters_);
        
        for (int m = 0; m < num_filters_; m++) {
            float sum = 0.0f;
            
            for (size_t k = 0; k < fft_frame.size(); k++) {
                float magnitude = std::abs(fft_frame[k]);
                sum += mel_filters_[m][k] * magnitude * magnitude;
            }
            
            mel_spectrum[m] = std::log(std::max(sum, 1e-10f));
        }
        
        return mel_spectrum;
    }

    std::vector<float> computeDCT(const std::vector<float>& mel_spectrum) {
        std::vector<float> mfcc(num_mfcc_);
        
        for (int n = 0; n < num_mfcc_; n++) {
            float sum = 0.0f;
            
            for (int m = 0; m < num_filters_; m++) {
                sum += mel_spectrum[m] * std::cos(M_PI * n * (m + 0.5f) / num_filters_);
            }
            
            mfcc[n] = sum;
        }
        
        return mfcc;
    }

    int sample_rate_;
    int num_mfcc_;
    int num_filters_;
    int frame_size_;
    int hop_length_;
    std::vector<std::vector<float>> mel_filters_;
};

/**
 * @brief Voice Activity Detection for speech segment detection
 */
class VoiceActivityDetector {
public:
    VoiceActivityDetector(int sample_rate = 16000)
        : sample_rate_(sample_rate)
        , frame_size_(512)
        , hop_length_(256)
        , energy_threshold_(0.01f)
        , min_speech_duration_ms_(200)
        , max_speech_duration_ms_(10000)
        , is_speaking_(false)
        , speech_start_time_(0)
        , silence_duration_(0) {
    }

    bool isVoiceDetected(const std::vector<float>& audio_frame) {
        float frame_energy = computeFrameEnergy(audio_frame);
        
        if (frame_energy > energy_threshold_) {
            if (!is_speaking_) {
                is_speaking_ = true;
                speech_start_time_ = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count();
            }
            silence_duration_ = 0;
        } else {
            if (is_speaking_) {
                silence_duration_ += static_cast<int>(frame_size_ * 1000.0f / sample_rate_);
                
                // End of speech if silence duration exceeds threshold
                if (silence_duration_ > 300) { // 300ms silence threshold
                    auto speech_duration = getCurrentSpeechDuration();
                    if (speech_duration >= min_speech_duration_ms_ && 
                        speech_duration <= max_speech_duration_ms_) {
                        is_speaking_ = false;
                        return true; // Speech segment completed
                    } else {
                        is_speaking_ = false; // Too short or too long, ignore
                    }
                }
            }
        }
        
        return false;
    }

    float getCurrentEnergyLevel() const {
        return current_energy_level_;
    }

    void setEnergyThreshold(float threshold) {
        energy_threshold_ = threshold;
    }

private:
    float computeFrameEnergy(const std::vector<float>& frame) {
        float energy = 0.0f;
        for (float sample : frame) {
            energy += sample * sample;
        }
        current_energy_level_ = std::sqrt(energy / frame.size());
        return current_energy_;
    }

    int getCurrentSpeechDuration() const {
        auto current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        return static_cast<int>(current_time - speech_start_time_);
    }

    int sample_rate_;
    int frame_size_;
    int hop_length_;
    float energy_threshold_;
    int min_speech_duration_ms_;
    int max_speech_duration_ms_;
    
    mutable std::atomic<bool> is_speaking_;
    long long speech_start_time_;
    int silence_duration_;
    mutable float current_energy_level_;
    float current_energy_;
};

/**
 * @brief Pattern matching for command recognition
 */
class PatternMatcher {
public:
    struct Pattern {
        std::string text;
        VoiceCommandRecognizer::CommandType type;
        std::vector<std::string> parameters;
        float confidence;
        std::string language;
    };

    PatternMatcher() {
        initializeDefaultPatterns();
    }

    std::vector<Pattern> matchPatterns(const std::string& input) {
        std::vector<Pattern> matches;
        std::string lower_input = input;
        std::transform(lower_input.begin(), lower_input.end(), lower_input.begin(), ::tolower);
        
        for (const auto& pattern : patterns_) {
            float similarity = computeSimilarity(lower_input, pattern.text);
            if (similarity > 0.6f) {
                Pattern match = pattern;
                match.confidence = similarity;
                matches.push_back(match);
            }
        }
        
        // Sort by confidence
        std::sort(matches.begin(), matches.end(), [](const Pattern& a, const Pattern& b) {
            return a.confidence > b.confidence;
        });
        
        return matches;
    }

private:
    void initializeDefaultPatterns() {
        // Parameter control patterns
        patterns_.push_back({"increase volume", VoiceCommandRecognizer::CommandType::ParameterControl, {"master_volume", "increase"}, 0.9f, "en-US"});
        patterns_.push_back({"decrease volume", VoiceCommandRecognizer::CommandType::ParameterControl, {"master_volume", "decrease"}, 0.9f, "en-US"});
        patterns_.push_back({"increase cutoff", VoiceCommandRecognizer::CommandType::ParameterControl, {"filter_cutoff", "increase"}, 0.9f, "en-US"});
        patterns_.push_back({"decrease cutoff", VoiceCommandRecognizer::CommandType::ParameterControl, {"filter_cutoff", "decrease"}, 0.9f, "en-US"});
        patterns_.push_back({"increase resonance", VoiceCommandRecognizer::CommandType::ParameterControl, {"filter_resonance", "increase"}, 0.9f, "en-US"});
        patterns_.push_back({"decrease resonance", VoiceCommandRecognizer::CommandType::ParameterControl, {"filter_resonance", "decrease"}, 0.9f, "en-US"});
        patterns_.push_back({"set volume to", VoiceCommandRecognizer::CommandType::ParameterControl, {"master_volume", "set"}, 0.9f, "en-US"});
        patterns_.push_back({"set cutoff to", VoiceCommandRecognizer::CommandType::ParameterControl, {"filter_cutoff", "set"}, 0.9f, "en-US"});
        patterns_.push_back({"set resonance to", VoiceCommandRecognizer::CommandType::ParameterControl, {"filter_resonance", "set"}, 0.9f, "en-US"});
        
        // Preset navigation patterns
        patterns_.push_back({"next preset", VoiceCommandRecognizer::CommandType::PresetNavigation, {"navigate", "next"}, 0.9f, "en-US"});
        patterns_.push_back({"previous preset", VoiceCommandRecognizer::CommandType::PresetNavigation, {"navigate", "previous"}, 0.9f, "en-US"});
        patterns_.push_back({"random preset", VoiceCommandRecognizer::CommandType::PresetNavigation, {"navigate", "random"}, 0.9f, "en-US"});
        patterns_.push_back({"load", VoiceCommandRecognizer::CommandType::PresetNavigation, {"load"}, 0.7f, "en-US"});
        
        // Tutorial patterns
        patterns_.push_back({"start tutorial", VoiceCommandRecognizer::CommandType::Tutorial, {"start"}, 0.9f, "en-US"});
        patterns_.push_back({"pause tutorial", VoiceCommandRecognizer::CommandType::Tutorial, {"pause"}, 0.9f, "en-US"});
        patterns_.push_back({"resume tutorial", VoiceCommandRecognizer::CommandType::Tutorial, {"resume"}, 0.9f, "en-US"});
        patterns_.push_back({"stop tutorial", VoiceCommandRecognizer::CommandType::Tutorial, {"stop"}, 0.9f, "en-US"});
        patterns_.push_back({"help", VoiceCommandRecognizer::CommandType::Help, {}, 0.9f, "en-US"});
        
        // Spanish patterns
        patterns_.push_back({"aumentar volumen", VoiceCommandRecognizer::CommandType::ParameterControl, {"master_volume", "increase"}, 0.9f, "es-ES"});
        patterns_.push_back({"disminuir volumen", VoiceCommandRecognizer::CommandType::ParameterControl, {"master_volume", "decrease"}, 0.9f, "es-ES"});
        patterns_.push_back({"siguiente preset", VoiceCommandRecognizer::CommandType::PresetNavigation, {"navigate", "next"}, 0.9f, "es-ES"});
        
        // French patterns
        patterns_.push_back({"augmenter le volume", VoiceCommandRecognizer::CommandType::ParameterControl, {"master_volume", "increase"}, 0.9f, "fr-FR"});
        patterns_.push_back({"diminuer le volume", VoiceCommandRecognizer::CommandType::ParameterControl, {"master_volume", "decrease"}, 0.9f, "fr-FR"});
        
        // German patterns
        patterns_.push_back({"lautstärke erhöhen", VoiceCommandRecognizer::CommandType::ParameterControl, {"master_volume", "increase"}, 0.9f, "de-DE"});
        patterns_.push_back({"lautstärke verringern", VoiceCommandRecognizer::CommandType::ParameterControl, {"master_volume", "decrease"}, 0.9f, "de-DE"});
        
        // Japanese patterns
        patterns_.push_back({"ボリュームを上げる", VoiceCommandRecognizer::CommandType::ParameterControl, {"master_volume", "increase"}, 0.9f, "ja-JP"});
        patterns_.push_back({"ボリュームを下げる", VoiceCommandRecognizer::CommandType::ParameterControl, {"master_volume", "decrease"}, 0.9f, "ja-JP"});
    }

    float computeSimilarity(const std::string& str1, const std::string& str2) {
        // Simple Levenshtein distance-based similarity
        int len1 = str1.length();
        int len2 = str2.length();
        
        if (len1 == 0) return len2 == 0 ? 1.0f : 0.0f;
        if (len2 == 0) return 0.0f;
        
        std::vector<std::vector<int>> matrix(len1 + 1, std::vector<int>(len2 + 1));
        
        for (int i = 0; i <= len1; i++) matrix[i][0] = i;
        for (int j = 0; j <= len2; j++) matrix[0][j] = j;
        
        for (int i = 1; i <= len1; i++) {
            for (int j = 1; j <= len2; j++) {
                int cost = (str1[i-1] == str2[j-1]) ? 0 : 1;
                matrix[i][j] = std::min({
                    matrix[i-1][j] + 1,     // deletion
                    matrix[i][j-1] + 1,     // insertion
                    matrix[i-1][j-1] + cost // substitution
                });
            }
        }
        
        int distance = matrix[len1][len2];
        int max_len = std::max(len1, len2);
        return static_cast<float>(max_len - distance) / max_len;
    }

    std::vector<Pattern> patterns_;
};

/**
 * @brief Implementation class for VoiceCommandRecognizer
 */
class VoiceCommandRecognizer::Impl {
public:
    Impl() 
        : current_state_(RecognitionState::Idle)
        , is_listening_(false)
        , audio_level_(0.0f)
        , command_count_(0)
        , settings_()
        , mfcc_extractor_(std::make_unique<MFCCExtractor>())
        , vad_detector_(std::make_unique<VoiceActivityDetector>())
        , pattern_matcher_(std::make_unique<PatternMatcher>())
        , audio_buffer_(1024)
        , command_queue_() {
    }

    bool initialize(const RecognitionSettings& settings) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        settings_ = settings;
        current_state_ = RecognitionState::Ready;
        
        // Load default language models
        loadDefaultLanguageModels();
        
        return true;
    }

    void startListening() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (current_state_ == RecognitionState::Ready) {
            current_state_ = RecognitionState::Listening;
            is_listening_ = true;
            startAudioProcessing();
        }
    }

    void stopListening() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        is_listening_ = false;
        current_state_ = RecognitionState::Ready;
    }

    void pauseListening() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (is_listening_) {
            current_state_ = RecognitionState::Idle;
        }
    }

    void resumeListening() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!is_listening_) {
            current_state_ = RecognitionState::Listening;
        }
    }

    void registerCommandCallback(CommandCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        command_callbacks_.push_back(callback);
    }

    void registerStateChangeCallback(StateChangeCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        state_callbacks_.push_back(callback);
    }

    void processAudioBuffer(const float* audio_data, int num_samples) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!is_listening_ || current_state_ != RecognitionState::Listening) {
            return;
        }
        
        // Add audio data to buffer
        for (int i = 0; i < num_samples && audio_buffer_.size() < 4096; i++) {
            audio_buffer_.push_back(audio_data[i]);
        }
        
        // Process audio in chunks
        while (audio_buffer_.size() >= settings_.buffer_size) {
            std::vector<float> frame(audio_buffer_.begin(), audio_buffer_.begin() + settings_.buffer_size);
            audio_buffer_.erase(audio_buffer_.begin(), audio_buffer_.begin() + settings_.buffer_size);
            
            processAudioFrame(frame);
        }
    }

    void setRecognitionSettings(const RecognitionSettings& settings) {
        std::lock_guard<std::mutex> lock(mutex_);
        settings_ = settings;
    }

    RecognitionSettings getRecognitionSettings() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return settings_;
    }

    bool loadLanguageModel(const std::string& language_code) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Load language-specific acoustic and language models
        // For now, just add to supported languages
        supported_languages_.push_back(language_code);
        return true;
    }

    void setActiveLanguage(const std::string& language_code) {
        std::lock_guard<std::mutex> lock(mutex_);
        active_language_ = language_code;
    }

    std::vector<std::string> getSupportedLanguages() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return supported_languages_;
    }

    RecognitionState getCurrentState() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return current_state_;
    }

    float getAudioLevel() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return audio_level_;
    }

    int getCommandCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return command_count_;
    }

    std::vector<VoiceCommand> getRecentCommands() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::vector<VoiceCommand> recent_commands;
        int count = std::min(10, static_cast<int>(recent_commands_.size()));
        
        for (int i = recent_commands_.size() - count; i < recent_commands_.size(); i++) {
            recent_commands.push_back(recent_commands_[i]);
        }
        
        return recent_commands;
    }

    bool isVoiceDetected() const {
        return vad_detector_->isVoiceDetected({});
    }

    void setVADSettings(float threshold, int min_duration_ms) {
        vad_detector_->setEnergyThreshold(threshold);
        // Set minimum duration if needed
    }

private:
    void startAudioProcessing() {
        // Start audio processing thread
        std::thread processing_thread(&Impl::audioProcessingLoop, this);
        processing_thread.detach();
    }

    void audioProcessingLoop() {
        while (is_listening_) {
            if (current_state_ == RecognitionState::Listening) {
                // Process audio buffer
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }

    void processAudioFrame(const std::vector<float>& frame) {
        // Update audio level
        audio_level_ = computeAudioLevel(frame);
        
        // Voice Activity Detection
        if (settings_.enable_vad && vad_detector_->isVoiceDetected(frame)) {
            processSpeechSegment(frame);
        }
    }

    float computeAudioLevel(const std::vector<float>& frame) {
        float rms = 0.0f;
        for (float sample : frame) {
            rms += sample * sample;
        }
        rms = std::sqrt(rms / frame.size());
        return rms;
    }

    void processSpeechSegment(const std::vector<float>& frame) {
        current_state_ = RecognitionState::Processing;
        
        // Extract MFCC features
        auto mfcc_features = mfcc_extractor_->extractMFCC(frame);
        
        // Perform pattern matching
        std::string recognized_text = performSpeechRecognition(mfcc_features);
        
        if (!recognized_text.empty()) {
            processRecognizedText(recognized_text);
        }
        
        current_state_ = RecognitionState::Listening;
    }

    std::string performSpeechRecognition(const std::vector<float>& mfcc_features) {
        // Simplified speech recognition using pattern matching
        // In production, this would use more sophisticated ASR models
        
        // For now, return empty string to indicate no recognition
        // The pattern matching is done at a higher level
        return "";
    }

    void processRecognizedText(const std::string& text) {
        // Use pattern matcher to identify command
        auto matches = pattern_matcher_->matchPatterns(text);
        
        if (!matches.empty()) {
            auto& best_match = matches[0];
            
            if (best_match.confidence >= settings_.confidence_threshold) {
                VoiceCommand command;
                command.text = best_match.text;
                command.type = best_match.type;
                command.parameters = best_match.parameters;
                command.confidence = best_match.confidence;
                command.is_valid = true;
                command.language = best_match.language;
                command.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch());
                
                // Add to recent commands
                recent_commands_.push_back(command);
                if (recent_commands_.size() > 100) {
                    recent_commands_.erase(recent_commands_.begin());
                }
                
                command_count_++;
                
                // Notify callbacks
                for (const auto& callback : command_callbacks_) {
                    if (callback) {
                        callback(command);
                    }
                }
            }
        }
    }

    void loadDefaultLanguageModels() {
        // Load default supported languages
        supported_languages_ = MultiLanguageSupport::getDefaultLanguages();
        
        // Set default language
        if (!supported_languages_.empty()) {
            active_language_ = supported_languages_[0];
        }
    }

    RecognitionSettings settings_;
    mutable std::atomic<RecognitionState> current_state_;
    std::atomic<bool> is_listening_;
    
    mutable std::mutex mutex_;
    std::vector<CommandCallback> command_callbacks_;
    std::vector<StateChangeCallback> state_callbacks_;
    
    std::string active_language_;
    std::vector<std::string> supported_languages_;
    
    mutable float audio_level_;
    int command_count_;
    
    std::vector<float> audio_buffer_;
    std::vector<VoiceCommand> recent_commands_;
    
    std::unique_ptr<MFCCExtractor> mfcc_extractor_;
    std::unique_ptr<VoiceActivityDetector> vad_detector_;
    std::unique_ptr<PatternMatcher> pattern_matcher_;
};

// VoiceCommandRecognizer implementation
VoiceCommandRecognizer::VoiceCommandRecognizer() 
    : p_impl_(std::make_unique<Impl>()) {
}

VoiceCommandRecognizer::~VoiceCommandRecognizer() = default;

bool VoiceCommandRecognizer::initialize(const RecognitionSettings& settings) {
    return p_impl_->initialize(settings);
}

void VoiceCommandRecognizer::startListening() {
    p_impl_->startListening();
}

void VoiceCommandRecognizer::stopListening() {
    p_impl_->stopListening();
}

void VoiceCommandRecognizer::pauseListening() {
    p_impl_->pauseListening();
}

void VoiceCommandRecognizer::resumeListening() {
    p_impl_->resumeListening();
}

void VoiceCommandRecognizer::registerCommandCallback(CommandCallback callback) {
    p_impl_->registerCommandCallback(callback);
}

void VoiceCommandRecognizer::registerStateChangeCallback(StateChangeCallback callback) {
    p_impl_->registerStateChangeCallback(callback);
}

void VoiceCommandRecognizer::processAudioBuffer(const float* audio_data, int num_samples) {
    p_impl_->processAudioBuffer(audio_data, num_samples);
}

void VoiceCommandRecognizer::setRecognitionSettings(const RecognitionSettings& settings) {
    p_impl_->setRecognitionSettings(settings);
}

VoiceCommandRecognizer::RecognitionSettings VoiceCommandRecognizer::getRecognitionSettings() const {
    return p_impl_->getRecognitionSettings();
}

bool VoiceCommandRecognizer::loadLanguageModel(const std::string& language_code) {
    return p_impl_->loadLanguageModel(language_code);
}

void VoiceCommandRecognizer::setActiveLanguage(const std::string& language_code) {
    p_impl_->setActiveLanguage(language_code);
}

std::vector<std::string> VoiceCommandRecognizer::getSupportedLanguages() const {
    return p_impl_->getSupportedLanguages();
}

VoiceCommandRecognizer::RecognitionState VoiceCommandRecognizer::getCurrentState() const {
    return p_impl_->getCurrentState();
}

float VoiceCommandRecognizer::getAudioLevel() const {
    return p_impl_->getAudioLevel();
}

int VoiceCommandRecognizer::getCommandCount() const {
    return p_impl_->getCommandCount();
}

std::vector<VoiceCommandRecognizer::VoiceCommand> VoiceCommandRecognizer::getRecentCommands() const {
    return p_impl_->getRecentCommands();
}

bool VoiceCommandRecognizer::isVoiceDetected() const {
    return p_impl_->isVoiceDetected();
}

void VoiceCommandRecognizer::setVADSettings(float threshold, int min_duration_ms) {
    p_impl_->setVADSettings(threshold, min_duration_ms);
}

} // namespace voice_control
} // namespace vital