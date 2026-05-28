#include "Quest_PrehistoricQuestTriggerSystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UQuest_PrehistoricQuestTriggerSystem::UQuest_PrehistoricQuestTriggerSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    LocationCheckInterval = 1.0f;
    bEnableDebugOutput = false;
    LastPlayerLocation = FVector::ZeroVector;
    LocationCheckTimer = 0.0f;
}

void UQuest_PrehistoricQuestTriggerSystem::BeginPlay()
{
    Super::BeginPlay();
    
    if (AActor* PlayerActor = GetPlayerActor())
    {
        LastPlayerLocation = PlayerActor->GetActorLocation();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Quest Trigger System initialized with %d active triggers"), ActiveTriggers.Num());
}

void UQuest_PrehistoricQuestTriggerSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LocationCheckTimer += DeltaTime;
    
    if (LocationCheckTimer >= LocationCheckInterval)
    {
        CheckLocationTriggers();
        LocationCheckTimer = 0.0f;
    }
    
    UpdateTimerTriggers(DeltaTime);
}

void UQuest_PrehistoricQuestTriggerSystem::RegisterQuestTrigger(const FString& QuestID, const FQuest_TriggerCondition& TriggerCondition)
{
    if (QuestID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot register quest trigger with empty QuestID"));
        return;
    }
    
    ActiveTriggers.Add(QuestID, TriggerCondition);
    
    if (TriggerCondition.TriggerType == EQuest_TriggerType::Timer)
    {
        TriggerTimers.Add(QuestID, 0.0f);
    }
    
    if (bEnableDebugOutput)
    {
        UE_LOG(LogTemp, Log, TEXT("Registered quest trigger for %s (Type: %d)"), *QuestID, (int32)TriggerCondition.TriggerType);
    }
}

void UQuest_PrehistoricQuestTriggerSystem::UnregisterQuestTrigger(const FString& QuestID)
{
    if (ActiveTriggers.Remove(QuestID) > 0)
    {
        TriggerTimers.Remove(QuestID);
        
        if (bEnableDebugOutput)
        {
            UE_LOG(LogTemp, Log, TEXT("Unregistered quest trigger for %s"), *QuestID);
        }
    }
}

bool UQuest_PrehistoricQuestTriggerSystem::CheckTriggerCondition(const FString& QuestID)
{
    if (!ActiveTriggers.Contains(QuestID))
    {
        return false;
    }
    
    const FQuest_TriggerCondition& Condition = ActiveTriggers[QuestID];
    
    if (!Condition.bIsActive)
    {
        return false;
    }
    
    switch (Condition.TriggerType)
    {
        case EQuest_TriggerType::Location:
            return CheckLocationTrigger(Condition);
            
        case EQuest_TriggerType::Proximity:
            return CheckProximityTrigger(Condition);
            
        case EQuest_TriggerType::Resource:
            return CheckResourceTrigger(Condition);
            
        case EQuest_TriggerType::Timer:
            return CheckTimerTrigger(QuestID, Condition);
            
        case EQuest_TriggerType::Interaction:
        case EQuest_TriggerType::Combat:
        case EQuest_TriggerType::Discovery:
        case EQuest_TriggerType::Survival:
            // These are event-based triggers, checked externally
            return false;
            
        default:
            return false;
    }
}

void UQuest_PrehistoricQuestTriggerSystem::ActivateTrigger(const FString& QuestID)
{
    if (ActiveTriggers.Contains(QuestID))
    {
        ActiveTriggers[QuestID].bIsActive = true;
        
        if (bEnableDebugOutput)
        {
            UE_LOG(LogTemp, Log, TEXT("Activated quest trigger: %s"), *QuestID);
        }
    }
}

void UQuest_PrehistoricQuestTriggerSystem::DeactivateTrigger(const FString& QuestID)
{
    if (ActiveTriggers.Contains(QuestID))
    {
        ActiveTriggers[QuestID].bIsActive = false;
        
        if (bEnableDebugOutput)
        {
            UE_LOG(LogTemp, Log, TEXT("Deactivated quest trigger: %s"), *QuestID);
        }
    }
}

bool UQuest_PrehistoricQuestTriggerSystem::CheckLocationTrigger(const FQuest_TriggerCondition& Condition)
{
    AActor* PlayerActor = GetPlayerActor();
    if (!PlayerActor)
    {
        return false;
    }
    
    FVector PlayerLocation = PlayerActor->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, Condition.TargetLocation);
    
    return Distance <= Condition.TriggerRadius;
}

