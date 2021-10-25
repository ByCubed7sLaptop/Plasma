#include "Engine/Vector2.h"
#include "Player.h"
//#include "Game.h"


void Player::Update(double delta, Game& game)
{
	// - Input

	// Velocity
	if (game.input.IsKey(Input::Key_D)) velocity.x = 20.0f * game.timeStep;
	else if (game.input.IsKey(Input::Key_A)) velocity.x = -20.0f * game.timeStep;
	else velocity.x = 0;

	if (game.input.IsKey(Input::Key_S)) velocity.y = 20.0f * game.timeStep;
	else if (game.input.IsKey(Input::Key_W)) velocity.y = -20.0f * game.timeStep;
	else velocity.y = 0;

	// Rotation
	if (game.input.IsKey(Input::Key_W)) rotation = 270;
	if (game.input.IsKey(Input::Key_A)) rotation = 180;
	if (game.input.IsKey(Input::Key_S)) rotation = 90;
	if (game.input.IsKey(Input::Key_D)) rotation = 0;
	
	// Normilize velo
	if (velocity.Magnitude()) 
	{
		velocity.Normalize();
		velocity = velocity * 300;
	}
}