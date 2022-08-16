// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	EIR_Damaged		UMETA(DisplayName = "Damaged"),
	EIR_Common		UMETA(DisplayName = "Common"),
	EIR_Uncommon	UMETA(DisplayName = "Uncommon"),
	EIR_Rare		UMETA(DisplayName = "Rare"),
	EIR_Legendary	UMETA(DisplayName = "Legendary"),

	EIR_MAX			UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EItemState : uint8
{
	EIS_Pickup			UMETA(DisplayName = "Pickup"),
	EIS_EquipInterping	UMETA(DisplayName = "EquipInterping"),
	EIS_PickedUp		UMETA(DisplayName = "PickedUp"),
	EIS_Equipped		UMETA(DisplayName = "Equipped"),
	EIS_Falling			UMETA(DisplayName = "Falling"),

	EIS_MAX				UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EItemType : uint8
{
	EIT_Ammo	UMETA(DisplayName = "Ammo"),
	EIT_Weapon	UMETA(DisplayName = "Weapon"),

	EIT_MAX		UMETA(DisplayName = "DefaultMAX")
};

USTRUCT(BlueprintType)
struct FItemRarityTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GlowColor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor LightColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DarkColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumberOfStars;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* IconBackground;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CustomDepthStencil;
};
UCLASS()
class SHOOTER_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when overlapping AreaSphere
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Called when overlapping with AreaSphere ended
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
	
	// Sets the ActiveStars array of bools based on item rarity
	void SetActiveStars();

	// Sets properties of the item's components based on state
	virtual void SetItemProperties(EItemState State);

	// Called when ItemInterpTimer is finished
	void FinishInterping();

	// Handles item interpolation when in the EquipInterping state
	void ItemInterp(float DeltaTime);

	// Get InterpLocation based on the item type
	FVector GetInterpLocation();

	void PlayPickupSound(bool bForcePlaySound = false);
	
	virtual void InitializeCustomDepth();

	virtual void OnConstruction(const FTransform& Transform) override;

	void EnableGlowMaterial();

	void StartPulseTimer();
	void ResetPulseTimer();
	void UpdatePulse();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called from the AShooterCharacter::GetPickupItem()
	void PlayEquipSound(bool bForcePlaySound = false);

	virtual void EnableCustomDepth();
	virtual void DisableCustomDepth();
	void DisableGlowMaterial();

