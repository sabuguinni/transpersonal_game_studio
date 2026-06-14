#include "StoryTrigger.h"
#include "Engine/Engine.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"

AStoryTrigger::AStoryTrigger()
{
    PrimaryActorTick.bCanEverTick = false;
    
    bTriggerOnce = true;
    bRequirePlayerCharacter = true;
    bHasTriggered = false;
    
    // Set up collision
    GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GetCollisionComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
    GetCollisionComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}

void AStoryTrigger::BeginPlay()
{
    Super::BeginPlay();
    
    // Get narrative manager
    if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
    {
        NarrativeManager = GameInstance->GetSubsystem<UNarrativeManager>();
    }
    
    // Bind overlap events
    OnActorBeginOverlap.AddDynamic(this, &AStoryTrigger::OnOverlapBegin);
    OnActorEndOverlap.AddDynamic(this, &AStoryTrigger::OnOverlapEnd);
}

void AStoryTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!CanTrigger(OtherActor))
    {
        return;
    }

    if (bTriggerOnce && bHasTriggered)
    {
        return;
    }

    // Check if required story flags are met
    if (NarrativeManager)
    {
        bool bCanTrigger = true;
        for (const FString& Flag : RequiredFlags)
        {
            if (!NarrativeManager->GetStoryFlag(Flag))
            {
                bCanTrigger = false;
                break;
            }
        }

        if (bCanTrigger)
        {
            // Trigger story beat
            if (!StoryBeatToTrigger.IsEmpty())
            {
                NarrativeManager->StartStoryBeat(StoryBeatToTrigger);
                UE_LOG(LogTemp, Log, TEXT("Story trigger activated: %s"), *StoryBeatToTrigger);
            }

            // Trigger dialogue if specified
            if (!TriggerDialogue.IsEmpty())
            {
                NarrativeManager->TriggerDialogue(TriggerDialogue, this);
            }

            bHasTriggered = true;
        }
    }
}

void AStoryTrigger::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // Handle any cleanup when actor leaves trigger
}

bool AStoryTrigger::CanTrigger(AActor* TriggeringActor) const
{
    if (!TriggeringActor)
    {
        return false;
    }

    // Check if we require player character
    if (bRequirePlayerCharacter)
    {
        ACharacter* Character = Cast<ACharacter>(TriggeringActor);
        if (!Character || !Character->IsPlayerControlled())
        {
            return false;
        }
    }

    return true;
}