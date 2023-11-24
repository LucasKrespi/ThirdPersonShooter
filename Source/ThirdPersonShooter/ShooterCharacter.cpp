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
#include "Ammo.h"
#include "Item.h"
#include "BulletHitInterface.h"
#include "Enemy.h"




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
	IsCrouching(false),
	//PickUp sound proprieties
	ShouldPlayPickUpSound(true),
	ShouldPlayEquipSound(true),
	PickUpSoundResetTime(0.2f),
	EquipSoundResetTime(0.2f),
	HighLightedSlot(-1)
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

	WeaponInterpComp = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Interpolation Component"));
	WeaponInterpComp->SetupAttachment(GetFollowCamera());

	InterpCompOne = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component One"));
	InterpCompOne->SetupAttachment(GetFollowCamera());

	InterpCompTwo = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component Two"));
	InterpCompTwo->SetupAttachment(GetFollowCamera());

	InterpCompTree = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component Tree"));
	InterpCompTree->SetupAttachment(GetFollowCamera());

	InterpCompFour = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component Four"));
	InterpCompFour->SetupAttachment(GetFollowCamera());

	InterpCompFive = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component Five"));
	InterpCompFive->SetupAttachment(GetFollowCamera());

	InterpCompSix = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component Six"));
	InterpCompSix->SetupAttachment(GetFollowCamera());
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
	Inventory.Add(EquippedWeapon);
	EquippedWeapon->SetSlotIndex(0);
	EquippedWeapon->DisableCustomDepth();
	EquippedWeapon->DisableGlowMaterial();
	EquippedWeapon->SetCharacter(this);
	InitializeAmmoMap();
	InitializeInterpLocations();
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

			const auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
			
			if (TraceHitWeapon)
			{
				if (HighLightedSlot == -1)
				{
					HighLightInventorySlot();
				}
			}
			else
			{
				if (HighLightedSlot != -1)
					UnhighLightInventorySlot();
			}

			if (TraceHitItem && TraceHitItem->GetItemState() == EItemState::EIS_EquipInterp)
				TraceHitItem = nullptr;


			if (TraceHitItem && TraceHitItem->GetPickUpWidget())
			{
				TraceHitItem->GetPickUpWidget()->SetVisibility(true);
				TraceHitItem->EnableCustomDepth();

				TraceHitItem->SetCharacterInventoryFull((Inventory.Num() >= INVENTORY_CAPACITY));
			}
			
			if (TracedHitItemLastFrame)
			{
				if (TraceHitItem != TracedHitItemLastFrame)
				{
					TracedHitItemLastFrame->GetPickUpWidget()->SetVisibility(false);
					TracedHitItemLastFrame->DisableCustomDepth();
				}
			}

			TracedHitItemLastFrame = TraceHitItem;
		}
	}
	else if (TracedHitItemLastFrame)
	{
		//if(TraceHitItem)
		TracedHitItemLastFrame->DisableCustomDepth();
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

void AShooterCharacter::PickUpAmmo(AAmmo* Ammo)
{
	if (AmmoMap.Find(Ammo->GetAmmoType()))
	{
		//get the ammount of ammo in ammomap for ammo`s type
		int32 AmmoCount{ AmmoMap[Ammo->GetAmmoType()] };
		AmmoCount += Ammo->GetItemCount();
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}

	if (EquippedWeapon->GetWeaponAmmoType() == Ammo->GetAmmoType())
	{
		//check to see if gun is empty
		if (EquippedWeapon->GetAmmo() == 0)
		{
			ReloadWeapon();
		}
	}

	Ammo->Destroy();
}

void AShooterCharacter::FKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 0) return;
	ExchangeInventoryItens(EquippedWeapon->GetSlotIndex(), 0);
}

void AShooterCharacter::OneKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 1) return;
	ExchangeInventoryItens(EquippedWeapon->GetSlotIndex(), 1);
}

void AShooterCharacter::TwoKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 2) return;
	ExchangeInventoryItens(EquippedWeapon->GetSlotIndex(), 2);
}

void AShooterCharacter::ThreeKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 3) return;
	ExchangeInventoryItens(EquippedWeapon->GetSlotIndex(), 3);
}

