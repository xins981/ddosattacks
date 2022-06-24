import pandas as pd
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.lines import Line2D
import sys
import signal


class Hyperparameter:

    def __init__(self):
        self.load_limit = 0
        self.test_cycle = 0
        self.threshold_low = 0
        self.threshold_high = 0

class Detecter:

    def __init__(self, ax, file_path, statistic_name,  hyperparameter):
        self.ax = ax

        self.file_path = file_path
        self.statistic_name = statistic_name

        self.load_limit = hyperparameter.load_limit
        self.test_cycle = hyperparameter.test_cycle
        self.threshold_low = hyperparameter.threshold_low
        self.threshold_high = hyperparameter.threshold_high

        self.time_seq = None
        self.recived_packets = None
        self.traffic = [0]
        self.traffic_mean = [0]
        self.dvar = [-1]
        self.time = [0]
        self.max_time = 0

        self.is_update = False
        self.have_data = False

        self.line_normal_traffic = Line2D(self.time, self.traffic, linewidth=2, color='tab:green', label='正常流量')
        self.line_alert_traffic = Line2D(self.time, self.traffic, linewidth=2, color='tab:red', label='异常流量')
        self.line_traffic_mean = Line2D(self.time, self.traffic_mean, linewidth=2, color='tab:orange', label='流量均值', linestyle='--')
        # self.line_dvar = Line2D(self.time, self.traffic, linewidth=2, color='#9467bd', linestyle='-.', label='差分方差')

        self.ax.add_line(self.line_traffic_mean)
        self.ax.add_line(self.line_normal_traffic)
        self.ax.add_line(self.line_alert_traffic)
        # self.ax.add_line(self.line_dvar)
        ax.legend(loc='best')
        self.show_range_x = 50
        self.ax.set_xlim(0, self.show_range_x)

    
    def GetVector(self):
        try:
            statistics = pd.read_csv(self.file_path, converters={
                'vectime': parse_ndarray,
                'vecvalue': parse_ndarray
            })
            target_row = statistics[(statistics.type == 'vector') & (statistics.name == self.statistic_name)].iloc[0]
            time = target_row.vectime.astype(int)
            recived_packets = target_row.vecvalue.astype(int)
            if np.array_equiv(self.time_seq, time) and np.array_equiv(self.recived_packets, recived_packets):
                self.is_update = False
            else:
                self.time_seq = time
                self.recived_packets = recived_packets
                self.is_update = True
            self.have_data = True
        except:
            self.have_data = False

    def GetTraffic(self):
        self.max_time = max(self.time_seq) + 1
        indexPacketNum = np.zeros(self.max_time, dtype=int)
        for i in range(0, self.max_time):
            indexArr = np.argwhere(self.time_seq == i)
            if len(indexArr) > 0:
                indexPacketNum[i] = indexArr[-1]
            else:
                indexPacketNum[i] = indexPacketNum[i - 1]
        self.traffic = np.zeros(self.max_time, dtype=float)
        for t in range(0, self.max_time):
            self.traffic[t] = self.recived_packets[indexPacketNum[t]]
        self.traffic = np.insert(np.diff(self.traffic), 0, 0)

    def Mu(self, t):
        if self.traffic[t] < (self.threshold_low * self.traffic_mean[t]):
            return 0
        elif self.traffic[t] > (self.threshold_high * self.traffic_mean[t]):
            return 1
        else:
            first = self.traffic[t] / self.traffic_mean[t] - self.threshold_low
            second = self.threshold_high - self.threshold_low
            return first / second

    def Dvar(self, t):
        if t < 2:
            self.dvar[t] = 0
            return
        z = self.traffic[t] - self.traffic[t-1]
        first = (t - 2) * self.dvar[t - 1] + np.square(z)
        second = t - 1
        self.dvar[t] = first / second

    def GetAlertTime(self):
        alert_time = []
        self.traffic_mean = np.zeros(self.max_time)
        self.dvar = np.zeros(self.max_time)

        a = 0
        A = 0
        last_A = 0
        mu = 0
        last_mu = 0
        is_compute_trafficmean = True
        for t in np.arange(1, self.max_time):
            if self.traffic[t] == 0:
                s = sys.maxsize
            else:
                s = int(self.test_cycle * self.load_limit / self.traffic[t])
            self.Dvar(t)
            self.traffic_mean[t] = self.traffic_mean[t - 1]
            if s == 0:
                alert_time.append(t)
                is_compute_trafficmean = False
                A = 1
                mu = 1
            else:
                if is_compute_trafficmean:
                    self.traffic_mean[t] = ((t - 1) * self.traffic_mean[t - 1] + self.traffic[t]) / t
                mu = self.Mu(t)
                if mu > 0:
                    a += 1
                    A = max(mu, last_mu) if self.dvar[t] < self.dvar[t - 1] else mu
                    if a >= s:
                        if A > last_A:
                            alert_time.append(t)
                            is_compute_trafficmean = False
                        else:
                            a = s - 1
                else:
                    self.traffic_mean[t] = ((t - 1) * self.traffic_mean[t - 1] + self.traffic[t]) / t
                    is_compute_trafficmean = True
                    A = 0
                    a = 0
            last_A = A
            last_mu = mu

        return alert_time

    def Update(self, i):
        # 获取最新数据
        self.GetVector()
        if self.have_data and self.is_update:
            self.GetTraffic()
            alert_time_list = self.GetAlertTime()
            self.time = np.arange(0, self.max_time)
            self.alert_time = np.ma.masked_where(np.isin(self.time, alert_time_list, invert=True), self.time)
            self.normal_time = np.ma.masked_where(np.isin(self.time, alert_time_list, invert=False), self.time)

            # 更新坐标轴
            xmin, xmax = ax.get_xlim()
            if self.max_time > xmax:
                ax.set_xlim(xmax-10, xmax+self.show_range_x)
                ax.figure.canvas.draw()

            xmin, xmax = ax.get_xlim()
            xmin = int(xmin)
            xmax = int(xmax)
            ymin, ymax = ax.get_ylim()
            max_statistic_value = np.max(np.append(self.traffic[xmin:xmax+1], self.traffic_mean[xmin:xmax+1]))
            if max_statistic_value > ymax:
                ax.set_ylim(-0.05 * max_statistic_value, 1.02 * max_statistic_value)
                ax.figure.canvas.draw()

            # 更新折线
            self.line_normal_traffic.set_data(self.normal_time, self.traffic)
            self.line_alert_traffic.set_data(self.alert_time, self.traffic)
            self.line_traffic_mean.set_data(self.time, self.traffic_mean)
            # self.line_dvar.set_data(self.time, self.dvar)

            return self.line_traffic_mean, self.line_normal_traffic, self.line_alert_traffic,

