# user-application

Welcome to the user-application.
This is directory contains user space application files. One may need to use these files in their application. Copy and replace the toxic.c in the toxic source directory. One may down load Toxic sources from toxic original project github sources.Also copy rest of the files in same directory. Compile normally in order to build toxic executable.Now run toxic from command prompt this will show the menu options in the LCD display.As of now the application can is implemented up-to add new toxid and corresponding names.
Selecting a contact and making call are still pending which need some minimal code that one can do very easily. Please remove comments corresponding code in the toxic.c file. This part is still pending.
Already tested this functionality from command prompt ,which was working fine.So one may implement the minimal extra code to achieve it form LCD display and keypad interface.
General description of the files is as follows
toxic.c — is the file copied from toxic project sources and modified accordingly
lcdlib.c — this file is an implementation of library functions that are interface the keypad and lcd.
lcdlib.c makes calls to ioctl which are functionality of kernel module, one may need to insert the module provided in sources.
make a corresponding node to open LCD (cum keypad) device. run toxic application from toxic sources#./build/toxic
Now your will be able to see display menu on LCD, if everything is properly compiled and installed necessary library for basic toxic application to run.
