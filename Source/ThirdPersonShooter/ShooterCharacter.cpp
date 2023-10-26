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
#include "BulletHitInterface.h"



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
	ShouldTraceForItens(false),
	//Camera Intrp valeus
	CameraInterpDistance(250.0f),
	CameraInterpElevation(65.0f),
	//StartingAmmo
	Starting9mmAmmo(85),
	StartingARAmmo(120),
	CombatState(ECombatState::ECS_Unocupied),
	IsCrouching(false)
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

	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComp"));
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
	InitializeAmmoMap();
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
			TraceHitItem = Cast<AItem>(ItemTraceResult.GetActor());

			if (TraceHitItem && TraceHitItem->GetPickUpWidget())
			{
				TraceHitItem->GetPickUpWidget()->SetVisibility(true);
			}
			
			if (TracedHitItemLastFrame)
			{
				if (TraceHitItem != TracedHitItemLastFrame)
				{
					TracedHitItemLastFrame->GetPickUpWidget()->SetVisibility(false);
				}
			}

			TracedHitItemLastFrame = TraceHitItem;
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
	CombatState = ECombatState::ECS_FireTimerInProgress;
	
	GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::FinishFireTimer, AutomaticGunFireRate);
}

void AShooterCharacter::FinishFireTimer()
{
	CombatState = ECombatState::ECS_Unocupied;

	if (WeaponHasAmmo())
	{
		if (IsFireButtonPressed)
		{
			FireWeapon();
		}
	}
	else
	{
		ReloadWeapon();
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
	if (EquippedWeapon == nullptr) return;

	if (CombatState != ECombatState::ECS_Unocupied) return;

	if (WeaponHasAmmo())
	{
		PlayFireSound();
		SendBullet();
		PlayHipFireMontage();
		EquippedWeapon->DecrementAmmo();

		StartFireTimer();
		StartCrossHairBulletFire();
	}

}

void AShooterCharacter::PlayHipFireMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

void AShooterCharacter::SendBullet()
{
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetMesh()->GetSocketByName("BarrelSocket");

	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetMesh());

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FHitResult BeanHitResult;

		if (GetBeanEndLocation(SocketTransform.GetLocation(), BeanHitResult))
		{
			//does hit actor implement hit bullet interface
			if (BeanHitResult.GetActor())
			{
				IBulletHitInterface* BulletHitInterface = Cast<IBulletHitInterface>(BeanHitResult.GetActor());

				if (BulletHitInterface)
				{
					BulletHitInterface->BulletHit_Implementation(BeanHitResult);
				}
				else //spawn default particles
				{
					if (ImpactParticles)
					{
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeanHitResult.Location);
					}

					if (BeanParticles)
					{
						UParticleSystemComponent* Bean = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeanParticles, SocketTransform);

						if (Bean)
							Bean->SetVectorParameter(FName("Target"), BeanHitResult.Location);
					}
				}
			}
		}
	}
}

void AShooterCharacter::PlayFireSound()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
}

