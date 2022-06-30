import numpy as np
import matplotlib.pyplot as plt

N = 100
M = 1000

def montecarlo() -> list:
    list_pi = []
    for i in range(1, N):
        point_x = np.random.rand(M)
        point_y = np.random.rand(M)
        point_length = np.hypot(point_x, point_y)

        in_M = 0
        for length in point_length:
            if length <= 1:
                in_M += 1

        pie = in_M / M * 4
        list_pi.append(pie)
    return list_pi


def draw_hist(pies: list) -> None:
    plt.hist(pies, bins=20)
    plt.title("pi-list")
    plt.show()


def main():
    # print(montecarlo())
    draw_hist(montecarlo())
    
    
if __name__ == '__main__':
    main()
