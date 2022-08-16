// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"

#include "Ammo.h"
#include "BulletHitInterface.h"
#include "DrawDebugHelpers.h"
#include "Enemy.h"
#include "EnemyController.h"
#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Item.h"
//#include "Shooter.h"
#include "Weapon.h"
#include "BehaviorTree/BlackboardComponent.h"
//#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
//#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

// Sets default values
AShooterCharacter::AShooterCharacter() :
// Base rates for turning/looking up
BaseTurnRate(45.f),
BaseLookUpRate(45.f),
// Turn rates for aiming/not aiming
HipTurnRate(90.f),
HipLookUpRate(90.f),
AimingTurnRate(40.f),
AimingLookUpRate(40.f),
//Mouse look sensitivity scale factors
MouseHipTurnRate(1.f),
MouseHipLookUpRate(1.f),
MouseAimingTurnRate(0.9f),
MouseAimingLookUpRate(0.9f),
// True when aiming 
bAiming(false),
// Camera field of view values
CameraDefaultFOV(0.f), // Set in BeginPlay()
CameraZoomedFOV(24.f),
CameraCurrentFOV(0.f),
ZoomInterpSpeed(20.f),
// Crosshair spread factors
CrosshairSpreadMultiplier(0.f),
CrosshairVelocityFactor(0.f),
CrosshairInAirFactor(0.f),
CrosshairAimFactor(0.f),
CrosshairShootingFactor(0.f),
// Bullet fire timer variables
ShootTimeDuration(0.05f),
bFiringBullet(false),
// Automatic fire variables
bFireButtonPressed(false),
bShouldFire(true),
// Item trace variables
bShouldTraceForItems(false),
OverlappedItemCount(0),
// Camera interp location variables
CameraInterpDistance(150.f),
CameraInterpElevation(10.f),
// Ammo variables
Starting9MMAmmo(85),
StartingARAmmo(230),
// Combat variables
CombatState(ECombatState::ECS_Unoccupied),
bCrouching(false),
//Movement speed
BaseMovementSpeed(650.f),
CrouchMovementSpeed(300.f),
CurrentCapsuleHalfHeight(88.f),
StandingCapsuleHalfHeight(88.f),
CrouchingCapsuleHalfHeight(55.f),
BaseGroundFriction(2.f),
CrouchingGroundFriction(100.f),
bAimingButtonPressed(false),
// Pickup sound timer properties
bShouldPlayPickupSound(true),
bShouldPlayEquipSound(true),
PickupSoundResetTime(0.1f),
EquipSoundResetTime(0.1f),
// Icon animation properties
HighlightedSlot(-1),
MaxHealth(100.f),
Health(MaxHealth),
StunChance(0.1f),
bDead(false)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a CameraBoom (pulls in towards the character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 240.f; // The Camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector(0.f, 35.f, 80.f);

	// Create a FollowCamera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach FollowCamera to the end of the CameraBoom
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to the arm

	// Don't rotate in pitch & roll when the controller rotates
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input ...
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // ... at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create HandSceneComponent
	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComponent"));

	// Create interpolation components
	WeaponInterpComp = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponInterpolationComponent"));
	WeaponInterpComp->SetupAttachment(GetFollowCamera());
	InterpComp1 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent1"));
	InterpComp1->SetupAttachment(GetFollowCamera());
	InterpComp2 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent2"));
	InterpComp2->SetupAttachment(GetFollowCamera());
	InterpComp3 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent3"));
	InterpComp3->SetupAttachment(GetFollowCamera());
	InterpComp4 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent4"));
	InterpComp4->SetupAttachment(GetFollowCamera());
	InterpComp5 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent5"));
	InterpComp5->SetupAttachment(GetFollowCamera());
	InterpComp6 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent6"));
	InterpComp6->SetupAttachment(GetFollowCamera());	
	
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}

	// Spawn the default weapon and equip it / Disable material effects
	EquipWeapon(SpawnDefaultWeapon());
	Inventory.Add(EquippedWeapon);
	EquippedWeapon->SetSlotIndex(0);
	EquippedWeapon->DisableCustomDepth();
	EquippedWeapon->DisableGlowMaterial();
	EquippedWeapon->SetCharacter(this);
	
	InitializeAmmoMap();

	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;

	// Create FInterpLocation structs for each interp location, add to array
	InitializeInterpLocations();
}

