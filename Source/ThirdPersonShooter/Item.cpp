// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "Curves/CurveFloat.h"
#include "Camera/CameraComponent.h"
#include "ShooterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Curves/CurveVector.h"

// Sets default values
AItem::AItem():
	ItemName(FString("Default")),
	ItemCount(0),
	ItemRarity(EItemRarity::EIR_Common),
	ItemState(EItemState::EIS_PickUp),
	//Item interp Vectors
	ZCurveTime(0.7f),
	ItemInterpStartLocation(FVector(0.0f)),
	CameraTargetLocation(FVector(0.0f)),
	isInterping(false),
	ItemInterpX(0.0f),
	ItemInterpY(0.0f),
	InterpInitialYawOffset(0.0f),
	ItemType(EItemType::EIT_Max),
	InterpLocIndex(0),
	MateralIndex(0),
	CanChangeCustonDepth(true),
	//Dynamic material params
	GlowAmmount(150.0f),
	FresnelExponent(3.0f),
	FresnelReflectFraction(4.0f),
	PulseCurveTime(5.0f),
	SlotIndex(0),
	IsInventoryFull(false)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(ItemMesh);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(GetRootComponent());

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	
	//Hide Widget on start
	if(PickupWidget)
		PickupWidget->SetVisibility(false);

	SetActiveStars();

	//Set up overlap for area sphere
	if (AreaSphere)
	{
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);
	}

	SetItemProperties(ItemState);

	InitializeCustonDepth();

	StartPulseTimer();
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ItemInterp(DeltaTime);

	UpdatePulse();
}


void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);

		if (ShooterCharacter)
			ShooterCharacter->InvrementOverlappedItemCount(1);
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);

		if (ShooterCharacter)
		{
			ShooterCharacter->InvrementOverlappedItemCount(-1);
			ShooterCharacter->UnhighLightInventorySlot();
		}
	}
}

void AItem::SetActiveStars()
{
	for (int32 i = 0; i <= 5; i++)
	{
		ActiveStars.Add(false);
	}

	switch (ItemRarity)
	{
	case EItemRarity::EIR_Damaged:
		ActiveStars[1] = true;
		break;
	case EItemRarity::EIR_Common:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		break;
	case EItemRarity::EIR_Uncommon:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		break;
	case EItemRarity::EIR_Rare:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		ActiveStars[4] = true;
		break;
	case EItemRarity::EIR_Legendary:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		ActiveStars[4] = true;
		ActiveStars[5] = true;
		break;
	case EItemRarity::EIR_MAX:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		ActiveStars[4] = true;
		ActiveStars[5] = true;
		break;
	default:
		break;
	}
}

void AItem::SetItemProperties(EItemState State)
{
	switch (State)
	{
	case EItemState::EIS_PickUp:

		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ItemMesh->SetEnableGravity(false);

		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		break;
	case EItemState::EIS_EquipInterp:
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ItemMesh->SetEnableGravity(false);

		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		PickupWidget->SetVisibility(false);
		break;
	case EItemState::EIS_PickedUp:
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetVisibility(false);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ItemMesh->SetEnableGravity(false);

		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		PickupWidget->SetVisibility(false);
		break;
	case EItemState::EIS_Equipped:

		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ItemMesh->SetEnableGravity(false);

		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		PickupWidget->SetVisibility(false);
		break;
	case EItemState::EIS_Falling:
		ItemMesh->SetSimulatePhysics(true);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
		ItemMesh->SetEnableGravity(true);
		ItemMesh->SetVisibility(true);

		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_MAX:
		break;
	default:
		break;
	}
}

void AItem::FinishInterping()
{
	isInterping = false;
	if (Character)
	{
		Character->IncrementIterpLocItemCount(InterpLocIndex, -1);
		Character->GetPickUpItem(this);
		Character->UnhighLightInventorySlot();
	}

	SetActorScale3D(FVector(1.0f));
	DisableGlowMaterial();
	
	CanChangeCustonDepth = true;
	DisableCustomDepth();
}

