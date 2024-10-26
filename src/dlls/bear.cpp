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
#define BEAR_ATTACK_RIGHT		( 871 )
#define BEAR_ATTACK_BITE		( 872 )
#define BEAR_ATTACK_MAUL		( 873 )

//=========================================================
// IgnoreConditions 
//=========================================================
int CMBear::IgnoreConditions(void)
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

const char *CMBear::pAttackSounds[] = 
{
	"anotherway/mobs/bear/attack1.wav",
	"anotherway/mobs/bear/attack2.wav",
	"anotherway/mobs/bear/maul.wav",
};

const char *CMBear::pAttackMissSounds[] =
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char *CMBear::pIdleSounds[] =
{
	"anotherway/mobs/bear/aggro1.wav",
	"anotherway/mobs/bear/aggro2.wav",
};

const char *CMBear::pAlertSounds[] =
{
	"anotherway/mobs/bear/aggro1.wav",
	"anotherway/mobs/bear/aggro2.wav",
};

const char *CMBear::pPainSounds[] =
{
	"anotherway/mobs/bear/wound1.wav",
	"anotherway/mobs/bear/wound2.wav",
};

const char *CMBear::pDieSounds[] =
{
	"anotherway/mobs/bear/die1.wav",
	"anotherway/mobs/bear/die2.wav",
};

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CMBear::SetYawSpeed(void)
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
int CMBear::ISoundMask( void )
{
	return 0;
}

void CMBear::HandleAnimEvent(MonsterEvent_t *pEvent)
{
	switch (pEvent->event)
	{
		case BEAR_ATTACK_RIGHT:
		{
			edict_t *pHurt = CheckTraceHullAttack( 100, RANDOM_FLOAT(70, 80), DMG_SLASH );
			if( pHurt )
			{
				if (pHurt->v.flags & (FL_MONSTER | FL_CLIENT))
				{
					pHurt->v.punchangle.z = -18;
					pHurt->v.punchangle.x = 5;
					pHurt->v.velocity = pHurt->v.velocity + gpGlobals->v_forward * 10;
				}
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackSounds[0], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
			}
			else
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackMissSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		}
		break;
		case BEAR_ATTACK_BITE:
		{
			edict_t *pHurt = CheckTraceHullAttack(100, RANDOM_FLOAT(70, 80), DMG_SLASH);
			if (pHurt)
			{
				if (pHurt->v.flags & (FL_MONSTER | FL_CLIENT))
				{
					pHurt->v.punchangle.z = 18;
					pHurt->v.punchangle.x = 5;
					pHurt->v.velocity = pHurt->v.velocity + gpGlobals->v_forward * 10;
				}
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackSounds[1], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
			}
			else // Play a random attack miss sound
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackMissSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		}
		break;
		case BEAR_ATTACK_MAUL:
		{
			edict_t *pHurt = CheckTraceHullAttack(100, RANDOM_FLOAT(100, 110), DMG_SLASH);
			if (pHurt)
			{
				if (pHurt->v.flags & (FL_MONSTER | FL_CLIENT))
				{
					pHurt->v.punchangle.z = 18;
					pHurt->v.punchangle.x = 5;
					pHurt->v.velocity = pHurt->v.velocity + gpGlobals->v_forward * 50;
				}
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackSounds[2], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
			}
			else // Play a random attack miss sound
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackMissSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackMissSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		}
		break;
	default:
		CMBaseMonster::HandleAnimEvent(pEvent);
	}
}

int	CMBear::Classify(void)
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
void CMBear::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), (!FStringNull( pev->model ) ? STRING( pev->model ) : "models/anotherway/mobs/bear_z.mdl"));
	UTIL_SetSize( pev, Vector( -32, -32, 0 ), Vector( 32, 32, 80 ) );

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = !m_bloodColor ? BLOOD_COLOR_YELLOW : m_bloodColor;
	pev->effects = 0;
	pev->health = Tier3_HP;
	switch(RANDOM_LONG(1,5)) {
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
		case 5: 
			pev->skin = 4;
			break;
	}
	m_flFieldOfView = 0.2;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = MONSTERSTATE_NONE;
	MonsterInit();
	m_flGroundSpeed = 60.0;
	pev->classname = MAKE_STRING( "monster_bear" );
	if ( strlen( STRING( m_szMonsterName ) ) == 0 )
	{
		// default name
		m_szMonsterName = MAKE_STRING( "Bear" );
	}
}
void CMBear::SetActivity(Activity NewActivity)
{
	CMBaseMonster::SetActivity(NewActivity);

	switch (m_Activity)
	{
	case ACT_WALK:
		m_flGroundSpeed = 120.0;
		break;
	case ACT_RUN:
		m_flGroundSpeed = 140.0;
		break;
	default:
		m_flGroundSpeed = 60.0;
		break;
	}
}
//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CMBear::Precache()
{
	PRECACHE_MODEL("models/anotherway/mobs/bear_z.mdl");

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
void CMBear::IdleSound(void)
{
	EMIT_SOUND(ENT(pev), CHAN_VOICE, pIdleSounds[RANDOM_LONG(0, ARRAYSIZE(pIdleSounds)-1)], 1, 1.5);
}

//=========================================================
// PainSound
//=========================================================
void CMBear::PainSound(void)
{
	int iPitch = RANDOM_LONG(85, 120);
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pPainSounds[RANDOM_LONG(0, ARRAYSIZE(pPainSounds)-1)], 1, ATTN_NORM, 0, iPitch);
}

//=========================================================
// AlertSound
//=========================================================
void CMBear::AlertSound(void)
{
	int iPitch = RANDOM_LONG(140, 160);
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pAlertSounds[RANDOM_LONG(0, ARRAYSIZE(pAlertSounds)-1)], 1, ATTN_NORM, 0, iPitch);
}
//=========================================================
// DeathSound
//=========================================================
void CMBear::DeathSound(void)
{
	EMIT_SOUND(ENT(pev), CHAN_VOICE, pDieSounds[RANDOM_LONG(0, ARRAYSIZE(pDieSounds)-1)], 1, ATTN_NORM);
}