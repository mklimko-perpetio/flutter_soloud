#pragma once

#ifndef PLAYER_H
#define PLAYER_H

#include "enums.h"
#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_speech.h"

#include <iostream>
#include <vector>
#include <map>
#include <mutex>
#include <memory>
#include <atomic>
#include <thread>

typedef enum PlayerMessages
{
    MSG_NONE,
    MSG_STOP
} PlayerMessages_t;

/// The default number of concurrent voices - maximum number of "streams" - is 16,
/// but this can be adjusted at runtime
struct ActiveSound
{
    SoLoud::Wav sound;
    std::string completeFileName;
    /// many istances of [sound] can be played without re-loading it
    std::vector<SoLoud::handle> handle;

    // unique identifier of this sound based on the file name
    unsigned int soundHash;
};

class Player
{
public:
    Player();
    ~Player();

    /// @brief Initialize the player. Must be called before any other player functions
    /// @return Returns [PlayerErrors.SO_NO_ERROR] if success
    PlayerErrors init();

    /// @brief Must be called when there is no more need of the player or when closing the app
    /// @return
    void dispose();

    bool isInited();
    int getSoundsCount();

    /// @brief
    /// @param aErrorCode
    /// @return a string represented by the PlayerErrors code
    const std::string getErrorString(PlayerErrors aErrorCode) const;

    /// @brief Load a new sound to be played once or multiple times later
    /// @param completeFileName the complete file path + file name
    /// @param hash return the hash of the sound
    /// @return Returns [PlayerErrors.SO_NO_ERROR] if success
    PlayerErrors loadFile(const std::string &completeFileName, unsigned int &hash);

    /// @brief Pause or unpause already loaded sound identified by [handle]
    /// @param handle the sound handle
    void pauseSwitch(unsigned int handle);

    /// @brief Gets the pause state
    /// @param handle the sound handle
    /// @return true if paused
    bool getPause(unsigned int handle);

    /// @brief Play already loaded sound identified by [soundHash]
    /// @param soundHash
    /// @param volume 1.0f full volume
    /// @param pan 0.0f centered
    /// @param paused 0 not pause
    /// @return the handle of the sound, 0 if error
    unsigned int play(
        unsigned int soundHash,
        float volume = 1.0f,
        float pan = 0.0f,
        bool paused = 0);

    /// @brief Stop already loaded sound identified by [handle] and clear it
    /// @param handle
    void stop(unsigned int handle);

    /// @brief Stop all handles of the already loaded sound identified by [soundHash] and clear it
    /// @param soundHash
    void stopSound(unsigned int soundHash);

    /// @brief This function can be used to set a sample to play on repeat, 
    /// instead of just playing once
    /// @param handle
    /// @param enable
    void setLooping(unsigned int handle, bool enable);

    /// @brief Speech
    /// @param textToSpeech
    /// @param handle handle of the sound. -1 if error
    /// @return Returns [PlayerErrors.SO_NO_ERROR] if success
    /// TODO: add other T2S parameters
    PlayerErrors textToSpeech(const std::string &textToSpeech, unsigned int &handle);

    /// @brief Enable or disable visualization
    /// @param enabled
    /// @return
    void setVisualizationEnabled(bool enabled);

    bool isVisualizationEnabled();

    /// @brief Calculates FFT of the currently playing sound
    /// @return a 256 float pointer to the result
    float *calcFFT();

    /// @brief Gets 256 samples of the currently playing sound
    /// @return a 256 float pointer to the result
    float *getWave();

    /// @brief get the sound length in seconds
    /// @param soundHash
    /// @return returns sound length in seconds
    double getLength(unsigned int soundHash);

    /// @brief seek playing in [time] seconds
    /// @param handle the sound handle
    /// @param time the time to seek in seconds
    /// @return Returns [PlayerErrors.SO_NO_ERROR] if success
    PlayerErrors seek(SoLoud::handle handle, float time);

    /// @brief get current sound position in seconds
    /// @return time in seconds
    double getPosition(SoLoud::handle handle);

    /// @brief check if a handle is still valid.
    /// @param handle handle to check
    /// @return true if it still exists
    bool getIsValidVoiceHandle(SoLoud::handle handle);

    /// @brief Find a sound by its handle
    /// @param handle
    /// @return If not found, return nullptr.
    ///    [handleId] is the index of the handles of the sound found
    ActiveSound *findByHandle(SoLoud::handle handle, int *handleId);

    void debug();

    /// @brief apply change to voices when changing some 3d audio params
    void update3dAudio();

    /// @brief play3d() is the 3d version of the play() call
    /// @return the handle of the sound, 0 if error
    unsigned int play3d(
        unsigned int soundHash,
        float posX,
        float posY,
        float posZ,
        float velX = 0.0f,
        float velY = 0.0f,
        float velZ = 0.0f,
        float volume = 1.0f,
        bool paused = 0,
        unsigned int bus = 0);

    /// You can set and get the current value of the speed of
    /// sound width the get3dSoundSpeed() and set3dSoundSpeed() functions.
    /// The speed of sound is used to calculate doppler effects in
    /// addition to the distance delay.

    /// Since SoLoud has no knowledge of the scale of your coordinates,
    /// you may need to adjust the speed of sound for these effects
    /// to work correctly. The default value is 343, which assumes
    /// that your world coordinates are in meters (where 1 unit is 1 meter),
    /// and that the environment is dry air at around 20 degrees Celsius.
    void set3dSoundSpeed(float speed);
    float get3dSoundSpeed();

    void set3dListenerParameters(
        float posX, float posY, float posZ,
        float atX, float atY, float atZ,
        float upX, float upY, float upZ,
        float velocityX, float velocityY, float velocityZ);
    void set3dListenerPosition(float posX,
                               float posY,
                               float posZ);
    void set3dListenerAt(float atX,
                         float atY,
                         float atZ);
    void set3dListenerUp(float upX,
                         float upY,
                         float upZ);
    void set3dListenerVelocity(float velocityX,
                               float velocityY,
                               float velocityZ);

    void set3dSourceParameters(unsigned int handle,
                               float posX,
                               float posY,
                               float posZ,
                               float velocityX,
                               float velocityY,
                               float velocityZ);
    void set3dSourcePosition(unsigned int handle,
                             float posX,
                             float posY,
                             float posZ);
    void set3dSourceVelocity(unsigned int handle,
                             float velocityX,
                             float velocityY,
                             float velocityZ);
    void set3dSourceMinMaxDistance(unsigned int handle,
                                   float minDistance,
                                   float maxDistance);
    void set3dSourceAttenuation(unsigned int handle,
                                unsigned int attenuationModel,
                                float attenuationRolloffFactor);
    void set3dSourceDopplerFactor(unsigned int handle,
                                  float dopplerFactor);

public:
    /// all the sounds loaded
    std::vector<std::unique_ptr<ActiveSound>> sounds;

    /// true when the backend is initialized
    bool mInited;

    /// main SoLoud engine
    SoLoud::Soloud soloud;

    /// speech object
    SoLoud::Speech speech;
};

#endif // PLAYER_H
