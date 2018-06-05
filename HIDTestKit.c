//     File: main.c
// Abstract: source code for HID LED test tool
//  Version: 1.3
// 
// Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
// Inc. ("Apple") in consideration of your agreement to the following
// terms, and your use, installation, modification or redistribution of
// this Apple software constitutes acceptance of these terms.  If you do
// not agree with these terms, please do not use, install, modify or
// redistribute this Apple software.
// 
// In consideration of your agreement to abide by the following terms, and
// subject to these terms, Apple grants you a personal, non-exclusive
// license, under Apple's copyrights in this original Apple software (the
// "Apple Software"), to use, reproduce, modify and redistribute the Apple
// Software, with or without modifications, in source and/or binary forms;
// provided that if you redistribute the Apple Software in its entirety and
// without modifications, you must retain this notice and the following
// text and disclaimers in all such redistributions of the Apple Software.
// Neither the name, trademarks, service marks or logos of Apple Inc. may
// be used to endorse or promote products derived from the Apple Software
// without specific prior written permission from Apple.  Except as
// expressly stated in this notice, no other rights or licenses, express or
// implied, are granted by Apple herein, including but not limited to any
// patent rights that may be infringed by your derivative works or by other
// works in which the Apple Software may be incorporated.
// 
// The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
// MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
// THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
// OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
// 
// IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
// MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
// AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
// STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// Copyright (C) 2015 Apple Inc. All Rights Reserved.
// 
// ****************************************************
#pragma mark -
#pragma mark * complation directives *
// ----------------------------------------------------

#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif

// ****************************************************
#pragma mark -
#pragma mark * includes & imports *
// ----------------------------------------------------


#define DEBUG1

#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/HID/IOHIDKeys.h>

#define VENDOR 9610
#define PRODUCT 1
#define DEVICETYPE kHIDUsage_GD_Keyboard

// ****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *
// ----------------------------------------------------
// function to create a matching dictionary for usage page & usage
static CFMutableDictionaryRef hu_CreateMatchingDictionaryUsagePageUsage( Boolean isDeviceNotElement,
																		UInt32 inUsagePage,
																		UInt32 inUsage )
{
	// create a dictionary to add usage page / usages to
	CFMutableDictionaryRef result = CFDictionaryCreateMutable( kCFAllocatorDefault,
															  0,
															  &kCFTypeDictionaryKeyCallBacks,
															  &kCFTypeDictionaryValueCallBacks );
	
	if ( result ) {
		if ( inUsagePage ) {
			// Add key for device type to refine the matching dictionary.
			CFNumberRef pageCFNumberRef = CFNumberCreate( kCFAllocatorDefault, kCFNumberIntType, &inUsagePage );
			
			if ( pageCFNumberRef ) {
				if ( isDeviceNotElement ) {
					CFDictionarySetValue( result, CFSTR( kIOHIDDeviceUsagePageKey ), pageCFNumberRef );
				} else {
					CFDictionarySetValue( result, CFSTR( kIOHIDElementUsagePageKey ), pageCFNumberRef );
				}
				CFRelease( pageCFNumberRef );
				
				// note: the usage is only valid if the usage page is also defined
				if ( inUsage ) {
					CFNumberRef usageCFNumberRef = CFNumberCreate( kCFAllocatorDefault, kCFNumberIntType, &inUsage );
					
					if ( usageCFNumberRef ) {
						if ( isDeviceNotElement ) {
							CFDictionarySetValue( result, CFSTR( kIOHIDDeviceUsageKey ), usageCFNumberRef );
						} else {
							CFDictionarySetValue( result, CFSTR( kIOHIDElementUsageKey ), usageCFNumberRef );
						}
						CFRelease( usageCFNumberRef );
					} else {
						fprintf( stderr, "%s: CFNumberCreate( usage ) failed.", __PRETTY_FUNCTION__ );
					}
				}
			} else {
				fprintf( stderr, "%s: CFNumberCreate( usage page ) failed.", __PRETTY_FUNCTION__ );
			}
		}
	} else {
		fprintf( stderr, "%s: CFDictionaryCreateMutable failed.", __PRETTY_FUNCTION__ );
	}
	return result;
}	// hu_CreateMatchingDictionaryUsagePageUsage

