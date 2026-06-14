#include "Quest_QuestTrigger.h"
#include "Engine/Engine.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "TranspersonalCharacter.h"

AQuest_QuestTrigger::AQuest_QuestTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize default values
    QuestID = TEXT("");
    ObjectiveID = TEXT("");
    TriggerType = EQuest_ObjectiveType::ReachLocation;
    RequiredItemID = TEXT("");
    TriggerCount = 1;
    bOneTimeUse = true;
    bIsActivated = false;

    // Set up collision
    if (GetCollisionComponent())
    {
        GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        GetCollisionComponent()->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
        GetCollisionComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
        GetCollisionComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
        
        // Bind overlap events
        GetCollisionComponent()->OnComponentBeginOverlap.AddDynamic(this, &AQuest_QuestTrigger::OnOverlapBegin);
        GetCollisionComponent()->OnComponentEndOverlap.AddDynamic(this, &AQuest_QuestTrigger::OnOverlapEnd);
    }
}

void AQuest_QuestTrigger::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Quest Trigger initialized: %s (Quest: %s, Objective: %s)"), 
           *GetName(), *QuestID, *ObjectiveID);
}

void AQuest_QuestTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
                                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                        bool bFromSweep, const FHitResult& SweepResult)
{
    if (!CanTrigger(OtherActor))
    {
        return;
    }

    // Check if it's the player character
    ATranspersonalCharacter* PlayerCharacter = Cast<ATranspersonalCharacter>(OtherActor);
    if (!PlayerCharacter)
    {
        return;
    }

    ActivateTrigger(OtherActor);
}

void AQuest_QuestTrigger::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // Handle any cleanup when player leaves trigger area
    UE_LOG(LogTemp, Log, TEXT("Player left quest trigger: %s"), *GetName());
}

void AQuest_QuestTrigger::ActivateTrigger(AActor* TriggeringActor)
{
    if (bIsActivated && bOneTimeUse)
    {
        return;
    }

    bIsActivated = true;
    
    UE_LOG(LogTemp, Log, TEXT("Quest Trigger Activated: %s (Quest: %s, Objective: %s)"), 
           *GetName(), *QuestID, *ObjectiveID);

    // Fire Blueprint events
    OnQuestTriggerActivated(QuestID, ObjectiveID);

    // TODO: Integrate with quest manager to update objective progress
    // This would typically call the quest manager to update the specific objective
    
    if (TriggerType == EQuest_ObjectiveType::ReachLocation)
    {
        OnQuestObjectiveCompleted(QuestID, ObjectiveID);
        UE_LOG(LogTemp, Log, TEXT("Location objective completed: %s"), *ObjectiveID);
    }
}

bool AQuest_QuestTrigger::CanTrigger(AActor* TriggeringActor) const
{
    if (!TriggeringActor)
    {
        return false;
    }

    if (bIsActivated && bOneTimeUse)
    {
        return false;
    }

    // Check if it's a player character
    ATranspersonalCharacter* PlayerCharacter = Cast<ATranspersonalCharacter>(TriggeringActor);
    if (!PlayerCharacter)
    {
        return false;
    }

    // Additional checks can be added here (quest active, has required items, etc.)
    
    return true;
}

void AQuest_QuestTrigger::ResetTrigger()
{
    bIsActivated = false;
    UE_LOG(LogTemp, Log, TEXT("Quest Trigger Reset: %s"), *GetName());
}