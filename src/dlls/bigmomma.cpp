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
#if !defined( OEM_BUILD ) && !defined( HLDEMO_BUILD )

//=========================================================
// monster template
//=========================================================
#include	"extdll.h"
#include	"util.h"
#include	"cmbase.h"
#include	"cmbasemonster.h"
#include	"monsters.h"
#include	"schedule.h"
#include	"decals.h"

#define SF_INFOBM_RUN		0x0001
#define SF_INFOBM_WAIT		0x0002

// AI Nodes for Big Momma
class CMInfoBM : public CMPointEntity
{
public:
	void Spawn( void );
	void KeyValue( KeyValueData* pkvd );

	// name in pev->targetname
	// next in pev->target
	// radius in pev->scale
	// health in pev->health
	// Reach target in pev->message
	// Reach delay in pev->speed
	// Reach sequence in pev->netname
	
	int		m_preSequence;
};

void CMInfoBM::Spawn( void )
{
	pev->classname = MAKE_STRING( "info_bigmomma" );
}

void CMInfoBM::KeyValue( KeyValueData* pkvd )
{
	if (FStrEq(pkvd->szKeyName, "radius"))
	{
		pev->scale = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "reachdelay"))
	{
		pev->speed = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "reachtarget"))
	{
		pev->message = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "reachsequence"))
	{
		pev->netname = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "presequence"))
	{
		m_preSequence = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CMPointEntity::KeyValue( pkvd );
}

//=========================================================
// Mortar shot entity
//=========================================================
class CMBMortar : public CMBaseEntity
{
public:
	void Spawn( void );

	static CMBMortar *Shoot( edict_t *pOwner, Vector vecStart, Vector vecVelocity );
	void Touch( edict_t *pOther );
	void EXPORT Animate( void );

	int  m_maxFrame;
};

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define	BIG_AE_STEP1				1		// Footstep left
#define	BIG_AE_STEP2				2		// Footstep right
#define	BIG_AE_STEP3				3		// Footstep back left
#define	BIG_AE_STEP4				4		// Footstep back right
#define BIG_AE_SACK					5		// Sack slosh
#define BIG_AE_DEATHSOUND			6		// Death sound

#define	BIG_AE_MELEE_ATTACKBR		8		// Leg attack
#define	BIG_AE_MELEE_ATTACKBL		9		// Leg attack
#define	BIG_AE_MELEE_ATTACK1		10		// Leg attack
#define BIG_AE_MORTAR_ATTACK1		11		// Launch a mortar
#define BIG_AE_LAY_CRAB				12		// Lay a headcrab
#define BIG_AE_JUMP_FORWARD			13		// Jump up and forward
#define BIG_AE_SCREAM				14		// alert sound
#define BIG_AE_PAIN_SOUND			15		// pain sound
#define BIG_AE_ATTACK_SOUND			16		// attack sound
#define BIG_AE_BIRTH_SOUND			17		// birth sound
#define BIG_AE_EARLY_TARGET			50		// Fire target early



// User defined conditions
#define bits_COND_NODE_SEQUENCE			( bits_COND_SPECIAL1 )		// pev->netname contains the name of a sequence to play

// Attack distance constants
#define BIG_ATTACKDIST		170
#define BIG_MORTARDIST		800
#define BIG_MAXCHILDREN		20			// Max # of live headcrab children


#define bits_MEMORY_CHILDPAIR		(bits_MEMORY_CUSTOM1)
#define bits_MEMORY_ADVANCE_NODE	(bits_MEMORY_CUSTOM2)
#define bits_MEMORY_COMPLETED_NODE	(bits_MEMORY_CUSTOM3)
#define bits_MEMORY_FIRED_NODE		(bits_MEMORY_CUSTOM4)

int gSpitSprite, gSpitDebrisSprite;
Vector VecCheckSplatToss( entvars_t *pev, const Vector &vecSpot1, Vector vecSpot2, float maxHeight );
void MortarSpray( const Vector &position, const Vector &direction, int spriteModel, int count );


// UNDONE:	
//
//#define BIG_CHILDCLASS		"monster_babycrab"

const char *CMBigMomma::pChildDieSounds[] = 
{
	"gonarch/gon_childdie1.wav",
	"gonarch/gon_childdie2.wav",
	"gonarch/gon_childdie3.wav",
};

const char *CMBigMomma::pSackSounds[] = 
{
	"gonarch/gon_sack1.wav",
	"gonarch/gon_sack2.wav",
	"gonarch/gon_sack3.wav",
};

const char *CMBigMomma::pDeathSounds[] = 
{
	"gonarch/gon_die1.wav",
};

