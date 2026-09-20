#ifndef KEYHANDLER_H
#define KEYHANDLER_H

// INCLUDES

// System includes
#include <e32std.h>

// CLASS DECLARATION

/*
*
* TkeyHandler records the current state of the device keys
*
*/

class TKeyHandler
{
public:
	enum TKeyState
	{
		ELeft = 0x01,
		ERight = 0x02
	};

	TKeyHandler();
	TInt State() const;
	void SetState( TKeyState aKey, TBool aState );

private:
	TInt iDirection;
};

#endif

// End of File