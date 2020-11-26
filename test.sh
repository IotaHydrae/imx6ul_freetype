echo -e "\033[31m Auto Testing\033[0m"
echo -e "\033[32m Usage: ./show_line ./simsun.ttc <lcd_x> <lcd_y> [font-size] [row-spacing]\033[0m"
TASK=./show_line
FONT=./simsun.ttc

$TASK $FONT 0 0
sleep 1
$TASK $FONT 0 0 50
sleep 1
$TASK $FONT 0 0 50 10
sleep 1

$TASK $FONT 50 0 50 10
sleep 1

$TASK $FONT 50 50 50 10
sleep 1

echo -e "\033[31m All Finshed\033[0m"