void AShooterCharacter::MoveForward(float Value)
{
	if (Controller && Value != 0)
	{
		// Find out which way is forward
		const FRotator Rotation{Controller->GetControlRotation()};
		const FRotator YawRotation{0, Rotation.Yaw, 0};

		const FVector Direction{FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X)};
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if (Controller && Value != 0)
	{
		// Find out which way is right
		const FRotator Rotation{Controller->GetControlRotation()};
		const FRotator YawRotation{0, Rotation.Yaw, 0};

		const FVector Direction{FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y)};
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	// degrees per second * seconds per frame
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	// degrees per second * seconds per frame
}

void AShooterCharacter::Turn(float Value)
{
	float TurnScaleFactor{};
	if (bAiming)
	{
		TurnScaleFactor = MouseAimingTurnRate;
	}
	else
	{
		TurnScaleFactor = MouseHipTurnRate;
	}
	AddControllerYawInput(Value * TurnScaleFactor);
}

void AShooterCharacter::LookUp(float Value)
{
	float LookUpScaleFactor{};
	if (bAiming)
	{
		LookUpScaleFactor = MouseAimingLookUpRate;
	}
	else
	{
		LookUpScaleFactor = MouseHipLookUpRate;
	}
	AddControllerPitchInput(Value * LookUpScaleFactor);
}

void AShooterCharacter::FireWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	if (WeaponHasAmmo())
	{
		PlayFireSound();
		SendBullet();
		PlayGunfireMontage();
		EquippedWeapon->DecrementAmmo();
		
		StartFireTimer();
		// Start bullet fire timer for crosshairs
		StartCrosshairBulletFire();

		if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol)
		{
			// Start moving slide timer
			EquippedWeapon->StartSlideTimer();
		}
	}	
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult)
{
	FVector OutBeamLocation;
	// Check for crosshair trace hit
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation);

	if (bCrosshairHit)
	{
		// Tentative beam location - Still need to trace from gun barrel
		OutBeamLocation = CrosshairHitResult.Location;
	}
	else // No crosshair trace hit
	{
		// OutBeamLocation is the end location for the line trace
	}

	// Perform a second trace, this time from the gun barrel
	const FVector WeaponTraceStart{MuzzleSocketLocation};
	const FVector StartToEnd{OutBeamLocation - MuzzleSocketLocation};
	const FVector WeaponTraceEnd{OutBeamLocation + StartToEnd * 1.25f};

	GetWorld()->LineTraceSingleByChannel(OutHitResult, WeaponTraceStart, WeaponTraceEnd,
										 ECollisionChannel::ECC_Visibility);
	if (!OutHitResult.bBlockingHit) // if there is not an object between the barrel and beam end point
	{
		OutHitResult.Location = OutBeamLocation;
		return false;
	}
	return true;
}

void AShooterCharacter::AimingButtonPressed()
{
	bAimingButtonPressed = true;
	if (CombatState != ECombatState::ECS_Reloading && CombatState != ECombatState::ECS_Equipping && CombatState != ECombatState::ECS_Stunned)
	{
		Aim();
	}
}

void AShooterCharacter::AimingButtonReleased()
{
	bAimingButtonPressed = false;
	StopAiming();
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	// If aiming button is pressed interpolate to zoomed field of view
	if (bAiming)
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
		GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
	}
	else if(CameraCurrentFOV != CameraDefaultFOV) // If aiming button is released interpolate back to default field of view
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
		GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
	}
}

