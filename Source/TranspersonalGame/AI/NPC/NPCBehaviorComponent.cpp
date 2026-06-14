#include "NPCBehaviorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/PrimitiveComponent.h"

UNPC_BehaviorComponent::UNPC_BehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize default state
    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    CurrentEmotionalState = ENPC_EmotionalState::Calm;
    StateChangeTime = 0.0f;

    // Memory system defaults
    MaxMemories = 50;
    MemoryDecayRate = 0.1f;
    CurrentRoutineIndex = -1;

    // Behavior parameters
    PersonalSpace = 200.0f;
    AlertRadius = 1500.0f;
    FleeThreshold = 0.7f;
    AggressionLevel = 0.3f;
    CuriosityLevel = 0.5f;
}

void UNPC_BehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Set up timers
    GetWorld()->GetTimerManager().SetTimer(RoutineUpdateTimer, this, &UNPC_BehaviorComponent::UpdateRoutine, 30.0f, true);
    GetWorld()->GetTimerManager().SetTimer(MemoryCleanupTimer, this, &UNPC_BehaviorComponent::CleanupMemories, 60.0f, true);
    GetWorld()->GetTimerManager().SetTimer(BehaviorUpdateTimer, this, &UNPC_BehaviorComponent::UpdateBehavior, 1.0f, true);

    // Initialize routine
    UpdateRoutine();
}

void UNPC_BehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update state change time
    StateChangeTime += DeltaTime;

    // Check for player proximity
    if (IsPlayerNearby(AlertRadius))
    {
        AActor* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (Player)
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
            ReactToPlayer(Player, Distance);
        }
    }
}

void UNPC_BehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentBehaviorState != NewState && ShouldChangeState(NewState))
    {
        ENPC_BehaviorState PreviousState = CurrentBehaviorState;
        CurrentBehaviorState = NewState;
        StateChangeTime = 0.0f;

        // Add memory of state change
        AddMemory(GetOwner()->GetActorLocation(), GetOwner(), 
                 FString::Printf(TEXT("StateChange_%s_to_%s"), 
                 *UEnum::GetValueAsString(PreviousState), 
                 *UEnum::GetValueAsString(NewState)), 0.5f);

        UE_LOG(LogTemp, Log, TEXT("%s: Changed behavior state from %s to %s"), 
               *GetOwner()->GetName(),
               *UEnum::GetValueAsString(PreviousState),
               *UEnum::GetValueAsString(NewState));
    }
}

void UNPC_BehaviorComponent::SetEmotionalState(ENPC_EmotionalState NewState)
{
    if (CurrentEmotionalState != NewState)
    {
        CurrentEmotionalState = NewState;
        
        // Emotional state affects behavior
        switch (NewState)
        {
            case ENPC_EmotionalState::Fearful:
                if (CurrentBehaviorState != ENPC_BehaviorState::Fleeing)
                {
                    SetBehaviorState(ENPC_BehaviorState::Fleeing);
                }
                break;
            case ENPC_EmotionalState::Aggressive:
                if (CurrentBehaviorState == ENPC_BehaviorState::Idle)
                {
                    SetBehaviorState(ENPC_BehaviorState::Territorial);
                }
                break;
            case ENPC_EmotionalState::Curious:
                if (CurrentBehaviorState == ENPC_BehaviorState::Idle)
                {
                    SetBehaviorState(ENPC_BehaviorState::Investigating);
                }
                break;
        }
    }
}

