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
	ClipBoneName(FName("smg_clip"))
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

void AWeapon::StopFalling()
{
	isFalling = false;
	SetItemState(EItemState::EIS_PickUp);
}