void AShooterCharacter::FourKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 4) return;
	ExchangeInventoryItens(EquippedWeapon->GetSlotIndex(), 4);
}

void AShooterCharacter::FiveKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 5) return;
	ExchangeInventoryItens(EquippedWeapon->GetSlotIndex(), 5);
}

void AShooterCharacter::ExchangeInventoryItens(int32 CurrentItenIndex, int32 NewItenIndex)
{
	if ((CurrentItenIndex == NewItenIndex) 
	|| (NewItenIndex >= Inventory.Num())) return;

	if ((CombatState == ECombatState::ECS_Unocupied) || (CombatState == ECombatState::ECS_Equipping))
	{
		auto OldEquippedWaepon = EquippedWeapon;
		auto NewWeapon = Cast<AWeapon>(Inventory[NewItenIndex]);

		EquipWeapon(NewWeapon);
	
		OldEquippedWaepon->SetItemState(EItemState::EIS_PickedUp);
		NewWeapon->SetItemState(EItemState::EIS_Equipped);

		CombatState = ECombatState::ECS_Equipping;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance && EquippingMontage)
		{
			AnimInstance->Montage_Play(EquippingMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName("Equip"));
		}

		NewWeapon->PlayEquipSound(true);
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

				AEnemy* HitEnemy = Cast<AEnemy>(BeanHitResult.GetActor());

				if (HitEnemy)
				{
					int32 Damage{ };
					bool headShoot{ false };
					if (BeanHitResult.BoneName.ToString() == HitEnemy->GetHeadBone())
					{
						//Headshoot
						Damage = EquippedWeapon->GetHeadShootDamage();
						headShoot = true;
					}
					else
					{
						//Regular shoot
						Damage = EquippedWeapon->GetDamage();
					}
					
					UGameplayStatics::ApplyDamage(BeanHitResult.GetActor(), Damage, GetController(), this, UDamageType::StaticClass());

					HitEnemy->ShowHitNumer(Damage, BeanHitResult.Location, headShoot);
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
	if (CombatState != ECombatState::ECS_Unocupied) return;
	if (TraceHitItem)
	{
		TraceHitItem->StartItemCurv(this, true);
		TraceHitItem = nullptr;
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

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip, bool IsSwapping)
{
	if (WeaponToEquip)
	{
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("Right_Hand_Socket"));

		if (HandSocket)
		{
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}

		if (EquippedWeapon == nullptr)
		{
			//-1 No Equipped Weapon yet. no need to reverse the icon animation
			EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());
		}
		else if(!IsSwapping)
		{
			EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(), WeaponToEquip->GetSlotIndex());
		}

		EquippedWeapon = WeaponToEquip;
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
	if (Inventory.Num() - 1 >= EquippedWeapon->GetSlotIndex())
	{
		Inventory[EquippedWeapon->GetSlotIndex()] = WeaponToSawp;
		WeaponToSawp->SetSlotIndex(EquippedWeapon->GetSlotIndex());
	}

	DropWeapon();
	EquipWeapon(WeaponToSawp, true);
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

void AShooterCharacter::FinishEquipping()
{
	CombatState = ECombatState::ECS_Unocupied;
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
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("hand_l")));
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

void AShooterCharacter::InitializeInterpLocations()
{
	FInterpLocation WeaponLocation{ WeaponInterpComp, 0 };
	InterpLocations.Add(WeaponLocation);

	FInterpLocation InterpLoc1{ InterpCompOne, 0 };
	InterpLocations.Add(InterpLoc1);

	FInterpLocation InterpLoc2{ InterpCompTwo, 0 };
	InterpLocations.Add(InterpLoc2);

	FInterpLocation InterpLoc3{ InterpCompTree, 0 };
	InterpLocations.Add(InterpLoc3);

	FInterpLocation InterpLoc4{ InterpCompFour, 0 };
	InterpLocations.Add(InterpLoc4);

	FInterpLocation InterpLoc5{ InterpCompFive, 0 };
	InterpLocations.Add(InterpLoc5);

	FInterpLocation InterpLoc6{ InterpCompSix, 0 };
	InterpLocations.Add(InterpLoc6);
}

void AShooterCharacter::ResetPickUpSoundTimer()
{
	ShouldPlayPickUpSound = true;
}

