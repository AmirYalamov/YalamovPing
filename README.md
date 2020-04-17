<p align="center">
  <img width="300" height="220" src="https://github.com/AmirYalamov/cloudflare-internship-2020-systems-submission/blob/master/images/yalamovping.png">
</p>

The Yalamov Ping is a small Ping CLI for MacOS and Linux. This project was done as a submission to the 2020 Cloudflare Internship Application: Systems challenge.

## Installation

Download the project ZIP file and change directories into the project's directory. Type the following command into the terminal to install the args library:

```bash
sudo make install
```
After that, to build the project, simply type into the command terminal 

```bash
g++ ping.cpp -o ping
```

And you are good to go to use the Yalamov Ping!

## Features

This Ping CLI comes with a nice helpful ```OPTIONS``` menu that can be accessed by running the command ```sudo ./ping -h``` or ```sudo ./ping --help``` which shows all the available features:

![ping help screen](https://github.com/AmirYalamov/cloudflare-internship-2020-systems-submission/blob/master/images/pinghelp.png)


## Using Ping
To use the ping utility, make sure that you have a working network connection by being connected to WiFi or by Ethernet cable. Then enter the command to run ping and either a hostname or IP address as an argument. For example, both commands are valid: 

```bash
sudo ./ping google.com
sudo ./ping 172.217.1.14
```
which will result in ping running until you hit "Control + C" on your keyboard to exit the ping utility:

![ping outout](https://github.com/AmirYalamov/cloudflare-internship-2020-systems-submission/blob/master/images/pingcommand.png)

Ping can have example arguments as such:
- ```sudo ./bing -t 20 vw.ca```
- ```sudo ./bing --ipv6 porsche.com``` 
- ```sudo ./bing -t 50 --ipv6 lamborghini.com```
- ```sudo ./ping -i 3.14 reddit.com```
- ```sudo ./ping -p 2 news.ycombinator.com```

## Built With

- [C++](https://www.cplusplus.com/)
- [args](https://github.com/taywee/args)
- [Atom](https://atom.io/)
- Love🧡

## Credits

I want to thank [Taylor C. Richberger](https://github.com/Taywee) for developing the args library, the C++ langauge team, and of course Cloudflare for offering this amazing opportunity.

## License
[MIT](https://choosealicense.com/licenses/mit/) © [Amir Yalamov](https://amiryalamov.github.io/)
