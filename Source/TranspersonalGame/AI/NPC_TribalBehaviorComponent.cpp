#include "NPC_TribalBehaviorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"

UNPC_TribalBehaviorComponent::UNPC_TribalBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Default tribal role and activity
    TribalRole = ENPC_TribalRole::Gatherer;
    CurrentActivity = ENPC_TribalActivity::Idle;
    
    // Default tribe settings
    TribeName = TEXT("DefaultTribe");
    SocialRadius = 1000.0f;
    PatrolRadius = 1500.0f;
    FleeRadius = 500.0f;

    // Memory and decision settings
    MemoryRetentionTime = 300.0f; // 5 minutes
    DecisionUpdateInterval = 2.0f; // 2 seconds

    // Default personality traits
    Courage = 0.5f;
    Curiosity = 0.6f;
    Sociability = 0.7f;
    Aggression = 0.3f;

    // Initialize state
    CurrentTarget = nullptr;
    CurrentDestination = FVector::ZeroVector;
    bIsInDanger = false;
    CurrentFear = 0.0f;
    
    LastDecisionTime = 0.0f;
    LastMemoryUpdate = 0.0f;
}

void UNPC_TribalBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize tribal memory
    TribalMemory = FNPC_TribalMemory();
    
    // Set up timers for AI updates
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            DecisionTimerHandle,
            this,
            &UNPC_TribalBehaviorComponent::OnDecisionUpdate,
            DecisionUpdateInterval,
            true
        );

        World->GetTimerManager().SetTimer(
            MemoryUpdateHandle,
            this,
            &UNPC_TribalBehaviorComponent::OnMemoryUpdate,
            1.0f,
            true
        );
    }

    // Find initial tribe members
    FindNearbyTribeMembers();
    
    UE_LOG(LogTemp, Log, TEXT("TribalBehaviorComponent initialized for %s with role %d"), 
           *GetOwner()->GetName(), (int32)TribalRole);
}

void UNPC_TribalBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateTribalBehavior(DeltaTime);
}

void UNPC_TribalBehaviorComponent::UpdateTribalBehavior(float DeltaTime)
{
    if (!GetOwner())
    {
        return;
    }

    // Update fear level based on threats
    if (bIsInDanger)
    {
        CurrentFear = FMath::FInterpTo(CurrentFear, 1.0f, DeltaTime, 2.0f);
    }
    else
    {
        CurrentFear = FMath::FInterpTo(CurrentFear, 0.0f, DeltaTime, 1.0f);
    }

    // Update current activity based on role and situation
    switch (TribalRole)
    {
        case ENPC_TribalRole::Scout:
            if (CurrentActivity == ENPC_TribalActivity::Idle)
            {
                SetCurrentActivity(ENPC_TribalActivity::Patrolling);
            }
            break;
            
        case ENPC_TribalRole::Hunter:
            if (CurrentActivity == ENPC_TribalActivity::Idle && FMath::RandRange(0.0f, 1.0f) < 0.3f)
            {
                SetCurrentActivity(ENPC_TribalActivity::Hunting);
            }
            break;
            
        case ENPC_TribalRole::Gatherer:
            if (CurrentActivity == ENPC_TribalActivity::Idle && FMath::RandRange(0.0f, 1.0f) < 0.4f)
            {
                SetCurrentActivity(ENPC_TribalActivity::Gathering);
            }
            break;
            
        default:
            break;
    }

    // Handle danger response
    if (bIsInDanger)
    {
        if (ShouldFlee())
        {
            SetCurrentActivity(ENPC_TribalActivity::Fleeing);
        }
        else if (ShouldFight())
        {
            SetCurrentActivity(ENPC_TribalActivity::Fighting);
        }
    }
}

