// ------------------------------------------------------------------------------- //
// Advanced Kalman Filtering and Sensor Fusion Course - Linear Kalman Filter

#include "kalmanfilter.h"
#include "utils.h"

// -------------------------------------------------- //
// YOU CAN USE AND MODIFY THESE CONSTANTS HERE
constexpr bool INIT_ON_FIRST_PREDICTION = false;
constexpr double INIT_POS_STD = 10;
constexpr double INIT_VEL_STD = 10;
constexpr double ACCEL_STD = 0.3;
constexpr double GPS_POS_STD = 3.0;
// -------------------------------------------------- //

void KalmanFilter::predictionStep(double dt)
{
    if (!isInitialised() && INIT_ON_FIRST_PREDICTION)
    {
        // Implement the State Vector and Covariance Matrix Initialisation in the
        // section below if you want to initialise the filter WITHOUT waiting for
        // the first measurement to occur. Make sure you call the setState() /
        // setCovariance() functions once you have generated the initial conditions.
        // Hint: Assume the state vector has the form [X,Y,VX,VY].
        // Hint: You can use the constants: INIT_POS_STD, INIT_VEL_STD
        // ----------------------------------------------------------------------- //
        // ENTER YOUR CODE HERE
        VectorXd state = Vector4d::Zero();
        MatrixXd cov = Matrix4d::Zero();

        // Assume the initial position is (X,Y) = (0,0) m
        // Assume the initial velocity is 5 m/s at 45 degrees (VX,VY) = (5*cos(45deg),5*sin(45deg)) m/s
        state << 0, 0, 5.0*cos(M_PI/4), 5.0*sin(M_PI/4);
        cov(0, 0) = INIT_POS_STD * INIT_POS_STD;
        cov(1, 1) = INIT_POS_STD * INIT_POS_STD;
        cov(2, 2) = INIT_VEL_STD * INIT_VEL_STD;
        cov(3, 3) = INIT_VEL_STD * INIT_VEL_STD;

        setState(state);
        setCovariance(cov);
        // ----------------------------------------------------------------------- //
    }

    if (isInitialised())
    {
        VectorXd state = getState();
        MatrixXd cov = getCovariance();

        // Implement The Kalman Filter Prediction Step for the system in the  
        // section below.
        // Hint: You can use the constants: ACCEL_STD
        // ----------------------------------------------------------------------- //
        // ENTER YOUR CODE HERE
        
        const MatrixXd F = (Matrix4d() << 1, 0, dt, 0, 0, 1 ,0, dt, 0, 0, 1, 0, 0, 0, 0, 1).finished();
        const MatrixXd L = (Eigen::MatrixXd(4, 2) << 0.5*dt*dt, 0, 0, 0.5*dt*dt, dt, 0, 0, dt).finished();
        const Eigen::DiagonalMatrix<double, 2> Q(ACCEL_STD * ACCEL_STD, ACCEL_STD * ACCEL_STD);

        state = F * state;
        cov = F*cov*F.transpose() + L*Q*L.transpose();

        // ----------------------------------------------------------------------- //

        setState(state);
        setCovariance(cov);
    }
}

void KalmanFilter::handleGPSMeasurement(GPSMeasurement meas)
{
    if(isInitialised())
    {
        VectorXd state = getState();
        MatrixXd cov = getCovariance();

        // Implement The Kalman Filter Update Step for the GPS Measurements in the 
        // section below.
        // Hint: Assume that the GPS sensor has a 3m (1 sigma) position uncertainty.
        // Hint: You can use the constants: GPS_POS_STD
        // ----------------------------------------------------------------------- //
        // ENTER YOUR CODE HERE 
        const MatrixXd H = (MatrixXd(2, 4) << 1, 0, 0, 0, 0, 1, 0, 0).finished();
        const MatrixXd R = (MatrixXd(2, 2) << GPS_POS_STD * GPS_POS_STD, 0, 0, GPS_POS_STD * GPS_POS_STD).finished();

        const Vector2d z = (Vector2d() << meas.x, meas.y).finished();
        const VectorXd y = z - H * state;
        const MatrixXd S = H * cov * H.transpose() + R;
        const MatrixXd K = cov * H.transpose() * S.inverse();

        state = state + K * y;
        cov = (MatrixXd::Identity(4, 4) - K * H) * cov;

        // ----------------------------------------------------------------------- //

        setState(state);
        setCovariance(cov);
    }
    else
    {
        // Implement the State Vector and Covariance Matrix Initialisation in the
        // section below. Make sure you call the setState/setCovariance functions
        // once you have generated the initial conditions.
        // Hint: Assume the state vector has the form [X,Y,VX,VY].
        // Hint: You can use the constants: GPS_POS_STD, INIT_VEL_STD
        // ----------------------------------------------------------------------- //
        // ENTER YOUR CODE HERE
            VectorXd state = Vector4d::Zero();
            MatrixXd cov = Matrix4d::Zero();

            state << meas.x, meas.y, 0, 0;
            cov(0, 0) = GPS_POS_STD * GPS_POS_STD;
            cov(1, 1) = GPS_POS_STD * GPS_POS_STD;
            cov(2, 2) = INIT_VEL_STD * INIT_VEL_STD;
            cov(3, 3) = INIT_VEL_STD * INIT_VEL_STD;

            setState(state);
            setCovariance(cov);
        // ----------------------------------------------------------------------- //
    }        
}

Matrix2d KalmanFilter::getVehicleStatePositionCovariance()
{
    Matrix2d pos_cov = Matrix2d::Zero();
    MatrixXd cov = getCovariance();
    if (isInitialised() && cov.size() != 0){pos_cov << cov(0,0), cov(0,1), cov(1,0), cov(1,1);}
    return pos_cov;
}

VehicleState KalmanFilter::getVehicleState()
{
    if (isInitialised())
    {
        VectorXd state = getState(); // STATE VECTOR [X,Y,VX,VY]
        double psi = std::atan2(state[3],state[2]);
        double V = std::sqrt(state[2]*state[2] + state[3]*state[3]);
        return VehicleState(state[0],state[1],psi,V);
    }
    return VehicleState();
}

void KalmanFilter::predictionStep(GyroMeasurement gyro, double dt){predictionStep(dt);}
void KalmanFilter::handleLidarMeasurements(const std::vector<LidarMeasurement>& dataset, const BeaconMap& map){}
void KalmanFilter::handleLidarMeasurement(LidarMeasurement meas, const BeaconMap& map){}

