// INCLUDE FILES

// Class includes
#include "GameApplication.h" // KUidNGTS

// User includes
#include "GameDocument.h"

// ============= MEMBER FUNCTIONS ===============

// Inherited from CApaApplication; returns the application's UID (KUidNGTS)
TUid CGameApplication::AppDllUid() const
{
	return KUidNGTS;
}

// Inherited from CApaApplication; creates a CGameDocument object
CApaDocument *CGameApplication::CreateDocumentL()
{
	return CGameDocument::NewL( *this );
}

// ================= OTHER EXPORTED FUNCTIONS ===================

// Constructs a CGameApplication object
EXPORT_C CApaApplication *NewApplication()
{
	return new CGameApplication;
}

// This is the enrtry point function for this Series 60 App
GLDEF_C TInt E32Dll( TDllReason )
{
	return KErrNone;
}

// End of File
