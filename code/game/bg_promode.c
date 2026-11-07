/*
 * bg_promode.c: OSP's CPM physics and item rules: the CPM settings of each mode
 * (CPM_UpdateSettings), CPM air control and the CPM ammo / armor pickup rules.
 */

#include "q_shared.h"
#include "bg_public.h"
#include "bg_local.h"

// OSP: CPM settings, in OSP's data order, including
// pm_waterfriction / pm_wateraccelerate (OSP moved them here from bg_pmove.c) and
// modeUnk_* entries for the
// words whose purpose is unknown.
float modePromode_pm_airaccelerate_1 = 1.0f;
float modePredictionKoeff2 = 0;
float modePromode_pm_airaccelerate_2 = 1.0f;
float modeWishspeedLimit = 400.0f;
int modePredictionKoeff1 = 0;
float pm_waterfriction = 1.0f;
float modeSwimScale1 = 0.5f;
float modeSwimScale2 = 0.699999988079071f;
float pm_wateraccelerate = 4.0f;
float modeUnk_2744 = 24.0f;
float modeUnk_2748 = 1.0f;
float modeAmmoBoxBullets = 50.0f;
float modeMGDamage = 7.0f;
float modeMGDamageCTF = 7.0f;
float modeMGDamageTeam = 5.0f;
int modeWeaponAmmoMG = 100;
float modeShotgunDamage = 10.0f;
float modeShotgunKoeff = 700.0f;
int modeShotgunNumberOfPellets = 11;
float modeUnused8 = 1.0f;
int modeMaxAmmoShotgun = 200;
float modeGrenadeTime = 700.0f;
int modeMaxAmmoGrenade = 200;
float modeRocketSpeed = 900.0f;
int modeMaxAmmoRocket = 200;
float modeUnk_2784 = 1.0f;
float modeUnk_2788 = 1.0f;
float modeUnk_278c = 1.0f;
int modeMaxAmmoRail = 200;
int modeAmmoBoxSlugs = 10;
float modePlasmaDamage = 20.0f;
float modeUnk_279c = 1.0f;
float modeBeginWeaponChangeTime = 200.0f;
float modeFinishWeaponChangeTime = 250.0f;
float modePMNoAmmoTime = 500.0f;
int modeBackpacks = 0;
int modeNoThruFloors = 0;
int pm_armorPromode = 0;
float modeUnk_27b8 = 0.5f;
float modeUnk_27bc = 1.25f;
int modeRespawnAmmo = 40;
int modeRespawnHealth = 35;
int modeRespawnPowerup = 120;
int modeRespawnBattlesuit = 120;
int modeRespawnPowerupCTF = 120;
int modeUnk_27d4 = 0;
int modeMegaHealthWait = 0;
float modeUnk_27dc = 1700.0f;
int modeForceRespawn = 20;
int modeHitLevelSounds = 0;
float modeSlimeDamage = 10.0f;
float modeLavaDamage = 30.0f;
int modePainDebounce = 700;
int modeUnk_27f4 = 1;
int modeUnknown2 = 0;
int modeUnknown3 = 0;
int modeUnknown4 = 0;
int modeShotgunPromode = 0;
float armorKoeff[3] = { 0.5f, 0.66f, 0.75f };   // OSP: not static, CheckArmor uses it

// OSP: defined here, not in bg_slidemove.c;
// bg_local.h's extern makes it
// the highest of this file's bss.
float modePromodePhysKoeff;
float cpm_unk_107594;                           // unknown, OSP field: 1500 in every mode, written only (CPM_UpdateSettings)

