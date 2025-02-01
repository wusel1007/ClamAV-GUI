# clamav-gui
Thank you for downloading ClamAV-GUI:
 a tool that simplifies virus scanning on Linux.

 - This application provides a graphical user interface for clamav and
   freshclam with an integrated service menu for Konqueror/Dolphin
   to scan files directly from your file manager.

Requirements:
 - clamav
 - freshclam

  *For Ubuntu: $ sudo apt install clamav clamav-freshclam

  fixed
- correct dependencies for Ubuntu

Installation:
 - If you use Ubuntu, you can use the available DEB package
 - If you use OpenSUSE, you can use the available RPM package
 - Alternatively, you can build from source

Build Instructions:
 - Requires qt5 base development files, qtchooser, g++, and make
   *For Ubuntu <= 20.04: $ sudo apt install qtchooser qt5-default g++ make
   *For Ubuntu >= 22.04: $ sudo apt install qtchooser qtbase5-dev g++ make

 - Extract ClamAV-GUI-*.tar.gz
   Example: $ tar -xf ClamAV-GUI-*.tar.gz
 - Enter the generated folder
   Example: $ cd ClamAV-GUI-x.x.x
   *For Ubuntu <= 20.04
    also change ( Qt::endl; --> endl; ) in file "setupfilehandler.cpp"
 - Build the package as root with 'qmake', 'make', and finally 'make install'
   Example: $ qmake && make && sudo make install

Questions / Suggestions:
 - Report any problems or suggestions to
   Joerg Macedo da Costa Zopes <joerg.zopes@gmx.de>

Have fun with your virus-free machine.

INFO - Changes using clamav 1.0.7 or 1.4.1
Version 1.0.7 and 1.4.1 of clamav have an issue with "freshclam". Freshclam needs the file "ca-bundle.crt" in the folder "/etc/pki/tls/certs" and neither the folder nor the file exists in Open SuSE.
On Open SuSE I found the file "ca-bundle.pem" in the folder "/etc/ssl" and a softlink to "/etc/pki/tls/certs/ca-bundle.crt" solves the problem.
