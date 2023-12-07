// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "WeaponType.h"
#include "ShooterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EOffsetState : uint8
{
	EOS_Aiming UMETA(DisplayName = "Aiming"),
	EOS_Hip UMETA(DisplayName = "Hip"),
	EOS_Reloading UMETA(DisplayName = "Reloading"),
	EOS_InAir UMETA(DisplayName = "InAir"),

	EOS_MAX UMETA(DisplayName = "DefaultMAX")

};
/**
 * 
 */
UCLASS()
class THIRDPERSONSHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	UShooterAnimInstance();

	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);

	virtual void NativeInitializeAnimation() override;

protected:
	//Handle turn in place variabels
	void TurnInPlace();

	//Handle leaning while running
	void Lean(float DeltaTime);

private:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class AShooterCharacter* ShooterCharacter;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool IsInAir;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool IsAccelerating;

	//Offset yaw used for strafing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MovementOffsetYaw;
	
	//Offset yaw the frame before we start moving
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float LastMovementOffsetYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool IsAiming;

	//Yaw of the character this frame, use for turn in place
	float TIPCharacterYaw;

	//Yaw of the character the previous frame, use for turn in place
	float TIPCharacterYawLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TurnInPlace", meta = (AllowPrivateAccess = "true"))
	float RootYawOffset;

	//Rotation curve value this frame
	float RotationCurve;
	
	//Rotation curve value last frame
	float RotationCurveLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TurnInPlace", meta = (AllowPrivateAccess = "true"))
	float Pitch;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TurnInPlace", meta = (AllowPrivateAccess = "true"))
	bool IsReloading;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TurnInPlace", meta = (AllowPrivateAccess = "true"))
	EOffsetState OffsetState;

	//Yaw of the character this frame
	FRotator CharacterRotator;

	//Yaw of the character the previous frame
	FRotator CharacterRotatorLastFrame;

	//Yaw delta used for leaning in the running blendspace
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Lean", meta = (AllowPrivateAccess = "true"))
	float YawDelta;

	//True when crounching;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Crounching", meta = (AllowPrivateAccess = "true"))
	bool IsCrounching;
	
	//True when equipping weapon;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Crounching", meta = (AllowPrivateAccess = "true"))
	bool IsEquipping;

	//Weapon type of the current equipped weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Crounching", meta = (AllowPrivateAccess = "true"))
	EWeaponType EquippedWeaponType;

	//True when not reloading/equipping
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Crounching", meta = (AllowPrivateAccess = "true"))
	bool ShouldUseFABRIK;
};
