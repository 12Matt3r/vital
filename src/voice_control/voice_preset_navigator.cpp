#include "vital_voice_control.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <numeric>

namespace vital {
namespace voice_control {

/**
 * @brief Audio feature analyzer for preset similarity
 */
class AudioFeatureAnalyzer {
public:
    struct AudioFeatures {
        float spectral_centroid = 0.0f;
        float spectral_rolloff = 0.0f;
        float zero_crossing_rate = 0.0f;
        float mfcc_mean = 0.0f;
        float mfcc_variance = 0.0f;
        float energy = 0.0f;
        std::vector<float> spectral_bandwidth;
        std::vector<float> chroma_features;
    };

    static AudioFeatures extractFeatures(const std::vector<float>& audio_data, int sample_rate = 44100) {
        AudioFeatures features;
        
        if (audio_data.empty()) return features;
        
        // Calculate energy
        features.energy = std::accumulate(audio_data.begin(), audio_data.end(), 0.0f, 
            [](float sum, float sample) { return sum + sample * sample; });
        features.energy /= audio_data.size();
        
        // Simple spectral analysis (for production, use FFT)
        int frame_size = 1024;
        int hop_size = 512;
        
        std::vector<float> magnitudes;
        for (int i = 0; i + frame_size <= audio_data.size(); i += hop_size) {
            float frame_energy = 0.0f;
            for (int j = 0; j < frame_size; j++) {
                frame_energy += audio_data[i + j] * audio_data[i + j];
            }
            magnitudes.push_back(std::sqrt(frame_energy / frame_size));
        }
        
        // Calculate spectral centroid (simplified)
        if (!magnitudes.empty()) {
            float weighted_sum = 0.0f;
            float total_energy = 0.0f;
            
            for (size_t i = 0; i < magnitudes.size(); i++) {
                float freq = static_cast<float>(i) * sample_rate / (2 * magnitudes.size());
                weighted_sum += freq * magnitudes[i];
                total_energy += magnitudes[i];
            }
            
            features.spectral_centroid = (total_energy > 0) ? weighted_sum / total_energy : 0.0f;
        }
        
        // Calculate zero crossing rate
        int zero_crossings = 0;
        for (size_t i = 1; i < audio_data.size(); i++) {
            if ((audio_data[i] >= 0) != (audio_data[i-1] >= 0)) {
                zero_crossings++;
            }
        }
        features.zero_crossing_rate = static_cast<float>(zero_crossings) / audio_data.size();
        
        return features;
    }

    static float computeSimilarity(const AudioFeatures& features1, const AudioFeatures& features2) {
        float similarity = 0.0f;
        
        // Energy similarity
        float energy_diff = std::abs(features1.energy - features2.energy);
        similarity += (1.0f - energy_diff) * 0.2f;
        
        // Spectral centroid similarity
        float centroid_diff = std::abs(features1.spectral_centroid - features2.spectral_centroid);
        float centroid_similarity = 1.0f / (1.0f + centroid_diff / 1000.0f);
        similarity += centroid_similarity * 0.3f;
        
        // Zero crossing rate similarity
        float zcr_diff = std::abs(features1.zero_crossing_rate - features2.zero_crossing_rate);
        similarity += (1.0f - zcr_diff) * 0.2f;
        
        // MFCC similarity (simplified)
        float mfcc_diff = std::abs(features1.mfcc_mean - features2.mfcc_mean);
        similarity += (1.0f - mfcc_diff) * 0.3f;
        
        return std::clamp(similarity, 0.0f, 1.0f);
    }
};

/**
 * @brief Smart recommendation engine
 */
class RecommendationEngine {
public:
    struct RecommendationContext {
        VoicePresetNavigator::NavigationDirection direction;
        std::string search_query;
        std::vector<std::string> tags;
        PresetCategory preferred_category;
        bool use_favorites;
        bool use_history;
        int max_recommendations;
    };

    struct PresetScore {
        const VoicePresetNavigator::PresetInfo* preset;
        float score;
        float similarity;
        float popularity;
        float recency;
    };

