// HUGE thanks to DrBeef for his hlsdk-xash3d-opfor repository!

/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

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
#define GUARD_ATTACK_RIGHT		( 876 )
#define GUARD_ATTACK_LEFT		( 877 )
#define GUARD_STOMP				( 878 )

//=========================================================
// IgnoreConditions 
//=========================================================
int CMDemonGuard::IgnoreConditions(void)
{
	int iIgnore = CMBaseMonster::IgnoreConditions();

	if ((m_Activity == ACT_MELEE_ATTACK1) || (m_Activity == ACT_MELEE_ATTACK2))
	{
			if (m_flNextFlinch >= gpGlobals->time)
				iIgnore |= (bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE);
	}

	if ((m_Activity == ACT_SMALL_FLINCH) || (m_Activity == ACT_BIG_FLINCH))
	{
		if (m_flNextFlinch < gpGlobals->time)
			m_flNextFlinch = gpGlobals->time + 2.0;
	}

	return iIgnore;

}

const char *CMDemonGuard::pAttackSounds[] = 
{
	"anotherway/mobs/demonguard/attack1.wav",
	"anotherway/mobs/demonguard/attack2.wav",
};

const char *CMDemonGuard::pAttackMissSounds[] =
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char *CMDemonGuard::pIdleSounds[] =
{
	"anotherway/mobs/demonguard/aggro1.wav",
	"anotherway/mobs/demonguard/aggro2.wav",
};

const char *CMDemonGuard::pAlertSounds[] =
{
	"anotherway/mobs/demonguard/alert1.wav",
	"anotherway/mobs/demonguard/alert2.wav",
};

const char *CMDemonGuard::pPainSounds[] =
{
	"anotherway/mobs/demonguard/wound1.wav",
	"anotherway/mobs/demonguard/wound2.wav",
};

const char *CMDemonGuard::pDieSounds[] =
{
	"anotherway/mobs/demonguard/die1.wav",
	"anotherway/mobs/demonguard/die2.wav",
};

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CMDemonGuard::SetYawSpeed(void)
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

//=========================================================
// ISoundMask - returns a bit mask indicating which types
// of sounds this monster regards. In the base class implementation,
// monsters care about all sounds, but no scents.
//=========================================================
int CMDemonGuard::ISoundMask( void )
{
	return 0;
}

void CMDemonGuard::HandleAnimEvent(MonsterEvent_t *pEvent)
{
	switch (pEvent->event)
	{
		case GUARD_ATTACK_RIGHT:
		{
			edict_t *pHurt = CheckTraceHullAttack( 100, RANDOM_FLOAT(200, 250), DMG_SLASH );
			if( pHurt )
			{
				if (pHurt->v.flags & (FL_MONSTER | FL_CLIENT))
				{
					pHurt->v.punchangle.z = -18;
					pHurt->v.punchangle.x = 5;
					pHurt->v.velocity = pHurt->v.velocity + gpGlobals->v_forward * 10;
				}
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
			}
			else
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackMissSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
			m_flNextAttack = gpGlobals->time + 2.0;
		}
		break;
		case GUARD_ATTACK_LEFT:
		{
			edict_t *pHurt = CheckTraceHullAttack(100, RANDOM_FLOAT(200, 250), DMG_SLASH);
			if (pHurt)
			{
				if (pHurt->v.flags & (FL_MONSTER | FL_CLIENT))
				{
					pHurt->v.punchangle.z = 18;
					pHurt->v.punchangle.x = 5;
					pHurt->v.velocity = pHurt->v.velocity + gpGlobals->v_forward * 10;
				}
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
			}
			else // Play a random attack miss sound
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackMissSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
			m_flNextAttack = gpGlobals->time + 2.0;
		}
		break;
		case GUARD_STOMP:
		{
			edict_t *pHurt = CheckTraceHullAttack(100, RANDOM_FLOAT(200, 250), DMG_SLASH);
			if (pHurt)
			{
				if (pHurt->v.flags & (FL_MONSTER | FL_CLIENT))
				{
					pHurt->v.punchangle.z = 18;
					pHurt->v.punchangle.x = 5;
					pHurt->v.velocity = pHurt->v.velocity + gpGlobals->v_forward * 250;
					pHurt->v.velocity.z *= 25;
				}
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
			}
			else // Play a random attack miss sound
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackMissSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackMissSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		}
		break;
	default:
		CMBaseMonster::HandleAnimEvent(pEvent);
	}
}

int	CMDemonGuard::Classify(void)
{
	if ( m_iClassifyOverride == -1 ) // helper
		return CLASS_NONE;
	else if ( m_iClassifyOverride > 0 )
		return m_iClassifyOverride; // override
	
	return CLASS_RACEX_PITDRONE;
}

//=========================================================
// Spawn
//=========================================================
void CMDemonGuard::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), (!FStringNull( pev->model ) ? STRING( pev->model ) : "models/anotherway/mobs/felguardc.mdl"));
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
	pev->classname = MAKE_STRING( "monster_demonguard" );
	if ( strlen( STRING( m_szMonsterName ) ) == 0 )
	{
		// default name
		m_szMonsterName = MAKE_STRING( "demonguard" );
	}
}
void CMDemonGuard::SetActivity(Activity NewActivity)
{
	CMBaseMonster::SetActivity(NewActivity);

	switch (m_Activity)
	{
	case ACT_WALK:
		m_flGroundSpeed = 60.0;
		break;
	case ACT_RUN:
		m_flGroundSpeed = 100.0;
		break;
	default:
		m_flGroundSpeed = 60.0;
		break;
	}
}
//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CMDemonGuard::Precache()
{
	PRECACHE_MODEL("models/anotherway/mobs/felguardc.mdl");

	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pDieSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
}

//=========================================================
// IdleSound
//=========================================================
void CMDemonGuard::IdleSound(void)
{
	EMIT_SOUND(ENT(pev), CHAN_VOICE, pIdleSounds[RANDOM_LONG(0, ARRAYSIZE(pIdleSounds)-1)], 1, 1.5);
}

//=========================================================
// PainSound
//=========================================================
void CMDemonGuard::PainSound(void)
{
	int iPitch = RANDOM_LONG(85, 120);
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pPainSounds[RANDOM_LONG(0, ARRAYSIZE(pPainSounds)-1)], 1, ATTN_NORM, 0, iPitch);
}

//=========================================================
// AlertSound
//=========================================================
void CMDemonGuard::AlertSound(void)
{
	int iPitch = RANDOM_LONG(140, 160);
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pAlertSounds[RANDOM_LONG(0, ARRAYSIZE(pAlertSounds)-1)], 1, ATTN_NORM, 0, iPitch);
}
//=========================================================
// DeathSound
//=========================================================
void CMDemonGuard::DeathSound(void)
{
	EMIT_SOUND(ENT(pev), CHAN_VOICE, pDieSounds[RANDOM_LONG(0, ARRAYSIZE(pDieSounds)-1)], 1, ATTN_NORM);
}