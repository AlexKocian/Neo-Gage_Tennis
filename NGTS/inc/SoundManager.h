#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

// INCLUDES
#include <MdaAudioOutputStream.h>
#include <mda\common\audio.h>
#include <f32file.h>
#include "SettingsManager.h"

// OBSERVER MIXIN DEFINITION
class MSoundManagerObserver
{
public:
	virtual void HandlePlayingStoppedL() = 0;
};

// SOUND WRAPPER

// Takes care of a single sound file's audio data
// Important note: the file cannot be smaller in size than KDataChunkSize (SoundManager.cpp)
class CSoundFileWrapper : CBase
{
public:
	static CSoundFileWrapper *NewL( TFileName aSoundFile );
	static CSoundFileWrapper *NewLC( TFileName aSoundFile );
	~CSoundFileWrapper();

private:
	CSoundFileWrapper( TFileName aSoundFile );
	void ConstructL( TFileName aSoundFile );

public:
	TBool iPlaying;
	const TUint8 *GetNextChunk();
	void ResetReadHead();
	void StartPlayback();
	void StopPlayback();

public:
	TUint8 *iEOF; // Pointer to right after the data

private:
	TUint8 *iReadHead;
	HBufC8 *iSoundFileBuf;
};

// CLASS DECLARATION

class CSoundManager : public CBase, public MMdaAudioOutputStreamCallback
{
public:
	friend class CSettingsManager; // For sound settings

public:
	static CSoundManager *NewL( MSoundManagerObserver &aObserver, CSettingsManager &aSettingsManager );
	static CSoundManager *NewLC( MSoundManagerObserver &aObserver, CSettingsManager &aSettingsManager );
	~CSoundManager();

public:
	// Provided to user for specifying which file to play
	enum TWavFile
	{
		EBallBeep1 = 0x00,
		EBallBeep2 = 0x01,
		EMenuBeepAccept = 0x02,
		EMenuBeepMove = 0x03
	};

public:
	// From MMdaAudioOutputStreamCallback
	virtual void MaoscOpenComplete( TInt aError );
	virtual void MaoscBufferCopied( TInt aError, const TDesC8& aBuffer );
	virtual void MaoscPlayComplete( TInt aError );

public:
	void PlayWav( TWavFile aWavFileChosen );
	void Stop();
	
private:
	CSoundManager( MSoundManagerObserver &aObserver, CSettingsManager &aSettingsManager );
	void ConstructL();

private:
	MSoundManagerObserver &iObserver;
	CSettingsManager &iSettingsManager;

	CMdaAudioOutputStream *iPlayerStream;
	TMdaAudioDataSettings iStreamSettings;

	// Silence for when no sound should play
	TBuf8< 512 > iSilenceChunk;
	// Data chunk for mixing sound (KDataChunkSize) TODO constant as template?
	TBuf8< 512 > iDataChunk;

	// Individual sound files
	TFileName iPathBeep1;
	TFileName iPathBeep2;
	TFileName iPathMenuAccept;
	TFileName iPathMenuMove;

	RPointerArray< CSoundFileWrapper > iSoundFileArray;
};

#endif