#ifndef GAME_H
#define GAME_H

// INCLUDES

// System includes
#include <e32std.h>
#include <e32base.h>
#include <w32std.h>

// User includes
#include "KeyHandler.h"
#include "SettingsManager.h"
#include "SoundManager.h"

// CONSTANTS

// FORWARD DECLARATIONS
class CScoreManager;
class CBitmapManager;

// HELPER CLASS

// Keeps track of the framerate (rounded down)
class CFrameCounter : public CBase
{
public:
	static CFrameCounter *NewL();
	static CFrameCounter *NewLC();
	~CFrameCounter();

private:
	CFrameCounter();
	void ConstructL();

public:
	void IncrementCounter();
	TInt GetFrameCount() const;

private:
	TTime iLastSecond; // In microseconds
	TTimeIntervalMicroSeconds32 iOneSecond;

	TInt iLastFrameCount;
	TInt iCurrentFrameCount;
};

// CLASS DECLARATION

class CEngine : public CBase
{
public:
	friend class CGameLoop;
	friend class CRenderer;

	static CEngine *NewL( TRect aRect, CSoundManager &aSoundManager );
	static CEngine *NewLC( TRect aRect, CSoundManager &aSoundManager );
	~CEngine();

public:
	void UpdateEngine();
	void Reset( TRect aRect );
	TKeyHandler &KeyHandler();
	const TKeyHandler &KeyHandler() const;
	TInt GetPlayerScore() const;
	TInt GetRandomNumber( TInt64 &aSeed, TInt aMaxValue );

private:
	CEngine( TRect aRect, CSoundManager &aSoundManager );
	void ConstructL( TRect aRect, CSoundManager &aSoundManager );
	void CheckCollision( TInt &aDirX, TInt &aDirY );
	void DirectPlayer();
	void DirectEnemy();

private:
	TRect iScreenRect;
	TKeyHandler iKeyHandler;
	CSoundManager &iSoundManager;

	// Ball parameters
	TPoint iOldBallPos; // Ball's position on last frame
	TPoint iBallPos;
	TPoint iBallVec; // Horizontal, vertical speed
	TInt iBallSize;

	// Player paddle parameters
	TSize iPlayerSize;
	TPoint iPlayerPos;
	TInt iPlayerSpeed;

	// Enemy paddle parameters
	TPoint iEnemyPos;
	TInt iEnemySpeed;

	// Score
	TInt iPlayerScore;
	TInt iEnemyScore;

	// Puck grace period
	TInt iGraceTime;	// Measured in frames

	// Seed for pseudo-random functions
	TInt64 iSeed;
};

class CDSAWrapper : public CBase
{
public:
	static CDSAWrapper *NewL( RWsSession &aWs, CWsScreenDevice &aScreenDevice, RWindowBase &aWindow );
	static CDSAWrapper *NewLC( RWsSession &aWs, CWsScreenDevice &aScreenDevice, RWindowBase &aWindow );
	~CDSAWrapper();

public:
	TBool DSAAvailable() const;
	CFbsBitGc* DSAGc();
	void InstantiateDSAL( MDirectScreenAccess &aObserver );
	void DeleteDSA();
	void StartDSAL();
	void CancelDSA();
	TBool DSAActive() const;
	void UpdateScreenDevice();

private:
	void ConstructL();
	CDSAWrapper( RWsSession &aWs, CWsScreenDevice &aScreenDevice, RWindowBase &aWindow );

private:
	CDirectScreenAccess *iDSA;
	RWsSession &iWs;
	CWsScreenDevice &iScreenDevice;
	RWindowBase &iWindow;
	TBool iCanBeActivated;
};

class CBackBuffer : public CBase
{
public:
	static CBackBuffer *NewL( TSize aSize, TDisplayMode aDisplayMode );
	static CBackBuffer *NewLC( TSize aSize, TDisplayMode aDisplayMode );
	~CBackBuffer();

public:
	const CFbsBitmap &GetBackBufferBitmap() const;
	CFbsBitGc *GetBackBufferGc() const;

private:
	CBackBuffer( TSize aSize, TDisplayMode aDisplayMode );
	void ConstructL( TSize aSize, TDisplayMode aDisplayMode );

private:
	CFbsBitmap *iBackBufferBitmap;
	CFbsBitmapDevice *iBackBufferBitmapDevice;
	CFbsBitGc *iBackBufferGc;
};

class CRenderer : public CBase
{
public:
	static CRenderer *NewL( CEngine  &aEngine, TSize aSize, CScoreManager &aScoreManager, CSettingsManager &aSettingsManager );
	static CRenderer *NewLC( CEngine &aEngine, TSize aSize, CScoreManager &aScoreManager, CSettingsManager &aSettingsManager );
	~CRenderer();

public:
	enum TScreenType
	{
		EScreenMainMenu = 0x01,
		EScreenHighScores,
		EScreenOptions,
		EScreenPaused,
		EScreenGame,
	};

	void RenderToBackBuffer() const;
	void BlitToFrontBuffer( CFbsBitGc *aDSAGc ) const;

	void SetScreenType( TScreenType aScreenType );
	const TScreenType &GetScreenType() const;

private:
	CRenderer( CEngine &aEngine, TSize aSize, CScoreManager &aScoreManager, CSettingsManager &aSettingsManager );
	void ConstructL( CEngine &aEngine, TSize aSize, CScoreManager &aScoreManager, CSettingsManager &aSettingsManager );

	void RenderGame() const;
	void RenderMainMenu() const;
	void RenderPaused() const;
	void RenderHighScores() const;
	void RenderOptions() const;

private:
	TScreenType iScreenType;
	CEngine &iEngine;
	CScoreManager &iScoreManager;
	CSettingsManager &iSettingsManager;

	CBackBuffer *iBackBuffer;
	TSize iScreenSize;

	CBitmapManager *iBitmapManager;
	CFrameCounter *iFrameCounter;

public:
	TInt iHighlightedMenuItem; // Init to 0
};

class CGameLoop : public CTimer, public MDirectScreenAccess
{
public:
	static CGameLoop *NewL( CEngine &aEngine, CDSAWrapper &aDSA, CRenderer &aRenderer, CSoundManager &aSoundManager );
	static CGameLoop *NewLC( CEngine &aEngine, CDSAWrapper &aDSA, CRenderer &aRenderer, CSoundManager &aSoundManager );
	~CGameLoop();

public:
	void Start( TTimeIntervalMicroSeconds32 aInterval );
	void SetPaused( TBool aFlag );

public:
	void QueueSoundPlayback( CSoundManager::TWavFile aWavFileChosen );

public:
	// From MDirectScreenAccess
	void Restart( RDirectScreenAccess::TTerminationReasons aReason );
	void AbortNow( RDirectScreenAccess::TTerminationReasons aReason );

private:
	CGameLoop( CEngine &aEngine, CDSAWrapper &aDSA, CRenderer &aRenderer, CSoundManager &aSoundManager );
	void ConstructL();

private:
	// From CActive
	virtual void RunL();
	virtual TInt RunError( TInt aError );
	virtual void DoCancel();

public:
	TTimeIntervalMicroSeconds32 iInterval;

	// Sound playback requested by the container (OfferKeyEventL)
	TBool iSoundQueued;
	CSoundManager::TWavFile iQueuedWavFile;

private:
	CEngine &iEngine;
	CDSAWrapper &iDSA;
	CRenderer &iRenderer;
	CSoundManager &iSoundManager;

	TBool iPaused; // Rendering and engine paused
};

#endif
