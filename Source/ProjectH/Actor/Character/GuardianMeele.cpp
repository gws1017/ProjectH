#include "Actor/Character/GuardianMeele.h"
#include "Actor/Item/Weapon/Weapon.h"
#include "Global.h"

#include "Components/SphereComponent.h"

AGuardianMeele::AGuardianMeele()
{
	MaxHP = 15;
	HP = MaxHP;
	Exp = 20;

	DetectRadius = 500.f;
	ActionRadius = 300.f;

}

void AGuardianMeele::BeginPlay()
{
	Super::BeginPlay();

	if (!!WeaponClass)
		WeaponInstance = AWeapon::Spawn<AWeapon>(GetWorld(), WeaponClass, this);

	GetMesh()->HideBoneByName("weapon_sword_l", EPhysBodyOp::PBO_None);
	GetMesh()->HideBoneByName("weapon_sword_r", EPhysBodyOp::PBO_None);
	WeaponInstance->Begin_Equip();
}

float AGuardianMeele::GetDamage(const EEquipType Type) const
{
	return WeaponInstance->GetDamage(); 
}

void AGuardianMeele::Attack()
{
	AEnemy::Attack();
	CheckNull(AttackMontage);
	CurrentStamina -= WeaponInstance->GetStaminaCost();
	//if (bAlerted && !WeaponInstance->GetEquipped())
		//WeaponInstance->Equip(EEquipType::ET_RightWeapon);
	PlayAnimMontage(AttackMontage);
}

void AGuardianMeele::Disappear()
{
	AEnemy::Disappear();
	if (WeaponInstance)
		WeaponInstance->End_UnEquip();
}