const char *CMBigMomma::pAttackSounds[] = 
{
	"gonarch/gon_attack1.wav",
	"gonarch/gon_attack2.wav",
	"gonarch/gon_attack3.wav",
};
const char *CMBigMomma::pAttackHitSounds[] = 
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char *CMBigMomma::pBirthSounds[] = 
{
	"gonarch/gon_birth1.wav",
	"gonarch/gon_birth2.wav",
	"gonarch/gon_birth3.wav",
};

const char *CMBigMomma::pAlertSounds[] = 
{
	"gonarch/gon_alert1.wav",
	"gonarch/gon_alert2.wav",
	"gonarch/gon_alert3.wav",
};

const char *CMBigMomma::pPainSounds[] = 
{
	"gonarch/gon_pain2.wav",
	"gonarch/gon_pain4.wav",
	"gonarch/gon_pain5.wav",
};

const char *CMBigMomma::pFootSounds[] = 
{
	"gonarch/gon_step1.wav",
	"gonarch/gon_step2.wav",
	"gonarch/gon_step3.wav",
};


int CMBigMomma :: GetNodeSequence( void )
{
	edict_t *pTarget = m_hTargetEnt;
	if ( pTarget )
	{
		return pTarget->v.netname;	// netname holds node sequence
	}
	return 0;
}

int CMBigMomma :: GetNodePresequence( void )
{
	if (!m_hTargetEnt || (m_hTargetEnt->v.euser4 == NULL))
		return 0;
	
	CMInfoBM *pTarget = (CMInfoBM *)CMBaseEntity::Instance(m_hTargetEnt);
	if ( pTarget )
	{
		return pTarget->m_preSequence;
	}
	return 0;
}

float CMBigMomma :: GetNodeDelay( void )
{
	edict_t *pTarget = m_hTargetEnt;
	if ( pTarget )
	{
		return pTarget->v.speed;	// Speed holds node delay
	}
	return 0;
}

float CMBigMomma :: GetNodeRange( void )
{
	edict_t *pTarget = m_hTargetEnt;
	if ( pTarget )
	{
		return pTarget->v.scale;	// Scale holds node delay
	}
	return 1e6;
}

float CMBigMomma :: GetNodeYaw( void )
{
	edict_t *pTarget = m_hTargetEnt;
	if ( pTarget )
	{
		if ( pTarget->v.angles.y != 0 )
			return pTarget->v.angles.y;
	}
	return pev->angles.y;
}

BOOL CMBigMomma :: CanLayCrab( void ) 
{ 
	if ( m_crabTime < gpGlobals->time && m_crabCount < BIG_MAXCHILDREN )
	{
		// Don't spawn crabs inside each other
		Vector mins = pev->origin - Vector( 32, 32, 0 );
		Vector maxs = pev->origin + Vector( 32, 32, 0 );

		edict_t *pList[2];
		int count = UTIL_EntitiesInBox( pList, 2, mins, maxs, FL_MONSTER );
		for ( int i = 0; i < count; i++ )
		{
			if ( pList[i] != edict() )	// Don't hurt yourself!
				return FALSE;
		}
		return TRUE;
	}

	return FALSE;
}

