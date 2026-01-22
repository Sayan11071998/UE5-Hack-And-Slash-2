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
	
private:
	bool bWasMovingLastFrame;
};