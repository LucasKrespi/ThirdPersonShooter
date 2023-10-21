// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"

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

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool IsAiming;

	//Yaw of the character this frame
	float CharacterYaw;

	//Yaw of the character the previous frame
	float CharacterYawLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "TurnInPlace", meta = (AllowPrivateAccess = "true"))
	float RootYawOffset;

	//Rotation curve value this frame
	float RotationCurve;
	
	//Rotation curve value last frame
	float RotationCurveLastFrame;

};
