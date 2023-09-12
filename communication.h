class Move
{
    public:
    uint8_t xAxis = 127; // 0 - Left, 255 - Right, any other number - do not move
    uint8_t yAxis = 127; // 0 - "Down", 255 - "Up", any other number - do not move
    bool initiateClaw = false; // true - initiate Claw movment, false - do nothing, after once set to true should be set back to false

    void sendMsg();
    void setDefaultValues();
    void setLeft();
    void setRight();
    void setUp();
    void setDown();
    void setClaw();


    private:
    uint8_t _x = 127;
    uint8_t _y = 127;
    bool claw = false;
};

    void Move::sendMsg()
    {}
    
    void Move::setDefaultValues()
    {}

    void Move::setLeft()
    {}

    void Move::setRight()
    {}

    void Move::setUp()
    {}

    void Move::setDown()
    {}

    void Move::setClaw()
    {}

