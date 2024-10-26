/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// Zombie
//=========================================================

// UNDONE: Don't flinch every time you get hit

#include	"extdll.h"
#include	"util.h"
#include	"cmbase.h"
#include	"cmbasemonster.h"
#include	"monsters.h"
#include	"schedule.h"


//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define HOUND_ATTACK		( 881 )

#define GHOUL_FLINCH_DELAY			2		// at most one flinch every n secs

const char *CMFelHound::pAttackMissSounds[] = 
{
	"anotherway/mobs/miss.wav"
};

const char *CMFelHound::pAttackSounds[] = 
{
	"anotherway/mobs/hound/attack1.wav",
	"anotherway/mobs/hound/attack2.wav"
};

const char *CMFelHound::pIdleSounds[] = 
{
	"anotherway/mobs/hound/alert.wav"
};

const char *CMFelHound::pAlertSounds[] = 
{
	"anotherway/mobs/hound/aggro.wav"
};

const char *CMFelHound::pPainSounds[] = 
{
	"anotherway/mobs/hound/wound1.wav",
	"anotherway/mobs/hound/wound2.wav"
};

const char *CMFelHound::pDeathSounds[] = 
{
	"anotherway/mobs/hound/die.wav"
};
//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CMFelHound :: Classify ( void )
{
	if ( m_iClassifyOverride == -1 ) // helper
		return CLASS_NONE;
	else if ( m_iClassifyOverride > 0 )
		return m_iClassifyOverride; // override
	
	return	CLASS_ALIEN_MONSTER;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CMFelHound :: SetYawSpeed ( void )
{
	int ys;

	ys = 120;

#if 0
	switch ( m_Activity )
	{
	}
#endif

	pev->yaw_speed = ys;
}

int CMFelHound :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	// HACK HACK -- until we fix this.
	if ( IsAlive() )
		PainSound();
	return CMBaseMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

void CMFelHound :: PainSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);

	EMIT_SOUND_DYN ( ENT(pev), CHAN_VOICE, pPainSounds[ RANDOM_LONG(0,ARRAYSIZE(pPainSounds)-1) ], 1.0, ATTN_NORM, 0, pitch );
}

void CMFelHound :: AlertSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);

	EMIT_SOUND_DYN ( ENT(pev), CHAN_VOICE, pAlertSounds[ RANDOM_LONG(0,ARRAYSIZE(pAlertSounds)-1) ], 1.0, ATTN_NORM, 0, pitch );
}

void CMFelHound :: IdleSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);

	// Play a random idle sound
	EMIT_SOUND_DYN ( ENT(pev), CHAN_VOICE, pIdleSounds[ RANDOM_LONG(0,ARRAYSIZE(pIdleSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
}

void CMFelHound :: AttackSound( void )
{
	// Play a random attack sound
	EMIT_SOUND_DYN ( ENT(pev), CHAN_VOICE, pAttackSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
}

void CMFelHound :: DeathSound ( void )
{
	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, pDeathSounds[ RANDOM_LONG(0,ARRAYSIZE(pDeathSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
}
//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CMFelHound :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
		case HOUND_ATTACK:
		{
			// do stuff for this event.
			edict_t *pHurt = CheckTraceHullAttack( 70, RANDOM_FLOAT(70, 80), DMG_SLASH );
			if ( pHurt )
			{
				if ( pHurt->v.flags & (FL_MONSTER|FL_CLIENT) )
				{
					pHurt->v.punchangle.z = -18;
					pHurt->v.punchangle.x = 5;
					pHurt->v.velocity = pHurt->v.velocity - gpGlobals->v_right * 100;
				}
			}
			else // Play a random attack miss sound
				EMIT_SOUND_DYN ( ENT(pev), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );

			if (RANDOM_LONG(0,1))
				AttackSound();
		}
		break;

		default:
			CMBaseMonster::HandleAnimEvent( pEvent );
			break;
	}
}
void CMFelHound::SetActivity(Activity NewActivity)
{
	CMBaseMonster::SetActivity(NewActivity);

	switch (m_Activity)
	{
		case ACT_RUN:
			m_flGroundSpeed = 120;
			break;
		case ACT_WALK:
			m_flGroundSpeed = 100;
			break;
	}
}
//=========================================================
// Spawn
//=========================================================
void CMFelHound :: Spawn()
{
	Precache( );
	
	SET_MODEL(ENT(pev), (!FStringNull( pev->model ) ? STRING( pev->model ) : "models/anotherway/mobs/felhoundc.mdl"));
	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	//m_flGroundSpeed		= 200;
	switch(RANDOM_LONG(1,4)) {
		case 1: 
			pev->skin = 0;
			break;
		case 2: 
			pev->skin = 1;
			break;
		case 3: 
			pev->skin = 2;
			break;
		case 4: 
			pev->skin = 3;
			break;
	}
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= !m_bloodColor ? BLOOD_COLOR_YELLOW : m_bloodColor;
	pev->health = Tier2_HP;
	pev->view_ofs		= VEC_VIEW;// position of the eyes relative to monster's origin.
	m_flFieldOfView		= 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_DOORS_GROUP;

	MonsterInit();
	
	pev->classname = MAKE_STRING( "monster_hound" );
	if ( strlen( STRING( m_szMonsterName ) ) == 0 )
	{
		// default name
		m_szMonsterName = MAKE_STRING( "Hound" );
	}
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CMFelHound :: Precache()
{
	PRECACHE_MODEL("models/anotherway/mobs/felhoundc.mdl");

	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);
}	

//=========================================================
// AI Schedules Specific to this monster
//=========================================================



int CMFelHound::IgnoreConditions ( void )
{
	int iIgnore = CMBaseMonster::IgnoreConditions();

	if ((m_Activity == ACT_MELEE_ATTACK1) || (m_Activity == ACT_MELEE_ATTACK1))
	{
#if 0
		if (pev->health < 20)
			iIgnore |= (bits_COND_LIGHT_DAMAGE|bits_COND_HEAVY_DAMAGE);
		else
#endif			
		if (m_flNextFlinch >= gpGlobals->time)
			iIgnore |= (bits_COND_LIGHT_DAMAGE|bits_COND_HEAVY_DAMAGE);
	}

	if ((m_Activity == ACT_SMALL_FLINCH) || (m_Activity == ACT_BIG_FLINCH))
	{
		if (m_flNextFlinch < gpGlobals->time)
			m_flNextFlinch = gpGlobals->time + GHOUL_FLINCH_DELAY;
	}

	return iIgnore;
	
}
