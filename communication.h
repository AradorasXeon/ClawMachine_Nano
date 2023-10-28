#include <Wire.h>

#define COMMUNICATION_MOVEMENT 0u

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

class Move
{
    public:
    Move(bool isMaster);

    void sendMsg();
    void setDefaultValues();
    void setLeft();
    void setRight();
    void setUp();
    void setDown();
    void setClawDown();
    void setClawUp();

    void readMsg(int byteCount);

    X_Direction getX();
    Y_Direction getY();
    Claw_Direction getClaw();

    static Move* instance; //for wire library hack

    static void onReceiveCallBack(int byteCount);

    private:
    X_Direction _x = X_Direction::IDLE_X;
    Y_Direction _y = Y_Direction::IDLE_Y;
    Claw_Direction _claw = Claw_Direction::IDLE_CLAW;
};

    ///@brief This is to hack the Wire.onReceive function, should be used ike: Wire.onReceive(Move::onReceiveCallBack); 
    void Move::onReceiveCallBack(int byteCount)
    {
        if (instance != nullptr)
        {
            instance->readMsg(byteCount);
        }
    }


    /// @brief ctor for the Move com interface
    /// @param isMaster true if this is the i2c master
    Move::Move(bool isMaster)
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
    void Move::sendMsg()
    {
        Wire.beginTransmission(I2C_MOTOR_CTRL_ADDRESS);
        Wire.write((uint8_t)_x);
        Wire.write((uint8_t)_y);
        Wire.write((uint8_t)_claw);
        Wire.endTransmission(true);
        delay(2);
    }

    void Move::readMsg(int byteCount)
    {
        _x = (X_Direction)Wire.read();
        _y = (Y_Direction)Wire.read();
        _claw = (Claw_Direction)Wire.read();
    }
    
    void Move::setDefaultValues()
    {
        _x = X_Direction::IDLE_X;
        _y = Y_Direction::IDLE_Y;
        _claw = Claw_Direction::IDLE_CLAW;
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