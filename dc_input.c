/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// dc_input.c - Dreamcast/KOS input driver
// By BlackAura - 2003

#include "quakedef.h"
#include <kos.h>

// =============================================
// Keyboard
// =============================================
const static int q_keytrans[]= {
	/*0*/	0, 0, 0, 0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
		'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
		'u', 'v', 'w', 'x', 'y', 'z',
	/*1e*/	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	/*28*/	K_ENTER, K_ESCAPE, K_BACKSPACE, K_TAB, K_SPACE, '-', '=', '[', ']', '\\', 0, ';', '\'',
	/*35*/	'`', ',', '.', '/', 0, K_F1, K_F2, K_F3, K_F4, K_F5, K_F6, K_F7, K_F8, K_F9, K_F10, K_F11, K_F12,
	/*46*/	0, 0, K_PAUSE, K_INS, K_HOME, K_PGUP, K_DEL, K_END, K_PGDN, K_RIGHTARROW, K_LEFTARROW, K_DOWNARROW, K_UPARROW,
	/*53*/	0, '/', '*', '-', '+', 13, '1', '2', '3', '4', '5', '6',
	/*5f*/	'7', '8', '9', '0', '.', 0
};
#define NUMKEYS	sizeof(q_keytrans)
static int		key_shift;
static uint8	key_matrix[NUMKEYS];

static void Key_Init()
{
	int i;
	for(i = 0; i < NUMKEYS; i++)
		key_matrix[i] = 0;
	key_shift = 0;
}

static void Key_Update()
{
	int			i;
	kbd_state_t	*state;				// Current keyboard state
	maple_device_t	*dev;			// Keyboard maple device
	int			new_key_shift = 0;	// Keyboard shift states

	// Find a keyboard
	dev = maple_enum_type(0, MAPLE_FUNC_KEYBOARD);
	if(!dev)
		return;

	// Get keyboard state
	state = (kbd_state_t *)maple_dev_status(dev);
	if(!state)
		return;

	// Scan keys
	for(i = 0; i < NUMKEYS; i++)
	{
		if(key_matrix[i] != state->matrix[i])
		{
			// Convert scancode to Quake key number
			int keynum = q_keytrans[i];
			if(keynum)
				Key_Event(keynum, state->matrix[i]);
			key_matrix[i] = state->matrix[i];
		}
	}

	// Deal with shift keys
	new_key_shift = 0x00;
	if((state->shift_keys & KBD_MOD_LSHIFT)||(state->shift_keys & KBD_MOD_RSHIFT))
		new_key_shift |= 0x01;
	if((state->shift_keys & KBD_MOD_LCTRL)||(state->shift_keys & KBD_MOD_RCTRL))
		new_key_shift |= 0x02;
	if((state->shift_keys & KBD_MOD_LALT)||(state->shift_keys & KBD_MOD_RALT))
		new_key_shift |= 0x04;

	if( (new_key_shift & 0x01) && !(key_shift & 0x01) )
		Key_Event(K_SHIFT, 1);
	if( !(new_key_shift & 0x01) && (key_shift & 0x01) )
		Key_Event(K_SHIFT, 0);

	if( (new_key_shift & 0x02) && !(key_shift & 0x02) )
		Key_Event(K_CTRL, 1);
	if( !(new_key_shift & 0x02) && (key_shift & 0x02) )
		Key_Event(K_CTRL, 0);

	if( (new_key_shift & 0x04) && !(key_shift & 0x04) )
		Key_Event(K_ALT, 1);
	if( !(new_key_shift & 0x04) && (key_shift & 0x04) )
		Key_Event(K_ALT, 0);

	key_shift = new_key_shift;
}

// =============================================
// Mouse
// =============================================
static void Mouse_UpdateButtons()
{
	maple_device_t	*dev;
	mouse_state_t	*state;
	int				i;
	int				q_mousekeys[] = {K_MOUSE1, K_MOUSE2, K_MOUSE3};
	static int		mouse_oldbuttons = 0;

	// Find a mouse
	dev = maple_enum_type(0, MAPLE_FUNC_MOUSE);
	if(!dev)
		return;

	// Get mouse state
	state = (mouse_state_t *)maple_dev_status(dev);
	if(!state)
		return;

	// Parse squeaker buttons
	for(i = 0; i < 3; i++)
	{
		int mask = 1 << i;
		if( (state->buttons & mask) && (!(mouse_oldbuttons & mask)) )
			Key_Event(q_mousekeys[i], 1);
		else if( (!(state->buttons & mask)) && (mouse_oldbuttons & mask) )
			Key_Event(q_mousekeys[i], 0);
	}
	mouse_oldbuttons = state->buttons;
}

