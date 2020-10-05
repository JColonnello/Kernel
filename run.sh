#!/bin/bash
qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 1G -rtc base=localtime -cpu Nehalem #-s -S
