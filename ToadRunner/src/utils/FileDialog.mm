#ifdef __APPLE__
#import "pch.h"
#import <Cocoa/Cocoa.h>
#import "FileDialog.h"

namespace Toad
{

    std::string OpenFileDialogMac()
    {
        NSString *result = @"";

        NSOpenPanel *openPanel = [NSOpenPanel openPanel];
        [openPanel setCanChooseFiles:YES];
        [openPanel setCanChooseDirectories:NO];
        [openPanel setAllowsMultipleSelection:NO];
        [openPanel setCanCreateDirectories:YES];

        NSInteger clicked = [openPanel runModal];
        if (clicked == NSModalResponseOK) {
            NSURL *url = [openPanel URL];
            result = [url path];
        }

        return [result UTF8String];
    }
     
    std::vector<std::string> OpenMultipleFilesDialogMac()
    {
        NSString *result = @"";
        std::vector<std::string> files;

        NSOpenPanel *openPanel = [NSOpenPanel openPanel];
        [openPanel setCanChooseFiles:YES];
        [openPanel setCanChooseDirectories:NO];
        [openPanel setAllowsMultipleSelection:YES];
        [openPanel setCanCreateDirectories:YES];

        NSInteger clicked = [openPanel runModal];
        if (clicked == NSModalResponseOK)
        {
            NSArray<NSURL *> *urls = [openPanel URLs];
            for (NSURL *url in urls) 
            {
                files.push_back([url.path UTF8String]);
            }
        }

        return files;
    }

    std::string OpenFolderDialogMac()
    {
        NSString *result = @"";

        NSOpenPanel *openPanel = [NSOpenPanel openPanel];
        [openPanel setCanChooseFiles:NO];
        [openPanel setCanChooseDirectories:YES];
        [openPanel setAllowsMultipleSelection:NO];
        [openPanel setCanCreateDirectories:NO];
        [openPanel setCanCreateDirectories:YES];

        NSInteger clicked = [openPanel runModal];
        if (clicked == NSModalResponseOK) {
            NSURL *url = [openPanel URL];
            result = [url path];
        }

        return [result UTF8String];
    }

}
#endif