#include "Component/InventoryComponent.h"
#include "System/MyGameInstance.h"
#include "Actor/Item/Item.h"
#include "Actor/Item/Weapon/Weapon.h"
#include "Actor/Character/PlayerCharacter.h"
#include "UI/Slot.h"
#include "Global.h"

UInventoryComponent::UInventoryComponent()
	:Capacity(50.f), CurrentWeight(0.f)
{
	PrimaryComponentTick.bCanEverTick = false;

	/*EquipInfo.Add({ EEquipType::ET_Shoe });
	EquipInfo.Add({ EEquipType::ET_LeftHand });
	EquipInfo.Add({ EEquipType::ET_RightHand });
	EquipInfo.Add({ EEquipType::ET_Top });
	EquipInfo.Add({ EEquipType::ET_Bottom });
	EquipInfo.Add({ EEquipType::ET_Head });
	EquipInfo.Add({ EEquipType::ET_Consumalbe });*/

	
	/*EquipInfo[EEquipType::ET_Shoe].ItemSlots.Reserve(5);
	EquipInfo[EEquipType::ET_LeftHand].ItemSlots.Reserve(5);
	EquipInfo[EEquipType::ET_RightHand].ItemSlots.Reserve(5);
	EquipInfo[EEquipType::ET_Top].ItemSlots.Reserve(5);
	EquipInfo[EEquipType::ET_Bottom].ItemSlots.Reserve(5);
	EquipInfo[EEquipType::ET_Head].ItemSlots.Reserve(5);
	EquipInfo[EEquipType::ET_Consumalbe].ItemSlots.Reserve(5);*/
	
}

// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

TArray<FInventoryItem> UInventoryComponent::GetInventoryContents() const
{
	TArray<FInventoryItem> OutArray;
	InventoryContents.GenerateValueArray(OutArray);
	return OutArray;
}

