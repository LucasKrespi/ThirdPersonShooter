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
	TIPCharacterYaw(0.0f),
	TIPCharacterYawLastFrame(0.0f),
	RootYawOffset(0.0f),
	Pitch(0.0f),
	IsReloading(false),
	OffsetState(EOffsetState::EOS_Hip),
	CharacterRotator(FRotator(0.0f)),
	CharacterRotatorLastFrame(FRotator(0.0f)),
	YawDelta(0.0f)
{

}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if(ShooterCharacter == nullptr)
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());

	if (ShooterCharacter)
	{
		IsCrounching = ShooterCharacter->GetIsCrounching();
		IsReloading = ShooterCharacter->GetCombateState() == ECombatState::ECS_Reloading;
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

		if (IsReloading)
		{
			OffsetState = EOffsetState::EOS_Reloading;
		}
		else if (IsInAir)
		{
			OffsetState = EOffsetState::EOS_InAir;
		}
		else if (ShooterCharacter->GetIsAiming())
		{
			OffsetState = EOffsetState::EOS_Aiming;
		}
		else
		{
			OffsetState = EOffsetState::EOS_Hip;
		}
	}

	TurnInPlace();
	Lean(DeltaTime);
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::TurnInPlace()
{
	if (ShooterCharacter == nullptr) return;

	Pitch = ShooterCharacter->GetBaseAimRotation().Pitch;

	if (Speed > 0 || IsInAir)
	{
		//Don`t want to turn in place, character is moving
		RootYawOffset = 0;
		TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		TIPCharacterYawLastFrame = TIPCharacterYaw;

		RotationCurveLastFrame = 0.0f;
		RotationCurve = 0.0f;
	}
	else
	{
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;

		const float TIPYawDelta{ TIPCharacterYaw - TIPCharacterYawLastFrame };

		//Root yaw offset updated and clamped to [-180, 180] degrees
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - TIPYawDelta);

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

void UShooterAnimInstance::Lean(float DeltaTime)
{
	if (ShooterCharacter == nullptr) return;

	CharacterRotatorLastFrame = CharacterRotator;
	CharacterRotator = ShooterCharacter->GetActorRotation();

	const FRotator Delta{UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotator, CharacterRotatorLastFrame)};

	const float Target{ (float)Delta.Yaw / DeltaTime };

	const float Interp{ FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.0f) };

	YawDelta = FMath::Clamp(Interp, -90, 90);
}
