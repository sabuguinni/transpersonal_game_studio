#include "NPC_SocialInteractionManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UNPC_SocialInteractionManager::UNPC_SocialInteractionManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Default Configuration
    PerceptionRadius = 1000.0f;
    SocialUpdateInterval = 1.0f;
    MemoryDecayRate = 0.1f;
    MaxSocialMemories = 20;
    
    bIsGroupLeader = false;
    LastSocialUpdate = 0.0f;
}

void UNPC_SocialInteractionManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("NPC Social Interaction Manager initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UNPC_SocialInteractionManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    LastSocialUpdate += DeltaTime;
    
    if (LastSocialUpdate >= SocialUpdateInterval)
    {
        ProcessNearbyActors();
        UpdateSocialRelations(DeltaTime);
        DecayMemories(DeltaTime);
        LastSocialUpdate = 0.0f;
    }
}

void UNPC_SocialInteractionManager::AddSocialRelation(AActor* TargetActor, ENPC_SocialRelationType RelationType, float InitialStrength)
{
    if (!TargetActor || TargetActor == GetOwner())
    {
        return;
    }

    // Check if relation already exists
    for (FNPC_SocialRelation& Relation : SocialRelations)
    {
        if (Relation.TargetActor == TargetActor)
        {
            // Update existing relation
            Relation.RelationType = RelationType;
            Relation.RelationshipStrength = InitialStrength;
            return;
        }
    }

    // Create new relation
    FNPC_SocialRelation NewRelation;
    NewRelation.TargetActor = TargetActor;
    NewRelation.RelationType = RelationType;
    NewRelation.RelationshipStrength = InitialStrength;
    NewRelation.LastInteractionTime = GetWorld()->GetTimeSeconds();
    NewRelation.InteractionCount = 0;

    SocialRelations.Add(NewRelation);

    UE_LOG(LogTemp, Log, TEXT("%s: Added social relation with %s (Type: %d, Strength: %.2f)"), 
           *GetOwner()->GetName(), *TargetActor->GetName(), (int32)RelationType, InitialStrength);
}

void UNPC_SocialInteractionManager::UpdateRelationship(AActor* TargetActor, float StrengthChange)
{
    if (!TargetActor)
    {
        return;
    }

    for (FNPC_SocialRelation& Relation : SocialRelations)
    {
        if (Relation.TargetActor == TargetActor)
        {
            float OldStrength = Relation.RelationshipStrength;
            Relation.RelationshipStrength = FMath::Clamp(Relation.RelationshipStrength + StrengthChange, -100.0f, 100.0f);
            Relation.LastInteractionTime = GetWorld()->GetTimeSeconds();
            Relation.InteractionCount++;

            // Update relation type based on strength
            if (Relation.RelationshipStrength > 50.0f)
            {
                Relation.RelationType = ENPC_SocialRelationType::Friendly;
            }
            else if (Relation.RelationshipStrength < -50.0f)
            {
                Relation.RelationType = ENPC_SocialRelationType::Hostile;
            }
            else
            {
                Relation.RelationType = ENPC_SocialRelationType::Neutral;
            }

            UE_LOG(LogTemp, Log, TEXT("%s: Updated relationship with %s (%.2f -> %.2f)"), 
                   *GetOwner()->GetName(), *TargetActor->GetName(), OldStrength, Relation.RelationshipStrength);
            return;
        }
    }

    // If no existing relation, create neutral one and update
    AddSocialRelation(TargetActor, ENPC_SocialRelationType::Neutral, StrengthChange);
}

FNPC_SocialRelation UNPC_SocialInteractionManager::GetRelationship(AActor* TargetActor)
{
    FNPC_SocialRelation EmptyRelation;
    
    if (!TargetActor)
    {
        return EmptyRelation;
    }

    for (const FNPC_SocialRelation& Relation : SocialRelations)
    {
        if (Relation.TargetActor == TargetActor)
        {
            return Relation;
        }
    }

    return EmptyRelation;
}

bool UNPC_SocialInteractionManager::HasRelationship(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return false;
    }

    for (const FNPC_SocialRelation& Relation : SocialRelations)
    {
        if (Relation.TargetActor == TargetActor)
        {
            return true;
        }
    }

    return false;
}

