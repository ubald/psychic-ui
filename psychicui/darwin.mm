#import <Cocoa/Cocoa.h>

namespace psychicui {

    void disable_saved_application_state_osx() {
        [[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"NSQuitAlwaysKeepsWindows"];
    }

}
