// INCLUDE FILES

// Class include
#include "ScoreManager.h"

// User includes
#include <coemain.h>	// CCoeEnv
#include <aknutils.h>	// CompleteWithAppPath

// CONSTANTS
_LIT( KTxtHighScoresFileName, "HighScores.txt" );
_LIT( KTxtDevName, "DEV" );
_LIT( KTxtPlayerName, "USR" );

// HELPER CLASS

TScoreTuple::TScoreTuple( TBufC< 3 > aPlayerName, TUint16 aScore ) :
iPlayerName( aPlayerName ), iScore( aScore )
{}

TInt TScoreTuple::CompareScoresDesc( const TScoreTuple &aLeft, const TScoreTuple &aRight )
{
	if ( aLeft.iScore > aRight.iScore )
	{
		return -1;
	}

	if ( aLeft.iScore == aRight.iScore )
	{
		return 0;
	}

	return 1;
}


// MEMBER FUNCTIONS

CScoreManager::CScoreManager() : iFs( CEikonEnv::Static()->FsSession() ) {}

CScoreManager *CScoreManager::NewL()
{
	CScoreManager *self = CScoreManager::NewLC();
	CleanupStack::Pop( self );

	return self;
}

CScoreManager *CScoreManager::NewLC()
{
	CScoreManager *self = new ( ELeave ) CScoreManager;
	CleanupStack::PushL( self );

	self->ConstructL();
	return self;
}

void CScoreManager::ConstructL()
{
	// Drive-agnostic full path completion
	iFileName = KTxtHighScoresFileName();
	CompleteWithAppPath( iFileName );

	// Load save scores into iScoreList
	// or create new file and populate with default scores
	LoadHighScoresL();
}

CScoreManager::~CScoreManager()
{
	iScoreList.Reset();
}

void CScoreManager::InitHighScoresL()
{
	RFileWriteStream WriteStream;
	User::LeaveIfError( WriteStream.Replace( iFs, iFileName, EFileWrite | EFileShareExclusive ) );

	// Push onto CleanupStack
	WriteStream.PushL();

	// Write iScoreList to file
	TUint16 Score = 70;

	for ( TInt i = 0; i < 8; ++i )
	{
		WriteStream << KTxtDevName();
		WriteStream.WriteUint16L( Score );

		Score -= 10;
	}

	WriteStream.CommitL();
	CleanupStack::PopAndDestroy( &WriteStream );
}

void CScoreManager::LoadHighScoresL()
{
	RFileReadStream ReadStream;
	TInt ErrorCode = ReadStream.Open( iFs, iFileName, EFileRead | EFileShareReadersOnly );

	if ( ErrorCode != KErrNone )
	{
		// File does not yet exist
		if ( ErrorCode == KErrNotFound || ErrorCode == KErrPathNotFound )
		{
			InitHighScoresL();
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

	// Load scores into iScoreList (default granularity = 8)
	TUint16 Score;

	iScoreList.Reset();

	for ( TInt i = 0; i < 8; ++i )
	{
		HBufC16 *PlayerName = HBufC16::NewLC( ReadStream, 3 );
		Score = ReadStream.ReadUint16L();

		User::LeaveIfError( iScoreList.Append( TScoreTuple( *PlayerName, Score ) ) );
		
		CleanupStack::PopAndDestroy( PlayerName );
	}

	CleanupStack::PopAndDestroy( &ReadStream );
}

void CScoreManager::SaveHighScoresL()
{
	RFileWriteStream WriteStream;
	User::LeaveIfError( WriteStream.Replace( iFs, iFileName, EFileWrite | EFileShareExclusive ) );

	// Push onto CleanupStack
	WriteStream.PushL();

	// Write iScoreList to file
	for ( TInt i = 0; i < iScoreList.Count(); ++i )
	{
		WriteStream << iScoreList[ i ].iPlayerName;
		WriteStream.WriteUint16L( iScoreList[ i ].iScore );
	}

	WriteStream.CommitL();
	CleanupStack::PopAndDestroy( &WriteStream );;
}

// Remove lowest score, insert new high score and sort
void CScoreManager::SetHighScoreL( TUint16 aHighScore )
{
	// If the new high score is lower than all of the present ones, NOP
	if ( aHighScore <= iScoreList[ iScoreList.Count() - 1 ].iScore )
	{
		return;
	}

	TLinearOrder< TScoreTuple > ScoreOrderDesc( TScoreTuple::CompareScoresDesc );
	if ( iScoreList.Count() > 0 )
	{
		iScoreList.Remove( iScoreList.Count() - 1 );
	}

	User::LeaveIfError( iScoreList.InsertInOrder(  TScoreTuple( KTxtPlayerName(), aHighScore ), ScoreOrderDesc ) );
}