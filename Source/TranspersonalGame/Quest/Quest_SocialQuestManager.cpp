#include "Quest_SocialQuestManager.h"
#include "../Crowd/Crowd_SocialDynamicsManager.h"
#include "Quest_NPCManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UQuest_SocialQuestManager::UQuest_SocialQuestManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    TriggerCheckInterval = 2.0f;
    MaxSimultaneousConflicts = 3;
    QuestGenerationRadius = 2000.0f;
    MaxActiveQuests = 10;
    LastTriggerCheckTime = 0.0f;
    
    // Initialize quest templates
    AvailableQuestTemplates.Add(TEXT("TribalMediation"));
    AvailableQuestTemplates.Add(TEXT("ResourceDispute"));
    AvailableQuestTemplates.Add(TEXT("TerritorialNegotiation"));
    AvailableQuestTemplates.Add(TEXT("AllianceFormation"));
    AvailableQuestTemplates.Add(TEXT("ConflictResolution"));
}

void UQuest_SocialQuestManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find social dynamics manager reference
    if (GetWorld())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            if (UCrowd_SocialDynamicsManager* SocialComp = Actor->FindComponentByClass<UCrowd_SocialDynamicsManager>())
            {
                SocialDynamicsRef = SocialComp;
                break;
            }
            
            if (UQuest_NPCManager* NPCComp = Actor->FindComponentByClass<UQuest_NPCManager>())
            {
                NPCManagerRef = NPCComp;
            }
        }
    }
    
    // Start periodic trigger checking
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(TriggerCheckTimer, this, &UQuest_SocialQuestManager::CheckSocialTriggers, TriggerCheckInterval, true);
        GetWorld()->GetTimerManager().SetTimer(ConflictUpdateTimer, this, &UQuest_SocialQuestManager::UpdateActiveConflicts, 5.0f, true);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Quest_SocialQuestManager: Initialized with %d quest templates"), AvailableQuestTemplates.Num());
}

void UQuest_SocialQuestManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update conflict intensities based on time
    for (FQuest_TribalConflictData& Conflict : ActiveConflicts)
    {
        float TimeSinceStart = GetWorld()->GetTimeSeconds() - Conflict.StartTime;
        if (TimeSinceStart > 30.0f) // Conflicts escalate over time
        {
            Conflict.ConflictIntensity = FMath::Min(1.0f, Conflict.ConflictIntensity + DeltaTime * 0.1f);
        }
    }
}

void UQuest_SocialQuestManager::RegisterSocialTrigger(const FQuest_SocialTrigger& NewTrigger)
{
    ActiveSocialTriggers.Add(NewTrigger);
    UE_LOG(LogTemp, Log, TEXT("Quest_SocialQuestManager: Registered social trigger for quest %s"), *NewTrigger.QuestID);
}

void UQuest_SocialQuestManager::RemoveSocialTrigger(const FString& QuestID)
{
    ActiveSocialTriggers.RemoveAll([QuestID](const FQuest_SocialTrigger& Trigger)
    {
        return Trigger.QuestID == QuestID;
    });
    UE_LOG(LogTemp, Log, TEXT("Quest_SocialQuestManager: Removed social trigger for quest %s"), *QuestID);
}

bool UQuest_SocialQuestManager::CheckSocialTriggers()
{
    if (!SocialDynamicsRef || !GetWorld())
    {
        return false;
    }
    
    bool bTriggersActivated = false;
    LastTriggerCheckTime = GetWorld()->GetTimeSeconds();
    
    // Check proximity-based triggers
    CheckProximityTriggers();
    
    // Check interaction-based triggers
    CheckInteractionTriggers();
    
    return bTriggersActivated;
}

void UQuest_SocialQuestManager::StartTribalConflict(const TArray<AActor*>& Tribe1, const TArray<AActor*>& Tribe2, const FVector& Location)
{
    if (ActiveConflicts.Num() >= MaxSimultaneousConflicts)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_SocialQuestManager: Cannot start new conflict - maximum conflicts reached"));
        return;
    }
    
    FQuest_TribalConflictData NewConflict;
    NewConflict.ConflictID = GenerateUniqueConflictID();
    NewConflict.Tribe1Members = Tribe1;
    NewConflict.Tribe2Members = Tribe2;
    NewConflict.ConflictLocation = Location;
    NewConflict.ConflictIntensity = 0.3f; // Start with low intensity
    NewConflict.ResolutionType = EQuest_ConflictResolution::Negotiation;
    NewConflict.StartTime = GetWorld()->GetTimeSeconds();
    
    ActiveConflicts.Add(NewConflict);
    
    // Generate related quest
    FString ConflictQuestID = GenerateQuestFromSocialEvent(ECrowd_SocialInteractionType::Aggressive, Tribe1);
    
    UE_LOG(LogTemp, Log, TEXT("Quest_SocialQuestManager: Started tribal conflict %s with %d vs %d members"), 
           *NewConflict.ConflictID, Tribe1.Num(), Tribe2.Num());
}

