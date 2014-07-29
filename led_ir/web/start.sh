#!/bin/sh
tmux new -d -s led_ir_tty 'stty -F /dev/ttyACM0 9600; netcat -k -l -p 7777 > /dev/ttyACM0'
tmux new -d -s led_ir 'unicorn -c unicorn.conf'
