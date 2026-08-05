# Renames the output binary: firmware.bin -> gps-tracker.bin
Import("env")
env.Replace(PROGNAME="gps-tracker")
