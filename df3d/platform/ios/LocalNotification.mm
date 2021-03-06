#include <df3d/platform/LocalNotification.h>

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

namespace df3d {

bool LocalNotification::schedule(int notificationID, const char *msg, double secondsFromNow)
{
    if (secondsFromNow <= 0.0)
        return false;
#ifndef DF3D_APPLETV
    UILocalNotification *notif = [[UILocalNotification alloc] init];
    notif.fireDate = [NSDate dateWithTimeIntervalSinceNow:secondsFromNow];
    notif.timeZone = [NSTimeZone defaultTimeZone];
    notif.alertBody = [NSString stringWithUTF8String:msg];
    notif.alertAction = @"";
    notif.applicationIconBadgeNumber = 1;
    notif.soundName = UILocalNotificationDefaultSoundName;
    notif.userInfo = [NSDictionary dictionaryWithObject:[NSNumber numberWithInt:notificationID] forKey:@"df3d_id"];

    [[UIApplication sharedApplication] scheduleLocalNotification:notif];
    [notif release];
#endif

    return true;
}

void LocalNotification::cancel(int notificationID)
{
#ifndef DF3D_APPLETV
    for (UILocalNotification *notif : [[UIApplication sharedApplication] scheduledLocalNotifications])
    {
        if ([[notif.userInfo objectForKey:@"df3d_id"] isEqualToNumber:[NSNumber numberWithInt:notificationID]])
            [[UIApplication sharedApplication] cancelLocalNotification:notif];
    }
#endif
}

void LocalNotification::registerLocalNotifications()
{
#ifndef DF3D_APPLETV
    if ([UIDevice currentDevice].systemVersion.floatValue >= 8.0)
    {
        UIUserNotificationSettings *settings = [UIUserNotificationSettings settingsForTypes:UIUserNotificationTypeBadge | UIUserNotificationTypeAlert | UIUserNotificationTypeSound categories:nil];
        [[UIApplication sharedApplication] registerUserNotificationSettings:settings];
    }
#endif
}

}
