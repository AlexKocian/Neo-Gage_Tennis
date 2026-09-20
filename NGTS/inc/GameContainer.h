#ifndef GAMECONTAINER_H
#define GAMECONTAINER_H

// INCLUDES

// System includes
#include <coecntrl.h> // CCoeControl

// User includes
#include "Game.h"
#include "ScoreManager.h"
#include "SettingsManager.h"
#include "SoundManager.h"

// FORWARD DECLARATIONS
class CEikLabel;

// CLASS DECLARATION

/*
*
* CGameContainer class
* This is the container class
*
*/
class CGameContainer : public CCoeControl, public MSoundManagerObserver
{
public: // Constructors and destructor

	static CGameContainer *NewL( const TRect &aRect );
	static CGameContainer *NewLC( const TRect &aRect );
	~CGameContainer();

	void StartDSA();
	void StopDSA();

	TTimeIntervalMicroSeconds32 GetFrameTime() const;

	// From CCoeControl
	virtual TKeyResponse OfferKeyEventL( const TKeyEvent &aKeyEvent, TEventCode aType );

private:
	CGameContainer( const TRect &aRect );
	void ConstructL( const TRect &aRect );

public:	
	// From MSoundManagerObserver
	virtual void HandlePlayingStoppedL();

private:
	// From CCoeControl
	void Draw( TRect &aRect ) const;

private:
	CPeriodic *iPeriodicTimer;
	CGameLoop *iLoop;

	CEngine *iEngine;
	CDSAWrapper *iDSAWrapper;
	CRenderer *iRenderer;
	CScoreManager *iScoreManager;
	CSettingsManager *iSettingsManager;
	CSoundManager *iSoundManager;
};

#endif

// End of File