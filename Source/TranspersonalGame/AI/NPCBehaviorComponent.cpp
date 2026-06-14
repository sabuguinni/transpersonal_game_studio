#include "NPCBehaviorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    CurrentEmotionalState = ENPC_EmotionalState::Calm;
    
    PatrolCenter = FVector::ZeroVector;
    PatrolRadius = 2000.0f;
    CurrentPatrolIndex = 0;
    
    MaxMemories = 20;
    MemoryDecayRate = 0.1f;
    
    SightRange = 3000.0f;
    HearingRange = 1500.0f;
    AlertLevel = 0.0f;
    CurrentTarget = nullptr;
    
    SocialRadius = 1000.0f;
    bCanInteractWithPlayer = true;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Set patrol center to current location
    if (GetOwner())
    {
        PatrolCenter = GetOwner()->GetActorLocation();
        GeneratePatrolPoints();
        
        // Start behavior update timer
        GetWorld()->GetTimerManager().SetTimer(BehaviorUpdateTimer, this, &UNPCBehaviorComponent::UpdateBehavior, 1.0f, true);
        GetWorld()->GetTimerManager().SetTimer(MemoryUpdateTimer, this, &UNPCBehaviorComponent::UpdateMemories, 5.0f, true, 0.0f);
        GetWorld()->GetTimerManager().SetTimer(DetectionUpdateTimer, this, &UNPCBehaviorComponent::UpdateDetection, 0.5f, true);
        
        // Start with patrol behavior
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update alert level decay
    if (AlertLevel > 0.0f)
    {
        AlertLevel = FMath::Max(0.0f, AlertLevel - DeltaTime * 0.5f);
    }
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;
        
        // Add memory of state change
        if (GetOwner())
        {
            AddMemory(GetOwner()->GetActorLocation(), FString::Printf(TEXT("StateChange_%s"), 
                *UEnum::GetValueAsString(NewState)), 0.3f);
        }
    }
}

void UNPCBehaviorComponent::SetEmotionalState(ENPC_EmotionalState NewState)
{
    CurrentEmotionalState = NewState;
}

void UNPCBehaviorComponent::StartPatrol()
{
    SetBehaviorState(ENPC_BehaviorState::Patrol);
    CurrentPatrolIndex = 0;
}

void UNPCBehaviorComponent::GeneratePatrolPoints()
{
    PatrolPoints.Empty();
    
    // Generate 4-6 patrol points in a circle around patrol center
    int32 NumPoints = FMath::RandRange(4, 6);
    float AngleStep = 360.0f / NumPoints;
    
    for (int32 i = 0; i < NumPoints; i++)
    {
        float Angle = i * AngleStep + FMath::RandRange(-30.0f, 30.0f);
        float Distance = PatrolRadius * FMath::RandRange(0.3f, 1.0f);
        
        FVector Offset = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            0.0f
        );
        
        PatrolPoints.Add(PatrolCenter + Offset);
    }
}

FVector UNPCBehaviorComponent::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        GeneratePatrolPoints();
    }
    
    if (PatrolPoints.Num() > 0)
    {
        FVector NextPoint = PatrolPoints[CurrentPatrolIndex];
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
        return NextPoint;
    }
    
    return PatrolCenter;
}

void UNPCBehaviorComponent::AddMemory(FVector Location, FString EventType, float Importance)
{
    FNPC_MemoryEntry NewMemory;
    NewMemory.Location = Location;
    NewMemory.EventType = EventType;
    NewMemory.Importance = Importance;
    NewMemory.Timestamp = GetWorld()->GetTimeSeconds();
    
    Memories.Add(NewMemory);
    
    // Remove oldest memories if we exceed max
    if (Memories.Num() > MaxMemories)
    {
        Memories.RemoveAt(0);
    }
}

void UNPCBehaviorComponent::UpdateMemories(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Decay memory importance over time
    for (int32 i = Memories.Num() - 1; i >= 0; i--)
    {
        float Age = CurrentTime - Memories[i].Timestamp;
        Memories[i].Importance = FMath::Max(0.0f, Memories[i].Importance - Age * MemoryDecayRate);
        
        // Remove very old or unimportant memories
        if (Memories[i].Importance < 0.01f || Age > 300.0f)
        {
            Memories.RemoveAt(i);
        }
    }
}

FNPC_MemoryEntry UNPCBehaviorComponent::GetMostImportantMemory()
{
    FNPC_MemoryEntry MostImportant;
    float HighestImportance = 0.0f;
    
    for (const FNPC_MemoryEntry& Memory : Memories)
    {
        if (Memory.Importance > HighestImportance)
        {
            HighestImportance = Memory.Importance;
            MostImportant = Memory;
        }
    }
    
    return MostImportant;
}

