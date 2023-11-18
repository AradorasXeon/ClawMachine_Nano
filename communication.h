#include <Wire.h>
#include "millisTimer.h"

#define COMMUNICATION_MOVEMENT 0u
#define COMMUNICATION_CALIBRATION 1u

#define I2C_MOTOR_CTRL_ADDRESS 0xF2

enum class X_Direction : uint8_t
{
    LEFT = 0,
    IDLE_X = 127,
    RIGHT = 255
};

enum class Y_Direction : uint8_t
{
    DOWN = 0,
    IDLE_Y = 127,
    UP = 255
};

enum class Claw_Direction : uint8_t
{
    CLAW_DOWN = 0,
    IDLE_CLAW = 127,
    CLAW_UP = 255
};

enum Claw_Calibration : uint8_t //last time it only built if this was after the operators, now only this .... -.-
{
    CLAW_CALIB_IDLE_STATE = 0,
    CLAW_CALIB_INIT = 1,
    CLAW_CALIB_TOP_STATE_IN_PROGRESS = 6,
    CLAW_CALIB_DOWN_STATE_IN_PROGRESS = 14,
    CLAW_CALIB_BAD = 32,
    CLAW_CALIB_TOP_DONE = 64,
    CLAW_CALIB_DOWN_DONE = 128
};

Claw_Calibration operator|(Claw_Calibration left, Claw_Calibration right)
{
    return static_cast<Claw_Calibration>(static_cast<uint8_t>(left) | static_cast<uint8_t>(right));
}

Claw_Calibration operator&(Claw_Calibration left, Claw_Calibration right)
{
    return static_cast<Claw_Calibration>(static_cast<uint8_t>(left) & static_cast<uint8_t>(right));
}

Claw_Calibration operator~(Claw_Calibration input)
{
    return static_cast<Claw_Calibration>(~static_cast<uint8_t>(input));
}

enum class Main_Button : uint8_t
{
    NOT_PUSHED = 0,
    PUSHED = 255
};


struct MovementDataPack
{
    Claw_Calibration calibState;
    int zDirectionMaxStepCountRange;
};


class Move
{
    public:
    Move(bool isMaster);

    void sendMsg(uint8_t msgType);
    void readFromSlave();
    void setDefaultValues();

    void setLeft();
    void setRight();
    void setUp();
    void setDown();
    void setClawDown();
    void setClawUp();
    void setButtonPushed();
    void unsetButtonPushed();

    void initCalibration();
    void startTopCalib();
    void topCalibDone();
    void startDownCalib();
    void downCalibDone();
    void finishCalibration();
    void setBadCalibState();
    void unsetBadCalibState();


    void refreshMovementState(int currentZstepState);


    X_Direction getX();
    Y_Direction getY();
    Claw_Direction getClaw();
    Claw_Calibration getClawCalibState();
    Main_Button getButtonState();
    MovementDataPack getMovementState();

    static Move* instance; //for wire library hack

    static void onReceiveCallBack(int byteCount);

    private:
    void readMsg(int byteCount);

