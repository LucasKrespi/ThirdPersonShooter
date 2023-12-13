#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_SMG UMETA(DislayName = "SMG"),
	EWT_AR UMETA(DislayName = "AR"),
	EWT_Pistol UMETA(DislayName = "Pistol"),

	EWT_MAX UMETA(DislayName = "DefaultMAX")
};