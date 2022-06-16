import matplotlib.pyplot as plt
import numpy as np
import time

# 以下をコメントアウトすると実行の度に乱数列が変わる
np.random.seed(100) # 乱数のシード値を指定（毎回同じ乱数列が出力されるのでデバッグしやすい）

# N: 試行回数
N = 100

start_time = time.perf_counter()

# M個のランダム点を生成
M = 1000
x_list = []
y_list = []

for i in range(M):
  x = np.random.rand()
  x_list.append(x)
  y = np.random.rand()
  y_list.append(y)

# 1/4円弧
circle_x = np.arange(0,1,0.0001)
circle_y = np.sqrt(1 - circle_x**2)

end_time = time.perf_counter()
print("実行時間: {}".format(end_time - start_time))

# matplotlibによる描画
fig = plt.figure(figsize=(5,5))
plt.xlabel("X")
plt.ylabel("Y")
plt.plot(circle_x, circle_y, color='red')
plt.scatter(x_list, y_list, marker='.')
plt.show()