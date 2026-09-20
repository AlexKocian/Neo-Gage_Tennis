// User includes
#include "KeyHandler.h"

TKeyHandler::TKeyHandler() {}

// @return a bit pattern indicating which keys are currently pressed
TInt TKeyHandler::State() const
{
	return iDirection;
}


// @param aKey the key that's being set or unset
// @param aState a boolean flag to indicate whether the button is being pressed or released
void TKeyHandler::SetState( TKeyHandler::TKeyState aKey, TBool aState )
{
	if ( aState ) // Pressed
	{
		iDirection |= static_cast <TInt> ( aKey );
	}
	else // Released
	{
		iDirection &= static_cast <TInt> ( ~aKey );
	}
}