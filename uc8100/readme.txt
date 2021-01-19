A. What is MXIO Linux Library for 8100 Series ?
=======================================
     MXIO Linux Library is designed for programmers accessing remote digital and analog data
 from Moxa iologik 4000 I/O Server, Moxa iologik 2000 I/O Server,Moxa iologik 4200 I/O Server, 
 Moxa iologik 5000 I/O Server, ioLogik 1200 I/O Server and and ioLogik R1000 I/O Server module
 over an Ethernet or RS-485 network.
 MXIO Linux Library for 8100 Series ,(arm-linux-2.12-mxio-library-x.x.x.x.tgz), supports MOXA UC-8100 Series.
 Please be noticed that Click&Go command is only used for ioLogik 2000,ioLogik 4200 and ioLogik 5000 Ethernet series

B. Supported Platforms
======================
   arm-linux-2.12-mxio-library-x.x.x.x.sh	V	Support UC-8100 Series 


C. Supported I/O Modules
========================
      MXIO Linux Library supported below I/O modules.
      ioLogik 4000 series: NA-4010, NA-4020, NA-4021
                           M-1400, M-1401, M-1410, M-1411, M-1800, M-1801, M-1600, M-1601, M-1450, M-1451
                           M-2400, M-2401, M-2800, M-2801, M-2600, M-2601, M-2402, M-2403, M-2404, M-2405, M-2250, M-2254
                           M-3400, M-3401, M-3402, M-3403, M-3410, M-3411, M-3412, M-3413, M-3414, M-3415
                           M-6200, M-6201
                           M-4201, M-4202, M-4210, M-4211, M-4212
      ioLogik 2000 series: E2210, E2212, E2240, E2260, R2110, R2140, E2214, E2242, E2262 
	  ioLogik 4200 series: E4200
                           M-1400, M-1401, M-1410, M-1411, M-1800, M-1801, M-1600, M-1601, M-1450, M-1451
                           M-2400, M-2401, M-2800, M-2801, M-2600, M-2601, M-2402, M-2403, M-2404, M-2405, M-2250, M-2254
                           M-3400, M-3401, M-3402, M-3403, M-3410, M-3411, M-3412, M-3413, M-3414, M-3415
                           M-6200, M-6201
                           M-4201, M-4202, M-4210, M-4211, M-4212
      ioLogik 5000 series: W5340, W5312
	  ioLogik 1200 series: E1210, E1211, E1212, E1214, E1240, E1241, E1242, E1260, E1262, E1261-WP-T, E1261H, E1263H, E1213
	  ioLogik 1500 series: E1510, E1512
      ioLogik R1000 series: R1210, R1212, R1214, R1240, R1241

D. File List:
=============   
   ../                            API_SUPPORT_LIST.pdf
   ../                            MXIO DLL API Reference (mxio.chm)
   ../                            MOXA END-USER LICENSE AGREEMENT FOR MXIO SOFTWARE (25-MOXA EULA_MXIO_2015-11-17.rtf)
   ../installer/                  arm-linux-2.12-mxio-library-x.x.x.x.sh
   ../examples/ethernet/          e12xx.cpp
                                  makefile
   ../examples/activetag/         activetag.cpp
                                  makefile
E. Installation of MXIO Linux Library:
======================================
   For MOXA UC-8100 Series:
   Step1. First, you must install UC-8100 Series tool chain.
   Step2. To install the Library, unpacked arm-linux-2.12-mxio-library-1.x.x.x.tgz on your host PC
          # cd /
          # mkdir uc8100 Series 
          # cd uc8100 Series 
          # cp /mnt/cdrom/arm-linux-2.12-mxio-library-x.x.x.x.tgz .
          # tar xvfz arm-linux-2.12-mxio-library-x.x.x.x.tgz
          # cd /installer
   Step3. Next, run the following script as root, to install the libraries and header file in the /usr/local/arm-linux-2.12 directory
          # ./arm-linux-2.12-mxio-library-x.x.x.x.sh
   Step4. The Library installation will take a few minutes to complete.
   Step5. Programming your program on your development envelopment.







