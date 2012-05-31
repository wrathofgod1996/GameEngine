#pragma once
#include "istatecreator.h"
#include "GameStateFactory.h"

class GameOfLifeCreator : public IStateCreator
{
public:

	GameOfLifeCreator();
	virtual IGameState* Create() const;

} ;

