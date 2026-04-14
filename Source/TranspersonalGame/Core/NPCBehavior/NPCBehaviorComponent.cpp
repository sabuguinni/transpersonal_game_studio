#include "NPCBehaviorComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Default values
    NPCName = TEXT("Unnamed NPC");
    Personality = ENPC_Personality::Friendly;
    CurrentState = ENPC_BehaviorState::Idle;
    AwarenessRadius = 1000.0f;
    InteractionRadius = 200.0f;
    MovementSpeed = 300.0f;
    SocialRadius = 500.0f;
    
    MaxMemories = 20;
    MemoryDecayRate = 0.1f;
    bFollowSchedule = true;
    bCanTrade = false;
    
    StateTimer = 0.0f;
    CurrentTarget = nullptr;
    HomeLocation = FVector::ZeroVector;
    bIsPlayerNearby = false;
    LastPlayerInteractionTime = 0.0f;

    // Initialize default daily schedule
    FNPC_DailySchedule MorningWork;
    MorningWork.StartHour = 8.0f;
    MorningWork.EndHour = 12.0f;
    MorningWork.Activity = ENPC_BehaviorState::Working;
    MorningWork.ActivityDescription = TEXT("Morning Work");
    DailySchedule.Add(MorningWork);

    FNPC_DailySchedule Lunch;
    Lunch.StartHour = 12.0f;
    Lunch.EndHour = 13.0f;
    Lunch.Activity = ENPC_BehaviorState::Eating;
    Lunch.ActivityDescription = TEXT("Lunch Break");
    DailySchedule.Add(Lunch);

    FNPC_DailySchedule AfternoonWork;
    AfternoonWork.StartHour = 13.0f;
    AfternoonWork.EndHour = 18.0f;
    AfternoonWork.Activity = ENPC_BehaviorState::Working;
    AfternoonWork.ActivityDescription = TEXT("Afternoon Work");
    DailySchedule.Add(AfternoonWork);

    FNPC_DailySchedule Evening;
    Evening.StartHour = 18.0f;
    Evening.EndHour = 22.0f;
    Evening.Activity = ENPC_BehaviorState::Socializing;
    Evening.ActivityDescription = TEXT("Evening Social Time");
    DailySchedule.Add(Evening);

    FNPC_DailySchedule Sleep;
    Sleep.StartHour = 22.0f;
    Sleep.EndHour = 8.0f;
    Sleep.Activity = ENPC_BehaviorState::Sleeping;
    Sleep.ActivityDescription = TEXT("Sleep");
    DailySchedule.Add(Sleep);

    // Default dialogues
    KnownDialogues.Add(TEXT("Greetings, traveler."));
    KnownDialogues.Add(TEXT("The weather has been strange lately."));
    KnownDialogues.Add(TEXT("Have you seen any unusual creatures around?"));
    KnownDialogues.Add(TEXT("The spirits seem restless today."));
    KnownDialogues.Add(TEXT("Safe travels on your journey."));
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Store home location
    if (GetOwner())
    {
        HomeLocation = GetOwner()->GetActorLocation();
    }

    // Start daily routine timer
    if (bFollowSchedule)
    {
        StartDailyRoutine();
    }

    // Start memory decay timer
    GetWorld()->GetTimerManager().SetTimer(
        MemoryDecayTimerHandle,
        this,
        &UNPCBehaviorComponent::DecayMemories,
        30.0f, // Every 30 seconds
        true
    );
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimer += DeltaTime;
    
    // Process nearby actors for awareness
    ProcessNearbyActors();
    
    // Update current activity based on schedule
    if (bFollowSchedule)
    {
        UpdateCurrentActivity();
    }
    
    // Handle player interaction if nearby
    if (bIsPlayerNearby)
    {
        HandlePlayerInteraction();
    }
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;
        
        // Log state change for debugging
        if (GEngine)
        {
            FString StateString = UEnum::GetValueAsString(NewState);
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, 
                FString::Printf(TEXT("%s changed state to %s"), *NPCName, *StateString));
        }
    }
}

