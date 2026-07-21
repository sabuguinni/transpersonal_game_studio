#include "Quest_CrowdIntegrationManager.h"
#include "../Crowd/Crowd_MassEntityManager.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AQuest_CrowdIntegrationManager::AQuest_CrowdIntegrationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    EventCheckInterval = 2.0f;
    MaxSimultaneousEvents = 5;
    bEnableCrowdQuestIntegration = true;
    LastEventCheck = 0.0f;
    CrowdManager = nullptr;

    // Initialize default crowd events
    FQuest_CrowdEvent GatheringEvent;
    GatheringEvent.EventID = TEXT("TribalGathering");
    GatheringEvent.TriggerType = EQuest_ObjectiveType::Gather;
    GatheringEvent.RequiredCrowdSize = 25;
    GatheringEvent.EventRadius = 1500.0f;
    GatheringEvent.EventDuration = 120.0f;
    ActiveCrowdEvents.Add(GatheringEvent);

    FQuest_CrowdEvent HuntingParty;
    HuntingParty.EventID = TEXT("HuntingParty");
    HuntingParty.TriggerType = EQuest_ObjectiveType::Hunt;
    HuntingParty.RequiredCrowdSize = 15;
    HuntingParty.EventRadius = 2000.0f;
    HuntingParty.EventDuration = 180.0f;
    ActiveCrowdEvents.Add(HuntingParty);

    // Initialize crowd responses
    FQuest_CrowdResponse CelebrationResponse;
    CelebrationResponse.ResponseID = TEXT("QuestCelebration");
    CelebrationResponse.NewBehavior = ECrowd_BehaviorType::Gathering;
    CelebrationResponse.ResponseIntensity = 2.0f;
    CelebrationResponse.ResponseDuration = 60.0f;
    CelebrationResponse.AffectedEntityCount = 100;
    CrowdResponses.Add(CelebrationResponse);

    FQuest_CrowdResponse AlertResponse;
    AlertResponse.ResponseID = TEXT("QuestAlert");
    AlertResponse.NewBehavior = ECrowd_BehaviorType::Fleeing;
    AlertResponse.ResponseIntensity = 3.0f;
    AlertResponse.ResponseDuration = 45.0f;
    AlertResponse.AffectedEntityCount = 200;
    CrowdResponses.Add(AlertResponse);
}

void AQuest_CrowdIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    ValidateCrowdManager();
    
    if (bEnableCrowdQuestIntegration)
    {
        GetWorldTimerManager().SetTimer(EventCheckTimer, this, &AQuest_CrowdIntegrationManager::CheckActiveEvents, EventCheckInterval, true);
        UE_LOG(LogTemp, Log, TEXT("Quest-Crowd Integration Manager initialized with %d events"), ActiveCrowdEvents.Num());
    }
}

void AQuest_CrowdIntegrationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableCrowdQuestIntegration)
    {
        LastEventCheck += DeltaTime;
        if (LastEventCheck >= EventCheckInterval)
        {
            UpdateQuestCrowdObjectives();
            ProcessCrowdResponses();
            LastEventCheck = 0.0f;
        }
    }
}

void AQuest_CrowdIntegrationManager::TriggerCrowdEvent(const FString& EventID, FVector Location, int32 CrowdSize)
{
    if (!bEnableCrowdQuestIntegration || !CrowdManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot trigger crowd event - integration disabled or no crowd manager"));
        return;
    }

    for (FQuest_CrowdEvent& Event : ActiveCrowdEvents)
    {
        if (Event.EventID == EventID && !Event.bIsActive)
        {
            Event.bIsActive = true;
            Event.EventLocation = Location;
            Event.RequiredCrowdSize = CrowdSize;
            
            // Trigger crowd behavior change
            CrowdManager->SetBehaviorInRadius(Location, Event.EventRadius, ECrowd_BehaviorType::Gathering);
            
            UE_LOG(LogTemp, Log, TEXT("Triggered crowd event: %s at location (%f, %f, %f) with %d entities"), 
                *EventID, Location.X, Location.Y, Location.Z, CrowdSize);
            break;
        }
    }
}

