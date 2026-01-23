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
	
	// Ground locomotion init
	bWasMovingLastFrame = false;
	StartDirection = 0.f;
	StopDirection = 0.f;
	
	// Jump init
	bWasFallingLastFrame = false;
	JumpStartThreshold = 100.f;
	ApexThreshold = 50.f;
	VerticalVelocity = 0.f;
	bIsFalling = false;
	bIsJumping = false;
	bIsFallingDown = false;
	bIsAtApex = false;
}

void UHackAndSlashPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (PlayerCharacterMovement && PlayerCharacter)
	{
		// ---- GROUND LOCOMOTION LOGIC ---- //
		
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
		
		// ---- JUMP LOGIC ---- //
		
		// Get vertical velocity
		VerticalVelocity = PlayerCharacterMovement->Velocity.Z;
		
		// Check if character is falling
		bIsFalling = PlayerCharacterMovement->IsFalling();
		
		// Determine jump phase based on vertical velocity
		if (bIsFalling)
		{
			// Rising up (Jump Start phase)
			bIsJumping = VerticalVelocity > JumpStartThreshold;
			
			// At apex (near zero velocity)
			bIsAtApex = FMath::Abs(VerticalVelocity) <= ApexThreshold;
			
			// Falling down (past apex)
			bIsFallingDown = VerticalVelocity < -ApexThreshold;
		}
		else
		{
			// On ground - reset all jump flags
			bIsJumping = false;
			bIsAtApex = false;
			bIsFallingDown = false;
		}
		
		// Store previous falling frame
		bWasFallingLastFrame = bIsFalling;
	}
}