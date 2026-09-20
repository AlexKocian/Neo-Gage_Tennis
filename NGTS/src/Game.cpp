#include "Game.h"
#include "ScoreManager.h"
#include "BitmapManager.h"
#include "SoundManager.h"
#include <aknutils.h>	// CompletePathWithAppPath
#include <e32math.h>	// Math
#include <e32std.h>		// Buffers
#include <gdi.h>		// CFont

#include <eikenv.h>		// CEikonEnv::Static()

// CONSTANTS
_LIT( KStartGameText, "Start Game" );
_LIT( KHighScoresText, "High Scores" );
_LIT( KOptionsText, "Options" );
_LIT( KExitText, "Exit" );

const TInt KCBAPaddingHorizontal = 2;
const TInt KCBAPaddingVertical = 5;

const TInt KLogoPaddingHorizontal = 5;
const TInt KLogoPaddingVertical = 10;

////////////////
// CFrameCounter
////////////////

CFrameCounter::CFrameCounter() : iCurrentFrameCount( 0 ), iLastFrameCount( 0 ), iOneSecond( 1000000 )
{
	iLastSecond = TTime();
	iLastSecond.UniversalTime();
}

CFrameCounter *CFrameCounter::NewL()
{
	CFrameCounter *self = CFrameCounter::NewLC();
	CleanupStack::Pop( self );

	return self;
}

CFrameCounter *CFrameCounter::NewLC()
{
	CFrameCounter *self = new ( ELeave ) CFrameCounter();
	CleanupStack::PushL( self );

	self->ConstructL();
	return self;
}

void CFrameCounter::ConstructL() {}

CFrameCounter::~CFrameCounter() {}

void CFrameCounter::IncrementCounter()
{
	TTime CurrentTime = TTime();
	CurrentTime.UniversalTime();
	if ( iLastSecond + iOneSecond <= CurrentTime )
	{
		iLastFrameCount = iCurrentFrameCount;
		iCurrentFrameCount = 1; // We count this frame render

		iLastSecond = CurrentTime;
		return;
	}

	++iCurrentFrameCount;
	// Clamp to 999 for buffer storage
	if ( iCurrentFrameCount >= 999 )
	{
		iCurrentFrameCount = 999;
	}
}

TInt CFrameCounter::GetFrameCount() const
{
	return iLastFrameCount;
}

//////////////
// CEngine
//////////////

CEngine::CEngine( TRect /*aRect*/, CSoundManager &aSoundManager) : iSoundManager( aSoundManager ) {}

// Symbian OS 2nd phase constructor
CEngine *CEngine::NewL( TRect aRect, CSoundManager &aSoundManager )
{
	CEngine *self = NewLC( aRect, aSoundManager );
	CleanupStack::Pop( self );
	return self;
}

// Symbian OS 2nd phase constructor
CEngine *CEngine::NewLC( TRect aRect, CSoundManager &aSoundManager )
{
	CEngine *self = new ( ELeave ) CEngine( aRect, aSoundManager );
	CleanupStack::PushL( self );

	self->ConstructL( aRect, aSoundManager );
	return self;
}

// Destructor
CEngine::~CEngine() {}

	
// Symbian OS 2nd phase constructor
void CEngine::ConstructL( TRect aRect, CSoundManager &/*aSoundManager*/ )
{
	this->Reset( aRect );

	// Get a seed for pseudo-random functions
	TTime CurrentTime;
	CurrentTime.UniversalTime();

	iSeed = CurrentTime.Int64();
}

// Initialises all values
void CEngine::Reset( TRect aRect )
{
	iScreenRect = aRect;
	iBallSize = 6;
	iBallVec = TPoint( 0, 8 ); // Going straight down
	iBallPos = aRect.Center() - TPoint( iBallSize / 2, iBallSize / 2 );

	iPlayerSize = TSize( 30, 2 );
	iPlayerPos = TPoint( aRect.Center().iX  - iPlayerSize.iWidth / 2,
						 static_cast< TInt >( iScreenRect.iBr.iY - 2 * iPlayerSize.iHeight ) );
	iPlayerSpeed = 0;

	iEnemyPos = TPoint( aRect.Center().iX - iPlayerSize.iWidth / 2,
						static_cast< TInt >( iScreenRect.iTl.iY + iPlayerSize.iHeight ) );

	iEnemySpeed = 0;

	iPlayerScore = 0;
	iEnemyScore = 0;

	iGraceTime = 20;	// Puck will not move for 20 frames
}

// Gives you a random number from interval [0, aMaxValue]
TInt CEngine::GetRandomNumber( TInt64 &aSeed, TInt aMaxValue )
{
	return Math::Rand( aSeed ) % ( aMaxValue + 1 );
}

// Looks at the KeyHandler for user input passed to it
void CEngine::DirectPlayer()
{
	if ( iKeyHandler.State() & TKeyHandler::ELeft )
	{
		iPlayerSpeed = -4;
	}

	else if ( iKeyHandler.State() & TKeyHandler::ERight )
	{
		iPlayerSpeed = 4;
	}

	else
	{
		iPlayerSpeed = 0;
	}
}

// Determines the enemy's movement direction based on the ball's position
void CEngine::DirectEnemy()
{
	// Only move if the ball is moving towards the enemy
	if ( iBallVec.iY >= 0 )
	{
		iEnemySpeed = 0;
		return;
	}

	if ( iBallPos.iX <= iEnemyPos.iX )
	{
		iEnemySpeed = -4;
	}

	else if ( ( iBallPos.iX >= iEnemyPos.iX + 2 * iBallSize ) &&
			  ( iBallPos.iX <= iEnemyPos.iX + iPlayerSize.iWidth / 2 ) )
	{
		iEnemySpeed = 4;
	}

	else if ( ( iBallPos.iX <= iEnemyPos.iX + iPlayerSize.iWidth - 2 * iBallSize ) &&
			  ( iBallPos.iX >= iEnemyPos.iX + iPlayerSize.iWidth / 2 ) )
	{
		iEnemySpeed = -4;
	}

	else if ( iBallPos.iX >= iEnemyPos.iX + iPlayerSize.iWidth )
	{
		iEnemySpeed = 4;
	}
	
	else
	{
		iEnemySpeed = 0;
	}
}