    std::vector<PresetScore> generateRecommendations(
        const std::vector<VoicePresetNavigator::PresetInfo>& presets,
        const RecommendationContext& context) {
        
        std::vector<PresetScore> scored_presets;
        
        for (const auto& preset : presets) {
            float score = computePresetScore(preset, context);
            scored_presets.push_back({&preset, score, 0.0f, 0.0f, 0.0f});
        }
        
        // Sort by score
        std::sort(scored_presets.begin(), scored_presets.end(), 
            [](const PresetScore& a, const PresetScore& b) {
                return a.score > b.score;
            });
        
        // Limit results
        if (scored_presets.size() > context.max_recommendations) {
            scored_presets.resize(context.max_recommendations);
        }
        
        return scored_presets;
    }

private:
    float computePresetScore(const VoicePresetNavigator::PresetInfo& preset, 
                           const RecommendationContext& context) {
        float score = 0.0f;
        
        // Category match
        if (preset.category == context.preferred_category) {
            score += 0.3f;
        }
        
        // Tag match
        float tag_match_score = calculateTagMatch(preset.tags, context.tags);
        score += tag_match_score * 0.4f;
        
        // Search query match
        if (!context.search_query.empty()) {
            float search_score = calculateSearchMatch(preset, context.search_query);
            score += search_score * 0.2f;
        }
        
        // Popularity (rating and usage)
        score += preset.rating / 5.0f * 0.05f;
        score += std::min(preset.usage_count / 100.0f, 1.0f) * 0.05f;
        
        // Favorites boost
        if (context.use_favorites && preset.is_favorite) {
            score += 0.1f;
        }
        
        return score;
    }

    float calculateTagMatch(const std::string& preset_tags, const std::vector<std::string>& query_tags) {
        if (query_tags.empty()) return 0.0f;
        
        int matches = 0;
        std::string lower_preset_tags = preset_tags;
        std::transform(lower_preset_tags.begin(), lower_preset_tags.end(), 
                      lower_preset_tags.begin(), ::tolower);
        
        for (const auto& query_tag : query_tags) {
            std::string lower_query = query_tag;
            std::transform(lower_query.begin(), lower_query.end(), 
                          lower_query.begin(), ::tolower);
            
            if (lower_preset_tags.find(lower_query) != std::string::npos) {
                matches++;
            }
        }
        
        return static_cast<float>(matches) / query_tags.size();
    }

    float calculateSearchMatch(const VoicePresetNavigator::PresetInfo& preset, 
                             const std::string& query) {
        std::string lower_query = query;
        std::transform(lower_query.begin(), lower_query.end(), 
                      lower_query.begin(), ::tolower);
        
        // Check name match
        std::string lower_name = preset.name;
        std::transform(lower_name.begin(), lower_name.end(), 
                      lower_name.begin(), ::tolower);
        
        if (lower_name.find(lower_query) != std::string::npos) {
            return 1.0f;
        }
        
        // Check description match
        std::string lower_desc = preset.description;
        std::transform(lower_desc.begin(), lower_desc.end(), 
                      lower_desc.begin(), ::tolower);
        
        if (lower_desc.find(lower_query) != std::string::npos) {
            return 0.7f;
        }
        
        // Check tag match
        if (calculateTagMatch(preset.tags, {query}) > 0.0f) {
            return 0.8f;
        }
        
        return 0.0f;
    }
};

/**
 * @brief Preset database manager
 */
class PresetDatabase {
public:
    bool loadFromFile(const std::string& file_path) {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            return false;
        }
        
        // Simple JSON-like parsing
        std::string line;
        VoicePresetNavigator::PresetInfo current_preset;
        bool parsing_preset = false;
        
        while (std::getline(file, line)) {
            if (line.find("{") != std::string::npos) {
                parsing_preset = true;
                current_preset = VoicePresetNavigator::PresetInfo{};
                continue;
            }
            
            if (line.find("}") != std::string::npos) {
                if (parsing_preset) {
                    presets_.push_back(current_preset);
                    parsing_preset = false;
                }
                continue;
            }
            
            if (parsing_preset) {
                parsePresetProperty(line, current_preset);
            }
        }
        
        return true;
    }

    void saveToFile(const std::string& file_path) const {
        std::ofstream file(file_path);
        if (!file.is_open()) {
            return;
        }
        
        for (const auto& preset : presets_) {
            file << "{\n";
            file << "  \"id\": \"" << preset.id << "\",\n";
            file << "  \"name\": \"" << preset.name << "\",\n";
            file << "  \"author\": \"" << preset.author << "\",\n";
            file << "  \"category\": " << static_cast<int>(preset.category) << ",\n";
            file << "  \"tags\": \"" << preset.tags << "\",\n";
            file << "  \"description\": \"" << preset.description << "\",\n";
            file << "  \"rating\": " << preset.rating << ",\n";
            file << "  \"usage_count\": " << preset.usage_count << ",\n";
            file << "  \"is_favorite\": " << (preset.is_favorite ? "true" : "false") << "\n";
            file << "}\n";
        }
    }

