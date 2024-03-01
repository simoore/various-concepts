import numpy as np
import scipy.signal as signal

from methodA import MethodA

# Initialize the filter.
filter = MethodA(accel_std=1e-3, gyro_std=1e-3, init_angle_std=1e-2, init_bias_std=1e-3)

# Generating true motion. Let's filtfilt the 
dt = 10e-3
time = dt * np.arange(1000)
bias_x = 6e-3
bias_y = 5e-3
bias_z = 4e-3

roll = np.zeros(time.shape)
pitch = np.zeros(time.shape)
yaw = np.zeros(time.shape)
roll[time > 2] = 1.0
pitch[time > 4] = -1.0
yaw[time > 6] = -0.5

# Generate measurements
gyro_x = roll + bias_x + np.random.normal(0.0, 1e-3, size=time.shape)
gyro_y = pitch + bias_y + np.random.normal(0.0, 1e-3, size=time.shape)
gyro_z = 0.0 + bias_z + np.random.normal(0.0, 1e-3, size=time.shape)

accel = R.T()