bool UQuest_PrehistoricQuestTriggerSystem::CheckProximityTrigger(const FQuest_TriggerCondition& Condition)
{
    AActor* PlayerActor = GetPlayerActor();
    if (!PlayerActor)
    {
        return false;
    }
    
    TArray<AActor*> TargetActors = GetActorsWithTag(Condition.TargetActorTag);
    FVector PlayerLocation = PlayerActor->GetActorLocation();
    
    for (AActor* TargetActor : TargetActors)
    {
        if (TargetActor)
        {
            float Distance = FVector::Dist(PlayerLocation, TargetActor->GetActorLocation());
            if (Distance <= Condition.TriggerRadius)
            {
                return true;
            }
        }
    }
    
    return false;
}

bool UQuest_PrehistoricQuestTriggerSystem::CheckResourceTrigger(const FQuest_TriggerCondition& Condition)
{
    // This would typically check player inventory
    // For now, return false as inventory system is not implemented
    return false;
}

bool UQuest_PrehistoricQuestTriggerSystem::CheckTimerTrigger(const FString& QuestID, const FQuest_TriggerCondition& Condition)
{
    if (!TriggerTimers.Contains(QuestID))
    {
        return false;
    }
    
    return TriggerTimers[QuestID] >= Condition.TimerDuration;
}

void UQuest_PrehistoricQuestTriggerSystem::OnPlayerLocationChanged(const FVector& NewLocation)
{
    LastPlayerLocation = NewLocation;
    
    // Check all location-based triggers
    for (auto& TriggerPair : ActiveTriggers)
    {
        const FString& QuestID = TriggerPair.Key;
        const FQuest_TriggerCondition& Condition = TriggerPair.Value;
        
        if (Condition.bIsActive && Condition.TriggerType == EQuest_TriggerType::Location)
        {
            if (CheckLocationTrigger(Condition))
            {
                // Trigger quest event
                if (bEnableDebugOutput)
                {
                    UE_LOG(LogTemp, Log, TEXT("Location trigger activated for quest: %s"), *QuestID);
                }
            }
        }
    }
}

void UQuest_PrehistoricQuestTriggerSystem::OnResourceCollected(const FString& ResourceType, int32 Quantity)
{
    for (auto& TriggerPair : ActiveTriggers)
    {
        const FString& QuestID = TriggerPair.Key;
        const FQuest_TriggerCondition& Condition = TriggerPair.Value;
        
        if (Condition.bIsActive && 
            Condition.TriggerType == EQuest_TriggerType::Resource &&
            Condition.TargetActorTag == ResourceType)
        {
            if (Quantity >= Condition.RequiredQuantity)
            {
                if (bEnableDebugOutput)
                {
                    UE_LOG(LogTemp, Log, TEXT("Resource trigger activated for quest: %s (Resource: %s, Quantity: %d)"), 
                           *QuestID, *ResourceType, Quantity);
                }
            }
        }
    }
}

void UQuest_PrehistoricQuestTriggerSystem::OnCombatEvent(const FString& EventType, AActor* Target)
{
    for (auto& TriggerPair : ActiveTriggers)
    {
        const FString& QuestID = TriggerPair.Key;
        const FQuest_TriggerCondition& Condition = TriggerPair.Value;
        
        if (Condition.bIsActive && Condition.TriggerType == EQuest_TriggerType::Combat)
        {
            if (bEnableDebugOutput)
            {
                UE_LOG(LogTemp, Log, TEXT("Combat trigger activated for quest: %s (Event: %s)"), *QuestID, *EventType);
            }
        }
    }
}

void UQuest_PrehistoricQuestTriggerSystem::OnDiscoveryEvent(const FString& DiscoveryType, const FVector& Location)
{
    for (auto& TriggerPair : ActiveTriggers)
    {
        const FString& QuestID = TriggerPair.Key;
        const FQuest_TriggerCondition& Condition = TriggerPair.Value;
        
        if (Condition.bIsActive && Condition.TriggerType == EQuest_TriggerType::Discovery)
        {
            if (bEnableDebugOutput)
            {
                UE_LOG(LogTemp, Log, TEXT("Discovery trigger activated for quest: %s (Type: %s)"), *QuestID, *DiscoveryType);
            }
        }
    }
}

TArray<FString> UQuest_PrehistoricQuestTriggerSystem::GetActiveQuestTriggers()
{
    TArray<FString> ActiveQuestIDs;
    
    for (auto& TriggerPair : ActiveTriggers)
    {
        if (TriggerPair.Value.bIsActive)
        {
            ActiveQuestIDs.Add(TriggerPair.Key);
        }
    }
    
    return ActiveQuestIDs;
}