private:
	// Skeletal mesh for the item
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ItemMesh;
	
	// Line trace collides with collision box to show HUD widgets
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* CollisionBox;

	// Pop-up widget when the player looks at the item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* PickupWidget;

	// Enables item tracing when overlapped
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AreaSphere;

	// The name which appears on the PickupWidget
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	FString ItemName;

	// Item count(ammo etc)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	int32 ItemCount;

	// Determines # of stars in PickupWidget
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	EItemRarity ItemRarity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	TArray<bool> ActiveStars;

	// State of the item
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	EItemState ItemState;

	// The curve asset used for Item's Z curve when interping
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	class UCurveFloat* ItemZCurve;

	// Starting location when interping begins
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	FVector ItemInterpStartLocation;

	// Target interp location in front of the camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	FVector CameraTargetLocation;

	// True when interping
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	bool bInterping;

	// Plays when we start interping
	FTimerHandle ItemInterpTimer;

	// Duration of the curve and timer
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	float ZCurveTime;

	// Pointer to the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	class AShooterCharacter* Character;

	// X and Y for the item while interping in the EquippInterping state
	float ItemInterpX;
	float ItemInterpY;

	// Initial yaw offset between the camera and the interping item
	float InterpInitialYawOffset;

	// The curve used to scale the item when interping
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	UCurveFloat* ItemScaleCurve;

	// Sound played when item is picked up
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	class USoundCue* PickupSound;

	// Sound played when the item is equipped
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	USoundCue* EquipSound;

	// Enum for the type of item this item is
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	EItemType ItemType;

	// Index of the interp location this item is interpolating to
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	int32 InterpLocIndex;

	// Index for the material we'd like to change at runtime
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	int32 MaterialIndex;

	// Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	UMaterialInstanceDynamic* DynamicMaterialInstance;

	// Material instance used with the DynamicMaterialInstance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* MaterialInstance;

	bool bCanChangeCustomDepth;

	// Curve to drive the dynamic material paramaters
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	class UCurveVector* PulseCurve;

	// Curve to drive the dynamic material paramaters
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	UCurveVector* InterpPulseCurve;

	FTimerHandle PulseTimer;

	// Time for the PulseTimer
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	float PulseCurveTime;

	UPROPERTY(VisibleAnywhere, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	float GlowAmount;

	UPROPERTY(VisibleAnywhere, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	float FresnelExponent;

	UPROPERTY(VisibleAnywhere, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	float FresnelReflectFraction;

	// The icon for this item in the inventory
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconItem;

	// The icon for ammo in the inventory
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconAmmo;

	// Slot in the inventory array
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	int32 SlotIndex;

	// True when the character's inventory is full
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	bool bCharacterInventoryFull;

	// Item rarity data table
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	class UDataTable* ItemRarityDateTable;

	// Color in the glow material
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	FLinearColor GlowColor;

	// Light color in the PickupWidget
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	FLinearColor LightColor;

	// Dark color in the PickupWidget
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	FLinearColor DarkColor;

	// Number of stars in the PickupWidget
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	int32 NumberOfStars;

	// Background icon for the inventory
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconBackground;
	
public:

	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }

	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }

	FORCEINLINE UBoxComponent* GetCollisionBox() const { return CollisionBox; }

	FORCEINLINE EItemState GetItemState() const { return ItemState; }

	void SetItemState(EItemState State);

	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return ItemMesh; }

	// Called from the AShooterCharacter class
	void StartItemCurve(AShooterCharacter* Char, bool bForcePlaySound = false);

	FORCEINLINE USoundCue* GetPickupSound() const { return PickupSound; }

	FORCEINLINE void SetPickupSound(USoundCue* Sound) { PickupSound = Sound; }
	
	FORCEINLINE USoundCue* GetEquipSound() const { return EquipSound; }

	FORCEINLINE void SetEquipSound(USoundCue* Sound) { EquipSound = Sound; }

	FORCEINLINE int32 GetItemCount() const { return ItemCount; }

	FORCEINLINE void SetItemType(EItemType Type) { ItemType = Type; }

	FORCEINLINE int32 GetSlotIndex() const { return SlotIndex; }

	FORCEINLINE void SetSlotIndex(int32 Index) { SlotIndex = Index; }

	FORCEINLINE void SetCharacter(AShooterCharacter* Char) { Character = Char; }

	FORCEINLINE void SetCharacterInventoryFull(bool bFull) { bCharacterInventoryFull = bFull; }

	FORCEINLINE void SetItemName(FString Name) { ItemName = Name; }

	FORCEINLINE void SetIconItem(UTexture2D* Icon) { IconItem = Icon; }

	FORCEINLINE void SetIconAmmo(UTexture2D* Icon) { IconAmmo = Icon; }

	FORCEINLINE void SetMaterialInstance(UMaterialInstance* Instance) { MaterialInstance = Instance; }

	FORCEINLINE UMaterialInstance* GetMaterialInstance() const { return MaterialInstance; }

	FORCEINLINE void SetDynamicMaterialInstance(UMaterialInstanceDynamic* Instance) { DynamicMaterialInstance = Instance; }

	FORCEINLINE UMaterialInstanceDynamic* GetDynamicMaterialInstance() const { return DynamicMaterialInstance; }
	
	FORCEINLINE FLinearColor GetGlowColor() const { return GlowColor; }

	FORCEINLINE void SetMaterialIndex(int32 Index) { MaterialIndex = Index; }
	
	FORCEINLINE int32 GetMaterialIndex() const { return MaterialIndex; }
	
};