void AQuest_CrowdIntegrationManager::EndCrowdEvent(const FString& EventID)
{
    for (FQuest_CrowdEvent& Event : ActiveCrowdEvents)
    {
        if (Event.EventID == EventID && Event.bIsActive)
        {
            Event.bIsActive = false;
            
            if (CrowdManager)
            {
                // Reset crowd behavior to wandering
                CrowdManager->SetBehaviorInRadius(Event.EventLocation, Event.EventRadius, ECrowd_BehaviorType::Wandering);
            }
            
            UE_LOG(LogTemp, Log, TEXT("Ended crowd event: %s"), *EventID);
            break;
        }
    }
}

void AQuest_CrowdIntegrationManager::SetCrowdBehaviorForQuest(const FString& QuestID, ECrowd_BehaviorType NewBehavior, float Duration)
{
    if (!CrowdManager)
    {
        return;
    }

    // Find all active events related to this quest and update their behavior
    for (FQuest_CrowdEvent& Event : ActiveCrowdEvents)
    {
        if (Event.bIsActive && Event.EventID.Contains(QuestID))
        {
            CrowdManager->SetBehaviorInRadius(Event.EventLocation, Event.EventRadius, NewBehavior);
            
            // Schedule behavior reset after duration
            FTimerHandle ResetTimer;
            FTimerDelegate ResetDelegate;
            ResetDelegate.BindLambda([this, Event, Duration]()
            {
                if (CrowdManager)
                {
                    CrowdManager->SetBehaviorInRadius(Event.EventLocation, Event.EventRadius, ECrowd_BehaviorType::Wandering);
                }
            });
            
            GetWorldTimerManager().SetTimer(ResetTimer, ResetDelegate, Duration, false);
        }
    }
}

bool AQuest_CrowdIntegrationManager::IsCrowdEventActive(const FString& EventID) const
{
    for (const FQuest_CrowdEvent& Event : ActiveCrowdEvents)
    {
        if (Event.EventID == EventID)
        {
            return Event.bIsActive;
        }
    }
    return false;
}

int32 AQuest_CrowdIntegrationManager::GetCrowdSizeInRadius(FVector Center, float Radius) const
{
    if (!CrowdManager)
    {
        return 0;
    }
    
    return CrowdManager->GetEntityCountInRadius(Center, Radius);
}

void AQuest_CrowdIntegrationManager::CreateQuestGatheringEvent(FVector Location, int32 RequiredPeople, float Duration)
{
    FQuest_CrowdEvent NewEvent;
    NewEvent.EventID = FString::Printf(TEXT("Gathering_%d"), FMath::RandRange(1000, 9999));
    NewEvent.TriggerType = EQuest_ObjectiveType::Gather;
    NewEvent.RequiredCrowdSize = RequiredPeople;
    NewEvent.EventRadius = 1200.0f;
    NewEvent.EventLocation = Location;
    NewEvent.EventDuration = Duration;
    NewEvent.bIsActive = false;
    
    ActiveCrowdEvents.Add(NewEvent);
    TriggerCrowdEvent(NewEvent.EventID, Location, RequiredPeople);
    
    UE_LOG(LogTemp, Log, TEXT("Created quest gathering event at (%f, %f, %f) for %d people"), 
        Location.X, Location.Y, Location.Z, RequiredPeople);
}

void AQuest_CrowdIntegrationManager::CreateQuestEscortEvent(FVector StartLocation, FVector EndLocation, int32 EscortSize)
{
    FQuest_CrowdEvent NewEvent;
    NewEvent.EventID = FString::Printf(TEXT("Escort_%d"), FMath::RandRange(1000, 9999));
    NewEvent.TriggerType = EQuest_ObjectiveType::Escort;
    NewEvent.RequiredCrowdSize = EscortSize;
    NewEvent.EventRadius = 800.0f;
    NewEvent.EventLocation = StartLocation;
    NewEvent.EventDuration = 300.0f; // 5 minutes
    NewEvent.bIsActive = false;
    
    ActiveCrowdEvents.Add(NewEvent);
    
    if (CrowdManager)
    {
        // Create escort path from start to end
        CrowdManager->CreateEscortPath(StartLocation, EndLocation, EscortSize);
        TriggerCrowdEvent(NewEvent.EventID, StartLocation, EscortSize);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Created quest escort event from (%f, %f, %f) to (%f, %f, %f) with %d entities"), 
        StartLocation.X, StartLocation.Y, StartLocation.Z, EndLocation.X, EndLocation.Y, EndLocation.Z, EscortSize);
}

