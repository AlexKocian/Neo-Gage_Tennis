// INCLUDE FILES

// Class include
#include "GameContainer.h"
#include "Game.h"
#include "SoundManager.h"

// System includes
#include <eiklabel.h> // CEikLabel
#include <NGTS.rsg> // Resources
#include <stringloader.h> // StringLoader

#include <eikenv.h>
#include <avkon.hrh> // EAknSoftkeyExit

// User includes
#include "KeyHandler.h"

// FORWARD DECLARATIONS

// CONSTANTS

// ================= MEMBER FUNCTIONS ===================

// Symbian OS 2nd phase constructor. Constructs the CGameContainer using the
// NewLC method, popping the constructing object from the Cleanup Stack before
// returning it (thereby transfering ownership to the AppUi)
CGameContainer *CGameContainer::NewL( const TRect &aRect )
{
	CGameContainer *self = CGameContainer::NewLC( aRect );
	CleanupStack::Pop( self );
	return self;
}

// Symbian OS 2nd phase constructor. Constructs the CGameContainer using the
// constructor and ConstructL method, leaving the constructed object on the Cleanup Stack
// before returning it.
CGameContainer *CGameContainer::NewLC( const TRect &aRect )
{
	CGameContainer *self = new ( ELeave ) CGameContainer( aRect );
	CleanupStack::PushL( self );

	self->ConstructL( aRect );
	return self;
}

// Symbian OS 2nd phase constructor. Creates a Window for the controls, which it contains.
// Constructs a label and adds it to the window, which it then activates.
void CGameContainer::ConstructL( const TRect &aRect )
{
	CreateWindowL();

	Window().SetBackgroundColor( KRgbBlack );

	SetRect( aRect );

	iSettingsManager = CSettingsManager::NewL();

	iSoundManager = CSoundManager::NewL( *this, *iSettingsManager ); // Passing ourselves as the observer

	iEngine = CEngine::NewL( Rect(), *iSoundManager );

	iScoreManager = CScoreManager::NewL();
	
	iRenderer = CRenderer::NewL( *iEngine, Rect().Size(), *iScoreManager, *iSettingsManager );

	iDSAWrapper = CDSAWrapper::NewL( iEikonEnv->WsSession(), *( iEikonEnv->ScreenDevice() ), Window() );

	iLoop = CGameLoop::NewL( *iEngine, *iDSAWrapper, *iRenderer, *iSoundManager );

	ActivateL();
}

CGameContainer::CGameContainer( const TRect &/*aRect*/ ) {}

// Destructor.
CGameContainer::~CGameContainer()
{
	delete iEngine;
	delete iDSAWrapper;
	delete iLoop;
	delete iRenderer;
	delete iScoreManager;
	delete iSettingsManager;
	delete iSoundManager;
}

// Currently we just give up
void CGameContainer::HandlePlayingStoppedL()
{}

// Draw this application's view to the screen
// Not needed, because we aren't using WSERV and instead drawing to the screen's buffer directly
void CGameContainer::Draw( TRect &/*aRect*/ ) const
{}

void CGameContainer::StopDSA()
{
	// Stop the timer if it is active
	if ( iLoop && iLoop->IsActive() )
	{
		iLoop->Cancel();
	}
}

void CGameContainer::StartDSA()
{
	iLoop->SetPaused( EFalse );

	StopDSA();

	TTimeIntervalMicroSeconds32 FrameTime = GetFrameTime();

	// If the timer is not already running, start it
	if ( !iLoop->IsActive() )
	{
		iLoop->Start( FrameTime );
	}
}

TTimeIntervalMicroSeconds32 CGameContainer::GetFrameTime() const
{	
	TTimeIntervalMicroSeconds32 FrameTime = 0;
	switch( iSettingsManager->iSettingsItems[ 0 ].iRadioIndex ) // Framerate setting
	{
		case 0:
			FrameTime = 31000; // 30 FPS
			break;
		case 1:
			FrameTime = 15000; // 60 FPS
			break;
		case 2:
			FrameTime = 1;// Uncapped
			break;
		default:
			FrameTime = 1;
			break;
	}

	return FrameTime;
}

