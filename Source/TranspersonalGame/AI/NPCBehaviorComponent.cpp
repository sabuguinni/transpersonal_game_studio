#include "NPCBehaviorComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"

UNPC_BehaviorComponent::UNPC_BehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance

    // Initialize default values
    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    CurrentEmotion = ENPC_Emotion::Neutral;
    EmotionIntensity = 1.0f;
    NPCName = TEXT("Unnamed NPC");
    NPCRole = TEXT("Villager");

    // Default personality traits (can be randomized or set per NPC)
    Courage = 0.5f;
    Curiosity = 0.5f;
    Sociability = 0.5f;
    Aggression = 0.3f;
    Intelligence = 0.5f;

    // Memory system defaults
    MaxMemories = 20;
    MemoryDecayRate = 0.01f;

    // Schedule defaults
    CurrentTimeOfDay = 0.0f;
    bFollowDailySchedule = true;

    // Internal state
    LastEmotionChangeTime = 0.0f;
    LastBehaviorChangeTime = 0.0f;
    CurrentTarget = nullptr;
    HomeLocation = FVector::ZeroVector;
}

void UNPC_BehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Set home location to current position
    if (GetOwner())
    {
        HomeLocation = GetOwner()->GetActorLocation();
    }

    // Initialize default daily schedule if none exists
    if (DailySchedule.Num() == 0)
    {
        // Create a basic schedule: Sleep, Work, Socialize, Rest
        AddScheduledActivity(0.0f, 6.0f, ENPC_BehaviorState::Sleeping, HomeLocation, TEXT("Night rest"));
        AddScheduledActivity(6.0f, 12.0f, ENPC_BehaviorState::Working, HomeLocation + FVector(500, 0, 0), TEXT("Morning work"));
        AddScheduledActivity(12.0f, 14.0f, ENPC_BehaviorState::Eating, HomeLocation, TEXT("Midday meal"));
        AddScheduledActivity(14.0f, 18.0f, ENPC_BehaviorState::Socializing, HomeLocation + FVector(0, 500, 0), TEXT("Afternoon socializing"));
        AddScheduledActivity(18.0f, 20.0f, ENPC_BehaviorState::Eating, HomeLocation, TEXT("Evening meal"));
        AddScheduledActivity(20.0f, 24.0f, ENPC_BehaviorState::Idle, HomeLocation, TEXT("Evening rest"));
    }

    UE_LOG(LogTemp, Log, TEXT("NPC Behavior Component initialized for %s"), *NPCName);
}

void UNPC_BehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!GetOwner())
    {
        return;
    }

    // Update time tracking
    LastEmotionChangeTime += DeltaTime;
    LastBehaviorChangeTime += DeltaTime;

    // Process emotional decay
    ProcessEmotionalDecay(DeltaTime);

    // Update daily routine based on time
    if (bFollowDailySchedule)
    {
        // Get current time of day from world (simplified - using game time)
        CurrentTimeOfDay = FMath::Fmod(GetWorld()->GetTimeSeconds() / 3600.0f, 24.0f); // Convert to hours
        UpdateDailyRoutine(CurrentTimeOfDay);
    }

    // Check for nearby actors periodically
    if (LastBehaviorChangeTime > 2.0f) // Check every 2 seconds
    {
        CheckForNearbyActors();
        LastBehaviorChangeTime = 0.0f;
    }

    // Execute current behavior
    ExecuteCurrentBehavior();

    // Clean up old memories
    if (FMath::RandRange(0.0f, 1.0f) < 0.01f) // 1% chance per tick
    {
        ForgetOldMemories();
    }
}