static void Mouse_UpdateAxes(usercmd_t *cmd)
{
	maple_device_t	*dev;
	mouse_state_t	*state;
	float	mouse_x, mouse_y;

	// Find a mouse
	dev = maple_enum_type(0, MAPLE_FUNC_MOUSE);
	if(!dev)
		return;

	// Get mouse state
	state = (mouse_state_t *)maple_dev_status(dev);
	if(!state)
		return;

	// Scale axes for sensitivity
	mouse_x = (float)state->dx * sensitivity.value;
	mouse_y = (float)state->dy * sensitivity.value;

	// Handle X-axis
	if ( (in_strafe.state & 1) || (lookstrafe.value && (in_mlook.state & 1) ))
		// Strafing
		cmd->sidemove += m_side.value * mouse_x;
	else
		// Turning
		cl.viewangles[YAW] -= m_yaw.value * mouse_x;
	
	// Stop pitch drift when MouseLook is enabled
	if (in_mlook.state & 1)
		V_StopPitchDrift ();

	// Handle Y-axis
	if ( (in_mlook.state & 1) && !(in_strafe.state & 1))
	{
		// Mouselook
		cl.viewangles[PITCH] += m_pitch.value * mouse_y;
		if (cl.viewangles[PITCH] > 80)
			cl.viewangles[PITCH] = 80;
		if (cl.viewangles[PITCH] < -70)
			cl.viewangles[PITCH] = -70;
	}
	else
	{
		// Walking
		if ((in_strafe.state & 1) && noclip_anglehack)
			cmd->upmove -= m_forward.value * mouse_y;
		else
			cmd->forwardmove -= m_forward.value * mouse_y;
	}	
}

// =============================================
// Joystick
// =============================================
static void Joy_UpdateButtons()
{
	maple_device_t	*dev;
	cont_state_t	*state;
	int				i;
	static int		joy_oldbuttons = 0;
	static int		old_left = 0, old_right = 0;
	int				q_joykeys[] =
	{
		K_DC_C, K_DC_B, K_DC_A, K_DC_START,
		K_DPAD1_UP, K_DPAD1_DOWN, K_DPAD1_LEFT,
		K_DPAD1_RIGHT, K_DC_Z, K_DC_Y, K_DC_X,
		K_DC_D, K_DPAD2_UP, K_DPAD2_DOWN,
		K_DPAD2_LEFT, K_DPAD2_RIGHT
	};

	// Find a controller
	dev = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
	if(!dev)
		return;

	// Get controller state
	state = (cont_state_t *)maple_dev_status(dev);
	if(!state)
		return;

	// Parse buttons
	for(i = 0; i < 16; i++)
	{
		int mask = 1 << i;
		if( (state->buttons & mask) && (!(joy_oldbuttons & mask)) )
			Key_Event(q_joykeys[i], 1);
		else if( (!(state->buttons & mask)) && (joy_oldbuttons & mask) )
			Key_Event(q_joykeys[i], 0);
	}
	joy_oldbuttons = state->buttons;

	// Parse left/right triggers
	if((state->ltrig > 0) && (old_left == 0))
		Key_Event(K_DC_LTRIG, 1);
	else if((state->ltrig == 0) && (old_left > 0))
		Key_Event(K_DC_LTRIG, 0);
	if((state->rtrig > 0) && (old_right == 0))
		Key_Event(K_DC_RTRIG, 1);
	else if((state->rtrig == 0) && (old_right > 0))
		Key_Event(K_DC_RTRIG, 0);
	old_left = state->ltrig;
	old_right = state->rtrig;
}

cvar_t	axis_x_function =	{"dc_axisx_function", "1", true};
cvar_t	axis_y_function =	{"dc_axisy_function", "4", true};
cvar_t	axis_l_function =	{"dc_axisl_function", "0", true};
cvar_t	axis_r_function =	{"dc_axisr_function", "0", true};
cvar_t	axis_x2_function =	{"dc_axisx2_function", "2", true};
cvar_t	axis_y2_function =	{"dc_axisy2_function", "3", true};

cvar_t	axis_x_scale =		{"dc_axisx_scale", "1", true};
cvar_t	axis_y_scale =		{"dc_axisy_scale", "1", true};
cvar_t	axis_l_scale =		{"dc_axisl_scale", "1", true};
cvar_t	axis_r_scale =		{"dc_axisr_scale", "1", true};
cvar_t	axis_x2_scale =		{"dc_axisx2_scale", "1", true};
cvar_t	axis_y2_scale =		{"dc_axisy2_scale", "1", true};

