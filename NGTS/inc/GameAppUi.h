#ifndef GAMEAPPUI_H
#define GAMEAPPUI_H

// INCLUDES

// System includes
#include <aknappui.h> //CAknAppUi

// FORWARD DECLARATIONS
class CGameContainer;

// CLASS DECLARATION

/*
*
* CGameAppUi class
* This is the main application UI class
* It provides support for the EIKON control architecture.
*
*/
class CGameAppUi : public CAknAppUi
{
public: // Constructors and destructor

	void ConstructL();
	~CGameAppUi();

private: // from CEikAppUi

	void HandleCommandL( TInt aCommand );
	void HandleForegroundEventL( TBool aForeground );
	void SetEmphasis( CCoeControl *aMenuControl, TBool aEmphasis );

private: // Data

	CGameContainer *iAppContainer;
	TBool iMenuDisplay;
	TBool iGamePaused;
};

#endif

// End of File