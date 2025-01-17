#include "PlaySoundDelayedSF.h"
#include "AudioManager.h"

namespace argosClient {

  PlaySoundDelayedSF::PlaySoundDelayedSF()
    : ScriptFunction("SoundDelayed", "PlaySoundDelayedSF"),
      _audioManager(AudioManager::getInstance()) {

  }

  void PlaySoundDelayedSF::_execute(const std::vector<std::string>& args, int id) {
    AudioManager::getInstance().play(args[0], getArgAsInt(args[1]));
  }

}