// If the ball has collided with something, change its direction
void CEngine::CheckCollision( TInt &aDirX, TInt &aDirY )
{
	///////////////////////
	// Ball collision
	///////////////////////

	// Check vertical collision
	if ( iBallPos.iY < iScreenRect.iTl.iY )
	{
		iBallPos = iScreenRect.Center() - TPoint( iBallSize / 2, iBallSize / 2 );
		iBallVec = TPoint( 0, 8 );

		iGraceTime = 20;

		++iPlayerScore;
		if ( iPlayerScore > 999 )
		{
			iPlayerScore = 999;
		}

		aDirY = 0;
		aDirX = 0;
	}

	if ( iBallPos.iY + iBallSize >= iScreenRect.iBr.iY )
	{
		iBallPos = iScreenRect.Center() - TPoint( iBallSize / 2, iBallSize / 2 );
		iBallVec = TPoint( 0, -8 );

		iGraceTime = 20;

		++iEnemyScore;
		if ( iEnemyScore > 999 )
		{
			iEnemyScore = 999;
		}

		aDirX = 0;
		aDirY = 0;
	}

	// Check horizontal collision
	if ( iBallPos.iX < iScreenRect.iTl.iX )
	{
		iSoundManager.PlayWav( static_cast< CSoundManager::TWavFile >(
			GetRandomNumber( iSeed, static_cast< TInt >( CSoundManager::EBallBeep2 ) ) )
		);

		iBallPos.iX = iScreenRect.iTl.iX;
		iBallVec.iX = -iBallVec.iX;

		aDirX = -aDirX;

	}

	if ( iBallPos.iX + iBallSize >= iScreenRect.iBr.iX )
	{
		iSoundManager.PlayWav( static_cast< CSoundManager::TWavFile >(
			GetRandomNumber( iSeed, static_cast< TInt >( CSoundManager::EBallBeep2 ) ) )
		);

		iBallPos.iX = iScreenRect.iBr.iX - iBallSize - 1;
		iBallVec.iX = -iBallVec.iX;

		aDirX = -aDirX;
	}

	/////////////////////////
	// Paddle collision
	/////////////////////////

	// Player paddle
	if ( iPlayerPos.iX < iScreenRect.iTl.iX )
	{
		iPlayerPos.iX = iScreenRect.iTl.iX;
	}

	else if ( iPlayerPos.iX + iPlayerSize.iWidth >= iScreenRect.iBr.iX )
	{
		iPlayerPos.iX = iScreenRect.iBr.iX - iPlayerSize.iWidth;
	}

	// Enemy paddle
	if ( iEnemyPos.iX < iScreenRect.iTl.iX )
	{
		iEnemyPos.iX = iScreenRect.iTl.iX;
	}

	else if ( iEnemyPos.iX + iPlayerSize.iWidth >= iScreenRect.iBr.iX )
	{
		iEnemyPos.iX = iScreenRect.iBr.iX - iPlayerSize.iWidth;
	}

	TRect BallRect = TRect( iBallPos, TSize( iBallSize, iBallSize ) );
	TRect PlayerRect = TRect( iPlayerPos, iPlayerSize );
	TRect EnemyRect = TRect( iEnemyPos, iPlayerSize );

	// Collision with player
	if ( BallRect.Intersects( PlayerRect ) )
	{
		iSoundManager.PlayWav( static_cast< CSoundManager::TWavFile >(
			GetRandomNumber( iSeed, static_cast< TInt >( CSoundManager::EBallBeep2 ) ) )
		);

		iBallPos.iY = iPlayerPos.iY - iBallSize;
		iBallVec.iY = -iBallVec.iY;

		TInt BallCenterX   = iBallPos.iX + ( iBallSize / 2 );
		TInt PlayerCenterX = iPlayerPos.iX + ( iPlayerSize.iWidth / 2 );

		iBallVec.iX = ( BallCenterX - PlayerCenterX ) / 3;

		aDirX = ( iBallVec.iX == 0 ) ? 0 : ( ( iBallVec.iX > 0 ) ? 1 : -1 );
		aDirY = ( iBallVec.iY == 0 ) ? 0 : ( ( iBallVec.iY > 0 ) ? 1 : -1 );
	}

	// Collision with enemy
	if ( BallRect.Intersects( EnemyRect ) )
	{
		iSoundManager.PlayWav( static_cast< CSoundManager::TWavFile >(
			GetRandomNumber( iSeed, static_cast< TInt >( CSoundManager::EBallBeep2 ) ) )
		);

		iBallPos.iY = iEnemyPos.iY + iPlayerSize.iHeight;
		iBallVec.iY = -iBallVec.iY;

		TInt BallCenterX   = iBallPos.iX + ( iBallSize / 2 );
		TInt EnemyCenterX = iEnemyPos.iX + ( iPlayerSize.iWidth / 2 );
		
		iBallVec.iX = ( BallCenterX - EnemyCenterX ) / 3;

		aDirX = ( iBallVec.iX == 0 ) ? 0 : ( ( iBallVec.iX > 0 ) ? 1 : -1 );
		aDirY = ( iBallVec.iY == 0 ) ? 0 : ( ( iBallVec.iY > 0 ) ? 1 : -1 );
	}
}

