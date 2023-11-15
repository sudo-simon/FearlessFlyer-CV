echo "Installation and setup nginx server :)"
sudo apt update && add-apt-repository universe && apt install libnginx-mod-rtmp ffmpeg
sudo systemctl start nginx
echo "Done :)"
