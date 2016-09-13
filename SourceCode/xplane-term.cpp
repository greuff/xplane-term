/*
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMDataAccess.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPLMUtilities.h"
#include "XPWidgetUtils.h"
#include "XPLMDefs.h"
#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include <string>
#include <map>

XPLMHotKeyID	gHotKey = NULL;
std::map<std::string, XPLMDataRef> freqDataRefs;
XPLMDataRef transponderRef = NULL;
XPLMDataRef altitudeRef = NULL;
XPLMDataRef verticalSpeedRef = NULL;
XPLMDataRef headingRef = NULL;
XPLMDataRef courseRef = NULL;
XPLMDataRef barometerRef = NULL;
XPLMDataRef autopilotStateRef = NULL;
XPLMDataRef iasRef = NULL;

XPLMCommandRef flightDirCmdRef = NULL;
XPLMCommandRef autopilotCmdRef = NULL;
XPLMCommandRef headingSyncCmdRef = NULL;
XPLMCommandRef verticalSpeedCmdRef = NULL;
XPLMCommandRef approachCmdRef = NULL;
XPLMCommandRef flcCmdRef = NULL;
XPLMCommandRef altSyncCmdRef = NULL;
XPLMCommandRef iasSyncCmdRef = NULL;
XPLMCommandRef vsSyncCmdRef = NULL;
XPLMCommandRef altArmCmdRef = NULL;

void	MyHotKeyCallback(void *               inRefcon);
int	termWidgetHandler(
					  XPWidgetMessage			inMessage,
					  XPWidgetID				inWidget,
					  long					inParam1,
					  long					inParam2);
int	textFieldWidgetHandler(
					  XPWidgetMessage			inMessage,
					  XPWidgetID				inWidget,
					  long					inParam1,
					  long					inParam2);

XPWidgetID	termWidget = NULL;
XPWidgetID  textInputField = NULL;

/*
 * XPluginStart
 * 
 * Our start routine registers our window and does any other initialization we 
 * must do.
 * 
 */
PLUGIN_API int XPluginStart(
						char *		outName,
						char *		outSig,
						char *		outDesc)
{
	/* First we must fill in the passed in buffers to describe our
	 * plugin to the plugin-system. */

	strcpy(outName, "XPlaneTerm");
	strcpy(outSig, "at.wana.xplane.term");
	strcpy(outDesc, "A Terminal for X-Plane.");
	
	freqDataRefs["nav1"] = XPLMFindDataRef("sim/cockpit/radios/nav1_freq_hz");
	freqDataRefs["nav2"] = XPLMFindDataRef("sim/cockpit/radios/nav2_freq_hz");
	freqDataRefs["com1"] = XPLMFindDataRef("sim/cockpit/radios/com1_freq_hz");
	freqDataRefs["com2"] = XPLMFindDataRef("sim/cockpit/radios/com2_freq_hz");
	freqDataRefs["adf1"] = XPLMFindDataRef("sim/cockpit/radios/adf1_freq_hz");
	freqDataRefs["adf2"] = XPLMFindDataRef("sim/cockpit/radios/adf2_freq_hz");
	transponderRef = XPLMFindDataRef("sim/cockpit/radios/transponder_code");
	altitudeRef    = XPLMFindDataRef("sim/cockpit/autopilot/altitude");
	verticalSpeedRef = XPLMFindDataRef("sim/cockpit/autopilot/vertical_velocity");
	headingRef     = XPLMFindDataRef("sim/cockpit/autopilot/heading_mag");
	courseRef      = XPLMFindDataRef("sim/cockpit/radios/nav1_obs_degm");
	barometerRef   = XPLMFindDataRef("sim/cockpit/misc/barometer_setting");
	autopilotStateRef = XPLMFindDataRef("sim/cockpit/autopilot/autopilot_state");
	iasRef         = XPLMFindDataRef("sim/cockpit/autopilot/airspeed");
	
	flightDirCmdRef  = XPLMFindCommand("sim/autopilot/fdir_toggle");
	autopilotCmdRef = XPLMFindCommand("sim/autopilot/servos_toggle");
	headingSyncCmdRef = XPLMFindCommand("sim/autopilot/heading_sync");
	verticalSpeedCmdRef = XPLMFindCommand("sim/autopilot/vertical_speed");
	approachCmdRef = XPLMFindCommand("sim/autopilot/approach");
	flcCmdRef = XPLMFindCommand("sim/autopilot/level_change");
	altSyncCmdRef = XPLMFindCommand("sim/autopilot/altitude_sync");
	vsSyncCmdRef = XPLMFindCommand("sim/autopilot/vertical_speed_sync");
	iasSyncCmdRef = XPLMFindCommand("sim/autopilot/airspeed_sync");
	altArmCmdRef = XPLMFindCommand("sim/autopilot/altitude_arm");
	
	gHotKey = XPLMRegisterHotKey(XPLM_VK_TAB, xplm_DownFlag,
								 "X-Plane Terminal",
								 MyHotKeyCallback,
								 NULL);
	
	/* We must return 1 to indicate successful initialization, otherwise we
	 * will not be called back again. */
	return 1;
}