TKeyResponse CGameContainer::OfferKeyEventL( const TKeyEvent &aKeyEvent, TEventCode aType )
{
	TKeyHandler &MyKeyHandler = iEngine->KeyHandler();
	
	TTimeIntervalMicroSeconds32 FrameTime = GetFrameTime();

	if ( iLoop && !iLoop->IsActive() )
	{
		return EKeyWasNotConsumed;
	}

	if ( !iRenderer )
	{
		return EKeyWasNotConsumed;
	}

	if ( aType == EEventKeyDown )
	{
		switch( aKeyEvent.iScanCode )
		{
			case EStdKeyLeftArrow:
				if ( iRenderer->GetScreenType() == CRenderer::EScreenOptions )
				{
					iLoop->QueueSoundPlayback( CSoundManager::EMenuBeepMove );

					TSettingsItem &SettingsItem = iSettingsManager->iSettingsItems[ iRenderer->iHighlightedMenuItem ];

					if ( SettingsItem.iItemType == TSettingsItem::ESettingsItemTypeRadio )
					{
						SettingsItem.iRadioIndex = SettingsItem.iRadioIndex <= 0 ? SettingsItem.iRadioValueCount - 1 : SettingsItem.iRadioIndex - 1;
					}
					else if ( SettingsItem.iItemType == TSettingsItem::ESettingsItemTypeToggle )
					{
						SettingsItem.iToggle = !SettingsItem.iToggle;
					}
				}

				// Intentional fall-through

			case EStdKeyNkp4:
			case '4':
				MyKeyHandler.SetState( TKeyHandler::ELeft, ETrue );
				break;

			case EStdKeyRightArrow:
				if ( iRenderer->GetScreenType() == CRenderer::EScreenOptions )
				{
					iLoop->QueueSoundPlayback( CSoundManager::EMenuBeepMove );

					TSettingsItem &SettingsItem = iSettingsManager->iSettingsItems[ iRenderer->iHighlightedMenuItem ];

					if ( SettingsItem.iItemType == TSettingsItem::ESettingsItemTypeRadio )
					{
						SettingsItem.iRadioIndex = SettingsItem.iRadioIndex >= SettingsItem.iRadioValueCount - 1 ? 0 : SettingsItem.iRadioIndex + 1;
					}
					else if ( SettingsItem.iItemType == TSettingsItem::ESettingsItemTypeToggle )
					{
						SettingsItem.iToggle = !SettingsItem.iToggle;
					}
				}

				// Intentional fall-through

			case EStdKeyNkp6:
			case '6':
				MyKeyHandler.SetState( TKeyHandler::ERight, ETrue );
				break;

			case EStdKeyDevice0: // Letf softkey
				if ( iRenderer->GetScreenType() == CRenderer::EScreenMainMenu )
				{
					iLoop->QueueSoundPlayback( CSoundManager::EMenuBeepAccept );
					iRenderer->SetScreenType( CRenderer::EScreenOptions );
					iRenderer->iHighlightedMenuItem = 0;
				}

				else if ( iRenderer->GetScreenType() == CRenderer::EScreenHighScores )
				{
					iLoop->QueueSoundPlayback( CSoundManager::EMenuBeepAccept );

					iScoreManager->InitHighScoresL();
					iScoreManager->LoadHighScoresL();
				}

				else if ( iRenderer->GetScreenType() == CRenderer::EScreenOptions )
				{
					iLoop->QueueSoundPlayback( CSoundManager::EMenuBeepAccept );

					iSettingsManager->InitSettingsL();
					iSettingsManager->LoadSettingsL();
				}

				// Intentional fall-through

			case EStdKeyDevice3: // Center D-Pad on taco or 'Tick' key on QD
			case EStdKeyNkp5:
			case '5':
				switch ( iRenderer->GetScreenType() )
				{
					case CRenderer::EScreenMainMenu:
						{
							iLoop->QueueSoundPlayback( CSoundManager::EMenuBeepAccept );

							switch( iRenderer->iHighlightedMenuItem )
							{
								case 0: // "Start Game" TODO make an enum for these
									iRenderer->SetScreenType( CRenderer::EScreenGame );
									break;
								case 1: // "High Scores"
									iRenderer->SetScreenType( CRenderer::EScreenHighScores );
									break;
								default:
									break;
							}

							iRenderer->iHighlightedMenuItem = 0;
						}
						break;

					case CRenderer::EScreenPaused:
						{
							iLoop->QueueSoundPlayback( CSoundManager::EMenuBeepAccept );

							// TODO make an enum for these
							if ( iRenderer->iHighlightedMenuItem == 0 ) // "Resume Game"
							{
								iRenderer->SetScreenType( CRenderer::EScreenGame );
							}
							
							if ( iRenderer->iHighlightedMenuItem == 1 ) // "Back To Menu"
							{
								iRenderer->SetScreenType( CRenderer::EScreenMainMenu );
								iScoreManager->SetHighScoreL( static_cast< TUint16 >( iEngine->GetPlayerScore() ) );
								iScoreManager->SaveHighScoresL();

								// Initialise the game engine
								iEngine->Reset( Rect() );
							}
						}

						break;

					default:
						break;
				}

				break;

			case EStdKeyUpArrow:
				if ( iRenderer->GetScreenType() == CRenderer::EScreenMainMenu )
				{
					iLoop->QueueSoundPlayback( CSoundManager::EMenuBeepMove );

					iRenderer->iHighlightedMenuItem = iRenderer->iHighlightedMenuItem <= 0 ? 1 : iRenderer->iHighlightedMenuItem - 1;
				}

				else if ( iRenderer->GetScreenType() == CRenderer::EScreenPaused )
				{
					iLoop->QueueSoundPlayback( CSoundManager::EMenuBeepMove );

					iRenderer->iHighlightedMenuItem = iRenderer->iHighlightedMenuItem <= 0 ? 1 : iRenderer->iHighlightedMenuItem - 1;
				}

				else if ( iRenderer->GetScreenType() == CRenderer::EScreenOptions )
				{
					iLoop->QueueSoundPlayback( CSoundManager::EMenuBeepMove );

					iRenderer->iHighlightedMenuItem = iRenderer->iHighlightedMenuItem <= 0 ? iSettingsManager->iSettingsItems.Count() - 1 : iRenderer->iHighlightedMenuItem - 1;
				}

				break;

			case EStdKeyDownArrow:
				if ( iRenderer->GetScreenType() == CRenderer::EScreenMainMenu )
				{
					iLoop->QueueSoundPlayback( CSoundManager::EMenuBeepMove );

					iRenderer->iHighlightedMenuItem = iRenderer->iHighlightedMenuItem >= 1 ? 0 : iRenderer->iHighlightedMenuItem + 1;
				}

				else if ( iRenderer->GetScreenType() == CRenderer::EScreenPaused )
				{
					iLoop->QueueSoundPlayback( CSoundManager::EMenuBeepMove );

					iRenderer->iHighlightedMenuItem = iRenderer->iHighlightedMenuItem >= 1 ? 0 : iRenderer->iHighlightedMenuItem + 1;
				}

				else if ( iRenderer->GetScreenType() == CRenderer::EScreenOptions )
				{
					iLoop->QueueSoundPlayback( CSoundManager::EMenuBeepMove );

					iRenderer->iHighlightedMenuItem = iRenderer->iHighlightedMenuItem >= iSettingsManager->iSettingsItems.Count() - 1 ? 0 : iRenderer->iHighlightedMenuItem + 1;
				}

				break;

			case EStdKeyDevice1: // Right softkey
				switch( iRenderer->GetScreenType() )
				{
					case CRenderer::EScreenMainMenu:
						iLoop->QueueSoundPlayback( CSoundManager::EMenuBeepAccept );

						if ( iSoundManager )
						{
							iSoundManager->Stop();
						}

						User::Exit( 0 );
						break;

					case CRenderer::EScreenGame:
						iLoop->QueueSoundPlayback( CSoundManager::EMenuBeepAccept );

						iRenderer->iHighlightedMenuItem = 0;
						iRenderer->SetScreenType( CRenderer::EScreenPaused );
						break;

					case CRenderer::EScreenHighScores:
						iLoop->QueueSoundPlayback( CSoundManager::EMenuBeepAccept );

						iRenderer->iHighlightedMenuItem = 0;
						iRenderer->SetScreenType( CRenderer::EScreenMainMenu );
						break;

					case CRenderer::EScreenOptions:
						iLoop->QueueSoundPlayback( CSoundManager::EMenuBeepAccept );

						iRenderer->iHighlightedMenuItem = 0;
						iRenderer->SetScreenType( CRenderer::EScreenMainMenu );
						iSettingsManager->SaveSettingsL();

						iLoop->iInterval = FrameTime;

						break;

					default:
						break;
				}

				break;

			default:
				break;
		}

		return EKeyWasConsumed;
	}

	else if ( aType == EEventKeyUp )
	{
		switch( aKeyEvent.iScanCode )
		{
			case EStdKeyLeftArrow:
			case EStdKeyNkp4:
			case '4':
				MyKeyHandler.SetState( TKeyHandler::ELeft, EFalse );
				break;
			case EStdKeyRightArrow:
			case EStdKeyNkp6:
			case '6':
				MyKeyHandler.SetState( TKeyHandler::ERight, EFalse );
				break;
			default:
				break;
		}

		return EKeyWasConsumed;
	}

	return EKeyWasNotConsumed;
}

// End of File