void AShooterCharacter::SetLookRates()
{
	if (bAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else if(BaseTurnRate != HipTurnRate || BaseLookUpRate != HipLookUpRate)
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange{0.f, 600.f};
	FVector2D VelocityMultiplierRange{0.f, 1.f};
	FVector Velocity{GetVelocity()};
	Velocity.Z = 0.f;

	// Calculate crosshair velocity factor
	if (Velocity.Size() > 0)
	{
		CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
	}
	else
	{
		CrosshairVelocityFactor = 0.f;
	}
		
	// Calculate crosshair in air factor
	if (GetCharacterMovement()->IsFalling()) // is in air?
	{
		// Spread the crosshairs slowly while in air
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else if(CrosshairInAirFactor > 0.f) // Character has just landed on the ground
	{
		// Shrink the crosshairs rapidly while on the ground
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
	}

	// Calculate crosshair aiming factor
	if (bAiming) // Are we aiming?
	{
		// Shrink crosshairs a small amount very quickly
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.45f, DeltaTime, 30.f);
	}
	else if(CrosshairAimFactor > 0.f) // Just stopped aiming
	{
		// Spread crosshairs back to normal very quickly
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}

	// True 0.05 seconds after firing
	if (bFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 1.f, DeltaTime, 60.f);
	}
	else if(CrosshairShootingFactor > 0.f)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.0f, DeltaTime, 60.f);
	}
	
	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;
	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &AShooterCharacter::FinishCrosshairBulletFire, ShootTimeDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	FireWeapon();	
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	CombatState = ECombatState::ECS_FireTimerInProgress;
	
	GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, EquippedWeapon->GetAutoFireRate());
}

void AShooterCharacter::AutoFireReset()
{
	if (CombatState == ECombatState::ECS_Stunned) return;
	CombatState = ECombatState::ECS_Unoccupied;

	if (WeaponHasAmmo())
	{
		if (bFireButtonPressed && EquippedWeapon->GetAutomatic())
		{
			FireWeapon();
		}
	}
	else
	{
		ReloadWeapon();
	}
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	// Get current Viewport's size
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	
	// Get screenspace location of crosshairs
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	CrosshairLocation.Y -= 18.f;
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Get world position & direction of crosshairs
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
																   CrosshairLocation, CrosshairWorldPosition,
																   CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		// Trace from crosshair's world location outward
		const FVector Start{CrosshairWorldPosition};
		const FVector End{Start + CrosshairWorldDirection * 50000};
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}
	return false;
}

