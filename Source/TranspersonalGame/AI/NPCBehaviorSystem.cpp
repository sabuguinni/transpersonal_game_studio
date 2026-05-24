#include "NPCBehaviorSystem.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"

UNPCBehaviorSystem::UNPCBehaviorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Default values
    Personality = ENPCPersonality::Peaceful;
    CurrentState = ENPCState::Idle;
    NPCName = TEXT("Unnamed NPC");
    NPCAge = 25;
    
    // Memory settings
    MemoryRetentionTime = 300.0f; // 5 minutes
    MemoryCleanupTimer = 0.0f;
    
    // Routine settings
    bFollowDailyRoutine = true;
    RoutineUpdateTimer = 0.0f;
    
    // Perception settings
    SightRadius = 1500.0f;
    HearingRadius = 800.0f;
    PeripheralVisionAngle = 90.0f;
    
    // Initialize memory
    NPCMemory = FNPCMemory();
}

void UNPCBehaviorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get AI Controller reference
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        NPCAIController = Cast<AAIController>(OwnerPawn->GetController());
        
        if (NPCAIController)
        {
            BlackboardComponent = NPCAIController->GetBlackboardComponent();
            
            // Start behavior tree if available
            if (BehaviorTree)
            {
                StartBehaviorTree();
            }
        }
    }
    
    // Setup default daily routine if none exists
    if (DailyRoutines.Num() == 0)
    {
        SetupDefaultRoutines();
    }
    
    UE_LOG(LogTemp, Log, TEXT("NPC Behavior System initialized for: %s"), *NPCName);
}

void UNPCBehaviorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update memory system
    ProcessMemory(DeltaTime);
    
    // Update daily routine
    if (bFollowDailyRoutine)
    {
        UpdateDailyRoutine();
    }
    
    // Handle perception
    HandlePerception();
    
    // Update blackboard values
    if (BlackboardComponent)
    {
        UpdateBlackboardValues();
    }
}

void UNPCBehaviorSystem::SetNPCState(ENPCState NewState)
{
    if (CurrentState != NewState)
    {
        ENPCState PreviousState = CurrentState;
        CurrentState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("NPC %s state changed from %d to %d"), 
               *NPCName, (int32)PreviousState, (int32)CurrentState);
        
        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("CurrentState"), (uint8)CurrentState);
        }
    }
}

void UNPCBehaviorSystem::RememberActor(AActor* Actor, float ThreatLevel)
{
    if (!Actor) return;
    
    // Add to known actors if not already known
    if (!NPCMemory.KnownActors.Contains(Actor))
    {
        NPCMemory.KnownActors.Add(Actor);
    }
    
    // Update threat level
    NPCMemory.ActorThreatLevels.Add(Actor, ThreatLevel);
    
    // Special handling for player
    if (Actor->IsA<APawn>() && Cast<APawn>(Actor)->IsPlayerControlled())
    {
        NPCMemory.bPlayerIsKnown = true;
        NPCMemory.LastPlayerInteractionTime = GetWorld()->GetTimeSeconds();
    }
    
    UE_LOG(LogTemp, Log, TEXT("NPC %s remembered actor: %s (Threat: %f)"), 
           *NPCName, *Actor->GetName(), ThreatLevel);
}

void UNPCBehaviorSystem::ForgetActor(AActor* Actor)
{
    if (!Actor) return;
    
    NPCMemory.KnownActors.Remove(Actor);
    NPCMemory.ActorThreatLevels.Remove(Actor);
    
    UE_LOG(LogTemp, Log, TEXT("NPC %s forgot actor: %s"), *NPCName, *Actor->GetName());
}

bool UNPCBehaviorSystem::IsActorKnown(AActor* Actor) const
{
    return NPCMemory.KnownActors.Contains(Actor);
}

FNPCRoutine UNPCBehaviorSystem::GetCurrentRoutine() const
{
    float CurrentTime = GetCurrentTimeOfDay();
    
    for (const FNPCRoutine& Routine : DailyRoutines)
    {
        float EndTime = Routine.StartTime + Routine.Duration;
        
        // Handle routines that cross midnight
        if (EndTime > 24.0f)
        {
            if (CurrentTime >= Routine.StartTime || CurrentTime <= (EndTime - 24.0f))
            {
                return Routine;
            }
        }
        else
        {
            if (CurrentTime >= Routine.StartTime && CurrentTime <= EndTime)
            {
                return Routine;
            }
        }
    }
    
    // Return default idle routine if no routine matches
    FNPCRoutine DefaultRoutine;
    DefaultRoutine.Activity = ENPCState::Idle;
    DefaultRoutine.ActivityDescription = TEXT("Free Time");
    return DefaultRoutine;
}