void CMBigMomma :: KeyValue( KeyValueData *pkvd )
{
#if 0
	if (FStrEq(pkvd->szKeyName, "volume"))
	{
		m_volume = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
#endif
		CMBaseMonster::KeyValue( pkvd );
}

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CMBigMomma :: Classify ( void )
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
void CMBigMomma :: SetYawSpeed ( void )
{
	int ys;

	switch ( m_Activity )
	{
	case ACT_IDLE:
		ys = 100;
		break;
	default:
		ys = 90;
	}
	pev->yaw_speed = ys;
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//
// Returns number of events handled, 0 if none.
//=========================================================
void CMBigMomma :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
		case BIG_AE_MELEE_ATTACKBR:
		case BIG_AE_MELEE_ATTACKBL:
		case BIG_AE_MELEE_ATTACK1:
		{
			Vector forward, right;

			UTIL_MakeVectorsPrivate( pev->angles, forward, right, NULL );

			Vector center = pev->origin + forward * 128;
			Vector mins = center - Vector( 64, 64, 0 );
			Vector maxs = center + Vector( 64, 64, 64 );

			edict_t *pList[8];
			int count = UTIL_EntitiesInBox( pList, 8, mins, maxs, FL_MONSTER|FL_CLIENT );
			edict_t *pHurt = NULL;

			for ( int i = 0; i < count && !pHurt; i++ )
			{
				if ( pList[i] != this->edict() )
				{
					if ( pList[i]->v.owner != edict() )
						pHurt = pList[i];
				}
			}
					
			if ( pHurt )
			{
				if (UTIL_IsPlayer(pHurt))
				{
					UTIL_TakeDamage( pHurt, pev, pev, gSkillData.bigmommaDmgSlash, DMG_CRUSH | DMG_SLASH );
					pHurt->v.punchangle.x = 15;
					switch( pEvent->event )
					{
						case BIG_AE_MELEE_ATTACKBR:
							pHurt->v.velocity = pHurt->v.velocity + (forward * 150) + Vector(0,0,250) - (right * 200);
						break;

						case BIG_AE_MELEE_ATTACKBL:
							pHurt->v.velocity = pHurt->v.velocity + (forward * 150) + Vector(0,0,250) + (right * 200);
						break;

						case BIG_AE_MELEE_ATTACK1:
							pHurt->v.velocity = pHurt->v.velocity + (forward * 220) + Vector(0,0,200);
						break;
					}

					pHurt->v.flags &= ~FL_ONGROUND;
					EMIT_SOUND_DYN( edict(), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackHitSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
				}
				else if (pHurt->v.euser4 != NULL)
            {
					CMBaseMonster *pMonster = GetClassPtr((CMBaseMonster *)VARS(pHurt));

					pMonster->TakeDamage( pev, pev, gSkillData.bigmommaDmgSlash, DMG_CRUSH | DMG_SLASH );
					pMonster->pev->punchangle.x = 15;
					switch( pEvent->event )
					{
						case BIG_AE_MELEE_ATTACKBR:
							pMonster->pev->velocity = pMonster->pev->velocity + (forward * 150) + Vector(0,0,250) - (right * 200);
						break;

						case BIG_AE_MELEE_ATTACKBL:
							pMonster->pev->velocity = pMonster->pev->velocity + (forward * 150) + Vector(0,0,250) + (right * 200);
						break;

						case BIG_AE_MELEE_ATTACK1:
							pMonster->pev->velocity = pMonster->pev->velocity + (forward * 220) + Vector(0,0,200);
						break;
					}

					pMonster->pev->flags &= ~FL_ONGROUND;
					EMIT_SOUND_DYN( edict(), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackHitSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
				}
			}
		}
		break;
		
		case BIG_AE_SCREAM:
			EMIT_SOUND_ARRAY_DYN( CHAN_VOICE, pAlertSounds );
			break;
		
		case BIG_AE_PAIN_SOUND:
			EMIT_SOUND_ARRAY_DYN( CHAN_VOICE, pPainSounds );
			break;
		
		case BIG_AE_ATTACK_SOUND:
			EMIT_SOUND_ARRAY_DYN( CHAN_WEAPON, pAttackSounds );
			break;

		case BIG_AE_BIRTH_SOUND:
			EMIT_SOUND_ARRAY_DYN( CHAN_BODY, pBirthSounds );
			break;

		case BIG_AE_SACK:
			if ( RANDOM_LONG(0,100) < 30 )
				EMIT_SOUND_ARRAY_DYN( CHAN_BODY, pSackSounds );
			break;

		case BIG_AE_DEATHSOUND:
			EMIT_SOUND_ARRAY_DYN( CHAN_VOICE, pDeathSounds );
			break;

		case BIG_AE_STEP1:		// Footstep left
		case BIG_AE_STEP3:		// Footstep back left
			EMIT_SOUND_ARRAY_DYN( CHAN_ITEM, pFootSounds );
			break;

		case BIG_AE_STEP4:		// Footstep back right
		case BIG_AE_STEP2:		// Footstep right
			EMIT_SOUND_ARRAY_DYN( CHAN_BODY, pFootSounds );
			break;

		case BIG_AE_MORTAR_ATTACK1:
			LaunchMortar();
			break;

		case BIG_AE_LAY_CRAB:
			LayHeadcrab();
			break;

		case BIG_AE_JUMP_FORWARD:
			ClearBits( pev->flags, FL_ONGROUND );

			UTIL_SetOrigin (pev, pev->origin + Vector ( 0 , 0 , 1) );// take him off ground so engine doesn't instantly reset onground 
			UTIL_MakeVectors ( pev->angles );

			pev->velocity = (gpGlobals->v_forward * 200) + gpGlobals->v_up * 500;
			break;

		case BIG_AE_EARLY_TARGET:
			{
				edict_t *pTarget = m_hTargetEnt;
				if ( pTarget && pTarget->v.message )
					FireTargets( STRING(pTarget->v.message), this->edict(), this->edict(), USE_TOGGLE, 0 );
				Remember( bits_MEMORY_FIRED_NODE );
			}
			break;

		default:
			CMBaseMonster::HandleAnimEvent( pEvent );
			break;
	}
}

void CMBigMomma :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType )
{
/*
	if ( ptr->iHitgroup != 1 )
	{
		// didn't hit the sack?
		
		if ( pev->dmgtime != gpGlobals->time || (RANDOM_LONG(0,10) < 1) )
		{
			UTIL_Ricochet( ptr->vecEndPos, RANDOM_FLOAT( 1, 2) );
			pev->dmgtime = gpGlobals->time;
		}

		flDamage = 0.1;// don't hurt the monster much, but allow bits_COND_LIGHT_DAMAGE to be generated
	}
	else */
	if ( gpGlobals->time > m_painSoundTime )
	{
		m_painSoundTime = gpGlobals->time + RANDOM_LONG(1, 3);
		EMIT_SOUND_ARRAY_DYN( CHAN_VOICE, pPainSounds );
	}

	CMBaseMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}


int CMBigMomma :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	// Don't take any acid damage -- BigMomma's mortar is acid
	if ( bitsDamageType & DMG_ACID )
		flDamage = 0;

	if ( !HasMemory(bits_MEMORY_PATH_FINISHED) )
	{
		if ( pev->health <= flDamage )
		{
			pev->health = flDamage + 1;
			Remember( bits_MEMORY_ADVANCE_NODE | bits_MEMORY_COMPLETED_NODE );
			ALERT( at_aiconsole, "BM: Finished node health!!!\n" );
		}
	}

	return CMBaseMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

void CMBigMomma :: LayHeadcrab( void )
{
//	CMBaseEntity *pChild = CMBaseEntity::Create( BIG_CHILDCLASS, pev->origin, pev->angles, edict() );

	CMBabyCrab *pChild = CreateClassPtr((CMBabyCrab *)NULL);

	if (pChild != NULL)
	{
		pChild->pev->origin = pev->origin;
		pChild->pev->angles = pev->angles;
		pChild->pev->owner = edict();

		// Initialize these for entities who don't link to the world
		pChild->pev->absmin = pChild->pev->origin - Vector(1,1,1);
		pChild->pev->absmax = pChild->pev->origin + Vector(1,1,1);

		pChild->Spawn();

		pChild->pev->spawnflags |= SF_MONSTER_FALL_TO_GROUND;

		// Is this the second crab in a pair?
		if ( HasMemory( bits_MEMORY_CHILDPAIR ) )
		{
			m_crabTime = gpGlobals->time + RANDOM_FLOAT( 5, 10 );
			Forget( bits_MEMORY_CHILDPAIR );
		}
		else
		{
			m_crabTime = gpGlobals->time + RANDOM_FLOAT( 0.5, 2.5 );
			Remember( bits_MEMORY_CHILDPAIR );
		}

		TraceResult tr;
		UTIL_TraceLine( pev->origin, pev->origin - Vector(0,0,100), ignore_monsters, edict(), &tr);
		UTIL_DecalTrace( &tr, DECAL_MOMMABIRTH );

		EMIT_SOUND_DYN( edict(), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pBirthSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
		m_crabCount++;
	}
}



void CMBigMomma::DeathNotice( entvars_t *pevChild )
{
	if ( m_crabCount > 0 )		// Some babies may cross a transition, but we reset the count then
		m_crabCount--;
	if ( IsAlive() )
	{
		// Make the "my baby's dead" noise!
		EMIT_SOUND_ARRAY_DYN( CHAN_WEAPON, pChildDieSounds );
	}
}


void CMBigMomma::LaunchMortar( void )
{
	m_mortarTime = gpGlobals->time + RANDOM_FLOAT( 2, 15 );
	
	Vector startPos = pev->origin;
	startPos.z += 180;

	EMIT_SOUND_DYN( edict(), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pSackSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
	CMBMortar *pBomb = CMBMortar::Shoot( edict(), startPos, pev->movedir );
	if (pBomb)
	{
		pBomb->pev->gravity = 1.0;
		MortarSpray( startPos, Vector(0,0,1), gSpitSprite, 24 );
	}
}

//=========================================================
// Spawn
//=========================================================
void CMBigMomma :: Spawn()
{
	Precache( );

	SET_MODEL(ENT(pev), (!FStringNull( pev->model ) ? STRING( pev->model ) : "models/big_mom.mdl"));
//	UTIL_SetSize( pev, Vector( -32, -32, 0 ), Vector( 32, 32, 64 ) );
	UTIL_SetSize( pev, Vector( -64, -64, 0 ), Vector( 64, 64, 128 ) );

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_GREEN;
	pev->health			= 150 * gSkillData.bigmommaHealthFactor;
	pev->view_ofs		= Vector ( 0, 0, 128 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= 0.3;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;

	MonsterInit();
	
	pev->classname = MAKE_STRING( "monster_bigmomma" );
	if ( strlen( STRING( m_szMonsterName ) ) == 0 )
	{
		// default name
		m_szMonsterName = MAKE_STRING( "Big Momma" );
	}
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CMBigMomma :: Precache()
{
	PRECACHE_MODEL("models/big_mom.mdl");

	PRECACHE_SOUND_ARRAY( pChildDieSounds );
	PRECACHE_SOUND_ARRAY( pSackSounds );
	PRECACHE_SOUND_ARRAY( pDeathSounds );
	PRECACHE_SOUND_ARRAY( pAttackSounds );
	PRECACHE_SOUND_ARRAY( pAttackHitSounds );
	PRECACHE_SOUND_ARRAY( pBirthSounds );
	PRECACHE_SOUND_ARRAY( pAlertSounds );
	PRECACHE_SOUND_ARRAY( pPainSounds );
	PRECACHE_SOUND_ARRAY( pFootSounds );

//	UTIL_PrecacheOther( BIG_CHILDCLASS );
   CMBabyCrab babycrab;
   babycrab.Precache();

	// TEMP: Squid
	PRECACHE_MODEL("sprites/mommaspit.spr");// spit projectile.
	gSpitSprite = PRECACHE_MODEL("sprites/mommaspout.spr");// client side spittle.
	gSpitDebrisSprite = PRECACHE_MODEL("sprites/mommablob.spr" );

	PRECACHE_SOUND( "bullchicken/bc_acid1.wav" );
	PRECACHE_SOUND( "bullchicken/bc_spithit1.wav" );
	PRECACHE_SOUND( "bullchicken/bc_spithit2.wav" );
}	


void CMBigMomma::Activate( void )
{
	if ( m_hTargetEnt == NULL )
		Remember( bits_MEMORY_ADVANCE_NODE );	// Start 'er up
}


void CMBigMomma::NodeStart( int iszNextNode )
{
	pev->netname = iszNextNode;

	edict_t *pTarget = NULL;

	if ( pev->netname )
	{
		edict_t *pentTarget = FIND_ENTITY_BY_TARGETNAME ( NULL, STRING(pev->netname) );

		if ( !FNullEnt(pentTarget) )
			pTarget = pentTarget;
	}


	if ( !pTarget )
	{
		ALERT( at_aiconsole, "BM: Finished the path!!\n" );
		Remember( bits_MEMORY_PATH_FINISHED );
		return;
	}
	Remember( bits_MEMORY_ON_PATH );
	m_hTargetEnt = pTarget;
}


void CMBigMomma::NodeReach( void )
{
	edict_t *pTarget = m_hTargetEnt;

	Forget( bits_MEMORY_ADVANCE_NODE );

	if ( !pTarget )
		return;

	if ( pTarget->v.health )
		pev->max_health = pev->health = pTarget->v.health * gSkillData.bigmommaHealthFactor;

	if ( !HasMemory( bits_MEMORY_FIRED_NODE ) )
	{
		if ( pTarget->v.message )
			FireTargets( STRING(pTarget->v.message), this->edict(), this->edict(), USE_TOGGLE, 0 );
	}
	Forget( bits_MEMORY_FIRED_NODE );

	pev->netname = pTarget->v.target;
	if ( pTarget->v.health == 0 )
		Remember( bits_MEMORY_ADVANCE_NODE );	// Move on if no health at this node
}


	// Slash
BOOL CMBigMomma::CheckMeleeAttack1( float flDot, float flDist )
{
	if (flDot >= 0.7)
	{
		if ( flDist <= BIG_ATTACKDIST )
			return TRUE;
	}
	return FALSE;
}


// Lay a crab
BOOL CMBigMomma::CheckMeleeAttack2( float flDot, float flDist )
{
	return CanLayCrab();
}


// Mortar launch
BOOL CMBigMomma::CheckRangeAttack1( float flDot, float flDist )
{
	if ( flDist <= BIG_MORTARDIST && m_mortarTime < gpGlobals->time )
	{
		edict_t *pEnemy = m_hEnemy;

		if ( pEnemy )
		{
			Vector startPos = pev->origin;
			startPos.z += 180;
			pev->movedir = VecCheckSplatToss( pev, startPos, UTIL_BodyTarget( pEnemy, pev->origin ), RANDOM_FLOAT( 150, 500 ) );
			if ( pev->movedir != g_vecZero )
				return TRUE;
		}
	}
	return FALSE;
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================

enum
{
	SCHED_BIG_NODE = LAST_COMMON_SCHEDULE + 1,
	SCHED_NODE_FAIL,
};

enum
{
	TASK_MOVE_TO_NODE_RANGE = LAST_COMMON_TASK + 1,	// Move within node range
	TASK_FIND_NODE,									// Find my next node
	TASK_PLAY_NODE_PRESEQUENCE,						// Play node pre-script
	TASK_PLAY_NODE_SEQUENCE,						// Play node script
	TASK_PROCESS_NODE,								// Fire targets, etc.
	TASK_WAIT_NODE,									// Wait at the node
	TASK_NODE_DELAY,								// Delay walking toward node for a bit. You've failed to get there
	TASK_NODE_YAW,									// Get the best facing direction for this node
};


Task_t	tlBigNode[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_NODE_FAIL },
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FIND_NODE,			(float)0		},	// Find my next node
	{ TASK_PLAY_NODE_PRESEQUENCE,(float)0		},	// Play the pre-approach sequence if any
	{ TASK_MOVE_TO_NODE_RANGE,	(float)0		},	// Move within node range
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_NODE_YAW,			(float)0		},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_WAIT_NODE,			(float)0		},	// Wait for node delay
	{ TASK_PLAY_NODE_SEQUENCE,	(float)0		},	// Play the sequence if one exists
	{ TASK_PROCESS_NODE,		(float)0		},	// Fire targets, etc.
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},
};

Schedule_t	slBigNode[] =
{
	{ 
		tlBigNode,
		ARRAYSIZE ( tlBigNode ), 
		0,
		0,
		"Big Node"
	},
};


Task_t	tlNodeFail[] =
{
	{ TASK_NODE_DELAY,			(float)10		},	// Try to do something else for 10 seconds
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},
};