// Performs collision detection and updates the position of paddles and ball
void CEngine::UpdateEngine()
{
	DirectPlayer();
	DirectEnemy();

	// Movement speeds
	TInt StepsP = Abs( iPlayerSpeed );
	TInt StepsE = Abs( iEnemySpeed );
	
	TInt StepsX = Abs( iBallVec.iX );
	TInt StepsY = Abs( iBallVec.iY );
	if ( iGraceTime > 0 )
	{
		--iGraceTime;
		StepsX = 0;
		StepsY = 0;
	}

	TInt Steps = Max( StepsP, Max( StepsE, Max( StepsX, StepsY ) ) );

	if ( Steps == 0 )
	{
		return;
	}

	// Direction (-1, 0 or 1)
	TInt DirP = ( iPlayerSpeed == 0 ) ? 0 : ( ( iPlayerSpeed < 0 ) ? -1 : 1 );
	TInt DirE = ( iEnemySpeed == 0 ) ? 0 : ( ( iEnemySpeed < 0 ) ? -1 : 1 );
	TInt DirX = ( iBallVec.iX == 0 ) ? 0 : ( ( iBallVec.iX < 0 ) ? -1 : 1 );
	TInt DirY = ( iBallVec.iY == 0 ) ? 0 : ( ( iBallVec.iY < 0 ) ? -1 : 1 );

	for ( TInt i = 0; i < Steps; ++i )
	{
		if ( StepsE > 0 || StepsP > 0 )
		{
			if ( StepsP > 0 )
			{
				iPlayerPos.iX += DirP;
				--StepsP;
			}

			if ( StepsE > 0 )
			{
				iEnemyPos.iX += DirE;
				--StepsE;
			}

			CheckCollision( DirX, DirY );
		}

		if ( StepsX > 0 )
		{
			iBallPos.iX += DirX;
			--StepsX;

			CheckCollision( DirX, DirY );
		}

		if ( StepsY > 0 )
		{
			iBallPos.iY += DirY;
			--StepsY;

			CheckCollision( DirX, DirY );
		}
	}
}

TKeyHandler &CEngine::KeyHandler()
{
	return iKeyHandler;
}

const TKeyHandler &CEngine::KeyHandler() const
{
	return iKeyHandler;
}

TInt CEngine::GetPlayerScore() const
{
	return iPlayerScore;
}

//////////////////
// CGameLoop
//////////////////

CGameLoop *CGameLoop::NewL( CEngine &aEngine, CDSAWrapper &aDSA, CRenderer &aRenderer, CSoundManager &aSoundManager )
{
	CGameLoop *self = NewLC( aEngine, aDSA, aRenderer, aSoundManager );
	CleanupStack::Pop( self );
	return self;
}

CGameLoop *CGameLoop::NewLC( CEngine &aEngine, CDSAWrapper &aDSA, CRenderer &aRenderer, CSoundManager &aSoundManager )
{
	CGameLoop *self = new ( ELeave ) CGameLoop( aEngine, aDSA, aRenderer, aSoundManager );
	CleanupStack::PushL( self );
	self->ConstructL();
	return self;
}

CGameLoop::~CGameLoop()
{
	Cancel();
}

// @param aInterval the time in ms between timer events
void CGameLoop::Start( TTimeIntervalMicroSeconds32 aInterval )
{
	Cancel();
	iInterval = aInterval;
	After( iInterval );
}

CGameLoop::CGameLoop( CEngine &aEngine, CDSAWrapper &aDSA, CRenderer &aRenderer, CSoundManager &aSoundManager ) :
CTimer( EPriorityHigh ), iEngine( aEngine ), iDSA( aDSA ), iRenderer( aRenderer ), iPaused( EFalse ), iSoundManager( aSoundManager ), iSoundQueued( EFalse ) {}

// From CActive. Updates the game engine and screen
void CGameLoop::RunL()
{
	// Dynamic wait time calculation
	TTime BeforeFrame;
	TTime AfterFrame;
	BeforeFrame.UniversalTime();

	if ( !iPaused )
	{
		// Play sound queued by container
		if ( iSoundQueued )
		{
			iSoundQueued = EFalse;
			iSoundManager.PlayWav( iQueuedWavFile );
		}

		// Instantiates DSA if not already present
		if ( !iDSA.DSAAvailable() )
		{
			iDSA.InstantiateDSAL( *this );
		}

		iDSA.StartDSAL();

		if ( iRenderer.GetScreenType() == CRenderer::EScreenGame )
		{
			iEngine.UpdateEngine();
		}

		iRenderer.RenderToBackBuffer();
		iRenderer.BlitToFrontBuffer( iDSA.DSAGc() );

		iDSA.UpdateScreenDevice();
	}

	AfterFrame.UniversalTime();

	// Calculate how long the frame took to finish
	TTimeIntervalMicroSeconds Elapsed = AfterFrame.MicroSecondsFrom( BeforeFrame );

	if ( Elapsed.Int64().Low() < static_cast< TUint32 >( iInterval.Int() ) )
	{
		TInt32 SleepTime = iInterval.Int() - Elapsed.Int64().Low();
		After( SleepTime );
	}
	else
	{
		// Frame drop! We took too long to render
		After( 1 ); 
	}
}

void CGameLoop::ConstructL()
{
	CTimer::ConstructL();
	CActiveScheduler::Add( this );
}

// From CActive. Called if RunL leaves
TInt CGameLoop::RunError( TInt /*aError*/ )
{
	return KErrNone;
}

// From CActive
void CGameLoop::DoCancel()
{
	iDSA.CancelDSA();
	iDSA.DeleteDSA();
	CTimer::DoCancel();
}

// Sets the paused flag. If this is set, the game engine and screen aren't updated
void CGameLoop::SetPaused( TBool aFlag )
{
	iPaused = aFlag;

	// Show the pause menu when we come back
	if ( iRenderer.GetScreenType() == CRenderer::EScreenGame )
	{
		iRenderer.SetScreenType( CRenderer::EScreenPaused );
	}
}

