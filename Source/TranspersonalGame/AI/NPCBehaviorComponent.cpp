#include "NPCBehaviorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"

UNPC_BehaviorComponent::UNPC_BehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize default values
    CurrentEmotionalState = ENPC_EmotionalState::Calm;
    SocialRole = ENPC_SocialRole::Lone;
    AwarenessRadius = 2000.0f;
    TerritoryRadius = 5000.0f;
    HomeLocation = FVector::ZeroVector;

    MaxMemories = 20;
    MemoryDecayTime = 300.0f; // 5 minutes
    SocialUpdateInterval = 2.0f;
    PatrolInterval = 10.0f;
    AlertCooldownTime = 30.0f;
    EmotionalDecayRate = 0.1f;

    CurrentTarget = nullptr;
    LastPlayerInteractionTime = 0.0f;
    bIsInCombat = false;
    bIsPatrolling = false;
}

void UNPC_BehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Set home location to current position
    if (GetOwner())
    {
        HomeLocation = GetOwner()->GetActorLocation();
    }

    // Start behavior timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(PatrolTimer, this, &UNPC_BehaviorComponent::UpdatePatrol, PatrolInterval, true);
        World->GetTimerManager().SetTimer(MemoryDecayTimer, this, &UNPC_BehaviorComponent::DecayMemories, MemoryDecayTime, true);
        World->GetTimerManager().SetTimer(SocialUpdateTimer, this, &UNPC_BehaviorComponent::UpdateSocialRelations, SocialUpdateInterval, true);
    }
}

void UNPC_BehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateEmotionalState(DeltaTime);

    // Check for nearby threats
    if (AActor* NearestThreat = GetNearestThreat())
    {
        if (CurrentTarget != NearestThreat)
        {
            CurrentTarget = NearestThreat;
            
            // Add memory of this threat
            AddMemory(NearestThreat->GetActorLocation(), 8, NearestThreat->IsA<APawn>());
            
            // Update emotional state based on threat
            if (ShouldFlee())
            {
                SetEmotionalState(ENPC_EmotionalState::Fearful);
            }
            else if (ShouldAttack())
            {
                SetEmotionalState(ENPC_EmotionalState::Aggressive);
                StartCombatBehavior(NearestThreat);
            }
            else
            {
                SetEmotionalState(ENPC_EmotionalState::Alert);
            }
        }
    }
    else if (CurrentTarget)
    {
        // No threats nearby, return to calm state
        CurrentTarget = nullptr;
        if (CurrentEmotionalState != ENPC_EmotionalState::Calm)
        {
            SetEmotionalState(ENPC_EmotionalState::Calm);
        }
        if (bIsInCombat)
        {
            EndCombatBehavior();
        }
    }
}

void UNPC_BehaviorComponent::SetEmotionalState(ENPC_EmotionalState NewState)
{
    if (CurrentEmotionalState != NewState)
    {
        CurrentEmotionalState = NewState;
        
        // Log state change for debugging
        if (GetOwner())
        {
            UE_LOG(LogTemp, Log, TEXT("%s emotional state changed to %d"), *GetOwner()->GetName(), (int32)NewState);
        }
    }
}

void UNPC_BehaviorComponent::AddMemory(FVector Location, int32 ThreatLevel, bool bIsPlayer)
{
    FNPC_Memory NewMemory;
    NewMemory.Location = Location;
    NewMemory.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    NewMemory.ThreatLevel = ThreatLevel;
    NewMemory.bIsPlayerMemory = bIsPlayer;

    Memories.Add(NewMemory);

    // Remove oldest memories if we exceed the limit
    if (Memories.Num() > MaxMemories)
    {
        Memories.RemoveAt(0);
    }
}