    const std::vector<VoicePresetNavigator::PresetInfo>& getAllPresets() const {
        return presets_;
    }

    void addPreset(const VoicePresetNavigator::PresetInfo& preset) {
        presets_.push_back(preset);
    }

    void removePreset(const std::string& preset_id) {
        presets_.erase(
            std::remove_if(presets_.begin(), presets_.end(),
                [&preset_id](const VoicePresetNavigator::PresetInfo& p) {
                    return p.id == preset_id;
                }),
            presets_.end()
        );
    }

    std::vector<VoicePresetNavigator::PresetInfo> getByCategory(VoicePresetNavigator::PresetCategory category) const {
        std::vector<VoicePresetNavigator::PresetInfo> category_presets;
        
        for (const auto& preset : presets_) {
            if (preset.category == category) {
                category_presets.push_back(preset);
            }
        }
        
        return category_presets;
    }

private:
    void parsePresetProperty(const std::string& line, VoicePresetNavigator::PresetInfo& preset) {
        // Simple key-value parsing
        size_t colon_pos = line.find(":");
        if (colon_pos == std::string::npos) return;
        
        std::string key = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);
        
        // Remove quotes and commas
        key.erase(std::remove(key.begin(), key.end(), '\"'), key.end());
        key.erase(std::remove(key.begin(), key.end(), ' '), key.end());
        value.erase(std::remove(value.begin(), value.end(), '\"'), value.end());
        value.erase(std::remove(value.begin(), value.end(), ','), value.end());
        
        if (key == "id") {
            preset.id = value;
        } else if (key == "name") {
            preset.name = value;
        } else if (key == "author") {
            preset.author = value;
        } else if (key == "category") {
            preset.category = static_cast<VoicePresetNavigator::PresetCategory>(std::stoi(value));
        } else if (key == "tags") {
            preset.tags = value;
        } else if (key == "description") {
            preset.description = value;
        } else if (key == "rating") {
            preset.rating = std::stof(value);
        } else if (key == "usage_count") {
            preset.usage_count = std::stoi(value);
        } else if (key == "is_favorite") {
            preset.is_favorite = (value == "true");
        }
    }

    std::vector<VoicePresetNavigator::PresetInfo> presets_;
};

/**
 * @brief Implementation class for VoicePresetNavigator
 */
class VoicePresetNavigator::Impl {
public:
    Impl()
        : recommendation_engine_(std::make_unique<RecommendationEngine>())
        , database_(std::make_unique<PresetDatabase>())
        , feature_analyzer_(std::make_unique<AudioFeatureAnalyzer>())
        , smart_navigation_enabled_(true)
        , similarity_threshold_(0.7f)
        , max_recommendations_(5) {
        
        initializeDefaultPresets();
    }

    void loadPresetDatabase(const std::string& database_path) {
        std::lock_guard<std::mutex> lock(mutex_);
        database_->loadFromFile(database_path);
    }