void AShooterCharacter::TraceForItems()
{
	if (bShouldTraceForItems)
	{
		FHitResult ItemTraceResult;
		FVector HitLocation;
		TraceUnderCrosshairs(ItemTraceResult, HitLocation);
		if (ItemTraceResult.bBlockingHit)
		{
			TraceHitItem = Cast<AItem>(ItemTraceResult.Actor);
			const auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);

			if (TraceHitWeapon)
			{
				if (HighlightedSlot == -1)
				{
					// Not Currently highlighting a slot, highlight one
					HighlightInventorySlot();
				}
			}
			else
			{
				// Is a slot being highlighted?
				if (HighlightedSlot != -1)
				{
					UnHighlightInventorySlot();
				}
			}

			if (TraceHitItem && TraceHitItem->GetItemState() == EItemState::EIS_EquipInterping)
			{
				TraceHitItem = nullptr;
			}
			
			if (TraceHitItem && TraceHitItem->GetPickupWidget())
			{
				// Show item's pick up widget
				TraceHitItem->GetPickupWidget()->SetVisibility(true);
				TraceHitItem->EnableCustomDepth();

				if (Inventory.Num() >= INVENTORY_CAPACITY)
				{
					// Inventory is full
					TraceHitItem->SetCharacterInventoryFull(true);					
				}
				else
				{
					// Inventory has room
					TraceHitItem->SetCharacterInventoryFull(false);		
				}
			}
			
			// We hit an AItem last frame
			if (TraceHitItemLastFrame)
			{
				if (TraceHitItem != TraceHitItemLastFrame)
				{
					// We are hitting a different AItem this frame from last frame
					// Or AItem is null
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
					TraceHitItemLastFrame->DisableCustomDepth();
				}
			}
			// Store a reference to HitItem for next frame
			TraceHitItemLastFrame = TraceHitItem;
		}
	}
	else if (TraceHitItemLastFrame)
	{
		// No longer overlapping any items
		// Item last frame should not show widget
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
		TraceHitItemLastFrame->DisableCustomDepth();
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	// Check the TSubclassOf variable
	if	(DefaultWeaponClass)
	{
		// Spawn the default weapon set in blueprints
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	}
	return nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip, bool bSwapping)
{
	if (WeaponToEquip)
	{
		// Get a ref to RightHandSocket
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			// Attach the default weapon to the RightHandSocket
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}
		
		if (!EquippedWeapon)
		{
			// -1 == No EquippedWeapon yet, no need to reverse the icon animation
			EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());
		}
		else if (!bSwapping)
		{
			EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(), WeaponToEquip->GetSlotIndex());
		}
			
		// Set equipped weapon to the newly spawned weapon
		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}
}

void AShooterCharacter::DropWeapon()
{
	if (EquippedWeapon)
	{
		FDetachmentTransformRules DetachmentTransformRules{EDetachmentRule::KeepWorld, true};
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);
		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();
	}
}

void AShooterCharacter::SelectButtonPressed()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	if (TraceHitItem)
	{
		TraceHitItem->StartItemCurve(this, true);
		TraceHitItem = nullptr;
	}
}

void AShooterCharacter::SelectButtonReleased()
{
	
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	if (Inventory.Num() - 1 >= EquippedWeapon->GetSlotIndex())
	{
		Inventory[EquippedWeapon->GetSlotIndex()] = WeaponToSwap;
		WeaponToSwap->SetSlotIndex(EquippedWeapon->GetSlotIndex());
	}
	DropWeapon();
	EquipWeapon(WeaponToSwap, true);
	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9MMAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

bool AShooterCharacter::WeaponHasAmmo()
{
	if (!EquippedWeapon) return false;
	return EquippedWeapon->GetAmmo() > 0;
}

void AShooterCharacter::PlayFireSound()
{
	// Play fire sound
	if (EquippedWeapon->GetFireSound())
	{
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->GetFireSound());
	}
}

void AShooterCharacter::SendBullet()
{
	// Send bullet
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());

		if (EquippedWeapon->GetMuzzleFlash())
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EquippedWeapon->GetMuzzleFlash(), SocketTransform);
		}

		FHitResult BeamHitResult;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamHitResult);

		if (bBeamEnd)
		{
			// Does hit ACtor implement BulletHitInterface?
			if (BeamHitResult.Actor.IsValid())
			{
				IBulletHitInterface* BulletHitInterface = Cast<IBulletHitInterface>(BeamHitResult.Actor.Get());
				if (BulletHitInterface)
				{
					BulletHitInterface->BulletHit_Implementation(BeamHitResult, this, GetController());
				}
				AEnemy* HitEnemy = Cast<AEnemy>(BeamHitResult.Actor.Get());
				if (HitEnemy)
				{
					int32 Damage{};
					if (BeamHitResult.BoneName.ToString() == HitEnemy->GetHeadBone())
					{
						// Head shot
						Damage = EquippedWeapon->GetHeadShotDamage();
						UGameplayStatics::ApplyDamage(BeamHitResult.Actor.Get(), Damage,
						GetController(), this, UDamageType::StaticClass());
						HitEnemy->ShowHitNumber(Damage, BeamHitResult.Location, true);
					}
					else
					{
						// Body shot
						Damage = EquippedWeapon->GetDamage();
						UGameplayStatics::ApplyDamage(BeamHitResult.Actor.Get(), Damage,
						GetController(), this, UDamageType::StaticClass());
						HitEnemy->ShowHitNumber(Damage, BeamHitResult.Location, false);
					}
				}
			}
			else
			{
				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamHitResult.Location);
				}

				if (BeamParticles)
				{
					UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
						GetWorld(), BeamParticles, SocketTransform);
					if (Beam)
					{
						Beam->SetVectorParameter(FName("Target"), BeamHitResult.Location);
					}
				}	
			}
		}
	}
}

