/*
 * top.mm:  Top-level windows on AppKit.  An NSWindow whose content view (flipped,
 *   so y grows down as on Windows) shows the window's GDI surface and turns
 *   AppKit's input and window events into the core's.
 */

#include "appkit/appkit_backend.h"


#include <algorithm>
#include <cstring>
#include <mutex>

using namespace w32::mac;
using w32::native::Top;

namespace {

bool keys_down[256];
bool keys_toggled[256];
bool swallowed_keys[256];   // pressed as the Mac's, not the game's (key:down:)

unsigned KeysFromFlags(NSEventModifierFlags flags)
{
   unsigned keys = 0;
   NSUInteger buttons = [NSEvent pressedMouseButtons];
   if (buttons & 1) keys |= MK_LBUTTON;
   if (buttons & 2) keys |= MK_RBUTTON;
   if (buttons & 4) keys |= MK_MBUTTON;
   if (flags & NSEventModifierFlagShift) keys |= MK_SHIFT;
   if (flags & NSEventModifierFlagControl) keys |= MK_CONTROL;
   return keys;
}

void Resize(Top *top, int width, int height)
{
   width = std::max(width, 1);
   height = std::max(height, 1);
   if (width == top->width && height == top->height)
      return;
   // Keep what was drawn, as the client may only repaint what's new.
   std::vector<uint8_t> next((size_t) width * height * 4, 0);
   for (int y = 0; y < std::min(height, top->height); y++)
      memcpy(&next[(size_t) y * width * 4], &top->pixels[(size_t) y * top->width * 4],
             (size_t) std::min(width, top->width) * 4);
   for (size_t i = 3; i < next.size(); i += 4)
      next[i] = 0xFF;
   top->pixels.swap(next);
   top->width = width;
   top->height = height;
}

// The surface's position from the window's content rect.
void TrackPosition(Top *top)
{
   NSRect content = [top->window contentRectForFrameRect:top->window.frame];
   w32::mac::Point p = FromAppKit(NSMakePoint(content.origin.x, content.origin.y + content.size.height));
   top->x = p.x;
   top->y = p.y;
}

}  // namespace

@interface M59View : NSView
{
@public
   Top *top;
   NSTrackingArea *tracking;
}
@end

@implementation M59View

- (BOOL)isFlipped
{
   return YES;
}

- (BOOL)acceptsFirstResponder
{
   return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)event
{
   return YES;
}

- (BOOL)isOpaque
{
   return YES;
}

- (void)drawRect:(NSRect)dirty
{
   if (top == nullptr || top->pixels.empty())
      return;
   CGContextRef cg = [NSGraphicsContext currentContext].CGContext;
   CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
   CGDataProviderRef data = CGDataProviderCreateWithData(nullptr, top->pixels.data(), top->pixels.size(), nullptr);
   // BGRA in memory: 32-bit little endian, alpha first, which the surface keeps opaque.
   CGImageRef image = CGImageCreate(top->width, top->height, 8, 32, top->width * 4, space,
                                    kCGBitmapByteOrder32Little | kCGImageAlphaNoneSkipFirst, data, nullptr, false,
                                    kCGRenderingIntentDefault);
   CGContextSaveGState(cg);
   CGContextClipToRect(cg, NSRectToCGRect(dirty));
   CGContextSetInterpolationQuality(cg, kCGInterpolationNone);
   // The view is flipped: draw the image upright.
   CGContextTranslateCTM(cg, 0, top->height);
   CGContextScaleCTM(cg, 1, -1);
   CGContextDrawImage(cg, CGRectMake(0, 0, top->width, top->height), image);
   CGContextRestoreGState(cg);
   CGImageRelease(image);
   CGDataProviderRelease(data);
   CGColorSpaceRelease(space);
}

- (void)setFrameSize:(NSSize)size
{
   [super setFrameSize:size];
   if (top == nullptr)
      return;
   Resize(top, (int) size.width, (int) size.height);
   OnResized(top->hwnd, top->width, top->height);
   UpdateGLView(top);
}

