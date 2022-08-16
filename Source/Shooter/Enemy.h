// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BulletHitInterface.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

UCLASS()
class SHOOTER_API AEnemy : public ACharacter, public IBulletHitInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent)
	void ShowHealthBar();
	void ShowHealthBar_Implementation();

	UFUNCTION(BlueprintImplementableEvent)
	void HideHealthBar();
	
	void Die();

	void PlayHitMontage(FName Section, float PlayRate = 1.0);

	void ResetHitReactTimer();

	UFUNCTION(BlueprintCallable)
	void StoreHitNumber(UUserWidget* HitNumber, FVector Location);

	UFUNCTION()
	void DestroyHitNumber(UUserWidget* HitNumber);

	void UpdateHitNumbers();

	// Called when overlapping AgroSphere
	UFUNCTION()
	void AgroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Called when overlapping with AgroSphere ended
	UFUNCTION()
	void AgroSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void SetStunned(bool Stunned);

	// Called when overlapping CombatRangeSphere
	UFUNCTION()
	void CombatRangeSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Called when overlapping with CombatRangeSphere ended
	UFUNCTION()
	void CombatRangeSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void PlayAttackMontage(FName Section, float PlayRate = 1.0);

	UFUNCTION(BlueprintPure)
	FName GetAttackSectionName();

	// Called when left weapon collides with the character
	UFUNCTION()
	void OnLeftWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Called when right weapon collides with the character
	UFUNCTION()
	void OnRightWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Activate/Deactivate collision for weapons
	UFUNCTION(BlueprintCallable)
	void ActivateLeftWeapon();
	UFUNCTION(BlueprintCallable)
	void DeactivateLeftWeapon();
	UFUNCTION(BlueprintCallable)
	void ActivateRightWeapon();
	UFUNCTION(BlueprintCallable)
	void DeactivateRightWeapon();

	void DoDamage(class AShooterCharacter* ShooterCharacter);
	
	void SpawnBlood(AShooterCharacter* ShooterCharacter, FName WeaponSocket);

	// Attempt to stun the character
	void StunCharacter(AShooterCharacter* ShooterCharacter);

	void ResetCanAttack();

	UFUNCTION(BlueprintCallable)
	void FinishDeath();

    UFUNCTION()
	void DestroyEnemy();
	
private:
	// Particles to spawn when hit by bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* ImpactParticles;

	// Sound to play when hit by bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* ImpactSound;

	// Max health of the enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float MaxHealth;

	// Current health of the enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float Health;

	// Name of the head bone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FString HeadBone;

	//Time to display HealthBar once shot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HealthBarDisplayTime;

	FTimerHandle HealthBarTimer;

	// Montage containing hit and death animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HitMontage;

	FTimerHandle HitReactTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HitReactTimeMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HitReactTimeMax;

	bool bCanHitReact;

	// Map to store hit number widgets and their hit locations
	UPROPERTY(VisibleAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TMap<UUserWidget*, FVector> HitNumbers;

	// Time before a hit number removed from the screen
	UPROPERTY(EditAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HitNumberDestroyTime;

	// BehaviorTree for the AI character
	UPROPERTY(EditAnywhere, Category = BehaviorTree, meta = (AllowPrivateAccess = "true"))
	class UBehaviorTree* BehaviorTree;

	// Points for the enemy to move to
	UPROPERTY(EditAnywhere, Category = BehaviorTree, meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPoint1;
	UPROPERTY(EditAnywhere, Category = BehaviorTree, meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPoint2;

	class AEnemyController* EnemyController;

	// Overlap sphere for when the enemy become hostile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AgroSphere;

	// True when playing the GetHit animation
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bStunned;

	// Chance of being stunned
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float StunChance;

	// True when in attack range
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bInAttackRange;

	// Overlap sphere for when the enemy can attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USphereComponent* CombatRangeSphere;

	// Montage containing attack animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* AttackMontage;

	// The 4 attack montage section names
	FName AttackLFast;
	FName AttackRFast;
	FName AttackL;
	FName AttackR;

	// Collision volumes for the weapons
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* LeftWeaponCollision;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* RightWeaponCollision;

	// Base damage for enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float BaseDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName LeftWeaponSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName RightWeaponSocket;

	// True when enemy can attack
	UPROPERTY(VisibleAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bCanAttack;

	FTimerHandle AttackWaitTimer;

	// Minimum wait time between attacks
	UPROPERTY(EditAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float AttackWaitTime;

	// DeathAnimMontage for the enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DeathMontage;

	bool bDying;

	FTimerHandle DeathTimer;

	// Time after death until Destroy()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float DeathTime;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* ShooterController) override;

	// Take combat damage
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	FORCEINLINE FString GetHeadBone() const { return HeadBone; }

	UFUNCTION(BlueprintImplementableEvent)
	void ShowHitNumber(int32 Damage, FVector Hitlocation, bool bHeadShot);

	FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }
};
