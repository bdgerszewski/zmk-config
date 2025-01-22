#!/bin/bash

START_DIR=$(pwd)
cd /home/ben/repos/zmk-config
source ~/zephyrproject/.venv/bin/activate

west build -s zmk/app -d /home/ben/zmk-dongle-build-mctech/left -b nice_nano_v2 -p always --   -DZMK_CONFIG=/home/ben/repos/zmk-config -DSHIELD="corne_left_peripheral nice_view_adapter nice_view_benflix" &
west build -s zmk/app -d /home/ben/zmk-dongle-build-mctech/right -b nice_nano_v2 -p always --  -DZMK_CONFIG=/home/ben/repos/zmk-config -DSHIELD="corne_right nice_view_adapter nice_view_benflix" &
west build -s zmk/app -d /home/ben/zmk-dongle-build-mctech/dongle -b nice_nano_v2 -p always -- -DZMK_CONFIG=/home/ben/repos/zmk-config -DSHIELD="corne_dongle_pro_micro" &
west build -s zmk/app -d /home/ben/zmk-dongle-build-mctech/reset -b nice_nano_v2 -p always --  -DZMK_CONFIG=/home/ben/repos/zmk-config -DSHIELD="settings_reset" &
wait

cp /home/ben/zmk-dongle-build-mctech/left/zephyr/zmk.uf2   /home/ben/corne-fw-mctech/corne_left.uf2
cp /home/ben/zmk-dongle-build-mctech/right/zephyr/zmk.uf2  /home/ben/corne-fw-mctech/corne_right.uf2
cp /home/ben/zmk-dongle-build-mctech/dongle/zephyr/zmk.uf2 /home/ben/corne-fw-mctech/corne_dongle.uf2
cp /home/ben/zmk-dongle-build-mctech/reset/zephyr/zmk.uf2  /home/ben/corne-fw-mctech/corne_reset.uf2

cd $START_DIR
deactivate