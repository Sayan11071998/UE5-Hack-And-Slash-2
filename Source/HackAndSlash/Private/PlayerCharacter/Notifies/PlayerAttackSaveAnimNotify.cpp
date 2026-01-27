#include "PlayerCharacter/Notifies/PlayerAttackSaveAnimNotify.h"
#include "PlayerCharacter/HackAndSlashPlayer.h"

void UPlayerAttackSaveAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if (MeshComp && MeshComp->GetOwner())
	{
		AHackAndSlashPlayer* PlayerCharacter = Cast<AHackAndSlashPlayer>(MeshComp->GetOwner());
		
		if (PlayerCharacter)
		{
			// PlayerCharacter->SaveAttack();
		}
	}
}