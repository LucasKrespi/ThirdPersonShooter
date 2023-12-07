// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

AWeapon::AWeapon() :
	ThrowWeaponTime(0.7f),
	isFalling(false),
	Ammo(32),
	MagazineCapacity(32),
	WeaponType(EWeaponType::EWT_SMG),
	AmmoType(EAmmoType::EAT_9mm),
	ReloadMontageSelection(FName("Reload_SMG")),
	ClipBoneName(FName("smg_clip")),
	AttachBoneName(FName("Right_Hand_Socket"))
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetItemState() == EItemState::EIS_Falling && isFalling)
	{
		const FRotator MeshRotation{ 0.0f, GetMesh()->GetComponentRotation().Yaw, 0.0f };
		GetMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AWeapon::ThrowWeapon()
{
	FRotator MeshRotation{ 0.0f, GetMesh()->GetComponentRotation().Yaw, 0.0f };
	GetMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

	const FVector MeshForward{ GetMesh()->GetForwardVector() };
	const FVector MeshRight{ GetMesh()->GetRightVector() };
	
	//Direction in which the weapon ios thrown
	FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.0f, MeshForward);

	float RandonRotation{ FMath::FRandRange(-90.f, 90.f) };
	ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandonRotation, FVector(0.0f, 0.0f, 1.0f));
	ImpulseDirection *= 2'000.0f;

	GetMesh()->AddImpulse(ImpulseDirection);

	isFalling = true;
	GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime);

	EnableGlowMaterial();
}

void AWeapon::ReloadAmmo(int32 Ammount)
{
	checkf(Ammo + Ammount <= MagazineCapacity, TEXT("Attempted to reload with more them magazine capacity"));
	Ammo += Ammount;
}

void AWeapon::DecrementAmmo()
{
	if (Ammo - 1 <= 0)
	{
		Ammo = 0;
	}
	else
	{
		--Ammo;
	}
}

bool AWeapon::ClipIsFull()
{
	return Ammo >= MagazineCapacity;
}

void AWeapon::StopFalling()
{
	isFalling = false;
	SetItemState(EItemState::EIS_PickUp);
	StartPulseTimer();
}

void AWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const FString WaponTablePath{ TEXT("/Script/Engine.DataTable'/Game/DataTables/WeaponData.WeaponData'") };

	UDataTable* WeaponTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WaponTablePath));

	if (WeaponTableObject)
	{
		FWeaponDataTable* WeaponDataRow = nullptr;

		switch (WeaponType)
		{
		case EWeaponType::EWT_SMG:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("SubmachineGun"), TEXT(""));
			break;
		case EWeaponType::EWT_AR:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("AssaultRifle"), TEXT(""));
			break;
		case EWeaponType::EWT_MAX:
			break;
		default:
			break;
		}

		if (WeaponDataRow)
		{
			AmmoType = WeaponDataRow->AmmoType;
			Ammo = WeaponDataRow->WeaponAmmo;
			MagazineCapacity = WeaponDataRow->MagazineCapacity;
			SetPickUpSound(WeaponDataRow->EquipSound);
			SetEquipedSound(WeaponDataRow->EquipSound);
			GetMesh()->SetSkeletalMesh(WeaponDataRow->ItemMesh);
			SetItemName(WeaponDataRow->ItemName);
			SetIconItem(WeaponDataRow->IventoryIcon);
			SetIconAmmo(WeaponDataRow->AmmoIcon);
			SetMaterialInstace(WeaponDataRow->MaterialInstace);
			PreviusMaterialIndex = GetMaterialIndex();
			GetMesh()->SetMaterial(PreviusMaterialIndex, nullptr);
			SetMaterialIndex(WeaponDataRow->MaterialIndex);
		}

		if (GetMaterialInstance())
		{
			SetDynamicMaterialInstace(UMaterialInstanceDynamic::Create(GetMaterialInstance(), this));
			GetDynamicMaterialInstance()->SetVectorParameterValue(TEXT("FresnelColor"), GetGlowColor());
			GetMesh()->SetMaterial(GetMaterialIndex(), GetDynamicMaterialInstance());
			EnableGlowMaterial();
		}
	}

}