static void Handle_DeviceMatchingCallback(
                                          void *          inContext,       // context from IOHIDManagerRegisterDeviceMatchingCallback
                                          IOReturn        inResult,        // the result of the matching operation
                                          void *          inSender,        // the IOHIDManagerRef for the new device
                                          IOHIDDeviceRef  inIOHIDDeviceRef // the new HID device
) {
    if ( !IOHIDDeviceConformsTo( inIOHIDDeviceRef , kHIDPage_GenericDesktop, DEVICETYPE ) ) {
        return;    // ...skip it
    }
    
    //check for sucess
    CFNumberRef vendor = (CFNumberRef)IOHIDDeviceGetProperty(inIOHIDDeviceRef, CFSTR(kIOHIDVendorIDKey));
    CFNumberRef product = (CFNumberRef)IOHIDDeviceGetProperty(inIOHIDDeviceRef, CFSTR(kIOHIDProductIDKey));
    
    if(vendor && product){
        int vendorId, productId;
        if(!CFNumberGetValue(vendor, 3, &vendorId)){
            printf("Cannot fetch Vendor ID\n");
        }
        
        if(!CFNumberGetValue(product, 3, &productId)){
            printf("Cannot fetch Product ID\n");
        }
        
        if(vendorId == VENDOR && productId == PRODUCT){
            
            printf("%s(result: %d, sender: %p, device: %p).\n",
                   __PRETTY_FUNCTION__, inResult, inSender, (void*) inIOHIDDeviceRef);
            
#ifdef DEBUG1
            printf("device = %p.\n", inIOHIDDeviceRef );
            printf("Vendor ID: %d\n", vendorId);
            printf("Product ID: %d\n", productId);
#endif
            
            CFDictionaryRef matchingElementCFDictRef = hu_CreateMatchingDictionaryUsagePageUsage(FALSE, kHIDPage_LEDs, 0 );
            
            CFArrayRef elementCFArrayRef = IOHIDDeviceCopyMatchingElements( inIOHIDDeviceRef,
                                                                           matchingElementCFDictRef,
                                                                           kIOHIDOptionsTypeNone );
            
            if(!elementCFArrayRef)
            {
                fprintf(stderr, "element loading failed\n");
                return;
            }
            
            CFRelease(matchingElementCFDictRef);
            
            CFIndex elementIndex, elementCount = CFArrayGetCount( elementCFArrayRef );
            for ( elementIndex = 0; elementIndex < elementCount; elementIndex++ ) {
                IOHIDElementRef tIOHIDElementRef = ( IOHIDElementRef ) CFArrayGetValueAtIndex( elementCFArrayRef, elementIndex );
                __Require( tIOHIDElementRef, next_element );
                
                uint32_t usagePage = IOHIDElementGetUsagePage( tIOHIDElementRef );
                
                // if this isn't an LED element...
                if ( kHIDPage_LEDs != usagePage ) {
                    continue;    // ...skip it
                }
                uint32_t usage = IOHIDElementGetUsage( tIOHIDElementRef );
                IOHIDElementType tIOHIDElementType = IOHIDElementGetType( tIOHIDElementRef );
#ifdef DEBUG1
                printf( "         element = %p (page: %d, usage: %d, type: %d ).\n",
                       tIOHIDElementRef, usagePage, usage, tIOHIDElementType );
#endif
                
                CFIndex maxCFIndex = IOHIDElementGetLogicalMax( tIOHIDElementRef );
                
                CFIndex tCFIndex = maxCFIndex;
                
#ifdef DEBUG1
                printf( "             value = 0x%08lX.\n", tCFIndex );
#endif
                
                uint64_t timestamp = 0; // create the IO HID Value to be sent to this LED element
                IOHIDValueRef tIOHIDValueRef = IOHIDValueCreateWithIntegerValue( kCFAllocatorDefault, tIOHIDElementRef, timestamp, tCFIndex );
                if ( tIOHIDValueRef ) {
                    // now set it on the device
                    IOReturn tIOReturn = IOHIDDeviceSetValue( inIOHIDDeviceRef, tIOHIDElementRef, tIOHIDValueRef );
                    CFRelease( tIOHIDValueRef );
                    __Require_noErr( tIOReturn, next_element );
                    //delayFlag = TRUE;    // set this TRUE so we'll delay before changing our LED values again
                }
            next_element:    ;
                continue;
                
            }
            
        }
        

    }

}

// this will be called when a HID device is removed (unplugged)
static void Handle_RemovalCallback(
                                   void *         inContext,       // context from IOHIDManagerRegisterDeviceMatchingCallback
                                   IOReturn       inResult,        // the result of the removing operation
                                   void *         inSender,        // the IOHIDManagerRef for the device being removed
                                   IOHIDDeviceRef inIOHIDDeviceRef // the removed HID device
) {
    printf("%s(result: %d, sender: %p, device: %p).\n",
           __PRETTY_FUNCTION__, inResult, inSender, (void*) inIOHIDDeviceRef);
}   // Handle_RemovalCallback


int main( int argc, const char * argv[] )
{
#pragma unused ( argc, argv )
    //IOHIDDeviceRef * tIOHIDDeviceRefs = nil;
    
    // create a IO HID Manager reference
    IOHIDManagerRef tIOHIDManagerRef = IOHIDManagerCreate( kCFAllocatorDefault, kIOHIDOptionsTypeNone );
    __Require( tIOHIDManagerRef, Oops );
    

    
    // set the HID device matching dictionary
    IOHIDManagerSetDeviceMatching( tIOHIDManagerRef, NULL);

    
    
    //Register a device Matching Callback
    
    IOHIDManagerRegisterDeviceMatchingCallback(tIOHIDManagerRef, &Handle_DeviceMatchingCallback, NULL);
    IOHIDManagerRegisterDeviceRemovalCallback(tIOHIDManagerRef, &Handle_RemovalCallback, NULL);
    
    //Register a RunLoop
    
    IOHIDManagerScheduleWithRunLoop(tIOHIDManagerRef, CFRunLoopGetCurrent() , kCFRunLoopDefaultMode);

    
    // Now open the IO HID Manager reference
    IOReturn tIOReturn = IOHIDManagerOpen( tIOHIDManagerRef, kIOHIDOptionsTypeNone );
    if(tIOReturn != kIOReturnSuccess){
        fprintf(stderr, "failed to open manager");
        return -1;
    }
    
    CFRunLoopRun();
    

    
Oops:
    
    if ( tIOHIDManagerRef ) {
        CFRelease( tIOHIDManagerRef );
    }
    
    fprintf(stderr, "something went wrong oops\n");
    
    return -1;
    
}