void UNPC_BehaviorComponent::AddMemory(const FVector& Location, AActor* Actor, const FString& EventType, float Importance)
{
    FNPC_MemoryEntry NewMemory;
    NewMemory.Location = Location;
    NewMemory.Actor = Actor;
    NewMemory.EventType = EventType;
    NewMemory.Importance = Importance;
    NewMemory.Timestamp = GetWorld()->GetTimeSeconds();

    Memories.Add(NewMemory);

    // Remove oldest memories if we exceed the limit
    if (Memories.Num() > MaxMemories)
    {
        // Sort by importance and timestamp, remove least important/oldest
        Memories.Sort([](const FNPC_MemoryEntry& A, const FNPC_MemoryEntry& B) {
            if (FMath::IsNearlyEqual(A.Importance, B.Importance))
            {
                return A.Timestamp > B.Timestamp; // Newer first if same importance
            }
            return A.Importance > B.Importance; // More important first
        });

        Memories.RemoveAt(Memories.Num() - 1);
    }
}

void UNPC_BehaviorComponent::ForgetOldMemories(float MaxAge)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    Memories.RemoveAll([CurrentTime, MaxAge](const FNPC_MemoryEntry& Memory) {
        return (CurrentTime - Memory.Timestamp) > MaxAge;
    });
}

TArray<FNPC_MemoryEntry> UNPC_BehaviorComponent::GetMemoriesOfType(const FString& EventType) const
{
    TArray<FNPC_MemoryEntry> FilteredMemories;
    
    for (const FNPC_MemoryEntry& Memory : Memories)
    {
        if (Memory.EventType.Contains(EventType))
        {
            FilteredMemories.Add(Memory);
        }
    }
    
    return FilteredMemories;
}

FNPC_MemoryEntry UNPC_BehaviorComponent::GetMostImportantMemory() const
{
    if (Memories.Num() == 0)
    {
        return FNPC_MemoryEntry();
    }

    const FNPC_MemoryEntry* MostImportant = &Memories[0];
    for (const FNPC_MemoryEntry& Memory : Memories)
    {
        if (Memory.Importance > MostImportant->Importance)
        {
            MostImportant = &Memory;
        }
    }

    return *MostImportant;
}

void UNPC_BehaviorComponent::AddRoutineActivity(float StartHour, float EndHour, ENPC_BehaviorState Activity, const FVector& Location, float Priority)
{
    FNPC_DailyRoutine NewRoutine;
    NewRoutine.StartHour = StartHour;
    NewRoutine.EndHour = EndHour;
    NewRoutine.Activity = Activity;
    NewRoutine.TargetLocation = Location;
    NewRoutine.Priority = Priority;

    DailyRoutines.Add(NewRoutine);

    // Sort routines by start time
    DailyRoutines.Sort([](const FNPC_DailyRoutine& A, const FNPC_DailyRoutine& B) {
        return A.StartHour < B.StartHour;
    });
}

FNPC_DailyRoutine UNPC_BehaviorComponent::GetCurrentRoutineActivity() const
{
    float CurrentTime = GetCurrentTimeOfDay();
    
    for (const FNPC_DailyRoutine& Routine : DailyRoutines)
    {
        if (CurrentTime >= Routine.StartHour && CurrentTime <= Routine.EndHour)
        {
            return Routine;
        }
    }
    
    // Return default routine if none found
    FNPC_DailyRoutine DefaultRoutine;
    DefaultRoutine.Activity = ENPC_BehaviorState::Idle;
    DefaultRoutine.TargetLocation = GetOwner()->GetActorLocation();
    return DefaultRoutine;
}

void UNPC_BehaviorComponent::UpdateRoutine()
{
    FNPC_DailyRoutine CurrentRoutine = GetCurrentRoutineActivity();
    
    // Only change behavior if it's not overridden by higher priority states
    if (CurrentBehaviorState == ENPC_BehaviorState::Idle || 
        CurrentBehaviorState == ENPC_BehaviorState::Patrolling ||
        CurrentBehaviorState == ENPC_BehaviorState::Resting)
    {
        SetBehaviorState(CurrentRoutine.Activity);
    }
}