void UNPC_BehaviorComponent::UpdateSocialRelation(AActor* Actor, float RelationChange)
{
    if (!Actor) return;

    // Find existing relation or create new one
    FNPC_SocialRelation* ExistingRelation = nullptr;
    for (FNPC_SocialRelation& Relation : SocialRelations)
    {
        if (Relation.RelatedActor.Get() == Actor)
        {
            ExistingRelation = &Relation;
            break;
        }
    }

    if (ExistingRelation)
    {
        ExistingRelation->RelationshipValue += RelationChange;
        ExistingRelation->LastInteractionTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        ExistingRelation->bIsHostile = ExistingRelation->RelationshipValue < -5.0f;
    }
    else
    {
        FNPC_SocialRelation NewRelation;
        NewRelation.RelatedActor = Actor;
        NewRelation.RelationshipValue = RelationChange;
        NewRelation.LastInteractionTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        NewRelation.bIsHostile = RelationChange < -5.0f;
        SocialRelations.Add(NewRelation);
    }
}

AActor* UNPC_BehaviorComponent::GetNearestThreat()
{
    if (!GetOwner() || !GetWorld()) return nullptr;

    AActor* NearestThreat = nullptr;
    float NearestDistance = AwarenessRadius;

    FVector OwnerLocation = GetOwner()->GetActorLocation();

    // Check for player
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        float Distance = FVector::Dist(OwnerLocation, PlayerPawn->GetActorLocation());
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestThreat = PlayerPawn;
        }
    }

    // Check for other threatening actors based on memories
    for (const FNPC_Memory& Memory : Memories)
    {
        if (Memory.ThreatLevel > 5)
        {
            float Distance = FVector::Dist(OwnerLocation, Memory.Location);
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                // Note: We'd need to find the actual actor at this location
            }
        }
    }

    return NearestThreat;
}

bool UNPC_BehaviorComponent::ShouldFlee()
{
    // Flee if we're a herbivore or if we're heavily outnumbered
    return (SocialRole == ENPC_SocialRole::Juvenile || 
            SocialRole == ENPC_SocialRole::Omega ||
            CurrentEmotionalState == ENPC_EmotionalState::Fearful);
}

bool UNPC_BehaviorComponent::ShouldAttack()
{
    // Attack if we're aggressive, territorial, or protecting territory
    if (!CurrentTarget) return false;

    bool bInTerritory = FVector::Dist(GetOwner()->GetActorLocation(), HomeLocation) < TerritoryRadius;
    bool bIsAggressive = (CurrentEmotionalState == ENPC_EmotionalState::Aggressive ||
                         CurrentEmotionalState == ENPC_EmotionalState::Territorial);
    bool bIsAlpha = (SocialRole == ENPC_SocialRole::Alpha || SocialRole == ENPC_SocialRole::PackLeader);

    return (bInTerritory && bIsAggressive) || (bIsAlpha && CurrentEmotionalState != ENPC_EmotionalState::Fearful);
}

FVector UNPC_BehaviorComponent::GetPatrolDestination()
{
    if (!GetOwner()) return FVector::ZeroVector;

    // Generate random patrol point within territory
    FVector RandomDirection = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0.0f
    ).GetSafeNormal();

    float PatrolDistance = FMath::RandRange(TerritoryRadius * 0.3f, TerritoryRadius * 0.8f);
    return HomeLocation + (RandomDirection * PatrolDistance);
}

