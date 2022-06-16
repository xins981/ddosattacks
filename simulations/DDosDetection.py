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
        self.dvar = [0]
        self.time = [0]
        self.max_time = 0

        self.line_traffic_mean = Line2D(self.time, self.traffic_mean, label='流量均值')
        self.line_normal_traffic = Line2D(self.time, self.traffic, color='g', label='正常流量')
        self.line_alert_traffic = Line2D(self.time, self.traffic, color='r', label='异常流量')

        self.ax.add_line(self.line_traffic_mean)
        self.ax.add_line(self.line_normal_traffic)
        self.ax.add_line(self.line_alert_traffic)
        ax.legend()
    
    def GetVector(self):
        try:
            statistics = pd.read_csv(self.file_path, converters={
                'vectime': parse_ndarray,
                'vecvalue': parse_ndarray
            })
            target_row = statistics[(statistics.type == 'vector') & (statistics.name == self.statistic_name)].iloc[0]
            self.time_seq = target_row.vectime
            self.recived_packets = target_row.vecvalue
        except:
            self.time_seq = None
            self.recived_packets = None

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
        self.traffic = np.diff(self.traffic)
        self.traffic = np.insert(self.traffic, 0, 0)

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
            return 0
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
                    self.test_cycle += 1
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
        if (not self.time_seq is None) and (not self.recived_packets is None):
            self.time_seq = self.time_seq.astype(int)
            self.recived_packets = self.recived_packets.astype(int)
            self.GetTraffic()
            alert_time_list = self.GetAlertTime()
            self.time = np.arange(0, self.max_time)
            self.alert_time = np.ma.masked_where(np.isin(self.time, alert_time_list, invert=True), self.time)
            self.normal_time = np.ma.masked_where(np.isin(self.time, alert_time_list, invert=False), self.time)

            # 更新坐标轴
            xmin, xmax = ax.get_xlim()
            if self.max_time > xmax:
                ax.set_xlim(0, 2 * self.max_time)
                ax.figure.canvas.draw()
            ymin, ymax = ax.get_ylim()

            max_statistic_value = np.max(np.append(self.traffic, self.traffic_mean))
            if max_statistic_value > ymax:
                ax.set_ylim(0, 2 * max_statistic_value)
                ax.figure.canvas.draw()

            # 更新折线
            self.line_traffic_mean.set_data(self.time, self.traffic_mean)
            self.line_normal_traffic.set_data(self.normal_time, self.traffic)
            self.line_alert_traffic.set_data(self.alert_time, self.traffic)
            # self.line_traffic_dvar(time, ndarrDvar, label='Dvar', label='差分方差')

        return self.line_traffic_mean, self.line_normal_traffic, self.line_alert_traffic,

def parse_ndarray(value):
    return np.fromstring(value, sep=' ') if value else None

def SignalHandler(signum, frame):
    ani.pause()


signal.signal(signal.SIGTERM, SignalHandler)

file_path = './results/traffic.csv'
statistic_name = 'NumReceivedIpPackets:vector'

hyperparameter = Hyperparameter()
hyperparameter.load_limit = 30
hyperparameter.test_cycle = 2
hyperparameter.threshold_low = 2.5
hyperparameter.threshold_high = 3.5

mpl.rcParams[u'font.sans-serif'] = ['simhei']

fig, ax = plt.subplots()
ax.set_title('实时流量统计')  # 添加子标题
ax.set_xlabel('时间', fontsize=10)  # 添加轴标签
ax.set_ylabel('流量', fontsize=20)
ax.grid()

detecter = Detecter(ax, file_path, statistic_name, hyperparameter)

ani = animation.FuncAnimation(fig, detecter.Update, interval=500)

plt.show()