void AShooterCharacter::PlayGunfireMontage()
{
	// Play HipFireMontage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

void AShooterCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	
	// Do we have ammo of the correct type? & is the clip full?
	if (CarryingAmmo() && !EquippedWeapon->ClipIsFull()) 
	{
		if (bAiming)
		{
			StopAiming();
		}
		
		CombatState = ECombatState::ECS_Reloading;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && ReloadMontage)
		{
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());
		}	
	}
}

void AShooterCharacter::FinishReloading()
{
	if (CombatState == ECombatState::ECS_Stunned) return;
	
	// Update CombatState
	CombatState = ECombatState::ECS_Unoccupied;

	if (bAimingButtonPressed)
	{
		Aim();
	}

	if (!EquippedWeapon) return;

	const auto AmmoType = EquippedWeapon->GetAmmoType();
	// Update AmmoMap
	if (AmmoMap.Contains(AmmoType))
	{
		// Amount of ammo the character is carrying of the EquippedWeapon type
		int32 CarriedAmmo = AmmoMap[AmmoType];
		// Space left in the magazine of EquippedWeapon
		const int32 MagEmptySpace = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo();
		if (MagEmptySpace > CarriedAmmo)
		{
			// Reload the magazine with all the ammo we're carrying
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
		else
		{
			// Fill the magazine
			EquippedWeapon->ReloadAmmo(MagEmptySpace);
			CarriedAmmo -= MagEmptySpace;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
	}
}

void AShooterCharacter::FinishEquipping()
{
	if (CombatState == ECombatState::ECS_Stunned) return;
	
	CombatState = ECombatState::ECS_Unoccupied;
	if (bAimingButtonPressed)
	{
		Aim();
	}
}

bool AShooterCharacter::CarryingAmmo()
{
	if (!EquippedWeapon) return false;
	auto AmmoType = EquippedWeapon->GetAmmoType();
	
	if (AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;	
	}
	return false;
}

void AShooterCharacter::GrabClip()
{
	if (!EquippedWeapon) return;
	if (!HandSceneComponent) return;

	// Index for the clip bone on the equip weapon
	int32 ClipBoneIndex{EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName())};
	// Store the transform of the clip
	ClipTransform = EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("Hand_L")));
	HandSceneComponent->SetWorldTransform(ClipTransform);
	EquippedWeapon->SetMovingClip(true);
}

void AShooterCharacter::ReleaseClip()
{
	EquippedWeapon->SetMovingClip(false);
}

void AShooterCharacter::CrouchButtonPressed()
{
	if (!GetCharacterMovement()->IsFalling())
	{
		bCrouching = !bCrouching;
	}
	if (bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
		GetCharacterMovement()->GroundFriction = CrouchingGroundFriction;		
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
		GetCharacterMovement()->GroundFriction = BaseGroundFriction;
	}
}

void AShooterCharacter::Jump()
{
	if (bCrouching)
	{
		bCrouching = false;
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
		GetCharacterMovement()->GroundFriction = BaseGroundFriction;
	}
	else
	{
		Super::Jump();
		
		ACharacter::Jump();
	}
}

