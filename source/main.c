#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include <io/pad.h>
#include <io/msg.h>

#include <sysmodule/sysmodule.h>

#include <sysutil/events.h>

#include <psl1ght/lv2.h>
#include <psl1ght/lv2/filesystem.h>

#include "rsxutil.h"

int currentBuffer = 0;
msgButton dlg_action = 0;

void flipscreen()
{
	flip(currentBuffer); // Flip buffer onto screen
	waitFlip(); // Wait for the last flip to finish, so we can draw to the old buffer
	currentBuffer = !currentBuffer;
}

void handledialog(msgButton button, void *userdata)
{
	dlg_action = button;
}

int direxists(const char* path)
{
	Lv2FsFile fd;
	int ret = lv2FsOpenDir(path, &fd);
	lv2FsCloseDir(fd);
	return ret;
}

int showmessage(msgType type, const char* message)
{
	msgDialogOpen(type, message, handledialog, 0, NULL);
	
	dlg_action = 0;
	while(!dlg_action)
	{
		sysCheckCallback();
		flipscreen();
	}
	
	msgDialogClose();
	
	return dlg_action;
}

int main(int argc, const char* argv[])
{
	msgType mdialogok	= MSGDIALOG_NORMAL | MSGDIALOG_BUTTON_TYPE_OK;
	msgType mdialogyesno	= MSGDIALOG_NORMAL | MSGDIALOG_BUTTON_TYPE_YESNO;
	
	init_screen();
	ioPadInit(7);
	
	waitFlip();
	
	showmessage(mdialogok, "dev_blind mounter created by @jjolano");
	
	int is_mounted = direxists("/dev_blind");
	int button = showmessage(mdialogyesno, (is_mounted == 0) ? "/dev_blind is already mounted - unmount?" : "Mount writable dev_flash to /dev_blind?");
	
	if(button == MSGDIALOG_BUTTON_OK)
	{
		if(is_mounted == 0)
		{
			// unmount /dev_blind
			int ret = Lv2Syscall1(838, (u64)"/dev_blind");
			showmessage(mdialogok, (ret == 0) ? "Successfully unmounted /dev_blind." : "Failed to unmount /dev_blind.");
		}
		else
		{
			// mount /dev_flash to /dev_blind
			int ret = Lv2Syscall8(837, (u64)"CELL_FS_IOS:BUILTIN_FLSH1", (u64)"CELL_FS_FAT", (u64)"/dev_blind", 0, 0, 0, 0, 0);
			showmessage(mdialogok, (ret == 0) ? "Successfully mounted writable dev_flash to /dev_blind.\nBe careful out there!" : "Failed to mount writable dev_flash to /dev_blind.");
		}
	}
	
	return 0;
}

