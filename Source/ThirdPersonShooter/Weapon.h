// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_SMG UMETA(DislayName = "SMG"),
	EWT_AR UMETA(DislayName = "AR"),

	EWT_MAX UMETA(DislayName = "DefaultMAX")
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

	//TODO: Add to data table
	//Amount of damage caused by one bullet
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Porperties", meta = (AllowPrivateAccess = "true"))
	float Damage;
	//Amount of damage caused by one bullet in the head
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Porperties", meta = (AllowPrivateAccess = "true"))
	float HeadShootDamage;
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