void AShooterCharacter::InterpCapsuleHalfHeight(float DeltaTime)
{
	float TargetCapsuleHalfHeight{};
	if (bCrouching)
	{
		TargetCapsuleHalfHeight = CrouchingCapsuleHalfHeight;
	}
	else
	{
		TargetCapsuleHalfHeight = StandingCapsuleHalfHeight;
	}
	const float InterpHalfHeight{FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
		TargetCapsuleHalfHeight, DeltaTime, 5.f)};
	// Negative value if crouching, positive value if standing
	const float DeltaCapsuleHalfHeight{InterpHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight()};
	const FVector MeshOffset{0.f, 0.f, -DeltaCapsuleHalfHeight};
	GetMesh()->AddLocalOffset(MeshOffset);
	
	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHeight);
}

void AShooterCharacter::Aim()
{
	bAiming = true;
	GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
}

void AShooterCharacter::StopAiming()
{
	bAiming = false;
	if (!bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
}

void AShooterCharacter::PickupAmmo(AAmmo* Ammo)
{
	// Check to see if AmmoMap contains Ammo's AmmoType
	if (AmmoMap.Find(Ammo->GetAmmoType()))
	{
		// Get amount of ammo in our AmmoMap for Ammo's type
		int32 AmmoCount{AmmoMap[Ammo->GetAmmoType()]};
		AmmoCount += Ammo->GetItemCount();
		// Set the amount of ammo in the AmmoMap for this type
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}

	if (EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType())
	{
		// Check to see if the gun is empty
		if (EquippedWeapon->GetAmmo() == 0)
		{
			ReloadWeapon();
		}
	}

	Ammo->Destroy();
}

void AShooterCharacter::InitializeInterpLocations()
{
	FInterpLocation WeaponLocation{WeaponInterpComp, 0};
	InterpLocations.Add(WeaponLocation);

	FInterpLocation InterpLoc1{InterpComp1, 0};
	InterpLocations.Add(InterpLoc1);
	
	FInterpLocation InterpLoc2{InterpComp2, 0};
	InterpLocations.Add(InterpLoc2);
	
	FInterpLocation InterpLoc3{InterpComp3, 0};
	InterpLocations.Add(InterpLoc3);
	
	FInterpLocation InterpLoc4{InterpComp4, 0};
	InterpLocations.Add(InterpLoc4);
	
	FInterpLocation InterpLoc5{InterpComp5, 0};
	InterpLocations.Add(InterpLoc5);
	
	FInterpLocation InterpLoc6{InterpComp6, 0};
	InterpLocations.Add(InterpLoc6);
	
	
}

void AShooterCharacter::ResetPickupSoundTimer()
{
	bShouldPlayPickupSound = true;
}

void AShooterCharacter::ResetEquipSoundTimer()
{
	bShouldPlayEquipSound = true;
}

void AShooterCharacter::FKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 0) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 0);
}

void AShooterCharacter::OneKeyPressed()
{
	RagdollStart();
	if (EquippedWeapon->GetSlotIndex() == 1) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 1);
}

void AShooterCharacter::TwoKeyPressed()
{
	RagdollEnd();
	if (EquippedWeapon->GetSlotIndex() == 2) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 2);
}

void AShooterCharacter::ThreeKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 3) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 3);
}

void AShooterCharacter::FourKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 4) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 4);
}

void AShooterCharacter::FiveKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 5) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 5);
}

void AShooterCharacter::ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
{
	const bool bCanExchangeItems = (CurrentItemIndex != NewItemIndex) && (NewItemIndex < Inventory.Num()) &&
		(CombatState == ECombatState::ECS_Unoccupied || CombatState == ECombatState::ECS_Equipping);
	
	if (bCanExchangeItems)
	{
		if (bAiming)
		{
			StopAiming();
		}
		
		auto OldEquippedWeapon = EquippedWeapon;
		auto NewWeapon = Cast<AWeapon>(Inventory[NewItemIndex]);
		EquipWeapon(NewWeapon);

		OldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
		NewWeapon->SetItemState(EItemState::EIS_Equipped);
	
		CombatState = ECombatState::ECS_Equipping;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && EquipMontage)
		{
			AnimInstance->Montage_Play(EquipMontage, 1.f);
			AnimInstance->Montage_JumpToSection(FName("Equip"));
		}
		NewWeapon->PlayEquipSound(true);
	}
}

