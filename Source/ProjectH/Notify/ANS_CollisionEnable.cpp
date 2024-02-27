#include "Notify/ANS_CollisionEnable.h"
#include "Actor/Item/Weapon/Weapon.h"
#include "Actor/Character/PlayerCharacter.h"
#include "Interface/WeaponInterface.h"
#include "Global.h"

FString UANS_CollisionEnable::GetNotifyName_Implementation() const
{
	return "CollisionEnable";
}

void UANS_CollisionEnable::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	CheckNull(MeshComp);

	IWeaponInterface* owner = Cast<IWeaponInterface>(MeshComp->GetOwner());
	CheckNull(owner);

	auto Player = Cast<APlayerCharacter>(owner);
	if (Player)
	{
		Player->ActiveWeapon->Begin_Collision();
		return;
	}

	AWeapon* weapon = owner->GetWeapon();
	if (!!weapon)
	{
		weapon->Begin_Collision();
	}
}

void UANS_CollisionEnable::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	CheckNull(MeshComp);

	IWeaponInterface* owner = Cast<IWeaponInterface>(MeshComp->GetOwner());
	CheckNull(owner);
	auto Player = Cast<APlayerCharacter>(owner);
	if (Player)
	{
		Player->ActiveWeapon->End_Collision();
		return;
	}
	AWeapon* weapon = owner->GetWeapon();
	if (!!weapon)
	{
		weapon->End_Collision();
	}
}