- (void)updateTrackingAreas
{
   if (tracking)
   {
      [self removeTrackingArea:tracking];
      [tracking release];
   }
   tracking = [[NSTrackingArea alloc]
      initWithRect:NSZeroRect
           options:NSTrackingMouseMoved | NSTrackingMouseEnteredAndExited | NSTrackingActiveAlways |
                   NSTrackingInVisibleRect
             owner:self
          userInfo:nil];
   [self addTrackingArea:tracking];
   [super updateTrackingAreas];
}

- (NSPoint)where:(NSEvent *)event
{
   return [self convertPoint:event.locationInWindow fromView:nil];
}

- (void)mouse:(NSEvent *)event button:(int)button down:(BOOL)down
{
   if (top == nullptr)
      return;
   NSPoint p = [self where:event];
   unsigned keys = KeysFromFlags(event.modifierFlags);
   if (button < 0)
      OnMouseMove(top->hwnd, (int) p.x, (int) p.y, keys);
   else
   {
      static const int vks[] = {VK_LBUTTON, VK_RBUTTON, VK_MBUTTON};
      static const unsigned bits[] = {MK_LBUTTON, MK_RBUTTON, MK_MBUTTON};
      keys_down[vks[button]] = down;
      // The button's own state from this event, whatever pressedMouseButtons says yet.
      keys = down ? (keys | bits[button]) : (keys & ~bits[button]);
      OnButton(top->hwnd, (Button) button, down, (int) p.x, (int) p.y, keys);
   }
   if (top && top->cursor && !top->cursor_hidden)
      [top->cursor set];
}

- (void)mouseDown:(NSEvent *)e { [self mouse:e button:0 down:YES]; }
- (void)mouseUp:(NSEvent *)e { [self mouse:e button:0 down:NO]; }
- (void)rightMouseDown:(NSEvent *)e { [self mouse:e button:1 down:YES]; }
- (void)rightMouseUp:(NSEvent *)e { [self mouse:e button:1 down:NO]; }
- (void)otherMouseDown:(NSEvent *)e { [self mouse:e button:2 down:YES]; }
- (void)otherMouseUp:(NSEvent *)e { [self mouse:e button:2 down:NO]; }
- (void)mouseMoved:(NSEvent *)e { [self mouse:e button:-1 down:NO]; }
- (void)mouseDragged:(NSEvent *)e { [self mouse:e button:-1 down:NO]; }
- (void)rightMouseDragged:(NSEvent *)e { [self mouse:e button:-1 down:NO]; }
- (void)otherMouseDragged:(NSEvent *)e { [self mouse:e button:-1 down:NO]; }

- (void)mouseExited:(NSEvent *)e
{
   if (top)
      OnMouseLeave(top->hwnd);
}

- (void)scrollWheel:(NSEvent *)e
{
   if (top == nullptr || e.scrollingDeltaY == 0)
      return;
   NSPoint p = [self where:e];
   // Precise (trackpad) deltas are points; a wheel notch is about one line.
   double notches = e.hasPreciseScrollingDeltas ? e.scrollingDeltaY / 10.0 : e.scrollingDeltaY;
   OnWheel(top->hwnd, (int) p.x, (int) p.y, KeysFromFlags(e.modifierFlags), (int) (notches * WHEEL_DELTA));
}

