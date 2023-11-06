#include <algorithm>
#include <iostream>

#include "captstone_utils.h"

static constexpr double ACCEL_STD = 1.0;
static constexpr double GYRO_STD = 0.01/180.0 * M_PI;
static constexpr double INIT_VEL_STD = 10.0;
static constexpr double INIT_PSI_STD = 45.0/180.0 * M_PI;
static constexpr double GPS_POS_STD = 3.0;
static constexpr double LIDAR_RANGE_STD = 3.0;
static constexpr double LIDAR_THETA_STD = 0.02;
static constexpr double BIAS_STD = 0.01/180.0 * M_PI;

struct Measurement {
    double range;
    double theta;
};

struct MapBeacon {
    double x;
    double y;
};

int main() {

    using namespace Eigen;

    Measurement meas { .range = 32.5135, .theta = -0.676827 };
    VectorXd state = VectorXd::Zero(4);
    state << 500.0, 500.0, -2.356194490192345, 5.0;
    MatrixXd cov = MatrixXd::Zero(4, 4);
    cov(0, 0) = GPS_POS_STD * GPS_POS_STD;
    cov(1, 1) = GPS_POS_STD * GPS_POS_STD;
    cov(2, 2) = INIT_PSI_STD * INIT_PSI_STD;
    cov(3, 3) = INIT_VEL_STD * INIT_VEL_STD;
    MapBeacon map_beacon { .x = 467.695, .y = 496.461 };

    ///////////////////////////////////////////////////////////////////////////

    const size_t nz = 2;
    const size_t nx = state.size();
    const size_t na = nx + nz;

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

    // Assert sigmaPoints
    // With a diagonal covariance, we expect the offset on each of the state to be sqrt(3) (1.73205080) times the 
    // standard deviation given to each. The psi offset is 77 degs.
    double multiplier = 1.73205080;
    std::cout << "--- Test Sigma Points ---" << std::endl; 
    std::cout << "expected: " << -multiplier * GPS_POS_STD << ", actual: " << augState(0) - sigmaPoints[1](0) << std::endl;
    std::cout << "expected: " << multiplier * GPS_POS_STD << ", actual: " << augState(0) - sigmaPoints[2](0) << std::endl;
    std::cout << "expected: " << -multiplier * GPS_POS_STD << ", actual: " << augState(1) - sigmaPoints[3](1) << std::endl;
    std::cout << "expected: " << multiplier * GPS_POS_STD << ", actual: " << augState(1) - sigmaPoints[4](1) << std::endl;
    std::cout << "expected: " << -multiplier * INIT_PSI_STD << ", actual: " << augState(2) - sigmaPoints[5](2) << std::endl;
    std::cout << "expected: " << multiplier * INIT_PSI_STD << ", actual: " << augState(2) - sigmaPoints[6](2) << std::endl;
    std::cout << "expected: " << -multiplier * INIT_VEL_STD << ", actual: " << augState(3) - sigmaPoints[7](3) << std::endl;
    std::cout << "expected: " << multiplier * INIT_VEL_STD << ", actual: " << augState(3) - sigmaPoints[8](3) << std::endl;
    std::cout << "expected: " << -multiplier * LIDAR_RANGE_STD << ", actual: " << augState(4) - sigmaPoints[9](4) << std::endl;
    std::cout << "expected: " << multiplier * LIDAR_RANGE_STD << ", actual: " << augState(4) - sigmaPoints[10](4) << std::endl;
    std::cout << "expected: " << -multiplier * LIDAR_THETA_STD << ", actual: " << augState(5) - sigmaPoints[11](5) << std::endl;
    std::cout << "expected: " << multiplier * LIDAR_THETA_STD << ", actual: " << augState(5) - sigmaPoints[12](5) << std::endl;

    // Assert weights
    std::cout << "--- Test Weights ---" << std::endl; 
    std::cout << "expected: " << -1.0 << ", actual: " << weights[0] << std::endl;
    for (size_t i = 1; i < weights.size(); i++) {
        std::cout << "expected: " << 0.16668 << ", actual: " << weights[i] << std::endl;
    }

    std::vector<Vector2d> transformedPoints;
    transformedPoints.push_back(lidarMeasurementModel(sigmaPoints[0], map_beacon.x, map_beacon.y, std::nullopt));
    for (size_t i = 1; i < sigmaPoints.size(); i++) {
        transformedPoints.push_back(lidarMeasurementModel(sigmaPoints[i], map_beacon.x, map_beacon.y, transformedPoints[0](1)));
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

    std::cout << "Handle Lidar" << std::endl;
    std::cout << "State [X, Y, psi, V] = " << state(0) << ", " << state(1) << ", " << state(2) * 180.0 / M_PI << ", " << state(3) << std::endl;

    return 0;
}