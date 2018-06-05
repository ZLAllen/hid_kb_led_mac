USB Keyboard Backlit Control for Mac

Usage: Edit vendor ID and product ID macro in HelloTestKit.c, then edit the deviceType macro. The example used here is CM Devastator II keyboard which has the following property: Vendor ID 9610, Product ID 1, device type kHIDUsage_GD_Keyboard

The code will proceed to scan all usb devices and return the matching one. Then it will search for LED elements inside the devices and set it to the maximum value (Brightness, ON/OFF). The program serves as a runloop to constantly detect the plugin and unplug of the matching device.