void AShooterCharacter::ResetEquipSoundTimer()
{
	ShouldPlayEquipSound = true;
}

int32 AShooterCharacter::GetEmptyInventorySlot()
{
	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		if (Inventory[i] == nullptr)
		{
			return i;
		}
	}

	if (Inventory.Num() < INVENTORY_CAPACITY)
	{
		return Inventory.Num();
	}
	return -1; //inventory full
}

void AShooterCharacter::HighLightInventorySlot()
{
	const int32 Emptyslot{ GetEmptyInventorySlot() };
	HighLightIconDelegate.Broadcast(Emptyslot, true);
	HighLightedSlot = Emptyslot;
}

void AShooterCharacter::UnhighLightInventorySlot()
{
	HighLightIconDelegate.Broadcast(HighLightedSlot, false);
	HighLightedSlot = -1;
}

int32 AShooterCharacter::GetInterpLocationIndex()
{
	int32 LowestIndex = 1;
	int32 LowestCount = INT_MAX;

	for (int32 i = 1; i < InterpLocations.Num(); i++)
	{
		if (InterpLocations[i].ItemCount < LowestCount)
		{
			LowestIndex = i;
			LowestCount = InterpLocations[i].ItemCount;
		}
	}

	return LowestIndex;
}

void AShooterCharacter::IncrementIterpLocItemCount(int32 Index, int32 Ammount)
{
	if (Ammount < -1 || Ammount > 1) return;

	if (InterpLocations.Num() >= Index)
	{
		InterpLocations[Index].ItemCount += Ammount;
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

	PlayerInputComponent->BindAction("FKey", IE_Pressed, this, &AShooterCharacter::FKeyPressed);
	PlayerInputComponent->BindAction("OneKey", IE_Pressed, this, &AShooterCharacter::OneKeyPressed);
	PlayerInputComponent->BindAction("TwoKey", IE_Pressed, this, &AShooterCharacter::TwoKeyPressed);
	PlayerInputComponent->BindAction("ThreeKey", IE_Pressed, this, &AShooterCharacter::ThreeKeyPressed);
	PlayerInputComponent->BindAction("FourKey", IE_Pressed, this, &AShooterCharacter::FourKeyPressed);
	PlayerInputComponent->BindAction("FiveKey", IE_Pressed, this, &AShooterCharacter::FiveKeyPressed);
}

void AShooterCharacter::StartPickUpSoundTimer()
{
	ShouldPlayPickUpSound = false;

	GetWorldTimerManager().SetTimer(PickUpSoundTimer, this, &AShooterCharacter::ResetPickUpSoundTimer, PickUpSoundResetTime);
}

void AShooterCharacter::StartEquipSoundTimer()
{
	ShouldPlayEquipSound = false;

	GetWorldTimerManager().SetTimer(EquippedSoundTimer, this, &AShooterCharacter::ResetEquipSoundTimer, EquipSoundResetTime);
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

//FVector AShooterCharacter::GetCameraInterpLocation()
//{
//	const FVector CameraWorldLocation{ FollowCamera->GetComponentLocation() };
//	const FVector CameraForward{ FollowCamera->GetForwardVector() };
//
//
//	return CameraWorldLocation + CameraForward * CameraInterpDistance + FVector(0.0f, 0.0f, CameraInterpElevation);
//}

void AShooterCharacter::GetPickUpItem(AItem* Item)
{
	Item->PlayEquipSound();

	auto Weapon = Cast<AWeapon>(Item);
	if (Weapon)
	{
		if (Inventory.Num() < INVENTORY_CAPACITY)
		{
			Weapon->SetSlotIndex(Inventory.Num());
			Inventory.Add(Weapon);
			Weapon->SetItemState(EItemState::EIS_PickedUp);
		}
		else //inventory full chage the equipped weapon
		{
			SwapWeapon(Weapon);
		}
	}

	auto Ammo = Cast<AAmmo>(Item);
	if (Ammo)
	{
		PickUpAmmo(Ammo);
	}
}

FInterpLocation AShooterCharacter::GetInterpLocationByIndex(int32 index)
{
	if (index <= InterpLocations.Num())
	{
		return InterpLocations[index];
	}

	return FInterpLocation();
}


