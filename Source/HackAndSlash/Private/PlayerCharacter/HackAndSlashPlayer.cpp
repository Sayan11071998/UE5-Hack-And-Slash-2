#include "PlayerCharacter/HackAndSlashPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Items/Weapon.h"

AHackAndSlashPlayer::AHackAndSlashPlayer() :
	CameraArmLength(430.f)
{
	PrimaryActorTick.bCanEverTick = true;
	
	bUseControllerRotationRoll = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = CameraArmLength;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 10.f;
	CameraBoom->CameraRotationLagSpeed = 12.f;
	CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
	CameraBoom->SetRelativeRotation(FRotator(-15.f, 0.f, 0.f));
	
	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("View Camera"));
	ViewCamera->SetupAttachment(CameraBoom);
	ViewCamera->bUsePawnControlRotation = false;
	ViewCamera->SetRelativeLocation(FVector(0.f, 40.f, 0.f));
	
	ActionState = EActionState::EAS_Unoccupied;
	
	ComboCounter = 0;
	bAttackInputQueued = false;
	bIsComboWindowOpen = false;
	MaxComboCount = 5;
}

void AHackAndSlashPlayer::BeginPlay()
{
	Super::BeginPlay();
	
	// Adding input mapping context
	if (APlayerController* PlayerController =
		Cast<APlayerController>(GetController()))
	{
		// Get local player subsystem
		if (UEnhancedInputLocalPlayerSubsystem* PlayerSubsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			PlayerSubsystem->AddMappingContext(PlayerDefaultMappingContext, 0);
		}
	}
	
	// Spawn and equip default weapon
	if (DefaultWeaponClass)
	{
		AWeapon* DefaultWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
		EquipWeapon(DefaultWeapon);
	}
}

void AHackAndSlashPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Failsafe: If we're stuck in attacking state but no montage is playing, reset
	if (ActionState == EActionState::EAS_Attacking)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && !AnimInstance->Montage_IsPlaying(PlayerAttackMontage))
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, 
					TEXT("FAILSAFE: Resetting stuck attack state!"));
			}
			
			ActionState = EActionState::EAS_Unoccupied;
			ComboCounter = 0;
			bAttackInputQueued = false;
			bIsComboWindowOpen = false;
		}
	}
}

void AHackAndSlashPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(PlayerMoveAction, ETriggerEvent::Triggered, this, &AHackAndSlashPlayer::Move);
		EnhancedInputComponent->BindAction(PlayerLookAction, ETriggerEvent::Triggered, this, &AHackAndSlashPlayer::Look);
		
		EnhancedInputComponent->BindAction(PlayerJumpAction, ETriggerEvent::Started, this, &AHackAndSlashPlayer::Jump);
		EnhancedInputComponent->BindAction(PlayerJumpAction, ETriggerEvent::Completed, this, &AHackAndSlashPlayer::StopJumping);
		
		EnhancedInputComponent->BindAction(PlayerAttackAction, ETriggerEvent::Started, this, &AHackAndSlashPlayer::Attack);
	}
}

void AHackAndSlashPlayer::Move(const FInputActionValue& Value)
{
	// Block movement if during attack, hit reaction etc
	if (!IsUnoccupied()) return;
	
	FVector2D MovementVector = Value.Get<FVector2D>();
	
	if (GetController() == nullptr) return;

	const FRotator Rotation = GetController()->GetControlRotation();
	const FRotator YawRotation {0.f, Rotation.Yaw, 0.f };

	const FVector ForwardDirection { FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X) };
	const FVector RightDirection { FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y) };

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void AHackAndSlashPlayer::Look(const FInputActionValue& Value)
{
	FVector2D LookVector = Value.Get<FVector2D>();

	if (GetController() == nullptr) return;

	// Mouse
	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);
}

void AHackAndSlashPlayer::Jump()
{
	if (IsUnoccupied())
	{
		Super::Jump();
	}
}

void AHackAndSlashPlayer::StopJumping()
{
	Super::StopJumping();
}

