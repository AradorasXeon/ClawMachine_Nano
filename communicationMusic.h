#include <Wire.h>

#define I2C_MUSIC_CTRL_ADDRESS 0xF1

enum class CurrentMusic : uint8_t
{
    NOT_SET = 0,
    CALIBRATION = 1,
    GAME_PLAY = 2,
    CLAW_ACTION = 3,
    PRIZE_DROP = 4
};

class Music
{
    public:
    Music(bool isMaster);
    void sendMsg();
    static void onReceiveCallBack(int byteCount);

    void setCalibrationMusic();
    void setGamePlayMusic();
    void setClawActionMusic();
    void setPrizeDropMusic();

    CurrentMusic getMusicToPlay();
    static Music* instance; //for wire library hack

    private:
    void readMsg(int byteCount);
    CurrentMusic _music = CurrentMusic::NOT_SET;
};

/// @brief ctor for the Music com interface
/// @param isMaster true if this is the i2c master(nano)
Music::Music(bool isMaster)
{
    if(isMaster)
    {
        Wire.begin();
    }
    else
    {
        Wire.begin(I2C_MUSIC_CTRL_ADDRESS);
    }
}

/// @brief sends the msg on i2c
void Music::sendMsg()
{
    Wire.beginTransmission(I2C_MUSIC_CTRL_ADDRESS);
    Wire.write((uint8_t)_music);
    Wire.endTransmission(true);
    //timer.doDelay();
    #ifdef DEBUG
    Serial.println("MUSIC -- Done Writing to SLAVE");
    #endif // DEBUG
}

///@brief This is to hack the Wire.onReceive function, should be used ike: Wire.onReceive(Music::onReceiveCallBack); 
void Music::onReceiveCallBack(int byteCount)
{
    if (instance != nullptr)
    {
        instance->readMsg(byteCount);
    }
}

void Music::readMsg(int byteCount)
{   
    _music = (CurrentMusic)Wire.read();
    
    #ifdef DEBUG
    Serial.println("MUSIC -- Done reading from MASTER");
    #endif // DEBUG
}


/// @brief Sets the Calibration Music
void Music::setCalibrationMusic()
{
    _music = CurrentMusic::CALIBRATION;
}

/// @brief Sets the Normal Gameplay music
void Music::setGamePlayMusic()
{
    _music = CurrentMusic::GAME_PLAY;
}

/// @brief Sets the music for Claw Action
void Music::setClawActionMusic()
{
    _music = CurrentMusic::CLAW_ACTION;
}

/// @brief Sets the music to drop prize
void Music::setPrizeDropMusic()
{
    _music = CurrentMusic::PRIZE_DROP;
}

/// @brief Gets the actual music
CurrentMusic Music::getMusicToPlay()
{
    return _music;
}
