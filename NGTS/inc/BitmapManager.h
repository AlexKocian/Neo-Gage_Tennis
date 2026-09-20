#ifndef BITMAPMANAGER_H
#define BITMAPMANAGER_H

// INCLUDES
#include <aknutils.h>	// CompletePathWithAppPath
#include <e32std.h>		// Buffers

// CONSTANTS

// CLASS DECLARATION

class CBitmapManager : public CBase
{
public:
	static CBitmapManager *NewL();
	static CBitmapManager *NewLC();
	~CBitmapManager();

private:
	CBitmapManager();
	void ConstructL();

public:
	TFileName iBitmapPath;
	CFbsBitmap *iPaddleBitmap;
	CFbsBitmap *iPaddleMask;
	CFbsBitmap *iBallBitmap;
	CFbsBitmap *iBallMask;

	CFbsBitmap *iRightArrowBitmap;
	CFbsBitmap *iRightArrowMask;
	CFbsBitmap *iLeftArrowBitmap;
	CFbsBitmap *iLeftArrowMask;

	CFbsBitmap *iMainMenuLogoBitmap;

	// TODO make an RPointerArray for these
};

#endif