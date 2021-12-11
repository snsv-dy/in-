import numpy as np
from matplotlib import pyplot as plt

# a = np.arange(9, dtype=np.float64).reshape((3, 3))
# a[1:,] += a[1:,:]
# print(a)


# exit()

# Renormalizes the values of `x` to `bounds`
def normalize(x, bounds=(0, 1)):
  return np.interp(x, (x.min(), x.max()), bounds)

# Fourier-based power law noise with frequency bounds.
def fbm(shape, p, lower=-np.inf, upper=np.inf):
  freqs = tuple(np.fft.fftfreq(n, d=1.0 / n) for n in shape)
  freq_radial = np.hypot(*np.meshgrid(*freqs))
  envelope = (np.power(freq_radial, p, where=freq_radial!=0) *
              (freq_radial > lower) * (freq_radial < upper))
  envelope[0][0] = 0.0
  phase_noise = np.exp(2j * np.pi * np.random.rand(*shape))
  return normalize(np.real(np.fft.ifft2(np.fft.fft2(phase_noise) * envelope)))

# Simple gradient by taking the diff of each cell's horizontal and vertical
# neighbors.
def simple_gradient(a):
  dx = 0.5 * (np.roll(a, 1, axis=0) - np.roll(a, -1, axis=0))
  dy = 0.5 * (np.roll(a, 1, axis=1) - np.roll(a, -1, axis=1))
  return 1j * dx + dy

shape = [100, 100]

cell_area = 1.0
rain_rate = 0.0008 * cell_area

dt = 0.005

pipe_length = 1.1
pipe_area = 1.1
g = 9.8
terrain = plt.imread("height.png")
terrain_zeros = np.zeros(shape=terrain.shape)
print(terrain.shape)

# gradient constant for now since it's not eroded
def computeDeltas(terrain, water):
	delta_height_L = np.hstack([np.zeros(shape=(terrain.shape[0], 1)), terrain[:,:-1] + water[:, :-1]])
	delta_height_L = terrain + water - delta_height_L

	delta_height_R = np.hstack([terrain[:,1:] + water[:, 1:], np.zeros(shape=(terrain.shape[0], 1))])
	delta_height_R = terrain + water - delta_height_R

	delta_height_T = np.vstack([np.zeros(shape=(1, terrain.shape[1]) ), terrain[:-1,:] + water[:-1, :]])
	delta_height_T = terrain + water - delta_height_T

	delta_height_B = np.vstack([terrain[1:,:] + water[1:, :], np.zeros(shape=(1, terrain.shape[1]) )])
	delta_height_B = terrain + water - delta_height_B
	return delta_height_L, delta_height_R, delta_height_T, delta_height_B

def normalMap(terrain):
	# L = np.pad(terrain[:, :-1], (1 ,1))
	L = np.pad(terrain[:, :-1], ((0, 0), (1, 0)))
	R = np.pad(terrain[:, 1:], ((0, 0), (0, 1)))
	T = np.pad(terrain[:-1, :], ((1, 0), (0, 0)))
	B = np.pad(terrain[1:, :], ((0, 1), (0, 0)))
	norm = np.zeros(shape=(terrain.shape[0], terrain.shape[1], 3))
	norm[:, :, 0] = (R - L) / 2
	norm[:, :, 1] = (B - T) / 2
	norm[:, :, 2] = -1

	# up = np.zeros(shape=norm.shape)
	# up[:, :, 1] = 1.0
	
	angle = np.zeros(shape=terrain.shape)

	for y in range(norm.shape[0]):
		for x in range(norm.shape[1]):
			angle[y, x] = np.dot(norm[y, x], [0, 1, 0])
	print(angle)
	return angle

# az = np.array([[1, 2], [3, 4]])
# plt.imshow(normalMap(terrain))
# plt.show()
# exit()

# delta_height_L = terrain[:,1:]
# zeros = np.zeros(shape=(terrain.shape[0], 1))
# print(zeros.shape)
# print(delta_height_T)
# print(terrain)
# print(delta_height_T.shape)
# exit()

