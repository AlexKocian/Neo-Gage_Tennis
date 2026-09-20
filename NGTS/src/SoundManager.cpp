// INCLUDES
#include "SoundManager.h"

#include <eikenv.h>
#include <stringloader.h>
#include <barsread.h>
#include <aknutils.h>

// CONSTANTS
_LIT( KBallBeep1File, "BallBeep1.wav" );
_LIT( KBallBeep2File, "BallBeep2.wav" );
_LIT( KMenuBeepAcceptFile, "MenuBeepAccept.wav" );
_LIT( KMenuBeepMoveFile, "MenuBeepMove.wav" );

const TInt KVolumeDenominator = 4;
const TInt KDataChunkSize = 512; // In bytes
const TInt KSoundFileArrayGranularity = 8;

// SOUND WRAPPER FUNCTIONS

CSoundFileWrapper::CSoundFileWrapper( TFileName /*aSoundFile*/ ) : iPlaying( EFalse )
{}

CSoundFileWrapper *CSoundFileWrapper::NewL( TFileName aSoundFile )
{
	CSoundFileWrapper *self = CSoundFileWrapper::NewLC( aSoundFile );
	CleanupStack::Pop( self );

	return self;
}

CSoundFileWrapper *CSoundFileWrapper::NewLC( TFileName aSoundFile )
{
	CSoundFileWrapper *self = new ( ELeave ) CSoundFileWrapper( aSoundFile );
	CleanupStack::PushL( self );

	self->ConstructL( aSoundFile );
	return self;
}

CSoundFileWrapper::~CSoundFileWrapper()
{
	iReadHead = NULL;
	delete iSoundFileBuf;
}

void CSoundFileWrapper::ConstructL( TFileName aSoundFile )
{
	RFs Fs;
	CleanupClosePushL( Fs );
	User::LeaveIfError( Fs.Connect() );

	RFile File;
	CleanupClosePushL( File );

	TFileName SoundFilePath = aSoundFile;
	User::LeaveIfError( CompleteWithAppPath( SoundFilePath ) );
	User::LeaveIfError( File.Open( Fs, SoundFilePath, EFileRead | EFileShareReadersOnly ) );

	TInt FileSize = 0;
	File.Size( FileSize );

	// Allocate a sufficiently large buffer on the heap
	iSoundFileBuf = HBufC8::NewMaxL( FileSize );

	// Set the read head to right after the 44 header bytes
	iReadHead = const_cast< TUint8 * >( iSoundFileBuf->Ptr() ) + 44;
	iEOF = const_cast< TUint8 * >( iSoundFileBuf->Ptr() ) + iSoundFileBuf->Length();

	TPtr8 TmpPtr = iSoundFileBuf->Des();
	File.Read( TmpPtr );

	CleanupStack::PopAndDestroy( 2 ); // File and Fs
}

const TUint8 *CSoundFileWrapper::GetNextChunk()
{
	const TUint8 *NextChunk = iReadHead;
	iReadHead += KDataChunkSize;

	// Check if playback has finished
	if ( iReadHead >= iSoundFileBuf->Ptr() + iSoundFileBuf->Length() )
	{
		iPlaying = EFalse;
		ResetReadHead();
	}

	return NextChunk;
}

void CSoundFileWrapper::ResetReadHead()
{
	iReadHead = const_cast< TUint8 * >( iSoundFileBuf->Ptr() ) + 44;
}

void CSoundFileWrapper::StartPlayback()
{
	ResetReadHead();
	iPlaying = ETrue;
}

void CSoundFileWrapper::StopPlayback()
{
	ResetReadHead();
	iPlaying = EFalse;
}

// MEMBER FUNCTIONS

CSoundManager::CSoundManager( MSoundManagerObserver &aObserver, CSettingsManager &aSettingsManager ) :
iObserver( aObserver ), iSoundFileArray( KSoundFileArrayGranularity ), iSettingsManager( aSettingsManager ) {}

CSoundManager::~CSoundManager()
{
	Stop();
	iSoundFileArray.ResetAndDestroy();
}

CSoundManager *CSoundManager::NewL( MSoundManagerObserver &aObserver, CSettingsManager &aSettingsManager )
{
	CSoundManager *self = CSoundManager::NewLC( aObserver, aSettingsManager );
	CleanupStack::Pop( self );

	return self;
}

CSoundManager *CSoundManager::NewLC( MSoundManagerObserver &aObserver, CSettingsManager &aSettingsManager )
{
	CSoundManager *self = new ( ELeave ) CSoundManager( aObserver, aSettingsManager );
	CleanupStack::PushL( self );

	self->ConstructL();
	return self;
}

