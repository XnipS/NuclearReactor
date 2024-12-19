// Output = Kp * error + Ki * /int^t_0 e(tau) d tau + Kd * derror/dt

#include <cmath>
class PID {
public:
    PID() { }
    PID(float Kp, float Ki, float Kd, float endpoint)
    {
        PID(Kp, Ki, Kd, endpoint, INFINITY, -INFINITY);
    }
    PID(float Kp, float Ki, float Kd, float endpoint, float max, float min)
    {
        this->Kp = Kp;
        this->Ki = Ki;
        this->Kd = Kd;
        this->endpoint = endpoint;
        oldError = 0;
        integral = 0;
        maxI = max;
        minI = min;
    }

    float Calculate(float realpoint, float deltaTime)
    {
        float error = endpoint - realpoint;
        // P
        float P = Kp * error;
        // I
        integral += error * deltaTime;
        if (integral > maxI)
            integral = maxI;
        if (integral < minI)
            integral = minI;
        float I = Ki * integral;

        // D
        float derivative = (error - oldError) / deltaTime;
        float D = Kd * derivative;

        oldError = error;

        return P + I + D;
    }
    float Calculate(float realpoint, float deltaTime, float Kp, float Ki, float Kd)
    {
        this->Kp = Kp;
        this->Ki = Ki;
        this->Kd = Kd;
        return Calculate(realpoint, deltaTime);
    }

private:
    float Kp, Ki, Kd;
    float oldError, integral, endpoint;
    float maxI, minI;
};