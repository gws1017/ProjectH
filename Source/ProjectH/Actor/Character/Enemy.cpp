#include "Actor/Character/Enemy.h"
#include "Actor/Character/PlayerCharacter.h"
#include "Actor/Controller/EnemyController.h"
#include "Global.h"

#include "Animation/AnimMontage.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystem.h"

AEnemy::AEnemy()
	: MaxHP(1), HP(1), Exp(1)
{
	PrimaryActorTick.bCanEverTick = false;

	UHelpers::CreateComponent<USphereComponent>(this, &AgroSphere, "AgroSphere", GetRootComponent());
	UHelpers::CreateComponent<USphereComponent>(this, &CombatSphere, "CombatSphere", GetRootComponent());

	AgroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	Tags.Add("Enemy");
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AIControllerClass = EnemyControllerClass;
	EnemyController = Cast<AEnemyController>(GetController());

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapEnd);

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapEnd);

	SpawnLocation = GetActorLocation();
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageAmount <= 0.f)
		return DamageAmount;

	if (HP - DamageAmount <= 0.f) //ü���� 0�̵ɶ� ������ Die�Լ� ȣ��
	{
		HP = FMath::Clamp(HP - DamageAmount, 0.0f, MaxHP);
		//CombatTarget->IncreamentExp(Exp);
		Die();
	}
	else //�Ϲ����� ������ ���
	{
		HP = FMath::Clamp(HP - DamageAmount, 0.0f, MaxHP);
	}
	UE_LOG(LogTemp, Display, L"Enemy Current HP : %f", HP);
	return DamageAmount;
}

void AEnemy::TargetApplyDamage(APlayerCharacter* player, float damage, const FVector& HitLocation)
{
	CheckNull(player);
	player->Hit(HitLocation);
	UGameplayStatics::ApplyDamage(player, damage, GetController(), this, TSubclassOf<UDamageType>());
}

FVector AEnemy::GetCombatTargetLocation() const
{
	CheckNullResult(CombatTarget, GetActorForwardVector());
	return CombatTarget->GetActorLocation();
}

void AEnemy::Begin_Attack()
{
}

void AEnemy::End_Attack()
{
	//Notify�� ȣ��
	bAttacking = false;
}

void AEnemy::Attack()
{
	CheckFalse(Alive());
	CheckNull(CombatTarget);
	CheckFalse(CombatTarget->Alive());
	CheckFalse(bAlerted);
	CheckTrue(bAttacking);

	bAttacking = true;

	if (EnemyController)
	{
		GetController()->StopMovement();
	}
}

void AEnemy::Hit(const FVector& ParticleSpawnLocation)
{
	//AudioComponent->Play();
	if (HitParticle)
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, ParticleSpawnLocation, FRotator(0.f), false);
	PlayAnimMontage(HitMontage);
}

bool AEnemy::Alive()
{
	if (FMath::IsNearlyZero(HP)) return false;
	else return true;
}

void AEnemy::Die()
{
	//if (Spawner)
	//	Spawner->DecrementMonsterCount();
	if (CombatTarget)
		CombatTarget = nullptr;

	StopAnimMontage();
	PlayAnimMontage(DeathMontage);

	AgroSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;

	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::Disappear, DeathDelay);
}

void AEnemy::Disappear()
{
	//������鼭 �ؾ��� �͵� �ۼ�
	Destroy();
}

bool AEnemy::IsHitActorAreaAttack(const FVector& start, const FVector& end, float radius, TArray<AActor*>& HitActors)
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	EObjectTypeQuery Pawn = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn);
	ObjectTypes.Add(Pawn);

	TArray<AActor*> IgnoreActors;
	//�ڱ��ڽ��� �浹�˻� X
	IgnoreActors.Add(this);

	TArray<FHitResult> HitResults;
	bool result = UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), start, end, radius,
		ObjectTypes, false, IgnoreActors, bShowDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, HitResults, true);


	CheckFalseResult(result, result);

	for (auto hitresult : HitResults)
		HitActors.AddUnique(hitresult.GetActor());

	return result;
}

bool AEnemy::IsRanged(float radius)
{
	FVector start = GetActorLocation() - FVector(radius, 0, 0);
	FVector end = GetActorLocation() + FVector(radius, 0, 0);

	TArray<AActor*> HitActors;

	if (IsHitActorAreaAttack(start, end, radius, HitActors))
		return true;

	return false;
}