int32 UQuest_PrehistoricQuestTriggerSystem::GetTriggerCount()
{
    return ActiveTriggers.Num();
}

void UQuest_PrehistoricQuestTriggerSystem::ClearAllTriggers()
{
    ActiveTriggers.Empty();
    TriggerTimers.Empty();
    
    if (bEnableDebugOutput)
    {
        UE_LOG(LogTemp, Log, TEXT("Cleared all quest triggers"));
    }
}

void UQuest_PrehistoricQuestTriggerSystem::DebugPrintActiveTriggers()
{
    UE_LOG(LogTemp, Log, TEXT("=== Active Quest Triggers ==="));
    
    for (auto& TriggerPair : ActiveTriggers)
    {
        const FString& QuestID = TriggerPair.Key;
        const FQuest_TriggerCondition& Condition = TriggerPair.Value;
        
        FString TriggerTypeStr = "";
        switch (Condition.TriggerType)
        {
            case EQuest_TriggerType::Location: TriggerTypeStr = "Location"; break;
            case EQuest_TriggerType::Proximity: TriggerTypeStr = "Proximity"; break;
            case EQuest_TriggerType::Interaction: TriggerTypeStr = "Interaction"; break;
            case EQuest_TriggerType::Timer: TriggerTypeStr = "Timer"; break;
            case EQuest_TriggerType::Resource: TriggerTypeStr = "Resource"; break;
            case EQuest_TriggerType::Combat: TriggerTypeStr = "Combat"; break;
            case EQuest_TriggerType::Discovery: TriggerTypeStr = "Discovery"; break;
            case EQuest_TriggerType::Survival: TriggerTypeStr = "Survival"; break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Quest: %s | Type: %s | Active: %s"), 
               *QuestID, *TriggerTypeStr, Condition.bIsActive ? TEXT("Yes") : TEXT("No"));
    }
}

void UQuest_PrehistoricQuestTriggerSystem::TestLocationTrigger(const FVector& TestLocation, float TestRadius)
{
    FQuest_TriggerCondition TestCondition;
    TestCondition.TriggerType = EQuest_TriggerType::Location;
    TestCondition.TargetLocation = TestLocation;
    TestCondition.TriggerRadius = TestRadius;
    TestCondition.bIsActive = true;
    
    bool bTriggered = CheckLocationTrigger(TestCondition);
    
    UE_LOG(LogTemp, Log, TEXT("Location trigger test: Location=%s, Radius=%.1f, Triggered=%s"), 
           *TestLocation.ToString(), TestRadius, bTriggered ? TEXT("Yes") : TEXT("No"));
}

void UQuest_PrehistoricQuestTriggerSystem::UpdateTimerTriggers(float DeltaTime)
{
    for (auto& TimerPair : TriggerTimers)
    {
        const FString& QuestID = TimerPair.Key;
        
        if (ActiveTriggers.Contains(QuestID) && ActiveTriggers[QuestID].bIsActive)
        {
            TimerPair.Value += DeltaTime;
            
            // Check if timer trigger is complete
            const FQuest_TriggerCondition& Condition = ActiveTriggers[QuestID];
            if (Condition.TriggerType == EQuest_TriggerType::Timer && 
                TimerPair.Value >= Condition.TimerDuration)
            {
                if (bEnableDebugOutput)
                {
                    UE_LOG(LogTemp, Log, TEXT("Timer trigger completed for quest: %s (%.1fs)"), 
                           *QuestID, TimerPair.Value);
                }
            }
        }
    }
}

void UQuest_PrehistoricQuestTriggerSystem::CheckLocationTriggers()
{
    AActor* PlayerActor = GetPlayerActor();
    if (!PlayerActor)
    {
        return;
    }
    
    FVector CurrentLocation = PlayerActor->GetActorLocation();
    
    // Only check if player has moved significantly
    if (FVector::Dist(CurrentLocation, LastPlayerLocation) > 50.0f)
    {
        OnPlayerLocationChanged(CurrentLocation);
    }
}

AActor* UQuest_PrehistoricQuestTriggerSystem::GetPlayerActor()
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            return PC->GetPawn();
        }
    }
    return nullptr;
}

TArray<AActor*> UQuest_PrehistoricQuestTriggerSystem::GetActorsWithTag(const FString& Tag)
{
    TArray<AActor*> FoundActors;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->Tags.Contains(FName(*Tag)))
            {
                FoundActors.Add(Actor);
            }
        }
    }
    
    return FoundActors;
}