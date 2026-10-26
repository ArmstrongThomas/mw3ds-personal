#include <cassert>
#include <cstdio>
#include "mwinput/touchlook.hpp"
#include "mwinput/stylusinput.hpp"

int main()
{
	TouchLook t;
	assert(t.update(true, true, 100, 100));
	assert(t.dx == 0 && t.dy == 0);
	assert(t.update(true, true, 120, 90));
	assert(t.dx == 20 && t.dy == -10);
	// A swipe remains camera input over the menu row, through release.
	assert(t.update(true, true, 140, 222));
	assert(t.update(false, true, 0, 0));
	assert(t.dx == 0 && t.dy == 0);
	assert(t.update(true, true, 280, 120));
	assert(t.dx == 0 && t.dy == 0);
	// Opening a menu cancels the swipe; closing it cannot resume the same contact.
	assert(t.update(true, false, 280, 130));
	assert(t.update(true, true, 290, 140));
	assert(t.dx == 0 && t.dy == 0);
	assert(t.update(false, false, 0, 0));
	// A button-origin gesture cannot turn into a camera gesture or activate on release over the pad.
	assert(!t.update(true, true, 20, 222));
	assert(t.update(true, true, 20, 100));
	assert(t.dx == 0 && t.dy == 0);
	assert(t.update(false, true, 0, 0));
	// Ordinary taps outside the pad, and menu drags, remain UI input.
	assert(!t.update(true, true, 20, 222));
	assert(!t.update(false, true, 0, 0));
	assert(!t.update(true, false, 20, 100));
	assert(!t.update(true, false, 30, 120));
	assert(!t.update(false, false, 0, 0));
	assert(TouchLook::contains(6, 48));
	assert(!TouchLook::contains(314, 100));
	assert(!TouchLook::contains(20, 174));
	for (int i = 0; i < 5; i++)
	{
		int x = TouchLook::actionLeft + i * TouchLook::actionStep;
		assert(TouchLook::actionAt(x, 190) == i);
		assert(TouchLook::actionAt(x + 33, 190) == -1);
	}
	assert(TouchLook::actionAt(190, 190) == -1);
	assert(TouchLook::actionAt(10, 205) == -1);
	PlayerInput right = {};
	right.moveX = 0.4f;
	right.moveY = 0.8f;
	stylusInput(right, false, KEY_L | KEY_DUP, KEY_DUP);
	assert(right.attack && right.activate && !right.cast);
	assert(right.moveX == 0.4f && right.moveY == 0.8f);
	PlayerInput left = {};
	stylusInput(left, true, KEY_X | KEY_A | KEY_R, KEY_X | KEY_A | KEY_R);
	assert(left.attack && !left.activate && !left.readyToggle && !left.jump);
	assert(left.moveX > 0.70f && left.moveX < 0.71f);
	assert(left.moveY == left.moveX);
	stylusInput(left, true, KEY_X | KEY_B | KEY_A | KEY_Y, 0);
	assert(left.moveX == 0 && left.moveY == 0 && !left.attack);
	stylusInput(right, false, KEY_DDOWN, KEY_DDOWN);
	assert(right.jump && right.up && !right.attack);
	stylusInput(right, false, KEY_DDOWN, 0);
	assert(!right.jump && right.up);
	stylusInput(right, false, KEY_R, KEY_DRIGHT | KEY_DLEFT | KEY_SELECT);
	assert(right.sneak && right.down && right.cast && right.readyToggle && right.togglePov);
	puts("touchlook: gesture ownership, menu transitions, lift/reposition and action hitboxes passed");
}
