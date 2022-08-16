// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"

#include "DrawDebugHelpers.h"
#include "EnemyController.h"
#include "ShooterCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AEnemy::AEnemy():
MaxHealth(100.f),
Health(MaxHealth),
HealthBarDisplayTime(4.f),
HitReactTimeMin(0.5f),
HitReactTimeMax(1.f),
bCanHitReact(true),
HitNumberDestroyTime(1.5f),
PatrolPoint1(FVector(100.f, 0.f, 0.f)),
PatrolPoint2(FVector(200.f, 0.f, 0.f)),
bStunned(false),
StunChance(0.5f),
bInAttackRange(false),
AttackLFast(TEXT("AttackLFast")),
AttackRFast(TEXT("AttackRFast")),
AttackL(TEXT("AttackL")),
AttackR(TEXT("AttackR")),
BaseDamage(10.f),
LeftWeaponSocket(TEXT("FX_Trail_L_01")),
RightWeaponSocket(TEXT("FX_Trail_R_01")),
bCanAttack(true),
AttackWaitTime(1.f),
bDying(false),
DeathTime(30.f)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create the AgroSphere
	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	AgroSphere->SetSphereRadius(1000.f);

	// Create the CombatRangeSphere
	CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatRangeSphere"));
	CombatRangeSphere->SetupAttachment(GetRootComponent());
	CombatRangeSphere->SetSphereRadius(180.f);

	// Construct weapon collision boxes
	LeftWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftWeaponBox"));
	LeftWeaponCollision->SetupAttachment(GetMesh(), FName("LeftWeaponBone"));
	RightWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightWeaponBox"));
	RightWeaponCollision->SetupAttachment(GetMesh(), FName("RightWeaponBone"));
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	// Ignore the camera for mesh and capsule comp.
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	// Bind functions to overlap events
	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOverlap);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereEndOverlap);
	CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatRangeSphereOverlap);
	CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatRangeSphereEndOverlap);
	LeftWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnLeftWeaponOverlap);
	RightWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnRightWeaponOverlap);
	
	// Set collision presets for weapon collision
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftWeaponCollision->SetCollisionObjectType(ECC_WorldDynamic);
	LeftWeaponCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	LeftWeaponCollision->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightWeaponCollision->SetCollisionObjectType(ECC_WorldDynamic);
	RightWeaponCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	RightWeaponCollision->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);

	// Get the AIController
	EnemyController = Cast<AEnemyController>(GetController());

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("bCanAttack"), true);
	}

	const FVector WorldPatrolPoint1 = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint1);
	const FVector WorldPatrolPoint2 = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint2);

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsVector(FName("PatrolPoint1"), WorldPatrolPoint1);
		EnemyController->GetBlackboardComponent()->SetValueAsVector(FName("PatrolPoint2"), WorldPatrolPoint2);

		EnemyController->RunBehaviorTree(BehaviorTree);
	}
}

void AEnemy::ShowHealthBar_Implementation()
{
	GetWorldTimerManager().ClearTimer(HealthBarTimer);
	GetWorldTimerManager().SetTimer(HealthBarTimer, this, &AEnemy::HideHealthBar, HealthBarDisplayTime);
}

void AEnemy::Die()
{
	if (bDying) return;
	bDying = true;
	
	HideHealthBar();
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}
	
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("bDead"), true);
		EnemyController->StopMovement();
	}
}

void AEnemy::PlayHitMontage(FName Section, float PlayRate)
{
	if (bCanHitReact)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && HitMontage)
		{
			AnimInstance->Montage_Play(HitMontage, PlayRate);
			AnimInstance->Montage_JumpToSection(Section, HitMontage);
		}
		bCanHitReact = false;
		const float HitReactTime{FMath::FRandRange(HitReactTimeMin, HitReactTimeMax)};
		GetWorldTimerManager().SetTimer(HitReactTimer, this, &AEnemy::ResetHitReactTimer, HitReactTime);
	}
}

void AEnemy::ResetHitReactTimer()
{
	bCanHitReact = true;
}

void AEnemy::StoreHitNumber(UUserWidget* HitNumber, FVector Location)
{
	HitNumbers.Add(HitNumber, Location);

	FTimerHandle HitNumberTimer;
	FTimerDelegate HitNumberDelegate;
	HitNumberDelegate.BindUFunction(this, FName("DestroyHitNumber"), HitNumber);
	GetWorld()->GetTimerManager().SetTimer(HitNumberTimer, HitNumberDelegate, HitNumberDestroyTime, false);
}

void AEnemy::DestroyHitNumber(UUserWidget* HitNumber)
{
	HitNumbers.Remove(HitNumber);
	HitNumber->RemoveFromParent();
}

void AEnemy::UpdateHitNumbers()
{
	for (auto& HitPair : HitNumbers)
	{
		UUserWidget* HitNumber{HitPair.Key};
		const FVector Location{HitPair.Value};
		FVector2D ScreenPosition;
		UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), Location, ScreenPosition);
		HitNumber->SetPositionInViewport(ScreenPosition);
	}
}

