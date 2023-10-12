// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/WidgetComponent.h"
#include "Weapon.h"
#include "Item.h"



// Sets default values
AShooterCharacter::AShooterCharacter() :
	//Base Hates for turning
	BaseTurnRate(45.0f),
	BaseLookUpRate(45.0f),
	//Turn hates for when not aiming
	HipTurnRate(90.0f),
	HipLookUpRate(90.0f),
	MouseHipTurnRate(1.0f),
	MouseHipLookUpRate(1.0f),
	//Turn hates for when aiming
	AimingTurnRate(20.0f),
	AimingLookUpRate(20.0f),
	MouseAimingTurnRate(0.3f),
	MouseAimingLookUpRate(0.3f),
	//True when aiming the weapon
	IsAiming(false),
	//Camera FOV values
	CameraDefaultFOV(0.0f),//set in begin play;
	CameraZoomFOV(45.0f),
	CameraCurrentFOV(0.0f),
	ZoomInterpSpeed(20.0f),
	//CrossHair spread fator
	CrossHairMultiplier(0.0f),
	CrossHairVelocityFactor(0.0f),
	CrossHairInAirFactor(0.0f),
	CrossHairAimFactor(0.0f),
	CrossHairShootingFactor(0.0f),
	//Bullet fire timer variables
	ShootTimeDuration(0.05f),
	isFiringBullet(false),
	//AutomaticFireGunRate
	AutomaticGunFireRate(0.1f),
	ShouldFire(true),
	IsFireButtonPressed(false),
	ShouldTraceForItens(false)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create a Camera Boom (pulls in towards the caracter if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; //Camera follows at this distance behinde the character
	CameraBoom->bUsePawnControlRotation = true; //Rotate the arm based ibn the controller input
	CameraBoom->SocketOffset = FVector(0.0f, 100.0f, 50.f);

	//Create a follow  camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach camera to the end of Boom
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm;

	//Stop character rotate with controller
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	//Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false;//Character moves in the direction of input
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);// at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.3f;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}

	EquipWeapon(SpawnDefaultWeapon());
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CameraInterpZoom(DeltaTime);
	CalculateCrossHairSpread(DeltaTime);
	SetLookRates();
	TraceForItems();

}

