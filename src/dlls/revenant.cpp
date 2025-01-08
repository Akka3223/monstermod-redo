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
#include	"nodes.h"
#include	"effects.h"
#include	"decals.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define	ZOMBIE_AE_ATTACK_BOTH		1
#define ZOMBIE_FLINCH_DELAY			2		// at most one flinch every n secs

const char *CMRevenant::pAttackHitSounds[] = 
{
	"anotherway/mobs/revenant/attack1.wav",
    "anotherway/mobs/revenant/attack2.wav",
};

const char *CMRevenant::pAttackMissSounds[] = 
{
	"anotherway/mobs/miss.wav",
};

const char *CMRevenant::pAttackSounds[] = 
{
	"anotherway/mobs/revenant/aggro.wav",
};

const char *CMRevenant::pIdleSounds[] = 
{
	"anotherway/mobs/revenant/idle.wav",
};

const char *CMRevenant::pAlertSounds[] = 
{
	"anotherway/mobs/revenant/alert.wav",
};
const char *CMRevenant::pDeathSounds[] = 
{
	"anotherway/mobs/revenant/death.wav",
};
const char *CMRevenant::pPainSounds[] = 
{
	"anotherway/mobs/revenant/pain1.wav",
	"anotherway/mobs/revenant/pain2.wav",
};

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CMRevenant :: Classify ( void )
{
	if ( m_iClassifyOverride == -1 ) // helper
		return CLASS_NONE;
	else if ( m_iClassifyOverride > 0 )
		return m_iClassifyOverride; // override
	
	return	CLASS_ALIEN_MONSTER;
}
void CMRevenant::SetActivity(Activity NewActivity)
{
	CMBaseMonster::SetActivity(NewActivity);

	switch (m_Activity)
	{
	case ACT_WALK:
		m_flGroundSpeed = 60;
		break;
	default:
		m_flGroundSpeed = 60;
		break;
	}
}
//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CMRevenant :: SetYawSpeed ( void )
{
	int ys;

	ys = 0;

	switch (m_Activity)
	{
		default:
			ys = 120;
			break;
	}

	pev->yaw_speed = ys;
}

int CMRevenant :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	// Take 30% damage from bullets
	/*if ( bitsDamageType == DMG_BULLET )
	{
		Vector vecDir = pev->origin - (pevInflictor->absmin + pevInflictor->absmax) * 0.5;
		vecDir = vecDir.Normalize();
		float flForce = DamageForce( flDamage );
		pev->velocity = pev->velocity + vecDir * flForce;
		flDamage *= 0.3;
	}*/

	// HACK HACK -- until we fix this.
	if ( IsAlive() )
		PainSound();
	return CMBaseMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

void CMRevenant :: PainSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);

	if (RANDOM_LONG(0,5) < 2)
		EMIT_SOUND_DYN ( ENT(pev), CHAN_VOICE, pPainSounds[ RANDOM_LONG(0,ARRAYSIZE(pPainSounds)-1) ], 1.0, ATTN_NORM, 0, pitch );
}

void CMRevenant :: AlertSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);

	EMIT_SOUND_DYN ( ENT(pev), CHAN_VOICE, pAlertSounds[ RANDOM_LONG(0,ARRAYSIZE(pAlertSounds)-1) ], 1.0, ATTN_NORM, 0, pitch );
}

void CMRevenant :: IdleSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);

	// Play a random idle sound
	EMIT_SOUND_DYN ( ENT(pev), CHAN_VOICE, pIdleSounds[ RANDOM_LONG(0,ARRAYSIZE(pIdleSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
}

void CMRevenant :: AttackSound( void )
{
	// Play a random attack sound
	EMIT_SOUND_DYN ( ENT(pev), CHAN_VOICE, pAttackSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
}
void CMRevenant :: DeathSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);
	EMIT_SOUND_DYN ( ENT(pev), CHAN_WEAPON, pDeathSounds[ RANDOM_LONG(0,ARRAYSIZE(pDeathSounds)-1) ], 1.0, ATTN_NORM, 0, pitch );
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CMRevenant :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
		case ZOMBIE_AE_ATTACK_BOTH:
		{
			// do stuff for this event.
			edict_t *pHurt = CheckTraceHullAttack( 90, RANDOM_FLOAT(190, 230), DMG_SLASH );
			if ( pHurt )
			{
				if ( pHurt->v.flags & (FL_MONSTER|FL_CLIENT) )
				{
					pHurt->v.punchangle.x = 5;
					pHurt->v.velocity = pHurt->v.velocity + gpGlobals->v_forward * 300;
				}
				EMIT_SOUND_DYN ( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
			}
			else
				EMIT_SOUND_DYN ( ENT(pev), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );

			if (RANDOM_LONG(0,1))
				AttackSound();

			m_flNextAttack = gpGlobals->time + 2.0;
		}
		break;

		default:
			CMBaseMonster::HandleAnimEvent( pEvent );
			break;


	}
}
BOOL CMRevenant :: CheckMeleeAttack1 ( float flDot, float flDist )
{
	if ( flDist <= 80 )	
	{
		return TRUE;
	}
	return FALSE;
}
BOOL CMRevenant :: CheckMeleeAttack2 ( float flDot, float flDist )
{
	if ( flDist <= 80 )
	{
		return TRUE;
	}
	return FALSE;
}
//=========================================================
// Spawn
//=========================================================
void CMRevenant :: Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), (!FStringNull( pev->model ) ? STRING( pev->model ) : "models/anotherway/mobs/xrevenantx.mdl"));
	UTIL_SetSize( pev, Vector( -32, -32, 0 ), Vector( 32, 32, 80 ) );

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = !m_bloodColor ? BLOOD_COLOR_YELLOW : m_bloodColor;
	pev->effects = 0;
	pev->health = Tier5_HP;

	m_flFieldOfView = 0.2;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = MONSTERSTATE_NONE;
	MonsterInit();
	m_flGroundSpeed = 60.0;
	pev->classname = MAKE_STRING( "monster_revenant" );
	if ( strlen( STRING( m_szMonsterName ) ) == 0 )
	{
		// default name
		m_szMonsterName = MAKE_STRING( "Revenant" );
	}
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CMRevenant :: Precache()
{
	PRECACHE_MODEL((char *)"models/anotherway/mobs/xrevenantx.mdl");
        
	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
}	

//=========================================================
// AI Schedules Specific to this monster
//=========================================================



int CMRevenant::IgnoreConditions ( void )
{
	int iIgnore = CMBaseMonster::IgnoreConditions();
    
    
    //if (m_flNextIdleSound < gpGlobals->time) {
   //     m_flNextIdleSound = gpGlobals->time + 6;
   //     EMIT_SOUND_DYN ( ENT(pev), CHAN_AUTO, pIdleSounds[ RANDOM_LONG(0,ARRAYSIZE(pIdleSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
   // }

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

	return iIgnore;
	
}
