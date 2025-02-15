FreeRTOSv10.2.1_191129 combines the latest FreeRTOS Labs projects into the
FreeRTOSv10.2.1 release.  The FreeRTOS kernel code has not changed since the
last release.  The head revision of the FreeRTOS kernel is available in SVN and
Github.


Directories:

+ FreeRTOS/source contains the FreeRTOS real time kernel source code.

+ FreeRTOS/demo contains a pre-configured demo project for every official
  FreeRTOS port.

+ See http://www.freertos.org/a00017.html for full details of the FreeRTOS
  directory structure and information on locating the files you require.

+ FreeRTOS-Plus contains additional FreeRTOS components and third party
  complementary products.  THESE ARE LICENSED SEPARATELY FROM FreeRTOS although
  all contain open source options.  See the license files in each respective
  directory for information.

+ FreeRTOS-Plus/Demo contains pre-configured demo projects for the FreeRTOS-Plus
  components.  Most demo projects run in a Windows environment using the
  FreeRTOS windows simulator.  These are documented on the FreeRTOS web site
  http://www.FreeRTOS.org/plus

+ FreeRTOS-Labs contains libraries and demos that are fully functional, but
  undergoing optimizations or refactoring to improve memory usage, modularity,
  documentation, demo usability, or test coverage.

Further readme files are contains in sub-directories as appropriate.

The easiest way to use FreeRTOS is to start with one of the pre-configured demo
application projects (found in the FreeRTOS/Demo directory).  That way you will
have the correct FreeRTOS source files included, and the correct include paths
configured.  Once a demo application is building and executing you can remove
the demo application file, and start to add in your own application source
files.

See also -
http://www.freertos.org/FreeRTOS-quick-start-guide.html
http://www.freertos.org/FAQHelp.html
