// INCLUDE FILES

// Class include
#include "SettingsManager.h"

// User includes
#include <coemain.h>	// CCoeEnv
#include <aknutils.h>	// CompleteWithAppPath

// CONSTANTS
_LIT( KTxtFileName, "Settings.ini" );

_LIT( KTxtSettingFramerate, "Target FPS" );
_LIT( KTxtSettingFramerate30, "30" );	// Default
_LIT( KTxtSettingFramerate60, "60" );
_LIT( KTxtSettingFramerateUC, "Inf" );

_LIT( KTxtSettingFrameCounter, "FPS Counter" );	// Defaults to 0

_LIT( KTxtSettingSound, "Sound" ); // Defaults to 1

const TInt KSettingsItemCount = 3;

// HELPER CLASS

TSettingsItem::TSettingsItem() {}

TSettingsItem::~TSettingsItem()
{
	iValueNames.Reset();
}

void TSettingsItem::ExternalizeL( RWriteStream aStream ) const
{
	aStream.WriteInt32L( static_cast< TInt32 >( iItemType ) );
	aStream << iItemName;

	aStream.WriteInt8L( static_cast< TInt8 >( iToggle ) );

	aStream.WriteInt32L( static_cast< TInt32 >( iRadioIndex ) );
	aStream.WriteInt32L( static_cast< TInt32 >( iRadioValueCount ) );
	for ( TInt i = 0; i < iValueNames.Count(); ++i )
	{
		aStream << iValueNames[ i ];
	}
}

void TSettingsItem::InternalizeL( RReadStream aStream )
{
	iItemType = static_cast< TSettingsItemType >( aStream.ReadInt32L() );
	HBufC16 *TempBuf = HBufC16::NewL( aStream, 20 ); // TODO hard-coded max length
	iItemName = *TempBuf;
	delete TempBuf;

	iToggle = static_cast< TBool >( aStream.ReadInt8L() );

	iRadioIndex = static_cast< TInt >( aStream.ReadInt32L() );
	iRadioValueCount = static_cast< TInt >( aStream.ReadInt32L() );
	for ( TInt i = 0; i < iRadioValueCount; ++i )
	{
		HBufC16 *ValueName = HBufC16::NewLC( aStream, 20 ); // TODO again here
		User::LeaveIfError( iValueNames.Append( *ValueName ) );
		CleanupStack::PopAndDestroy( ValueName );
	}
}

// MEMBER FUNCTIONS

CSettingsManager::CSettingsManager() : iFs( CEikonEnv::Static()->FsSession() ), iSettingCount( KSettingsItemCount ) {}

CSettingsManager *CSettingsManager::NewL()
{
	CSettingsManager *self = CSettingsManager::NewLC();
	CleanupStack::Pop( self );

	return self;
}

CSettingsManager *CSettingsManager::NewLC()
{
	CSettingsManager *self = new ( ELeave ) CSettingsManager;
	CleanupStack::PushL( self );

	self->ConstructL();
	return self;
}

void CSettingsManager::ConstructL()
{
	// Drive-agnostic full path completion
	iFileName = KTxtFileName();
	CompleteWithAppPath( iFileName );

	LoadSettingsL();
}

CSettingsManager::~CSettingsManager()
{
	iSettingsItems.Reset();
}

void CSettingsManager::InitSettingsL()
{
	RFileWriteStream WriteStream;
	User::LeaveIfError( WriteStream.Replace( iFs, iFileName, EFileWrite | EFileShareExclusive ) );

	// Push onto CleanupStack
	WriteStream.PushL();

	// Framerate (radio setting)
	TSettingsItem SettingsItem;

	SettingsItem.iItemType = TSettingsItem::ESettingsItemTypeRadio;
	SettingsItem.iItemName = KTxtSettingFramerate();
	SettingsItem.iRadioIndex = 0;
	SettingsItem.iRadioValueCount = 3;
	User::LeaveIfError( SettingsItem.iValueNames.Append( KTxtSettingFramerate30() ) );
	User::LeaveIfError( SettingsItem.iValueNames.Append( KTxtSettingFramerate60() ) );
	User::LeaveIfError( SettingsItem.iValueNames.Append( KTxtSettingFramerateUC() ) );

	SettingsItem.ExternalizeL( WriteStream );

	// FPS Counter
	SettingsItem.iItemType = TSettingsItem::ESettingsItemTypeToggle;
	SettingsItem.iItemName = KTxtSettingFrameCounter();
	SettingsItem.iToggle = 0;

	SettingsItem.ExternalizeL( WriteStream );

	// Sound
	SettingsItem.iItemType = TSettingsItem::ESettingsItemTypeToggle;
	SettingsItem.iItemName = KTxtSettingSound();
	SettingsItem.iToggle = 1;

	SettingsItem.ExternalizeL( WriteStream );

	WriteStream.CommitL();
	CleanupStack::PopAndDestroy( &WriteStream );
}

void CSettingsManager::LoadSettingsL()
{
	RFileReadStream ReadStream;
	TInt ErrorCode = ReadStream.Open( iFs, iFileName, EFileRead | EFileShareReadersOnly );

	if ( ErrorCode != KErrNone )
	{
		// File does not yet exist
		if ( ErrorCode == KErrNotFound || ErrorCode == KErrPathNotFound )
		{
			InitSettingsL();
			User::LeaveIfError( ReadStream.Open( iFs, iFileName, EFileRead | EFileShareReadersOnly ) );
		}
		// Unrecoverable error TODO check if low disk space
		else
		{
			User::LeaveIfError( ErrorCode );
		}
	}

	// Push onto CleanupStack
	ReadStream.PushL();

	// Load items
	iSettingsItems.Reset();

	for ( TInt i = 0; i < KSettingsItemCount; ++i )
	{
		User::LeaveIfError( iSettingsItems.Append( TSettingsItem() ) );
		iSettingsItems[ i ].InternalizeL( ReadStream );
	}

	CleanupStack::PopAndDestroy( &ReadStream );
}

void CSettingsManager::SaveSettingsL()
{
	RFileWriteStream WriteStream;
	User::LeaveIfError( WriteStream.Replace( iFs, iFileName, EFileWrite | EFileShareExclusive ) );

	// Push onto CleanupStack
	WriteStream.PushL();

	for ( TInt i = 0; i < iSettingsItems.Count(); ++i )
	{
		iSettingsItems[ i ].ExternalizeL( WriteStream );
	}

	WriteStream.CommitL();
	CleanupStack::PopAndDestroy( &WriteStream );
}