Schedule_t	slNodeFail[] =
{
	{ 
		tlNodeFail,
		ARRAYSIZE ( tlNodeFail ), 
		0,
		0,
		"NodeFail"
	},
};

DEFINE_CUSTOM_SCHEDULES( CMBigMomma )
{
	slBigNode,
	slNodeFail,
};

IMPLEMENT_CUSTOM_SCHEDULES( CMBigMomma, CMBaseMonster );




Schedule_t *CMBigMomma::GetScheduleOfType( int Type )
{
	switch( Type )
	{
		case SCHED_BIG_NODE:
			return slBigNode;
		break;

		case SCHED_NODE_FAIL:
			return slNodeFail;
		break;
	}

	return CMBaseMonster::GetScheduleOfType( Type );
}


BOOL CMBigMomma::ShouldGoToNode( void )
{
	if ( HasMemory( bits_MEMORY_ADVANCE_NODE ) )
	{
		if ( m_nodeTime < gpGlobals->time )
			return TRUE;
	}
	return FALSE;
}



Schedule_t *CMBigMomma::GetSchedule( void )
{
	if ( ShouldGoToNode() )
	{
		return GetScheduleOfType( SCHED_BIG_NODE );
	}

	return CMBaseMonster::GetSchedule();
}


void CMBigMomma::StartTask( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_FIND_NODE:
		{
			edict_t *pTarget = m_hTargetEnt;
			if ( !HasMemory( bits_MEMORY_ADVANCE_NODE ) )
			{
				if ( pTarget )
					pev->netname = m_hTargetEnt->v.target;
			}
			NodeStart( pev->netname );
			TaskComplete();
			ALERT( at_aiconsole, "BM: Found node %s\n", STRING(pev->netname) );
		}
		break;

	case TASK_NODE_DELAY:
		m_nodeTime = gpGlobals->time + pTask->flData;
		TaskComplete();
		ALERT( at_aiconsole, "BM: FAIL! Delay %.2f\n", pTask->flData );
		break;

	case TASK_PROCESS_NODE:
		ALERT( at_aiconsole, "BM: Reached node %s\n", STRING(pev->netname) );
		NodeReach();
		TaskComplete();
		break;

	case TASK_PLAY_NODE_PRESEQUENCE:
	case TASK_PLAY_NODE_SEQUENCE:
		{
			int sequence;
			if ( pTask->iTask == TASK_PLAY_NODE_SEQUENCE )
				sequence = GetNodeSequence();
			else
				sequence = GetNodePresequence();

			ALERT( at_aiconsole, "BM: Playing node sequence %s\n", STRING(sequence) );
			if ( sequence )
			{
				sequence = LookupSequence( STRING( sequence ) );
				if ( sequence != -1 )
				{
					pev->sequence = sequence;
					pev->frame = 0;
					ResetSequenceInfo( );
					ALERT( at_aiconsole, "BM: Sequence %s\n", STRING(GetNodeSequence()) );
					return;
				}
			}
			TaskComplete();
		}
		break;

	case TASK_NODE_YAW:
		pev->ideal_yaw = GetNodeYaw();
		TaskComplete();
		break;

	case TASK_WAIT_NODE:
		m_flWait = gpGlobals->time + GetNodeDelay();
		if ( m_hTargetEnt->v.spawnflags & SF_INFOBM_WAIT )
			ALERT( at_aiconsole, "BM: Wait at node %s forever\n", STRING(pev->netname) );
		else
			ALERT( at_aiconsole, "BM: Wait at node %s for %.2f\n", STRING(pev->netname), GetNodeDelay() );
		break;


	case TASK_MOVE_TO_NODE_RANGE:
		{
			edict_t *pTarget = m_hTargetEnt;
			if ( !pTarget )
				TaskFail();
			else
			{
				if ( (pTarget->v.origin - pev->origin).Length() < GetNodeRange() )
					TaskComplete();
				else
				{
					Activity act = ACT_WALK;
					if ( pTarget->v.spawnflags & SF_INFOBM_RUN )
						act = ACT_RUN;

					m_vecMoveGoal = pTarget->v.origin;
					if ( !MoveToTarget( act, 2 ) )
					{
						TaskFail();
					}
				}
			}
		}
		ALERT( at_aiconsole, "BM: Moving to node %s\n", STRING(pev->netname) );

		break;

	case TASK_MELEE_ATTACK1:
		// Play an attack sound here
		EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAttackSounds), 1.0, ATTN_NORM, 0, PITCH_NORM );
		CMBaseMonster::StartTask( pTask );
		break;

	default: 
		CMBaseMonster::StartTask( pTask );
		break;
	}
}