void UNPCBehaviorComponent::AddMemory(AActor* Actor, float Relationship, const FString& Interaction)
{
    if (!Actor) return;

    // Check if we already have a memory of this actor
    FNPC_Memory* ExistingMemory = GetMemoryOfActor(Actor);
    
    if (ExistingMemory)
    {
        // Update existing memory
        ExistingMemory->LastKnownLocation = Actor->GetActorLocation();
        ExistingMemory->Relationship = FMath::Clamp(ExistingMemory->Relationship + Relationship, -100.0f, 100.0f);
        ExistingMemory->LastSeenTime = GetWorld()->GetTimeSeconds();
        ExistingMemory->LastInteraction = Interaction;
    }
    else
    {
        // Create new memory
        FNPC_Memory NewMemory;
        NewMemory.Actor = Actor;
        NewMemory.LastKnownLocation = Actor->GetActorLocation();
        NewMemory.Relationship = FMath::Clamp(Relationship, -100.0f, 100.0f);
        NewMemory.LastSeenTime = GetWorld()->GetTimeSeconds();
        NewMemory.LastInteraction = Interaction;
        
        MemoryBank.Add(NewMemory);
        
        // Remove oldest memory if we exceed max
        if (MemoryBank.Num() > MaxMemories)
        {
            MemoryBank.RemoveAt(0);
        }
    }
}

FNPC_Memory* UNPCBehaviorComponent::GetMemoryOfActor(AActor* Actor)
{
    for (FNPC_Memory& Memory : MemoryBank)
    {
        if (Memory.Actor == Actor)
        {
            return &Memory;
        }
    }
    return nullptr;
}

void UNPCBehaviorComponent::UpdateRelationshipWithActor(AActor* Actor, float RelationshipChange)
{
    FNPC_Memory* Memory = GetMemoryOfActor(Actor);
    if (Memory)
    {
        Memory->Relationship = FMath::Clamp(Memory->Relationship + RelationshipChange, -100.0f, 100.0f);
    }
    else
    {
        AddMemory(Actor, RelationshipChange, TEXT("Relationship update"));
    }
}

TArray<AActor*> UNPCBehaviorComponent::GetNearbyActors()
{
    TArray<AActor*> NearbyActors;
    
    if (!GetOwner()) return NearbyActors;

    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Get all actors in the world
    UWorld* World = GetWorld();
    if (!World) return NearbyActors;

    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor && Actor != GetOwner())
        {
            float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
            if (Distance <= AwarenessRadius)
            {
                NearbyActors.Add(Actor);
            }
        }
    }
    
    return NearbyActors;
}

void UNPCBehaviorComponent::StartDailyRoutine()
{
    GetWorld()->GetTimerManager().SetTimer(
        ScheduleTimerHandle,
        this,
        &UNPCBehaviorComponent::UpdateCurrentActivity,
        60.0f, // Check every minute
        true
    );
}

FNPC_DailySchedule UNPCBehaviorComponent::GetCurrentScheduleActivity()
{
    float CurrentHour = GetCurrentGameHour();
    
    for (const FNPC_DailySchedule& Schedule : DailySchedule)
    {
        if (Schedule.StartHour <= Schedule.EndHour)
        {
            // Normal time range (e.g., 8-12)
            if (CurrentHour >= Schedule.StartHour && CurrentHour < Schedule.EndHour)
            {
                return Schedule;
            }
        }
        else
        {
            // Overnight time range (e.g., 22-8)
            if (CurrentHour >= Schedule.StartHour || CurrentHour < Schedule.EndHour)
            {
                return Schedule;
            }
        }
    }
    
    // Default to idle if no schedule matches
    FNPC_DailySchedule DefaultSchedule;
    DefaultSchedule.Activity = ENPC_BehaviorState::Idle;
    return DefaultSchedule;
}

void UNPCBehaviorComponent::ReactToPlayer(AActor* Player)
{
    if (!Player) return;

    FNPC_Memory* PlayerMemory = GetMemoryOfActor(Player);
    float Relationship = PlayerMemory ? PlayerMemory->Relationship : 0.0f;

    // React based on personality and relationship
    switch (Personality)
    {
        case ENPC_Personality::Friendly:
            if (Relationship >= 0.0f)
            {
                SetBehaviorState(ENPC_BehaviorState::Socializing);
            }
            break;
            
        case ENPC_Personality::Cautious:
            if (Relationship < -20.0f)
            {
                SetBehaviorState(ENPC_BehaviorState::Fleeing);
            }
            else if (Relationship > 20.0f)
            {
                SetBehaviorState(ENPC_BehaviorState::Socializing);
            }
            else
            {
                SetBehaviorState(ENPC_BehaviorState::Investigating);
            }
            break;
            
        case ENPC_Personality::Aggressive:
            if (Relationship < -10.0f)
            {
                SetBehaviorState(ENPC_BehaviorState::Investigating);
            }
            break;
            
        case ENPC_Personality::Curious:
            SetBehaviorState(ENPC_BehaviorState::Investigating);
            break;
            
        case ENPC_Personality::Fearful:
            if (Relationship < 50.0f)
            {
                SetBehaviorState(ENPC_BehaviorState::Fleeing);
            }
            break;
            
        default:
            SetBehaviorState(ENPC_BehaviorState::Investigating);
            break;
    }

    // Add or update memory of this interaction
    AddMemory(Player, 1.0f, TEXT("Player encounter"));
}