// Called by the container
void CGameLoop::QueueSoundPlayback( CSoundManager::TWavFile aWavFileChosen )
{
	iSoundQueued = ETrue;
	iQueuedWavFile = aWavFileChosen;
}

// From MDirectScreenAccess
void CGameLoop::Restart( RDirectScreenAccess::TTerminationReasons /*aReason*/ )
{
	iPaused = EFalse;
}

// From MDirectScreenAccess
void CGameLoop::AbortNow( RDirectScreenAccess::TTerminationReasons /*aReason*/ )
{
	iDSA.CancelDSA();
	iPaused = ETrue;
}

//////////////
// CBackBuffer
//////////////

CBackBuffer::CBackBuffer( TSize /*aSize*/, TDisplayMode /*aDisplayMode*/ ) {}

CBackBuffer *CBackBuffer::NewL( TSize aSize, TDisplayMode aDisplayMode )
{
	CBackBuffer *self = NewLC( aSize, aDisplayMode );
	CleanupStack::Pop( 3, self );
	return self;
}

CBackBuffer *CBackBuffer::NewLC( TSize aSize, TDisplayMode aDisplayMode )
{
	CBackBuffer *self = new ( ELeave ) CBackBuffer( aSize, aDisplayMode );
	CleanupStack::PushL( self );

	self->ConstructL( aSize, aDisplayMode );
	return self;
}

void CBackBuffer::ConstructL( TSize aSize, TDisplayMode aDisplayMode )
{
	iBackBufferBitmap = new ( ELeave ) CFbsBitmap();
	iBackBufferBitmap->Create( aSize, aDisplayMode );

	CleanupStack::PushL( iBackBufferBitmap );

	iBackBufferBitmapDevice = CFbsBitmapDevice::NewL( iBackBufferBitmap );
	CleanupStack::PushL( iBackBufferBitmapDevice );

	iBackBufferBitmapDevice->CreateContext( iBackBufferGc );
}

const CFbsBitmap &CBackBuffer::GetBackBufferBitmap() const
{
	return *iBackBufferBitmap;
}

CFbsBitGc *CBackBuffer::GetBackBufferGc() const
{
	return iBackBufferGc;
}

CBackBuffer::~CBackBuffer()
{
	delete iBackBufferBitmap;
	delete iBackBufferBitmapDevice;
	delete iBackBufferGc;
}

////////////////
// CRenderer
////////////////

CRenderer::CRenderer( CEngine &aEngine, TSize aSize, CScoreManager &aScoreManager, CSettingsManager &aSettingsManager ) :
iScreenType( EScreenMainMenu ), iEngine( aEngine ), iScreenSize( aSize ), iHighlightedMenuItem( 0 ),
iScoreManager( aScoreManager ), iSettingsManager( aSettingsManager ) {}

CRenderer *CRenderer::NewL( CEngine &aEngine, TSize aSize, CScoreManager &aScoreManager, CSettingsManager &aSettingsManager )
{
	CRenderer *self = CRenderer::NewLC( aEngine, aSize, aScoreManager, aSettingsManager );
	CleanupStack::Pop( self );
	return self;
}

CRenderer *CRenderer::NewLC( CEngine &aEngine, TSize aSize, CScoreManager &aScoreManager, CSettingsManager &aSettingsManager )
{
	CRenderer *self = new ( ELeave ) CRenderer( aEngine, aSize, aScoreManager, aSettingsManager );
	CleanupStack::PushL( self );
	
	self->ConstructL( aEngine, aSize, aScoreManager, aSettingsManager );
	return self;
}

void CRenderer::ConstructL( CEngine &/*aEngine*/, TSize aSize, CScoreManager &/*aScoreManager*/, CSettingsManager &/*aSettingsManager*/ )
{
	iBackBuffer = CBackBuffer::NewL( aSize, CEikonEnv::Static()->ScreenDevice()->DisplayMode() );

	iBitmapManager = CBitmapManager::NewL();

	iFrameCounter = CFrameCounter::NewL();
}

CRenderer::~CRenderer()
{
	delete iBackBuffer;
	delete iBitmapManager;
	delete iFrameCounter;
}

void CRenderer::BlitToFrontBuffer( CFbsBitGc *aDSAGc ) const
{
	CFbsBitGc *BackBufferGc = iBackBuffer->GetBackBufferGc();
	if ( aDSAGc && BackBufferGc )
	{
		aDSAGc->BitBlt( TPoint( 0, 0 ), *BackBufferGc );
	}
}

