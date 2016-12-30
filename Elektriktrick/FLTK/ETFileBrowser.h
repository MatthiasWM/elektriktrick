//
//  ETFileBrowser.h
//  Elektriktrick
//
//  Created by Matthias Melcher on 12/25/16.
//  Copyright © 2016 M.Melcher GmbH. All rights reserved.
//

#ifndef ETFileBrowser_h
#define ETFileBrowser_h

#include <FL/Fl_File_Browser.H>


class ETFileBrowser : public Fl_File_Browser
{
public:
    ETFileBrowser(int x, int y, int w, int h, const char *title=0);
    ~ETFileBrowser();
    void file_select_callback(void (*v)(const char *)) { pFileSelectCB = v; }
    void directory_select_callback(void (*v)(const char *)) { pDirectorySelectCB = v; }
    void directory_up_callback(void (*v)(const char *)) { pDirectoryUpCB = v; }
    void directory_filter(const char *filter);
    int load(const char *path);
    void do_file_select_callback();
    void do_directory_select_callback();
    void do_directory_up_callback();
    const char *full_path();
private:
    int handle(int event);
    char *pDirFilter;
    char *pDirectory;
    void (*pFileSelectCB)(const char *);
    void (*pDirectorySelectCB)(const char *);
    void (*pDirectoryUpCB)(const char *);
    void pCallback();
    static void pCallback(Fl_Widget*, void*);
};


#endif /* ETFileBrowser_h */
