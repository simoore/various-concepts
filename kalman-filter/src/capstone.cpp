/// Goal:
/// - To estimate the position, velocity, and orientation of a moving vehicle based on lidar-like measurements
///   to knowm landmarks/features, GPS measurements, and gyroscope measurements under non-ideal simulated conditions.
///
/// Problem
/// - Assume the vehicle travels at a constant speed in the 2DX-Y plane int he direction it is facing. Let the inputs
///   be a turn rate of the vehicle from a biased gyroscope and assume that the acceleration is a random variable.
/// - Assume we get position measurements of the vehicles location (ie. GPS), that can have sensor faults.
/// - Assume we get a number of range and relative bearing measurements of known landmark locations (ie. lidar) with
///   and without matching data assocations.
///
/// Dealing with Non-Zero Initial Conditions
/// - Use the GPS position measurements to initialize the X/Y position states.
/// - Use the GPS position and LIDAR measurements to landmarks(s) (when available) to estimate the heading of the 
///   vehicle.
/// - Make initial estimates of the velocity and gyro bias. (Could assume to be zero?)
/// - Combine the outputs of the above 3 processes to generate the initial condition and then start the full kalman 
///   filter.
///
/// Dealing with Gyroscope Sensor Bias
/// - Add a gyro bias state to the state vector adn vehicle model.
///
/// Dealing with Faulty GPS Measurements
/// - Don't fuse any faulty GPS measurements.
/// - Evaluate each measurement (such as innnovation checking)
/// - Only fuse measurements that pass the test
/// - Use NIS: e = v^TS^{-1}v, e must be smaller than a threshold
///
/// Dealing with LIDAR Data Association
/// - Use estimated position and maximum LIDAR range to find all possible landmarks that could be in view.
/// - Use current estimated position and heading to find taht best match up with possible landmarks to find data
///   assocaitions.
/// - If you don't know the heading estimate (ie. during initialization), then you might need to find the best heading 
///   that fits the data.

#include "kalmanfilter.h"
#include "utils.h"

///////////////////////////////////////////////////////////////////////////////
// UNCERTAINTIES
///////////////////////////////////////////////////////////////////////////////

constexpr double ACCEL_STD = 1.0;
constexpr double GYRO_STD = 0.01/180.0 * M_PI;
constexpr double INIT_VEL_STD = 10.0;
constexpr double INIT_PSI_STD = 45.0/180.0 * M_PI;
constexpr double GPS_POS_STD = 3.0;
constexpr double LIDAR_RANGE_STD = 3.0;
constexpr double LIDAR_THETA_STD = 0.02;

///////////////////////////////////////////////////////////////////////////////
// UTILITY FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

VectorXd normaliseState(VectorXd state) {
    state(2) = wrapAngle(state(2));
    return state;
}


/// Each lidar measurement consists of a range and angle component. This function will wrap the angle of the 
/// measurement.
VectorXd normaliseLidarMeasurement(VectorXd meas) {
    meas(1) = wrapAngle(meas(1));
    return meas;
}


std::vector<VectorXd> generateSigmaPoints(VectorXd state, MatrixXd cov) {

    std::vector<VectorXd> sigmaPoints;

    sigmaPoints.push_back(state);

    int n = state.size();
    double k = 3.0 - n;
    MatrixXd covSqrt = cov.llt().matrixL();
    MatrixXd delta = std::sqrt(n + k) * covSqrt;
    
    for (int i = 0; i < n; i++) {
        sigmaPoints.push_back(state + delta.col(i));
        sigmaPoints.push_back(state - delta.col(i));
    }

    return sigmaPoints;
}

std::vector<double> generateSigmaWeights(size_t numStates) {

    std::vector<double> weights;

    double k = 3.0 - numStates;
    weights.push_back(k / (numStates + k));
    double val = 1 / (2*(numStates + k));

    for (size_t i = 1; i <= 2*numStates; i++) {
        weights.push_back(val);
    }

    return weights;
}

VectorXd lidarMeasurementModel(VectorXd aug_state, double beaconX, double beaconY) {

    Vector2d z_hat = Vector2d::Zero();

    const double xdiff = beaconX - aug_state(0);
    const double ydiff = beaconY - aug_state(1);
    z_hat(0) = std::sqrt(xdiff*xdiff + ydiff*ydiff) + aug_state(4);
    z_hat(1) = std::atan2(ydiff, xdiff) - aug_state(2) + aug_state(5);

    return z_hat;
}

