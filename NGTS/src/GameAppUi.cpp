// INCLUDE FILES

// Class include
#include "GameAppUi.h"

// System includes
#include <aknquerydialog.h>
#include <NGTS.rsg>

// User includes
#include "Game.hrh"				// Command IDs
#include "GameContainer.h"		// CGameContainer

// ============== MEMBER FUNCTIONS ==================

// Symbian OS 2nd phase constructor. Constructs the application's
// container, setting itself as its MOP parent and adding it to
// the control stack
void CGameAppUi::ConstructL()
{
	BaseConstructL();

	// Hide status pane and CBA
	StatusPane()->MakeVisible( EFalse );
	Cba()->MakeVisible( EFalse );

	iAppContainer = CGameContainer::NewL( ApplicationRect() );
	iAppContainer->SetMopParent( this );
	AddToStackL( iAppContainer );

	// Enables resposiveness to multiple simultaneous key presses
	SetKeyBlockMode( ENoKeyBlock );
}

// Destructor. Removes the application's container from
// the control stack and deletes it
CGameAppUi::~CGameAppUi()
{
	if ( iAppContainer )
	{
		RemoveFromStack( iAppContainer );
		delete iAppContainer;
	}
}


// From CEikAppUi. Takes care of command handling
void CGameAppUi::HandleCommandL( TInt aCommand )
{
	switch( aCommand )
	{

		case EAknSoftkeyBack:
		case EEikCmdExit:
		case EAknSoftkeyExit:
		{
			Exit();
			break;
		}

		default:
			break;
	}
}

// From CEikAppUi, takes care of loss of focus
// @param aForeground ETrue if app just gained focus, EFalse otherwise
void CGameAppUi::HandleForegroundEventL( TBool aForeground )
{
	if ( aForeground )
	{
		if ( !iGamePaused )
		{
			iAppContainer->StartDSA();
		}

		SetKeyBlockMode( ENoKeyBlock );
	}
	else
	{
		iAppContainer->StopDSA();
		SetKeyBlockMode( EDefaultBlockMode );
	}
}

// From CEikAppUi, handles the emphasising or de-emphasising of a menu window
// @param aMenuControl the menu control
// @param ETrue to emphasise the menu, EFalse otherwise
void CGameAppUi::SetEmphasis( CCoeControl * /*aMenuControl*/, TBool aEmphasis )
{
	if ( aEmphasis )
	{
		iMenuDisplay = ETrue;
	}
	else
	{
		if ( iMenuDisplay )
		{
			iMenuDisplay = EFalse;
			iAppContainer->StartDSA();
		}
	}
}

// End of File