//=========================================================
// RunTask
//=========================================================
void CMBigMomma::RunTask( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_MOVE_TO_NODE_RANGE:
		{
			float distance;

			if ( m_hTargetEnt == NULL )
				TaskFail();
			else
			{
				distance = ( m_vecMoveGoal - pev->origin ).Length2D();
				// Set the appropriate activity based on an overlapping range
				// overlap the range to prevent oscillation
				if ( (distance < GetNodeRange()) || MovementIsComplete() )
				{
					ALERT( at_aiconsole, "BM: Reached node!\n" );
					TaskComplete();
					RouteClear();		// Stop moving
				}
			}
		}

		break;

	case TASK_WAIT_NODE:
		if ( m_hTargetEnt != NULL && (m_hTargetEnt->v.spawnflags & SF_INFOBM_WAIT) )
			return;

		if ( gpGlobals->time > m_flWaitFinished )
			TaskComplete();
		ALERT( at_aiconsole, "BM: The WAIT is over!\n" );
		break;

	case TASK_PLAY_NODE_PRESEQUENCE:
	case TASK_PLAY_NODE_SEQUENCE:
		if ( m_fSequenceFinished )
		{
			m_Activity = ACT_RESET;
			TaskComplete();
		}
		break;

	default:
		CMBaseMonster::RunTask( pTask );
		break;
	}
}



