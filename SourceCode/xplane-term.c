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

XPLMHotKeyID	gHotKey = NULL;
XPLMDataRef		gNav1DataRef = NULL;

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
	
	gNav1DataRef = XPLMFindDataRef("sim/cockpit/radios/nav1_freq_hz");
	
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
int parseFrequency(const char *frequency)
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

void processFrequencyCommand(const char *what, int channel, const char *frequencyStr)
{
	int frequency = parseFrequency(frequencyStr);
	XPLMSetDatai(gNav1DataRef, frequency);
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
				int channel = '0' - outbuf[3];
				processFrequencyCommand("nav", channel, outbuf + 5);
			}
			else if(strstr(outbuf, "com") == outbuf)
			{
				int channel = '0' - outbuf[3];
				processFrequencyCommand("com", channel, outbuf + 5);
			}
			else if(strstr(outbuf, "adf") == outbuf)
			{
				int channel = '0' - outbuf[3];
				processFrequencyCommand("adf", channel, outbuf + 5);
			}
		}
		
		// Let the event through
		return 0;
	}
	
	return 0;
}


