// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

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

	bool GetBeanEndLocation(const FVector& MuzzleScketLocation, FVector& OutBeanLocation);

	/**
	* Set isAiming to true or false;
	*/
	void AimingButtonPressed();
	void AimingButtonRelease();

private:

	void CameraInterpZoom(float DeltaTime);

	/** Set base look up and base turn hates on aiming*/
	void SetLookHates();

	void CalculateCrossHairSpread(float DeltaTime);

	void StartCrossHairBulletFire();
	
	//Needs to a be a Ufunction to used as callback for FTimerHandle
	UFUNCTION()
	void FinishCrossHairBulletFire();

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
	
	//Randomized gunshot sound cue
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;

	//Flash Spawned when weapon is fired
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;
	
	//Particle Spawned on bullet collision
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;

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


public:

	//Return Camera Boom subObject
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	
	//Return Camera Folloew subobject
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE bool GetIsAiming() const { return IsAiming; }

	//Its not inline because is blueprint callable
	UFUNCTION(BlueprintCallable)
	float const GetCrossHairMultiplier() const { return CrossHairMultiplier; }

};