void UNPC_BehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        ENPC_BehaviorState OldState = CurrentBehaviorState;
        CurrentBehaviorState = NewState;
        LastBehaviorChangeTime = 0.0f;

        UE_LOG(LogTemp, Log, TEXT("%s changed behavior from %d to %d"), 
               *NPCName, (int32)OldState, (int32)NewState);

        // Trigger emotional responses to behavior changes
        switch (NewState)
        {
        case ENPC_BehaviorState::Fleeing:
            SetEmotion(ENPC_Emotion::Fearful, 0.8f);
            break;
        case ENPC_BehaviorState::Socializing:
            if (Sociability > 0.6f)
            {
                SetEmotion(ENPC_Emotion::Happy, 0.6f);
            }
            break;
        case ENPC_BehaviorState::Working:
            SetEmotion(ENPC_Emotion::Neutral, 0.5f);
            break;
        case ENPC_BehaviorState::Investigating:
            if (Curiosity > 0.5f)
            {
                SetEmotion(ENPC_Emotion::Curious, 0.7f);
            }
            break;
        }
    }
}

void UNPC_BehaviorComponent::SetEmotion(ENPC_Emotion NewEmotion, float Intensity)
{
    CurrentEmotion = NewEmotion;
    EmotionIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    LastEmotionChangeTime = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("%s emotion changed to %d (intensity: %.2f)"), 
           *NPCName, (int32)NewEmotion, EmotionIntensity);
}

ENPC_BehaviorState UNPC_BehaviorComponent::GetCurrentBehaviorState() const
{
    return CurrentBehaviorState;
}

ENPC_Emotion UNPC_BehaviorComponent::GetCurrentEmotion() const
{
    return CurrentEmotion;
}

void UNPC_BehaviorComponent::AddMemory(AActor* Subject, ENPC_Emotion Emotion, ENPC_Relationship Relationship, const FString& Description)
{
    if (!Subject)
    {
        return;
    }

    // Check if memory already exists
    FNPC_Memory* ExistingMemory = FindMemory(Subject);
    if (ExistingMemory)
    {
        // Update existing memory
        ExistingMemory->LastSeenLocation = Subject->GetActorLocation();
        ExistingMemory->LastSeenTime = GetWorld()->GetTimeSeconds();
        ExistingMemory->AssociatedEmotion = Emotion;
        ExistingMemory->Relationship = Relationship;
        ExistingMemory->MemoryDescription = Description;
    }
    else
    {
        // Create new memory
        FNPC_Memory NewMemory;
        NewMemory.Subject = Subject;
        NewMemory.LastSeenLocation = Subject->GetActorLocation();
        NewMemory.LastSeenTime = GetWorld()->GetTimeSeconds();
        NewMemory.AssociatedEmotion = Emotion;
        NewMemory.Relationship = Relationship;
        NewMemory.TrustLevel = 0.0f;
        NewMemory.MemoryDescription = Description;

        Memories.Add(NewMemory);

        // Remove oldest memory if we exceed max
        if (Memories.Num() > MaxMemories)
        {
            Memories.RemoveAt(0);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("%s added memory of %s: %s"), 
           *NPCName, *Subject->GetName(), *Description);
}

FNPC_Memory* UNPC_BehaviorComponent::FindMemory(AActor* Subject)
{
    if (!Subject)
    {
        return nullptr;
    }

    for (FNPC_Memory& Memory : Memories)
    {
        if (Memory.Subject == Subject)
        {
            return &Memory;
        }
    }

    return nullptr;
}

void UNPC_BehaviorComponent::UpdateMemory(AActor* Subject, ENPC_Emotion Emotion, float TrustChange)
{
    FNPC_Memory* Memory = FindMemory(Subject);
    if (Memory)
    {
        Memory->AssociatedEmotion = Emotion;
        Memory->TrustLevel = FMath::Clamp(Memory->TrustLevel + TrustChange, -1.0f, 1.0f);
        Memory->LastSeenTime = GetWorld()->GetTimeSeconds();
        Memory->LastSeenLocation = Subject->GetActorLocation();

        UE_LOG(LogTemp, Log, TEXT("%s updated memory of %s (trust: %.2f)"), 
               *NPCName, *Subject->GetName(), Memory->TrustLevel);
    }
}

void UNPC_BehaviorComponent::ForgetOldMemories()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = Memories.Num() - 1; i >= 0; i--)
    {
        float MemoryAge = CurrentTime - Memories[i].LastSeenTime;
        float ForgetChance = MemoryAge * MemoryDecayRate;
        
        if (FMath::RandRange(0.0f, 1.0f) < ForgetChance)
        {
            UE_LOG(LogTemp, Log, TEXT("%s forgot memory of %s"), 
                   *NPCName, Memories[i].Subject ? *Memories[i].Subject->GetName() : TEXT("Unknown"));
            Memories.RemoveAt(i);
        }
    }
}