void UNPC_SocialInteractionManager::PerformSocialAction(AActor* TargetActor, ENPC_SocialAction Action)
{
    if (!TargetActor)
    {
        return;
    }

    // Update memory
    UpdateSocialMemory(TargetActor, TargetActor->GetActorLocation(), Action);

    // Update relationship based on action
    float RelationshipChange = 0.0f;
    
    switch (Action)
    {
        case ENPC_SocialAction::Greet:
        case ENPC_SocialAction::Help:
            RelationshipChange = 5.0f;
            break;
        case ENPC_SocialAction::Talk:
        case ENPC_SocialAction::Trade:
            RelationshipChange = 2.0f;
            break;
        case ENPC_SocialAction::Fight:
            RelationshipChange = -20.0f;
            break;
        case ENPC_SocialAction::Avoid:
            RelationshipChange = -2.0f;
            break;
        case ENPC_SocialAction::Follow:
            RelationshipChange = 3.0f;
            break;
        case ENPC_SocialAction::Ignore:
        default:
            RelationshipChange = 0.0f;
            break;
    }

    if (RelationshipChange != 0.0f)
    {
        UpdateRelationship(TargetActor, RelationshipChange);
    }

    UE_LOG(LogTemp, Log, TEXT("%s: Performed action %d on %s (Relationship change: %.2f)"), 
           *GetOwner()->GetName(), (int32)Action, *TargetActor->GetName(), RelationshipChange);
}

ENPC_SocialAction UNPC_SocialInteractionManager::DecideSocialAction(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return ENPC_SocialAction::Ignore;
    }

    FNPC_SocialRelation Relation = GetRelationship(TargetActor);
    
    // Decision based on relationship type and strength
    switch (Relation.RelationType)
    {
        case ENPC_SocialRelationType::Friendly:
            if (Relation.RelationshipStrength > 70.0f)
            {
                return ENPC_SocialAction::Help;
            }
            else if (Relation.RelationshipStrength > 30.0f)
            {
                return ENPC_SocialAction::Talk;
            }
            else
            {
                return ENPC_SocialAction::Greet;
            }
            
        case ENPC_SocialRelationType::Hostile:
            if (Relation.RelationshipStrength < -70.0f)
            {
                return ENPC_SocialAction::Fight;
            }
            else
            {
                return ENPC_SocialAction::Avoid;
            }
            
        case ENPC_SocialRelationType::Neutral:
        default:
            // Random chance for neutral interactions
            int32 RandomAction = FMath::RandRange(0, 2);
            switch (RandomAction)
            {
                case 0: return ENPC_SocialAction::Greet;
                case 1: return ENPC_SocialAction::Talk;
                case 2: return ENPC_SocialAction::Ignore;
                default: return ENPC_SocialAction::Ignore;
            }
    }
}

void UNPC_SocialInteractionManager::UpdateSocialMemory(AActor* Actor, const FVector& Location, ENPC_SocialAction Action)
{
    if (!Actor)
    {
        return;
    }

    // Find existing memory
    for (FNPC_SocialMemory& Memory : SocialMemories)
    {
        if (Memory.Actor == Actor)
        {
            Memory.LastKnownLocation = Location;
            Memory.LastSeenTime = GetWorld()->GetTimeSeconds();
            Memory.LastAction = Action;
            Memory.bIsVisible = CanSeeActor(Actor);
            return;
        }
    }

    // Create new memory if we haven't reached the limit
    if (SocialMemories.Num() < MaxSocialMemories)
    {
        FNPC_SocialMemory NewMemory;
        NewMemory.Actor = Actor;
        NewMemory.LastKnownLocation = Location;
        NewMemory.LastSeenTime = GetWorld()->GetTimeSeconds();
        NewMemory.LastAction = Action;
        NewMemory.bIsVisible = CanSeeActor(Actor);
        
        SocialMemories.Add(NewMemory);
    }
}

FNPC_SocialMemory UNPC_SocialInteractionManager::GetSocialMemory(AActor* Actor)
{
    FNPC_SocialMemory EmptyMemory;
    
    if (!Actor)
    {
        return EmptyMemory;
    }

    for (const FNPC_SocialMemory& Memory : SocialMemories)
    {
        if (Memory.Actor == Actor)
        {
            return Memory;
        }
    }

    return EmptyMemory;
}

void UNPC_SocialInteractionManager::ForgetActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    SocialMemories.RemoveAll([Actor](const FNPC_SocialMemory& Memory)
    {
        return Memory.Actor == Actor;
    });

    SocialRelations.RemoveAll([Actor](const FNPC_SocialRelation& Relation)
    {
        return Relation.TargetActor == Actor;
    });
}

