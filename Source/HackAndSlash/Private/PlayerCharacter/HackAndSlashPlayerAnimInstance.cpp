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
	
	// Turn in place init
	YawDelta = 0.f;
	bShouldTurnInPlace = false;
	LastRotation = FRotator::ZeroRotator;
	
	// Pivot detection init
	PreviousDirection = 0.f;
	bShouldPivot = false;
	PivotDirection = 0.f;
	PivotThreshold = 135.f;
	
	// Slope detection init
	SlopeAngle = 0.f;
	bIsOnSlope = false;
	
	// Locomotion state init
	bIsMoving = false;
	bJustStartedMoving = false;
	bJustStoppedMoving = false;
	
	// Combat state init
	bIsInCombat = false;
	bIsAttacking = false;
	bIsFullBody = false;
	
	// Rotation init
	Roll = 0.f;
	Pitch = 0.f;
	Yaw = 0.f;
	RotationLastTick = FRotator::ZeroRotator;
}

void UHackAndSlashPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (PlayerCharacterMovement && PlayerCharacter)
	{
		UpdateGroundLocomotion(DeltaSeconds);
		UpdateJumpLogic();
		UpdateTurnInPlace();
		UpdateRotation(DeltaSeconds);
		
		// Get action state from player
		ActionState = PlayerCharacter->GetActionState();

		// Check curve for FullBody state
		bIsFullBody = GetCurveValue(FName(TEXT("FullBody"))) > 0.f;
	}
}

void UHackAndSlashPlayerAnimInstance::UpdateGroundLocomotion(float DeltaSeconds)
{
	GroundSpeed = UKismetMathLibrary::VSizeXY(PlayerCharacterMovement->Velocity);
	bIsAccelerating = PlayerCharacterMovement->GetCurrentAcceleration().SizeSquared() > 0.f;
	
	FVector Velocity = PlayerCharacterMovement->Velocity;
	if (!Velocity.IsNearlyZero())
	{
		Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, PlayerCharacter->GetActorRotation());
	}
	
	// Track movement state
	bIsMoving = GroundSpeed > 10.f;
	bJustStartedMoving = bIsMoving && !bWasMovingLastFrame;
	bJustStoppedMoving = !bIsMoving && bWasMovingLastFrame;
	
	if (bJustStartedMoving)
	{
		StartDirection = Direction;
	}
	
	if (bJustStoppedMoving)
	{
		StopDirection = Direction;
	}
	
	// Pivot logic
	float DirectionDelta = FMath::FindDeltaAngleDegrees(PreviousDirection, Direction);
	
	if (bIsMoving && GroundSpeed > 100.f && FMath::Abs(DirectionDelta) > PivotThreshold)
	{
		bShouldPivot = true;
		PivotDirection = Direction;
	}
	else if (!bIsMoving || GroundSpeed < 50.f)
	{
		bShouldPivot = false;
	}
	
	if (bIsMoving)
	{
		PreviousDirection = Direction;
	}
	
	bWasMovingLastFrame = bIsMoving;
	
	// Slope detection
	if (!bIsFalling)
	{
		FHitResult HitResult;
		FVector Start = PlayerCharacter->GetActorLocation();
		FVector End = Start - FVector(0.f, 0.f, 200.f);
		
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(PlayerCharacter);
		
		if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
		{
			FVector FloorNormal = HitResult.ImpactNormal;
			SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(FloorNormal, FVector::UpVector)));
			bIsOnSlope = SlopeAngle > 5.f;
		}
		else
		{
			SlopeAngle = 0.f;
			bIsOnSlope = false;
		}
	}
	else
	{
		SlopeAngle = 0.f;
		bIsOnSlope = false;
	}
}

void UHackAndSlashPlayerAnimInstance::UpdateJumpLogic()
{
	VerticalVelocity = PlayerCharacterMovement->Velocity.Z;
	bIsFalling = PlayerCharacterMovement->IsFalling();
	
	if (bIsFalling)
	{
		bIsJumping = VerticalVelocity > JumpStartThreshold;
		bIsAtApex = FMath::Abs(VerticalVelocity) <= ApexThreshold;
		bIsFallingDown = VerticalVelocity < -ApexThreshold;
	}
	else
	{
		bIsJumping = false;
		bIsAtApex = false;
		bIsFallingDown = false;
	}
	
	bWasFallingLastFrame = bIsFalling;
}

void UHackAndSlashPlayerAnimInstance::UpdateTurnInPlace()
{
	if (!bIsFalling && GroundSpeed < 10.f)
	{
		FRotator CurrentRotation = PlayerCharacter->GetActorRotation();
		YawDelta = UKismetMathLibrary::NormalizedDeltaRotator(CurrentRotation, LastRotation).Yaw;
		LastRotation = CurrentRotation;
		
		bShouldTurnInPlace = FMath::Abs(YawDelta) > TurnInPlaceThreshold;
	}
	else
	{
		bShouldTurnInPlace = false;
		LastRotation = PlayerCharacter->GetActorRotation();
	}
}

void UHackAndSlashPlayerAnimInstance::UpdateRotation(float DeltaSeconds)
{
	FRotator ActorRotation = PlayerCharacter->GetActorRotation();
	FRotator BaseAimRotation = PlayerCharacter->GetBaseAimRotation();
	
	// Aim Offsets
	FRotator DeltaAim = UKismetMathLibrary::NormalizedDeltaRotator(BaseAimRotation, ActorRotation);
	Roll = DeltaAim.Roll;
	Pitch = DeltaAim.Pitch;
	Yaw = DeltaAim.Yaw;
	
	// Leaning (Smoothed)
	FRotator RotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(ActorRotation, RotationLastTick);
	float TargetLean = RotationDelta.Yaw / 7.0f;
	
	if (DeltaSeconds > 0.f)
	{
		YawDelta = FMath::FInterpTo(YawDelta, TargetLean, DeltaSeconds, 6.0f);
	}
	
	RotationLastTick = ActorRotation;
}