int32 AShooterCharacter::GetEmptyInventorySlot()
{
	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		if (!Inventory[i])
		{
			return i;
		}
	}
	if (Inventory.Num() < INVENTORY_CAPACITY)
	{
		return Inventory.Num();
	}
	return -1; // Inventory is full
}

void AShooterCharacter::HighlightInventorySlot()
{
	const int32 EmptySlot{GetEmptyInventorySlot()};
	HighlightIconDelegate.Broadcast(EmptySlot, true);
	HighlightedSlot = EmptySlot;
}

EPhysicalSurface AShooterCharacter::GetSurfaceType()
{
	FHitResult HitResult;
	const FVector Start{GetActorLocation()};
	const FVector End{Start + FVector(0.f, 0.f, -400.f)};
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;

	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, QueryParams);
	
	return UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
}

void AShooterCharacter::EndStun()
{
	CombatState = ECombatState::ECS_Unoccupied;
	if (bAimingButtonPressed)
	{
		Aim();
	}
}

void AShooterCharacter::Die()
{
	bDead = true;
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}
}

void AShooterCharacter::FinishDeath()
{
	GetMesh()->bPauseAnims = true;
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		DisableInput(PC);
	}
}

void AShooterCharacter::UnHighlightInventorySlot()
{
	HighlightIconDelegate.Broadcast(HighlightedSlot, false);
	HighlightedSlot = -1;
}

void AShooterCharacter::Stun()
{
	if	(Health <= 0.f) return;
	
	CombatState = ECombatState::ECS_Stunned;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
	}
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Handle interpolation for zoom when aiming 
	CameraInterpZoom(DeltaTime);

	// Change sensitivity based on aiming
	SetLookRates();

	// Calculate crosshair spread multiplier
	CalculateCrosshairSpread(DeltaTime);

	// Check OverlappedItemCount, trace for items
	TraceForItems();

	// Interpolate capsule half height transitioning between crouching & standing 
	InterpCapsuleHalfHeight(DeltaTime);
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);

	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AShooterCharacter::SelectButtonReleased);

	PlayerInputComponent->BindAction("ReloadButton", IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterCharacter::CrouchButtonPressed);

	PlayerInputComponent->BindAction("FKey", IE_Pressed, this, &AShooterCharacter::FKeyPressed);
	PlayerInputComponent->BindAction("OneKey", IE_Pressed, this, &AShooterCharacter::OneKeyPressed);
	PlayerInputComponent->BindAction("TwoKey", IE_Pressed, this, &AShooterCharacter::TwoKeyPressed);
	PlayerInputComponent->BindAction("ThreeKey", IE_Pressed, this, &AShooterCharacter::ThreeKeyPressed);
	PlayerInputComponent->BindAction("FourKey", IE_Pressed, this, &AShooterCharacter::FourKeyPressed);
	PlayerInputComponent->BindAction("FiveKey", IE_Pressed, this, &AShooterCharacter::FiveKeyPressed);
}

float AShooterCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	if (Health - DamageAmount <= 0.f)
	{
		Health = 0.f;
		Die();

		auto EnemyController = Cast<AEnemyController>(EventInstigator);
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("bCharacterDead"), true);
		}
	}
	else
	{
		Health -= DamageAmount;
	}
	return DamageAmount;
}