FString UNPCBehaviorComponent::GetRandomDialogue()
{
    if (KnownDialogues.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, KnownDialogues.Num() - 1);
        return KnownDialogues[RandomIndex];
    }
    return TEXT("...");
}

void UNPCBehaviorComponent::InitiateTrade(AActor* Trader)
{
    if (!bCanTrade || !Trader) return;

    SetBehaviorState(ENPC_BehaviorState::Trading);
    CurrentTarget = Trader;
    
    // Add positive relationship for trading
    AddMemory(Trader, 5.0f, TEXT("Trade initiated"));
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
            FString::Printf(TEXT("%s is ready to trade!"), *NPCName));
    }
}

void UNPCBehaviorComponent::UpdateCurrentActivity()
{
    if (!bFollowSchedule) return;

    FNPC_DailySchedule CurrentSchedule = GetCurrentScheduleActivity();
    
    if (CurrentState != CurrentSchedule.Activity)
    {
        SetBehaviorState(CurrentSchedule.Activity);
        
        // Move to target location if specified
        if (CurrentSchedule.TargetLocation != FVector::ZeroVector)
        {
            MoveToTarget(CurrentSchedule.TargetLocation);
        }
        else
        {
            // Use home location as default
            MoveToTarget(HomeLocation);
        }
    }
}

void UNPCBehaviorComponent::DecayMemories()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = MemoryBank.Num() - 1; i >= 0; i--)
    {
        FNPC_Memory& Memory = MemoryBank[i];
        
        // Check if actor still exists
        if (!IsValid(Memory.Actor))
        {
            MemoryBank.RemoveAt(i);
            continue;
        }
        
        // Decay relationship over time
        float TimeSinceLastSeen = CurrentTime - Memory.LastSeenTime;
        if (TimeSinceLastSeen > 300.0f) // 5 minutes
        {
            Memory.Relationship *= (1.0f - MemoryDecayRate);
            
            // Remove very old or weak memories
            if (FMath::Abs(Memory.Relationship) < 1.0f && TimeSinceLastSeen > 1800.0f) // 30 minutes
            {
                MemoryBank.RemoveAt(i);
            }
        }
    }
}

void UNPCBehaviorComponent::ProcessNearbyActors()
{
    TArray<AActor*> NearbyActors = GetNearbyActors();
    bIsPlayerNearby = false;
    
    for (AActor* Actor : NearbyActors)
    {
        // Check if this is the player
        if (Actor && Actor->IsA<APawn>())
        {
            APawn* Pawn = Cast<APawn>(Actor);
            if (Pawn && Pawn->IsPlayerControlled())
            {
                float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
                if (Distance <= InteractionRadius)
                {
                    bIsPlayerNearby = true;
                }
                
                // Update memory of seeing the player
                AddMemory(Actor, 0.5f, TEXT("Player spotted"));
            }
        }
    }
}

void UNPCBehaviorComponent::HandlePlayerInteraction()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Avoid spamming interactions
    if (CurrentTime - LastPlayerInteractionTime < 5.0f) return;
    
    // Find the player
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        ReactToPlayer(PlayerPawn);
        LastPlayerInteractionTime = CurrentTime;
    }
}

void UNPCBehaviorComponent::MoveToTarget(const FVector& TargetLocation)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Simple movement - in a real implementation, you'd use AI navigation
    FVector Direction = (TargetLocation - Owner->GetActorLocation()).GetSafeNormal();
    FVector NewLocation = Owner->GetActorLocation() + (Direction * MovementSpeed * GetWorld()->GetDeltaSeconds());
    
    Owner->SetActorLocation(NewLocation);
}

float UNPCBehaviorComponent::GetCurrentGameHour()
{
    // Simple time system - in a real game, this would connect to your day/night cycle
    float GameTime = GetWorld()->GetTimeSeconds();
    float HoursPerDay = 24.0f;
    float SecondsPerHour = 3600.0f / 60.0f; // Accelerated time: 1 minute = 1 hour
    
    float CurrentHour = FMath::Fmod(GameTime / SecondsPerHour, HoursPerDay);
    return CurrentHour;
}