![Yalamov Ping Logo](https://github.com/AmirYalamov/cloudflare-internship-2020-systems-submission/blob/master/images/yalamovping.png)

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


## Using ping
To use the ping utility, enter the command to run ping and either a hostname or IP address as an argument. For example, both commands are valid: 

```bash
sudo ./ping google.com
sudo ./ping 172.217.1.14
```
which will result in ping running until you hit "Control + C" on your keyboard to exit the ping utility:



## License
[MIT](https://choosealicense.com/licenses/mit/)
