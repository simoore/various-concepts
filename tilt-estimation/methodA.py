import numpy as np


class MethodA:
    """
    This method is based on a linear kalman filter approach published on the website,

    https://nitinjsanket.github.io/tutorials/attitudeest/kf
    
    It considers the gyroscope to be an input to the system for predication, and (roll, pitch) are measured by the 
    accelerometer as the output. We can use a linear kalman filter because we explicity compute roll and pitch
    from acceleration measurements prior to using the filter.

    The state of the filter is,
    x = [roll, pitch, yaw, gyro_bias_x, gyro_bias_y, gyro_bias_z]

    The input of the filter is,
    u = [roll_rate, pitch_rate, yaw_rate]

    The output of the filter is,
    y = [roll, pitch]

    Angles are in radians. We rotate the gyroscope measurements to attain the roll/pitch/yaw rate inputs, and we 
    transform the accelerometer to roll & pitch for the update step.
    """

    def __init__(self, accel_std: float, gyro_std: float, init_angle_std: float, init_bias_std: float) -> None:
        """
        Initializes the state and convariance matrix of the kalman filter, and sets the uncertainties of the 
        prediction and update steps.

        Parameters
        ----------
        accel_std
            The accelerometer measurement standard deviation. This is used to assign an uncertainty to the roll and
            pitch measurement during the update step. This is a tuning parameter.
        gyro_std
            The gyroscope measurement standard deviation. This is used to quantify the uncertainty of the gyro bias 
            and angles during the prediction step. This is a tuning parameter.
        init_angle_std
            This is the initial standard deviation of the state covariance for the roll, pitch, and yaw states.
        init_bias_std
            This is the initial standard deviation of the state covariance for the gyro biases.
        """
        self._n_state = 6
        self._n_outputs = 2
        self._n_inputs = 3

        self._gyro_std = gyro_std
        self._accel_std = accel_std

        self._state = np.zeros((self._n_state, 1))
        self._cov = np.zeros((self._n_state, self._n_state))
        self._cov[0, 0] = init_angle_std * init_angle_std
        self._cov[1, 1] = init_angle_std * init_angle_std
        self._cov[2, 2] = init_angle_std * init_angle_std
        self._cov[3, 3] = init_bias_std * init_bias_std
        self._cov[4, 4] = init_bias_std * init_bias_std
        self._cov[5, 5] = init_bias_std * init_bias_std

        
    def state(self) -> np.ndarray:
        """
        Getter for the estimated state.
        """
        return self._state


    @staticmethod
    def state_matrix(dt: float) -> np.ndarray:
        """
        This returns the state matrix for the prediction step. The state matrix models the natrual change in state 
        over time.

        Parameters
        ----------
        dt
            The timestep since the last update in seconds.

        Returns
        -------
        matA
            The state update matrix A
        """
        return np.array([
            [1.0, 0.0, 0.0, -dt, 0.0, 0.0],
            [0.0, 1.0, 0.0, 0.0, -dt, 0.0],
            [0.0, 0.0, 1.0, 0.0, 0.0, -dt],
            [0.0, 0.0, 0.0, 1.0, 0.0, 0.0],
            [0.0, 0.0, 0.0, 0.0, 1.0, 0.0],
            [0.0, 0.0, 0.0, 0.0, 0.0, 1.0]])
    
    
    @staticmethod
    def input_matrix(dt: float) -> np.ndarray:
        """
        This returns the input matrix for the prediction step. The input maps the gyroscope measurements to the
        change in roll, pitch, and yaw.

        Parameters
        ----------
        dt
            The timestep since the last update in seconds.

        Returns
        -------
        matB
            The input matrix B
        """
        return np.array([
            [dt, 0.0, 0.0],
            [0.0, dt, 0.0],
            [0.0, 0.0, dt],
            [0.0, 0.0, 0.0],
            [0.0, 0.0, 0.0],
            [0.0, 0.0, 0.0]])


    @staticmethod
    def output_matrix():
        """
        This returns the output matrix for the udpate step. The output maps the state to the observed measurement.

        Returns
        -------
        matC
            The output matrix C
        """    
        return np.array([
            [1.0, 0.0, 0.0, 0.0, 0.0, 0.0],
            [0.0, 1.0, 0.0, 0.0, 0.0, 0.0]])
    

    def model_uncertainty(self, dt: float) -> np.ndarray:
        """
        Returns the model unceratinty that is used in the predication step. Since the predication uses the gyroscope
        we use the gyroscope measurement noise to determine this uncertainty.

        Parameters
        ----------
        dt
            The timestep since the last update in seconds.

        Returns
        -------
        matQ
            The model uncertainty covariance Q
        """
        matQ = np.zeros((self._n_state, self._n_state))
        matQ[0, 0] = dt * dt * self._gyro_std * self._gyro_std
        matQ[1, 1] = dt * dt * self._gyro_std * self._gyro_std
        matQ[2, 2] = dt * dt * self._gyro_std * self._gyro_std
        matQ[3, 3] = self._gyro_std * self._gyro_std
        matQ[4, 4] = self._gyro_std * self._gyro_std
        matQ[5, 5] = self._gyro_std * self._gyro_std
        return matQ
    

    def measurement_uncertainty(self) -> np.ndarray:
        """
        The uncertainty in the measurements for the update step. Since the roll and pitch measurements are a function
        of the accelerometer measurements, we just assume the noise on the roll, and pitch measurement is the
        same as the accelerometer.

        Returns
        -------
        matR
            The measurement uncertainty matrix R.

        Returns
        -------
        matR
            The measurement uncertainty covariance R
        """
        matR = np.zeros((self._n_outputs, self._n_outputs))
        matR[0, 0] = self._accel_std * self._accel_std
        matR[1, 1] = self._accel_std * self._accel_std
        return matR
    

    @staticmethod
    def rotation_matrix(r: float, p: float, inv: bool) -> np.ndarray:
        """
        Parameters
        ----------
        r
            The roll angle.
        p
            The pitch angle.
        inv
            If true the inverse rotation matrix is returned.

        Returns
        -------
        rot
            The rotation between the gyroscope measurement and the roll/pitch/yaw rates.
        """
        rot = np.array([
            [np.cos(p), 0.0, -np.cos(r)*np.sin(p)],
            [0.0, 1.0, np.sin(p)],
            [np.sin(p), 0.0, -np.cos(r)*np.cos(p)]])
        if inv:
            return np.linalg.inv(rot)
        else:
            return rot
    

    def prediction(self, dt: float, gx: float, gy: float, gz: float) -> None:
        """
        Executes the prediction step of the kalman filter.

        Parameters
        ----------
        dt
            Time since the last prediction.
        gx
            The x-axis gyro measurement in radians/second.
        gy
            The y-axis gyro measurement in radians/second.
        gz
            The z-axis gyro measurement in radians/second.
        """
        matA = self.state_matrix(dt)
        matB = self.input_matrix(dt)
        matQ = self.model_uncertainty()
        rotinv = self.rotation_matrix(self._state[0], self._state[1], True)
        u = rotinv @ np.ndarray([[gx, gy, gz]]).T
        self._state = matA @ self._state + matB @ u
        self._cov = matA @ self.cov @ matA.T + matQ


    def update(self, ax, ay, az) -> None:
        """
        Executes the update step of the kalman filter.

        Parameters
        ----------
        ax
            The x-axis accelerometer measurement in m/s/s.
        ay
            The y-axis accelerometer measurement in m/s/s.
        az
            The z-axis accelerometer measurement in m/s/s.
        """
        # Compute the roll and pitch measurements from the accelerometer measurements.
        roll = np.arctan2(ay, np.sqrt(ax*ax + az*az))
        pitch = np.arctan2(ax, np.sqrt(ay*ay + az*az))
        y = np.array([[roll, pitch]])

        # Compute the kalman gain.
        matC = self.output_matrix()
        matR = self.measurement_uncertainty()
        gain = self.cov @ matC.T @ np.linalg.inv(matC @ self.cov @ self.cov.T + matR)

        # Update state and covariance matrix.
        self._state = self._state + gain @ (y - matC @ self._state)
        self.cov = self.cov - gain @ matC @ self.cov