void UNPCBehaviorSystem::AddRoutine(const FNPCRoutine& NewRoutine)
{
    DailyRoutines.Add(NewRoutine);
    
    // Sort routines by start time
    DailyRoutines.Sort([](const FNPCRoutine& A, const FNPCRoutine& B) {
        return A.StartTime < B.StartTime;
    });
}

void UNPCBehaviorSystem::UpdateRelationship(AActor* OtherActor, float RelationshipChange)
{
    if (!OtherActor) return;
    
    float* CurrentValue = RelationshipValues.Find(OtherActor);
    if (CurrentValue)
    {
        *CurrentValue = FMath::Clamp(*CurrentValue + RelationshipChange, -1.0f, 1.0f);
    }
    else
    {
        RelationshipValues.Add(OtherActor, FMath::Clamp(RelationshipChange, -1.0f, 1.0f));
    }
    
    UE_LOG(LogTemp, Log, TEXT("NPC %s relationship with %s changed by %f"), 
           *NPCName, *OtherActor->GetName(), RelationshipChange);
}

float UNPCBehaviorSystem::GetRelationshipValue(AActor* OtherActor) const
{
    if (const float* Value = RelationshipValues.Find(OtherActor))
    {
        return *Value;
    }
    return 0.0f; // Neutral relationship
}

void UNPCBehaviorSystem::StartBehaviorTree()
{
    if (NPCAIController && BehaviorTree)
    {
        // Set blackboard asset
        if (BlackboardAsset)
        {
            NPCAIController->GetBlackboardComponent()->InitializeBlackboard(*BlackboardAsset);
        }
        
        // Start behavior tree
        NPCAIController->RunBehaviorTree(BehaviorTree);
        
        UE_LOG(LogTemp, Log, TEXT("Started behavior tree for NPC: %s"), *NPCName);
    }
}

void UNPCBehaviorSystem::StopBehaviorTree()
{
    if (NPCAIController)
    {
        NPCAIController->GetBrainComponent()->StopLogic(TEXT("Manual Stop"));
        UE_LOG(LogTemp, Log, TEXT("Stopped behavior tree for NPC: %s"), *NPCName);
    }
}

void UNPCBehaviorSystem::UpdateBlackboardValues()
{
    if (!BlackboardComponent) return;
    
    // Update basic state information
    BlackboardComponent->SetValueAsEnum(TEXT("CurrentState"), (uint8)CurrentState);
    BlackboardComponent->SetValueAsEnum(TEXT("Personality"), (uint8)Personality);
    
    // Update routine information
    FNPCRoutine CurrentRoutine = GetCurrentRoutine();
    BlackboardComponent->SetValueAsVector(TEXT("RoutineLocation"), CurrentRoutine.TargetLocation);
    BlackboardComponent->SetValueAsEnum(TEXT("RoutineActivity"), (uint8)CurrentRoutine.Activity);
    
    // Update player information if known
    if (NPCMemory.bPlayerIsKnown)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            BlackboardComponent->SetValueAsObject(TEXT("PlayerPawn"), PlayerPawn);
            BlackboardComponent->SetValueAsVector(TEXT("PlayerLocation"), PlayerPawn->GetActorLocation());
            
            float TimeSinceLastInteraction = GetWorld()->GetTimeSeconds() - NPCMemory.LastPlayerInteractionTime;
            BlackboardComponent->SetValueAsFloat(TEXT("TimeSincePlayerInteraction"), TimeSinceLastInteraction);
        }
    }
    
    // Update threat information
    float HighestThreat = 0.0f;
    AActor* MostThreateningActor = nullptr;
    
    for (const auto& ThreatPair : NPCMemory.ActorThreatLevels)
    {
        if (ThreatPair.Value > HighestThreat && IsValid(ThreatPair.Key))
        {
            HighestThreat = ThreatPair.Value;
            MostThreateningActor = ThreatPair.Key;
        }
    }
    
    BlackboardComponent->SetValueAsFloat(TEXT("HighestThreatLevel"), HighestThreat);
    BlackboardComponent->SetValueAsObject(TEXT("MostThreateningActor"), MostThreateningActor);
}

void UNPCBehaviorSystem::UpdateDailyRoutine()
{
    RoutineUpdateTimer += GetWorld()->GetDeltaSeconds();
    
    // Check routine every 10 seconds
    if (RoutineUpdateTimer >= 10.0f)
    {
        RoutineUpdateTimer = 0.0f;
        
        FNPCRoutine CurrentRoutine = GetCurrentRoutine();
        
        // Update state based on routine
        if (CurrentState != CurrentRoutine.Activity)
        {
            SetNPCState(CurrentRoutine.Activity);
            
            UE_LOG(LogTemp, Log, TEXT("NPC %s starting routine: %s"), 
                   *NPCName, *CurrentRoutine.ActivityDescription);
        }
    }
}