Vector VecCheckSplatToss( entvars_t *pev, const Vector &vecSpot1, Vector vecSpot2, float maxHeight )
{
	TraceResult		tr;
	Vector			vecMidPoint;// halfway point between Spot1 and Spot2
	Vector			vecApex;// highest point 
	Vector			vecScale;
	Vector			vecGrenadeVel;
	Vector			vecTemp;
	float			flGravity = g_psv_gravity->value;

	// calculate the midpoint and apex of the 'triangle'
	vecMidPoint = vecSpot1 + (vecSpot2 - vecSpot1) * 0.5;
	UTIL_TraceLine(vecMidPoint, vecMidPoint + Vector(0,0,maxHeight), ignore_monsters, ENT(pev), &tr);
	vecApex = tr.vecEndPos;

	UTIL_TraceLine(vecSpot1, vecApex, dont_ignore_monsters, ENT(pev), &tr);
	if (tr.flFraction != 1.0)
	{
		// fail!
		return g_vecZero;
	}

	// Don't worry about actually hitting the target, this won't hurt us!

	// How high should the grenade travel (subtract 15 so the grenade doesn't hit the ceiling)?
	float height = (vecApex.z - vecSpot1.z) - 15;
	// How fast does the grenade need to travel to reach that height given gravity?
	float speed = sqrt( 2 * flGravity * height );
	
	// How much time does it take to get there?
	float time = speed / flGravity;
	vecGrenadeVel = (vecSpot2 - vecSpot1);
	vecGrenadeVel.z = 0;
	float distance = vecGrenadeVel.Length();
	
	// Travel half the distance to the target in that time (apex is at the midpoint)
	vecGrenadeVel = vecGrenadeVel * ( 0.5 / time );
	// Speed to offset gravity at the desired height
	vecGrenadeVel.z = speed;

	return vecGrenadeVel;
}