void CRenderer::RenderGame() const
{
	CFbsBitGc *BackBufferGc = iBackBuffer->GetBackBufferGc();

	if ( !BackBufferGc )
	{
		return;
	}

	BackBufferGc->SetPenStyle( CGraphicsContext::ENullPen );
	BackBufferGc->SetPenColor( KRgbGray );
	BackBufferGc->SetBrushStyle( CGraphicsContext::ESolidBrush );
	BackBufferGc->SetBrushColor( KRgbBlack );

	// Clear the screen with black
	BackBufferGc->Clear();

	// Draw score
	const CFont *ScoreFont = CEikonEnv::Static()->NormalFont();

	BackBufferGc->SetPenStyle( CGraphicsContext::ESolidPen );
	BackBufferGc->UseFont( ScoreFont );

	TBuf< 3 > PlayerScoreBuf;
	PlayerScoreBuf.Num( iEngine.iPlayerScore );
	TPoint PlayerScorePos = TPoint( iEngine.iScreenRect.Width() / 10, iEngine.iScreenRect.Height() / 2 + ScoreFont->HeightInPixels() / 2 + ScoreFont->HeightInPixels() );

	if ( iEngine.iPlayerScore >= 999 )
	{
		BackBufferGc->SetPenColor( KRgbYellow );
		BackBufferGc->DrawText( PlayerScoreBuf, PlayerScorePos );
		BackBufferGc->SetPenColor( KRgbGray );
	}
	else
	{
		BackBufferGc->DrawText( PlayerScoreBuf, PlayerScorePos );
	}

	TBuf< 3 > EnemyScoreBuf;
	EnemyScoreBuf.Num( iEngine.iEnemyScore );
	TPoint EnemyScorePos = TPoint( iEngine.iScreenRect.Width() / 10, iEngine.iScreenRect.Height() / 2 - ScoreFont->HeightInPixels() / 2 );

	if ( iEngine.iEnemyScore >= 999 )
	{
		BackBufferGc->SetPenColor( KRgbYellow );
		BackBufferGc->DrawText( EnemyScoreBuf, EnemyScorePos );
		BackBufferGc->SetPenColor( KRgbGray );
	}
	else
	{
		BackBufferGc->DrawText( EnemyScoreBuf, EnemyScorePos );
	}

	// Draw FPS count if toggled on
	if ( iSettingsManager.iSettingsItems[ 1 ].iToggle )
	{
		TBuf< 3 > FrameCountBuf;
		FrameCountBuf.Num( iFrameCounter->GetFrameCount() );
		TPoint FrameCountPos = TPoint( iEngine.iScreenRect.Width() - ScoreFont->TextWidthInPixels( FrameCountBuf ),
									   iEngine.iScreenRect.Height()  - ScoreFont->HeightInPixels() );
	
		BackBufferGc->DrawText( FrameCountBuf, FrameCountPos );
	}

	BackBufferGc->DiscardFont();

	// Draw dividing line
	BackBufferGc->SetPenStyle( CGraphicsContext::EDottedPen );
	TPoint LineStart = TPoint( 0, iEngine.iScreenRect.Height() / 2 );
	TPoint LineEnd = TPoint( iEngine.iScreenRect.Width(), iEngine.iScreenRect.Height() / 2 );

	BackBufferGc->DrawLine( LineStart, LineEnd );
	
	BackBufferGc->SetPenStyle( CGraphicsContext::ENullPen );

	// Draw the ball
	TRect BallMaskRect( TPoint( 0, 0 ), TSize( 8, 8 ) );
	BackBufferGc->BitBltMasked( iEngine.iBallPos, iBitmapManager->iBallBitmap, BallMaskRect, iBitmapManager->iBallMask, ETrue );

	// Draw the Player paddle
	TRect PaddleMaskRect( TPoint( 0, 0 ), iEngine.iPlayerSize ); // From top left of bitmap
	BackBufferGc->BitBltMasked( iEngine.iPlayerPos, iBitmapManager->iPaddleBitmap, PaddleMaskRect, iBitmapManager->iPaddleMask, ETrue );

	// Draw the Enemy paddle
	BackBufferGc->BitBltMasked( iEngine.iEnemyPos, iBitmapManager->iPaddleBitmap, PaddleMaskRect, iBitmapManager->iPaddleMask, ETrue );
}

void CRenderer::RenderMainMenu() const
{
	CFbsBitGc *BackBufferGc = iBackBuffer->GetBackBufferGc();

	if ( !BackBufferGc )
	{
		return;
	}

	RArray< TBufC< 20 > > MenuItems; // Default granularity is 8
	CleanupClosePushL( MenuItems );

	for ( TInt i = 0; i < 2; ++i )
	{
		User::LeaveIfError( MenuItems.Append( TBufC< 20 >() ) );
	}

	MenuItems[ 0 ] = KStartGameText();
	MenuItems[ 1 ] = KHighScoresText();

	BackBufferGc->SetBrushColor( KRgbBlack );
	BackBufferGc->Clear();

	// Draw the game logo
	TPoint LogoPos = TPoint( KLogoPaddingHorizontal, KLogoPaddingVertical );
	BackBufferGc->BitBlt( LogoPos, iBitmapManager->iMainMenuLogoBitmap );

	const CFont * MenuFont = CEikonEnv::Static()->NormalFont();

	BackBufferGc->UseFont( MenuFont );
	BackBufferGc->SetPenStyle( CGraphicsContext::ESolidPen );
	BackBufferGc->SetPenColor( KRgbWhite );

	for ( i = 0; i < 2; ++i )
	{
		TPoint TextPos = TPoint( iEngine.iScreenRect.Width() / 2 - ( MenuFont->TextWidthInPixels( MenuItems[ i ] ) / 2 ),
								 ( iEngine.iScreenRect.Height() / 5 ) * ( i + 3 ) - MenuFont->HeightInPixels() );

		if ( i == iHighlightedMenuItem )
		{
			BackBufferGc->SetPenColor( KRgbYellow );

			BackBufferGc->DrawText( MenuItems[ i ], TextPos );

			BackBufferGc->SetPenColor( KRgbWhite );
		}
		else
		{
			BackBufferGc->DrawText( MenuItems[ i ], TextPos );
		}

	}

	TBufC< 7 > OptionsTxtBuf = KOptionsText();
	TBufC< 4 > ExitTxtBuf = KExitText();

	TPoint OptionsPos = TPoint( KCBAPaddingHorizontal, iEngine.iScreenRect.Height() - KCBAPaddingVertical );
	TPoint ExitPos = TPoint( iEngine.iScreenRect.Width() - MenuFont->TextWidthInPixels( ExitTxtBuf ) - KCBAPaddingHorizontal,
							 iEngine.iScreenRect.Height() - KCBAPaddingVertical );

	BackBufferGc->DrawText( OptionsTxtBuf, OptionsPos );
	BackBufferGc->DrawText( ExitTxtBuf, ExitPos );

	BackBufferGc->DiscardFont();
	MenuItems.Reset();
	CleanupStack::Pop( &MenuItems );
}

