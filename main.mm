#import <UIKit/UIKit.h>
#import "AAPLAppDelegate.h"
//#import "Engine.h"

extern "C" void helloEngine();

// === Main Entry Point ===
int main(int argc, char * argv[]) {
    helloEngine();

    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AAPLAppDelegate class]));
    }
}
