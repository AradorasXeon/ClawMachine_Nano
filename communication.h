#include <Wire.h>
#include "millisTimer.hpp"

//#define COMMUNICATION_MOVEMENT 0u
//#define COMMUNICATION_CALIBRATION 1u

#define I2C_MOTOR_CTRL_ADDRESS 0xF2

enum class Claw_Controll_State : uint8_t
{
    CLAW_CONTROLL_STATE_IDLE    = 0b00000000,
    CLAW_CONTROLL_STATE_BUTTON  = 0b10000000,
    CLAW_CONTROLL_STATE_LEFT    = 0b00000001,
    CLAW_CONTROLL_STATE_RIGHT   = 0b00000010,
    CLAW_CONTROLL_STATE_UP      = 0b00000100,
    CLAW_CONTROLL_STATE_DOWN    = 0b00001000,
    CLAW_CONTROLL_STATE_ERROR   = 0b00010000
};

Claw_Controll_State operator|(Claw_Controll_State left, Claw_Controll_State right)
{
    return static_cast<Claw_Controll_State>(static_cast<uint8_t>(left) | static_cast<uint8_t>(right));
}

Claw_Controll_State operator&(Claw_Controll_State left, Claw_Controll_State right)
{
    return static_cast<Claw_Controll_State>(static_cast<uint8_t>(left) & static_cast<uint8_t>(right));
}

Claw_Controll_State operator~(Claw_Controll_State input)
{
    return static_cast<Claw_Controll_State>(~static_cast<uint8_t>(input));
}


enum class Claw_Calibration : uint8_t
{
    CLAW_CALIB_IDLE_STATE =             0b00000000,
    CLAW_CALIB_INIT =                   0b00000001,
    CLAW_CALIB_TOP_STATE_IN_PROGRESS =  0b00000010,
    CLAW_CALIB_DOWN_STATE_IN_PROGRESS = 0b00000100,
    CLAW_CALIB_BAD =                    0b00100000,
    CLAW_CALIB_TOP_DONE =               0b01000000,
    CLAW_CALIB_DOWN_DONE =              0b10000000
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

struct MovementDataPack
{
    Claw_Calibration calibState;
    int zDirectionMaxStepCountRange;
};

struct MovementControllPack
{
    Claw_Calibration controllCallibState;
    Claw_Controll_State controllMovementState;
};


class Move
{
    public:
    Move(bool isMaster);

    void sendControllMsg();
    void readFromSlave();
    void setDefaultValues();

    void setLeft();
    void setRight();
    void setUp();
    void setDown();
    void setButtonPushed();

    void initCalibration();
    void startTopCalib();
    void topCalibDone();
    void startDownCalib();
    void downCalibDone();
    void finishCalibration();
    void setBadCalibState();
    void unsetBadCalibState();

    void refreshMovementState(int currentZstepState);


    Claw_Controll_State getClawControllState();
    Claw_Calibration getClawCalibState();
    MovementDataPack getMovementStateFromUno();
    MovementControllPack getMovementControllState();

    static Move* instance; //for wire library hack

    static void onReceiveCallBack(int byteCount);
    static bool isClawCalibSet(Claw_Calibration lookInThis, Claw_Calibration lookForThis);
    static bool isClawControllSet(Claw_Controll_State lookInThis, Claw_Controll_State lookForThis);

    private:
    void readMsg(int byteCount);

    Claw_Calibration _clawCalibState = Claw_Calibration::CLAW_CALIB_IDLE_STATE;
    Claw_Controll_State _clawControllState = Claw_Controll_State::CLAW_CONTROLL_STATE_IDLE;
    MovementDataPack _movementMessageFromUno;
    MovementControllPack _movementControllMessage;
    MillisTimer timer; //inited to 2 ms |cleaning up remnant delay functions

};

    ///@brief This is to hack the Wire.onReceive function, should be used ike: Wire.onReceive(Move::onReceiveCallBack); 
    void Move::onReceiveCallBack(int byteCount)
    {
        if (instance != nullptr)
        {
            instance->readMsg(byteCount);
        }
    }

