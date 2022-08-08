import matplotlib.pyplot as plt
import numpy as np

fig,ax = plt.subplots()
# #A起点坐标(1,2)，终点坐标（6，8）
# A_START=[1,2]
# A_END=[6,8]

# #A起点坐标(0,8)，终点坐标（6，1）
# B_START=[0,8]
# B_END=[6,1]


# X=[A_START[0],B_START[0]]
# Y=[A_START[1],B_START[1]]

# U=[A_END[0]-A_START[0],B_END[0]-B_START[0]]
# V=[A_END[1]-A_START[1],B_END[1]-B_START[1]]

# cout = np.array([ [ 10, 10, 10, 110 ],
#   [ 110, 10, 10, 10 ],
#   [ -10, 10, -10, 110 ],
#   [ 90, 10, -10, 10 ],
#   [ 70, 110, 70, 10 ],
#   [ 70, 10, -30, 10 ],
#   [ -30, 70, 70, 70 ],
#   [ 70, 70, 70, -30 ],
#   [ 10, -30, 10, 70 ],
#   [ 10, 70, 110, 70 ],
#   [ 10, -10, 10, 90 ],
#   [ 110, -10, 10, -10 ]])

# ST_X = cout[:, 0]
# ST_Y = cout[:, 1]
# ED_X = cout[:, 2]
# ED_Y = cout[:, 3]

ST_X = [16,16,76,76]
ST_Y = [16,76,16,76]
ED_X = [16,76,16,76]
ED_Y = [76,76,16,16]

U = []
V = []
X = ST_X
Y = ST_Y
for i in range(len(ST_X)):
    U.append(ED_X[i] - ST_X[i])
    V.append(ED_Y[i] - ST_Y[i])

U = np.array(U)
V = np.array(V)
X = np.array(X)
Y = np.array(Y)


ax.quiver(X,Y,U,V,angles='xy', scale_units='xy', scale=1)
ax.set_xlim([min(min(ST_X), min(ED_X), 0)-10, max(max(ST_X), max(ED_X), 10)+10])
ax.set_ylim([min(min(ST_Y), min(ED_Y), 0)-10, max(max(ST_Y), max(ED_Y), 10)+10])
# ax.set_ylim([-20, 150])
plt.show()