TArray<AActor*> UNPC_SocialInteractionManager::GetNearbyNPCs(float Radius)
{
    TArray<AActor*> NearbyNPCs;
    
    if (!GetOwner())
    {
        return NearbyNPCs;
    }

    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Get all actors in the world
    UWorld* World = GetWorld();
    if (!World)
    {
        return NearbyNPCs;
    }

    for (TActorIterator<APawn> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        APawn* Pawn = *ActorIterator;
        
        if (Pawn && Pawn != GetOwner())
        {
            float Distance = FVector::Dist(OwnerLocation, Pawn->GetActorLocation());
            
            if (Distance <= Radius)
            {
                // Check if it's an NPC (has NPC tag or specific component)
                if (Pawn->Tags.Contains("NPC") || Pawn->FindComponentByClass<UNPC_SocialInteractionManager>())
                {
                    NearbyNPCs.Add(Pawn);
                }
            }
        }
    }

    return NearbyNPCs;
}

bool UNPC_SocialInteractionManager::CanSeeActor(AActor* TargetActor)
{
    if (!TargetActor || !GetOwner())
    {
        return false;
    }

    FVector Start = GetOwner()->GetActorLocation();
    FVector End = TargetActor->GetActorLocation();
    
    // Simple line trace to check visibility
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(TargetActor);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );

    return !bHit; // If no hit, we can see the actor
}

void UNPC_SocialInteractionManager::JoinGroup(const FString& GroupName)
{
    CurrentGroup = GroupName;
    UE_LOG(LogTemp, Log, TEXT("%s joined group: %s"), *GetOwner()->GetName(), *GroupName);
}

void UNPC_SocialInteractionManager::LeaveGroup()
{
    if (!CurrentGroup.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("%s left group: %s"), *GetOwner()->GetName(), *CurrentGroup);
        CurrentGroup.Empty();
        bIsGroupLeader = false;
    }
}

TArray<AActor*> UNPC_SocialInteractionManager::GetGroupMembers()
{
    TArray<AActor*> GroupMembers;
    
    if (CurrentGroup.IsEmpty())
    {
        return GroupMembers;
    }

    // Find all NPCs with the same group
    TArray<AActor*> AllNPCs = GetNearbyNPCs(5000.0f); // Large radius to find all group members
    
    for (AActor* NPC : AllNPCs)
    {
        UNPC_SocialInteractionManager* OtherSocialManager = NPC->FindComponentByClass<UNPC_SocialInteractionManager>();
        if (OtherSocialManager && OtherSocialManager->CurrentGroup == CurrentGroup)
        {
            GroupMembers.Add(NPC);
        }
    }

    return GroupMembers;
}

void UNPC_SocialInteractionManager::UpdateSocialRelations(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Decay relationships over time if no recent interaction
    for (FNPC_SocialRelation& Relation : SocialRelations)
    {
        float TimeSinceLastInteraction = CurrentTime - Relation.LastInteractionTime;
        
        // Decay after 60 seconds of no interaction
        if (TimeSinceLastInteraction > 60.0f)
        {
            float DecayAmount = MemoryDecayRate * DeltaTime;
            
            if (Relation.RelationshipStrength > 0.0f)
            {
                Relation.RelationshipStrength = FMath::Max(0.0f, Relation.RelationshipStrength - DecayAmount);
            }
            else if (Relation.RelationshipStrength < 0.0f)
            {
                Relation.RelationshipStrength = FMath::Min(0.0f, Relation.RelationshipStrength + DecayAmount);
            }
        }
    }
}

void UNPC_SocialInteractionManager::DecayMemories(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove old memories
    SocialMemories.RemoveAll([CurrentTime](const FNPC_SocialMemory& Memory)
    {
        return (CurrentTime - Memory.LastSeenTime) > 300.0f; // Forget after 5 minutes
    });
}

void UNPC_SocialInteractionManager::ProcessNearbyActors()
{
    TArray<AActor*> NearbyNPCs = GetNearbyNPCs(PerceptionRadius);
    
    for (AActor* NPC : NearbyNPCs)
    {
        if (CanSeeActor(NPC))
        {
            // Update memory for visible NPCs
            UpdateSocialMemory(NPC, NPC->GetActorLocation(), ENPC_SocialAction::Ignore);
            
            // Decide and perform social action if we don't have a recent interaction
            FNPC_SocialRelation Relation = GetRelationship(NPC);
            float CurrentTime = GetWorld()->GetTimeSeconds();
            
            if (CurrentTime - Relation.LastInteractionTime > 5.0f) // Wait 5 seconds between interactions
            {
                ENPC_SocialAction Action = DecideSocialAction(NPC);
                if (Action != ENPC_SocialAction::Ignore)
                {
                    PerformSocialAction(NPC, Action);
                }
            }
        }
    }
}