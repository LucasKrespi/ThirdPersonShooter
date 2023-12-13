// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AmmoType.h"
#include "ShooterCharacter.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unocupied UMETA(DisplayName = "Unocupied"),
	ECS_FireTimerInProgress UMETA(DisplayName = "FireTimerInProgress"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_Equipping UMETA(DisplayName = "Equipping"),

	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};

USTRUCT(BlueprintType)
struct FInterpLocation
{
	GENERATED_BODY()

	//Scene component to use for interping to this location
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneComponent;

	//Ammount of itens interping to this location
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ItemCount;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquipItemDelegate, int32, CurrentSlotIndex, int32, NewSlotIndex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHighLightIconDelegate, int32, SlotIndex, bool, StartAnimation);


UCLASS()
class THIRDPERSONSHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Valeu);
	void MoveRight(float Valeu);

	/**
	* Called via input to turn at as given rate
	* @paran Rate this is the normalized rate, i. e. 1.0 means 100% of the disered turn rate
	*/
	void TurnAtRate(float Rate);

	/**
	* Called via input to up/down at as given rate
	* @paran Rate this is the normalized rate, i. e. 1.0 means 100% of the disered turn rate
	*/
	void LookUpAtRate(float Rate);

	/**
	* Rotate controller based on mouse X movement
	* @param valeu is the input valeu from mouse movement
	*/
	void Turn(float value);

	/**
	* Rotate controller based on mouse Y movement
	* @param valeu is the input valeu from mouse movement
	*/
	void LookUp(float valeu);

	/**
	* Called via input to Fire the weapon
	*/
	void FireWeapon();

	void PlayHipFireMontage();
	void SendBullet();
	void PlayFireSound();

	bool GetBeanEndLocation(const FVector& MuzzleScketLocation, FHitResult& OutHitResult);

	bool TraceUnderCrossHair(FHitResult& OutHitResult, FVector& OutHitLocation);
	/**
	* Set isAiming to true or false;
	*/
	void AimingButtonPressed();
	void AimingButtonRelease();

	void FireButtonPressed();
	void FireButtonReleased();

	void SelectButtonPressed();
	void SelectButtonReleased();

	void ReloadButtonPressed();
	
	class AWeapon* SpawnDefaultWeapon();
	void EquipWeapon(AWeapon* WeaponToEquip, bool IsSwapping = false);

	void DropWeapon();

	void SwapWeapon(AWeapon* WeaponToSawp);

	void InitializeAmmoMap();

	bool WeaponHasAmmo();

	void ReloadWeapon();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();	
	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

	/** Checks to see if we got ammo for the corrent weapon */
	bool CarryingAmmo();

	/** Called from Anim Blueprint with Grab clip notify */
	UFUNCTION(BlueprintCallable)
	void GrabClip();

	/** Called from Anim Blueprint with Grab clip notify */
	UFUNCTION(BlueprintCallable)
	void ReleaseClip();

	void CrouchButtonPressed();

	void InitializeInterpLocations();

	void ResetPickUpSoundTimer();
	void ResetEquipSoundTimer();

	int32 GetEmptyInventorySlot();
	void HighLightInventorySlot();
	