void CRenderer::RenderPaused() const
{
	CFbsBitGc *BackBufferGc = iBackBuffer->GetBackBufferGc();

	if ( !BackBufferGc )
	{
		return;
	}

	RArray< TBufC< 20 > > MenuItems;
	CleanupClosePushL( MenuItems );

	_LIT( KStartGameText, "Resume Game" );
	_LIT( KHighScoresText, "Back To Menu" );

	for ( TInt i = 0; i < 2; ++i )
	{
		User::LeaveIfError( MenuItems.Append( TBufC< 20 >() ) );
	}

	MenuItems[ 0 ] = KStartGameText;
	MenuItems[ 1 ] = KHighScoresText;

	const CFont *MenuFont = CEikonEnv::Static()->NormalFont();

	BackBufferGc->SetBrushColor( KRgbBlack );
	BackBufferGc->UseFont( MenuFont );
	BackBufferGc->SetPenStyle( CGraphicsContext::ESolidPen );
	BackBufferGc->SetPenColor( KRgbWhite );

	// Draw pause menu
	TPoint MenuPos = TPoint( iEngine.iScreenRect.Width() / 8, iEngine.iScreenRect.Height() / 3 );
	TSize MenuSize( ( iEngine.iScreenRect.Width() / 8 ) * 6, iEngine.iScreenRect.Height() / 3 );

	BackBufferGc->DrawRect( TRect( MenuPos, MenuSize ) );

	TInt MenuHeightStep = ( MenuPos.iY + MenuSize.iHeight - MenuPos.iY ) / 5;

	for ( i = 0; i < 2; ++i )
	{
		TPoint TextPos = TPoint( iEngine.iScreenRect.Width() / 2 - ( MenuFont->TextWidthInPixels( MenuItems[ i ] ) / 2 ),
								 ( MenuPos.iY ) + 2 * ( i + 1 ) * MenuHeightStep );

		if ( i == iHighlightedMenuItem )
		{
			BackBufferGc->SetPenColor( KRgbYellow );

			BackBufferGc->DrawText( MenuItems[ i ], TextPos );

			BackBufferGc->SetPenColor( KRgbWhite );
		}
		else
		{
			BackBufferGc->DrawText( MenuItems[ i ], TextPos );
		}

	}

	BackBufferGc->DiscardFont();
	MenuItems.Reset();
	CleanupStack::Pop( &MenuItems );
}


void CRenderer::RenderHighScores() const
{
	CFbsBitGc *BackBufferGc = iBackBuffer->GetBackBufferGc();

	if ( !BackBufferGc )
	{
		return;
	}

	_LIT( KTxtHighScores, "High Scores" );
	_LIT( KTxtReset, "Reset" );
	_LIT( KTxtBack, "Back" );

	BackBufferGc->SetBrushColor( KRgbBlack );
	BackBufferGc->Clear();

	const CFont * MenuFont = CEikonEnv::Static()->NormalFont();

	BackBufferGc->UseFont( MenuFont );
	BackBufferGc->SetPenStyle( CGraphicsContext::ESolidPen );
	BackBufferGc->SetPenColor( KRgbWhite );

	TBufC< 11 > HighScoresTxtBuf = KTxtHighScores();
	TPoint HighScoresPos = TPoint( iEngine.iScreenRect.Width() / 2 - ( MenuFont->TextWidthInPixels( HighScoresTxtBuf ) / 2 ),
								   iEngine.iScreenRect.Height() / 10 );

	BackBufferGc->DrawText( HighScoresTxtBuf, HighScoresPos );

	RArray< TScoreTuple > &ScoreList = iScoreManager.iScoreList;
	TBuf< 3 > ScoreBuf;
	for ( TInt i = 0; i < ScoreList.Count(); ++i )
	{
		ScoreBuf.NumFixedWidth( ScoreList[ i ].iScore, EDecimal, 3 ); // 3 is the width -> pad with zeroes
		TPoint ScorePos = TPoint( iEngine.iScreenRect.Width() / 4 - MenuFont->TextWidthInPixels( ScoreBuf ),
								 ( iEngine.iScreenRect.Height() / 10 ) * ( i + 2 ) );
		TPoint NamePos = TPoint( ( iEngine.iScreenRect.Width() / 4 ) * 3,
								 ( iEngine.iScreenRect.Height() / 10 ) * ( i + 2 ) );
		BackBufferGc->DrawText( ScoreBuf, ScorePos );
		BackBufferGc->DrawText( ScoreList[ i ].iPlayerName, NamePos );
	}

	TBufC< 5 > ResetTxtBuf = KTxtReset();
	TBufC< 4 > BackTxtBuf = KTxtBack();

	TPoint ResetPos = TPoint( KCBAPaddingHorizontal, iEngine.iScreenRect.Height() - KCBAPaddingVertical );
	TPoint BackPos = TPoint( iEngine.iScreenRect.Width() - MenuFont->TextWidthInPixels( BackTxtBuf ) - KCBAPaddingHorizontal,
							 iEngine.iScreenRect.Height() - KCBAPaddingVertical );

	BackBufferGc->DrawText( ResetTxtBuf, ResetPos );
	BackBufferGc->DrawText( BackTxtBuf, BackPos );

	BackBufferGc->DiscardFont();
}

