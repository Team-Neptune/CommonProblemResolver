// #include <utils/util.h>
// #include "tools.h"
#include <storage/nx_sd.h>
#include "../fs/readers/folderReader.h"
#include "../fs/fstypes.h"
#include "../fs/fscopy.h"
#include <utils/sprintf.h>
#include <stdlib.h>
#include <string.h>
#include "../gfx/gfx.h"
#include "../gfx/gfxutils.h"
#include "../gfx/menu.h"
#include "../hid/hid.h"
// #include "utils.h"
#include "../utils/utils.h"
#include "../fs/fsutils.h"

void _DeleteFileSimple(char *thing){
    //char *thing = CombinePaths(path, entry.name);
    int res = f_unlink(thing);
    if (res)
        DrawError(newErrCode(res));
    free(thing);
}
void _RenameFileSimple(char *sourcePath, char *destPath){
    int res = f_rename(sourcePath, destPath);
    if (res){
        DrawError(newErrCode(res));
    }
}

int _fix_attributes(char *path, u32 *total, u32 hos_folder, u32 check_first_run){
	FRESULT res;
	DIR dir;
	u32 dirLength = 0;
	static FILINFO fno;

	if (check_first_run)
	{
		// Read file attributes.
		res = f_stat(path, &fno);
		if (res != FR_OK)
			return res;

		// Check if archive bit is set.
		if (fno.fattrib & AM_ARC)
		{
			*(u32 *)total = *(u32 *)total + 1;
			f_chmod(path, 0, AM_ARC);
		}
	}

	// Open directory.
	res = f_opendir(&dir, path);
	if (res != FR_OK)
		return res;

	dirLength = strlen(path);
	for (;;)
	{
		// Clear file or folder path.
		path[dirLength] = 0;

		// Read a directory item.
		res = f_readdir(&dir, &fno);

		// Break on error or end of dir.
		if (res != FR_OK || fno.fname[0] == 0)
			break;

		// Skip official Nintendo dir if started from root.
		if (!hos_folder && !strcmp(fno.fname, "Nintendo"))
			continue;

		// Set new directory or file.
		memcpy(&path[dirLength], "/", 1);
		memcpy(&path[dirLength + 1], fno.fname, strlen(fno.fname) + 1);

		// Check if archive bit is set.
		if (fno.fattrib & AM_ARC)
		{
			*total = *total + 1;
			f_chmod(path, 0, AM_ARC);
		}

		// Is it a directory?
		if (fno.fattrib & AM_DIR)
		{
			// Set archive bit to NCA folders.
			if (hos_folder && !strcmp(fno.fname + strlen(fno.fname) - 4, ".nca"))
			{
				*total = *total + 1;
				f_chmod(path, AM_ARC, AM_ARC);
			}

			// Enter the directory.
			res = _fix_attributes(path, total, hos_folder, 0);
			if (res != FR_OK)
				break;
		}
	}

	f_closedir(&dir);

	return res;
}


void m_entry_fixArchiveBit(u32 type){
    gfx_clearscreen();
    gfx_printf("\n\n-- Fix Archive Bits\n\n");

    char path[256];
	char label[16];

	u32 total = 0;
	if (sd_mount())
	{
		switch (type)
		{
		case 0:
			strcpy(path, "/");
			strcpy(label, "SD Card");
			break;
		case 1:
		default:
			strcpy(path, "/Nintendo");
			strcpy(label, "Nintendo folder");
			break;
		}

		gfx_printf("Traversing all %s files!\nThis may take some time...\n\n", label);
		_fix_attributes(path, &total, type, type);
		gfx_printf("%kTotal archive bits cleared: %d!%k", 0xFF96FF00, total, 0xFFCCCCCC);
		
        gfx_printf("\n\n Done, press a key to proceed.");
        hidWait();
	}
}


void m_entry_fixAIOUpdate(){
    gfx_clearscreen();
    gfx_printf("\n\n-- Fix broken Switch-AiO-Updater update.\n\n");

    char *aio_fs_path = CpyStr("sd:/atmosphere/fusee-secondary.bin.aio");
    char *aio_p_path = CpyStr("sd:/sept/payload.bin.aio");
    char *aio_strt_path = CpyStr("sd:/atmosphere/stratosphere.romfs.aio");

    char *o_fs_path = CpyStr("sd:/atmosphere/fusee-secondary.bin");
    char *o_p_path = CpyStr("sd:/sept/payload.bin");
    char *o_strt_path = CpyStr("sd:/atmosphere/stratosphere.romfs");

    if (FileExists(aio_fs_path)) {
        gfx_printf("Detected aio updated fusee-secondary file -> replacing original\n\n");
        if (FileExists(o_fs_path)) {
            _DeleteFileSimple(o_fs_path);
        }
        _RenameFileSimple(aio_fs_path, o_fs_path);
    }
    free(aio_fs_path);
    free(o_fs_path);

    if (FileExists(aio_p_path)) {
        gfx_printf("Detected aio updated paload file -> replacing original\n\n");
        if (FileExists(o_p_path)) {
            _DeleteFileSimple(o_p_path);
        }
        _RenameFileSimple(aio_p_path, o_p_path);
    }
    free(aio_p_path);
    free(o_p_path);

    if (FileExists(aio_strt_path)) {
        gfx_printf("Detected aio updated stratosphere file -> replacing original\n\n");
        if (FileExists(o_strt_path)) {
            _DeleteFileSimple(o_strt_path);
        }
        _RenameFileSimple(aio_strt_path, o_strt_path);
    }
    free(aio_strt_path);
    free(o_strt_path);


    gfx_printf("\n\n Done, press a key to proceed.");
    hidWait();
}