void AShooterCharacter::TraceForItems()
{
	if (ShouldTraceForItens)
	{
		FHitResult ItemTraceResult;
		FVector dummy;
		if (TraceUnderCrossHair(ItemTraceResult, dummy))
		{
			AItem* HitItem = Cast<AItem>(ItemTraceResult.GetActor());

			if (HitItem && HitItem->GetPickUpWidget())
			{
				HitItem->GetPickUpWidget()->SetVisibility(true); 
			}
			
			if (TracedHitItemLastFrame)
			{
				if (HitItem != TracedHitItemLastFrame)
				{
					TracedHitItemLastFrame->GetPickUpWidget()->SetVisibility(false);
				}
			}

			TracedHitItemLastFrame = HitItem;
		}
	}
	else if (TracedHitItemLastFrame)
	{
		TracedHitItemLastFrame->GetPickUpWidget()->SetVisibility(false);
	}
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	if (IsAiming)
	{
		if (CameraCurrentFOV != CameraZoomFOV)
		{
			CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomFOV, DeltaTime, ZoomInterpSpeed);
			GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
		}
	}
	else
	{
		if (CameraCurrentFOV != CameraDefaultFOV)
		{
			CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
			GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
		}
	}
}
void AShooterCharacter::SetLookRates()
{
	if (IsAiming && BaseTurnRate != AimingLookUpRate && BaseLookUpRate != AimingLookUpRate)
	{
		BaseTurnRate = AimingLookUpRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else if(BaseTurnRate != HipTurnRate && BaseLookUpRate != HipLookUpRate)
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}
void AShooterCharacter::CalculateCrossHairSpread(float DeltaTime)
{
	//Calculate velocity factor
	FVector2D WalkSpeedRange{ 0.0f, 600.0f };
	FVector2D VelocityMultiplierRange{ 0.0f, 1.0f };
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0;

	CrossHairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	//Calculate in air factor
	if (GetCharacterMovement()->IsFalling())
	{
		CrossHairInAirFactor = FMath::FInterpTo(CrossHairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else
	{
		CrossHairInAirFactor = FMath::FInterpTo(CrossHairInAirFactor, 0.0f, DeltaTime, 30.0f);
	}
	
	//Calculate aiming factor

	if (IsAiming && CrossHairInAirFactor < 1.0f )
	{
		CrossHairAimFactor = FMath::FInterpTo(CrossHairInAirFactor, 1.0f, DeltaTime, 30.0f);
	}
	else if (CrossHairInAirFactor >= 0.0f)
	{
		CrossHairAimFactor = FMath::FInterpTo(CrossHairInAirFactor, 0.0f, DeltaTime, 30.0f);
	}

	//Firing Bullet
	if (isFiringBullet)
	{
		CrossHairShootingFactor = FMath::FInterpTo(CrossHairShootingFactor, 0.3f, DeltaTime, 60.0f);
	}
	else
	{
		CrossHairShootingFactor = FMath::FInterpTo(CrossHairShootingFactor, 0.0f, DeltaTime, 60.0f);
	}

	CrossHairMultiplier = 0.25f + CrossHairVelocityFactor + CrossHairInAirFactor - CrossHairAimFactor + CrossHairShootingFactor;
}

void AShooterCharacter::StartCrossHairBulletFire()
{
	isFiringBullet = true;

	GetWorldTimerManager().SetTimer(CrossHairShootTimer, this, &AShooterCharacter::FinishCrossHairBulletFire, ShootTimeDuration);
}

void AShooterCharacter::FinishCrossHairBulletFire()
{
	isFiringBullet = false;
}

void AShooterCharacter::StartFireTimer()
{
	if (ShouldFire)
	{
		FireWeapon();
		ShouldFire = false;
		GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::FinishFireTimer, AutomaticGunFireRate);
	}
}

void AShooterCharacter::FinishFireTimer()
{
	ShouldFire = true;
	if (IsFireButtonPressed)
	{
		StartFireTimer();
	}
}

void AShooterCharacter::MoveForward(float Valeu)
{
	if ((Controller != nullptr) && Valeu != 0)
	{
		//find out foward direction
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
		AddMovementInput(Direction, Valeu);
	}
}

void AShooterCharacter::MoveRight(float Valeu)
{
	if ((Controller != nullptr) && Valeu != 0)
	{
		//find out foward direction
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
		AddMovementInput(Direction, Valeu);
	}
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::Turn(float value)
{
	float TurnScaleFactor = (IsAiming) ? MouseAimingTurnRate : MouseHipTurnRate;
	AddControllerYawInput(TurnScaleFactor * value);
}

void AShooterCharacter::LookUp(float valeu)
{
	float TurnScaleFactor = (IsAiming) ? MouseAimingLookUpRate : MouseHipLookUpRate;
	AddControllerPitchInput(TurnScaleFactor * valeu);
}

void AShooterCharacter::FireWeapon()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}

	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector BeanEndPoint;

		if (GetBeanEndLocation(SocketTransform.GetLocation(), BeanEndPoint))
		{
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeanEndPoint);
			}

			if (BeanParticles)
			{
				UParticleSystemComponent* Bean = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeanParticles, SocketTransform);

				if (Bean)
					Bean->SetVectorParameter(FName("Target"), BeanEndPoint);
			}
		}
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}

	StartCrossHairBulletFire();
}

