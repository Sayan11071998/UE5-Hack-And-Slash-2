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
	DirectionChangeRate = 0.f;
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
		// ---- GROUND LOCOMOTION LOGIC ---- //
		
		GroundSpeed = UKismetMathLibrary::VSizeXY(PlayerCharacterMovement->Velocity);
		bIsAccelerating = PlayerCharacterMovement->GetCurrentAcceleration().SizeSquared() > 0.f;
		
		// Calculate current direction
		FVector Velocity = PlayerCharacterMovement->Velocity;
		if (!Velocity.IsNearlyZero())
		{
			Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, PlayerCharacter->GetActorRotation());
		}
		
		// ---- ENHANCED LOCOMOTION STATE TRACKING ---- //
		
		// Track movement state changes
		bIsMoving = GroundSpeed > 10.f;
		bJustStartedMoving = bIsMoving && !bWasMovingLastFrame;
		bJustStoppedMoving = !bIsMoving && bWasMovingLastFrame;
		
		// Detect start of movement
		if (bJustStartedMoving)
		{
			StartDirection = Direction;
		}
		
		// Detect end of movement
		if (bJustStoppedMoving)
		{
			StopDirection = Direction;
		}
		
		// ---- PIVOT DETECTION ---- //
		
		// Calculate direction change rate for leaning and pivot detection
		float DirectionDelta = FMath::FindDeltaAngleDegrees(PreviousDirection, Direction);
		
		// Smooth the direction change rate for use in lean animations
		if (DeltaSeconds > 0.f)
		{
			DirectionChangeRate = FMath::FInterpTo(DirectionChangeRate, DirectionDelta / DeltaSeconds, DeltaSeconds, 5.f);
		}
		
		// Detect pivot (large direction change while moving at speed)
		if (bIsMoving && GroundSpeed > 100.f && FMath::Abs(DirectionDelta) > PivotThreshold)
		{
			bShouldPivot = true;
			PivotDirection = Direction;
		}
		else
		{
			// Clear pivot flag once we've transitioned or slowed down
			if (!bIsMoving || GroundSpeed < 50.f)
			{
				bShouldPivot = false;
			}
		}
		
		// Store previous direction for next frame comparison
		if (bIsMoving)
		{
			PreviousDirection = Direction;
		}
		
		bWasMovingLastFrame = bIsMoving;
		
		// ---- SLOPE DETECTION ---- //
		
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
		
		// ---- JUMP LOGIC ---- //
		
		VerticalVelocity = PlayerCharacterMovement->Velocity.Z;
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
		
		// ---- TURN IN PLACE LOGIC ---- //
		
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
		
		// ---- ROTATION & LEAN LOGIC (Matches Reference Screenshots) ---- //
		
		FRotator ActorRotation = PlayerCharacter->GetActorRotation();
		FRotator BaseAimRotation = PlayerCharacter->GetBaseAimRotation();
		
		// 1. Calculate Aim Offset Rotation (BaseAim - ActorRot)
		FRotator DeltaAim = UKismetMathLibrary::NormalizedDeltaRotator(BaseAimRotation, ActorRotation);
		Roll = DeltaAim.Roll;
		Pitch = DeltaAim.Pitch;
		Yaw = DeltaAim.Yaw;
		
		// 2. Calculate Smoothed Lean (Yaw Delta / 7.0 interpolated)
		FRotator RotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(ActorRotation, RotationLastTick);
		float TargetLean = RotationDelta.Yaw / 7.0f; // Scaling factor from screenshot
		
		// We re-use 'YawDelta' for the lean here as it matches the screenshot's 'YawDelta' variable name context
		// Note from screenshot: 'Setting Yaw Delta for Leans' -> sets 'Yaw Delta 0'
		if (DeltaSeconds > 0.f)
		{
			YawDelta = FMath::FInterpTo(YawDelta, TargetLean, DeltaSeconds, 6.0f);
		}
		
		RotationLastTick = ActorRotation;
		
		// ---- FULL BODY CURVE CHECK (Matches Screenshot) ---- //
		// Check for "FullBody" curve to determine if we should be in full body mode (e.g. during specific montages)
		float FullBodyCurveValue = GetCurveValue(FName("FullBody"));
		bIsFullBody = FullBodyCurveValue > 0.f;
	}
}