void UInventoryComponent::AddItem(const FItemData& ItemData, bool bEquipped)
{
	FInventoryItem* Item = InventoryContents.Find(ItemData.ItemCode);

	//���� Ȯ��
	if (!CheckWeight(ItemData.NumericData.Weight)) return;

	//�̹� �ִ¾������ΰ�?
	if (Item) Item->Count++;
	else InventoryContents.Add({ ItemData.ItemCode ,FInventoryItem(ItemData, bEquipped) });

	CurrentWeight += ItemData.NumericData.Weight;

	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::AddItem(const int64 ItemCode, bool bEquipped)
{
	FInventoryItem* Item = InventoryContents.Find(ItemCode);
	FItemData* Data = GetOwner()->GetGameInstance<UMyGameInstance>()->GetItemData(ItemCode);
	if (Data == nullptr)
		return;
	//���� Ȯ��
	if (!CheckWeight(Data->NumericData.Weight)) return;

	//�̹� �ִ¾������ΰ�?
	if (Item) Item->Count++;
	else
	{
		FInventoryItem NewItem = FInventoryItem(*Data, bEquipped);
		USlot* Slot = CreateWidget<USlot>(GetWorld(), SlotClass);
		if (Slot)
		{
			Slot->SetItemCode(ItemCode);
			ItemSlots.Add(Slot);
			OnInventoryUpdated.AddUFunction(Slot, "SlotUpdate");
			NewItem.Slot = Slot;
			InventoryContents.Add({ ItemCode ,NewItem });

		}
		
	}
	CurrentWeight += Data->NumericData.Weight;
	
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::RemoveItem(const int64 ItemCode)
{
	auto item = InventoryContents.Find(ItemCode);
	if (item->Count <= 1)
		InventoryContents.Remove(ItemCode);
	else
		item->Count--;

	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::ClearInventory()
{
	InventoryContents.Empty();
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::UpdateSlot()
{
	for (auto [code, item] : InventoryContents)
	{
		item.Slot->SetItemCode(code);
	}
	OnInventoryUpdated.Broadcast();

}

//void UInventoryComponent::UpdateEquipItem()
//{
//	for (auto [Type, Slots] : EquipInfo)
//	{
//		if (Slots.ItemSlots.IsEmpty()) continue;
//		//���Կ� ���ִ� ��� �� ���� ������ ��� �����Ѵ�.
//		auto Slot = EquipInfo[Type].ItemSlots[Slots.SelectSlot];
//		if (Slot && Slot->bEquipped)
//		{
//			GetCDOItem(Slot->ItemInfo.AssetData.ItemClass)->Equip(Type);
//		}
//		
//	}
//}
//
//void UInventoryComponent::EquipInSlot(const int64 ItemCode, int32 SlotNumber)
//{
//	FInventoryItem* Data = InventoryContents.Find(ItemCode);
//	CheckNull(Data);
//	EEquipType Type = Data->ItemInfo.EquipType;
//
//	FInventoryItem* Slot = EquipInfo[Type].ItemSlots[SlotNumber];
//	if(Slot != nullptr)
//	{
//		(Slot)->bEquipped = false;
//	}
//
//	Slot = Data;
//	Data->bEquipped = true;
//}
//
//void UInventoryComponent::UnEquipInSlot(EEquipType Type, int32 SlotNumber)
//{
//	auto Slot = EquipInfo[Type].ItemSlots[SlotNumber];
//	Slot->bEquipped = false;
//	Slot = nullptr;
//}

void UInventoryComponent::Equip(USlot* Slot, AItem* ItemInstance)
{
	//UI���� ���޹��� ���������� �������� ����Ѵ�
	if (Slot->bEquipped == false || Slot == nullptr)
	{
		CLog::Log("Equip State Error");
		return;
	}
	auto Type = Slot->ItemInfo.EquipType;
	AItem* Instance = GetCDOItem(Slot->ItemInfo.AssetData.ItemClass);
	if (Type == EEquipType::ET_LeftHand || Type == EEquipType::ET_RightHand)
	{
		Instance = AItem::Spawn<AItem>(GetWorld(),
			Slot->ItemInfo.AssetData.ItemClass, Cast<ACharacter>(GetOwner()));
		Cast<APlayerCharacter>(GetOwner())->SetWeapon(Cast<AWeapon>(Instance));
	}
	if (!!ItemInstance)
		ItemInstance = Instance;
	Instance->Equip(Slot->ItemInfo.EquipType);
}

void UInventoryComponent::UnEquip(USlot* Slot)
{

	if (Slot->bEquipped == false || Slot == nullptr)
	{
		CLog::Log("Equip State Error");
		return;
	}
	GetCDOItem(Slot->ItemInfo.AssetData.ItemClass)->UnEquip(Slot->ItemInfo.EquipType);
}

void UInventoryComponent::Use(const int64 ItemCode)
{
	FInventoryItem* Data = InventoryContents.Find(ItemCode);
	CheckNull(Data);
	GetCDOItem(Data->ItemInfo.AssetData.ItemClass)->Use();
}

bool UInventoryComponent::CheckWeight(float weight)
{
	if (CurrentWeight + weight <= Capacity)
		return true;
	//UI�� ���ų� �޽����� ����
	CLog::Print("�κ��丮 �߷��� �ʰ��߽��ϴ�.");
	return false;
}

FInventoryItem UInventoryComponent::GetItemDataFromSlot(USlot* Slot)
{
	for (const auto& [code, item] : InventoryContents)
	{
		if (item.Slot == Slot)
		{

			return item;
		}
	}
	return FInventoryItem{};
}

AItem* UInventoryComponent::GetCDOItem(TSubclassOf<AItem> Item) const
{
	if (Item)
	{
		AItem* CDO = Item.GetDefaultObject();
		CDO->SetOwner(GetOwner());
		return CDO;
	}
	else
	{
		CLog::Log("Item Class is Null!");
		return nullptr;
	}
}