    void scanPresetDirectories(const std::vector<std::string>& directories) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Scan directories for preset files
        for (const auto& directory : directories) {
            // In a real implementation, scan for .vitalpreset files
            // For now, just add some placeholder data
        }
    }

    void addPreset(const PresetInfo& preset) {
        std::lock_guard<std::mutex> lock(mutex_);
        database_->addPreset(preset);
        
        // Extract audio features if audio data is available
        if (!preset.audio_features.empty()) {
            auto features = AudioFeatureAnalyzer::extractFeatures(preset.audio_features);
            // Store features for similarity calculations
        }
    }

    void removePreset(const std::string& preset_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        database_->removePreset(preset_id);
    }

    std::vector<PresetInfo> getAllPresets() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return database_->getAllPresets();
    }

    std::vector<PresetInfo> getPresetsByCategory(PresetCategory category) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return database_->getByCategory(category);
    }

    void navigateToPreset(const std::string& preset_name) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto all_presets = database_->getAllPresets();
        
        // Find preset by name (fuzzy matching)
        PresetInfo* best_match = nullptr;
        float best_score = 0.0f;
        
        for (const auto& preset : all_presets) {
            float score = calculateNameSimilarity(preset.name, preset_name);
            if (score > best_score && score > 0.6f) {
                best_score = score;
                best_match = const_cast<PresetInfo*>(&preset);
            }
        }
        
        if (best_match) {
            // Notify callbacks
            for (const auto& callback : preset_callbacks_) {
                if (callback) {
                    callback(*best_match);
                }
            }
            
            // Mark as used
            markAsUsed(best_match->id);
        }
    }

    void navigateDirection(NavigationDirection direction) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto all_presets = database_->getAllPresets();
        if (all_presets.empty()) return;
        
        PresetInfo* target_preset = nullptr;
        
        switch (direction) {
            case NavigationDirection::Next:
                target_preset = getNextPreset(all_presets);
                break;
            case NavigationDirection::Previous:
                target_preset = getPreviousPreset(all_presets);
                break;
            case NavigationDirection::First:
                target_preset = &all_presets.front();
                break;
            case NavigationDirection::Last:
                target_preset = &all_presets.back();
                break;
            case NavigationDirection::Random:
                target_preset = getRandomPreset(all_presets);
                break;
            case NavigationDirection::Similar:
                target_preset = getSimilarPreset(all_presets);
                break;
            case NavigationDirection::Favorite:
                target_preset = getFavoritePreset(all_presets);
                break;
            case NavigationDirection::Recent:
                target_preset = getRecentPreset(all_presets);
                break;
        }
        
        if (target_preset) {
            // Notify callbacks
            for (const auto& callback : navigation_callbacks_) {
                if (callback) {
                    callback(direction, *target_preset);
                }
            }
            
            // Mark as used
            markAsUsed(target_preset->id);
        }
    }

    void navigateToCategory(PresetCategory category) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto category_presets = database_->getByCategory(category);
        if (category_presets.empty()) return;
        
        // Navigate to first preset in category
        navigateDirection(NavigationDirection::First);
        
        // Update navigation context
        context_.current_category = category;
    }

    void searchPresets(const std::string& query) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto all_presets = database_->getAllPresets();
        std::vector<PresetInfo> search_results;
        
        for (const auto& preset : all_presets) {
            float score = calculateSearchScore(preset, query);
            if (score > 0.3f) {
                search_results.push_back(preset);
            }
        }
        
        // Sort by relevance
        std::sort(search_results.begin(), search_results.end(),
            [&query](const PresetInfo& a, const PresetInfo& b) {
                return calculateSearchScore(a, query) > calculateSearchScore(b, query);
            });
        
        // Notify callbacks
        for (const auto& callback : search_callbacks_) {
            if (callback) {
                callback(search_results);
            }
        }
    }

    void navigateByTags(const std::vector<std::string>& tags) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto all_presets = database_->getAllPresets();
        std::vector<PresetInfo> tagged_presets;
        
        for (const auto& preset : all_presets) {
            float tag_score = calculateTagScore(preset.tags, tags);
            if (tag_score > 0.5f) {
                tagged_presets.push_back(preset);
            }
        }
        
        if (!tagged_presets.empty()) {
            // Navigate to first tagged preset
            const PresetInfo& target = tagged_presets.front();
            for (const auto& callback : preset_callbacks_) {
                if (callback) {
                    callback(target);
                }
            }
            markAsUsed(target.id);
        }
    }

    void toggleFavorite(const std::string& preset_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto& all_presets = const_cast<std::vector<PresetInfo>&>(database_->getAllPresets());
        
        for (auto& preset : all_presets) {
            if (preset.id == preset_id) {
                preset.is_favorite = !preset.is_favorite;
                break;
            }
        }
    }

    void markAsUsed(const std::string& preset_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto& all_presets = const_cast<std::vector<PresetInfo>&>(database_->getAllPresets());
        
        for (auto& preset : all_presets) {
            if (preset.id == preset_id) {
                preset.usage_count++;
                preset.last_used = std::chrono::system_clock::now();
                break;
            }
        }
        
        // Add to recent presets
        recent_presets_.push_back(preset_id);
        if (recent_presets_.size() > 50) {
            recent_presets_.erase(recent_presets_.begin());
        }
    }

    std::vector<PresetInfo> getFavoritePresets() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto all_presets = database_->getAllPresets();
        std::vector<PresetInfo> favorites;
        
        for (const auto& preset : all_presets) {
            if (preset.is_favorite) {
                favorites.push_back(preset);
            }
        }
        
        return favorites;
    }

    std::vector<PresetInfo> getRecentPresets(int count) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto all_presets = database_->getAllPresets();
        std::vector<PresetInfo> recent;
        
        // Get presets from recent_presets_ list
        for (auto it = recent_presets_.rbegin(); 
             it != recent_presets_.rend() && recent.size() < count; ++it) {
            
            for (const auto& preset : all_presets) {
                if (preset.id == *it) {
                    recent.push_back(preset);
                    break;
                }
            }
        }
        
        return recent;
    }

    void enableSmartNavigation(bool enabled) {
        std::lock_guard<std::mutex> lock(mutex_);
        smart_navigation_enabled_ = enabled;
    }

    void setSmartPreferences(float similarity_threshold, int max_recommendations) {
        std::lock_guard<std::mutex> lock(mutex_);
        similarity_threshold_ = std::clamp(similarity_threshold, 0.0f, 1.0f);
        max_recommendations_ = max_recommendations;
    }

    std::vector<PresetInfo> getSimilarPresets(const std::string& preset_id) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto all_presets = database_->getAllPresets();
        PresetInfo* target_preset = nullptr;
        
        // Find target preset
        for (const auto& preset : all_presets) {
            if (preset.id == preset_id) {
                target_preset = const_cast<PresetInfo*>(&preset);
                break;
            }
        }
        
        if (!target_preset) return {};
        
        std::vector<PresetInfo> similar_presets;
        
        for (const auto& preset : all_presets) {
            if (preset.id != preset_id) {
                float similarity = calculatePresetSimilarity(*target_preset, preset);
                if (similarity >= similarity_threshold_) {
                    similar_presets.push_back(preset);
                }
            }
        }
        
        // Sort by similarity
        std::sort(similar_presets.begin(), similar_presets.end(),
            [&target_preset, this](const PresetInfo& a, const PresetInfo& b) {
                return calculatePresetSimilarity(*target_preset, a) > 
                       calculatePresetSimilarity(*target_preset, b);
            });
        
        return similar_presets;
    }

    std::vector<PresetInfo> getRecommendations() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        RecommendationEngine::RecommendationContext context;
        context.direction = NavigationDirection::Similar;
        context.max_recommendations = max_recommendations_;
        context.preferred_category = context_.current_category;
        context.use_favorites = true;
        context.use_history = true;
        
        auto all_presets = database_->getAllPresets();
        auto recommendations = recommendation_engine_->generateRecommendations(all_presets, context);
        
        std::vector<PresetInfo> result;
        for (const auto& rec : recommendations) {
            result.push_back(*rec.preset);
        }
        
        return result;
    }

    void registerPresetCallback(PresetCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        preset_callbacks_.push_back(callback);
    }

    void registerNavigationCallback(NavigationCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        navigation_callbacks_.push_back(callback);
    }

    void registerSearchCallback(SearchCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        search_callbacks_.push_back(callback);
    }