private:

	void CameraInterpZoom(float DeltaTime);

	/** Set base look up and base turn hates on aiming*/
	void SetLookRates();

	void CalculateCrossHairSpread(float DeltaTime);

	void StartCrossHairBulletFire();

	void TraceForItems();
	
	//Needs to a be a Ufunction to used as callback for FTimerHandle
	UFUNCTION()
	void FinishCrossHairBulletFire();

	void StartFireTimer();
	
	UFUNCTION()
	void FinishFireTimer();

	void PickUpAmmo(class AAmmo* Ammo);

	void FKeyPressed();
	void OneKeyPressed();
	void TwoKeyPressed();
	void ThreeKeyPressed();
	void FourKeyPressed();
	void FiveKeyPressed();

	void Aim();
	void StopAiming();

	void ExchangeInventoryItens(int32 CurrentItenIndex, int32 NewItenIndex);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	//Camera Boom Positioning the Camera behind the Character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	//Camera that follows the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	//Base turn rate in deg/sec. Other scale may affect final turn rate
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate;

	//Base look up rate in deg/sec. Other scale may affect final turn rate
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	//Turn rate while not aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipTurnRate;	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipTurnRate;


	//look up rate while not aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipLookUpRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipLookUpRate;

	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingTurnRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingTurnRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingLookUpRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingLookUpRate;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool IsAiming;

	bool IsAimingButtonPressed;

	//Camera default field of view valeu

	float CameraDefaultFOV;


	//Camera field of view valeu when aiming

	float CameraZoomFOV;

	//Camera field of view this frame
	float CameraCurrentFOV;
	
	//Enterpolation speed for zooming when aiming
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;


	//=========Particles==============
	
	//Particle Spawned on bullet collision
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* ImpactParticles;

	//Smoke trail for bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeanParticles;

	//Montage for firing the weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* HipFireMontage;

	//=========CrossHair===========
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CrossHair, meta = (AllowPrivateAccess = "true"))
	float CrossHairMultiplier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CrossHair, meta = (AllowPrivateAccess = "true"))
	float CrossHairVelocityFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CrossHair, meta = (AllowPrivateAccess = "true"))
	float CrossHairInAirFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CrossHair, meta = (AllowPrivateAccess = "true"))
	float CrossHairAimFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CrossHair, meta = (AllowPrivateAccess = "true"))
	float CrossHairShootingFactor;

	float ShootTimeDuration;

	bool isFiringBullet;

	FTimerHandle CrossHairShootTimer;

	//=======Automatic Fire=========

	bool IsFireButtonPressed;

	//True when fire is allowd 
	bool ShouldFire;

	FTimerHandle AutoFireTimer;

	//======Item Detection===========

	bool ShouldTraceForItens;

	int8 OverlappedItemCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "True"))
	class AItem* TracedHitItemLastFrame;

	//The item being hit by trace func, can be null
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "True"))
	AItem* TraceHitItem;

	//=========Weapon==========
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "True"))
	AWeapon* EquippedWeapon;

	//Set this on blueprints for starter default weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "True"))
	TSubclassOf<AWeapon> DefaultWeaponClass;

	//Distance from the Camera for item interp destinatiom
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "True"))
	float CameraInterpDistance;	
	
	//Elevation from the Camera for item interp destinatiom
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "True"))
	float CameraInterpElevation;

	//========= Ammo =========
	//map to keep track of all ammo types;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "True"))
	TMap<EAmmoType, int32> AmmoMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
	int32 Starting9mmAmmo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
	int32 StartingARAmmo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "True"))
	ECombatState CombatState;

	//========= Reload ========
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* EquippingMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FTransform ClipTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USceneComponent* HandSceneComponent;

	//===========Crouching==========
	//true when crounching
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool IsCrouching;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WeaponInterpComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpCompOne;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpCompTwo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpCompTree;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpCompFour;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpCompFive;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpCompSix;

	//array of interp location structs
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FInterpLocation> InterpLocations;

	FTimerHandle PickUpSoundTimer;

	FTimerHandle EquippedSoundTimer;

	bool ShouldPlayPickUpSound;
	bool ShouldPlayEquipSound;

	//Time to wait before playing another pickup sound
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float PickUpSoundResetTime;
	//Time to wait before playing another equipped sound
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float EquipSoundResetTime;

	//================Inventory================

	//Array with AItens
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	TArray<AItem*> Inventory;

	const int32 INVENTORY_CAPACITY{ 6 };

	//Delegate for sending slot information to inventory bar when equipping
	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
	FEquipItemDelegate EquipItemDelegate;

	//Delegate for sending slot information for playing the icon Animation
	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
	FHighLightIconDelegate HighLightIconDelegate;

	//The index of the current highlighted slot
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	int32 HighLightedSlot;

public:

	//Return Camera Boom subObject
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	
	//Return Camera Folloew subobject
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE bool GetIsAiming() const { return IsAiming; }

	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }

	FORCEINLINE ECombatState GetCombateState() const { return CombatState; }
	
	FORCEINLINE AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

	FORCEINLINE bool GetIsCrounching() const { return IsCrouching; }

	FORCEINLINE bool GetShouldPlayPickUpSound() const { return ShouldPlayPickUpSound; }
	FORCEINLINE bool GetShouldPlayEquipSound() const { return ShouldPlayEquipSound; }

	void StartPickUpSoundTimer();
	void StartEquipSoundTimer();

	void InvrementOverlappedItemCount(int8 Ammount);
	
	//Its not inline because is blueprint callable
	UFUNCTION(BlueprintCallable)
	float const GetCrossHairMultiplier() const { return CrossHairMultiplier; }

	//No longer needed item has get interp location
	//FVector GetCameraInterpLocation();

	void GetPickUpItem(AItem* Item);

	//Return an FinterpLocation based on the index passed in
	FInterpLocation GetInterpLocationByIndex(int32 index);

	//Finds the interp index
	int32 GetInterpLocationIndex();

	void IncrementIterpLocItemCount(int32 Index, int32 Ammount);

	void UnhighLightInventorySlot();


};
