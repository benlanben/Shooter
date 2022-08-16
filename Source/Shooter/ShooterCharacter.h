// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AmmoType.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied				UMETA(DisplayName = "Unoccupied"),
	ECS_FireTimerInProgress		UMETA(DisplayName = "FireTimerInProgress"),
	ECS_Reloading				UMETA(DisplayName = "Reloading"),
	ECS_Equipping				UMETA(DisplayName = "Equipping"),
	ECS_Stunned					UMETA(DisplayName = "Stunned"),

	ECS_MAX						UMETA(DisplayName = "DefaultMAX")
};

USTRUCT(BlueprintType)
struct FInterpLocation
{
	GENERATED_BODY()

	// Scene component to for its location for interping
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneComponent;

	// Number of items interping to/at this scene components' location
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ItemCount;
	
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquipItemDelegate, int32, CurrentSlotIndex, int32, NewSlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHighlightIconDelegate, int32, SlotIndex, bool, bStartAnimation);

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called for Forwards/Backwards input
	void MoveForward(float Value);

	// Called for side to side input
	void MoveRight(float Value);

	// Called via input to turn at a given rate. @param Rate is a normalised rate; 1.0 means 100% of desired turn rate
	void TurnAtRate(float Rate);

	// Called via input to look up/down at a given rate. @param Rate is a normalised rate; 1.0 means 100% of desired turn rate
	void LookUpAtRate(float Rate);

	// Rotate controlled based on mouse X & Y movement. @param Value is input value from mouse movement
	void Turn(float Value);
	void LookUp(float Value);

	// Called when the fire button is pressed
	void FireWeapon();

	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult);

	// Set bAiming to true or false with button input
	void AimingButtonPressed();
	void AimingButtonReleased();

	void CameraInterpZoom(float DeltaTime);

	// Set base rate and base look up rate based on aiming
	void SetLookRates();

	void CalculateCrosshairSpread(float DeltaTime);
	
	void StartCrosshairBulletFire();
	
	UFUNCTION()
	void FinishCrosshairBulletFire();

	void FireButtonPressed();
	void FireButtonReleased();

	void StartFireTimer();
	
	UFUNCTION()
	void AutoFireReset();

	// Line trace for items under the crosshairs
	bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation);

	// Trace for Items if OverlappedItemCount > 0
	void TraceForItems();

	// Spawns a default weapon and equips it
	class AWeapon* SpawnDefaultWeapon();

	// Takes a weapon and attaches it to the mesh
	void EquipWeapon(AWeapon* WeaponToEquip, bool bSwapping = false);

	// Detach weapon and let it fall to the ground
	void DropWeapon();

	void SelectButtonPressed();

	void SelectButtonReleased();

	// Drops currently equipped weapon and equips trace hit weapon
	void SwapWeapon(AWeapon* WeaponToSwap);

	// Initialize ammmo map with ammo values
	void InitializeAmmoMap();

	// Check to make sure the weapon has ammo
	bool WeaponHasAmmo();
	
	// FireWeapon functions
	void PlayFireSound();
	void SendBullet();
	void PlayGunfireMontage();

	// Bound to the R key and gamepad face button top
	void ReloadButtonPressed();

	// Handle weapon reloading
	void ReloadWeapon();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

	// Checks to see if the character has ammo for the EquippedWeapon type.
	bool CarryingAmmo();

	// Called from animation blueprint with GrabClip notify
	UFUNCTION(BlueprintCallable)
	void GrabClip();

	// Called from animation blueprint with ReleaseClip notify
	UFUNCTION(BlueprintCallable)
	void ReleaseClip();

	void CrouchButtonPressed();

	virtual void Jump() override;

	// Interps capsule half height transitioning between crouching & standing 
	void InterpCapsuleHalfHeight(float DeltaTime);

	void Aim();

	void StopAiming();

	void PickupAmmo(class AAmmo* Ammo);

	void InitializeInterpLocations();

	void ResetPickupSoundTimer();
	void ResetEquipSoundTimer();

	void FKeyPressed();
	void OneKeyPressed();
	void TwoKeyPressed();
	void ThreeKeyPressed();
	void FourKeyPressed();
	void FiveKeyPressed();

	void ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex);

	int32 GetEmptyInventorySlot();

	void HighlightInventorySlot();

	UFUNCTION(BlueprintCallable)
	EPhysicalSurface GetSurfaceType();

	UFUNCTION(BlueprintCallable)
	void EndStun();

	void Die();

	UFUNCTION(BlueprintCallable)
	void FinishDeath();
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Take combat damage
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "Ragdoll System")
	void RagdollStart();

	UFUNCTION(BlueprintCallable, Category = "Ragdoll System")
	void RagdollEnd();

