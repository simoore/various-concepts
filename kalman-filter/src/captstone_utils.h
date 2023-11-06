#pragma once

#include <cstdint>
#include <iostream>
#include <optional>

#include <Eigen/Dense>

#include "utils.h"

/// Adds or substracts multiples of 2*pi from the heading state to keep the angle within [-pi,pi].
///
/// @param state 
///     The state vector [X, Y, psi, V].
/// @return 
///     The state with psi wrapped to within [-pi, pi].
Eigen::VectorXd normaliseState(Eigen::VectorXd state) {
    state(2) = wrapAngle(state(2));
    return state;
}

/// Each lidar measurement consists of a range and angle component. This function will wrap the angle of the 
/// measurement.
Eigen::VectorXd normaliseLidarMeasurement(Eigen::VectorXd meas) {
    meas(1) = wrapAngle(meas(1));
    return meas;
}

/// Generates the sigma points. The sigma points include the current state, and points where each component of the
/// state is perturbed by 
/// @param state 
///     The current estimated state.
/// @param cov 
/// @return 
std::vector<Eigen::VectorXd> generateSigmaPoints(Eigen::VectorXd state, Eigen::MatrixXd cov) {

    std::vector<Eigen::VectorXd> sigmaPoints;

    sigmaPoints.push_back(state);

    int n = state.size();
    double kappa = 3.0 - n;
    Eigen::MatrixXd covSqrt = cov.llt().matrixL();
    Eigen::MatrixXd delta = std::sqrt(n + kappa) * covSqrt;
    
    for (int i = 0; i < n; i++) {
        sigmaPoints.push_back(state + delta.col(i));
        sigmaPoints.push_back(state - delta.col(i));
    }

    return sigmaPoints;
}

std::vector<double> generateSigmaWeights(size_t numStates) {

    std::vector<double> weights;
    double n = static_cast<double>(numStates);

    double kappa = 3.0 - n;
    weights.push_back(kappa / (n + kappa));
    double val = 1 / (2*(n + kappa));

    for (size_t i = 1; i <= 2*numStates; i++) {
        weights.push_back(val);
    }

    return weights;
}

/// Computes zhat for each sigma point. zhat is the estimated lidar measurement given a state of the model. The lidar 
/// measurement contains the distance from the vehicle to the target, and the angle of the landmark relative to the 
/// heading of the the vehicle. The zhat's for each sigma point are combined to estimate a zhat that is used to 
/// estimate the innovation and its covariance.
///
/// @param augState 
///     augState(0) is the X position of the vehicle.
///     augState(1) is the Y position of the vehicle.
///     augState(2) is the heading of the vehicle.
///     augState(3) is the velocity of the vehicle.
///     augState(4) is the gyroscope bias.
///     augState(5) is the range noise of the lidar sensor.
///     augState(6) is the angle noise of the lidar sensor.
/// @param beaconX 
///     The X-coordinate of the landmark.
/// @param beaconY 
///     The Y-coordinate of the landmark.
/// @param normalizeAngle
///     If not nullopt, this indicates the estimated theta should be within [-pi,pi] range of normalizing angle.
///     Theta is wrapped if the estimated theta is too far away.
/// @return 
///     Returns the estimated [range, angle] of the landmark with respect to the vehicle frame of reference.
Eigen::VectorXd lidarMeasurementModel(Eigen::VectorXd augState, double beaconX, double beaconY, 
        std::optional<double> normalizeAngle) {

    Eigen::Vector2d zhat = Eigen::Vector2d::Zero();

    const double xdiff = beaconX - augState(0);
    const double ydiff = beaconY - augState(1);

    // There is an issue with the atan2 caculation. If we are operating near pi, there is an implicit wrapping in this
    // function which can add large offsets to the measurement when it is averaged to get zhat. How to deal with this?
    // Maybe we have to normalize to the mean state. That is after we transform the points, we check that theta
    // of all states are near each other. 

    zhat(0) = std::sqrt(xdiff*xdiff + ydiff*ydiff) + augState(5);
    zhat(1) = std::atan2(ydiff, xdiff) - augState(2) + augState(6);

    if (normalizeAngle) {
        double diff = zhat(1) - *normalizeAngle;
        if (diff > M_PI) {
            zhat(1) -= 2 * M_PI;
        } else if (diff < -M_PI) {
            zhat(1) += 2 * M_PI;
        }
    }

    return zhat;
}

/// @param augState 
///     augState(0) is the X position of the vehicle.
///     augState(1) is the Y position of the vehicle.
///     augState(2) is the heading of the vehicle.
///     augState(3) is the velocity of the vehicle.
///     augState(4) is the bias of the gyro.
///     augState(5) is the uncertainty of the heading rate.
///     augState(6) is the uncertainty of the acceleration.
///     augState(7) is the uncertainty of the gyro bias rate.
/// @param psi_dot 
///     The gyroscope measurement.
/// @param dt 
///     The time since the last prediction step.
/// @return 
///     An estimated state of the system given the gyroscope measurement and uncertainies in the system.
Eigen::VectorXd vehicleProcessModel(Eigen::VectorXd augState, double psi_dot, double dt) {

    Eigen::VectorXd newState = Eigen::VectorXd::Zero(5);

    newState(0) = augState(0) + dt * augState(3) * std::cos(augState(2));
    newState(1) = augState(1) + dt * augState(3) * std::sin(augState(2));
    newState(2) = augState(2) + dt * (psi_dot - augState(4) + augState(5));
    newState(3) = augState(3) + dt * augState(6);
    newState(4) = augState(4) + dt * augState(7);

    return newState;
}
