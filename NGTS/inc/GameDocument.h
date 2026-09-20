#ifndef GAMEDOCUMENT_H
#define GAMEDOCUMENT_H

// INCLUDES

// System includes
#include <akndoc.h> // CAknDocument

// FORWARD DECLARATIONS
class CEikAppUI;

// CLASS DECLARATION

/*
*
* CGameDocument class
* This is the document class
*
*/
class CGameDocument : public CAknDocument
{
public: // Constructor

	static CGameDocument *NewL( CEikApplication &aApp );

private: // Constructors

	CGameDocument( CEikApplication &aApp );
	void ConstructL();

private: // from CEikDocument

	CEikAppUi *CreateAppUiL();
};

#endif

// End of File