void UNPC_BehaviorComponent::StartCombatBehavior(AActor* Target)
{
    if (!Target) return;

    bIsInCombat = true;
    bIsPatrolling = false;
    CurrentTarget = Target;
    
    // Update social relation - combat makes us hostile
    UpdateSocialRelation(Target, -10.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("%s entering combat with %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
           *Target->GetName());
}

void UNPC_BehaviorComponent::EndCombatBehavior()
{
    bIsInCombat = false;
    CurrentTarget = nullptr;
    
    // Return to patrol behavior after a cooldown
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(PatrolTimer, this, &UNPC_BehaviorComponent::UpdatePatrol, AlertCooldownTime, false);
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s ending combat behavior"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UNPC_BehaviorComponent::UpdatePatrol()
{
    if (bIsInCombat || CurrentEmotionalState == ENPC_EmotionalState::Fearful) return;

    bIsPatrolling = true;
    
    // This would typically move the actor to a new patrol point
    // Implementation would depend on the movement component being used
    FVector PatrolDestination = GetPatrolDestination();
    
    UE_LOG(LogTemp, Log, TEXT("%s patrolling to %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
           *PatrolDestination.ToString());
}

void UNPC_BehaviorComponent::DecayMemories()
{
    if (!GetWorld()) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove old memories
    Memories.RemoveAll([CurrentTime, this](const FNPC_Memory& Memory)
    {
        return (CurrentTime - Memory.Timestamp) > MemoryDecayTime;
    });
}

void UNPC_BehaviorComponent::UpdateSocialRelations()
{
    if (!GetWorld()) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Decay social relations over time
    for (FNPC_SocialRelation& Relation : SocialRelations)
    {
        float TimeSinceInteraction = CurrentTime - Relation.LastInteractionTime;
        if (TimeSinceInteraction > 60.0f) // 1 minute
        {
            // Gradually move towards neutral
            if (Relation.RelationshipValue > 0.0f)
            {
                Relation.RelationshipValue = FMath::Max(0.0f, Relation.RelationshipValue - 0.1f);
            }
            else if (Relation.RelationshipValue < 0.0f)
            {
                Relation.RelationshipValue = FMath::Min(0.0f, Relation.RelationshipValue + 0.1f);
            }
            
            Relation.bIsHostile = Relation.RelationshipValue < -5.0f;
        }
    }
    
    // Remove relations for destroyed actors
    SocialRelations.RemoveAll([](const FNPC_SocialRelation& Relation)
    {
        return !Relation.RelatedActor.IsValid();
    });
}

void UNPC_BehaviorComponent::UpdateEmotionalState(float DeltaTime)
{
    // Gradually return to calm state when no threats are present
    if (!CurrentTarget && CurrentEmotionalState != ENPC_EmotionalState::Calm)
    {
        // Emotional decay towards calm
        static float EmotionalDecayTimer = 0.0f;
        EmotionalDecayTimer += DeltaTime;
        
        if (EmotionalDecayTimer >= EmotionalDecayRate)
        {
            EmotionalDecayTimer = 0.0f;
            
            // Gradually transition to calm
            if (CurrentEmotionalState == ENPC_EmotionalState::Alert)
            {
                SetEmotionalState(ENPC_EmotionalState::Calm);
            }
            else if (CurrentEmotionalState == ENPC_EmotionalState::Fearful)
            {
                SetEmotionalState(ENPC_EmotionalState::Alert);
            }
            else if (CurrentEmotionalState == ENPC_EmotionalState::Aggressive)
            {
                SetEmotionalState(ENPC_EmotionalState::Alert);
            }
        }
    }
}

float UNPC_BehaviorComponent::CalculateThreatLevel(AActor* Actor)
{
    if (!Actor) return 0.0f;

    float ThreatLevel = 1.0f;
    
    // Player is always a significant threat
    if (Actor->IsA<APawn>())
    {
        ThreatLevel += 5.0f;
    }
    
    // Distance affects threat level
    if (GetOwner())
    {
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
        if (Distance < 500.0f)
        {
            ThreatLevel += 3.0f;
        }
        else if (Distance < 1000.0f)
        {
            ThreatLevel += 1.0f;
        }
    }
    
    // Check social relations
    for (const FNPC_SocialRelation& Relation : SocialRelations)
    {
        if (Relation.RelatedActor.Get() == Actor)
        {
            if (Relation.bIsHostile)
            {
                ThreatLevel += 4.0f;
            }
            else if (Relation.RelationshipValue > 5.0f)
            {
                ThreatLevel -= 2.0f; // Friendly actors are less threatening
            }
            break;
        }
    }
    
    return FMath::Max(0.0f, ThreatLevel);
}