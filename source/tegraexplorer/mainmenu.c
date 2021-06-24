#include "mainmenu.h"
#include "../gfx/gfx.h"
#include "../gfx/gfxutils.h"
#include "../gfx/menu.h"
#include "tools.h"
#include "../hid/hid.h"
#include "../fs/menus/explorer.h"
#include <utils/btn.h>
#include <storage/nx_sd.h>
#include "tconf.h"
#include "../keys/keys.h"
#include "../storage/mountmanager.h"
#include "../storage/gptmenu.h"
#include "../storage/emummc.h"
#include <utils/util.h>
#include "../fs/fsutils.h"
#include <soc/fuse.h>
#include "../utils/utils.h"
#include "../config.h"

#include "../fs/readers/folderReader.h"
#include "../fs/fstypes.h"
#include "../fs/fscopy.h"
#include <utils/sprintf.h>
#include <stdlib.h>
#include <string.h>

#include "../cpr/cpr.h"

extern hekate_config h_cfg;

enum {
    MainExplore = 0,
    DeleteBootFlags,
    DeleteThemes,
    FixClingWrap,
    FixAIOUpdaterBoot,
    FixArchiveBitA,
    FixArchiveBitN,
    // FixMacSpecialFolders,
    FixAll,
    MainOther,
    MainViewStillNoBootInfo,
    MainViewCredits,
    MainExit,
    MainPowerOff,
    MainRebootRCM,
    // MainRebootNormal,
    MainRebootHekate,
    MainRebootAMS,
};

MenuEntry_t mainMenuEntries[] = {
    [MainExplore] = {.optionUnion = COLORTORGB(COLOR_WHITE) | SKIPBIT, .name = "-- Tools --"},
    [DeleteBootFlags] = {.optionUnion = COLORTORGB(COLOR_GREEN), .name = "Disable automatic sysmodule startup"},
    [DeleteThemes] = {.optionUnion = COLORTORGB(COLOR_GREEN), .name = "Delete installed themes"},
    [FixClingWrap] = {.optionUnion = COLORTORGB(COLOR_GREEN), .name = "Fix ClingWrap"},
    [FixAIOUpdaterBoot] = {.optionUnion = COLORTORGB(COLOR_GREEN), .name = "Fix Switch-AiO-Updater update"},
    [FixArchiveBitA] = {.optionUnion = COLORTORGB(COLOR_GREEN), .name = "Fix archive bit (all folders except nintendo)"},
    [FixArchiveBitN] = {.optionUnion = COLORTORGB(COLOR_GREEN), .name = "Fix archive bit (nintendo folder)"},
    // [FixMacSpecialFolders] = {.optionUnion = COLORTORGB(COLOR_GREEN), .name = "Remove special folders created by Mac"},
    [FixAll] = {.optionUnion = COLORTORGB(COLOR_GREEN), .name = "Try everything"},
    [MainOther] = {.optionUnion = COLORTORGB(COLOR_GREEN) | SKIPBIT, .name = "\n-- Other --"},


    
    
    // [MainBrowseSd] = {.optionUnion = COLORTORGB(COLOR_GREEN), .name = "Browse SD"},
    // [MainMountSd] = {.optionUnion = COLORTORGB(COLOR_YELLOW)}, // To mount/unmount the SD
    // [MainBrowseEmmc] = {.optionUnion = COLORTORGB(COLOR_BLUE), .name = "Browse EMMC"},
    // [MainBrowseEmummc] = {.optionUnion = COLORTORGB(COLOR_BLUE), .name = "Browse EMUMMC"},
    // [MainTools] = {.optionUnion = COLORTORGB(COLOR_WHITE) | SKIPBIT, .name = "\n-- Tools --"},
    // [MainPartitionSd] = {.optionUnion = COLORTORGB(COLOR_ORANGE), .name = "Partition the sd"},
    // [MainDumpFw] = {.optionUnion = COLORTORGB(COLOR_BLUE), .name = "Dump Firmware"},
    // [MainViewKeys] = {.optionUnion = COLORTORGB(COLOR_YELLOW), .name = "View dumped keys"},
    [MainOther] = {.optionUnion = COLORTORGB(COLOR_WHITE) | SKIPBIT, .name = "\n-- Other --"},
    [MainViewStillNoBootInfo] = {.optionUnion = COLORTORGB(COLOR_YELLOW), .name = "My switch still does not boot"},
    [MainViewCredits] = {.optionUnion = COLORTORGB(COLOR_YELLOW), .name = "Credits"},
    [MainExit] = {.optionUnion = COLORTORGB(COLOR_WHITE) | SKIPBIT, .name = "\n-- Exit --"},
    [MainPowerOff] = {.optionUnion = COLORTORGB(COLOR_VIOLET), .name = "Power off"},
    [MainRebootRCM] = {.optionUnion = COLORTORGB(COLOR_VIOLET), .name = "Reboot to RCM"},
    // [MainRebootNormal] = {.optionUnion = COLORTORGB(COLOR_VIOLET), .name = "Reboot normally"},
    [MainRebootHekate] = {.optionUnion = COLORTORGB(COLOR_VIOLET), .name = "Reboot to bootloader/update.bin"},
    [MainRebootAMS] = {.optionUnion = COLORTORGB(COLOR_VIOLET), .name = "Reboot to atmosphere/reboot_payload.bin"}
};

