#ifndef GAMEAPPLICATION_H
#define GAMEAPPLICATION_H

// INCLUDES

// System includes
#include <aknapp.h> // CAknApplication

// CONSTANTS
// UID of the application
const TUid KUidNGTS = {0x68C7C3B9}; // Experimental UID3

// CLASS DECLARATION

/*
*
* CGameApplication class
* This is the main application class
*
* It provides a way to create a document object and a method
* to retrieve the application's UID.
*
*/
class CGameApplication : public CAknApplication
{
private: // from CApaApplication

	CApaDocument *CreateDocumentL();
	TUid AppDllUid() const;
};

#endif

// End of File