VectorXd vehicleProcessModel(VectorXd augState, double psi_dot, double dt) {

    VectorXd newState = VectorXd::Zero(4);

    newState(0) = augState(0) + dt * augState(3) * std::cos(augState(2));
    newState(1) = augState(1) + dt * augState(3) * std::sin(augState(2));
    newState(2) = augState(2) + dt * (psi_dot + augState(4));
    newState(3) = augState(3) + dt * augState(5);

    return newState;
}

///////////////////////////////////////////////////////////////////////////////
// KALMAN FILTER CLASS FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

void KalmanFilter::handleLidarMeasurement(LidarMeasurement meas, const BeaconMap& map)
{
    if (isInitialised())
    {
        VectorXd state = getState();
        MatrixXd cov = getCovariance();

        // Implement The Kalman Filter Update Step for the Lidar Measurements in the section below
        // HINT: Use the normaliseState() and normaliseLidarMeasurement() functions to always keep angle values within 
        //  correct range
        // HINT: Do not normalise during sigma point calculation!
        // HINT: You can use the constants: LIDAR_RANGE_STD, LIDAR_THETA_STD
        // HINT: The mapped-matched beacon position can be accessed by the variables map_beacon.x and map_beacon.y
 
        // Match Beacon with built in Data Association Id.
        BeaconData map_beacon = map.getBeaconWithId(meas.id); 

        if (meas.id != -1 && map_beacon.id != -1) // Check that we have a valid beacon match
        {
            const size_t nz = 2;
            const size_t nx = state.size();
            const size_t na = nx + 2;

            // Augmentation of state and covariance.
            VectorXd augState = VectorXd::Zero(na);
            augState.head(nx) = state;

            MatrixXd augCov = MatrixXd::Zero(na, na);
            augCov.topLeftCorner(nx, nx) = cov;
            augCov(nx, nx) = LIDAR_RANGE_STD * LIDAR_RANGE_STD;
            augCov(nx + 1, nx + 1) = LIDAR_THETA_STD * LIDAR_THETA_STD;

            // Generate sigma points and weights.
            auto sigmaPoints = generateSigmaPoints(augState, augCov);
            auto weights = generateSigmaWeights(na);

            // Transform sigma points using lidar model.
            std::vector<Vector2d> transformedPoints;
            std::transform(sigmaPoints.begin(), sigmaPoints.end(), std::back_inserter(transformedPoints), 
                [=](const auto &p) {
                    return lidarMeasurementModel(p, map_beacon.x, map_beacon.y);
                }
            );

            // Calculate the mean of the transformed sigma points
            Vector2d zhat = Vector2d::Zero();
            for (size_t i = 0; i < transformedPoints.size(); i++) {
                zhat += weights[i] * transformedPoints[i];
            }
            
            // Calculate the innovation.
            Vector2d z = Vector2d::Zero();
            z << meas.range, meas.theta;
            Vector2d innovation = normaliseLidarMeasurement(z - zhat);

            // Calculate the covariance of the transformed signal.
            Matrix2d innovationcov = Matrix2d::Zero();
            for (size_t i = 0; i < transformedPoints.size(); i++) {
                Vector2d err = normaliseLidarMeasurement(transformedPoints[i] - zhat);
                innovationcov += weights[i] * err * err.transpose();
            }

            // Calculate the cross covariance
            MatrixXd crosscov = MatrixXd::Zero(nx, nz);
            for (size_t i = 0; i < transformedPoints.size(); i++) {
                VectorXd errz = normaliseLidarMeasurement(transformedPoints[i] - zhat);
                VectorXd errx = normaliseState(sigmaPoints[i].head(nx) - state);
                crosscov += weights[i] * errx * errz.transpose();
            }

            // UKF update step equations.
            MatrixXd kalmanGain = crosscov * innovationcov.inverse();
            state += kalmanGain * innovation;
            cov -= kalmanGain * innovationcov * kalmanGain.transpose();
        }

        setState(state);
        setCovariance(cov);
    }
}

