#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>
#include <chrono>
#include <JuceHeader.h>

namespace vital {
namespace ui {
namespace core {

/**
 * @brief Easing functions for animations
 */
enum class EasingFunction {
    Linear,
    EaseIn,
    EaseOut,
    EaseInOut,
    Spring,
    Bounce,
    Elastic,
    Back,
    Quint,
    Quart,
    Cubic,
    Expo,
    Sine,
    Circ,
    Custom
};

/**
 * @brief Animation type
 */
enum class AnimationType {
    Position,
    Size,
    Opacity,
    Color,
    Rotation,
    Scale,
    Custom
};

/**
 * @brief Animation state
 */
struct Animation {
    void* target = nullptr;  // Component or custom target
    AnimationType type = AnimationType::Position;
    EasingFunction easing = EasingFunction::EaseOut;
    
    // Start and end values
    juce::var start_value;
    juce::var end_value;
    juce::var current_value;
    
    // Animation parameters
    float duration_ms = 300.0f;
    float elapsed_ms = 0.0f;
    float progress = 0.0f;
    bool loop = false;
    bool reverse = false;
    bool playing = false;
    
    // Callbacks
    std::function<void()> on_start_callback;
    std::function<void()> on_complete_callback;
    std::function<void(float)> on_progress_callback;
    std::function<void(const juce::var&)> on_value_changed_callback;
    
    // Spring physics (for spring easing)
    float velocity = 0.0f;
    float mass = 1.0f;
    float stiffness = 40.0f;
    float damping = 10.0f;
    
    Animation() = default;
    
    Animation(void* target_ptr, AnimationType type, 
              const juce::var& start, const juce::var& end,
              float duration_ms = 300.0f, EasingFunction easing = EasingFunction::EaseOut)
        : target(target_ptr), type(type), start_value(start), end_value(end), 
          duration_ms(duration_ms), easing(easing) {}
};

/**
 * @brief GPU animation for complex visual effects
 */
struct GpuAnimation {
    juce::String program_id;
    std::unordered_map<juce::String, juce::var> uniforms;
    float duration_ms = 1000.0f;
    float elapsed_ms = 0.0f;
    float progress = 0.0f;
    bool playing = false;
    
    GpuAnimation() = default;
    GpuAnimation(const juce::String& program_id, float duration)
        : program_id(program_id), duration_ms(duration) {}
};

/**
 * @brief Animation engine for GPU and CPU-based animations
 */
class VITAL_MODERN_UI_API AnimationEngine {
public:
    /**
     * @brief Constructor
     */
    AnimationEngine();

    /**
     * @brief Destructor
     */
    ~AnimationEngine();

    //==============================================================================
    // Animation Management
    /**
     * @brief Create and start an animation
     * @param target Target object (usually Component*)
     * @param type Animation type
     * @param start_value Start value
     * @param end_value End value
     * @param duration_ms Duration in milliseconds
     * @param easing Easing function
     * @param callback Completion callback
     * @return Animation ID
     */
    int animate(void* target, AnimationType type,
               const juce::var& start_value, const juce::var& end_value,
               float duration_ms = 300.0f, 
               EasingFunction easing = EasingFunction::EaseOut,
               const std::function<void()>& callback = nullptr);

    /**
     * @brief Create a spring physics animation
     * @param target Target object
     * @param type Animation type
     * @param start_value Start value
     * @param end_value End value
     * @param stiffness Spring stiffness
     * @param damping Damping coefficient
     * @param mass Spring mass
     * @param callback Completion callback
     * @return Animation ID
     */
    int animateSpring(void* target, AnimationType type,
                     const juce::var& start_value, const juce::var& end_value,
                     float stiffness = 40.0f, float damping = 10.0f, float mass = 1.0f,
                     const std::function<void()>& callback = nullptr);

    /**
     * @brief Stop animation by ID
     * @param animation_id Animation ID
     */
    void stopAnimation(int animation_id);

    /**
     * @brief Stop all animations for target
     * @param target Target object
     */
    void stopAnimations(void* target);

    /**
     * @brief Stop all animations
     */
    void stopAllAnimations();

    /**
     * @brief Pause animation by ID
     * @param animation_id Animation ID
     */
    void pauseAnimation(int animation_id);

    /**
     * @brief Resume animation by ID
     * @param animation_id Animation ID
     */
    void resumeAnimation(int animation_id);

    /**
     * @brief Check if animation is playing
     * @param animation_id Animation ID
     */
    bool isAnimationPlaying(int animation_id) const;

    /**
     * @brief Get animation by ID
     * @param animation_id Animation ID
     * @return Animation pointer or nullptr
     */
    Animation* getAnimation(int animation_id) const;

    //==============================================================================
    // Value Interpolation
    /**
     * @brief Interpolate between two values
     * @param start Start value
     * @param end End value
     * @param progress Progress (0.0 to 1.0)
     * @param easing Easing function
     * @return Interpolated value
     */
    static juce::var interpolate(const juce::var& start, const juce::var& end,
                               float progress, EasingFunction easing);

    /**
     * @brief Get eased progress value
     * @param progress Raw progress (0.0 to 1.0)
     * @param easing Easing function
     * @return Eased progress
     */
    static float easeProgress(float progress, EasingFunction easing);