private:
    void initializeDefaultPresets() {
        // Load sample presets
        std::vector<PresetInfo> sample_presets = {
            // Pad presets
            {"preset_001", "Cosmic Pad", "Vital Sound Lab", PresetCategory::Pad, 
             "atmospheric, ambient, pads, warm", "Ethereal atmospheric pad with warm harmonics and evolving textures", 
             4.5f, 0, std::chrono::system_clock::now(), false, "", {}},
            
            {"preset_002", "Celestial Echo", "Vital Sound Lab", PresetCategory::Pad,
             "space, atmospheric, reverb, dreamy", "Wide stereo pad with long reverb tail", 
             4.7f, 0, std::chrono::system_clock::now(), true, "", {}},
            
            // Bass presets
            {"preset_003", "Analog Bass", "Vital Sound Lab", PresetCategory::Bass,
             "bass, analog, warm, punchy", "Classic analog-style bass synthesizer", 
             4.8f, 0, std::chrono::system_clock::now(), true, "", {}},
            
            {"preset_004", "Deep Sub", "Vital Sound Lab", PresetCategory::Bass,
             "sub, bass, low-end, heavy", "Deep sub-bass for electronic music", 
             4.6f, 0, std::chrono::system_clock::now(), false, "", {}},
            
            // Lead presets
            {"preset_005", "Digital Leader", "Vital Sound Lab", PresetCategory::Lead,
             "lead, digital, bright, cutting", "Bright digital lead with filter sweep", 
             4.4f, 0, std::chrono::system_clock::now(), false, "", {}},
            
            {"preset_006", "Warm Analog", "Vital Sound Lab", PresetCategory::Lead,
             "lead, analog, vintage, warm", "Vintage-style analog lead", 
             4.7f, 0, std::chrono::system_clock::now(), true, "", {}},
            
            // Percussion
            {"preset_007", "Electronic Kick", "Vital Sound Lab", PresetCategory::Percussion,
             "kick, percussion, electronic, punch", "Punchy electronic kick drum", 
             4.3f, 0, std::chrono::system_clock::now(), false, "", {}},
            
            {"preset_008", "Tonal Percussion", "Vital Sound Lab", PresetCategory::Percussion,
             "tonal, percussion, melodic, rhythmic", "Tonal percussion for rhythm tracks", 
             4.2f, 0, std::chrono::system_clock::now(), false, "", {}},
            
            // Effects
            {"preset_009", "Spectral Sweep", "Vital Sound Lab", PresetCategory::Effects,
             "sweep, filter, spectral, transition", "Spectral sweep effect for transitions", 
             4.1f, 0, std::chrono::system_clock::now(), false, "", {}},
            
            {"preset_010", "Granular Wash", "Vital Sound Lab", PresetCategory::Effects,
             "granular, texture, ambient, wash", "Granular texture wash", 
             4.5f, 0, std::chrono::system_clock::now(), true, "", {}},
            
            // Atmospheric
            {"preset_011", "Ambient Drone", "Vital Sound Lab", PresetCategory::Atmospheric,
             "drone, ambient, sustained, evolving", "Slow-evolving ambient drone", 
             4.6f, 0, std::chrono::system_clock::now(), false, "", {}},
            
            {"preset_012", "Shimmering Texture", "Vital Sound Lab", PresetCategory::Atmospheric,
             "shimmer, texture, high-frequency, ethereal", "High-frequency shimmering texture", 
             4.8f, 0, std::chrono::system_clock::now(), true, "", {}}
        };
        
        for (const auto& preset : sample_presets) {
            database_->addPreset(preset);
        }
    }

    float calculateNameSimilarity(const std::string& name1, const std::string& name2) {
        // Simple Levenshtein distance-based similarity
        std::string n1 = name1;
        std::string n2 = name2;
        std::transform(n1.begin(), n1.end(), n1.begin(), ::tolower);
        std::transform(n2.begin(), n2.end(), n2.begin(), ::tolower);
        
        int len1 = n1.length();
        int len2 = n2.length();
        
        if (len1 == 0) return len2 == 0 ? 1.0f : 0.0f;
        if (len2 == 0) return 0.0f;
        
        std::vector<std::vector<int>> matrix(len1 + 1, std::vector<int>(len2 + 1));
        
        for (int i = 0; i <= len1; i++) matrix[i][0] = i;
        for (int j = 0; j <= len2; j++) matrix[0][j] = j;
        
        for (int i = 1; i <= len1; i++) {
            for (int j = 1; j <= len2; j++) {
                int cost = (n1[i-1] == n2[j-1]) ? 0 : 1;
                matrix[i][j] = std::min({
                    matrix[i-1][j] + 1,
                    matrix[i][j-1] + 1,
                    matrix[i-1][j-1] + cost
                });
            }
        }
        
        int distance = matrix[len1][len2];
        int max_len = std::max(len1, len2);
        return static_cast<float>(max_len - distance) / max_len;
    }

    float calculateSearchScore(const PresetInfo& preset, const std::string& query) {
        float score = 0.0f;
        
        // Name match
        float name_sim = calculateNameSimilarity(preset.name, query);
        score += name_sim * 0.4f;
        
        // Tag match
        if (preset.tags.find(query) != std::string::npos) {
            score += 0.3f;
        }
        
        // Description match
        if (preset.description.find(query) != std::string::npos) {
            score += 0.2f;
        }
        
        // Author match
        if (preset.author.find(query) != std::string::npos) {
            score += 0.1f;
        }
        
        return std::min(score, 1.0f);
    }

    float calculateTagScore(const std::string& preset_tags, const std::vector<std::string>& query_tags) {
        if (query_tags.empty()) return 0.0f;
        
        int matches = 0;
        std::string lower_tags = preset_tags;
        std::transform(lower_tags.begin(), lower_tags.end(), lower_tags.begin(), ::tolower);
        
        for (const auto& query_tag : query_tags) {
            std::string lower_query = query_tag;
            std::transform(lower_query.begin(), lower_query.end(), 
                          lower_query.begin(), ::tolower);
            
            if (lower_tags.find(lower_query) != std::string::npos) {
                matches++;
            }
        }
        
        return static_cast<float>(matches) / query_tags.size();
    }

    float calculatePresetSimilarity(const PresetInfo& preset1, const PresetInfo& preset2) {
        float similarity = 0.0f;
        
        // Category match
        if (preset1.category == preset2.category) {
            similarity += 0.3f;
        }
        
        // Tag similarity
        similarity += calculateTagScore(preset1.tags, {preset2.tags}) * 0.4f;
        
        // Rating similarity
        float rating_diff = std::abs(preset1.rating - preset2.rating);
        similarity += (1.0f - rating_diff / 5.0f) * 0.2f;
        
        // Audio features similarity (if available)
        if (!preset1.audio_features.empty() && !preset2.audio_features.empty()) {
            auto features1 = AudioFeatureAnalyzer::extractFeatures(preset1.audio_features);
            auto features2 = AudioFeatureAnalyzer::extractFeatures(preset2.audio_features);
            similarity += AudioFeatureAnalyzer::computeSimilarity(features1, features2) * 0.1f;
        }
        
        return std::min(similarity, 1.0f);
    }

    PresetInfo* getNextPreset(const std::vector<PresetInfo>& presets) {
        if (context_.current_index >= 0 && context_.current_index < static_cast<int>(presets.size()) - 1) {
            context_.current_index++;
        } else {
            context_.current_index = 0; // Wrap around
        }
        
        return const_cast<PresetInfo*>(&presets[context_.current_index]);
    }

    PresetInfo* getPreviousPreset(const std::vector<PresetInfo>& presets) {
        if (context_.current_index > 0) {
            context_.current_index--;
        } else {
            context_.current_index = presets.size() - 1; // Wrap around
        }
        
        return const_cast<PresetInfo*>(&presets[context_.current_index]);
    }

    PresetInfo* getRandomPreset(const std::vector<PresetInfo>& presets) {
        if (presets.empty()) return nullptr;
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, presets.size() - 1);
        
        context_.current_index = dis(gen);
        return const_cast<PresetInfo*>(&presets[context_.current_index]);
    }

    PresetInfo* getSimilarPreset(const std::vector<PresetInfo>& presets) {
        if (context_.current_index < 0 || context_.current_index >= static_cast<int>(presets.size())) {
            return getRandomPreset(presets);
        }
        
        const PresetInfo& current_preset = presets[context_.current_index];
        
        // Find most similar preset
        PresetInfo* best_match = nullptr;
        float best_similarity = 0.0f;
        
        for (size_t i = 0; i < presets.size(); i++) {
            if (i != static_cast<size_t>(context_.current_index)) {
                float similarity = calculatePresetSimilarity(current_preset, presets[i]);
                if (similarity > best_similarity) {
                    best_similarity = similarity;
                    best_match = const_cast<PresetInfo*>(&presets[i]);
                }
            }
        }
        
        if (best_match) {
            // Update index to match the similar preset
            for (size_t i = 0; i < presets.size(); i++) {
                if (presets[i].id == best_match->id) {
                    context_.current_index = static_cast<int>(i);
                    break;
                }
            }
        }
        
        return best_match;
    }

    PresetInfo* getFavoritePreset(const std::vector<PresetInfo>& presets) {
        for (const auto& preset : presets) {
            if (preset.is_favorite) {
                // Update index
                for (size_t i = 0; i < presets.size(); i++) {
                    if (presets[i].id == preset.id) {
                        context_.current_index = static_cast<int>(i);
                        break;
                    }
                }
                return const_cast<PresetInfo*>(&preset);
            }
        }
        
        return nullptr;
    }

    PresetInfo* getRecentPreset(const std::vector<PresetInfo>& presets) {
        for (const auto& preset_id : recent_presets_) {
            for (size_t i = 0; i < presets.size(); i++) {
                if (presets[i].id == preset_id) {
                    context_.current_index = static_cast<int>(i);
                    return const_cast<PresetInfo*>(&presets[i]);
                }
            }
        }
        
        return nullptr;
    }

    NavigationContext context_;
    std::vector<std::string> recent_presets_;
    
    std::vector<PresetCallback> preset_callbacks_;
    std::vector<NavigationCallback> navigation_callbacks_;
    std::vector<SearchCallback> search_callbacks_;
    
    std::unique_ptr<RecommendationEngine> recommendation_engine_;
    std::unique_ptr<PresetDatabase> database_;
    std::unique_ptr<AudioFeatureAnalyzer> feature_analyzer_;
    
    bool smart_navigation_enabled_;
    float similarity_threshold_;
    int max_recommendations_;
    
    mutable std::mutex mutex_;
};

