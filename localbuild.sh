#!/bin/bash

START_DIR=$(pwd)
cd /home/ben/repos/zmk-config
source ~/zephyrproject/.venv/bin/activate

west build -s zmk/app -d /home/ben/zmk-dongle-build-mctech-aluminum/left -b nice_nano_v2 -p always --   -DZMK_CONFIG=/home/ben/repos/zmk-config -DSHIELD="corne_left_peripheral nice_view_adapter nice_view_benflix" &
west build -s zmk/app -d /home/ben/zmk-dongle-build-mctech-aluminum/right -b nice_nano_v2 -p always --  -DZMK_CONFIG=/home/ben/repos/zmk-config -DSHIELD="corne_right nice_view_adapter nice_view_benflix" &
west build -s zmk/app -d /home/ben/zmk-dongle-build-mctech-aluminum/dongle -b nice_nano_v2 -p always -- -DZMK_CONFIG=/home/ben/repos/zmk-config -DSHIELD="corne_dongle_pro_micro" &
west build -s zmk/app -d /home/ben/zmk-dongle-build-mctech-aluminum/reset -b nice_nano_v2 -p always --  -DZMK_CONFIG=/home/ben/repos/zmk-config -DSHIELD="settings_reset" &
wait

mkdir -p /home/ben/corne-fw-mctech-aluminum
cp /home/ben/zmk-dongle-build-mctech-aluminum/left/zephyr/zmk.uf2   /home/ben/corne-fw-mctech-aluminum/corne_left.uf2
cp /home/ben/zmk-dongle-build-mctech-aluminum/right/zephyr/zmk.uf2  /home/ben/corne-fw-mctech-aluminum/corne_right.uf2
cp /home/ben/zmk-dongle-build-mctech-aluminum/dongle/zephyr/zmk.uf2 /home/ben/corne-fw-mctech-aluminum/corne_dongle.uf2
cp /home/ben/zmk-dongle-build-mctech-aluminum/reset/zephyr/zmk.uf2  /home/ben/corne-fw-mctech-aluminum/corne_reset.uf2

cd $START_DIR
deactivate