flowL = np.zeros(shape=terrain.shape)
flowR = np.zeros(shape=terrain.shape)
flowT = np.zeros(shape=terrain.shape)
flowB = np.zeros(shape=terrain.shape)

water = np.zeros(shape=terrain.shape)

delta_height_L, delta_height_R, delta_height_T, delta_height_B = computeDeltas(terrain, water)

# water precipitation
water += 0.9 # np.random.rand(*terrain.shape) * rain_rate
v_y = 0
for i in range(100):
	# flowL = np.max(terrain_zeros, flowL + (delta_height_L + water) * dt * pipe_area * g / pipe_length)
	delta_height_L, delta_height_R, delta_height_T, delta_height_B = computeDeltas(terrain, water)
	# flowL = np.zeros(shape=(terrain.shape)) #np.maximum(0.0, flowL + dt * pipe_area * g * (delta_height_L) / pipe_length)
	# flowR = np.zeros(shape=(terrain.shape)) #np.maximum(0.0, flowR + dt * pipe_area * g * (delta_height_R) / pipe_length)
	# flowT = np.zeros(shape=(terrain.shape)) #np.maximum(0.0, flowT + dt * pipe_area * g * (delta_height_T) / pipe_length)
	# flowB = np.zeros(shape=(terrain.shape)) #np.maximum(0.0, flowB + dt * pipe_area * g * (delta_height_B) / pipe_length)

	flowL = np.maximum(0.0, flowL + dt * pipe_area * g * (delta_height_L) / pipe_length)
	flowR = np.maximum(0.0, flowR + dt * pipe_area * g * (delta_height_R) / pipe_length)
	flowT = np.maximum(0.0, flowT + dt * pipe_area * g * (delta_height_T) / pipe_length)
	flowB = np.maximum(0.0, flowB + dt * pipe_area * g * (delta_height_B) / pipe_length)

	K = np.minimum(1.0, water * cell_area * cell_area / (flowL + flowR + flowT + flowB) * dt )
	flowL *= K
	flowR *= K
	flowT *= K
	flowB *= K

	flowOut = np.zeros(shape=terrain.shape)
	flowOut[:, 1:] += flowL[:, 1:]
	flowOut[:, :-1] += flowR[:, :-1]
	flowOut[1:, :] += flowT[1:, :]
	flowOut[:-1, :] += flowB[:-1, :]

	flowIn = np.zeros(shape=terrain.shape)
	flowIn[:, :-1] += flowL[:, 1:]
	flowIn[:, 1:] += flowR[:, :-1]
	flowIn[:-1, :] += flowT[1:, :]
	flowIn[1:, :] += flowB[:-1, :]

	waterPassingX = np.zeros(shape=terrain.shape)
	waterPassingX[:, :-1] += flowL[:, 1:]
	waterPassingX[:, 1:] += flowR[:, :-1]
	waterPassingX[:, 1:] -= flowL[:, 1:]
	waterPassingX[:, :-1] -= flowR[:, :-1]

	waterPassingY = np.zeros(shape=terrain.shape)
	waterPassingY[1:, :] -= flowT[1:, :]
	waterPassingY[:-1, :] -= flowB[:-1, :]
	waterPassingY[:-1, :] += flowT[1:, :]
	waterPassingY[1:, :] += flowB[:-1, :]

	dV = (flowIn - flowOut)

	next_water = water + dV/(cell_area * cell_area)
	avg_water = (water + next_water) / 2.0
	
	# v_x = waterPassingX / (avg_water * cell_area)
	# v_y = waterPassingY / (avg_water * cell_area)

	# v_length = np.sqrt(np.power(v_x, 2.0) + np.power(v_y, 2.0))

	# tilt = computeTilt(terrain)

	# water = np.maximum(0.0, water - (flowL + flowR + flowT + flowB))
	water = next_water

plt.subplot(1, 2, 1)
plt.imshow(terrain)

plt.subplot(1, 2, 2)
plt.imshow(water)

plt.show()


# plt.subplot(1, 2, 1)
# plt.imshow(terrain)