private:
	// Camera boom positioning the camera behind the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	// Camera that follows the character 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	// Base turn rate in degrees per sec. Other scaling may effect final turn rate
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate;

	// Base look up/down rate in degrees per sec. Other scaling may effect final turn rate
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	// Turn rate while not aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipTurnRate;

	// Look up rate while not aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipLookUpRate;

	// Turn rate while aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingTurnRate;

	// Look up rate while aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingLookUpRate;

	// Mouse turn rate when not aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"),
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipTurnRate;

	// Mouse look up rate when not aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"),
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipLookUpRate;

	// Mouse turn rate when aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"),
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingTurnRate;

	// Mouse look up rate when not aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"),
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingLookUpRate;

	// Montage for firing the weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* HipFireMontage;

	// Particles spawned upon bullet impact
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;

	// Smoke trail for bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;

	// True when aiming
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bAiming;
	
	// Default camera field of view value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float CameraDefaultFOV;

	// Field of view value when zoomed in
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float CameraZoomedFOV;

	// Current field of view, this frame
	float CameraCurrentFOV;

	// Interp speed for zooming when aiming
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;

	// Determines the spread of crosshairs
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier;

	// Velocity component for crosshair spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;

	// In air component for crosshair spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor;

	// Aim component for crosshair spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor;

	// Shooting component for crosshair spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;

	float ShootTimeDuration;
	bool bFiringBullet;
	FTimerHandle CrosshairShootTimer;

	// Firing button pressed
	bool bFireButtonPressed;

	// True when we can fire, false when waiting for the timer
	bool bShouldFire;

	// Sets a timer between gunshots
	FTimerHandle AutoFireTimer;

	// True if we should trace every frame for items
	bool bShouldTraceForItems;

	// Number of overlapped Aitems
	int8 OverlappedItemCount;

	// The AItem we hit last frame
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	class AItem* TraceHitItemLastFrame;

	// Currently equipped weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AWeapon* EquippedWeapon;

	// Set this in blueprints for the default weapon class
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeaponClass;

	// The item we currently hit by TraceForItems (could be null)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AItem* TraceHitItem;

	// Distance outward from the camera for the interp destination
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpDistance;

	// Distance upward from the camera for the interp destination
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpElevation;

	// TMap to keep track of ammo of different ammo types
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	TMap<EAmmoType, int32> AmmoMap;

	// Starting amount of 9mm ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
	int32 Starting9MMAmmo;

	// Starting amount of AR ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
	int32 StartingARAmmo;

	// Combat state, can only fire or reload if unoccupied
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	ECombatState CombatState;
	
	// Montage for reload animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ReloadMontage;

	// Montage for equip animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* EquipMontage;

	// Transform of the clip used in reloading
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FTransform ClipTransform;

	// Scene component to attach to character's hand during reloading
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USceneComponent* HandSceneComponent;

	// True when crouching
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bCrouching;

	// Regular movement speed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseMovementSpeed;

	// Crouch movement speed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchMovementSpeed;

	// Current half height of the capsule
	float CurrentCapsuleHalfHeight;

	// Half height of the capsule when not crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float StandingCapsuleHalfHeight;

	// Half height of the capsule when crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchingCapsuleHalfHeight;

	// Ground friction while not crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseGroundFriction;

	// Ground friction while crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchingGroundFriction;

	// Used for knowing for the aiming button pressed
	bool bAimingButtonPressed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WeaponInterpComp;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp2;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp3;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp4;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp5;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp6;

	// Array of interp location structs
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FInterpLocation> InterpLocations;

	FTimerHandle PickupSoundTimer;
	FTimerHandle EquipSoundTimer;

	bool bShouldPlayPickupSound;
	bool bShouldPlayEquipSound;

	// Time to wait before we can play another pickup/equip sound
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float PickupSoundResetTime;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float EquipSoundResetTime;

	// Array of AItems for our inventory
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	TArray<AItem*> Inventory;

	const int32 INVENTORY_CAPACITY{6};

	// Delegate for sending slot information to InventoryBar when equipping
	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
	FEquipItemDelegate EquipItemDelegate;

	// Delegate for sending slot information for playing the icon animation
	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
	FHighlightIconDelegate HighlightIconDelegate;

	// The index for currently highlighted inventory slot
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	int32 HighlightedSlot;

	// Character max health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float MaxHealth;

	// Character current health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float Health;

	// Sound made when character gets hit by a melee attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* MeleeImpactSound;

	// Blood splatter particles for melee hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BloodParticles;

	// Montage to play for when character is stunned
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HitReactMontage;

	// Chance of getting stunned when hit by an enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float StunChance;

	// Montage to play for when character dies
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DeathMontage;

	// True when character dies
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bDead;
	
public:
	// Returns CameraBoom SubObject 
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	// Returns FollowCamera SubObject 
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE bool GetAiming() const { return bAiming; }

	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;

	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }

	// Adds or subtracts to/from OverlappedItemCount and updates bShouldTraceItems
	void IncrementOverlappedItemCount(int8 Amount);

	// No longer needed. AItem has GetInterpLocation()
	//FVector GetCameraInterpLocation();

	void GetPickupItem(AItem* Item);

	FORCEINLINE ECombatState GetCombatState() const { return CombatState; }

	FORCEINLINE bool GetCrouching() const { return bCrouching; }

	FInterpLocation GetInterpLocation(int32 Index);

	// Returns the index in InterpLocations array with the lowest item count
	int32 GetInterpLocationIndex();

	void IncrementInterpLocItemCount(int32 Index, int32 Amount);

	FORCEINLINE bool ShouldPlayPickupSound() const { return bShouldPlayPickupSound; }
	FORCEINLINE bool ShouldPlayEquipSound() const { return bShouldPlayEquipSound; }
	
	void StartPickupSoundTimer();
	void StartEquipSoundTimer();

	void UnHighlightInventorySlot();

	FORCEINLINE AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

	FORCEINLINE USoundCue* GetMeleeImpactSound() const { return MeleeImpactSound; }

	FORCEINLINE UParticleSystem* GetBloodParticles() const { return BloodParticles; }

	void Stun();

	FORCEINLINE float GetStunChance() const { return StunChance; }

	FORCEINLINE bool GetDead() const { return bDead; }
};
