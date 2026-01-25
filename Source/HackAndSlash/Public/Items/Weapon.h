#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UCLASS()
class HACKANDSLASH_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	
	void AttackMeshToComponent(TObjectPtr<USceneComponent> InParent, const FName& InSocketName);
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> WeaponMesh;
};