void AEnemy::AgroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;
	auto Character = Cast<AShooterCharacter>(OtherActor);
	if (Character && EnemyController)
	{
		// Assign Character to the Target blackboard key
		EnemyController->GetBlackboardComponent()->SetValueAsObject(FName("Target"), Character);
	}
}

void AEnemy::AgroSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;
	auto Character = Cast<AShooterCharacter>(OtherActor);
	if (Character && EnemyController)
	{
		// Clear the Target blackboard key
		//EnemyController->GetBlackboardComponent()->ClearValue(FName("Target"));
	}
}

void AEnemy::SetStunned(bool Stunned)
{
	bStunned = Stunned;
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("bStunned"), Stunned);
	}
}

void AEnemy::CombatRangeSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;
	auto Character = Cast<AShooterCharacter>(OtherActor);
	if (Character && EnemyController)
	{
		bInAttackRange = true;
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("bInAttackRange"), true);
	}
}

void AEnemy::CombatRangeSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;
	auto Character = Cast<AShooterCharacter>(OtherActor);
	if (Character && EnemyController)
	{
		bInAttackRange = false;
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("bInAttackRange"), false);
	}
}

void AEnemy::PlayAttackMontage(FName Section, float PlayRate)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage, PlayRate);
		AnimInstance->Montage_JumpToSection(Section, AttackMontage);
	}
	bCanAttack = false;
	GetWorldTimerManager().SetTimer(AttackWaitTimer, this, &AEnemy::ResetCanAttack, AttackWaitTime);
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("bCanAttack"), false);
	}
}

FName AEnemy::GetAttackSectionName()
{
	FName SectionName;
	const int32 Section{FMath::RandRange(1,4)};
	switch (Section)
	{
	case 1:
		SectionName = AttackLFast;
		break;
	case 2:
		SectionName = AttackRFast;
		break;
	case 3:
		SectionName = AttackL;
		break;
	case 4:
		SectionName = AttackR;
		break;
	}
	return SectionName;
}

void AEnemy::OnLeftWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	auto ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
	if (ShooterCharacter)
	{
		DoDamage(ShooterCharacter);
		SpawnBlood(ShooterCharacter, LeftWeaponSocket);
		StunCharacter(ShooterCharacter);
	}
}

void AEnemy::OnRightWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	auto ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
	if (ShooterCharacter)
	{
		DoDamage(ShooterCharacter);
		SpawnBlood(ShooterCharacter, RightWeaponSocket);
		StunCharacter(ShooterCharacter);
	}
}

void AEnemy::ActivateLeftWeapon()
{
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateLeftWeapon()
{
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ActivateRightWeapon()
{
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateRightWeapon()
{
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::DoDamage(AShooterCharacter* ShooterCharacter)
{	
	if (ShooterCharacter && EnemyController)
	{
		UGameplayStatics::ApplyDamage(ShooterCharacter, BaseDamage, EnemyController, this, UDamageType::StaticClass());
		if (ShooterCharacter->GetMeleeImpactSound())
		{
			UGameplayStatics::PlaySoundAtLocation(this, ShooterCharacter->GetMeleeImpactSound(), GetActorLocation());
		}
	}
}

void AEnemy::SpawnBlood(AShooterCharacter* ShooterCharacter, FName WeaponSocket)
{
	const USkeletalMeshSocket* TipSocket{GetMesh()->GetSocketByName(WeaponSocket)};
	if (TipSocket)
	{
		const FTransform SocketTransform{TipSocket->GetSocketTransform(GetMesh())};
		if (ShooterCharacter->GetBloodParticles())
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ShooterCharacter->GetBloodParticles(),
				SocketTransform);
		}
	}
}

void AEnemy::StunCharacter(AShooterCharacter* ShooterCharacter)
{
	if (ShooterCharacter)
	{
		const float Stun{FMath::FRandRange(0.f, 1.f)};
		if (Stun <= ShooterCharacter->GetStunChance())
		{
			ShooterCharacter->Stun();
		}
	}
}

void AEnemy::ResetCanAttack()
{
	bCanAttack = true;
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("bCanAttack"), true);
	}
}

void AEnemy::FinishDeath()
{
	GetMesh()->bPauseAnims = true;

	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::DestroyEnemy, DeathTime);
}

void AEnemy::DestroyEnemy()
{
	Destroy();
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateHitNumbers();

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* ShooterController)
{
	IBulletHitInterface::BulletHit_Implementation(HitResult, Shooter, ShooterController);

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, HitResult.Location, FRotator(0.f), true);
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
                         AActor* DamageCauser)
{
	// Set the Target blackboard key to agro the enemy
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsObject(FName("Target"), DamageCauser);
	}
	
	if (Health - DamageAmount <= 0.f)
	{
		Health = 0.f;
		Die();
	}
	else
	{
		Health -= DamageAmount;
	}

	if (bDying) return DamageAmount;
		
	ShowHealthBar();
	
	// Determine whether BulletHit stuns
	const float Stunned = FMath::FRandRange(0.f, 1.f);
	if (Stunned <= StunChance)
	{
		// Stun the enemy
		PlayHitMontage(FName("HitReactFront"));
		SetStunned(true);
	}
	
	return DamageAmount;
}