void AItem::ItemInterp(float DeltaTime)
{
	if (!isInterping) return;

	if (Character && ItemZCurve)
	{
		const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimer);
		const float CurveValeu = ItemZCurve->GetFloatValue(ElapsedTime);

		FVector ItemLocation = ItemInterpStartLocation;
		const FVector CameraInterpLocation = GetInterpLocation();
		const FVector ItemToCamera{ FVector(0.0f,0.0f, (CameraInterpLocation - ItemLocation).Z) };
		
		//Scale to multiply with curve value;
		const float DeltaZ = ItemToCamera.Size();

		ItemLocation.Z = CurveValeu * DeltaZ;

		//X and Y Location
		const FVector CurrentLocation{ GetActorLocation() };
		const float InpterpXValue = FMath::FInterpTo(CurrentLocation.X, CameraInterpLocation.X, DeltaTime, 30.0f);
		const float InpterpYValue = FMath::FInterpTo(CurrentLocation.Y, CameraInterpLocation.Y, DeltaTime, 30.0f);

		ItemLocation.X = InpterpXValue;
		ItemLocation.Y = InpterpYValue;

		SetActorLocation(ItemLocation, true, nullptr, ETeleportType::TeleportPhysics);

		//Rotation
		const FRotator CameraRotation{ Character->GetFollowCamera()->GetComponentRotation() };
		FRotator ItemRotation{ 0.0f, CameraRotation.Yaw + InterpInitialYawOffset, 0.0f };

		SetActorRotation(ItemRotation, ETeleportType::TeleportPhysics);

		//Scale
		if (ItemScaleCurve)
		{
			const float ScaleCurveValeu = ItemScaleCurve->GetFloatValue(ElapsedTime);
			SetActorScale3D(FVector(ScaleCurveValeu, ScaleCurveValeu, ScaleCurveValeu));
		}
	}

}

FVector AItem::GetInterpLocation()
{
	if (Character == nullptr) return FVector(0.f);

	switch (ItemType)
	{
	case EItemType::EIT_Ammo:
		return Character->GetInterpLocationByIndex(InterpLocIndex).SceneComponent->GetComponentLocation();
		break;
	case EItemType::EIT_Weapon:
		return Character->GetInterpLocationByIndex(0).SceneComponent->GetComponentLocation();
		break;
	case EItemType::EIT_Max:
		return FVector(0.f);
		break;
	default:
		return FVector(0.f);
		break;
	}
	return FVector();
}

void AItem::PlayPickUpSound(bool ForcePlaySound)
{
	if (Character)
	{
		if (ForcePlaySound)
		{
			if (PickUpSound)
			{
				UGameplayStatics::PlaySound2D(this, PickUpSound);
			}
		}
		else if (Character->GetShouldPlayPickUpSound())
		{
			Character->StartPickUpSoundTimer();
			if (PickUpSound)
			{
				UGameplayStatics::PlaySound2D(this, PickUpSound);
			}
		}
	}
}

void AItem::EnableCustomDepth()
{
	if(CanChangeCustonDepth)
		ItemMesh->SetRenderCustomDepth(true);
}

void AItem::DisableCustomDepth()
{
	if (CanChangeCustonDepth)
		ItemMesh->SetRenderCustomDepth(false);
}

void AItem::InitializeCustonDepth()
{
	DisableCustomDepth();
}

void AItem::EnableGlowMaterial()
{
	if (DynamicaMaterialInstance)
	{
		DynamicaMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendAlpha"), 0);
	}
}

void AItem::DisableGlowMaterial()
{
	if (DynamicaMaterialInstance)
	{
		DynamicaMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendAlpha"), 1);
	}
}

