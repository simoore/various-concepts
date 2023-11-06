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
/// - Add a gyro bias state to the state vector and vehicle model.
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

#include "captstone_utils.h"
#include "kalmanfilter.h"
#include "utils.h"

///////////////////////////////////////////////////////////////////////////////
// UNCERTAINTIES
///////////////////////////////////////////////////////////////////////////////

static constexpr double ACCEL_STD = 1.0;
static constexpr double GYRO_STD = 0.01/180.0 * M_PI;
static constexpr double INIT_VEL_STD = 10.0;
static constexpr double INIT_PSI_STD = 45.0/180.0 * M_PI;
static constexpr double GPS_POS_STD = 3.0;
static constexpr double LIDAR_RANGE_STD = 3.0;
static constexpr double LIDAR_THETA_STD = 0.02;
static constexpr double BIAS_STD = 0.005/180.0 * M_PI;
static constexpr size_t NX = 5;
static constexpr size_t NW = 3;
static constexpr size_t NZ_LIDAR = 2;

static VectorXd sInitialState = VectorXd::Zero(NX);

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
            // Augmentation of state and covariance.
            const size_t na = NX + NZ_LIDAR;

            VectorXd augState = VectorXd::Zero(na);
            augState.head(NX) = state;

            MatrixXd augCov = MatrixXd::Zero(na, na);
            augCov.topLeftCorner(NX, NX) = cov;
            augCov(NX, NX) = LIDAR_RANGE_STD * LIDAR_RANGE_STD;
            augCov(NX + 1, NX + 1) = LIDAR_THETA_STD * LIDAR_THETA_STD;

            // Generate sigma points and weights.
            auto sigmaPoints = generateSigmaPoints(augState, augCov);
            auto weights = generateSigmaWeights(na);

            // Transform sigma points using lidar model.
            std::vector<Vector2d> transformedPoints;
            transformedPoints.push_back(lidarMeasurementModel(sigmaPoints[0], map_beacon.x, map_beacon.y, 
                std::nullopt));
            for (size_t i = 1; i < sigmaPoints.size(); i++) {
                transformedPoints.push_back(lidarMeasurementModel(sigmaPoints[i], map_beacon.x, map_beacon.y, 
                    transformedPoints[0](1)));
            }

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
            using CrossCovMatrix = Eigen::Matrix<double, NX, NZ_LIDAR>;
            CrossCovMatrix crosscov = CrossCovMatrix::Zero();
            for (size_t i = 0; i < transformedPoints.size(); i++) {
                Vector2d errz = normaliseLidarMeasurement(transformedPoints[i] - zhat);
                VectorXd errx = normaliseState(sigmaPoints[i].head(NX) - state);
                crosscov += weights[i] * errx * errz.transpose();
            }

            // UKF update step equations.
            // TODO: There is an issue with the lidar measurement, it is giving a positive velocity when in fact the 
            // vehicle is moving backward. I feel the cross-covariance of row(3) and row(4) (zero based) has the wrong
            // sign or should be zero since the lidar model doesn't use them.
            
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
    if (isInitialised()) {
        
        VectorXd state = getState();
        MatrixXd cov = getCovariance();

        // Implement The Kalman Filter Prediction Step for the system in the section below.
        // HINT: Assume the state vector has the form [PX, PY, PSI, V, BIAS].
        // HINT: Use the Gyroscope measurement as an input into the prediction step.
        // HINT: You can use the constants: ACCEL_STD, GYRO_STD, BIAS_STD
        // HINT: Use the normaliseState() function to always keep angle values within correct range.
        // HINT: Do NOT normalise during sigma point calculation!

        size_t na = NX + NW;

        VectorXd augState = VectorXd::Zero(na);
        augState.head(NX) = state;

        MatrixXd covAug = MatrixXd::Zero(na, na);
        covAug.topLeftCorner(NX, NX) = cov;
        covAug(NX + 0, NX + 0) =  GYRO_STD * GYRO_STD;
        covAug(NX + 1, NX + 1) =  ACCEL_STD * ACCEL_STD;
        covAug(NX + 2, NX + 2) =  BIAS_STD * BIAS_STD;

        auto sigmaPoints = generateSigmaPoints(augState, covAug);
        auto weights = generateSigmaWeights(na);

        std::vector<VectorXd> transformedPoints;
        std::transform(sigmaPoints.begin(), sigmaPoints.end(), std::back_inserter(transformedPoints), 
            [=](const auto &p) {
                return vehicleProcessModel(p, gyro.psi_dot, dt);
            }
        );
        
        state = VectorXd::Zero(NX);
        for (size_t i = 0; i < transformedPoints.size(); i++) {
            state = state + weights[i] * transformedPoints[i];
        }

        cov = MatrixXd::Zero(NX, NX);
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
        MatrixXd H = MatrixXd(2, NX);
        MatrixXd R = Matrix2d::Zero();

        z << meas.x, meas.y;
        H << 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0;
        R(0, 0) = GPS_POS_STD * GPS_POS_STD;
        R(1, 1) = GPS_POS_STD * GPS_POS_STD;

        VectorXd z_hat = H * state;
        VectorXd y = z - z_hat;
        MatrixXd S = H * cov * H.transpose() + R;
        MatrixXd K = cov*H.transpose()*S.inverse();

        // Innovation check
        VectorXd NIS = y.transpose()*S.inverse()*y;
        if (NIS(0) < 5.99) {
            state = state + K*y;
            cov = (MatrixXd::Identity(NX, NX) - K*H) * cov;
        } else {
            std::cout << "GPS NIS Failed! " << NIS(0) << std::endl;
        }

        setState(state);
        setCovariance(cov);

    } else {

        // TODO: Determine initial states from measurements.
        MatrixXd cov = MatrixXd::Zero(NX, NX);

        sInitialState(0) = meas.x;
        sInitialState(1) = meas.y;
        sInitialState(2) = -M_PI / 2.0;
        sInitialState(3) = -2.0;
        sInitialState(4) = 0.0;

        cov(0, 0) = GPS_POS_STD*GPS_POS_STD;
        cov(1, 1) = GPS_POS_STD*GPS_POS_STD;
        cov(2, 2) = INIT_PSI_STD*INIT_PSI_STD;
        cov(3, 3) = INIT_VEL_STD*INIT_VEL_STD;
        cov(4, 4) = BIAS_STD*BIAS_STD;

        std::cout << "Initial posX (m): " << sInitialState(0) << std::endl;
        std::cout << "Initial posY (m): " << sInitialState(1) << std::endl;
        std::cout << "Initial angle (deg): " << sInitialState(2) * 180.0 / M_PI << std::endl;
        std::cout << "Initial velocity (m/s): " << sInitialState(3) << std::endl;
        std::cout << "Initial Bias (m/s): " << sInitialState(4) << std::endl;

        setState(sInitialState);
        setCovariance(cov);
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
        pos_cov << cov(0, 0), cov(0, 1), cov(1, 0), cov(1, 1);
    }
    return pos_cov;
}


VehicleState KalmanFilter::getVehicleState() {
    if (isInitialised()) {
        VectorXd state = getState(); // STATE VECTOR [X,Y,PSI,V,BIAS]
        return VehicleState(state[0], state[1], state[2], state[3]);
    }
    return VehicleState();
}

void KalmanFilter::predictionStep(double dt) {}
