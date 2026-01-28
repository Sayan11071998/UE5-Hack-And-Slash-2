#include "PlayerCharacter/Notifies/PlayerComboWindowAnimNotifyState.h"
#include "PlayerCharacter/HackAndSlashPlayer.h"

void UPlayerComboWindowAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	if (MeshComp && MeshComp->GetOwner())
	{
		AHackAndSlashPlayer* PlayerCharacter = Cast<AHackAndSlashPlayer>(MeshComp->GetOwner());
		
		if (PlayerCharacter)
		{
			PlayerCharacter->OpenComboWindow();
		}
	}
}

void UPlayerComboWindowAnimNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	
	// Check every frame if player queued an attack during the window
	if (MeshComp && MeshComp->GetOwner())
	{
		AHackAndSlashPlayer* PlayerCharacter = Cast<AHackAndSlashPlayer>(MeshComp->GetOwner());
		
		if (PlayerCharacter)
		{
			PlayerCharacter->CheckComboInput();
		}
	}
}

void UPlayerComboWindowAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	if (MeshComp && MeshComp->GetOwner())
	{
		AHackAndSlashPlayer* PlayerCharacter = Cast<AHackAndSlashPlayer>(MeshComp->GetOwner());
		
		if (PlayerCharacter)
		{
			PlayerCharacter->CloseComboWindow();
		}
	}
}