/*
 * menu.mm:  Drop-down and popup menus as NSMenu, built from the core's items.
 *   NSMenu tracks modally: the choice is known when it returns.
 */

#include "appkit/appkit_backend.h"

@interface M59MenuTarget : NSObject
{
@public
   unsigned chosen;
}
- (void)choose:(NSMenuItem *)item;
@end

@implementation M59MenuTarget
- (void)choose:(NSMenuItem *)item
{
   chosen = (unsigned) item.tag;
}
@end

namespace w32::mac {

namespace {

NSMenu *Build(const std::vector<MenuItem> &items, M59MenuTarget *target)
{
   NSMenu *menu = [[[NSMenu alloc] initWithTitle:@""] autorelease];
   menu.autoenablesItems = NO;
   for (const MenuItem &e : items)
   {
      if (e.separator)
      {
         [menu addItem:[NSMenuItem separatorItem]];
         continue;
      }
      NSString *title = [NSString stringWithUTF8String:e.title.c_str()] ?: @"";
      NSMenuItem *item = [[[NSMenuItem alloc] initWithTitle:title action:nil keyEquivalent:@""] autorelease];
      if (e.has_sub)
         item.submenu = Build(e.sub, target);
      else
      {
         item.target = target;
         item.action = @selector(choose:);
         item.tag = e.id;
      }
      item.enabled = !e.grayed;
      item.state = e.checked ? NSControlStateValueOn : NSControlStateValueOff;
      [menu addItem:item];
   }
   return menu;
}

}  // namespace

unsigned TrackMenu(Top *top, const std::vector<MenuItem> &items, const Rect &anchor)
{
   @autoreleasepool
   {
      M59MenuTarget *target = [[M59MenuTarget alloc] init];
      NSMenu *menu = Build(items, target);
      // Below the anchor, in the flipped view's coordinates.
      [menu popUpMenuPositioningItem:nil atLocation:NSMakePoint(anchor.left, anchor.bottom) inView:(NSView *) top->view];
      unsigned id = target->chosen;
      [target release];
      return id;
   }
}

}  // namespace w32::mac
