#include "PlayerCharacter/HackAndSlashPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

#include "HackAndSlashDebugHelper.h"

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
}

void AHackAndSlashPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	}
}

void AHackAndSlashPlayer::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	
	if (GetController() == nullptr) return;

	const FRotator Rotation = GetController()->GetControlRotation();
	const FRotator YawRotation {0.f, Rotation.Yaw, 0.f };

	const FVector ForwardDirection { FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X) };
	const FVector RightDirection { FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y) };

	AddMovementInput(ForwardDirection, MovementVector.Y);
	
	AddMovementInput(RightDirection, MovementVector.X);
	Debug::Print(TEXT("Move Working!"));
}

void AHackAndSlashPlayer::Look(const FInputActionValue& Value)
{
	FVector2D LookVector = Value.Get<FVector2D>();

	if (GetController() == nullptr) return;

	// Mouse
	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);
	
	Debug::Print(TEXT("Look Working!"));
}

void AHackAndSlashPlayer::Jump()
{
	Super::Jump();
	
	Debug::Print(TEXT("Jump Started!"));
}

void AHackAndSlashPlayer::StopJumping()
{
	Super::StopJumping();
	
	Debug::Print(TEXT("Jump Stopped!"));
}