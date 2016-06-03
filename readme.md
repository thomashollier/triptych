<h1>Triptych</h1>
This allows you to sync video playback across multiple raspberry pi mini computers.

<h3>Set up openframeworks on your raspberry pi</h3>
-Read this: http://forum.openframeworks.cc/t/raspberry-pi-2-setup-guide/18690<br>
-Put ofxOMXplayer stuff in addons and make sure the directory is named ofxOMXplayer<br>
-Add gpu_mem=384 in /boot/config.txt<br>

<h3>Using the default settings</h3>
I've set things up to use a settings file that tells the program the machine names, the mode they are in, and the video files they each load. That's the playback_settings.xml file. As you can see, you don't have to call your machines pi1, pi2 and pi3 though if you do you will have to update all the files I mention above to reflect the new names. 

<h4>Setup your host names</h4>
sudo vi /etc/hostname
Change the hostname to pi2 for the second machine and pi3 for the third.

<h4>Setup the IP address of each host</h4>
sudo vi /etc/network/interfaces
Set the desired IP on the "address" line (you will need to do this on all 3 machines if your network settings are different from mine)
<pre>
auto lo
iface lo inet loopback

iface eth0 inet static
address 192.168.11.45
netmask 255.255.255.0
gateway 192.168.11.1
network 192.168.11.1
broadcast 192.168.11.255

allow-hotplug wlan0
iface wlan0 inet manual
wpa-roam /etc/wpa_supplicant/wpa_supplicant.conf
iface default inet dhcp
</pre>
<h4>Setup your hosts file</h4>
sudo vi /etc/hosts
Set the correct IP address for pi1, pi2, and pi3
<pre>
127.0.0.1	localhost
::1		localhost ip6-localhost ip6-loopback
fe00::0		ip6-localnet
ff00::0		ip6-mcastprefix
ff02::1		ip6-allnodes
ff02::2		ip6-allrouters

192.168.11.45 	pi1
192.168.11.46 	pi2
192.168.11.47 	pi3
</pre>

<h4>setup ssh keys</h4>
In order to be able to shutdown and reboot the slaves from the master machine, you need to setup ssh keys so you can connect back and forth between the machines without needing to enter a password.
<pre>
First, on each machine:
rm .ssh/authorized_keys
then, go back to each machine and do this:
ssh-keygen -t rsa -C pi@pi2
cat ~/.ssh/id_rsa.pub | ssh pi@192.168.11.45 'cat >> .ssh/authorized_keys'
cat ~/.ssh/id_rsa.pub | ssh pi@192.168.11.47 'cat >> .ssh/authorized_keys'
Make sure the IP's in line 2 and 3 are the ones of whatever machines you are currently NOT logged into, and that the machine name in the first line is that of the machine you are currently logged in.
</pre>

<h4>Install the media</h4>
Copy the movie files somewhere onto your SD cards and make sure the "filename" value in the xml file reflects the path.

In theory, once that is done, you should be able to launch the machines. the process will automatically start, they will find each other and the movies will loop automatically

A technical detail: this setup uses the linux program "screen", which allows you to launch the program in a separate shell.  That separate shell is launched at the end of the .bashrc file. The "if" statement ensures the command is only run from the machine main boot shell.


I would also try to spend a little time getting your laptop on the same network. Plug your mac's ethernet cable into the router, select the ethernet connection in system preferences network pane, select Manual under Configure IPv4 and put in the same values for the fields that you have in the etc/network/interfaces file, except for the IP Address which should be unique to your machine.

You can use the convenience scripts called "reboot_all" and "poweroff_all" which allow you to reboot or poweroff all machines from the master machine