void UNPCBehaviorSystem::ProcessMemory(float DeltaTime)
{
    MemoryCleanupTimer += DeltaTime;
    
    // Clean up memory every 30 seconds
    if (MemoryCleanupTimer >= 30.0f)
    {
        MemoryCleanupTimer = 0.0f;
        
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        // Remove invalid actors
        NPCMemory.KnownActors.RemoveAll([](AActor* Actor) {
            return !IsValid(Actor);
        });
        
        // Clean up threat levels for invalid actors
        TArray<AActor*> ActorsToRemove;
        for (const auto& ThreatPair : NPCMemory.ActorThreatLevels)
        {
            if (!IsValid(ThreatPair.Key))
            {
                ActorsToRemove.Add(ThreatPair.Key);
            }
        }
        
        for (AActor* Actor : ActorsToRemove)
        {
            NPCMemory.ActorThreatLevels.Remove(Actor);
        }
        
        // Forget old player interactions
        if (NPCMemory.bPlayerIsKnown && 
            (CurrentTime - NPCMemory.LastPlayerInteractionTime) > MemoryRetentionTime)
        {
            NPCMemory.bPlayerIsKnown = false;
            UE_LOG(LogTemp, Log, TEXT("NPC %s forgot about player due to time"), *NPCName);
        }
    }
}

void UNPCBehaviorSystem::HandlePerception()
{
    // This would integrate with UE5's AI Perception system
    // For now, we'll do basic distance-based perception
    
    if (!GetOwner()) return;
    
    FVector NPCLocation = GetOwner()->GetActorLocation();
    
    // Find nearby actors
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner()) continue;
        
        float Distance = FVector::Dist(NPCLocation, Actor->GetActorLocation());
        
        // Within sight range
        if (Distance <= SightRadius)
        {
            // Simple line of sight check would go here
            if (!IsActorKnown(Actor))
            {
                float ThreatLevel = 0.0f;
                
                // Determine threat level based on personality and actor type
                if (APawn* Pawn = Cast<APawn>(Actor))
                {
                    if (Pawn->IsPlayerControlled())
                    {
                        switch (Personality)
                        {
                            case ENPCPersonality::Aggressive:
                                ThreatLevel = 0.7f;
                                break;
                            case ENPCPersonality::Fearful:
                                ThreatLevel = 0.8f;
                                break;
                            case ENPCPersonality::Curious:
                                ThreatLevel = 0.1f;
                                break;
                            default:
                                ThreatLevel = 0.3f;
                                break;
                        }
                    }
                }
                
                RememberActor(Actor, ThreatLevel);
            }
        }
    }
}

float UNPCBehaviorSystem::GetCurrentTimeOfDay() const
{
    // This should integrate with the game's time system
    // For now, return a simple time based on world time
    if (UWorld* World = GetWorld())
    {
        float GameTime = World->GetTimeSeconds();
        // Convert to 24-hour format (1 real second = 1 game minute)
        return FMath::Fmod(GameTime / 60.0f, 24.0f);
    }
    return 12.0f; // Default to noon
}

void UNPCBehaviorSystem::SetupDefaultRoutines()
{
    // Morning routine
    FNPCRoutine MorningRoutine;
    MorningRoutine.StartTime = 6.0f;
    MorningRoutine.Duration = 2.0f;
    MorningRoutine.Activity = ENPCState::Eating;
    MorningRoutine.ActivityDescription = TEXT("Morning Meal");
    AddRoutine(MorningRoutine);
    
    // Work routine
    FNPCRoutine WorkRoutine;
    WorkRoutine.StartTime = 8.0f;
    WorkRoutine.Duration = 8.0f;
    WorkRoutine.Activity = ENPCState::Working;
    WorkRoutine.ActivityDescription = TEXT("Daily Work");
    AddRoutine(WorkRoutine);
    
    // Evening social time
    FNPCRoutine SocialRoutine;
    SocialRoutine.StartTime = 18.0f;
    SocialRoutine.Duration = 3.0f;
    SocialRoutine.Activity = ENPCState::Socializing;
    SocialRoutine.ActivityDescription = TEXT("Evening Gathering");
    AddRoutine(SocialRoutine);
    
    // Sleep routine
    FNPCRoutine SleepRoutine;
    SleepRoutine.StartTime = 22.0f;
    SleepRoutine.Duration = 8.0f;
    SleepRoutine.Activity = ENPCState::Sleeping;
    SleepRoutine.ActivityDescription = TEXT("Rest");
    AddRoutine(SleepRoutine);
    
    UE_LOG(LogTemp, Log, TEXT("Setup default routines for NPC: %s"), *NPCName);
}