def parse_ndarray(value):
    return np.fromstring(value, sep=' ') if value else None

def SignalHandler(signum, frame):
    ani.pause()

def call_back(event):
    ax = event.inaxes
    x_min, x_max = ax.get_xlim()
    y_min, y_max = ax.get_ylim()
    range_x = (x_max - x_min) / 10
    range_y = (y_max - y_min) / 10
    if event.button == 'up':
        ax.set(xlim=(x_min + range_x, x_max - range_x), ylim=(y_min + range_y, y_max - range_y))
    elif event.button == 'down':
        ax.set(xlim=(x_min - range_x, x_max + range_x), ylim=(y_min - range_y, y_max + range_y))
    fig.canvas.draw_idle()


mpl.rcParams[u'font.sans-serif'] = ['simhei']
signal.signal(signal.SIGTERM, SignalHandler)

file_path = './results/traffic.csv'
statistic_name = 'NumReceivedIpPackets:vector'

hyperparameter = Hyperparameter()
hyperparameter.load_limit = 30
hyperparameter.test_cycle = 2
hyperparameter.threshold_low = 2.5
hyperparameter.threshold_high = 3.5

fig, ax = plt.subplots()
fig.canvas.manager.set_window_title('异常流量检测系统')
fig.canvas.mpl_connect('scroll_event', call_back)
ax.set_title('实时流量分析', fontsize=20)
ax.set_xlabel('时间（仿真秒）', fontsize=14)
ax.set_ylabel('流量（数据包/秒）', fontsize=14)
ax.grid()
plt.rcParams['axes.unicode_minus'] = False
plt.yticks(size=12)
plt.xticks(size=12)

detecter = Detecter(ax, file_path, statistic_name, hyperparameter)

ani = animation.FuncAnimation(fig, detecter.Update, interval=500)

plt.show()