bool AShooterCharacter::GetBeanEndLocation(const FVector& MuzzleScketLocation, FHitResult& OutHitResult)
{
	FHitResult FCrossHairHitResult;
	FVector OutBeanLocation;

	if (TraceUnderCrossHair(FCrossHairHitResult, OutBeanLocation))
	{
		OutBeanLocation = FCrossHairHitResult.Location;
	}
	else // no trace hit
	{

	}

	//Perform trace hit from the gun barrel
	const FVector WeaponTraceStart{ MuzzleScketLocation };
	const FVector StartToEnd{ OutBeanLocation - MuzzleScketLocation };
	const FVector WeaponTraceEnd{ MuzzleScketLocation + StartToEnd * 1.25f };

	GetWorld()->LineTraceSingleByChannel(OutHitResult, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

	if (!OutHitResult.bBlockingHit)
	{
		OutHitResult.Location = OutBeanLocation;
		return false;
	}
	return true;
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
	
	FireWeapon();
}

void AShooterCharacter::FireButtonReleased()
{
	IsFireButtonPressed = false;
}

void AShooterCharacter::SelectButtonPressed()
{
	if (TraceHitItem)
	{
		TraceHitItem->StartItemCurv(this);
		
		if (TraceHitItem->GetPickUpSound())
		{
			UGameplayStatics::PlaySound2D(this, TraceHitItem->GetPickUpSound());
		}
	}
}

void AShooterCharacter::SelectButtonReleased()
{
}

void AShooterCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
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

void AShooterCharacter::DropWeapon()
{
	if (EquippedWeapon)
	{
		FDetachmentTransformRules DetachmentTrasformRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetMesh()->DetachFromComponent(DetachmentTrasformRules);

		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();
	}
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSawp)
{
	DropWeapon();
	EquipWeapon(WeaponToSawp);
	TraceHitItem = nullptr;
	TracedHitItemLastFrame = nullptr;
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

bool AShooterCharacter::WeaponHasAmmo()
{
	if (EquippedWeapon == nullptr) return false;

	return EquippedWeapon->GetAmmo() > 0;
}

void AShooterCharacter::ReloadWeapon()
{
	if (CombatState != ECombatState::ECS_Unocupied) return;
	if(EquippedWeapon == nullptr) return;

	if (CarryingAmmo() && !EquippedWeapon->ClipIsFull())
	{
		//TODO: inplemment weapon type;
		CombatState = ECombatState::ECS_Reloading;

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (ReloadMontage && AnimInstance)
		{
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageName());
		}
	}
}

void AShooterCharacter::FinishReloading()
{
	//TODO: Update Ammo map;
	CombatState = ECombatState::ECS_Unocupied;

	if (EquippedWeapon == nullptr) return;

	const auto AmmoType = EquippedWeapon->GetWeaponAmmoType();

	if (AmmoMap.Contains(AmmoType))
	{
		int32 CarriedAmmo = AmmoMap[AmmoType];

		const int32 MagazineEmptySpace = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo();

		if (MagazineEmptySpace > CarriedAmmo)
		{
			//Reload magazine with all ammo we get
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
		else
		{
			//fill magazine empty space
			EquippedWeapon->ReloadAmmo(MagazineEmptySpace);
			CarriedAmmo -= MagazineEmptySpace;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
	}
}

bool AShooterCharacter::CarryingAmmo()
{
	if (EquippedWeapon == nullptr) return false;
	
	auto AmmoType = EquippedWeapon->GetWeaponAmmoType();

	if (AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}

	return false;
}

void AShooterCharacter::GrabClip()
{
	if (EquippedWeapon == nullptr) return;
	if (HandSceneComponent == nullptr) return;

	int32 ClipBoneIndex{ EquippedWeapon->GetMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName()) };
	ClipTransform = EquippedWeapon->GetMesh()->GetBoneTransform(ClipBoneIndex);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("Hand_L")));
	HandSceneComponent->SetWorldTransform(ClipTransform);

	EquippedWeapon->SetMovingClip(true);
}

void AShooterCharacter::ReleaseClip()
{
	if (EquippedWeapon == nullptr) return;

	EquippedWeapon->SetMovingClip(false);
}

void AShooterCharacter::CrouchButtonPressed()
{
	if (!GetCharacterMovement()->IsFalling())
	{
		IsCrouching = !IsCrouching;
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
	
	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AShooterCharacter::SelectButtonReleased);

	PlayerInputComponent->BindAction("ReloadButton", IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);

	PlayerInputComponent->BindAction("Crounch", IE_Pressed, this, &AShooterCharacter::CrouchButtonPressed);
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

FVector AShooterCharacter::GetCameraInterpLocation()
{
	const FVector CameraWorldLocation{ FollowCamera->GetComponentLocation() };
	const FVector CameraForward{ FollowCamera->GetForwardVector() };


	return CameraWorldLocation + CameraForward * CameraInterpDistance + FVector(0.0f, 0.0f, CameraInterpElevation);
}

void AShooterCharacter::GetPickUpItem(AItem* Item)
{
	if (Item->GetEquipedSound())
	{
		UGameplayStatics::PlaySound2D(this, Item->GetEquipedSound());
	}
	auto Weapon = Cast<AWeapon>(Item);

	if (Weapon)
	{
		SwapWeapon(Weapon);
	}
}