void HandleSD(){
    gfx_clearscreen();
    TConf.curExplorerLoc = LOC_SD;
    if (!sd_mount() || sd_get_card_removed()){
        gfx_printf("Sd is not mounted!");
        hidWait();
    }
    else
        FileExplorer("sd:/");
}


extern bool sd_mounted;
extern bool is_sd_inited;
extern int launch_payload(char *path);

void RebootToAMS(){
    launch_payload("sd:/atmosphere/reboot_payload.bin");
}

void RebootToHekate(){
    launch_payload("sd:/bootloader/update.bin");
}

void MountOrUnmountSD(){
    gfx_clearscreen();
    if (sd_mounted)
        sd_unmount();
    else if (!sd_mount())
        hidWait();
}


void archBitHelperA(){
    m_entry_fixArchiveBit(0);
}
void archBitHelperN(){
    m_entry_fixArchiveBit(1);
}


menuPaths mainMenuPaths[] = {
    [DeleteBootFlags] = m_entry_deleteBootFlags,
    [DeleteThemes] = m_entry_deleteInstalledThemes,
    [FixClingWrap] = m_entry_fixClingWrap,
    [FixAIOUpdaterBoot] = m_entry_fixAIOUpdate,
    [FixArchiveBitA] = archBitHelperA,
    [FixArchiveBitN] = archBitHelperN,
    // [FixMacSpecialFolders] = m_entry_fixMacSpecialFolders,
    [FixAll] = m_entry_fixAll,
    [MainViewStillNoBootInfo] = m_entry_stillNoBootInfo,
    [MainRebootHekate] = RebootToHekate,
    [MainRebootRCM] = reboot_rcm,
    [MainPowerOff] = power_off,
    [MainRebootAMS] = RebootToAMS,
    [MainViewCredits] = m_entry_ViewCredits,
};

void EnterMainMenu(){
    int res = 0;
    while (1){
        if (sd_get_card_removed())
            sd_unmount();

        // // -- Exit --
        mainMenuEntries[MainRebootAMS].hide = (!sd_mounted || !FileExists("sd:/atmosphere/reboot_payload.bin"));
        mainMenuEntries[MainRebootHekate].hide = (!sd_mounted || !FileExists("sd:/bootloader/update.bin"));
        mainMenuEntries[MainRebootRCM].hide = h_cfg.t210b01;

        gfx_clearscreen();
        gfx_putc('\n');
        
        Vector_t ent = vecFromArray(mainMenuEntries, ARR_LEN(mainMenuEntries), sizeof(MenuEntry_t));
        res = newMenu(&ent, res, 79, 30, ALWAYSREDRAW, 0);
        if (mainMenuPaths[res] != NULL)
            mainMenuPaths[res]();
    }
}
