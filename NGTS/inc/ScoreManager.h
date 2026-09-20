#ifndef SCOREMANAGER_H
#define SCOREMANAGER_H

// INCLUDES

// System includes
#include <s32std.h>
#include <f32file.h>
#include <s32file.h>

// HELPER CLASS

class TScoreTuple
{
public:
	TScoreTuple( TBufC< 3 > aPlayerName, TUint16 aScore );
	static TInt CompareScoresDesc( const TScoreTuple &aLeft, const TScoreTuple &aRight );

public:
	TBufC< 3 > iPlayerName;	// Default = "DEV" (for init) or "USR"
	TUint16 iScore;	// Max = 999
};

// CLASS DECLARATION

class CScoreManager : public CBase
{
public:
	static CScoreManager *NewL();
	static CScoreManager *NewLC();
	~CScoreManager();

private:
	void ConstructL();
	CScoreManager();

public:
	void InitHighScoresL();
	void LoadHighScoresL();
	void SaveHighScoresL();
	void SetHighScoreL( TUint16 aHighScore );

private:
	RFs &iFs;	// Owned by iCoeEnv

	TFileName iFileName;	// Full path to file

public:
	RArray< TScoreTuple > iScoreList;	// 8 entries
};

#endif