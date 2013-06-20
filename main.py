#!/usr/bin/python

import Gnuplot
import os
import sys

def main():
  if len(sys.argv) != 2:
    print ('USAGE: %s {build|clean|start|stop|status|plot}' % sys.argv[0])
    sys.exit()

  if sys.argv[1] == 'build':
    os.system('make -s')
  elif sys.argv[1] == 'clean':
    os.system('rm -f *.log *.png')
    os.system('make -s clean')
  elif sys.argv[1] == 'start':
    os.system('sudo modprobe tcp_probe port=8001 full=1')
    os.system('sudo cat /proc/net/tcpprobe > tcpprobe.log &')
    os.system('./DRWAServer downlink > throughput.log &')
  elif sys.argv[1] == 'stop':
    os.system('killall DRWAServer')
    os.system("sudo kill `ps aux | grep '[s]udo cat /proc/net/tcpprobe' | awk '{print $2}'`")
    os.system('sudo modprobe -r tcp_probe')
  elif sys.argv[1] == 'status':
    os.system('ps aux | grep "[s]udo cat /proc/net/tcpprobe"')
    os.system('ps aux | grep "[D]RWAServer"')
    os.system('lsmod | grep tcp_probe')
  elif sys.argv[1] == 'plot':
    time = []
    cwnd = []
    rwnd = []
    rtt = []
    for line in open('tcpprobe.log'):
      cols = line.split()
      time.append(float(cols[0]))
      cwnd.append(int(cols[6]))
      rwnd.append(int(cols[8]))
      rtt.append(int(cols[9]))

    if time and cwnd:
      g = Gnuplot.Gnuplot()
      g.xlabel('Time (s)')
      g.ylabel('cwnd (segment)')
      g.plot(Gnuplot.Data(time, cwnd, with_ = 'linespoints'))
      g.hardcopy('cwnd.png', terminal = 'png')

      g.xlabel('Time (s)')
      g.ylabel('rwnd (byte)')
      g.plot(Gnuplot.Data(time, rwnd, with_ = 'linespoints'))
      g.hardcopy('rwnd.png', terminal = 'png')

      g.xlabel('Time (s)')
      g.ylabel('RTT (ms)')
      g.plot(Gnuplot.Data(time, rtt, with_ = 'linespoints'))
      g.hardcopy('rtt.png', terminal = 'png')
  else:
    print 'Unknown command'

if __name__ == '__main__':
  main()