void m_entry_fixClingWrap(){
    gfx_clearscreen();
    gfx_printf("\n\n-- Fixing ClingWrap.\n\n");
    char *bpath = CpyStr("sd:/_b0otloader");
    char *bopath = CpyStr("sd:/bootloader");
    char *kpath = CpyStr("sd:/atmosphere/_k1ps");
    char *kopath = CpyStr("sd:/atmosphere/kips");

    char *ppath = CpyStr("sd:/bootloader/_patchesCW.ini");
    char *popath = CpyStr("sd:/atmosphere/patches.ini");

    if (FileExists(bpath)) {
        if (FileExists(bopath)) {
            FolderDelete(bopath);
        }
        int res = f_rename(bpath, bopath);
        if (res){
            DrawError(newErrCode(res));
        }
        gfx_printf("-- Fixed Bootloader\n");
    }

    if (FileExists(kpath)) {
        if (FileExists(kopath)) {
            FolderDelete(kopath);
        }
        int res = f_rename(kpath, kopath);
        if (res){
            DrawError(newErrCode(res));
        }
        gfx_printf("-- Fixed kips\n");
    }

    if (FileExists(ppath)) {
        if (FileExists(popath)) {
            _DeleteFileSimple(popath);
        }
        _RenameFileSimple(ppath,popath);
        gfx_printf("-- Fixed patches.ini\n");
    }

    free(bpath);
    free(bopath);
    free(kpath);
    free(kopath);
    free(ppath);
    free(popath);

    gfx_printf("\n\n Done, press a key to proceed.");
    hidWait();
}

void _deleteTheme(char* basePath, char* folderId){
    char *path = CombinePaths(basePath, folderId);
    if (FileExists(path)) {
        gfx_printf("-- Theme found: %s\n", path);
        FolderDelete(path);
    }
    free(path);
}

void m_entry_deleteInstalledThemes(){
    gfx_clearscreen();
    gfx_printf("\n\n-- Deleting installed themes.\n\n");
    _deleteTheme("sd:/atmosphere/contents", "0100000000001000");
    _deleteTheme("sd:/atmosphere/contents", "0100000000001007");
    _deleteTheme("sd:/atmosphere/contents", "0100000000001013");

    gfx_printf("\n\n Done, press a key to proceed.");
    hidWait();
}

void m_entry_deleteBootFlags(){
    gfx_clearscreen();
    gfx_printf("\n\n-- Disabling automatic sysmodule startup.\n\n");
    char *storedPath = CpyStr("sd:/atmosphere/contents");
    int readRes = 0;
    Vector_t fileVec = ReadFolder(storedPath, &readRes);
    if (readRes){
        clearFileVector(&fileVec);
        DrawError(newErrCode(readRes));
    } else {
        vecDefArray(FSEntry_t*, fsEntries, fileVec);
        for (int i = 0; i < fileVec.count; i++){

            char *suf = "/flags/boot2.flag";
            char *flagPath = CombinePaths(storedPath, fsEntries[i].name);
            flagPath = CombinePaths(flagPath, suf);

            if (FileExists(flagPath)) {
                gfx_printf("Deleting: %s\n", flagPath);
                _DeleteFileSimple(flagPath);
            }
            free(flagPath);
        }
    }
    gfx_printf("\n\n Done, press a key to proceed.");
    hidWait();
}

void m_entry_fixMacSpecialFolders(char *path){
    // browse path
    // list files & folders
    // if file -> delete
    // if folder !== nintendo
    //      if folder m_entry_fixMacSpecialFolders with new path
}

void m_entry_stillNoBootInfo(){
    gfx_clearscreen();
    gfx_printf("\n\n-- My switch still does not boot.\n\n");

    gfx_printf("%kDo you have a gamecard inserted?\n", COLOR_WHITE);
    gfx_printf("Try taking it out and reboot.\n\n");

    gfx_printf("%kDid you recently update Atmosphere/DeepSea?\n", COLOR_WHITE);
    gfx_printf("Insert your sdcard into a computer, delete 'atmosphere', 'bootloader' & 'sept', download your preffered CFW and put the files back on your switch.\n\n");

    gfx_printf("%kDid you just buy a new SD-card?\n", COLOR_WHITE);
    gfx_printf("Make sure its not a fake card.\n\n");

    gfx_printf("\n\n Done, press a key to proceed.");
    hidWait();
}

void m_entry_ViewCredits(){
    gfx_clearscreen();
    gfx_printf("\nCommon Problem Resolver v%d.%d.%d\nBy Team Neptune\n\nBased on TegraExplorer by SuchMemeManySkill,\nLockpick_RCM & Hekate, from shchmue & CTCaer\n\n\n", LP_VER_MJ, LP_VER_MN, LP_VER_BF);
    hidWait();
}

void m_entry_fixAll(){
    gfx_clearscreen();
    m_entry_deleteBootFlags();
    m_entry_deleteInstalledThemes();
    m_entry_fixClingWrap();
    m_entry_fixAIOUpdate();


    m_entry_stillNoBootInfo();
}
