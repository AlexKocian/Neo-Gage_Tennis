#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

//INCLUDES

// System includes
#include <s32std.h>
#include <f32file.h>
#include <s32file.h>

// HELPER CLASS

// Warning! This class allows for unsafe memory access for the sake of simplicity.
// Each function interacting with an object of this type needs to avoid
// accessing uninitialised values! Always query the setting's type first :)

// Implementation notes for future reference:
// For use within an RArray -> max size of 640 bytes, meaning additional functionality -
// or higher amount of options - may need to be allocated on the heap
class TSettingsItem
{
public:
	TSettingsItem();
	~TSettingsItem();

public:
	// Standard functions for writing to/reading from streams
	void ExternalizeL( RWriteStream aStream ) const;
	void InternalizeL( RReadStream aStream );

public:
	enum TSettingsItemType
	{
		ESettingsItemTypeToggle = 0x01,
		ESettingsItemTypeRadio,
		ESettingsItemTypeRange, // Unused
		ESettingsItemTypeTextInput, // Unused
		ESettingsItemTypeNumberInput, // Unused
	};

public:
	// Common member variables
	TSettingsItemType iItemType;
	TBufC< 20 > iItemName;

	// For ESettingsItemTypeToggle
	TBool iToggle;

	// For ESettingsItemTypeRadio
	TInt iRadioIndex;
	TInt iRadioValueCount;
	RArray< TBufC< 20 > > iValueNames;

	// More functionality to be added here
};

// CLASS DECLARATION

class CSettingsManager : public CBase
{
public:
	static CSettingsManager *NewL();
	static CSettingsManager *NewLC();
	~CSettingsManager();

private:
	void ConstructL();
	CSettingsManager();

public:
	void InitSettingsL();
	void SaveSettingsL();
	void LoadSettingsL();

private:
	RFs &iFs;	// Owned by iCoeEnv
	TFileName iFileName;

public:
	RArray< TSettingsItem > iSettingsItems;
	TInt iSettingCount;
};

#endif