void AHackAndSlashPlayer::Attack()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Cyan, 
			FString::Printf(TEXT("Attack Input - State: %d, Counter: %d, WindowOpen: %s"), 
			(int32)ActionState, ComboCounter, bIsComboWindowOpen ? TEXT("YES") : TEXT("NO")));
	}
	
	// Not attacking - start new combo
	if (CanAttack())
	{
		ComboCounter = 0;
		bAttackInputQueued = false;
		bIsComboWindowOpen = false;
		PerformComboAttack();
	}
	// Currently attacking - queue the input if window is open
	else if (ActionState == EActionState::EAS_Attacking && bIsComboWindowOpen)
	{
		bAttackInputQueued = true;
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(2, 2.0f, FColor::Green, 
				TEXT("Attack queued in combo window!"));
		}
	}
	// Currently attacking but window is closed - ignore input
	else if (ActionState == EActionState::EAS_Attacking && !bIsComboWindowOpen)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(2, 2.0f, FColor::Red, 
				TEXT("Attack ignored - combo window closed!"));
		}
	}
}

void AHackAndSlashPlayer::OpenComboWindow()
{
	bIsComboWindowOpen = true;
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(3, 2.0f, FColor::Yellow, 
			TEXT(">>> COMBO WINDOW OPENED <<<"));
	}
}

void AHackAndSlashPlayer::CloseComboWindow()
{
	bIsComboWindowOpen = false;
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(4, 2.0f, FColor::Orange, 
			TEXT(">>> COMBO WINDOW CLOSED <<<"));
	}
}

void AHackAndSlashPlayer::CheckComboInput()
{
	// This is called every frame during the combo window state
	// If player queued an attack, execute it immediately
	if (bAttackInputQueued && ComboCounter < MaxComboCount)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(5, 2.0f, FColor::Magenta, 
				TEXT("Executing queued attack!"));
		}
		
		// Reset the flag and continue combo
		bAttackInputQueued = false;
		PerformComboAttack();
	}
}

void AHackAndSlashPlayer::AttackEnd()
{
	// This is called at the END of the attack animation (after combo window)
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(6, 2.0f, FColor::Red, 
			FString::Printf(TEXT("AttackEnd - Counter: %d, InputQueued: %s"), 
			ComboCounter, bAttackInputQueued ? TEXT("YES") : TEXT("NO")));
	}
	
	// If we have a queued input that wasn't consumed, it means CheckComboInput didn't fire
	// This shouldn't happen if your AnimNotifyState is set up correctly
	if (bAttackInputQueued)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(7, 3.0f, FColor::Red, 
				TEXT("WARNING: Input was queued but not consumed! Check AnimNotifyState placement."));
		}
	}
	
	// Reset everything - combo is over
	ActionState = EActionState::EAS_Unoccupied;
	ComboCounter = 0;
	bAttackInputQueued = false;
	bIsComboWindowOpen = false;
}

void AHackAndSlashPlayer::EquipWeapon(TObjectPtr<AWeapon> Weapon)
{
	if (Weapon && GetMesh())
	{
		Weapon->AttachMeshToComponent(GetMesh(), WeaponSocketName);
		EquippedWeapon = Weapon;
	}
}

void AHackAndSlashPlayer::PlayMontageSection(TObjectPtr<UAnimMontage> MontageToPlay, const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && MontageToPlay)
	{
		if (!AnimInstance->Montage_IsPlaying(MontageToPlay))
		{
			AnimInstance->Montage_Play(MontageToPlay);
		}
		AnimInstance->Montage_JumpToSection(SectionName, MontageToPlay);
	}
}

void AHackAndSlashPlayer::PlayAttackMontage()
{
	PerformComboAttack();
}

void AHackAndSlashPlayer::PerformComboAttack()
{
	if (PlayerAttackMontage)
	{
		ActionState = EActionState::EAS_Attacking;
		
		// Get appropriate attack section name based on combo counter
		FName SectionName = GetAttackSectionName(ComboCounter);
		PlayMontageSection(PlayerAttackMontage, SectionName);
		
		ComboCounter++;
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(8, 2.0f, FColor::Yellow, 
				FString::Printf(TEXT("Playing: %s (Combo %d/%d)"), 
				*SectionName.ToString(), ComboCounter, MaxComboCount));
		}
	}
}

FName AHackAndSlashPlayer::GetAttackSectionName(int32 ComboIndex)
{
	int32 ClampedIndex = FMath::Clamp(ComboIndex, 0, MaxComboCount - 1);
	
	FString SectionString = FString::Printf(TEXT("%s%d"), *ComboSectionPrefix, ClampedIndex + 1);
	return FName(*SectionString);
}