#include "Notify/ANS_UnEquip.h"
#include "Actor/Item/Weapon/Weapon.h"
#include "Actor/Character/PlayerCharacter.h"
#include "Interface/WeaponInterface.h"
#include "Global.h"

FString UANS_UnEquip::GetNotifyName_Implementation() const
{
	return "UnEquip";
}

void UANS_UnEquip::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	CheckNull(MeshComp);

	IWeaponInterface* owner = Cast<IWeaponInterface>(MeshComp->GetOwner());
	CheckNull(owner);
	auto Player = Cast<APlayerCharacter>(owner);
	if (Player)
	{
		Player->ActiveWeapon->Begin_UnEquip();
		return;
	}
	owner->GetWeapon()->Begin_UnEquip();
}

void UANS_UnEquip::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	CheckNull(MeshComp);

	IWeaponInterface* owner = Cast<IWeaponInterface>(MeshComp->GetOwner());
	CheckNull(owner);
	auto Player = Cast<APlayerCharacter>(owner);
	if (Player)
	{
		Player->ActiveWeapon->End_UnEquip();
		return;
	}
	owner->GetWeapon()->End_UnEquip();
}