// ---------------------------------
//
// Mortar
//
// ---------------------------------
void MortarSpray( const Vector &position, const Vector &direction, int spriteModel, int count )
{
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, position );
		WRITE_BYTE( TE_SPRITE_SPRAY );
		WRITE_COORD( position.x);	// pos
		WRITE_COORD( position.y);	
		WRITE_COORD( position.z);	
		WRITE_COORD( direction.x);	// dir
		WRITE_COORD( direction.y);	
		WRITE_COORD( direction.z);	
		WRITE_SHORT( spriteModel );	// model
		WRITE_BYTE ( count );			// count
		WRITE_BYTE ( 130 );			// speed
		WRITE_BYTE ( 80 );			// noise ( client will divide by 100 )
	MESSAGE_END();
}


// UNDONE: right now this is pretty much a copy of the squid spit with minor changes to the way it does damage
void CMBMortar:: Spawn( void )
{
	pev->movetype = MOVETYPE_TOSS;
	pev->classname = MAKE_STRING( "bmortar" );
	
	pev->solid = SOLID_BBOX;
	pev->rendermode = kRenderTransAlpha;
	pev->renderamt = 255;

	SET_MODEL(ENT(pev), "sprites/mommaspit.spr");
	pev->frame = 0;
	pev->scale = 0.5;

	UTIL_SetSize( pev, Vector( 0, 0, 0), Vector(0, 0, 0) );

	m_maxFrame = (float) MODEL_FRAMES( pev->modelindex ) - 1;
	pev->dmgtime = gpGlobals->time + 0.4;
}

