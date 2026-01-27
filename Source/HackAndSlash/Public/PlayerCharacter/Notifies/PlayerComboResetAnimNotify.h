#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PlayerComboResetAnimNotify.generated.h"

UCLASS()
class HACKANDSLASH_API UPlayerComboResetAnimNotify : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	// ~ Begin UAnimNotify interface
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	// ~ End UAnimNotify interface
};