void UQuest_SocialQuestManager::ResolveTribalConflict(const FString& ConflictID, EQuest_ConflictResolution Resolution)
{
    for (int32 i = ActiveConflicts.Num() - 1; i >= 0; i--)
    {
        if (ActiveConflicts[i].ConflictID == ConflictID)
        {
            FQuest_TribalConflictData& Conflict = ActiveConflicts[i];
            Conflict.ResolutionType = Resolution;
            
            // Apply reputation changes based on resolution
            switch (Resolution)
            {
                case EQuest_ConflictResolution::Negotiation:
                    UpdatePlayerReputationWithTribe(TEXT("Tribe1"), 10.0f);
                    UpdatePlayerReputationWithTribe(TEXT("Tribe2"), 10.0f);
                    break;
                case EQuest_ConflictResolution::Force:
                    UpdatePlayerReputationWithTribe(TEXT("Tribe1"), -5.0f);
                    UpdatePlayerReputationWithTribe(TEXT("Tribe2"), -5.0f);
                    break;
                case EQuest_ConflictResolution::Avoidance:
                    // No reputation change
                    break;
            }
            
            // Remove resolved conflict
            ActiveConflicts.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Quest_SocialQuestManager: Resolved conflict %s via %s"), 
                   *ConflictID, *UEnum::GetValueAsString(Resolution));
            break;
        }
    }
}

void UQuest_SocialQuestManager::UpdateActiveConflicts()
{
    for (FQuest_TribalConflictData& Conflict : ActiveConflicts)
    {
        ProcessConflictEscalation(Conflict);
    }
}

FString UQuest_SocialQuestManager::GenerateQuestFromSocialEvent(ECrowd_SocialInteractionType InteractionType, const TArray<AActor*>& InvolvedActors)
{
    if (AvailableQuestTemplates.Num() == 0 || InvolvedActors.Num() == 0)
    {
        return TEXT("");
    }
    
    FString QuestTemplate = AvailableQuestTemplates[FMath::RandRange(0, AvailableQuestTemplates.Num() - 1)];
    FString QuestID = FString::Printf(TEXT("%s_%d"), *QuestTemplate, FMath::RandRange(1000, 9999));
    
    // Create social trigger for this quest
    FQuest_SocialTrigger NewTrigger;
    NewTrigger.QuestID = QuestID;
    NewTrigger.RequiredInteractionType = InteractionType;
    NewTrigger.MinGroupSize = FMath::Max(2, InvolvedActors.Num() / 2);
    NewTrigger.TriggerRadius = 800.0f;
    NewTrigger.bIsActive = true;
    
    RegisterSocialTrigger(NewTrigger);
    
    UE_LOG(LogTemp, Log, TEXT("Quest_SocialQuestManager: Generated quest %s from %s interaction"), 
           *QuestID, *UEnum::GetValueAsString(InteractionType));
    
    return QuestID;
}

TArray<FString> UQuest_SocialQuestManager::GetAvailableQuestsForLocation(const FVector& Location, float Radius)
{
    TArray<FString> AvailableQuests;
    
    for (const FQuest_SocialTrigger& Trigger : ActiveSocialTriggers)
    {
        if (Trigger.bIsActive && FVector::Dist(GetOwner()->GetActorLocation(), Location) <= Radius)
        {
            AvailableQuests.Add(Trigger.QuestID);
        }
    }
    
    return AvailableQuests;
}

bool UQuest_SocialQuestManager::CanPlayerInterveneInConflict(const FString& ConflictID)
{
    for (const FQuest_TribalConflictData& Conflict : ActiveConflicts)
    {
        if (Conflict.ConflictID == ConflictID)
        {
            return Conflict.ConflictIntensity < 0.8f; // Can only intervene before it gets too intense
        }
    }
    return false;
}

void UQuest_SocialQuestManager::PlayerInterventionChoice(const FString& ConflictID, EQuest_InterventionType InterventionType)
{
    for (FQuest_TribalConflictData& Conflict : ActiveConflicts)
    {
        if (Conflict.ConflictID == ConflictID)
        {
            switch (InterventionType)
            {
                case EQuest_InterventionType::Mediate:
                    Conflict.ConflictIntensity *= 0.5f; // Reduce conflict intensity
                    UpdatePlayerReputationWithTribe(TEXT("Tribe1"), 15.0f);
                    UpdatePlayerReputationWithTribe(TEXT("Tribe2"), 15.0f);
                    break;
                case EQuest_InterventionType::Support:
                    Conflict.ConflictIntensity *= 0.7f;
                    UpdatePlayerReputationWithTribe(TEXT("Tribe1"), 20.0f);
                    UpdatePlayerReputationWithTribe(TEXT("Tribe2"), -10.0f);
                    break;
                case EQuest_InterventionType::Ignore:
                    // No immediate effect, but conflict may escalate
                    break;
            }
            
            UE_LOG(LogTemp, Log, TEXT("Quest_SocialQuestManager: Player intervened in conflict %s with %s"), 
                   *ConflictID, *UEnum::GetValueAsString(InterventionType));
            break;
        }
    }
}

