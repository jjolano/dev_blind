// dev_blind - a (very simple) dev_flash write access mount enabler
// by @jjolano

#include <io/pad.h>
#include <io/msg.h>

#include <sysutil/events.h>

#include <lv2/process.h>

#include <psl1ght/lv2/filesystem.h>

#include "rsxutil.h"
#include "filesystem_mount.h"

const char* MOUNT_POINT = "/dev_blind"; // the path where dev_blind should be mounted to

int currentBuffer = 0;
msgButton dlg_action;

void handledialog(msgButton button, void *userdata)
{
	dlg_action = button;
}

void handleevent(u64 status, u64 param, void * userdata)
{
	if(status == EVENT_REQUEST_EXITAPP)
	{
		sysProcessExit(0);
	}
}

void showmessage(msgType type, const char* message)
{
	msgDialogOpen(type, message, handledialog, 0, NULL);
	
	dlg_action = 0;
	while(dlg_action == 0)
	{
		sysCheckCallback();
		
		flip(currentBuffer);
		waitFlip();
		currentBuffer = !currentBuffer;
	}
	
	msgDialogClose();
}

int main(int argc, const char* argv[])
{
	msgType mdialogok	= MSGDIALOG_NORMAL | MSGDIALOG_BUTTON_TYPE_OK;
	msgType mdialogyesno	= MSGDIALOG_NORMAL | MSGDIALOG_BUTTON_TYPE_YESNO;
	
	sysRegisterCallback(EVENT_SLOT0, handleevent, NULL);
	
	init_screen();
	ioPadInit(7);
	
	waitFlip();
	
	showmessage(mdialogyesno, "dev_blind v1.1 by jjolano (Twitter: @jjolano)\n\nThis program allows you to write to the flash memory (dev_flash) of your console.\nDO NOT USE THIS PROGRAM IF YOU DON'T KNOW EXACTLY WHAT YOU ARE DOING!\n\nCapiche?");
	
	if(dlg_action == MSGDIALOG_BUTTON_YES)
	{
		// see if dev_blind is mounted already
		Lv2FsStat entry;
		int is_mounted = lv2FsStat(MOUNT_POINT, &entry);
		
		showmessage(mdialogyesno, (is_mounted == 0) ? "Do you want to unmount dev_blind?" : "Do you want to mount dev_blind?");
		
		if(dlg_action == MSGDIALOG_BUTTON_YES)
		{
			if(is_mounted == 0)
			{
				// unmount dev_blind
				lv2FsUnmount(MOUNT_POINT);
				showmessage(mdialogok, "Successfully unmounted dev_blind.");
			}
			else
			{
				// mount dev_flash to dev_blind
				lv2FsMount(DEV_FLASH1, FS_FAT32, MOUNT_POINT, 0);
				showmessage(mdialogok, "Successfully mounted dev_blind.");
			}
		}
	}
	
	return 0;
}