void UNPC_BehaviorComponent::ReactToActor(AActor* OtherActor)
{
    if (!OtherActor || OtherActor == GetOwner())
    {
        return;
    }

    // Find existing memory or create impression
    FNPC_Memory* Memory = FindMemory(OtherActor);
    ENPC_Relationship Relationship = Memory ? Memory->Relationship : ENPC_Relationship::Stranger;

    // React based on personality and relationship
    float DistanceToActor = FVector::Dist(GetOwner()->GetActorLocation(), OtherActor->GetActorLocation());

    if (DistanceToActor < 500.0f) // Close proximity
    {
        switch (Relationship)
        {
        case ENPC_Relationship::Friend:
        case ENPC_Relationship::Ally:
            SetEmotion(ENPC_Emotion::Happy, 0.6f);
            if (CurrentBehaviorState == ENPC_BehaviorState::Idle)
            {
                SetBehaviorState(ENPC_BehaviorState::Socializing);
            }
            break;

        case ENPC_Relationship::Enemy:
            SetEmotion(ENPC_Emotion::Angry, 0.8f);
            if (Courage > 0.6f)
            {
                SetBehaviorState(ENPC_BehaviorState::Investigating);
            }
            else
            {
                SetBehaviorState(ENPC_BehaviorState::Fleeing);
            }
            break;

        case ENPC_Relationship::Stranger:
        case ENPC_Relationship::Unknown:
            if (Curiosity > 0.5f)
            {
                SetEmotion(ENPC_Emotion::Curious, 0.5f);
                SetBehaviorState(ENPC_BehaviorState::Investigating);
            }
            else if (Courage < 0.4f)
            {
                SetEmotion(ENPC_Emotion::Suspicious, 0.4f);
            }
            break;
        }

        // Add or update memory
        if (!Memory)
        {
            AddMemory(OtherActor, CurrentEmotion, ENPC_Relationship::Stranger, TEXT("First encounter"));
        }
        else
        {
            UpdateMemory(OtherActor, CurrentEmotion, 0.0f);
        }
    }
}

ENPC_Relationship UNPC_BehaviorComponent::GetRelationshipWith(AActor* OtherActor)
{
    FNPC_Memory* Memory = FindMemory(OtherActor);
    return Memory ? Memory->Relationship : ENPC_Relationship::Unknown;
}

void UNPC_BehaviorComponent::StartConversation(AActor* OtherActor)
{
    if (!OtherActor)
    {
        return;
    }

    SetBehaviorState(ENPC_BehaviorState::Socializing);
    CurrentTarget = OtherActor;

    ENPC_Relationship Relationship = GetRelationshipWith(OtherActor);
    
    // Adjust emotion based on relationship
    switch (Relationship)
    {
    case ENPC_Relationship::Friend:
        SetEmotion(ENPC_Emotion::Happy, 0.7f);
        break;
    case ENPC_Relationship::Enemy:
        SetEmotion(ENPC_Emotion::Angry, 0.6f);
        break;
    default:
        SetEmotion(ENPC_Emotion::Curious, 0.5f);
        break;
    }

    UE_LOG(LogTemp, Log, TEXT("%s started conversation with %s"), 
           *NPCName, *OtherActor->GetName());
}