bool AShooterCharacter::GetBeanEndLocation(const FVector& MuzzleScketLocation, FVector& OutBeanLocation)
{
	FHitResult FCrossHairHitResult;

	if (TraceUnderCrossHair(FCrossHairHitResult, OutBeanLocation))
	{
		OutBeanLocation = FCrossHairHitResult.Location;
	}
	else // no trace hit
	{

	}

	//Perform trace hit from the gun barrel
	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart{ MuzzleScketLocation };
	const FVector StartToEnd{ OutBeanLocation - MuzzleScketLocation };
	const FVector WeaponTraceEnd{ MuzzleScketLocation + StartToEnd * 1.25f };

	GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

	if (WeaponTraceHit.bBlockingHit)
	{
		OutBeanLocation = WeaponTraceHit.Location;
		return true;
	}
	/*else if (FCrossHairHitResult.bBlockingHit)
	{
		return true;
	}*/
	return false;

	//FVector2D ViewportSize;

	//if (GEngine && GEngine->GameViewport)
	//{
	//	GEngine->GameViewport->GetViewportSize(ViewportSize);
	//}

	//FVector2D CrossHairLocation(ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f);
	////CrossHairLocation.Y -= 70.f; // This Ugly god

	//FVector CrossHairWorldPosition;
	//FVector CrossHairWorldDirection;

	//bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrossHairLocation, CrossHairWorldPosition, CrossHairWorldDirection);

	//if (bScreenToWorld)
	//{
	//	//Perform hit from CrossHair
	//	FHitResult ScreenTraceHit;
	//	const FVector Start{ CrossHairWorldPosition };
	//	const FVector End{ CrossHairWorldPosition + CrossHairWorldDirection * 50'000.0f };

	//	OutBeanLocation = End;
	//	GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);

	//	if (ScreenTraceHit.bBlockingHit)
	//	{
	//		OutBeanLocation = ScreenTraceHit.Location;
	//	}

	//	//Perform trace hit from the gun barrel
	//	FHitResult WeaponTraceHit;
	//	const FVector WeaponTraceStart{ MuzzleScketLocation };
	//	const FVector WeaponTraceEnd{ OutBeanLocation };

	//	GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

	//	if (WeaponTraceHit.bBlockingHit)
	//	{
	//		OutBeanLocation = WeaponTraceHit.Location;
	//	}

	//	
	//	return true;
	//}
	//return false;
}

bool AShooterCharacter::TraceUnderCrossHair(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	FVector2D ViewportSize;

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrossHairLocation(ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f);

	FVector CrossHairWorldPosition;
	FVector CrossHairWorldDirection;

	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrossHairLocation, CrossHairWorldPosition, CrossHairWorldDirection);
	
	if (bScreenToWorld)
	{
		const FVector Start{ CrossHairWorldPosition };
		const FVector End{ CrossHairWorldPosition + CrossHairWorldDirection * 50'000.0f };
		OutHitLocation = End;


		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}
	return false;
}

void AShooterCharacter::AimingButtonPressed()
{
	IsAiming = true;
}

void AShooterCharacter::AimingButtonRelease()
{
	IsAiming = false;
}

void AShooterCharacter::FireButtonPressed()
{
	IsFireButtonPressed = true;
	StartFireTimer();
}

void AShooterCharacter::FireButtonReleased()
{
	IsFireButtonPressed = false;
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	if (DefaultWeaponClass)
	{
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	}

	return nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* Weapon)
{
	if (Weapon)
	{
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("Right_Hand_Socket"));

		if (HandSocket)
		{
			HandSocket->AttachActor(Weapon, GetMesh());
		}

		EquippedWeapon = Weapon;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	
	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);
	
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonRelease);
}

void AShooterCharacter::InvrementOverlappedItemCount(int8 Ammount)
{
	if (OverlappedItemCount + Ammount <= 0)
	{
		OverlappedItemCount = 0;
		ShouldTraceForItens = false;
	}
	else
	{
		OverlappedItemCount += Ammount;
		ShouldTraceForItens = true;
	}
}

