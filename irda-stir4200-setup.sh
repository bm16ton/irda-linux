#!/bin/bash
sudo modprobe irda
sudo modprobe ircomm
sudo modprobe ircomm-tty
sudo irattach irda0 -s
pilot-xfer -p /dev/ircomm0 -l

