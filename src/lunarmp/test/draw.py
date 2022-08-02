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

ST_X = [10,110,90,90,-10,90,10,10]
ST_Y = [10,10,110,10,90,90,-10,90]
ED_X = [10,10,90,-10,90,90,10,110]
ED_Y = [110,10,10,10,90,-10,90,90]

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
ax.set_xlim([-20, 120])
ax.set_ylim([-20, 120])
plt.show()