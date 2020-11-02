#!/bin/bash
qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 --no-reboot -m 128M -rtc base=localtime -cpu Nehalem #-s -S
