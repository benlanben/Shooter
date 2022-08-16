// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponType.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EOffsetState : uint8
{
	EOS_Aiming		UMETA(DisplayName = "Aiming"),
	EOS_Hip			UMETA(DisplayName = "Hip"),
	EOS_Reloading	UMETA(DisplayName = "Reloading"),
	EOS_InAir		UMETA(DisplayName = "InAir"),
	
	EOS_MAX			UMETA(DisplayName = "DefaultMAX")
};
/**
 * 
 */
UCLASS()
class SHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UShooterAnimInstance();
	
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime); // This is like tick function in actor classes.
	
	virtual void NativeInitializeAnimation() override; // This is like BeginPlay() in actor classes.

protected:
	// Handle turning in place variables
	void TurnInPlace();

	// Handle calculations for leaning while running
	void Lean(float DeltaTime);
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class AShooterCharacter* ShooterCharacter;

	// Speed of the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	// Whether or not the character is in air
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	// Whether or not the character is moving
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	// Offset yaw used for strafing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MovementOffsetYaw;

	// Offset yaw the frame before we stopped moving
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float LastMovementOffsetYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	// Yaw of the character this frame, only updated when standing still & not in air(TIP: TurnInPlace)
	float TIPCharacterYaw;

	// Yaw of the character the previous frame, only updated when standing still & not in air(TIP: TurnInPlace)
	float TIPCharacterYawLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = TurnInPlace, meta = (AllowPrivateAccess = "true"))
	float RootYawOffset;

	// RotationCurve value this frame
	float RotationCurve;

	// RotationCurve value last frame
	float RotationCurveLastFrame;

	// The pitch for aim rotation used for AimOffset
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = TurnInPlace, meta = (AllowPrivateAccess = "true"))
	float Pitch;

	// Used to prevent AimOffset while reloading
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = TurnInPlace, meta = (AllowPrivateAccess = "true"))
	bool bReloading;

	// OffsetState used to determine which aim offset to use
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = TurnInPlace, meta = (AllowPrivateAccess = "true"))
	EOffsetState OffsetState;

	// Rotation of the character this frame
	FRotator CharacterRotation;
	
	// Rotation of the character the previous frame
	FRotator CharacterRotationLastFrame;

	// YawDelta used for leaning in the RunningBlendspace
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Lean, meta = (AllowPrivateAccess = "true"))
	float YawDelta;

	// True when crouching 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Lean, meta = (AllowPrivateAccess = "true"))
	bool bCrouching;

	// True when equipping
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Lean, meta = (AllowPrivateAccess = "true"))
	bool bEquipping;

	// Change the recoil weight based on turning in place and aiming
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float RecoilWeight;

	// True when turning in place
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bTurningInPlace;

	// WeaponType for the currently equipped weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	EWeaponType EquippedWeaponType;

	// True when not reloading or equipping
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bShouldUseFABRIK;
};
