import numpy as np

from methodA import MethodA

filter = MethodA(accel_std=1e-3, gyro_std=1e-3, init_angle_std=1e-2, init_bias_std=1e-3)

dt = 10e-3
bias_x = 5e-3
bias_y = 5e-3
bias_z = 5e-3
time = dt * np.arange(1000)


roll = np.zeros(time.shape)
pitch = np.zeros(time.shape)
roll[time > 2] = 1.0
pitch[time > 4] = -1.0
yaw = 0.0

gyro_x = roll + bias_x + np.random.normal(0.0, 1e-3, size=time.shape)
gyro_y = pitch + bias_y + np.random.normal(0.0, 1e-3, size=time.shape)
gyro_z = 0.0 + bias_z + np.random.normal(0.0, 1e-3, size=time.shape)

accel = R.T()