    X_Direction _x = X_Direction::IDLE_X;
    Y_Direction _y = Y_Direction::IDLE_Y;
    Claw_Direction _claw = Claw_Direction::IDLE_CLAW;
    Claw_Calibration _clawCalibState = Claw_Calibration::CLAW_CALIB_IDLE_STATE;
    Main_Button _mainButton = Main_Button::NOT_PUSHED;
    MovementDataPack _movementMessage;
    MillisTimer timer; //cleaning up remnant delay functions

};

    ///@brief This is to hack the Wire.onReceive function, should be used ike: Wire.onReceive(Move::onReceiveCallBack); 
    void Move::onReceiveCallBack(int byteCount)
    {
        if (instance != nullptr)
        {
            instance->readMsg(byteCount);
        }
    }


    ///@returns the calibration state struct of the claw, used for i2c read from motor ctl
    MovementDataPack Move::getMovementState()
    {
        return _movementMessage;
    }

    /// @brief refreshes the claw calibration state
    /// @param currentZstepState give here the zDirectionMaxStepCountRange from the UNO side
    void Move::refreshMovementState(int currentZstepState)
    {
        _movementMessage.calibState = getClawCalibState();
        _movementMessage.zDirectionMaxStepCountRange = currentZstepState;
    }

    /// @brief ctor for the Move com interface
    /// @param isMaster true if this is the i2c master(nano)
    Move::Move(bool isMaster) : timer(2)
    {
        setDefaultValues();
        if(isMaster)
        {
            Wire.begin();
        }
        else
        {
            Wire.begin(I2C_MOTOR_CTRL_ADDRESS);
        }
    }

    /// @brief sends the msg on i2c
    /// @param msgType defines the type of message to be sent
    void Move::sendMsg(uint8_t msgType)
    {
        Wire.beginTransmission(I2C_MOTOR_CTRL_ADDRESS);
        Wire.write(msgType);
        switch (msgType)
        {
        case COMMUNICATION_MOVEMENT:
            Wire.write((uint8_t)_x);
            Wire.write((uint8_t)_y);
            Wire.write((uint8_t)_claw);
            break;
        case COMMUNICATION_CALIBRATION:
            Wire.write((uint8_t)_clawCalibState);
            break;
        
        default:
            break;
        }
        
        Wire.endTransmission(true);
        timer.doDelay();
        #ifdef DEBUG
        Serial.println("MOVE -- Done Writing to SLAVE");
        #endif // DEBUG
    }

    /// @brief reads the msg on i2c from UNO to NANO about movement
    void Move::readFromSlave()
    {
        Wire.requestFrom(I2C_MOTOR_CTRL_ADDRESS, sizeof(MovementDataPack), true);
        Wire.readBytes((byte*) &_movementMessage, sizeof(MovementDataPack));
        timer.doDelay();
        #ifdef DEBUG
        Serial.println("MOVE / DATA_PACK -- Done reading from SLAVE");
        #endif // DEBUG
    }

    void Move::readMsg(int byteCount)
    {
        uint8_t firstByte = Wire.read();
        switch (firstByte)
        {
        case COMMUNICATION_MOVEMENT:
            _x = (X_Direction)Wire.read();
            _y = (Y_Direction)Wire.read();
            _claw = (Claw_Direction)Wire.read();
            break;
        case COMMUNICATION_CALIBRATION:
            _clawCalibState = (Claw_Calibration)Wire.read();
            break;
        
        default:
            break;
        }
        #ifdef DEBUG
        Serial.println("MOVE / direction -- Done reading from MASTER");
        #endif // DEBUG
    }
    
    void Move::setDefaultValues()
    {
        _x = X_Direction::IDLE_X;
        _y = Y_Direction::IDLE_Y;
        _claw = Claw_Direction::IDLE_CLAW;
        _mainButton = Main_Button::NOT_PUSHED;
    }

    X_Direction Move::getX()
    {
        return _x;
    }

    Y_Direction Move::getY()
    {
        return _y;
    }

    Claw_Direction Move::getClaw()
    {
        return _claw;
    }

    Claw_Calibration Move::getClawCalibState()
    {
        return _clawCalibState;
    }

    Main_Button Move::getButtonState()
    {
        return _mainButton;
    }

    void Move::setLeft()
    {
        _x = X_Direction::LEFT;
    }

    void Move::setRight()
    {
        _x = X_Direction::RIGHT;
    }

    void Move::setUp()
    {
        _y = Y_Direction::UP;
    }

    void Move::setDown()
    {
        _y = Y_Direction::DOWN;
    }

    void Move::setClawDown()
    {
        _claw = Claw_Direction::CLAW_DOWN;
    }

    void Move::setClawUp()
    {
        _claw = Claw_Direction::CLAW_UP;
    }

    void Move::setButtonPushed()
    {
        _mainButton = Main_Button::PUSHED;
    }

    void Move::unsetButtonPushed()
    {
        _mainButton = Main_Button::NOT_PUSHED;
    }

    ///@brief 
    void Move::initCalibration()
    {
        _clawCalibState = Claw_Calibration::CLAW_CALIB_INIT;
        //startTopCalib();
    }

    void Move::startTopCalib()
    {
        _clawCalibState = static_cast<Claw_Calibration>(~Claw_Calibration::CLAW_CALIB_TOP_DONE & _clawCalibState); //making sure to remove top calib done state
        _clawCalibState = static_cast<Claw_Calibration>(_clawCalibState | Claw_Calibration::CLAW_CALIB_TOP_STATE_IN_PROGRESS); //short hand gave error for some reason ?!
    }

    void Move::topCalibDone()
    {
        _clawCalibState = static_cast<Claw_Calibration>(~Claw_Calibration::CLAW_CALIB_TOP_STATE_IN_PROGRESS & _clawCalibState); //since we are done
        _clawCalibState = static_cast<Claw_Calibration>(_clawCalibState | Claw_Calibration::CLAW_CALIB_TOP_DONE);
    }

    void Move::startDownCalib()
    {
        _clawCalibState = static_cast<Claw_Calibration>(~Claw_Calibration::CLAW_CALIB_DOWN_DONE & _clawCalibState);
        _clawCalibState = static_cast<Claw_Calibration>(_clawCalibState | Claw_Calibration::CLAW_CALIB_DOWN_STATE_IN_PROGRESS);
    }

    void Move::downCalibDone()
    {
        _clawCalibState = static_cast<Claw_Calibration>(~Claw_Calibration::CLAW_CALIB_DOWN_STATE_IN_PROGRESS & _clawCalibState); //since we are done
        _clawCalibState = static_cast<Claw_Calibration>(_clawCalibState | Claw_Calibration::CLAW_CALIB_DOWN_DONE);
    }

    void Move::finishCalibration()
    {
        _clawCalibState = static_cast<Claw_Calibration>(~Claw_Calibration::CLAW_CALIB_INIT & _clawCalibState); //clears init state
        //down and top done should already be set here
    }

    void Move::setBadCalibState()
    {
        _clawCalibState = static_cast<Claw_Calibration>(Claw_Calibration::CLAW_CALIB_BAD & _clawCalibState);
    }

    void Move::unsetBadCalibState()
    {
        _clawCalibState = static_cast<Claw_Calibration>(~Claw_Calibration::CLAW_CALIB_BAD & _clawCalibState);
    }