void CSoundManager::ConstructL()
{
	// BallBeep1
	iPathBeep1 = KBallBeep1File;
	User::LeaveIfError( CompleteWithAppPath( iPathBeep1 ) );

	CSoundFileWrapper *Wrapper = CSoundFileWrapper::NewLC( iPathBeep1 );
	User::LeaveIfError( iSoundFileArray.Append( Wrapper ) );
	CleanupStack::Pop( Wrapper );


	// BallBeep2
	iPathBeep2 = KBallBeep2File;
	User::LeaveIfError( CompleteWithAppPath( iPathBeep2 ) );

	Wrapper = CSoundFileWrapper::NewLC( iPathBeep2 );
	User::LeaveIfError( iSoundFileArray.Append( Wrapper ) );
	CleanupStack::Pop( Wrapper );


	// MenuBeepAccept
	iPathMenuAccept = KMenuBeepAcceptFile;
	User::LeaveIfError( CompleteWithAppPath( iPathMenuAccept ) );

	Wrapper = CSoundFileWrapper::NewLC( iPathMenuAccept );
	User::LeaveIfError( iSoundFileArray.Append( Wrapper ) );
	CleanupStack::Pop( Wrapper );


	// MenuBeepMove
	iPathMenuMove = KMenuBeepMoveFile;
	User::LeaveIfError( CompleteWithAppPath( iPathMenuMove ) );

	Wrapper = CSoundFileWrapper::NewLC( iPathMenuMove );
	User::LeaveIfError( iSoundFileArray.Append( Wrapper ) );
	CleanupStack::Pop( Wrapper );

	// Fill chunks with 0s
	iSilenceChunk.FillZ( iSilenceChunk.MaxLength() );
	iDataChunk.FillZ( iDataChunk.MaxLength() );

	iStreamSettings.iSampleRate = TMdaAudioDataSettings::ESampleRate8000Hz;
	iStreamSettings.iChannels = TMdaAudioDataSettings::EChannelsMono;
	iStreamSettings.iFlags = TMdaAudioDataSettings::ENoNetworkRouting; // Only sent to speakers, not phone line

	iPlayerStream = CMdaAudioOutputStream::NewL( *this );
	iPlayerStream->Open( &iStreamSettings );
}

// STREAM CALLBACK FUNCTIONS

// Gets called when the stream has been opened
void CSoundManager::MaoscOpenComplete( TInt /*aError*/ )
{
	TRAPD( err, iPlayerStream->WriteL( iSilenceChunk ) );
}

// Gets called when the last chunk has been copied to the stream
void CSoundManager::MaoscBufferCopied( TInt aError, const TDesC8 &/*aBuffer*/ )
{
	// Playback has completed
	if ( aError == KErrAbort )
	{
		return;
	}

	// Check if audio is muted
	if ( !iSettingsManager.iSettingsItems[ 2 ].iToggle )
	{
		iPlayerStream->SetVolume( 0 );
	}
	else
	{
		iPlayerStream->SetVolume( iPlayerStream->MaxVolume() / KVolumeDenominator );
	}

	iDataChunk.SetLength( KDataChunkSize );
	iSilenceChunk.SetLength( KDataChunkSize );

	// Reset the chunk to silence
	iDataChunk.FillZ();

	TInt16 *DataChunkPtr = reinterpret_cast< TInt16 * >( const_cast<TUint8 *>( iDataChunk.Ptr() ) );
	TInt NumSamples = KDataChunkSize / 2;

	TInt PlayingCount = 0;
	for ( TInt i = 0; i < iSoundFileArray.Count(); ++i )
	{
		if ( !iSoundFileArray[ i ]->iPlaying )
		{
			continue;
		}

		++PlayingCount;

		const TUint8 *SourceBytePtr = iSoundFileArray[ i ]->GetNextChunk();
		TInt16 *SourceSamplePtr = reinterpret_cast< TInt16 * >( const_cast< TUint8 * >( SourceBytePtr ) );

		// Pre-calculate the actual size of 
		TInt BytesRemaining = iSoundFileArray[ i ]->iEOF - SourceBytePtr;
		TInt SamplesRemaining = BytesRemaining / 2;
		TInt SamplesToMix = ( SamplesRemaining < NumSamples ) ? SamplesRemaining : NumSamples;

		// Mix chunks
		for ( TInt j = 0; j < SamplesToMix; ++j )
		{
			// Mix sounds using 32-bit headroom
			TInt32 Mix = DataChunkPtr[ j ] + SourceSamplePtr[ j ];

			// Clamp to 16-bit boundaries
			if ( Mix > 32767 )
			{
				Mix = 32767;
	
			}
			else if ( Mix < -32767 )
			{
				Mix = -32767;
			}

			DataChunkPtr[ j ] = static_cast< TInt16 >( Mix );
		}
	}

	// Nothing is playing => send silence chunk
	if ( PlayingCount == 0 )
	{
		TRAPD( err, iPlayerStream->WriteL( iSilenceChunk ) );
		return;
	}

	// Otherwise send mixed sound chunk
	TRAPD( err, iPlayerStream->WriteL( iDataChunk ) );
}

// Gets called when streaming has been terminated
void CSoundManager::MaoscPlayComplete( TInt /*aError*/ )
{
	TRAPD( err, iObserver.HandlePlayingStoppedL() );
}

// PLAYER FUNCTIONS
void CSoundManager::PlayWav( TWavFile aWavFileChosen )
{
	iSoundFileArray[ aWavFileChosen ]->StartPlayback();
}

void CSoundManager::Stop()
{
	if ( iPlayerStream )
	{
		// Calls CSoundManager::MaoscBufferCopied with error code KErrAbort
		iPlayerStream->Stop();
	}

	delete iPlayerStream;
	iPlayerStream = NULL;
}