    //==============================================================================
    // GPU Animations
    /**
     * @brief Create and start a GPU animation
     * @param program_id GPU shader program ID
     * @param uniforms Shader uniforms to animate
     * @param duration_ms Duration in milliseconds
     * @param easing Easing function
     * @return GpuAnimation ID
     */
    int animateGpu(const juce::String& program_id,
                  const std::unordered_map<juce::String, juce::var>& uniforms,
                  float duration_ms = 1000.0f,
                  EasingFunction easing = EasingFunction::EaseInOut);

    /**
     * @brief Stop GPU animation
     * @param animation_id GPU animation ID
     */
    void stopGpuAnimation(int animation_id);

    /**
     * @brief Get GPU animation
     * @param animation_id GPU animation ID
     * @return GpuAnimation pointer or nullptr
     */
    GpuAnimation* getGpuAnimation(int animation_id) const;

    //==============================================================================
    // Configuration
    /**
     * @brief Set target FPS
     * @param fps Frames per second
     */
    void setTargetFPS(float fps);

    /**
     * @brief Get target FPS
     */
    float getTargetFPS() const { return target_fps_; }

    /**
     * @brief Set quality level
     * @param quality Quality level (0 = low, 1 = medium, 2 = high)
     */
    void setQualityLevel(int quality);

    /**
     * @brief Get quality level
     */
    int getQualityLevel() const { return quality_level_; }

    /**
     * @brief Enable/disable animations
     * @param enabled Whether animations are enabled
     */
    void setEnabled(bool enabled);

    /**
     * @brief Check if animations are enabled
     */
    bool isEnabled() const { return enabled_; }

    /**
     * @brief Set reduced motion preference
     * @param reduced Whether reduced motion is preferred
     */
    void setReducedMotion(bool reduced);

    /**
     * @brief Check if reduced motion is enabled
     */
    bool isReducedMotion() const { return reduced_motion_; }

    //==============================================================================
    // Performance Monitoring
    /**
     * @brief Get active animation count
     */
    int getActiveAnimationCount() const;

    /**
     * @brief Get active GPU animation count
     */
    int getActiveGpuAnimationCount() const;

    /**
     * @brief Get average frame time
     */
    float getAverageFrameTime() const { return average_frame_time_.load(); }

    /**
     * @brief Get peak frame time
     */
    float getPeakFrameTime() const { return peak_frame_time_.load(); }

    /**
     * @brief Reset performance statistics
     */
    void resetPerformanceStats();

    //==============================================================================
    // Lifecycle
    /**
     * @brief Initialize animation engine
     */
    void initialize();

    /**
     * @brief Shutdown animation engine
     */
    void shutdown();

    /**
     * @brief Update all animations (called each frame)
     */
    void update();

    /**
     * @brief Check if initialized
     */
    bool isInitialized() const { return initialized_.load(); }

    //==============================================================================
    // Custom Easing Functions
    /**
     * @brief Register custom easing function
     * @param name Function name
     * @param func Easing function
     */
    static void registerCustomEasing(const juce::String& name, 
                                   std::function<float(float)> func);

    /**
     * @brief Get registered easing function
     * @param name Function name
     * @return Easing function or nullptr
     */
    static std::function<float(float)> getCustomEasing(const juce::String& name);

private:
    //==============================================================================
    // Private member variables
    mutable std::mutex animations_mutex_;
    mutable std::mutex gpu_animations_mutex_;

    std::vector<std::unique_ptr<Animation>> animations_;
    std::vector<std::unique_ptr<GpuAnimation>> gpu_animations_;

    // ID counters for animations
    int next_animation_id_ = 1;
    int next_gpu_animation_id_ = 1;

    // Configuration
    float target_fps_ = 60.0f;
    int quality_level_ = 1;  // 0=low, 1=medium, 2=high
    bool enabled_ = true;
    bool reduced_motion_ = false;
    bool initialized_ = false;

    // Performance tracking
    mutable std::atomic<float> average_frame_time_{16.67f};
    mutable std::atomic<float> peak_frame_time_{0.0f};
    std::chrono::steady_clock::time_point last_frame_time_;
    int frame_count_ = 0;

    // Custom easing functions
    static std::unordered_map<juce::String, std::function<float(float)>> custom_easings_;

    //==============================================================================
    // Private methods
    void updateAnimations();
    void updateGpuAnimations();
    void processAnimation(Animation& animation, float delta_time_ms);
    void processGpuAnimation(GpuAnimation& animation, float delta_time_ms);

    // Easing implementations
    static float easeLinear(float t);
    static float easeInQuad(float t);
    static float easeOutQuad(float t);
    static float easeInOutQuad(float t);
    static float easeSpring(float t);
    static float easeBounce(float t);
    static float easeElastic(float t);
    static float easeBack(float t);

    // Value type handling
    static juce::var interpolateNumeric(const juce::var& start, const juce::var& end, float t);
    static juce::var interpolateColor(const juce::var& start, const juce::var& end, float t);
    static juce::var interpolateRectangle(const juce::var& start, const juce::var& end, float t);

    // Cleanup
    void cleanupCompletedAnimations();
    void cleanupCompletedGpuAnimations();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnimationEngine)
};

} // namespace core
} // namespace ui
} // namespace vital
