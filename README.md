# What is this

A small sip cli app based on simpleua and clidemo example. So there's 70% identical with the source provided by pjsip.

# To compile

1. Download latest pjsua (2.4 for latest test) and compile

2. Install qmake (qt4-qmake or qt5-qmake is just fine, since we'll use it along with Qt Creator), and Qt Creator IDE.

	Assume you're on Ubuntu/Debian-based:

	> sudo apt-get install qt4-qmake qtcreator

3. Open Qt Creator and open project file: tinysip.pro

4. Edit the project file to point to where the pjproject-x.x is located at. [PJSIP_DIR]

5. Edit the target build name, find it under your pjproject build. [PJSIP_TARGET]

# Note on issues

1. If there's ssl error:
	- Configure the pjproject with --disable-ssl option

		> cd pjproject-x.x/

		> chmod +x aconfigure configure
		
		> ./configure --disable-ssl

	- Compile pjsip again

		> make dep && make

2. If there's uuid error:
	- Install uuid package

		> sudo apt-get install uuid uuid-dev

