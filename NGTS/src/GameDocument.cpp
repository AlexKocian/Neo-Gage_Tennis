// INCLUDE FILES

// Class includes
#include "GameDocument.h"

// User includes
#include "GameAppUi.h" // CGameAppUi

// ============= MEMBER FUNCTIONS ==============

// Constructs the document for aApp
CGameDocument::CGameDocument( CEikApplication &aApp )
: CAknDocument( aApp )
{}

// Symbian OS 2nd phase constructor
void CGameDocument::ConstructL()
{}

// Creates the AppUi for this document
CEikAppUi *CGameDocument::CreateAppUiL()
{
	return new ( ELeave ) CGameAppUi;
}

// Symbian OS 2nd phase constructor
// Constructs the CGameDocument using the constructor and the
// ConstructL method
CGameDocument *CGameDocument::NewL( CEikApplication &aApp )
{
	CGameDocument *self = new ( ELeave ) CGameDocument( aApp );
	CleanupStack::PushL( self );

	self->ConstructL();
	CleanupStack::Pop( self );
	return self;
}

// End of File