void AItem::OnConstruction(const FTransform& Transform)
{
	//Load the date in the item rarity data table

	//Path to Item rarity data table
	FString RarityTablePath(TEXT("/Script/Engine.DataTable'/Game/DataTables/ItemRarityTable.ItemRarityTable'"));

	UDataTable* RarityTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *RarityTablePath));

	if (RarityTableObject)
	{
		FItemRarityTable* RarityRow = nullptr;
		switch (ItemRarity)
		{
		case EItemRarity::EIR_Damaged:
			RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Damaged"), TEXT(""));
			break;
		case EItemRarity::EIR_Common:
			RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Common"), TEXT(""));
			break;
		case EItemRarity::EIR_Uncommon:
			RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Uncommon"), TEXT(""));
			break;
		case EItemRarity::EIR_Rare:
			RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Rare"), TEXT(""));
			break;
		case EItemRarity::EIR_Legendary:
			RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Legendary"), TEXT(""));
			break;
		case EItemRarity::EIR_MAX:
			break;
		default:
			break;
		}

		if (RarityRow)
		{
			GlowColor = RarityRow->GlowColor;
			LightColor = RarityRow->LightColor;
			DarkColor = RarityRow->DarkColor;
			NumberOfStars = RarityRow->NumberOfStars;
			IconBackground = RarityRow->IconBackground;

			if (GetMesh())
			{
				GetMesh()->SetCustomDepthStencilValue(RarityRow->CustomDepthStencil);
			}
		}
	}

	if (MaterialInstace)
	{
		DynamicaMaterialInstance = UMaterialInstanceDynamic::Create(MaterialInstace, this);
		DynamicaMaterialInstance->SetVectorParameterValue(TEXT("FresnelColor"), GlowColor);
		ItemMesh->SetMaterial(MateralIndex, DynamicaMaterialInstance);
		EnableGlowMaterial();
	}
}

void AItem::UpdatePulse()
{
	float ElapsedTime{};
	FVector CurveValeu{};

	switch (ItemState)
	{
	case EItemState::EIS_PickUp:
		if (PulseCurve)
		{
			ElapsedTime = GetWorldTimerManager().GetTimerElapsed(PulseTimer);
			CurveValeu = PulseCurve->GetVectorValue(ElapsedTime);
		}
		break;
	case EItemState::EIS_EquipInterp:
		if (InterpPulseCurve)
		{
			ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimer);
			CurveValeu = InterpPulseCurve->GetVectorValue(ElapsedTime);
		}
		break;
	case EItemState::EIS_PickedUp:
		break;
	case EItemState::EIS_Equipped:
		break;
	case EItemState::EIS_Falling:
		break;
	case EItemState::EIS_MAX:
		break;
	default:
		break;
	}

	if (DynamicaMaterialInstance)
	{
		DynamicaMaterialInstance->SetScalarParameterValue(TEXT("GlowAmmount"), CurveValeu.X * GlowAmmount);
		DynamicaMaterialInstance->SetScalarParameterValue(TEXT("FresnelExponent"), CurveValeu.Y * FresnelExponent);
		DynamicaMaterialInstance->SetScalarParameterValue(TEXT("FresnelReflectFraction"), CurveValeu.Z * FresnelReflectFraction);
	}
}

void AItem::PlayEquipSound(bool ForcePlaySound)
{
	if (Character)
	{
		if (ForcePlaySound)
		{
			if (EquipedSound)
			{
				UGameplayStatics::PlaySound2D(this, EquipedSound);
			}
		}
		else if (Character->GetShouldPlayEquipSound())
		{
			Character->StartEquipSoundTimer();
			if (EquipedSound)
			{
				UGameplayStatics::PlaySound2D(this, EquipedSound);
			}
		}
	}
}


void AItem::ResetPulseTimer()
{
	StartPulseTimer();
}

void AItem::StartPulseTimer()
{
	if (ItemState == EItemState::EIS_PickUp)
	{
		GetWorldTimerManager().SetTimer(PulseTimer, this, &AItem::ResetPulseTimer, PulseCurveTime);
	}
}

void AItem::SetItemState(EItemState State)
{
	ItemState = State;

	SetItemProperties(ItemState);
}

void AItem::StartItemCurv(AShooterCharacter* Char, bool ForcePlaySound)
{
	Character = Char;

	InterpLocIndex = Character->GetInterpLocationIndex();
	Character->IncrementIterpLocItemCount(InterpLocIndex, 1);
	
	PlayPickUpSound(ForcePlaySound);

	ItemInterpStartLocation = GetActorLocation();
	isInterping = true;
	SetItemState(EItemState::EIS_EquipInterp);

	GetWorldTimerManager().ClearTimer(PulseTimer);
	GetWorldTimerManager().SetTimer(ItemInterpTimer, this, &AItem::FinishInterping, ZCurveTime);

	const float CameraRotationYaw{ static_cast<float>(Character->GetFollowCamera()->GetComponentRotation().Yaw) };
	const float ItemRotationYaw{ static_cast<float>(GetActorRotation().Yaw) };

	InterpInitialYawOffset = ItemRotationYaw - CameraRotationYaw;

	CanChangeCustonDepth = false;
}

