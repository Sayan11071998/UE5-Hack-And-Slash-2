#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterTypes.h"
#include "HackAndSlashPlayer.generated.h"

struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
class UCameraComponent;
class USpringArmComponent;
class AWeapon;

UCLASS()
class HACKANDSLASH_API AHackAndSlashPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	AHackAndSlashPlayer();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;
	
	// Movement methods
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	virtual void Jump() override;
	virtual void StopJumping() override;
	
	// Attack method
	void Attack();
	
	// Combat
	void EquipWeapon(TObjectPtr<AWeapon> Weapon);
	
private:
	// Camera settings
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera Settings", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera Settings", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> ViewCamera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera Settings", meta = (AllowPrivateAccess = "true"))
	float CameraArmLength;
	
	// Enhanced input system
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> PlayerDefaultMappingContext;
	
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> PlayerMoveAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> PlayerLookAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> PlayerJumpAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> PlayerAttackAction;
	
	// Weapon system
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<AWeapon> DefaultWeaponClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FName WeaponSocketName;
	
	UPROPERTY()
	TObjectPtr<AWeapon> EquippedWeapon;
	
	// Action state
	UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;
	
public:
	FORCEINLINE bool IsUnoccupied() const { return ActionState == EActionState::EAS_Unoccupied ; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }
};