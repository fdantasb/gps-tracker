# Renomeia o binário de saída: firmware.bin -> gps-tracker.bin
Import("env")
env.Replace(PROGNAME="gps-tracker")