void AShooterCharacter::RagdollStart()
{
	// Step 1: Clear the Character Movement Mode and set the Movement State to Ragdoll
	GetCharacterMovement()->SetMovementMode(MOVE_None);
	//SetMovementState(EALSMovementState::Ragdoll);

	// Step 2: Disable capsule collision and enable mesh physics simulation starting from the pelvis.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionObjectType(ECC_PhysicsBody);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetAllBodiesBelowSimulatePhysics(TEXT("pelvis"), true, true);

	// Step 3: Stop any active montages.
	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Stop(0.2f);
	}
}

void AShooterCharacter::RagdollEnd()
{
	/*
	// Step 1: Save a snapshot of the current Ragdoll Pose for use in AnimGraph to blend out of the ragdoll
	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->SavePoseSnapshot(NAME_RagdollPose);
	}
	

	// Step 2: If the ragdoll is on the ground, set the movement mode to walking and play a Get Up animation.
	// If not, set the movement mode to falling and update the character movement velocity to match the last ragdoll velocity.
	if (bRagdollOnGround)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		if (MainAnimInstance)
		{
			MainAnimInstance->Montage_Play(GetGetUpAnimation(bRagdollFaceUp), 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
		}
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		GetCharacterMovement()->Velocity = LastRagdollVelocity;
	}
	*/
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	// Step 3: Re-Enable capsule collision, and disable physics simulation on the mesh.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetAllBodiesSimulatePhysics(false);
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}

/* No longer needed. AItem has GetInterpLocation()
FVector AShooterCharacter::GetCameraInterpLocation()
{
	const FVector CameraWorldLocation{FollowCamera->GetComponentLocation()};
	const FVector CameraForward{FollowCamera->GetForwardVector()};

	return CameraWorldLocation + CameraForward * CameraInterpDistance + FVector(0.f, 0.f, CameraInterpElevation);
}
*/

void AShooterCharacter::GetPickupItem(AItem* Item)
{
	Item->PlayEquipSound();
	
	auto Weapon = Cast<AWeapon>(Item);
	if (Weapon)
	{
		if (Inventory.Num() < INVENTORY_CAPACITY)
		{
			Weapon->SetSlotIndex(Inventory.Num());
			Inventory.Add(Weapon);
			Weapon->SetItemState(EItemState::EIS_PickedUp);
		}
		else // Inventory is full, swap with equipped weapon
		{
			SwapWeapon(Weapon);
		}
	}

	auto Ammo = Cast<AAmmo>(Item);
	if (Ammo)
	{
		PickupAmmo(Ammo);
	}
}

FInterpLocation AShooterCharacter::GetInterpLocation(int32 Index)
{
	if (Index <= InterpLocations.Num())
	{
		return InterpLocations[Index];
	}
	return FInterpLocation();
}

int32 AShooterCharacter::GetInterpLocationIndex()
{
	int32 LowestIndex = 1;
	int32 LowestCount = INT_MAX;
	for	(int32 i = 1; i < InterpLocations.Num(); i++)
	{
		if (InterpLocations[i].ItemCount < LowestCount)
		{
			LowestIndex = i;
			LowestCount = InterpLocations[i].ItemCount;
		}
	}
	return LowestIndex;
}

void AShooterCharacter::IncrementInterpLocItemCount(int32 Index, int32 Amount)
{
	if (Amount < -1 || Amount > 1) return;

	if (InterpLocations.Num() >= Index)
	{
		InterpLocations[Index].ItemCount += Amount;
	}
}

void AShooterCharacter::StartPickupSoundTimer()
{
	bShouldPlayPickupSound = false;
	GetWorldTimerManager().SetTimer(PickupSoundTimer, this, &AShooterCharacter::ResetPickupSoundTimer,
		PickupSoundResetTime);
}

void AShooterCharacter::StartEquipSoundTimer()
{
	bShouldPlayEquipSound = false;
	GetWorldTimerManager().SetTimer(EquipSoundTimer, this, &AShooterCharacter::ResetEquipSoundTimer,
		EquipSoundResetTime);
}