void KalmanFilter::predictionStep(GyroMeasurement gyro, double dt)
{
    if (isInitialised())
    {
        VectorXd state = getState();
        MatrixXd cov = getCovariance();

        // Implement The Kalman Filter Prediction Step for the system in the section below.
        // HINT: Assume the state vector has the form [PX, PY, PSI, V].
        // HINT: Use the Gyroscope measurement as an input into the prediction step.
        // HINT: You can use the constants: ACCEL_STD, GYRO_STD
        // HINT: Use the normaliseState() function to always keep angle values within correct range.
        // HINT: Do NOT normalise during sigma point calculation!

        int nx = state.size();
        int nw = 2;
        int na = nx + nw;

        VectorXd augState = VectorXd::Zero(na);
        augState.head(nx) = state;

        MatrixXd covAug = MatrixXd::Zero(na, na);
        covAug.topLeftCorner(nx, nx) = cov;
        covAug(nx, nx) =  GYRO_STD * GYRO_STD;
        covAug(nx + 1, nx + 1) =  ACCEL_STD * ACCEL_STD;

        auto sigmaPoints = generateSigmaPoints(augState, covAug);
        auto weights = generateSigmaWeights(na);
        std::vector<VectorXd> transformedPoints;
        std::transform(sigmaPoints.begin(), sigmaPoints.end(), std::back_inserter(transformedPoints), 
            [=](const auto &p) {
                return vehicleProcessModel(p, gyro.psi_dot, dt);
            }
        );

        state = VectorXd::Zero(nx);
        for (size_t i = 0; i < transformedPoints.size(); i++) {
            state = state + weights[i] * transformedPoints[i];
        }

        cov = MatrixXd::Zero(nx, nx);
        for (size_t i = 0; i < transformedPoints.size(); i++) {
            VectorXd err = normaliseState(transformedPoints[i] - state);
            cov = cov + weights[i] * err * err.transpose();
        }

        setState(state);
        setCovariance(cov);
    } 
}

void KalmanFilter::handleGPSMeasurement(GPSMeasurement meas)
{
    // All this code is the same as the LKF as the measurement model is linear
    // so the UKF update state would just produce the same result.
    if(isInitialised())
    {
        VectorXd state = getState();
        MatrixXd cov = getCovariance();

        VectorXd z = Vector2d::Zero();
        MatrixXd H = MatrixXd(2,4);
        MatrixXd R = Matrix2d::Zero();

        z << meas.x,meas.y;
        H << 1,0,0,0,0,1,0,0;
        R(0,0) = GPS_POS_STD*GPS_POS_STD;
        R(1,1) = GPS_POS_STD*GPS_POS_STD;

        VectorXd z_hat = H * state;
        VectorXd y = z - z_hat;
        MatrixXd S = H * cov * H.transpose() + R;
        MatrixXd K = cov*H.transpose()*S.inverse();

        state = state + K*y;
        cov = (MatrixXd::Identity(4,4) - K*H) * cov;

        setState(state);
        setCovariance(cov);
    }
    else
    {
        // You may modify this initialisation routine if you can think of a more
        // robust and accuracy way of initialising the filter.
        // ----------------------------------------------------------------------- //
        // YOU ARE FREE TO MODIFY THE FOLLOWING CODE HERE

        VectorXd state = Vector4d::Zero();
        MatrixXd cov = Matrix4d::Zero();

        state(0) = meas.x;
        state(1) = meas.y;
        cov(0, 0) = GPS_POS_STD*GPS_POS_STD;
        cov(1, 1) = GPS_POS_STD*GPS_POS_STD;
        cov(2, 2) = INIT_PSI_STD*INIT_PSI_STD;
        cov(3, 3) = INIT_VEL_STD*INIT_VEL_STD;

        setState(state);
        setCovariance(cov);

        // ----------------------------------------------------------------------- //
    }             
}

void KalmanFilter::handleLidarMeasurements(const std::vector<LidarMeasurement> &dataset, const BeaconMap &map) {
    // Assume No Correlation between the Measurements and Update Sequentially
    for (const auto &meas : dataset) {
        handleLidarMeasurement(meas, map);
    }
}

Matrix2d KalmanFilter::getVehicleStatePositionCovariance() {
    Matrix2d pos_cov = Matrix2d::Zero();
    MatrixXd cov = getCovariance();
    if (isInitialised() && cov.size() != 0) {
        pos_cov << cov(0,0), cov(0,1), cov(1,0), cov(1,1);
    }
    return pos_cov;
}


VehicleState KalmanFilter::getVehicleState() {
    if (isInitialised()) {
        VectorXd state = getState(); // STATE VECTOR [X,Y,PSI,V,...]
        return VehicleState(state[0], state[1], state[2], state[3]);
    }
    return VehicleState();
}

void KalmanFilter::predictionStep(double dt) {}