void UQuest_SocialQuestManager::UpdatePlayerReputationWithTribe(const FString& TribeID, float ReputationChange)
{
    if (TribeReputations.Contains(TribeID))
    {
        TribeReputations[TribeID] += ReputationChange;
    }
    else
    {
        TribeReputations.Add(TribeID, ReputationChange);
    }
    
    // Clamp reputation between -100 and 100
    TribeReputations[TribeID] = FMath::Clamp(TribeReputations[TribeID], -100.0f, 100.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Quest_SocialQuestManager: Updated reputation with %s: %f"), 
           *TribeID, TribeReputations[TribeID]);
}

float UQuest_SocialQuestManager::GetPlayerReputationWithTribe(const FString& TribeID)
{
    if (TribeReputations.Contains(TribeID))
    {
        return TribeReputations[TribeID];
    }
    return 0.0f; // Neutral reputation
}

void UQuest_SocialQuestManager::CheckProximityTriggers()
{
    if (!GetWorld() || !GetOwner())
    {
        return;
    }
    
    FVector PlayerLocation = GetOwner()->GetActorLocation();
    
    for (FQuest_SocialTrigger& Trigger : ActiveSocialTriggers)
    {
        if (!Trigger.bIsActive)
        {
            continue;
        }
        
        // Find nearby actors for proximity check
        TArray<AActor*> NearbyActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);
        
        int32 ValidActorsCount = 0;
        for (AActor* Actor : NearbyActors)
        {
            if (Actor && FVector::Dist(PlayerLocation, Actor->GetActorLocation()) <= Trigger.TriggerRadius)
            {
                ValidActorsCount++;
            }
        }
        
        if (ValidActorsCount >= Trigger.MinGroupSize)
        {
            UE_LOG(LogTemp, Log, TEXT("Quest_SocialQuestManager: Proximity trigger activated for quest %s"), *Trigger.QuestID);
            // Trigger quest logic here
        }
    }
}

void UQuest_SocialQuestManager::CheckInteractionTriggers()
{
    // Check for specific social interactions that should trigger quests
    if (SocialDynamicsRef)
    {
        // This would integrate with the social dynamics system to detect interactions
        // For now, we'll simulate interaction detection
        
        for (FQuest_SocialTrigger& Trigger : ActiveSocialTriggers)
        {
            if (Trigger.bIsActive && Trigger.RequiredInteractionType != ECrowd_SocialInteractionType::Neutral)
            {
                // Simulate interaction detection - in a real implementation,
                // this would query the social dynamics system
                if (FMath::RandRange(0, 100) < 5) // 5% chance per check
                {
                    UE_LOG(LogTemp, Log, TEXT("Quest_SocialQuestManager: Interaction trigger activated for quest %s"), *Trigger.QuestID);
                }
            }
        }
    }
}

void UQuest_SocialQuestManager::ProcessConflictEscalation(FQuest_TribalConflictData& Conflict)
{
    float TimeSinceStart = GetWorld()->GetTimeSeconds() - Conflict.StartTime;
    
    // Conflicts naturally escalate over time if unresolved
    if (TimeSinceStart > 60.0f && Conflict.ConflictIntensity < 0.9f)
    {
        Conflict.ConflictIntensity += 0.05f;
        
        // Generate escalation quest if intensity gets high
        if (Conflict.ConflictIntensity > 0.7f)
        {
            FString EscalationQuestID = GenerateQuestFromSocialEvent(ECrowd_SocialInteractionType::Aggressive, Conflict.Tribe1Members);
            UE_LOG(LogTemp, Warning, TEXT("Quest_SocialQuestManager: Conflict %s escalated - generated emergency quest %s"), 
                   *Conflict.ConflictID, *EscalationQuestID);
        }
    }
}

FString UQuest_SocialQuestManager::GenerateUniqueConflictID()
{
    return FString::Printf(TEXT("Conflict_%d_%d"), 
                          FMath::RandRange(1000, 9999), 
                          (int32)GetWorld()->GetTimeSeconds());
}

bool UQuest_SocialQuestManager::IsLocationSafeForQuest(const FVector& Location)
{
    // Check if location is too close to recent conflicts
    for (const FVector& ConflictLocation : RecentConflictLocations)
    {
        if (FVector::Dist(Location, ConflictLocation) < 1000.0f)
        {
            return false;
        }
    }
    return true;
}