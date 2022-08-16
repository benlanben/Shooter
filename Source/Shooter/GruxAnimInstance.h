// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GruxAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API UGruxAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);
	
private:
	// Lateral movement speed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	float Speed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	class AEnemy* Enemy;
	
};