/*
 * XPluginStop
 * 
 * Our cleanup routine deallocates our window.
 * 
 */
PLUGIN_API void	XPluginStop(void)
{
	XPLMUnregisterHotKey(gHotKey);
	
	if(termWidget != NULL)
	{
		XPDestroyWidget(termWidget, 1);
		termWidget = NULL;
		
		// No need to release textInputField here because XPDestroyWidget runs recursively.
		textInputField = NULL;
	}
}

/*
 * XPluginDisable
 * 
 * We do not need to do anything when we are disabled, but we must provide the handler.
 * 
 */
PLUGIN_API void XPluginDisable(void)
{
}

/*
 * XPluginEnable.
 * 
 * We don't do any enable-specific initialization, but we must return 1 to indicate
 * that we may be enabled at this time.
 * 
 */
PLUGIN_API int XPluginEnable(void)
{
	return 1;
}

/*
 * XPluginReceiveMessage
 * 
 * We don't have to do anything in our receive message handler, but we must provide one.
 * 
 */
PLUGIN_API void XPluginReceiveMessage(
					XPLMPluginID	inFromWho,
					long			inMessage,
					void *			inParam)
{
}

/// Called when the user pressed the global plugin hotkey.
void MyHotKeyCallback(void *inRefcon)
{
//	XPLMSetDatai(gNav1DataRef, XPLMGetDatai(gNav1DataRef) + (long) 1000L);
	
	if(termWidget == NULL)
	{
		// Create the input widget
		int x = 100;
		int y = 100;
		int width = 300;
		int height = 50;
		
		termWidget = XPCreateWidget(x, y, x+width, y-height,
					   1,	// Visible
					   "X-Plane Terminal",	// desc
					   1,		// root
					   NULL,	// no container
					   xpWidgetClass_MainWindow);
		
		// Add Close Box decorations to the Main Widget
		XPSetWidgetProperty(termWidget, xpProperty_MainWindowHasCloseBoxes, 1);
		
		// Add the text field
		textInputField = XPCreateWidget(x+20, y-20, x+width-40, y-40, 1, "", 0, termWidget, xpWidgetClass_TextField);
		XPSetWidgetProperty(textInputField, xpProperty_TextFieldType, xpTextEntryField);
		
		// Register our widget handler
		XPAddWidgetCallback(termWidget, termWidgetHandler);
		XPAddWidgetCallback(textInputField, textFieldWidgetHandler);
		
		XPSetKeyboardFocus(textInputField);
	}
	else
	{
		if(!XPIsWidgetVisible(termWidget))
		{
			XPShowWidget(termWidget);
			
			XPSetWidgetDescriptor(textInputField, "");
			XPSetKeyboardFocus(textInputField);
		}
		else
		{
			XPHideWidget(termWidget);
			XPLoseKeyboardFocus(textInputField);
		}
	}
}

// This is the handler for our widget
// It can be used to process button presses etc.
// In this example we are only interested when the close box is pressed
int	termWidgetHandler(
					   XPWidgetMessage			inMessage,
					   XPWidgetID				inWidget,
					   long					inParam1,
					   long					inParam2)
{
	if (inMessage == xpMessage_CloseButtonPushed)
	{
		if (termWidget != NULL)
		{
			XPHideWidget(termWidget);
			XPLoseKeyboardFocus(textInputField);
		}
		return 1;
	}
	
	return 0;
}

/// Parses the given input frequency into a frequency usable by X-Plane (10 Hz steps)
int parseMHzFrequency(const char *frequency)
{
	char buf[100];
	strncpy(buf, frequency, sizeof(buf));
	buf[sizeof(buf)-1] = 0;
	for(int i = 0; i < strlen(buf); i++)
	{
		if(buf[i] == ',')
			buf[i] = '.';
	}
	
	double f = atof(buf);
	return (int)(f * 100.0);
}

int parsekHzFrequency(const char *frequency)
{
	char buf[100];
	strncpy(buf, frequency, sizeof(buf));
	buf[sizeof(buf)-1] = 0;
	for(int i = 0; i < strlen(buf); i++)
	{
		if(buf[i] == ',')
			buf[i] = '.';
	}
	
	double f = atof(buf);
	return (int)(f);
}