    ///@returns true if lookForThis is set in lookInThis
    bool Move::isClawCalibSet(Claw_Calibration lookInThis, Claw_Calibration lookForThis)
    {
        Claw_Calibration temp = lookInThis & lookForThis;
        if(temp == lookForThis)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    ///@returns true if lookForThis is set in lookInThis
    bool Move::isClawControllSet(Claw_Controll_State lookInThis, Claw_Controll_State lookForThis)
    {
        Claw_Controll_State temp = lookInThis & lookForThis;
        if(temp == lookForThis)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    ///@returns the calibration state struct of the claw, used for i2c read from motor ctl
    MovementDataPack Move::getMovementStateFromUno()
    {
        return _movementMessageFromUno;
    }

    /// @brief refreshes the claw calibration state
    /// @param currentZstepState give here the zDirectionMaxStepCountRange from the UNO side
    void Move::refreshMovementState(int currentZstepState)
    {
        _movementMessageFromUno.zDirectionMaxStepCountRange = currentZstepState;
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

    /// @brief sends the ctrl msg on i2c
    void Move::sendControllMsg()
    {
        _movementControllMessage.controllCallibState = _clawCalibState;
        _movementControllMessage.controllMovementState = _clawControllState;
        /*
        #ifdef DEBUG
            Serial.println("MOVE -- START Writing to SLAVE");
            Serial.println(static_cast<uint8_t>(_movementControllMessage.controllCallibState), BIN);
            Serial.println(static_cast<uint8_t>(_movementControllMessage.controllMovementState), BIN);
        #endif // DEBUG
        */
        Wire.beginTransmission(I2C_MOTOR_CTRL_ADDRESS);
        Wire.write(static_cast<uint8_t>(_movementControllMessage.controllCallibState));
        Wire.write(static_cast<uint8_t>(_movementControllMessage.controllMovementState));
        Wire.endTransmission(true);
        timer.doDelay();
        #ifdef DEBUG
            //Serial.println("MOVE -- Done Writing to SLAVE");
        #endif // DEBUG
    }

    /// @brief reads the msg on i2c from UNO to NANO about movement
    void Move::readFromSlave()
    {
        Wire.requestFrom(I2C_MOTOR_CTRL_ADDRESS, sizeof(MovementDataPack), true);
        Wire.readBytes((byte*) &_movementMessageFromUno, sizeof(MovementDataPack));
        timer.doDelay();
        #ifdef DEBUG
            //Serial.println("MOVE / DATA_PACK -- Done reading from SLAVE");
        #endif // DEBUG
    }

    void Move::readMsg(int byteCount)
    {
        _movementControllMessage.controllCallibState = (Claw_Calibration)Wire.read();
        _movementControllMessage.controllMovementState = (Claw_Controll_State)Wire.read();
        _movementMessageFromUno.calibState = _movementControllMessage.controllCallibState; //save it to UNO side, also for historical reasons
        _clawCalibState = _movementMessageFromUno.calibState; //just in case i dont remeber if it is used somewhere
        
        #ifdef DEBUG
        Serial.println("MOVE / direction -- Done reading from MASTER the following:");
        Serial.println(static_cast<uint8_t>(_movementMessageFromUno.calibState), BIN);
        Serial.println(static_cast<uint8_t>(_movementControllMessage.controllMovementState), BIN);
        Serial.println("*****************************************************");
        #endif // DEBUG
    }
    
    void Move::setDefaultValues()
    {
        _clawControllState = Claw_Controll_State::CLAW_CONTROLL_STATE_IDLE;
    }

    MovementControllPack Move::getMovementControllState()
    {
        return _movementControllMessage;
    }

    Claw_Controll_State Move::getClawControllState()
    {
        return _clawControllState;
    }

    Claw_Calibration Move::getClawCalibState()
    {
        #ifdef DEBUG
            //Serial.println(static_cast<uint8_t>(_clawCalibState), BIN);
        #endif // DEBUG
        return _clawCalibState;
    }

    void Move::setLeft()
    {
        _clawControllState = _clawControllState | Claw_Controll_State::CLAW_CONTROLL_STATE_LEFT;
    }

    void Move::setRight()
    {
        _clawControllState = _clawControllState | Claw_Controll_State::CLAW_CONTROLL_STATE_RIGHT;
    }

    void Move::setUp()
    {
        _clawControllState = _clawControllState | Claw_Controll_State::CLAW_CONTROLL_STATE_UP;
    }

    void Move::setDown()
    {
        _clawControllState = _clawControllState | Claw_Controll_State::CLAW_CONTROLL_STATE_DOWN;
    }

    void Move::setButtonPushed()
    {
        _clawControllState = _clawControllState | Claw_Controll_State::CLAW_CONTROLL_STATE_BUTTON;
    }

    ///@brief 
    void Move::initCalibration()
    {
        _clawCalibState = Claw_Calibration::CLAW_CALIB_INIT;
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
