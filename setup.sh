sudo useradd -m dwm
sleep 2s
sudo bash -c 'echo "dwm ALL=(ALL:ALL) ALL" >> /etc/sudoers'
sleep 3s
sudo pacman -S xorg-server xorg-xinit xorg-xrandr xorg-xsetroot firefox hdparm
sudo timedatectl set-timezone Europe/Berlin
echo "===================== RELOG BEFORE COMPILING DWM OR ST!!!!!!!!! ====================="

