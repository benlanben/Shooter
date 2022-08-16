// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Ammo.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AAmmo : public AItem
{
	GENERATED_BODY()
public:
	AAmmo();

protected:
	
	virtual void BeginPlay() override;

	// Override of the function so we can set AmmoMesh(StaticMeshComponent) properties(ItemMesh is SkeletalMeshComponent)
	virtual void SetItemProperties(EItemState State) override;

	// Called when overlapping AmmoCollisionSphere
	UFUNCTION()
	void AmmoSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	
	virtual void Tick(float DeltaTime) override;

	virtual void EnableCustomDepth() override;
	virtual void DisableCustomDepth() override;

private:
	// Mesh for the ammo pickup
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* AmmoMesh;

	// Ammo type for the ammo
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

	// The texture for the ammo icon
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	UTexture2D* AmmoIconTexture;

	// Overlap sphere for picking up ammo
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AmmoCollisionSphere;

public:
	FORCEINLINE UStaticMeshComponent* GetAmmoMesh() const { return AmmoMesh; }

	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }
};
