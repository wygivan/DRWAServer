#!/usr/bin/python

import os
import sys

def main():
  if len(sys.argv) != 2:
    print ('USAGE: %s {build|clean|start|stop|status}' % sys.argv[0])
    sys.exit()

  if sys.argv[1] == 'build':
    os.system('make -s')
  elif sys.argv[1] == 'clean':
    os.system('rm -f *.log')
    os.system('make -s clean')
  elif sys.argv[1] == 'start':
    os.system('sudo modprobe tcp_probe port=8001')
    os.system('sudo cat /proc/net/tcpprobe > tcpprobe.log &')
    os.system('./DRWAServer > throughput.log &')
  elif sys.argv[1] == 'stop':
    os.system('killall DRWAServer')
    os.system("for i in `ps aux | grep '[c]at /proc/net/tcpprobe' | awk '{print $2}'`;do sudo kill -9 $i;done")
    os.system('sudo modprobe -r tcp_probe')
  elif sys.argv[1] == 'status':
    os.system('ps aux | grep "[c]at /proc/net/tcpprobe"')
    os.system('ps aux | grep "[D]RWAServer"')
    os.system('lsmod | grep tcp_probe')
  else:
    print 'Unknown command'

if __name__ == '__main__':
  main()
