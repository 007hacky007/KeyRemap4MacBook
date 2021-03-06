#import "ConfigXMLParser.h"
#import "KeyRemap4MacBookKeys.h"
#import "KeyRemap4MacBookNSDistributedNotificationCenter.h"
#import "bridge.h"

static ConfigXMLParser* global_instance = nil;

@interface ConfigXMLParser (AutoGen)
- (BOOL) reload_autogen;
@end

@interface ConfigXMLParser (PreferencePane)
- (BOOL) reload_preferencepane;
@end

@implementation ConfigXMLParser

+ (ConfigXMLParser*) getInstance
{
  @synchronized(self) {
    if (! global_instance) {
      global_instance = [ConfigXMLParser new];
    }
  }
  return global_instance;
}

- (id) init
{
  self = [super init];

  if (self) {
    [self reload];
  }

  return self;
}

- (void) dealloc
{
  [dict_config_name_ release];
  [remapclasses_initialize_vector_ release];
  [preferencepane_checkbox_ release];
  [preferencepane_number_ release];
  [keycode_ release];
  [error_message_ release];

  [super dealloc];
}

// ------------------------------------------------------------
- (BOOL) initialized {
  return initialized_;
}

- (BOOL) reload {
  @synchronized(self) {
    initialized_ = NO;
    [self removeErrorMessage];

    if ([self reload_autogen] &&
        [self reload_preferencepane]) {
      initialized_ = YES;
    }
  }

  // We need to send a notification outside synchronized block to prevent lock.
  if (initialized_) {
    [org_pqrs_KeyRemap4MacBook_NSDistributedNotificationCenter postNotificationName:kKeyRemap4MacBookConfigXMLReloadedNotification userInfo:nil];
  }

  return initialized_;
}

- (NSUInteger) count
{
  NSUInteger v = 0;
  @synchronized(self) {
    if (initialized_) {
      v = [remapclasses_initialize_vector_ countOfVectors];
    }
  }
  return v;
}

- (RemapClassesInitializeVector*) remapclasses_initialize_vector
{
  RemapClassesInitializeVector* v = nil;
  @synchronized(self) {
    if (initialized_) {
      v = remapclasses_initialize_vector_;
      [[v retain] autorelease];
    }
  }
  return v;
}

- (unsigned int) keycode:(NSString*)name
{
  unsigned int v = 0;
  @synchronized(self) {
    if (initialized_) {
      v = [keycode_ unsignedIntValue:name];
    }
  }
  return v;
}

- (NSString*) configname:(unsigned int)configindex
{
  NSString* name = nil;
  @synchronized(self) {
    if (initialized_) {
      name = [dict_config_name_ objectForKey:[NSNumber numberWithUnsignedInt:configindex]];
    }
  }
  return name;
}

- (NSArray*) preferencepane_checkbox
{
  NSArray* a = nil;
  @synchronized(self) {
    if (initialized_) {
      a = preferencepane_checkbox_;
    }
  }
  return a;
}

- (NSArray*) preferencepane_number
{
  NSArray* a = nil;
  @synchronized(self) {
    if (initialized_) {
      a = preferencepane_number_;
    }
  }
  return a;
}

- (NSString*) preferencepane_error_message
{
  return error_message_;
}

- (NSString*) preferencepane_get_private_xml_path
{
  NSFileManager* filemanager = [NSFileManager defaultManager];
  NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
  NSString* path = [paths objectAtIndex:0];
  path = [path stringByAppendingPathComponent:@"KeyRemap4MacBook"];
  if (! [filemanager fileExistsAtPath:path]) {
    [filemanager createDirectoryAtPath:path withIntermediateDirectories:YES attributes:nil error:NULL];
  }

  path = [path stringByAppendingPathComponent:@"private.xml"];
  if (! [filemanager fileExistsAtPath:path]) {
    [filemanager copyItemAtPath:[[NSBundle mainBundle] pathForResource:@"private" ofType:@"xml"] toPath:path error:NULL];
  }

  BOOL isDirectory;
  if ([filemanager fileExistsAtPath:path isDirectory:&isDirectory] && ! isDirectory) {
    return path;
  }

  return @"";
}

@end
