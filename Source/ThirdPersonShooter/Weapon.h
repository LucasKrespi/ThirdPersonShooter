// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Engine/DataTable.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_SMG UMETA(DislayName = "SMG"),
	EWT_AR UMETA(DislayName = "AR"),

	EWT_MAX UMETA(DislayName = "DefaultMAX")
};

USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAmmoType AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WeaponAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MagazineCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USoundCue* PickUpSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* EquipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* IventoryIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* AmmoIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstance* MaterialInstace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaterialIndex;
};

UCLASS()
class THIRDPERSONSHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()
	
public:

	AWeapon();

	virtual void Tick(float DeltaTime) override;

protected:

	void StopFalling();

	virtual void OnConstruction(const FTransform& Transform) override;

private:

	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool isFalling;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Porperties", meta = (AllowPrivateAccess = "true"))
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Porperties", meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Porperties", meta = (AllowPrivateAccess = "true"))
	int32 Ammo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Porperties", meta = (AllowPrivateAccess = "true"))
	int32 MagazineCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Porperties", meta = (AllowPrivateAccess = "true"))
	FName ReloadMontageSelection;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Porperties", meta = (AllowPrivateAccess = "true"))
	bool bMovingClip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Porperties", meta = (AllowPrivateAccess = "true"))
	FName ClipBoneName;

	//Data table to set weapon propriets
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	UDataTable* WeaponDataTable;

	//TODO: Add to data table
	//Amount of damage caused by one bullet
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Porperties", meta = (AllowPrivateAccess = "true"))
	float Damage;
	//Amount of damage caused by one bullet in the head
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Porperties", meta = (AllowPrivateAccess = "true"))
	float HeadShootDamage;

	int32 PreviusMaterialIndex;
public:

	void ThrowWeapon();

	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	
	FORCEINLINE EAmmoType GetWeaponAmmoType() const { return AmmoType; }

	FORCEINLINE FName GetReloadMontageName() const { return ReloadMontageSelection; }

	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	
	FORCEINLINE int32 GetMagazineCapacity() const { return MagazineCapacity; }

	FORCEINLINE FName GetClipBoneName() const { return ClipBoneName; }
	
	FORCEINLINE float GetDamage() const { return Damage; }

	FORCEINLINE float GetHeadShootDamage() const { return HeadShootDamage; }

	void ReloadAmmo(int32 Ammount);
	//Called from character class when firin;
	void DecrementAmmo();
	
	bool ClipIsFull();

	FORCEINLINE void SetMovingClip(bool move) { bMovingClip = move; }
};