/*
==================
CPM_UpdateSettings

OSP: sets the physics/weapon/item settings for a mode (OSP_SetPhysicsMode passes
level.physicsMode: bit 1 PROMODE, bit 2 CQ3, none VQ3; bits 16 / 32 select the railgun reload
and weapon switch variants of PROMODE). Each value is
(PROMODE value : (CQ3 value : VQ3 value)); the literal types reproduce OSP's conversions.
==================
*/
void CPM_UpdateSettings(int mode) {
	modePromodePhysKoeff = (mode & 1) ? 100 : ((mode & 2) ? 0 : 0);
	pm_friction = (mode & 1) ? 6 : ((mode & 2) ? 6 : 6);
	modePredictionKoeff1 = (mode & 1) ? 1 : ((mode & 2) ? 0 : 0);
	modePromode_pm_airaccelerate_1 = (mode & 1) ? 2.5f : ((mode & 2) ? 1 : 1);
	modePredictionKoeff2 = (mode & 1) ? 150 : ((mode & 2) ? 0 : 0);
	modePromode_pm_airaccelerate_2 = (mode & 1) ? 70 : ((mode & 2) ? 1 : 1);
	modeWishspeedLimit = (mode & 1) ? 30 : ((mode & 2) ? 400 : 400);
	pm_accelerate = (mode & 1) ? 15 : ((mode & 2) ? 10 : 10);
	modeSwimScale1 = (mode & 1) ? 0.54f : ((mode & 2) ? 0.54f : 0.5f);
	modeSwimScale2 = (mode & 1) ? 0.585f : ((mode & 2) ? 0.585f : 0.7f);
	pm_waterfriction = (mode & 1) ? 0.5f : ((mode & 2) ? 0.5f : 1.0f);
	pm_wateraccelerate = (mode & 1) ? 5.0f : ((mode & 2) ? 5.0f : 4.0f);
	modeUnk_2748 = (mode & 1) ? 0.5f : ((mode & 2) ? 1 : 1);
	modeAmmoBoxBullets = (mode & 1) ? 25 : ((mode & 2) ? 30 : 50);
	modeMGDamage = (mode & 1) ? 5 : ((mode & 2) ? 5 : 7);
	modeMGDamageTeam = (mode & 1) ? 5 : ((mode & 2) ? 4 : 5);
	modeMGDamageCTF = (mode & 1) ? 5 : ((mode & 2) ? 7 : 7);
	modeWeaponAmmoMG = (mode & 1) ? 50 : ((mode & 2) ? 50 : 100);
	modeShotgunDamage = (mode & 1) ? 6 : ((mode & 2) ? 6 : 10);
	modeShotgunKoeff = (mode & 1) ? 900 : ((mode & 2) ? 900 : 700);
	modeShotgunNumberOfPellets = (mode & 1) ? 16 : ((mode & 2) ? 16 : 11);
	modeUnused8 = (mode & 1) ? 1.35f : ((mode & 2) ? 1.35f : 1.0f);
	modeMaxAmmoShotgun = (mode & 1) ? 100 : ((mode & 2) ? 100 : 200);
	modeGrenadeTime = (mode & 1) ? 700 : ((mode & 2) ? 800 : 800);
	modeMaxAmmoGrenade = (mode & 1) ? 100 : ((mode & 2) ? 100 : 200);
	modeRocketSpeed = (mode & 1) ? 1000 : ((mode & 2) ? 900 : 900);
	modeMaxAmmoRocket = (mode & 1) ? 100 : ((mode & 2) ? 100 : 200);
	modeUnk_2784 = (mode & 1) ? 1.2f : ((mode & 2) ? 1 : 1);
	modeUnk_2788 = (mode & 1) ? 1.2f : ((mode & 2) ? 1 : 1);
	modeUnk_278c = (mode & 1) ? 1.3f : ((mode & 2) ? 1 : 1);
	modeMaxAmmoRail = (mode & 1) ? 100 : ((mode & 2) ? 100 : 200);
	modeAmmoBoxSlugs = (mode & 1) ? 5 : ((mode & 2) ? 5 : 10);
	cpm_unk_107594 = (mode & 1) ? 1500 : ((mode & 2) ? 1500 : 1500);
	modePlasmaDamage = (mode & 1) ? 15 : ((mode & 2) ? 20 : 20);
	modeUnk_279c = (mode & 1) ? 0.5f : ((mode & 2) ? 1 : 1);
	modeBeginWeaponChangeTime = (mode & 1) ? 0 : ((mode & 2) ? 200 : 200);
	modeFinishWeaponChangeTime = (mode & 1) ? 0 : ((mode & 2) ? 250 : 250);
	modePMNoAmmoTime = (mode & 1) ? 100 : ((mode & 2) ? 500 : 500);
	modeUnk_27b8 = (mode & 1) ? 0.25f : ((mode & 2) ? 0.25f : 0.5f);
	modeUnk_27bc = (mode & 1) ? 1.0f : ((mode & 2) ? 1.0f : 1.25f);
	modeBackpacks = (mode & 1) ? 1 : ((mode & 2) ? 0 : 0);
	modeUnk_2744 = (mode & 1) ? 36 : ((mode & 2) ? 24 : 24);
	modeNoThruFloors = (mode & 1) ? 1 : ((mode & 2) ? 1 : 0);
	pm_armorPromode = (mode & 1) ? 1 : ((mode & 2) ? 1 : 0);
	modeRespawnAmmo = (mode & 1) ? 30 : ((mode & 2) ? 30 : 40);
	modeRespawnHealth = (mode & 1) ? 30 : ((mode & 2) ? 30 : 35);
	modeRespawnPowerup = (mode & 1) ? 60 : ((mode & 2) ? 120 : 120);
	modeRespawnBattlesuit = (mode & 1) ? 120 : ((mode & 2) ? 120 : 120);
	modeRespawnPowerupCTF = (mode & 1) ? 120 : ((mode & 2) ? 120 : 120);
	modeUnk_27d4 = (mode & 1) ? 1 : ((mode & 2) ? 1 : 3);
	modeMegaHealthWait = (mode & 1) ? 1 : ((mode & 2) ? 1 : 0);
	modeUnk_27dc = (mode & 1) ? 500 : ((mode & 2) ? 1000 : 1700);
	modeForceRespawn = (mode & 1) ? 3 : ((mode & 2) ? 10 : 20);
	modeHitLevelSounds = (mode & 1) ? 1 : ((mode & 2) ? 1 : 0);
	modePickupDistance = (mode & 1) ? 66 : ((mode & 2) ? 66 : 36);
	modeSlimeDamage = (mode & 1) ? 1.3f : ((mode & 2) ? 1.3f : 10.0f);
	modeLavaDamage = (mode & 1) ? 4 : ((mode & 2) ? 4 : 30);
	modePainDebounce = (mode & 1) ? 100 : ((mode & 2) ? 100 : 700);
	modeUnk_27f4 = (mode & 1) ? 0 : ((mode & 2) ? 1 : 1);
	modeUnknown2 = (mode & 1) ? ((mode & 16) ? 1 : 0) : ((mode & 2) ? 0 : 0);
	modeUnknown3 = (mode & 1) ? ((mode & 32) ? 200 : 0) : ((mode & 2) ? 0 : 0);
	modeUnknown4 = (mode & 1) ? ((mode & 32) ? 100 : 0) : ((mode & 2) ? 0 : 0);
	modeShotgunPromode = (mode & 1) ? 1 : ((mode & 2) ? 2 : 0);
}