void CMBMortar::Animate( void )
{
	pev->nextthink = gpGlobals->time + 0.1;

	if ( gpGlobals->time > pev->dmgtime )
	{
		pev->dmgtime = gpGlobals->time + 0.2;
		MortarSpray( pev->origin, -pev->velocity.Normalize(), gSpitSprite, 3 );
	}
	if ( pev->frame++ )
	{
		if ( pev->frame > m_maxFrame )
		{
			pev->frame = 0;
		}
	}
}

CMBMortar *CMBMortar::Shoot( edict_t *pOwner, Vector vecStart, Vector vecVelocity )
{
	CMBMortar *pSpit = CreateClassPtr( (CMBMortar *)NULL );
	if (pSpit)
   {
		pSpit->Spawn();
	
		UTIL_SetOrigin( pSpit->pev, vecStart );
		pSpit->pev->velocity = vecVelocity;
		pSpit->pev->owner = pOwner;
		pSpit->pev->scale = 2.5;
		pSpit->SetThink ( &CMBMortar::Animate );
		pSpit->pev->nextthink = gpGlobals->time + 0.1;
	}
	return pSpit;
}


void CMBMortar::Touch( edict_t *pOther )
{
	TraceResult tr;
	int		iPitch;

	// splat sound
	iPitch = RANDOM_FLOAT( 90, 110 );

	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "bullchicken/bc_acid1.wav", 1, ATTN_NORM, 0, iPitch );	

	switch ( RANDOM_LONG( 0, 1 ) )
	{
	case 0:
		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "bullchicken/bc_spithit1.wav", 1, ATTN_NORM, 0, iPitch );	
		break;
	case 1:
		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "bullchicken/bc_spithit2.wav", 1, ATTN_NORM, 0, iPitch );	
		break;
	}

	if ( UTIL_IsBSPModel(pOther) )
	{

		// make a splat on the wall
		UTIL_TraceLine( pev->origin, pev->origin + pev->velocity * 10, dont_ignore_monsters, ENT( pev ), &tr );
		UTIL_DecalTrace(&tr, DECAL_MOMMASPLAT);
	}
	else
	{
		tr.vecEndPos = pev->origin;
		tr.vecPlaneNormal = -1 * pev->velocity.Normalize();
	}
	// make some flecks
	MortarSpray( tr.vecEndPos, tr.vecPlaneNormal, gSpitSprite, 24 );

	entvars_t *pevOwner = NULL;
	if ( pev->owner )
		pevOwner = VARS(pev->owner);

	RadiusDamage( pev->origin, pev, pevOwner, gSkillData.bigmommaDmgBlast, gSkillData.bigmommaRadiusBlast, CLASS_NONE, DMG_ACID );
	UTIL_Remove( this->edict() );
}

#endif