void UNPC_TribalBehaviorComponent::SetTribalRole(ENPC_TribalRole NewRole)
{
    TribalRole = NewRole;
    
    // Adjust personality based on role
    switch (NewRole)
    {
        case ENPC_TribalRole::Scout:
            Curiosity = FMath::Clamp(Curiosity + 0.2f, 0.0f, 1.0f);
            break;
        case ENPC_TribalRole::Hunter:
            Aggression = FMath::Clamp(Aggression + 0.3f, 0.0f, 1.0f);
            Courage = FMath::Clamp(Courage + 0.2f, 0.0f, 1.0f);
            break;
        case ENPC_TribalRole::Warrior:
            Courage = FMath::Clamp(Courage + 0.4f, 0.0f, 1.0f);
            Aggression = FMath::Clamp(Aggression + 0.3f, 0.0f, 1.0f);
            break;
        case ENPC_TribalRole::Elder:
            Sociability = FMath::Clamp(Sociability + 0.3f, 0.0f, 1.0f);
            break;
        default:
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s assigned tribal role: %d"), *GetOwner()->GetName(), (int32)NewRole);
}

void UNPC_TribalBehaviorComponent::SetCurrentActivity(ENPC_TribalActivity NewActivity)
{
    if (CurrentActivity != NewActivity)
    {
        CurrentActivity = NewActivity;
        UE_LOG(LogTemp, Log, TEXT("%s changed activity to: %d"), *GetOwner()->GetName(), (int32)NewActivity);
        
        // Set appropriate destination based on activity
        switch (NewActivity)
        {
            case ENPC_TribalActivity::Patrolling:
                CurrentDestination = GetPatrolDestination();
                break;
            case ENPC_TribalActivity::Fleeing:
                // Move away from threat
                if (TribalMemory.LastKnownThreatLocation != FVector::ZeroVector)
                {
                    FVector FleeDirection = (GetOwner()->GetActorLocation() - TribalMemory.LastKnownThreatLocation).GetSafeNormal();
                    CurrentDestination = GetOwner()->GetActorLocation() + FleeDirection * FleeRadius;
                }
                break;
            default:
                break;
        }
    }
}

void UNPC_TribalBehaviorComponent::DetectThreat(AActor* ThreatActor, float ThreatLevel)
{
    if (!ThreatActor)
    {
        return;
    }

    TribalMemory.LastKnownThreatLocation = ThreatActor->GetActorLocation();
    TribalMemory.ThreatLevel = ThreatLevel;
    TribalMemory.LastSeenThreatTime = GetWorld()->GetTimeSeconds();
    
    bIsInDanger = true;
    CurrentTarget = ThreatActor;
    
    // Share threat information with nearby tribe members
    ShareThreatInformation(TribalMemory.LastKnownThreatLocation, ThreatLevel);
    
    UE_LOG(LogTemp, Warning, TEXT("%s detected threat: %s (Level: %.1f)"), 
           *GetOwner()->GetName(), *ThreatActor->GetName(), ThreatLevel);
}

void UNPC_TribalBehaviorComponent::ShareThreatInformation(const FVector& ThreatLocation, float ThreatLevel)
{
    for (AActor* TribeMember : NearbyTribeMembers)
    {
        if (TribeMember && TribeMember != GetOwner())
        {
            if (UNPC_TribalBehaviorComponent* OtherTribal = TribeMember->FindComponentByClass<UNPC_TribalBehaviorComponent>())
            {
                OtherTribal->TribalMemory.LastKnownThreatLocation = ThreatLocation;
                OtherTribal->TribalMemory.ThreatLevel = ThreatLevel * 0.8f; // Slightly reduced for shared info
                OtherTribal->TribalMemory.LastSeenThreatTime = GetWorld()->GetTimeSeconds();
                OtherTribal->bIsInDanger = true;
            }
        }
    }
}

void UNPC_TribalBehaviorComponent::FindNearbyTribeMembers()
{
    NearbyTribeMembers.Empty();
    
    if (!GetOwner())
    {
        return;
    }

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor != GetOwner())
        {
            if (UNPC_TribalBehaviorComponent* OtherTribal = Actor->FindComponentByClass<UNPC_TribalBehaviorComponent>())
            {
                float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
                if (Distance <= SocialRadius && OtherTribal->TribeName == TribeName)
                {
                    NearbyTribeMembers.Add(Actor);
                }
            }
        }
    }
}

FVector UNPC_TribalBehaviorComponent::GetPatrolDestination()
{
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Generate random patrol point within patrol radius
    float RandomAngle = FMath::RandRange(0.0f, 360.0f);
    float RandomDistance = FMath::RandRange(PatrolRadius * 0.5f, PatrolRadius);
    
    FVector PatrolOffset = FVector(
        FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
        FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
        0.0f
    );
    
    return OwnerLocation + PatrolOffset;
}

bool UNPC_TribalBehaviorComponent::ShouldFlee()
{
    float FleeThreshold = 1.0f - Courage;
    return CurrentFear > FleeThreshold && TribalMemory.ThreatLevel > 0.6f;
}

bool UNPC_TribalBehaviorComponent::ShouldFight()
{
    float FightThreshold = Courage + Aggression * 0.5f;
    return CurrentFear < 0.5f && FightThreshold > 0.7f && NearbyTribeMembers.Num() >= 2;
}

void UNPC_TribalBehaviorComponent::UpdateMemory(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Fade threat memory over time
    if (CurrentTime - TribalMemory.LastSeenThreatTime > MemoryRetentionTime)
    {
        TribalMemory.ThreatLevel = FMath::FInterpTo(TribalMemory.ThreatLevel, 0.0f, DeltaTime, 0.5f);
        if (TribalMemory.ThreatLevel < 0.1f)
        {
            bIsInDanger = false;
            CurrentTarget = nullptr;
        }
    }
}

void UNPC_TribalBehaviorComponent::MakeDecision()
{
    // Update tribe member awareness
    FindNearbyTribeMembers();
    
    // Make role-based decisions
    switch (TribalRole)
    {
        case ENPC_TribalRole::Scout:
            if (CurrentActivity == ENPC_TribalActivity::Idle)
            {
                SetCurrentActivity(ENPC_TribalActivity::Patrolling);
            }
            break;
            
        case ENPC_TribalRole::Elder:
            if (NearbyTribeMembers.Num() > 0 && CurrentActivity != ENPC_TribalActivity::Socializing)
            {
                SetCurrentActivity(ENPC_TribalActivity::Socializing);
            }
            break;
            
        default:
            break;
    }
}

void UNPC_TribalBehaviorComponent::OnDecisionUpdate()
{
    MakeDecision();
    LastDecisionTime = GetWorld()->GetTimeSeconds();
}

void UNPC_TribalBehaviorComponent::OnMemoryUpdate()
{
    UpdateMemory(1.0f);
    LastMemoryUpdate = GetWorld()->GetTimeSeconds();
}