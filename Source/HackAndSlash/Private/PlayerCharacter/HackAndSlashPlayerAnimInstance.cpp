#include "PlayerCharacter/HackAndSlashPlayerAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "KismetAnimationLibrary.h"
#include "PlayerCharacter/HackAndSlashPlayer.h"

void UHackAndSlashPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	PlayerCharacter = Cast<AHackAndSlashPlayer>(TryGetPawnOwner());
	if (PlayerCharacter)
	{
		PlayerCharacterMovement = PlayerCharacter->GetCharacterMovement();
	}
	
	bWasMovingLastFrame = false;
	StartDirection = 0.f;
	StopDirection = 0.f;
}

void UHackAndSlashPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (PlayerCharacterMovement && PlayerCharacter)
	{
		// Calculate ground speed
		GroundSpeed = UKismetMathLibrary::VSizeXY(PlayerCharacterMovement->Velocity);
		
		// Checking for acceleration
		bIsAccelerating = PlayerCharacterMovement->GetCurrentAcceleration().SizeSquared() > 0.f;
		
		// Calculate current direction
		FVector Velocity = PlayerCharacterMovement->Velocity;
		if (!Velocity.IsNearlyZero())
		{
			Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, PlayerCharacter->GetActorRotation());
		}
		
		// Detect start of movement
		bool bIsMovingNow = GroundSpeed > 10.f;
		if (bIsMovingNow && !bWasMovingLastFrame)
		{
			StartDirection = Direction;
		}
		
		// Detect end of movement
		if (!bIsMovingNow && bWasMovingLastFrame)
		{
			StopDirection = Direction;
		}
		
		bWasMovingLastFrame = bIsMovingNow;
	}
}