- (void)key:(NSEvent *)e down:(BOOL)down
{
   if (top == nullptr)
      return;
   int vk = KeyCodeToVk(e.keyCode);
   if (vk == 0)
      return;
   bool repeat = down && e.isARepeat;
   NSEventModifierFlags flags = e.modifierFlags;
   // Command is the Mac's, not a Windows key: Cmd+A, C, V, X and Z do in text
   // fields what Ctrl+ does on Windows, the menu takes Cmd+Q, the game sees no
   // other (its bindings stay on Control). Ctrl+ those in a text field is
   // nothing, as on a Mac. A key's release goes where its press went.
   bool edit_key = vk == 'A' || vk == 'C' || vk == 'V' || vk == 'X' || vk == 'Z';
   bool command = (flags & NSEventModifierFlagCommand) != 0;
   if (down ? command || ((flags & NSEventModifierFlagControl) && edit_key && TextFieldFocused())
            : swallowed_keys[vk])
   {
      if (down && !repeat)
         swallowed_keys[vk] = true;
      else if (!down)
         swallowed_keys[vk] = false;
      if (down && command && edit_key)
         OnTextShortcut(top->hwnd, vk, e.keyCode);
      return;
   }
   keys_down[vk] = down;
   char text[16] = "";
   if (down && !(flags & (NSEventModifierFlagControl | NSEventModifierFlagOption | NSEventModifierFlagCommand)))
   {
      // The characters typed, unless they are a function key's private-use codes.
      NSString *chars = e.characters;
      if (chars.length && !([chars characterAtIndex:0] >= 0xF700 && [chars characterAtIndex:0] <= 0xF8FF))
         strlcpy(text, chars.UTF8String ? chars.UTF8String : "", sizeof(text));
      // Backspace types DEL here, BS on Windows.
      if (vk == VK_BACK)
         strlcpy(text, "\b", sizeof(text));
   }
   else if (down && (flags & NSEventModifierFlagControl) && vk >= 'A' && vk <= 'Z')
   {
      // Ctrl+letter types the control character, as on Windows.
      text[0] = (char) (vk - 'A' + 1);
      text[1] = 0;
   }
   bool alt = (flags & NSEventModifierFlagOption) != 0;
   OnKey(top->hwnd, down, vk, e.keyCode, repeat, alt, text);
}

- (void)keyDown:(NSEvent *)e { [self key:e down:YES]; }
- (void)keyUp:(NSEvent *)e { [self key:e down:NO]; }

// Modifier keys come as flag changes: which one, and whether it is now down.
- (void)flagsChanged:(NSEvent *)e
{
   if (top == nullptr)
      return;
   int vk = KeyCodeToVk(e.keyCode);
   if (vk == 0)
      return;
   NSEventModifierFlags f = e.modifierFlags;
   bool down;
   switch (vk)
   {
   case VK_LSHIFT: case VK_RSHIFT: down = (f & NSEventModifierFlagShift) != 0; break;
   case VK_LCONTROL: case VK_RCONTROL: down = (f & NSEventModifierFlagControl) != 0; break;
   case VK_LMENU: case VK_RMENU: down = (f & NSEventModifierFlagOption) != 0; break;
   case VK_CAPITAL:
      keys_toggled[VK_CAPITAL] = (f & NSEventModifierFlagCapsLock) != 0;
      down = true;
      OnKey(top->hwnd, true, VK_CAPITAL, e.keyCode, false, false, "");
      OnKey(top->hwnd, false, VK_CAPITAL, e.keyCode, false, false, "");
      return;
   default: return;
   }
   keys_down[vk] = down;
   // Windows reports the generic code, and keeps the generic key's state.
   int report = (vk == VK_LSHIFT || vk == VK_RSHIFT) ? VK_SHIFT : (vk == VK_LCONTROL || vk == VK_RCONTROL) ? VK_CONTROL
                : (vk == VK_LMENU || vk == VK_RMENU) ? VK_MENU : vk;
   if (report != vk)
      keys_down[report] = down;
   OnKey(top->hwnd, down, report, e.keyCode, false, report == VK_MENU && down, "");
}

- (void)resetCursorRects
{
   if (top && top->cursor && !top->cursor_hidden)
      [self addCursorRect:self.bounds cursor:top->cursor];
}

@end

@interface M59WindowDelegate : NSObject <NSWindowDelegate>
{
@public
   Top *top;
}
@end

@implementation M59WindowDelegate

- (BOOL)windowShouldClose:(NSWindow *)window
{
   OnCloseRequested(top->hwnd);
   return NO;   // the core destroys the window (DestroyWindow), or not
}

- (void)windowDidBecomeKey:(NSNotification *)n
{
   OnActivated(top->hwnd, true);
}

- (void)windowDidResignKey:(NSNotification *)n
{
   OnActivated(top->hwnd, false);
}