void processFrequencyCommand(const char *what, int channel, int frequency)
{
	char str[100];
	snprintf(str, sizeof(str), "%s%d", what, channel);
	
	XPLMDataRef ref = freqDataRefs[str];
	if(ref != NULL)
	{
		XPLMSetDatai(ref, frequency);
	}
}

int	textFieldWidgetHandler(
					  XPWidgetMessage			inMessage,
					  XPWidgetID				inWidget,
					  long					inParam1,
					  long					inParam2)
{
	if(inMessage == xpMsg_KeyPress && inWidget == textInputField && (KEY_FLAGS(inParam1) & xplm_DownFlag) == xplm_DownFlag )
	{
		char theChar = KEY_CHAR(inParam1);
		
		if (theChar == '\r')
		{
			// Retrieve the entered text
			char outbuf[100];
			XPGetWidgetDescriptor(textInputField, outbuf, sizeof(outbuf));
			outbuf[sizeof(outbuf)-1] = 0;
			
			// Hide the window
			XPHideWidget(termWidget);
			XPLoseKeyboardFocus(textInputField);
			
			// Process the command here
			if(strstr(outbuf, "nav") == outbuf)
			{
				int channel = outbuf[3] - '0';
				processFrequencyCommand("nav", channel, parseMHzFrequency(outbuf + 5));
			}
			else if(strstr(outbuf, "com") == outbuf)
			{
				int channel = outbuf[3] - '0';
				processFrequencyCommand("com", channel, parseMHzFrequency(outbuf + 5));
			}
			else if(strstr(outbuf, "adf") == outbuf)
			{
				int channel = outbuf[3] - '0';
				processFrequencyCommand("adf", channel, parsekHzFrequency(outbuf + 5));
			}
			else if(strstr(outbuf, "squawk ") == outbuf)
			{
				XPLMSetDatai(transponderRef, atoi(outbuf + 7));
			}
			else if(strcmp(outbuf, "alt sync") == 0)
			{
				XPLMCommandOnce(altSyncCmdRef);
			}
			else if(strstr(outbuf, "alt ") == outbuf)
			{
				XPLMSetDataf(altitudeRef, atof(outbuf + 4));
			}
			else if(strcmp(outbuf, "vs sync") == 0)
			{
				XPLMCommandOnce(vsSyncCmdRef);
			}
			else if(strstr(outbuf, "vs ") == outbuf)
			{
				float f = atof(outbuf + 3);
				XPLMSetDataf(verticalSpeedRef, f);
			}
			else if(strcmp(outbuf, "hdg sync") == 0)
			{
				XPLMCommandOnce(headingSyncCmdRef);
			}
			else if(strstr(outbuf, "hdg ") == outbuf)
			{
				float f = atof(outbuf + 4);
				XPLMSetDataf(headingRef, f);
			}
			else if(strcmp(outbuf, "ias sync") == 0)
			{
				XPLMCommandOnce(iasSyncCmdRef);
			}
			else if(strstr(outbuf, "ias ") == outbuf)
			{
				float f = atof(outbuf + 4);
				XPLMSetDataf(iasRef, f);
			}
			else if(strstr(outbuf, "crs ") == outbuf)
			{
				float f = atof(outbuf + 4);
				XPLMSetDataf(courseRef, f);
			}
			else if(strstr(outbuf, "qnh ") == outbuf)
			{
				float hPa = atof(outbuf + 4);
				// convert in hPa to inHg
				float inHg = hPa / 33.8638866667;
				XPLMSetDataf(barometerRef, inHg);
			}
			else if(strcmp(outbuf, "ap hdg") == 0)
			{
				XPLMCommandKeyStroke(xplm_key_otto_hdg);
			}
			else if(strcmp(outbuf, "ap nav") == 0)
			{
				XPLMCommandKeyStroke(xplm_key_otto_gps);
			}
			else if(strcmp(outbuf, "ap alt") == 0)
			{
				XPLMCommandOnce(altArmCmdRef);
			}
			else if(strcmp(outbuf, "ap vs") == 0)
			{
				XPLMCommandOnce(verticalSpeedCmdRef);
			}
			else if(strcmp(outbuf, "ap app") == 0)
			{
				XPLMCommandOnce(approachCmdRef);
			}
			else if(strcmp(outbuf, "ap flc") == 0)
			{
				XPLMCommandOnce(flcCmdRef);
			}
			else if(strcmp(outbuf, "ap") == 0)
			{
				XPLMCommandOnce(autopilotCmdRef);
			}
			else if(strcmp(outbuf, "fd") == 0)
			{
				XPLMCommandOnce(flightDirCmdRef);
			}
		}
		
		// Let the event through
		return 0;
	}
	
	return 0;
}