cvar_t	axis_pitch_dz =		{"dc_pitch_threshold", "0.15", true};
cvar_t	axis_yaw_dz =		{"dc_yaw_threshold", "0.15", true};
cvar_t	axis_walk_dz =		{"dc_walk_threshold", "0.15", true};
cvar_t	axis_strafe_dz =	{"dc_strafe_threshold", "0.15", true};

#define AXIS_NONE	'0'
#define	AXIS_TURN	'1'
#define	AXIS_WALK	'2'
#define	AXIS_STRAFE	'3'
#define	AXIS_LOOK	'4'

static void Joy_UpdateAxis(usercmd_t *cmd, char mode, float scale, int rawvalue)
{
	float value;
	float svalue;
	float speed, aspeed;

	// Don't bother if it's switched off
	if(mode == AXIS_NONE)
		return;

	// Convert value from -128...128 to -1...1, multiply by scale
	value = (rawvalue / 128.0);
	svalue = value * scale;

	// Handle +speed
	if (in_speed.state & 1)
		speed = cl_movespeedkey.value;
	else
		speed = 1;
	aspeed = speed * host_frametime;

	switch(mode)
	{
		// Turning
		case AXIS_TURN:
			if(fabs(value) > axis_yaw_dz.value)
				cl.viewangles[YAW] -= svalue  * aspeed * cl_yawspeed.value;
			break;
		
		// Walking
		case AXIS_WALK:
			if(fabs(value) > axis_walk_dz.value)
				cmd->forwardmove -= svalue * speed * cl_forwardspeed.value;
			break;

		// Strafing
		case AXIS_STRAFE:
			if(fabs(value) > axis_strafe_dz.value)
				cmd->sidemove -= svalue * speed * cl_sidespeed.value;
			break;

		// Looking
		case AXIS_LOOK:
			if(fabs(value) > axis_pitch_dz.value)
			{
				cl.viewangles[PITCH] += svalue * aspeed * cl_pitchspeed.value;
				V_StopPitchDrift();
			}
			else if(lookspring.value == 0.0)
				V_StopPitchDrift();
			break;
	}

	// bounds check pitch
	if (cl.viewangles[PITCH] > 80.0)
		cl.viewangles[PITCH] = 80.0;
	if (cl.viewangles[PITCH] < -70.0)
		cl.viewangles[PITCH] = -70.0;
}

static void Joy_UpdateAxes(usercmd_t *cmd)
{
	maple_device_t	*dev;
	cont_state_t	*state;

	// Find a controller
	dev = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
	if(!dev)
		return;

	// Get controller state
	state = (cont_state_t *)maple_dev_status(dev);
	if(!state)
		return;

	// Update all axes
	Joy_UpdateAxis(cmd, axis_x_function.string[0], axis_x_scale.value, state->joyx);
	Joy_UpdateAxis(cmd, axis_y_function.string[0], axis_y_scale.value, state->joyy);
	Joy_UpdateAxis(cmd, axis_l_function.string[0], axis_l_scale.value, (state->ltrig >> 1));
	Joy_UpdateAxis(cmd, axis_r_function.string[0], axis_r_scale.value, (state->rtrig >> 1));
	Joy_UpdateAxis(cmd, axis_x2_function.string[0], axis_x2_scale.value, state->joy2x);
	Joy_UpdateAxis(cmd, axis_y2_function.string[0], axis_y2_scale.value, state->joy2y);
}

// =============================================
// Quake control interface
// =============================================
void IN_Init (void)
{
	// Register cvars
	Cvar_RegisterVariable(&axis_x_function);
	Cvar_RegisterVariable(&axis_y_function);
	Cvar_RegisterVariable(&axis_l_function);
	Cvar_RegisterVariable(&axis_r_function);
	Cvar_RegisterVariable(&axis_x2_function);
	Cvar_RegisterVariable(&axis_y2_function);
	Cvar_RegisterVariable(&axis_x_scale);
	Cvar_RegisterVariable(&axis_y_scale);
	Cvar_RegisterVariable(&axis_l_scale);
	Cvar_RegisterVariable(&axis_r_scale);
	Cvar_RegisterVariable(&axis_x2_scale);
	Cvar_RegisterVariable(&axis_y2_scale);
	Cvar_RegisterVariable(&axis_pitch_dz);
	Cvar_RegisterVariable(&axis_yaw_dz);
	Cvar_RegisterVariable(&axis_walk_dz);
	Cvar_RegisterVariable(&axis_strafe_dz);

	Key_Init();
}

void IN_Shutdown (void)
{
}

void IN_Commands (void)
{
	Joy_UpdateButtons();
	Mouse_UpdateButtons();
}

void IN_Move (usercmd_t *cmd)
{
	Joy_UpdateAxes(cmd);
	Mouse_UpdateAxes(cmd);
}

void Sys_SendKeyEvents (void)
{
	Key_Update();
}