/*
==================
CPM_PM_Aircontrol

OSP: CPM air control
==================
*/
void CPM_PM_Aircontrol(const pmove_t* pmove, vec3_t wishdir, float wishvel) {
	float   zspeed, speed, dot, k;

	// OSP: the CPMA SDK's CPM_PM_Aircontrol with the loops written out
	// can't control movement if not moving forward or backward
	if ((pmove->ps->movementDir && pmove->ps->movementDir != 4) || wishvel == 0) {
		return;
	}

	zspeed = pmove->ps->velocity[2];
	pmove->ps->velocity[2] = 0;
	speed = VectorNormalize(pmove->ps->velocity);

	dot = DotProduct(pmove->ps->velocity, wishdir);
	k = 32 * modePredictionKoeff2 * dot * dot * pml.frametime;

	// we can't change direction while slowing down
	if (dot > 0) {
		pmove->ps->velocity[0] = pmove->ps->velocity[0] * speed + wishdir[0] * k;
		pmove->ps->velocity[1] = pmove->ps->velocity[1] * speed + wishdir[1] * k;
		VectorNormalize(pmove->ps->velocity);
	}

	pmove->ps->velocity[0] *= speed;
	pmove->ps->velocity[1] *= speed;
	pmove->ps->velocity[2] = zspeed;
}

/*
==================
CPM_CanGrabAmmo

OSP: CPM ammo rule (BG_CanItemBeGrabbed), taking the item
==================
*/
qboolean CPM_CanGrabAmmo(const gitem_t* item, const playerState_t* ps) {
	int     ammoMax;

	switch (item->giTag) {
		case WP_SHOTGUN:
			ammoMax = modeMaxAmmoShotgun;
			break;
		case WP_GRENADE_LAUNCHER:
			ammoMax = modeMaxAmmoGrenade;
			break;
		case WP_ROCKET_LAUNCHER:
			ammoMax = modeMaxAmmoRocket;
			break;
		case WP_RAILGUN:
			ammoMax = modeMaxAmmoRail;
			break;
		default:
			ammoMax = 200;
			break;
	}

	if (ps->ammo[item->giTag] < ammoMax) {
		return qtrue;
	}
	return qfalse;
}

/*
==================
CPM_CanGrabArmor

OSP: CPM armor rule (BG_CanItemBeGrabbed): one switch
setting type and limit, *2 limit without promode armor
==================
*/
qboolean CPM_CanGrabArmor(const gitem_t* item, const playerState_t* ps) {
	int     armorTypePlayer;
	int     armorTypeItem;
	int     armorMax;
	int     armor;

	armorTypePlayer = ps->stats[STAT_ARMOR_TYPE];

	if (!pm_armorPromode) {
		return ps->stats[STAT_ARMOR] < ps->stats[STAT_MAX_HEALTH] * 2;
	}

	switch (item->quantity) {
		case 100:
			armorTypeItem = ARMOR_RED;
			armorMax = 200;
			break;
		case 50:
			armorTypeItem = ARMOR_YELLOW;
			armorMax = 150;
			break;
		case 25:
			armorTypeItem = ARMOR_GREEN;
			armorMax = 100;
			break;
		case 5:
			armorTypeItem = armorTypePlayer;
			armorMax = 200;
			break;
		default:
			return qfalse;
	}

	armor = armorKoeff[armorTypeItem] / armorKoeff[armorTypePlayer] * armorMax;

	return armorTypeItem == armorTypePlayer ? ps->stats[STAT_ARMOR] < armorMax : ps->stats[STAT_ARMOR] <= armor;
}
