// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Engine/DataTable.h"
#include "WeaponType.h"
#include "Weapon.generated.h"


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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AttachBoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ClipBoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ReloadMontageSelection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> AnimBp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrossHairsMiddle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrossHairsTop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrossHairsBotton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrossHairsRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrossHairsLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoFireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BoneToHide;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsAutomaticFire;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeadShootDamage;
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

	virtual void BeginPlay() override;

	void FinishMovingSlide();

	void UpdateSlideDisplacement();

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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Porperties", meta = (AllowPrivateAccess = "true"))
	FName AttachBoneName;

	//Data table to set weapon propriets
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	UDataTable* WeaponDataTable;

	//Textures ofr the weapon CrossHair
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrossHairsMiddle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrossHairsTop;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrossHairsBotton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrossHairsRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrossHairsLeft;

	//The speed auto fire happens
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	float AutoFireRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	USoundCue* FireSound;

	//Amount of damage caused by one bullet
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Porperties", meta = (AllowPrivateAccess = "true"))
	float Damage;
	//Amount of damage caused by one bullet in the head
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Porperties", meta = (AllowPrivateAccess = "true"))
	float HeadShootDamage;

	//Name of the bone to hide on the weapon mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Porperties", meta = (AllowPrivateAccess = "true"))
	FName BoneToHide;

	//Ammount that the slide is push back on pistol fire
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	float SlideDisplacement;

	//Curve for the pistol slide displacement
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	UCurveFloat* SlideDisplacementCurve;

	//Timer handle for pistol slide on shoot
	FTimerHandle SlideTimer;

	float SliderDisplacementTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	bool IsMovingSlider;

	//Max distance for the slide on the pistol
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	float MaxSlideDisplacement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	float RecoilRotation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	float MaxRecoilRotation;

	//True for automatic gun fire
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Porperties", meta = (AllowPrivateAccess = "true"))
	bool IsAutomaticFire;

	int32 PreviusMaterialIndex;

public:

	void ThrowWeapon();

	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	
	FORCEINLINE EAmmoType GetWeaponAmmoType() const { return AmmoType; }

	FORCEINLINE FName GetReloadMontageName() const { return ReloadMontageSelection; }

	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	
	FORCEINLINE int32 GetMagazineCapacity() const { return MagazineCapacity; }

	FORCEINLINE FName GetClipBoneName() const { return ClipBoneName; }

	FORCEINLINE FName GetAttachBoneName() const { return AttachBoneName; }
	
	FORCEINLINE float GetDamage() const { return Damage; }

	FORCEINLINE float GetHeadShootDamage() const { return HeadShootDamage; }

	FORCEINLINE float GetAutoFireRate() const { return AutoFireRate; }

	FORCEINLINE bool GetAutomaticFire() const { return IsAutomaticFire; }
	
	FORCEINLINE UParticleSystem* GetMuzzleFlash() const { return MuzzleFlash; }

	FORCEINLINE USoundCue* GetFireSound() const { return FireSound; }



	void ReloadAmmo(int32 Ammount);
	//Called from character class when firing;
	void DecrementAmmo();
	
	bool ClipIsFull();

	void StartSlideTimer();

	FORCEINLINE void SetMovingClip(bool Move) { bMovingClip = Move; }

	FORCEINLINE void SetAttachBoneName(FName Name) { AttachBoneName = Name; }

	FORCEINLINE void SetClipBoneName(FName Name) { ClipBoneName = Name; }

	FORCEINLINE void SetReloadMontageSelection(FName Name) { ReloadMontageSelection = Name; }


};