void CRenderer::RenderOptions() const
{
	CFbsBitGc *BackBufferGc = iBackBuffer->GetBackBufferGc();

	if ( !BackBufferGc )
	{
		return;
	}

	_LIT( KTxtOptions, "Options" );
	_LIT( KTxtReset, "Reset" );
	_LIT( KTxtBack, "Back" );

	_LIT( KTxtOn, "On" );
	_LIT( KTxtOff, "Off" );

	BackBufferGc->SetBrushColor( KRgbBlack );
	BackBufferGc->Clear();

	const CFont * MenuFont = CEikonEnv::Static()->NormalFont();

	BackBufferGc->UseFont( MenuFont );
	BackBufferGc->SetPenStyle( CGraphicsContext::ESolidPen );
	BackBufferGc->SetPenColor( KRgbWhite );

	TBufC< 7 > OptionsTxtBuf = KTxtOptions();
	TPoint OptionsPos = TPoint( iEngine.iScreenRect.Width() / 2 - ( MenuFont->TextWidthInPixels( OptionsTxtBuf ) / 2 ),
								   iEngine.iScreenRect.Height() / 10 );

	BackBufferGc->DrawText( OptionsTxtBuf, OptionsPos );

	RArray< TSettingsItem > &SettingsItems = iSettingsManager.iSettingsItems;

	// Framerate setting
	TBufC< 20 > FramerateTxtBuf = SettingsItems[ 0 ].iItemName;
	TBufC< 20 > FramerateOptionTxtBuf = SettingsItems[ 0 ].iValueNames[ SettingsItems[ 0 ].iRadioIndex ];

	TPoint FramerateNamePos = TPoint( iEngine.iScreenRect.Width() / 10,
							  ( iEngine.iScreenRect.Height() / 4 ) * ( 1 ) );
	TPoint FramerateValuePos = TPoint( ( iEngine.iScreenRect.Width() / 10 ) * 8 - MenuFont->TextWidthInPixels( FramerateOptionTxtBuf ) / 2,
							 ( iEngine.iScreenRect.Height() / 4 ) * ( 1 ) );

	if ( iHighlightedMenuItem == 0 )
	{
		BackBufferGc->SetPenColor( KRgbYellow );

		TPoint LeftArrowPos = TPoint( FramerateValuePos.iX - MenuFont->HeightInPixels(), FramerateValuePos.iY - MenuFont->HeightInPixels() );
		TPoint RightArrowPos = TPoint( FramerateValuePos.iX + MenuFont->TextWidthInPixels( FramerateOptionTxtBuf ), FramerateValuePos.iY - MenuFont->HeightInPixels() );

		TRect ArrowMaskRect( TPoint( 0, 0 ), TSize( 7, 13 ) );
		BackBufferGc->BitBltMasked( LeftArrowPos, iBitmapManager->iLeftArrowBitmap, ArrowMaskRect, iBitmapManager->iLeftArrowMask, ETrue );
		BackBufferGc->BitBltMasked( RightArrowPos, iBitmapManager->iRightArrowBitmap, ArrowMaskRect, iBitmapManager->iRightArrowMask, ETrue );
	}
	else
	{
		BackBufferGc->SetPenColor( KRgbWhite );
	}

	BackBufferGc->DrawText( FramerateTxtBuf, FramerateNamePos );
	BackBufferGc->DrawText( FramerateOptionTxtBuf, FramerateValuePos );

	// Frame counter setting
	TBufC< 20 > FrameCounterTxtBuf = SettingsItems[ 1 ].iItemName;
	TBufC< 3 > FrameCounterOptionTxtBuf;
	if ( SettingsItems[ 1 ].iToggle )
	{
		FrameCounterOptionTxtBuf = KTxtOn();
	}
	else
	{
		FrameCounterOptionTxtBuf = KTxtOff();
	}

	TPoint FrameCounterNamePos = TPoint( iEngine.iScreenRect.Width() / 10,
							  ( iEngine.iScreenRect.Height() / 4 ) * ( 2 ) );
	TPoint FrameCounterValuePos = TPoint( ( iEngine.iScreenRect.Width() / 10 ) * 8 - MenuFont->TextWidthInPixels( FrameCounterOptionTxtBuf ) / 2,
							 ( iEngine.iScreenRect.Height() / 4 ) * ( 2 ) );

	if ( iHighlightedMenuItem == 1 )
	{
		BackBufferGc->SetPenColor( KRgbYellow );

		TPoint LeftArrowPos = TPoint( FrameCounterValuePos.iX - MenuFont->HeightInPixels(), FrameCounterValuePos.iY - MenuFont->HeightInPixels() );
		TPoint RightArrowPos = TPoint( FrameCounterValuePos.iX + MenuFont->TextWidthInPixels( FrameCounterOptionTxtBuf ), FrameCounterValuePos.iY - MenuFont->HeightInPixels() );

		TRect ArrowMaskRect( TPoint( 0, 0 ), TSize( 7, 13 ) );
		BackBufferGc->BitBltMasked( LeftArrowPos, iBitmapManager->iLeftArrowBitmap, ArrowMaskRect, iBitmapManager->iLeftArrowMask, ETrue );
		BackBufferGc->BitBltMasked( RightArrowPos, iBitmapManager->iRightArrowBitmap, ArrowMaskRect, iBitmapManager->iRightArrowMask, ETrue );

	}
	else
	{
		BackBufferGc->SetPenColor( KRgbWhite );
	}

	BackBufferGc->DrawText( FrameCounterTxtBuf, FrameCounterNamePos );
	BackBufferGc->DrawText( FrameCounterOptionTxtBuf, FrameCounterValuePos );

	BackBufferGc->SetPenColor( KRgbWhite );

	// Sound setting
	TBufC< 20 > SoundTxtBuf = SettingsItems[ 2 ].iItemName;
	TBufC< 3 > SoundOptionTxtBuf;
	if ( SettingsItems[ 2 ].iToggle )
	{
		SoundOptionTxtBuf = KTxtOn();
	}
	else
	{
		SoundOptionTxtBuf = KTxtOff();
	}

	TPoint SoundNamePos = TPoint( iEngine.iScreenRect.Width() / 10,
							  ( iEngine.iScreenRect.Height() / 4 ) * ( 3 ) );
	TPoint SoundValuePos = TPoint( ( iEngine.iScreenRect.Width() / 10 ) * 8 - MenuFont->TextWidthInPixels( SoundOptionTxtBuf ) / 2,
							 ( iEngine.iScreenRect.Height() / 4 ) * ( 3 ) );

	if ( iHighlightedMenuItem == 2 )
	{
		BackBufferGc->SetPenColor( KRgbYellow );

		TPoint LeftArrowPos = TPoint( SoundValuePos.iX - MenuFont->HeightInPixels(), SoundValuePos.iY - MenuFont->HeightInPixels() );
		TPoint RightArrowPos = TPoint( SoundValuePos.iX + MenuFont->TextWidthInPixels( SoundOptionTxtBuf ), SoundValuePos.iY - MenuFont->HeightInPixels() );

		TRect ArrowMaskRect( TPoint( 0, 0 ), TSize( 7, 13 ) );
		BackBufferGc->BitBltMasked( LeftArrowPos, iBitmapManager->iLeftArrowBitmap, ArrowMaskRect, iBitmapManager->iLeftArrowMask, ETrue );
		BackBufferGc->BitBltMasked( RightArrowPos, iBitmapManager->iRightArrowBitmap, ArrowMaskRect, iBitmapManager->iRightArrowMask, ETrue );

	}
	else
	{
		BackBufferGc->SetPenColor( KRgbWhite );
	}

	BackBufferGc->DrawText( SoundTxtBuf, SoundNamePos );
	BackBufferGc->DrawText( SoundOptionTxtBuf, SoundValuePos );

	BackBufferGc->SetPenColor( KRgbWhite );

	// CBA
	TBufC< 5 > ResetTxtBuf = KTxtReset();
	TBufC< 4 > BackTxtBuf = KTxtBack();

	TPoint ResetPos = TPoint( KCBAPaddingHorizontal, iEngine.iScreenRect.Height() - KCBAPaddingVertical );
	TPoint BackPos = TPoint( iEngine.iScreenRect.Width() - MenuFont->TextWidthInPixels( BackTxtBuf ) - KCBAPaddingHorizontal,
							 iEngine.iScreenRect.Height() - KCBAPaddingVertical );

	BackBufferGc->DrawText( ResetTxtBuf, ResetPos );
	BackBufferGc->DrawText( BackTxtBuf, BackPos );

	BackBufferGc->DiscardFont();
}