void UNPC_BehaviorComponent::UpdateDailyRoutine(float TimeOfDay)
{
    FNPC_DailySchedule* CurrentActivity = GetCurrentScheduledActivity();
    
    if (CurrentActivity && CurrentActivity->Activity != CurrentBehaviorState)
    {
        // Only change behavior if not in a high-priority state
        if (CurrentBehaviorState != ENPC_BehaviorState::Fleeing && 
            CurrentBehaviorState != ENPC_BehaviorState::Investigating)
        {
            SetBehaviorState(CurrentActivity->Activity);
            
            UE_LOG(LogTemp, Log, TEXT("%s following schedule: %s at %.1f"), 
                   *NPCName, *CurrentActivity->ActivityDescription, TimeOfDay);
        }
    }
}

FNPC_DailySchedule* UNPC_BehaviorComponent::GetCurrentScheduledActivity()
{
    for (FNPC_DailySchedule& Schedule : DailySchedule)
    {
        if (CurrentTimeOfDay >= Schedule.StartTime && CurrentTimeOfDay < Schedule.EndTime)
        {
            return &Schedule;
        }
    }
    
    return nullptr;
}

void UNPC_BehaviorComponent::AddScheduledActivity(float StartTime, float EndTime, ENPC_BehaviorState Activity, FVector Location, const FString& Description)
{
    FNPC_DailySchedule NewActivity;
    NewActivity.StartTime = StartTime;
    NewActivity.EndTime = EndTime;
    NewActivity.Activity = Activity;
    NewActivity.ActivityLocation = Location;
    NewActivity.ActivityDescription = Description;
    
    DailySchedule.Add(NewActivity);
    
    // Sort schedule by start time
    DailySchedule.Sort([](const FNPC_DailySchedule& A, const FNPC_DailySchedule& B) {
        return A.StartTime < B.StartTime;
    });
}

void UNPC_BehaviorComponent::ProcessEmotionalDecay(float DeltaTime)
{
    // Emotions gradually return to neutral over time
    if (CurrentEmotion != ENPC_Emotion::Neutral && LastEmotionChangeTime > 10.0f)
    {
        EmotionIntensity = FMath::Max(0.0f, EmotionIntensity - (DeltaTime * 0.1f));
        
        if (EmotionIntensity <= 0.1f)
        {
            SetEmotion(ENPC_Emotion::Neutral, 0.0f);
        }
    }
}

void UNPC_BehaviorComponent::CheckForNearbyActors()
{
    if (!GetOwner())
    {
        return;
    }

    // Find nearby actors
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor != GetOwner())
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
            
            // React to actors within perception range
            if (Distance < 1000.0f) // 10 meter perception range
            {
                ReactToActor(Actor);
                break; // Only react to one actor per check for performance
            }
        }
    }
}

void UNPC_BehaviorComponent::ExecuteCurrentBehavior()
{
    if (!GetOwner())
    {
        return;
    }

    // Simple behavior execution - in a full implementation, this would drive movement and animations
    switch (CurrentBehaviorState)
    {
    case ENPC_BehaviorState::Patrolling:
        // Move towards patrol points
        break;
        
    case ENPC_BehaviorState::Fleeing:
        // Move away from threats
        if (CurrentTarget)
        {
            FVector FleeDirection = (GetOwner()->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
            // In full implementation, would move the actor
        }
        break;
        
    case ENPC_BehaviorState::Investigating:
        // Move towards points of interest
        if (CurrentTarget)
        {
            FVector InvestigateDirection = (CurrentTarget->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
            // In full implementation, would move the actor
        }
        break;
        
    case ENPC_BehaviorState::Socializing:
        // Face towards social target
        if (CurrentTarget)
        {
            FVector LookDirection = (CurrentTarget->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
            // In full implementation, would rotate the actor
        }
        break;
        
    default:
        // Idle or other states - minimal processing
        break;
    }
}