void AQuest_CrowdIntegrationManager::CreateQuestDefenseEvent(FVector DefenseLocation, float DefenseRadius, float Duration)
{
    FQuest_CrowdEvent NewEvent;
    NewEvent.EventID = FString::Printf(TEXT("Defense_%d"), FMath::RandRange(1000, 9999));
    NewEvent.TriggerType = EQuest_ObjectiveType::Defend;
    NewEvent.RequiredCrowdSize = 30;
    NewEvent.EventRadius = DefenseRadius;
    NewEvent.EventLocation = DefenseLocation;
    NewEvent.EventDuration = Duration;
    NewEvent.bIsActive = false;
    
    ActiveCrowdEvents.Add(NewEvent);
    
    if (CrowdManager)
    {
        // Set defensive positions around the location
        CrowdManager->SetBehaviorInRadius(DefenseLocation, DefenseRadius, ECrowd_BehaviorType::Gathering);
        TriggerCrowdEvent(NewEvent.EventID, DefenseLocation, 30);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Created quest defense event at (%f, %f, %f) with radius %f for %f seconds"), 
        DefenseLocation.X, DefenseLocation.Y, DefenseLocation.Z, DefenseRadius, Duration);
}

void AQuest_CrowdIntegrationManager::UpdateQuestCrowdObjectives()
{
    if (!CrowdManager)
    {
        return;
    }

    // Check each active event and update quest objectives based on crowd behavior
    for (FQuest_CrowdEvent& Event : ActiveCrowdEvents)
    {
        if (Event.bIsActive)
        {
            int32 CurrentCrowdSize = GetCrowdSizeInRadius(Event.EventLocation, Event.EventRadius);
            
            // Check if quest objective is met
            if (CurrentCrowdSize >= Event.RequiredCrowdSize)
            {
                // Trigger quest completion logic here
                UE_LOG(LogTemp, Log, TEXT("Quest crowd objective met for event %s: %d/%d people gathered"), 
                    *Event.EventID, CurrentCrowdSize, Event.RequiredCrowdSize);
                
                // Could trigger quest completion callback here
                // OnQuestObjectiveCompleted.Broadcast(Event.EventID);
            }
        }
    }
}

void AQuest_CrowdIntegrationManager::CheckActiveEvents()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (FQuest_CrowdEvent& Event : ActiveCrowdEvents)
    {
        if (Event.bIsActive)
        {
            // Check if event duration has expired
            if (CurrentTime - LastEventCheck > Event.EventDuration)
            {
                EndCrowdEvent(Event.EventID);
            }
        }
    }
}

void AQuest_CrowdIntegrationManager::ProcessCrowdResponses()
{
    // Process any pending crowd responses to quest events
    for (const FQuest_CrowdResponse& Response : CrowdResponses)
    {
        if (CrowdManager)
        {
            // Apply crowd behavior changes based on quest state
            // This could be expanded to respond to specific quest events
            UE_LOG(LogTemp, Verbose, TEXT("Processing crowd response: %s"), *Response.ResponseID);
        }
    }
}

void AQuest_CrowdIntegrationManager::ValidateCrowdManager()
{
    if (!CrowdManager)
    {
        // Try to find the crowd manager in the world
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), UCrowd_MassEntityManager::StaticClass(), FoundActors);
        
        if (FoundActors.Num() > 0)
        {
            CrowdManager = Cast<UCrowd_MassEntityManager>(FoundActors[0]);
            UE_LOG(LogTemp, Log, TEXT("Found and linked to Crowd Manager"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No Crowd Manager found in world - quest-crowd integration disabled"));
        }
    }
}