void CRenderer::RenderToBackBuffer() const
{
	// Check for FPS counter toggle
	if ( iSettingsManager.iSettingsItems[ 1 ].iToggle )
	{
		iFrameCounter->IncrementCounter();
	}

	switch( iScreenType )
	{
		case EScreenMainMenu:
			RenderMainMenu();
			break;

		case EScreenGame:
			RenderGame();
			break;

		case EScreenPaused:
			RenderPaused();
			break;

		case EScreenHighScores:
			RenderHighScores();
			break;

		case EScreenOptions:
			RenderOptions();
			break;

		default:
			break;
	}
}

void CRenderer::SetScreenType( TScreenType aScreenType )
{
	iScreenType = aScreenType;
}

const CRenderer::TScreenType &CRenderer::GetScreenType() const
{
	return iScreenType;
}

//////////////
// CDSAWrapper
//////////////

CDSAWrapper *CDSAWrapper::NewL( RWsSession &aWs, CWsScreenDevice &aScreenDevice, RWindowBase &aWindow )
{
	CDSAWrapper *self = NewLC( aWs, aScreenDevice, aWindow );
	CleanupStack::Pop( self );
	return self;
}

CDSAWrapper *CDSAWrapper::NewLC( RWsSession &aWs, CWsScreenDevice &aScreenDevice, RWindowBase &aWindow )
{
	CDSAWrapper *self = new ( ELeave ) CDSAWrapper( aWs, aScreenDevice, aWindow );
	CleanupStack::PushL( self );
	self->ConstructL();
	return self;
}

// Indicates if DSA is available
TBool CDSAWrapper::DSAAvailable() const
{
	return iDSA != NULL;
}

// Accesses the graphic context of the DSA object
CFbsBitGc *CDSAWrapper::DSAGc()
{
	if ( !iDSA )
	{
		return NULL;
	}

	return iDSA->Gc();
}

// Instantiates the DSA object
void CDSAWrapper::InstantiateDSAL( MDirectScreenAccess &aObserver )
{
	if ( !iDSA )
	{
		iDSA = CDirectScreenAccess::NewL( iWs, iScreenDevice, iWindow, aObserver );
	}
}

// Deletes the DSA object
void CDSAWrapper::DeleteDSA()
{
	delete iDSA;
	iDSA = NULL;
}

// Starts the direct screen access
void CDSAWrapper::StartDSAL()
{
	if ( iDSA && !iDSA->IsActive() )
	{
		iDSA->StartL();
	}
}

// Cancels the direct screen access
void CDSAWrapper::CancelDSA()
{
	if ( iDSA && iDSA->IsActive() )
	{
		iDSA->Cancel();
	}
}

void CDSAWrapper::ConstructL() {}

CDSAWrapper::CDSAWrapper( RWsSession &aWs, CWsScreenDevice &aScreenDevice, RWindowBase &aWindow ) :
iWs( aWs ), iScreenDevice( aScreenDevice ), iWindow( aWindow ) {}

// Destructor
CDSAWrapper::~CDSAWrapper()
{
	delete iDSA;
}

// Indicates if DSA is active
TBool CDSAWrapper::DSAActive() const
{
	if ( iDSA )
	{
		return iDSA->IsActive();
	}

	return EFalse;
}

// Updates the screen device
void CDSAWrapper::UpdateScreenDevice()
{
	if ( DSAActive() )
	{
		iDSA->ScreenDevice()->Update();
	}
}