bool UNPCBehaviorComponent::CanSeeActor(AActor* Actor)
{
    if (!Actor || !GetOwner())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
    if (Distance > SightRange)
    {
        return false;
    }
    
    // Simple line trace for line of sight
    FHitResult HitResult;
    FVector Start = GetOwner()->GetActorLocation();
    FVector End = Actor->GetActorLocation();
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(Actor);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
    
    return !bHit; // Can see if no obstruction
}

bool UNPCBehaviorComponent::CanHearActor(AActor* Actor)
{
    if (!Actor || !GetOwner())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
    return Distance <= HearingRange;
}

void UNPCBehaviorComponent::UpdateDetection()
{
    if (!GetOwner())
    {
        return;
    }
    
    // Find player
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        bool bCanSeePlayer = CanSeeActor(PlayerPawn);
        bool bCanHearPlayer = CanHearActor(PlayerPawn);
        
        if (bCanSeePlayer || bCanHearPlayer)
        {
            AlertLevel = FMath::Min(1.0f, AlertLevel + 0.1f);
            CurrentTarget = PlayerPawn;
            
            // Add memory of player sighting
            AddMemory(PlayerPawn->GetActorLocation(), TEXT("PlayerSighting"), 0.8f);
            
            // Change behavior based on alert level
            if (AlertLevel > 0.7f && CurrentBehaviorState == ENPC_BehaviorState::Patrol)
            {
                SetBehaviorState(ENPC_BehaviorState::Investigate);
                SetEmotionalState(ENPC_EmotionalState::Alert);
            }
        }
    }
}

void UNPCBehaviorComponent::FindNearbyNPCs()
{
    if (!GetOwner())
    {
        return;
    }
    
    KnownNPCs.Empty();
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor != GetOwner() && Actor != UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
            if (Distance <= SocialRadius)
            {
                KnownNPCs.Add(Actor);
            }
        }
    }
}

void UNPCBehaviorComponent::InteractWithNPC(AActor* OtherNPC)
{
    if (!OtherNPC || !GetOwner())
    {
        return;
    }
    
    // Add memory of social interaction
    AddMemory(OtherNPC->GetActorLocation(), TEXT("SocialInteraction"), 0.5f);
    
    // Set social emotional state
    SetEmotionalState(ENPC_EmotionalState::Calm);
}

void UNPCBehaviorComponent::UpdateBehavior()
{
    switch (CurrentBehaviorState)
    {
        case ENPC_BehaviorState::Idle:
            HandleIdleBehavior();
            break;
        case ENPC_BehaviorState::Patrol:
            HandlePatrolBehavior();
            break;
        case ENPC_BehaviorState::Chase:
            HandleChaseBehavior();
            break;
        case ENPC_BehaviorState::Investigate:
            HandleInvestigateBehavior();
            break;
        case ENPC_BehaviorState::Social:
            HandleSocialBehavior();
            break;
    }
}

void UNPCBehaviorComponent::HandleIdleBehavior()
{
    // Randomly switch to patrol after some time
    if (FMath::RandRange(0.0f, 1.0f) < 0.3f)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::HandlePatrolBehavior()
{
    // Continue patrolling unless alert level is high
    if (AlertLevel > 0.5f)
    {
        SetBehaviorState(ENPC_BehaviorState::Investigate);
    }
    
    // Check for social opportunities
    FindNearbyNPCs();
    if (KnownNPCs.Num() > 0 && FMath::RandRange(0.0f, 1.0f) < 0.1f)
    {
        SetBehaviorState(ENPC_BehaviorState::Social);
    }
}

void UNPCBehaviorComponent::HandleChaseBehavior()
{
    // If we lose the target, go back to investigating
    if (!CurrentTarget || AlertLevel < 0.3f)
    {
        SetBehaviorState(ENPC_BehaviorState::Investigate);
        CurrentTarget = nullptr;
    }
}

void UNPCBehaviorComponent::HandleInvestigateBehavior()
{
    // If alert level drops, return to patrol
    if (AlertLevel < 0.2f)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
        SetEmotionalState(ENPC_EmotionalState::Calm);
    }
    
    // If alert level is very high, start chasing
    if (AlertLevel > 0.8f && CurrentTarget)
    {
        SetBehaviorState(ENPC_BehaviorState::Chase);
        SetEmotionalState(ENPC_EmotionalState::Aggressive);
    }
}

void UNPCBehaviorComponent::HandleSocialBehavior()
{
    // Interact with nearby NPCs
    if (KnownNPCs.Num() > 0)
    {
        AActor* NearestNPC = KnownNPCs[0];
        InteractWithNPC(NearestNPC);
    }
    
    // Return to patrol after social interaction
    if (FMath::RandRange(0.0f, 1.0f) < 0.4f)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}