// VoicePresetNavigator implementation
VoicePresetNavigator::VoicePresetNavigator() 
    : p_impl_(std::make_unique<Impl>()) {
}

VoicePresetNavigator::~VoicePresetNavigator() = default;

void VoicePresetNavigator::loadPresetDatabase(const std::string& database_path) {
    p_impl_->loadPresetDatabase(database_path);
}

void VoicePresetNavigator::scanPresetDirectories(const std::vector<std::string>& directories) {
    p_impl_->scanPresetDirectories(directories);
}

void VoicePresetNavigator::addPreset(const PresetInfo& preset) {
    p_impl_->addPreset(preset);
}

void VoicePresetNavigator::removePreset(const std::string& preset_id) {
    p_impl_->removePreset(preset_id);
}

std::vector<VoicePresetNavigator::PresetInfo> VoicePresetNavigator::getAllPresets() const {
    return p_impl_->getAllPresets();
}

std::vector<VoicePresetNavigator::PresetInfo> VoicePresetNavigator::getPresetsByCategory(PresetCategory category) const {
    return p_impl_->getPresetsByCategory(category);
}

void VoicePresetNavigator::navigateToPreset(const std::string& preset_name) {
    p_impl_->navigateToPreset(preset_name);
}

void VoicePresetNavigator::navigateDirection(NavigationDirection direction) {
    p_impl_->navigateDirection(direction);
}

