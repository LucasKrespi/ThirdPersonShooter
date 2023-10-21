// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UShooterAnimInstance::UShooterAnimInstance() :
	Speed(0.0f),
	IsInAir(false),
	IsAccelerating(false),
	MovementOffsetYaw(0.0f),
	LastMovementOffsetYaw(0.0f),
	IsAiming(false),
	CharacterYaw(0.0f),
	CharacterYawLastFrame(0.0f),
	RootYawOffset(0.0f)
{

}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if(ShooterCharacter == nullptr)
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());

	if (ShooterCharacter)
	{
		//Get the lateral speed of the character from velocity
		FVector Velocity{ ShooterCharacter->GetVelocity() };
		Velocity.Z = 0;
		Speed = Velocity.Size();

		//is the character in the air
		IsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();

		//is the character accelerating
		if (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0)
			IsAccelerating = true;
		else
			IsAccelerating = false;
		
		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
		
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
		
		if(ShooterCharacter->GetVelocity().Size() > 0.0f)
			LastMovementOffsetYaw = MovementOffsetYaw;

		IsAiming = ShooterCharacter->GetIsAiming();
	}

	TurnInPlace();
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::TurnInPlace()
{
	if (ShooterCharacter == nullptr) return;

	if (Speed > 0)
	{
		//Don`t want to turn in place, character is moving
		RootYawOffset = 0;
		CharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		CharacterYawLastFrame = CharacterYaw;

		RotationCurveLastFrame = 0.0f;
		RotationCurve = 0.0f;
	}
	else
	{
		CharacterYawLastFrame = CharacterYaw;
		CharacterYaw = ShooterCharacter->GetActorRotation().Yaw;

		const float YawDelta{ CharacterYaw - CharacterYawLastFrame };

		//Root yaw offset updated and clamped to [-180, 180] degrees
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - YawDelta);

		//1.0 if turning 0.0 if not
		const float Turning{ GetCurveValue(TEXT("Turning")) };
		
		if (Turning > 0)
		{
			RotationCurveLastFrame = RotationCurve;
			RotationCurve = GetCurveValue(TEXT("Rotation"));

			const float DeltaRotation{ RotationCurve - RotationCurveLastFrame };

			//Root Yaw offset > 0, turning left otherwise right
			RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;

			const float AbsRootYawOffset{ FMath::Abs(RootYawOffset) };

			if (AbsRootYawOffset > 90.0f)
			{
				const float YawExcess{ AbsRootYawOffset - 90.0f };
				RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}
	}

}