void UNPC_BehaviorComponent::ReactToPlayer(AActor* Player, float Distance)
{
    if (!Player) return;

    // Add memory of player encounter
    AddMemory(Player->GetActorLocation(), Player, TEXT("PlayerEncounter"), 
             FMath::Clamp(1.0f - (Distance / AlertRadius), 0.1f, 1.0f));

    if (Distance < PersonalSpace)
    {
        // Player is too close - react based on personality
        if (AggressionLevel > 0.6f)
        {
            SetEmotionalState(ENPC_EmotionalState::Aggressive);
            SetBehaviorState(ENPC_BehaviorState::Territorial);
        }
        else if (AggressionLevel < 0.3f)
        {
            SetEmotionalState(ENPC_EmotionalState::Fearful);
            SetBehaviorState(ENPC_BehaviorState::Fleeing);
        }
        else
        {
            SetEmotionalState(ENPC_EmotionalState::Alert);
            SetBehaviorState(ENPC_BehaviorState::Investigating);
        }
    }
    else if (Distance < AlertRadius * 0.5f)
    {
        // Player is nearby - become alert
        if (CurrentEmotionalState == ENPC_EmotionalState::Calm)
        {
            SetEmotionalState(ENPC_EmotionalState::Alert);
        }
        
        if (CuriosityLevel > 0.5f && CurrentBehaviorState == ENPC_BehaviorState::Idle)
        {
            SetBehaviorState(ENPC_BehaviorState::Investigating);
        }
    }
}

void UNPC_BehaviorComponent::ReactToThreat(AActor* Threat, float ThreatLevel)
{
    if (!Threat) return;

    AddMemory(Threat->GetActorLocation(), Threat, TEXT("Threat"), ThreatLevel);

    if (ThreatLevel > FleeThreshold)
    {
        SetEmotionalState(ENPC_EmotionalState::Fearful);
        SetBehaviorState(ENPC_BehaviorState::Fleeing);
    }
    else if (ThreatLevel > 0.4f)
    {
        SetEmotionalState(ENPC_EmotionalState::Alert);
        SetBehaviorState(ENPC_BehaviorState::Investigating);
    }
}

void UNPC_BehaviorComponent::ReactToNoise(const FVector& NoiseLocation, float NoiseLevel)
{
    AddMemory(NoiseLocation, nullptr, TEXT("Noise"), NoiseLevel);

    if (NoiseLevel > 0.7f)
    {
        SetEmotionalState(ENPC_EmotionalState::Alert);
        if (CurrentBehaviorState == ENPC_BehaviorState::Idle || CurrentBehaviorState == ENPC_BehaviorState::Resting)
        {
            SetBehaviorState(ENPC_BehaviorState::Investigating);
        }
    }
}

void UNPC_BehaviorComponent::InteractWithNPC(UNPC_BehaviorComponent* OtherNPC)
{
    if (!OtherNPC || !OtherNPC->GetOwner()) return;

    AActor* OtherActor = OtherNPC->GetOwner();
    float CurrentRelationship = GetRelationshipValue(OtherActor);

    // Positive interaction increases relationship
    ModifyRelationship(OtherActor, 0.1f);

    // Change to social behavior if appropriate
    if (CurrentRelationship > 0.5f && 
        (CurrentBehaviorState == ENPC_BehaviorState::Idle || CurrentBehaviorState == ENPC_BehaviorState::Patrolling))
    {
        SetBehaviorState(ENPC_BehaviorState::Socializing);
    }

    AddMemory(OtherActor->GetActorLocation(), OtherActor, TEXT("SocialInteraction"), 0.6f);
}

float UNPC_BehaviorComponent::GetRelationshipValue(AActor* OtherActor) const
{
    if (!OtherActor) return 0.0f;

    const float* RelationshipPtr = Relationships.Find(OtherActor);
    return RelationshipPtr ? *RelationshipPtr : 0.0f;
}

void UNPC_BehaviorComponent::ModifyRelationship(AActor* OtherActor, float Delta)
{
    if (!OtherActor) return;

    float CurrentValue = GetRelationshipValue(OtherActor);
    float NewValue = FMath::Clamp(CurrentValue + Delta, -1.0f, 1.0f);
    
    Relationships.Add(OtherActor, NewValue);
}

