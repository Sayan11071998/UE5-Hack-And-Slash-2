#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "HackAndSlashPlayerAnimInstance.generated.h"

class UCharacterMovementComponent;
class AHackAndSlashPlayer;

UCLASS()
class HACKANDSLASH_API UHackAndSlashPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	// ~ Begin UAnimInstance interface
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	// ~ End UAnimInstance interface
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AHackAndSlashPlayer> PlayerCharacter;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UCharacterMovementComponent> PlayerCharacterMovement;
	
	// Ground locomotion
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Direction;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsAccelerating;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float StartDirection;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float StopDirection;
	
	// Jump
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsFalling;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsJumping;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsFallingDown; // Negative velocity past apex
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsAtApex;
	
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float JumpStartThreshold;
	
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float ApexThreshold;
	
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float VerticalVelocity;
	
	// Turn in place
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float YawDelta;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bShouldTurnInPlace;
	
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float TurnInPlaceThreshold = 90.f;
	
	// Pivot detection (for 180 degree direction changes)
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float PreviousDirection;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float DirectionChangeRate;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bShouldPivot;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float PivotDirection;
	
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float PivotThreshold = 135.f;
	
	// Slope detection
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float SlopeAngle;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsOnSlope;
	
	// Locomotion state tracking
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsMoving;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bJustStartedMoving;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bJustStoppedMoving;
	
	// Combat state
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsInCombat;
	
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsAttacking;

	// Variables added from Reference Screenshots
	UPROPERTY(BlueprintReadOnly, Category = "Rotation")
	float Roll;

	UPROPERTY(BlueprintReadOnly, Category = "Rotation")
	float Pitch;

	UPROPERTY(BlueprintReadOnly, Category = "Rotation")
	float Yaw;

	UPROPERTY(BlueprintReadOnly, Category = "Rotation")
	FRotator RotationLastTick;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsFullBody;
	
private:
	bool bWasMovingLastFrame;
	bool bWasFallingLastFrame;
	FRotator LastRotation;
};