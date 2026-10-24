/*
 * dialogs.mm:  The common dialogs as AppKit's color and font panels, run
 *   modally with OK and Cancel, and the dark appearance.
 */

#include "appkit/appkit_backend.h"

#include <algorithm>
#include <cstring>

@interface M59PanelButtons : NSObject
- (void)ok:(id)sender;
- (void)cancel:(id)sender;
@end

@implementation M59PanelButtons
- (void)ok:(id)sender
{
   [NSApp stopModalWithCode:NSModalResponseOK];
}
- (void)cancel:(id)sender
{
   [NSApp stopModalWithCode:NSModalResponseCancel];
}
@end

namespace w32::mac {

namespace {

// OK and Cancel under a panel's own controls; true if OK ended the modal run.
bool RunPanel(NSPanel *panel)
{
   M59PanelButtons *buttons = [[M59PanelButtons alloc] init];
   NSView *bar = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 220, 40)];
   NSButton *cancel = [NSButton buttonWithTitle:@"Cancel" target:buttons action:@selector(cancel:)];
   NSButton *ok = [NSButton buttonWithTitle:@"OK" target:buttons action:@selector(ok:)];
   ok.keyEquivalent = @"\r";
   cancel.keyEquivalent = @"\e";
   cancel.frame = NSMakeRect(20, 6, 90, 28);
   ok.frame = NSMakeRect(115, 6, 90, 28);
   [bar addSubview:cancel];
   [bar addSubview:ok];
   if ([panel respondsToSelector:@selector(setAccessoryView:)])
      [(id) panel setAccessoryView:bar];
   NSModalResponse r = [NSApp runModalForWindow:panel];
   [panel orderOut:nil];
   if ([panel respondsToSelector:@selector(setAccessoryView:)])
      [(id) panel setAccessoryView:nil];
   [bar release];
   [buttons release];
   return r == NSModalResponseOK;
}

}  // namespace

bool PickColor(Top *, uint32_t initial, uint32_t *chosen)
{
   Init();
   @autoreleasepool
   {
      NSColorPanel *panel = [NSColorPanel sharedColorPanel];
      panel.showsAlpha = NO;
      panel.color = [NSColor colorWithSRGBRed:(initial & 0xFF) / 255.0 green:((initial >> 8) & 0xFF) / 255.0
                                         blue:((initial >> 16) & 0xFF) / 255.0 alpha:1];
      if (!RunPanel(panel))
         return false;
      NSColor *rgb = [panel.color colorUsingColorSpace:NSColorSpace.sRGBColorSpace];
      auto channel = [](CGFloat v) { return (uint32_t) std::clamp((int) (v * 255.0 + 0.5), 0, 255); };
      *chosen = channel(rgb.redComponent) | (channel(rgb.greenComponent) << 8) | (channel(rgb.blueComponent) << 16);
      return true;
   }
}

bool PickFont(Top *, const FontSpec &initial, FontSpec *chosen)
{
   Init();
   @autoreleasepool
   {
      NSFontManager *manager = [NSFontManager sharedFontManager];
      NSString *face = [NSString stringWithUTF8String:initial.face.c_str()];
      CGFloat size = std::max(1, std::abs(initial.height));
      NSFont *start = face ? [NSFont fontWithName:face size:size] : nil;
      if (start == nil)
         start = [NSFont systemFontOfSize:size];
      NSFontTraitMask traits = (initial.weight >= 700 ? NSBoldFontMask : 0) | (initial.italic ? NSItalicFontMask : 0);
      if (traits)
         start = [manager convertFont:start toHaveTrait:traits];
      [manager setSelectedFont:start isMultiple:NO];
      NSFontPanel *panel = [manager fontPanel:YES];
      if (!RunPanel(panel))
         return false;
      NSFont *font = [manager convertFont:start];
      *chosen = initial;
      chosen->face = font.familyName.UTF8String ?: "";
      chosen->height = -(int) (font.pointSize + 0.5);
      NSFontTraitMask t = [manager traitsOfFont:font];
      chosen->weight = (t & NSBoldFontMask) ? 700 : 400;
      chosen->italic = (t & NSItalicFontMask) != 0;
      return true;
   }
}

void SetDarkTheme(bool dark)
{
   Init();
   NSApp.appearance = [NSAppearance appearanceNamed:dark ? NSAppearanceNameDarkAqua : NSAppearanceNameAqua];
}

}  // namespace w32::mac