float UNPC_BehaviorComponent::GetCurrentTimeOfDay() const
{
    // Simple time system - could be replaced with proper day/night cycle
    float GameTime = GetWorld()->GetTimeSeconds();
    float DayLength = 1200.0f; // 20 minutes = 1 day
    float TimeInDay = FMath::Fmod(GameTime, DayLength);
    return (TimeInDay / DayLength) * 24.0f; // Convert to 0-24 hour format
}

bool UNPC_BehaviorComponent::IsPlayerNearby(float Radius) const
{
    AActor* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!Player) return false;

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    return Distance <= Radius;
}

AActor* UNPC_BehaviorComponent::FindNearestActorOfClass(TSubclassOf<AActor> ActorClass, float MaxDistance) const
{
    if (!ActorClass) return nullptr;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ActorClass, FoundActors);

    AActor* NearestActor = nullptr;
    float NearestDistance = MaxDistance;

    FVector MyLocation = GetOwner()->GetActorLocation();

    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner()) continue; // Skip self

        float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestActor = Actor;
        }
    }

    return NearestActor;
}

void UNPC_BehaviorComponent::UpdateBehavior()
{
    // Decay emotional states over time
    if (StateChangeTime > 10.0f) // After 10 seconds, start calming down
    {
        switch (CurrentEmotionalState)
        {
            case ENPC_EmotionalState::Alert:
                SetEmotionalState(ENPC_EmotionalState::Calm);
                break;
            case ENPC_EmotionalState::Fearful:
                if (StateChangeTime > 20.0f)
                {
                    SetEmotionalState(ENPC_EmotionalState::Alert);
                }
                break;
            case ENPC_EmotionalState::Aggressive:
                if (StateChangeTime > 15.0f)
                {
                    SetEmotionalState(ENPC_EmotionalState::Alert);
                }
                break;
        }
    }

    // Return to routine behavior if no overrides
    if (StateChangeTime > 30.0f && 
        (CurrentBehaviorState == ENPC_BehaviorState::Investigating || 
         CurrentBehaviorState == ENPC_BehaviorState::Territorial))
    {
        UpdateRoutine();
    }
}

void UNPC_BehaviorComponent::CleanupMemories()
{
    ForgetOldMemories(300.0f); // Forget memories older than 5 minutes

    // Decay memory importance over time
    float CurrentTime = GetWorld()->GetTimeSeconds();
    for (FNPC_MemoryEntry& Memory : Memories)
    {
        float Age = CurrentTime - Memory.Timestamp;
        Memory.Importance = FMath::Max(0.1f, Memory.Importance - (Age * MemoryDecayRate * 0.01f));
    }
}

bool UNPC_BehaviorComponent::ShouldChangeState(ENPC_BehaviorState NewState) const
{
    float NewStatePriority = CalculateStatePriority(NewState);
    float CurrentStatePriority = CalculateStatePriority(CurrentBehaviorState);

    // Allow state change if new state has higher priority or enough time has passed
    return NewStatePriority > CurrentStatePriority || StateChangeTime > 5.0f;
}

float UNPC_BehaviorComponent::CalculateStatePriority(ENPC_BehaviorState State) const
{
    switch (State)
    {
        case ENPC_BehaviorState::Fleeing: return 10.0f;
        case ENPC_BehaviorState::Hunting: return 8.0f;
        case ENPC_BehaviorState::Territorial: return 7.0f;
        case ENPC_BehaviorState::Investigating: return 5.0f;
        case ENPC_BehaviorState::Feeding: return 4.0f;
        case ENPC_BehaviorState::Socializing: return 3.0f;
        case ENPC_BehaviorState::Patrolling: return 2.0f;
        case ENPC_BehaviorState::Resting: return 1.5f;
        case ENPC_BehaviorState::Idle: return 1.0f;
        default: return 1.0f;
    }
}