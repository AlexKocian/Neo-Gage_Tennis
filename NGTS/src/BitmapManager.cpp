//INCLUDES
#include "BitmapManager.h"
#include <NGTS.mbg>		// Bitmap enumerations
#include <aknutils.h>	// CompletePathWithAppPath
#include <e32std.h>		// Buffers

// CONSTANTS
_LIT( KGameBitmapPath, "NGTS.mbm" );

// MEMBER FUNCTIONS
CBitmapManager::CBitmapManager() {}

CBitmapManager::~CBitmapManager()
{
	delete iPaddleBitmap;
	delete iPaddleMask;
	delete iBallBitmap;
	delete iBallMask;

	delete iRightArrowBitmap;
	delete iRightArrowMask;
	delete iLeftArrowBitmap;
	delete iLeftArrowMask;

	delete iMainMenuLogoBitmap;
}

CBitmapManager *CBitmapManager::NewL()
{
	CBitmapManager *self = CBitmapManager::NewLC();
	CleanupStack::Pop( self );

	return self;
}

CBitmapManager *CBitmapManager::NewLC()
{
	CBitmapManager *self = new ( ELeave ) CBitmapManager();
	CleanupStack::PushL( self );

	self->ConstructL();
	return self;
}

void CBitmapManager::ConstructL()
{
	iBitmapPath = KGameBitmapPath();
	CompleteWithAppPath( iBitmapPath );

	iPaddleBitmap = new ( ELeave ) CFbsBitmap();
	User::LeaveIfError( iPaddleBitmap->Load( iBitmapPath, EMbmNgtsPaddle ) );

	iPaddleMask = new ( ELeave ) CFbsBitmap();
	User::LeaveIfError( iPaddleMask->Load( iBitmapPath, EMbmNgtsPaddlemask ) );

	iBallBitmap = new ( ELeave ) CFbsBitmap();
	User::LeaveIfError( iBallBitmap->Load( iBitmapPath, EMbmNgtsBall ) );

	iBallMask = new ( ELeave ) CFbsBitmap();
	User::LeaveIfError( iBallMask->Load( iBitmapPath, EMbmNgtsBallmask ) );

	iRightArrowBitmap = new ( ELeave ) CFbsBitmap();
	User::LeaveIfError( iRightArrowBitmap->Load( iBitmapPath, EMbmNgtsRightarrow ) );

	iRightArrowMask = new ( ELeave ) CFbsBitmap();
	User::LeaveIfError( iRightArrowMask->Load( iBitmapPath, EMbmNgtsRightarrowmask ) );

	iLeftArrowBitmap = new ( ELeave ) CFbsBitmap();
	User::LeaveIfError( iLeftArrowBitmap->Load( iBitmapPath, EMbmNgtsLeftarrow ) );

	iLeftArrowMask = new ( ELeave ) CFbsBitmap();
	User::LeaveIfError( iLeftArrowMask->Load( iBitmapPath, EMbmNgtsLeftarrowmask ) );

	iMainMenuLogoBitmap = new ( ELeave ) CFbsBitmap();
	User::LeaveIfError( iMainMenuLogoBitmap->Load( iBitmapPath, EMbmNgtsMainmenulogo ) );
}