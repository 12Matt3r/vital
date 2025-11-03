#pragma once

#include "core/application.h"
#include "core/version.h"
#include "core/plugin_interface.h"
#include "core/plugin_bridge.h"

#include "synth/synthesizer.h"
#include "synth/voice.h"
#include "synth/voice_manager.h"
#include "synth/patch.h"
#include "synth/note_processor.h"

#include "dsp/oscillator.h"
#include "dsp/filter.h"
#include "dsp/envelope.h"
#include "dsp/lfo.h"
#include "dsp/mixer.h"
#include "dsp/keyboard.h"
#include "dsp/automation.h"

#include "effects/reverb.h"
#include "effects/chorus.h"
#include "effects/delay.h"
#include "effects/distortion.h"
#include "effects/compressor.h"
#include "effects/eq.h"

#include "midi/midi_input.h"
#include "midi/midi_output.h"
#include "midi/midi_parser.h"
#include "midi/midi_mapping.h"

#include "ui/main_window.h"
#include "ui/editor_window.h"
#include "ui/components/knob.h"
#include "ui/components/slider.h"
#include "ui/components/button.h"
#include "ui/components/waveform_display.h"
#include "ui/components/keyboard_widget.h"

#include "config/settings.h"
#include "config/file_handler.h"
#include "config/preset_manager.h"

#include "math/math_utils.h"
#include "math/signal_processing.h"

#include "io/audio_output.h"
#include "io/midi_io.h"

namespace vital {

} // namespace vital