void VoicePresetNavigator::navigateToCategory(PresetCategory category) {
    p_impl_->navigateToCategory(category);
}

void VoicePresetNavigator::searchPresets(const std::string& query) {
    p_impl_->searchPresets(query);
}

void VoicePresetNavigator::navigateByTags(const std::vector<std::string>& tags) {
    p_impl_->navigateByTags(tags);
}

void VoicePresetNavigator::toggleFavorite(const std::string& preset_id) {
    p_impl_->toggleFavorite(preset_id);
}

void VoicePresetNavigator::markAsUsed(const std::string& preset_id) {
    p_impl_->markAsUsed(preset_id);
}

std::vector<VoicePresetNavigator::PresetInfo> VoicePresetNavigator::getFavoritePresets() const {
    return p_impl_->getFavoritePresets();
}

std::vector<VoicePresetNavigator::PresetInfo> VoicePresetNavigator::getRecentPresets(int count) const {
    return p_impl_->getRecentPresets(count);
}

void VoicePresetNavigator::enableSmartNavigation(bool enabled) {
    p_impl_->enableSmartNavigation(enabled);
}

void VoicePresetNavigator::setSmartPreferences(float similarity_threshold, int max_recommendations) {
    p_impl_->setSmartPreferences(similarity_threshold, max_recommendations);
}

std::vector<VoicePresetNavigator::PresetInfo> VoicePresetNavigator::getSimilarPresets(const std::string& preset_id) const {
    return p_impl_->getSimilarPresets(preset_id);
}

std::vector<VoicePresetNavigator::PresetInfo> VoicePresetNavigator::getRecommendations() const {
    return p_impl_->getRecommendations();
}

void VoicePresetNavigator::registerPresetCallback(PresetCallback callback) {
    p_impl_->registerPresetCallback(callback);
}

void VoicePresetNavigator::registerNavigationCallback(NavigationCallback callback) {
    p_impl_->registerNavigationCallback(callback);
}

void VoicePresetNavigator::registerSearchCallback(SearchCallback callback) {
    p_impl_->registerSearchCallback(callback);
}

} // namespace voice_control
} // namespace vital