import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import sys
import signal

def parse_ndarray(s):
    return np.fromstring(s, sep=' ') if s else None

def Mu(nowTraffic, nowTrafficMean, l, h):
    if nowTraffic < (l * nowTrafficMean):
        return 0
    elif nowTraffic > (h * nowTrafficMean):
        return 1
    else:
        first = nowTraffic / nowTrafficMean - l
        second = h - l
        return first / second

def TrafficMean(t, lastTrafficMean, nowTraffic):
    return ((t - 1) * lastTrafficMean + nowTraffic) / t

def Dvar(t, lastDvar, nowTraffic, lastTraffic):
    if t < 2:
        return 0
    z = nowTraffic - lastTraffic
    first = (t - 2) * lastDvar + np.square(z)
    second = t - 1
    return first / second

def GetAlertTime(traffic, peakTime, peakTraffic, testCycle, l, h):
    alertTime = []
    a = 0
    nowA = 0
    lastA = 0
    nowMu = 0
    lastMu = 0
    isComputerTrafficMean = True
    trafficMean = np.zeros(peakTime)
    dvar = np.zeros(peakTime)
    for t in np.arange(1, peakTime):
        if traffic[t] == 0:
            s = sys.maxsize
        else:
            s = (testCycle * peakTraffic / traffic[t]).astype(int)
        dvar[t] = Dvar(t, dvar[t - 1], traffic[t], traffic[t - 1])
        trafficMean[t] = trafficMean[t - 1]
        if s == 0:
            alertTime.append(t)
            isComputerTrafficMean = False
            nowA = 1
            nowMu = 1
        else:
            if isComputerTrafficMean:
                trafficMean[t] = TrafficMean(t, trafficMean[t - 1], traffic[t])
            nowMu = Mu(traffic[t], trafficMean[t], l, h)
            if nowMu > 0:
                testCycle += 1
                a += 1
                nowA = max(nowMu, lastMu) if dvar[t] < dvar[t - 1] else nowMu
                if a >= s:
                    if nowA > lastA:
                        alertTime.append(t)
                        isComputerTrafficMean = False
                    else:
                        a = s - 1
            else:
                trafficMean[t] = TrafficMean(t, trafficMean[t - 1], traffic[t])
                isComputerTrafficMean = True
                nowA = 0
                a = 0
        lastA = nowA
        lastMu = nowMu

    return alertTime, trafficMean

def GetVector(filePath, statisticName):
    try:
        statistics = pd.read_csv(filePath, converters={
            'vectime': parse_ndarray,
            'vecvalue': parse_ndarray
        })
        targetRow = statistics[(statistics.type == 'vector') & (statistics.name == statisticName)].iloc[0]
    except:
        return None, None

    timeSeq = targetRow.vectime
    recivedPackets = targetRow.vecvalue
    return timeSeq, recivedPackets

def GetTraffic(timeSeq, recivedPackets):
    peakTime = max(timeSeq) + 1
    indexPacketNum = np.zeros(peakTime, dtype=int)
    for i in range(0, peakTime):
        indexArr = np.argwhere(timeSeq == i)
        if len(indexArr) > 0:
            indexPacketNum[i] = indexArr[-1]
        else:
            indexPacketNum[i] = indexPacketNum[i - 1]
    traffic = np.zeros(peakTime, dtype=float)
    for t in range(0, peakTime):
        traffic[t] = recivedPackets[indexPacketNum[t]]
    traffic = np.diff(traffic)
    traffic = np.insert(traffic, 0, 0)

    return traffic

def Draw(filePath, statisticName, peakTraffic, i, l, h):
    plt.ion()
    while isDone == False:
        timeSeq, recivedPackets = GetVector(filePath, statisticName)
        if (not timeSeq is None) and (not recivedPackets is None):
            plt.clf()
            timeSeq = timeSeq.astype(int)
            recivedPackets = recivedPackets.astype(int)
            traffic = GetTraffic(timeSeq, recivedPackets)
            peakTime = max(timeSeq) + 1
            alertTimeList, trafficMean = GetAlertTime(traffic, peakTime, peakTraffic, i, l, h)
            time = np.arange(0, peakTime)
            alertTime = np.ma.masked_where(np.isin(time, alertTimeList, invert=True), time)
            normalTime = np.ma.masked_where(np.isin(time, alertTimeList, invert=False), time)

            agraphic = plt.subplot()
            agraphic.set_title('subplot')  # 添加子标题
            agraphic.set_xlabel('x axis', fontsize=10)  # 添加轴标签
            agraphic.set_ylabel('y axis', fontsize=20)
            agraphic.plot(time, trafficMean, label='TrafficMean')  # 等于agraghic.plot(ax,ay,'g-')
            agraphic.plot(normalTime, traffic, color='g', label='NormalTraffic')
            agraphic.plot(alertTime, traffic, color='r', label='Alert')
            # agraphic.plot(ndarrTime, ndarrDvar, label='Dvar') 差分方差
            agraphic.legend()
            plt.pause(0.3)
    plt.ioff()
    plt.show()

def SignalHandler(signum, frame):
    global isDone
    isDone = True


isDone = False

signal.signal(signal.SIGTERM, SignalHandler)
filePath = './results/traffic.csv'
statisticName = 'NumReceivedIpPackets:vector'

peakTraffic = 30
i = 2
l = 2.5
h = 3.5

Draw(filePath, statisticName, peakTraffic, i, l, h)


