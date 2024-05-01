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
#define	ZOMBIE_AE_ATTACK_BOTH		1

#define ZOMBIE_FLINCH_DELAY			2		// at most one flinch every n secs


const char *CMSnake::pAttackHitSounds[] = 
{
	"anotherway/mobs/snake/snake_attack.wav",
};

const char *CMSnake::pAttackMissSounds[] = 
{
	"anotherway/mobs/snake/snake_agro1.wav",
	"anotherway/mobs/snake/snake_agro2.wav",
	"anotherway/mobs/snake/snake_agro3.wav",
	"anotherway/mobs/snake/snake_agro4.wav",
};

const char *CMSnake::pAttackSounds[] = 
{
	"anotherway/mobs/snake/snake_agro1.wav",
	"anotherway/mobs/snake/snake_agro2.wav",
	"anotherway/mobs/snake/snake_agro3.wav",
	"anotherway/mobs/snake/snake_agro4.wav",
};

const char *CMSnake::pIdleSounds[] = 
{
	"anotherway/mobs/snake/snake_agro1.wav",
	"anotherway/mobs/snake/snake_agro2.wav",
	"anotherway/mobs/snake/snake_agro3.wav",
	"anotherway/mobs/snake/snake_agro4.wav",
};

const char *CMSnake::pAlertSounds[] = 
{
	"anotherway/mobs/snake/snake_agro1.wav",
	"anotherway/mobs/snake/snake_agro2.wav",
	"anotherway/mobs/snake/snake_agro3.wav",
	"anotherway/mobs/snake/snake_agro4.wav",
};

const char *CMSnake::pPainSounds[] = 
{
	"anotherway/mobs/snake/snake_agro1.wav",
	"anotherway/mobs/snake/snake_agro2.wav",
	"anotherway/mobs/snake/snake_agro3.wav",
	"anotherway/mobs/snake/snake_agro4.wav",
};

const char *CMSnake::pDeathSounds[] = 
{
	"anotherway/mobs/snake/snake_death.wav",
};
//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CMSnake :: Classify ( void )
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
void CMSnake :: SetYawSpeed ( void )
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
void CMSnake :: DeathSound ( void )
{
	EMIT_SOUND_DYN( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDeathSounds), GetSoundVolume(), ATTN_IDLE, 0, GetVoicePitch() );
}

int CMSnake :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	// HACK HACK -- until we fix this.
	if ( IsAlive() )
		PainSound();
	return CMBaseMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

void CMSnake :: PainSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);

	if (RANDOM_LONG(0,5) < 2)
		EMIT_SOUND_DYN ( ENT(pev), CHAN_VOICE, pPainSounds[ RANDOM_LONG(0,ARRAYSIZE(pPainSounds)-1) ], 1.0, ATTN_NORM, 0, pitch );
}

void CMSnake :: AlertSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);

	EMIT_SOUND_DYN ( ENT(pev), CHAN_VOICE, pAlertSounds[ RANDOM_LONG(0,ARRAYSIZE(pAlertSounds)-1) ], 1.0, ATTN_NORM, 0, pitch );
}

void CMSnake :: IdleSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);

	// Play a random idle sound
	EMIT_SOUND_DYN ( ENT(pev), CHAN_VOICE, pIdleSounds[ RANDOM_LONG(0,ARRAYSIZE(pIdleSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
}

void CMSnake :: AttackSound( void )
{
	// Play a random attack sound
	EMIT_SOUND_DYN ( ENT(pev), CHAN_VOICE, pAttackSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
}

void CMSnake::SetActivity(Activity NewActivity)
{
	CMBaseMonster::SetActivity(NewActivity);

	switch (m_Activity)
	{
	case ACT_WALK:
		m_flGroundSpeed = 100;
		break;
	default:
		m_flGroundSpeed = 100;
		break;
	}
}
//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CMSnake :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
		case ZOMBIE_AE_ATTACK_BOTH:
		{
			// do stuff for this event.
			if ( m_hEnemy != NULL /*  && IsMoving() && pevAttacker == VARS((edict_t *)m_hEnemy)  && gpGlobals->time - m_flLastHurtTime > 3 */)
			{
				entvars_t *attacker = VARS((edict_t *)m_hEnemy);
				if ( attacker->flags & (FL_MONSTER|FL_CLIENT) )
				{
					UTIL_TakeDamage((edict_t *)m_hEnemy, pev, pev, gSkillData.zombieDmgBothSlash, DMG_SLASH );
					attacker->punchangle.x = 5;
					//attacker->velocity = attacker->velocity + gpGlobals->v_forward * 1;
					EMIT_SOUND_DYN ( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
				}
			}

			if (RANDOM_LONG(0,1))
				AttackSound();
		}
		break;

		default:
			CMBaseMonster::HandleAnimEvent( pEvent );
			break;
	}
}

//=========================================================
// Spawn
//=========================================================
void CMSnake :: Spawn()
{
	Precache( );
	SET_MODEL(ENT(pev), (!FStringNull( pev->model ) ? STRING( pev->model ) : "models/anotherway/mobs/snekmobp.mdl"));
	UTIL_SetSize(pev, Vector( -16, -30, 0), Vector(7, 30, 30));

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_GREEN;
	if (!pev->health)	{ pev->health = gSkillData.zombieHealth;
	pev->view_ofs		= VEC_VIEW;// position of the eyes relative to monster's origin.
	m_flFieldOfView		= 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_DOORS_GROUP;
	m_flGroundSpeed		= 100;
	
	MonsterInit();
	
	pev->classname = MAKE_STRING( "monster_snake" );
	if ( strlen( STRING( m_szMonsterName ) ) == 0 )
	{
		// default name
		m_szMonsterName = MAKE_STRING( "Snake" );
	}
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CMSnake :: Precache()
{
	PRECACHE_MODEL("models/anotherway/mobs/snekmobp.mdl");

	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
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



int CMSnake::IgnoreConditions ( void )
{
	int iIgnore = CMBaseMonster::IgnoreConditions();
/* 
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
			m_flNextFlinch = gpGlobals->time + ZOMBIE_FLINCH_DELAY;
	}
 */
	return iIgnore;
	
}
