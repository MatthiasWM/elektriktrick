//
//  ETFileBrowser.cpp
//  Elektriktrick
//
//  Created by Matthias Melcher on 12/25/16.
//  Copyright © 2016 M.Melcher GmbH. All rights reserved.
//

#include "ETFileBrowser.h"

#include <FL/filename.h>

#include <stdlib.h>


static int numericsort(struct dirent **A, struct dirent **B, int cs) {
    const char* a = (*A)->d_name;
    const char* b = (*B)->d_name;
    int ret = 0;
    for (;;) {
        if (isdigit(*a & 255) && isdigit(*b & 255)) {
            int diff,magdiff;
            while (*a == '0') a++;
            while (*b == '0') b++;
            while (isdigit(*a & 255) && *a == *b) {a++; b++;}
            diff = (isdigit(*a & 255) && isdigit(*b & 255)) ? *a - *b : 0;
            magdiff = 0;
            while (isdigit(*a & 255)) {magdiff++; a++;}
            while (isdigit(*b & 255)) {magdiff--; b++;}
            if (magdiff) {ret = magdiff; break;} /* compare # of significant digits*/
            if (diff) {ret = diff; break;}	/* compare first non-zero digit */
        } else {
            if (cs) {
                /* compare case-sensitive */
                if ((ret = *a-*b)) break;
            } else {
                /* compare case-insensitve */
                if ((ret = tolower(*a & 255)-tolower(*b & 255))) break;
            }
            
            if (!*a) break;
            a++; b++;
        }
    }
    if (!ret) return 0;
    else return (ret < 0) ? -1 : 1;
}


ETFileBrowser::ETFileBrowser(int x, int y, int w, int h, const char *title)
:   pDirFilter(0L),
    pDirectory(strdup("/")),
    Fl_File_Browser(x, y, w, h, title)
{
    callback(pCallback);
}


ETFileBrowser::~ETFileBrowser()
{
    if (pDirFilter) free(pDirFilter);
    if (pDirectory) free(pDirectory);
}


//fl_numericsort
int fl_numericsort(struct dirent **A, struct dirent **B) {
    return numericsort(A, B, 0);
}


int ETFileBrowser::load(const char *path)
{
    int		i;				// Looping var
    int		num_files;			// Number of files in directory
    int		num_dirs;			// Number of directories in list
    char		filename[4096];			// Current file
    Fl_File_Icon	*icon;				// Icon to use

    clear();
    if (pDirectory)
        free(pDirectory);
    if (!path) {
        pDirectory = strdup("/");
        return 0;
    }
    if (!path[0]) {
        pDirectory = strdup("/");
        Fl_File_Browser::load(path);
    } else {
        pDirectory = strdup(path);
        dirent	**files;	// Files in in directory


        //
        // Build the file list...
        //

#if (defined(WIN32) && !defined(__CYGWIN__)) || defined(__EMX__)
        strlcpy(filename, directory_, sizeof(filename));
        i = (int) (strlen(filename) - 1);

        if (i == 2 && filename[1] == ':' &&
            (filename[2] == '/' || filename[2] == '\\'))
            filename[2] = '/';
        else if (filename[i] != '/' && filename[i] != '\\')
            strlcat(filename, "/", sizeof(filename));

        num_files = fl_filename_list(filename, &files, fl_numericsort);
#else
        num_files = fl_filename_list(path, &files, fl_numericsort);
#endif /* WIN32 || __EMX__ */

        if (num_files <= 0)
            return (0);

        for (i = 0, num_dirs = 0; i < num_files; i ++) {
            if (strcmp(files[i]->d_name, "./")) {
                snprintf(filename, sizeof(filename), "%s/%s", path,
                         files[i]->d_name);

                icon = Fl_File_Icon::find(filename);
                if ((icon && icon->type() == Fl_File_Icon::DIRECTORY) || _fl_filename_isdir_quick(filename)) {
                    if (pDirFilter==0 || strcmp(files[i]->d_name, "../")==0 || fl_filename_match(files[i]->d_name, pDirFilter)) {
                        num_dirs ++;
#ifdef __APPLE__
                        add(files[i]->d_name, icon); // Apple keeps directories as part of the file list
#else
                        insert(num_dirs, files[i]->d_name, icon);
#endif
                    }
                } else if (filetype() == FILES && fl_filename_match(files[i]->d_name, filter())) {
                    add(files[i]->d_name, icon);
                }
            }
            
            free(files[i]);
        }
        
        free(files);
    }
    return num_files;
}


int ETFileBrowser::handle(int event)
{
    const char *path;
    switch (event) {
        case FL_MOUSEWHEEL:
            Fl_File_Browser::handle(event);
            return 1;
        case FL_KEYBOARD:
            if (Fl::event_state(FL_SHIFT|FL_ALT|FL_META|FL_CTRL)==0) {
                switch (Fl::event_key()) {
                    case FL_Left: do_directory_up_callback(); return 1;
                    case FL_Right:
                        path = full_path();
                        if (path) {
                            if (fl_filename_isdir(path)) do_directory_select_callback();
                        }
                        return 1;
                    case FL_Enter:
                    case ' ':
                        do_callback();
                        return 1;
                }
            }
        // TODO: arrow to the left should go up one directory and select the directory we came from
        // TODO: starting to type text could select files starting with that text
    }
    return Fl_File_Browser::handle(event);
}


void ETFileBrowser::directory_filter(const char *v)
{
    if (pDirFilter) free(pDirFilter);
    pDirFilter = 0L;
    if (v) pDirFilter = strdup(v);
}


void ETFileBrowser::pCallback()
{
    int selected = value();
    if (selected) {
        const char *name = text(selected);
        char path[FL_PATH_MAX];
        strcpy(path, pDirectory);
        strcat(path, text(selected));
        if (fl_filename_isdir(path)) {
            // directory is changed by doubleklick, spacekey, leftarrow or rightarrow
            switch (Fl::event()) {
                case FL_PUSH:
                case FL_RELEASE:
                    if (Fl::event_button()==1 && Fl::event_clicks()==1)
                        do_directory_select_callback();
                    break;
                case FL_KEYBOARD:
                    switch (Fl::event_key()) {
                        case FL_Left:
                        case FL_Right:
                        case FL_Enter:
                        case ' ':
                            do_directory_select_callback();
                            break;
                    }
                    break;
            }
        } else {
            // filename is changed by any event, including up and down arrow
            do_file_select_callback();
        }
    }
}


void ETFileBrowser::pCallback(Fl_Widget *w, void*)
{
    ETFileBrowser *This;
    This = (ETFileBrowser*)w;
    This->pCallback();
}


void ETFileBrowser::do_file_select_callback()
{
    int selected = value();
    if (selected && pFileSelectCB) {
        pFileSelectCB(text(selected));
    }
}


void ETFileBrowser::do_directory_select_callback()
{
    int selected = value();
    if (selected && pDirectorySelectCB) {
        pDirectorySelectCB(text(selected));
    }
}


void ETFileBrowser::do_directory_up_callback()
{
    if (pDirectoryUpCB) {
        char buf[FL_PATH_MAX];
        strcpy(buf, pDirectory);
        strcat(buf, "../");
        pDirectoryUpCB(buf);
    }
}


const char *ETFileBrowser::full_path()
{
    static char buf[FL_PATH_MAX];
    int selected = value();
    if (selected) {
        strcpy(buf, pDirectory);
        strcat(buf, text(selected));
        return buf;
    } else {
        buf[0] = 0;
        return 0;
    }
}