- (void)windowDidMove:(NSNotification *)n
{
   TrackPosition(top);
   OnMoved(top->hwnd, top->x, top->y);
}

- (void)windowDidMiniaturize:(NSNotification *)n
{
   OnMinimized(top->hwnd, true);
}

- (void)windowDidDeminiaturize:(NSNotification *)n
{
   OnMinimized(top->hwnd, false);
}

@end

// Windows without a title bar still take the keyboard.
@interface M59Window : NSWindow
@end

@implementation M59Window
- (BOOL)canBecomeKeyWindow
{
   return YES;
}
- (BOOL)canBecomeMainWindow
{
   return YES;
}
@end

@interface M59AppMenu : NSObject
@end

@implementation M59AppMenu
- (void)quit:(id)sender
{
   OnQuit();
}
@end

namespace w32::mac {

NSRect ToAppKit(int x, int y, int width, int height)
{
   CGFloat screen = NSScreen.screens.count ? NSScreen.screens[0].frame.size.height : 0;
   return NSMakeRect(x, screen - y - height, width, height);
}

Point FromAppKit(NSPoint p)
{
   CGFloat screen = NSScreen.screens.count ? NSScreen.screens[0].frame.size.height : 0;
   return {(int) p.x, (int) (screen - p.y)};
}

void Init()
{
   static std::once_flag once;
   std::call_once(once, [] {
      @autoreleasepool
      {
         [NSApplication sharedApplication];
         [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
         // The application menu, for Quit; the window's own menu bar is the core's.
         NSMenu *bar = [[NSMenu alloc] init];
         NSMenuItem *app = [[NSMenuItem alloc] init];
         [bar addItem:app];
         NSMenu *menu = [[NSMenu alloc] init];
         NSString *name = NSProcessInfo.processInfo.processName;
         // Quit closes the game's window, as its own Exit does, not the process.
         static M59AppMenu *target = [[M59AppMenu alloc] init];
         NSMenuItem *quit = [menu addItemWithTitle:[@"Quit " stringByAppendingString:name]
                                            action:@selector(quit:) keyEquivalent:@"q"];
         quit.target = target;
         app.submenu = menu;
         NSApp.mainMenu = bar;
         [menu release];
         [app release];
         [bar release];
         [NSApp finishLaunching];
         [NSApp activateIgnoringOtherApps:YES];
      }
   });
}

Top *CreateTop(void *hwnd, const TopStyle &style, int x, int y, int width, int height, Top *owner)
{
   Init();
   @autoreleasepool
   {
      auto *top = new Top;
      top->hwnd = hwnd;
      top->x = x;
      top->y = y;
      top->resizable = style.resizable;
      NSWindowStyleMask mask = 0;
      if (style.caption && !style.popup_menu_like)
      {
         mask |= NSWindowStyleMaskTitled | NSWindowStyleMaskClosable;
         if (style.minimize_box)
            mask |= NSWindowStyleMaskMiniaturizable;
         if (style.resizable)
            mask |= NSWindowStyleMaskResizable;
      }
      else
         mask = NSWindowStyleMaskBorderless;
      top->window = [[M59Window alloc] initWithContentRect:ToAppKit(x, y, std::max(width, 1), std::max(height, 1))
                                                 styleMask:mask
                                                   backing:NSBackingStoreBuffered
                                                     defer:NO];
      top->window.releasedWhenClosed = NO;
      top->window.acceptsMouseMovedEvents = YES;
      if (style.popup_menu_like || style.tool)
         top->window.level = owner ? owner->window.level : NSFloatingWindowLevel;
      if (!style.maximize_box)
         [top->window standardWindowButton:NSWindowZoomButton].enabled = NO;
      top->view = [[M59View alloc] initWithFrame:NSMakeRect(0, 0, std::max(width, 1), std::max(height, 1))];
      top->view->top = top;
      top->window.contentView = top->view;
      auto *delegate = [[M59WindowDelegate alloc] init];
      delegate->top = top;
      top->delegate = delegate;
      top->window.delegate = delegate;
      Resize(top, width, height);
      return top;
   }
}

void DestroyTop(Top *top)
{
   if (top == nullptr)
      return;
   @autoreleasepool
   {
      DestroyGLView(top);
      top->view->top = nullptr;
      ((M59WindowDelegate *) top->delegate)->top = nullptr;
      top->window.delegate = nil;
      [top->window orderOut:nil];
      [top->window close];
      [top->window release];
      [top->view release];
      [top->delegate release];
      [top->cursor release];
      delete top;
   }
}

void SetTitle(Top *top, const std::string &utf8)
{
   @autoreleasepool
   {
      top->window.title = [NSString stringWithUTF8String:utf8.c_str()] ?: @"";
   }
}

void SetGeometry(Top *top, int x, int y, int width, int height)
{
   top->x = x;
   top->y = y;
   NSRect frame = [top->window frameRectForContentRect:ToAppKit(x, y, std::max(width, 1), std::max(height, 1))];
   [top->window setFrame:frame display:YES];
   Resize(top, width, height);
}

void Show(Top *top, bool show)
{
   if (show)
      [top->window orderFront:nil];
   else
      [top->window orderOut:nil];
}

void SetMinimized(Top *top, bool minimized)
{
   if (minimized)
      [top->window miniaturize:nil];
   else
      [top->window deminiaturize:nil];
}

void SetMaximized(Top *top, bool maximized)
{
   if (maximized != (bool) top->window.zoomed)
      [top->window zoom:nil];
}

void Activate(Top *top)
{
   [NSApp activateIgnoringOtherApps:YES];
   [top->window makeKeyAndOrderFront:nil];
}

bool IsActive(Top *top)
{
   return top && top->window.isKeyWindow;
}

Surface GetSurface(Top *top)
{
   Surface s;
   s.pixels = top->pixels.data();
   s.width = top->width;
   s.height = top->height;
   s.stride = top->width * 4;
   return s;
}

void Present(Top *top, const Rect &r)
{
   [top->view setNeedsDisplayInRect:NSMakeRect(r.left, r.top, r.right - r.left, r.bottom - r.top)];
   GLViewDrawnOver(top, r);
}

Point GetCursorPos()
{
   return FromAppKit([NSEvent mouseLocation]);
}

bool SetCursorPos(int x, int y)
{
   CGWarpMouseCursorPosition(CGPointMake(x, y));
   // Warping freezes the pointer for a moment unless reassociated.
   CGAssociateMouseAndMouseCursorPosition(true);
   return true;
}

void ShowCursor(Top *top, bool show)
{
   if (top->cursor_hidden == !show)
      return;
   top->cursor_hidden = !show;
   if (show)
      [NSCursor unhide];
   else
      [NSCursor hide];
}

bool KeyDown(int vk)
{
   return vk >= 0 && vk < 256 && keys_down[vk];
}

bool KeyToggled(int vk)
{
   return vk >= 0 && vk < 256 && keys_toggled[vk];
}

Rect ScreenRect()
{
   Init();
   NSRect f = NSScreen.screens.count ? NSScreen.screens[0].frame : NSMakeRect(0, 0, 1440, 900);
   return {0, 0, (int) f.size.width, (int) f.size.height};
}

Rect WorkArea()
{
   Init();
   if (NSScreen.screens.count == 0)
      return ScreenRect();
   NSRect v = NSScreen.screens[0].visibleFrame;
   Point tl = FromAppKit(NSMakePoint(v.origin.x, v.origin.y + v.size.height));
   return {tl.x, tl.y, tl.x + (int) v.size.width, tl.y + (int) v.size.height};
}

void Beep()
{
   NSBeep();
}

void SetAppIcon(const std::vector<CursorSpec> &images)
{
   Init();
   @autoreleasepool
   {
      NSImage *image = nil;
      for (const CursorSpec &c : images)
      {
         NSBitmapImageRep *rep = [[NSBitmapImageRep alloc]
            initWithBitmapDataPlanes:nullptr
                          pixelsWide:c.width
                          pixelsHigh:c.height
                       bitsPerSample:8
                     samplesPerPixel:4
                            hasAlpha:YES
                            isPlanar:NO
                      colorSpaceName:NSDeviceRGBColorSpace
                        bitmapFormat:NSBitmapFormatAlphaNonpremultiplied
                         bytesPerRow:c.width * 4
                        bitsPerPixel:32];
         uint8_t *d = rep.bitmapData;
         for (size_t i = 0; i < (size_t) c.width * c.height; i++)
         {
            uint32_t p = c.pixels[i];
            d[i * 4] = (uint8_t) (p >> 16);
            d[i * 4 + 1] = (uint8_t) (p >> 8);
            d[i * 4 + 2] = (uint8_t) p;
            d[i * 4 + 3] = (uint8_t) (p >> 24);
         }
         if (image == nil || c.width > image.size.width)
         {
            [image release];
            image = [[NSImage alloc] initWithSize:NSMakeSize(c.width, c.height)];
         }
         [image addRepresentation:rep];
         [rep release];
      }
      if (image)
      {
         // The Dock shows the image whole, and Mac icons leave a margin round
         // their artwork (Apple's grid: 824 of 1024 points): so does this one,
         // its pixels scaled as they are.
         NSImage *icon = [NSImage imageWithSize:NSMakeSize(1024, 1024) flipped:NO
                                 drawingHandler:^BOOL(NSRect) {
                                    NSGraphicsContext.currentContext.imageInterpolation = NSImageInterpolationNone;
                                    [image drawInRect:NSMakeRect(100, 100, 824, 824) fromRect:NSZeroRect
                                            operation:NSCompositingOperationSourceOver fraction:1];
                                    return YES;
                                 }];
         NSApp.applicationIconImage = icon;
         [image release];
      }
   }
}

void SetCursor(Top *top, const CursorSpec &spec)
{
   const CursorSpec *c = &spec;
   @autoreleasepool
   {
      NSCursor *nc = nil;
      if (!c->standard.empty())
      {
         const std::string &s = c->standard;
         nc = s == "text"          ? NSCursor.IBeamCursor
              : s == "crosshair"   ? NSCursor.crosshairCursor
              : s == "pointer"     ? NSCursor.pointingHandCursor
              : s == "ew-resize"   ? NSCursor.resizeLeftRightCursor
              : s == "ns-resize"   ? NSCursor.resizeUpDownCursor
              : s == "not-allowed" ? NSCursor.operationNotAllowedCursor
              : s == "move"        ? NSCursor.openHandCursor
                                   : NSCursor.arrowCursor;
         [nc retain];
      }
      else
      {
         // An image: straight-alpha ARGB to a non-premultiplied RGBA bitmap.
         NSBitmapImageRep *rep = [[NSBitmapImageRep alloc]
            initWithBitmapDataPlanes:nullptr
                          pixelsWide:c->width
                          pixelsHigh:c->height
                       bitsPerSample:8
                     samplesPerPixel:4
                            hasAlpha:YES
                            isPlanar:NO
                      colorSpaceName:NSDeviceRGBColorSpace
                        bitmapFormat:NSBitmapFormatAlphaNonpremultiplied
                         bytesPerRow:c->width * 4
                        bitsPerPixel:32];
         uint8_t *d = rep.bitmapData;
         for (size_t i = 0; i < (size_t) c->width * c->height; i++)
         {
            uint32_t p = c->pixels[i];
            d[i * 4] = (uint8_t) (p >> 16);
            d[i * 4 + 1] = (uint8_t) (p >> 8);
            d[i * 4 + 2] = (uint8_t) p;
            d[i * 4 + 3] = (uint8_t) (p >> 24);
         }
         NSImage *image = [[NSImage alloc] initWithSize:NSMakeSize(c->width, c->height)];
         [image addRepresentation:rep];
         nc = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(c->hot_x, c->hot_y)];
         [image release];
         [rep release];
      }
      [top->cursor release];
      top->cursor = nc;
      if (!top->cursor_hidden)
      {
         [nc set];
         [top->window invalidateCursorRectsForView:top->view];
      }
   }
}

}  // namespace w32::mac
