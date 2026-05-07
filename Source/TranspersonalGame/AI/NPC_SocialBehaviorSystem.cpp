#include "NPC_SocialBehaviorSystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UNPC_SocialBehaviorSystem::UNPC_SocialBehaviorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    SocialRank = ENPC_SocialRank::Gatherer;
    SocialRadius = 1500.0f;
    AggressionLevel = 0.3f;
    CooperationLevel = 0.7f;
    bIsTribalMember = true;
    TerritoryRadius = 5000.0f;
    SocialUpdateInterval = 1.0f;
}

void UNPC_SocialBehaviorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize tribal territory based on spawn location
    if (APawn* Owner = Cast<APawn>(GetOwner()))
    {
        TribalTerritory = Owner->GetActorLocation();
    }
    
    LastSocialUpdate = GetWorld()->GetTimeSeconds();
}

void UNPC_SocialBehaviorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update social awareness periodically
    if (CurrentTime - LastSocialUpdate >= SocialUpdateInterval)
    {
        UpdateSocialAwareness();
        LastSocialUpdate = CurrentTime;
    }
    
    // Forget old memories periodically
    if (FMath::Fmod(CurrentTime, 30.0f) < DeltaTime)
    {
        ForgetOldMemories(300.0f);
    }
}

void UNPC_SocialBehaviorSystem::UpdateSocialAwareness()
{
    ProcessNearbyNPCs();
    UpdateTribalBehavior();
}

void UNPC_SocialBehaviorSystem::ProcessNearbyNPCs()
{
    TArray<APawn*> NearbyNPCs = FindNearbyNPCs();
    
    for (APawn* NPC : NearbyNPCs)
    {
        if (!NPC || NPC == GetOwner())
            continue;
            
        // Update or create relationship
        FNPC_SocialRelationship* Relationship = GetRelationship(NPC);
        if (!Relationship)
        {
            FNPC_SocialRelationship NewRelationship;
            NewRelationship.TargetNPC = NPC;
            NewRelationship.TrustLevel = 50.0f; // Neutral starting trust
            KnownNPCs.Add(NewRelationship);
            Relationship = &KnownNPCs.Last();
        }
        
        // Decide interaction based on relationship and context
        ENPC_SocialInteraction InteractionType = DecideInteractionType(NPC);
        
        // Update relationship based on proximity and time
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), NPC->GetActorLocation());
        if (Distance < SocialRadius * 0.5f)
        {
            // Close proximity increases familiarity
            UpdateRelationship(NPC, 0.1f, -0.05f);
        }
    }
}

void UNPC_SocialBehaviorSystem::UpdateTribalBehavior()
{
    if (!bIsTribalMember)
        return;
        
    APawn* Owner = Cast<APawn>(GetOwner());
    if (!Owner)
        return;
        
    // Check if we're in tribal territory
    bool bInTerritory = IsInTribalTerritory(Owner->GetActorLocation());
    
    if (!bInTerritory && SocialRank != ENPC_SocialRank::Leader)
    {
        // Move towards tribal territory if we're not a leader
        FVector DirectionToTerritory = (TribalTerritory - Owner->GetActorLocation()).GetSafeNormal();
        // This would typically trigger movement behavior in the AI controller
    }
    
    // Leaders establish new territory if needed
    if (SocialRank == ENPC_SocialRank::Leader)
    {
        TribalTerritory = Owner->GetActorLocation();
    }
}

TArray<APawn*> UNPC_SocialBehaviorSystem::FindNearbyNPCs()
{
    TArray<APawn*> NearbyNPCs;
    
    if (!GetOwner())
        return NearbyNPCs;
        
    UWorld* World = GetWorld();
    if (!World)
        return NearbyNPCs;
        
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Find all pawns in the world
    TArray<AActor*> AllPawns;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), AllPawns);
    
    for (AActor* Actor : AllPawns)
    {
        APawn* Pawn = Cast<APawn>(Actor);
        if (!Pawn || Pawn == GetOwner())
            continue;
            
        float Distance = FVector::Dist(OwnerLocation, Pawn->GetActorLocation());
        if (Distance <= SocialRadius)
        {
            NearbyNPCs.Add(Pawn);
        }
    }
    
    return NearbyNPCs;
}

void UNPC_SocialBehaviorSystem::InitiateSocialInteraction(APawn* TargetNPC, ENPC_SocialInteraction InteractionType)
{
    if (!TargetNPC)
        return;
        
    // Remember this interaction
    RememberInteraction(TargetNPC, InteractionType, TargetNPC->GetActorLocation());
    
    // Update relationship based on interaction type
    switch (InteractionType)
    {
        case ENPC_SocialInteraction::Greeting:
            UpdateRelationship(TargetNPC, 0.5f, -0.2f);
            break;
        case ENPC_SocialInteraction::Warning:
            UpdateRelationship(TargetNPC, -0.2f, 0.3f);
            break;
        case ENPC_SocialInteraction::Trading:
            UpdateRelationship(TargetNPC, 1.0f, -0.5f);
            break;
        case ENPC_SocialInteraction::Sharing:
            UpdateRelationship(TargetNPC, 1.5f, -0.3f);
            break;
        case ENPC_SocialInteraction::Conflict:
            UpdateRelationship(TargetNPC, -2.0f, 1.0f);
            break;
        case ENPC_SocialInteraction::Cooperation:
            UpdateRelationship(TargetNPC, 2.0f, -0.5f);
            break;
    }
}

void UNPC_SocialBehaviorSystem::UpdateRelationship(APawn* TargetNPC, float TrustChange, float FearChange)
{
    if (!TargetNPC)
        return;
        
    FNPC_SocialRelationship* Relationship = GetRelationship(TargetNPC);
    if (!Relationship)
    {
        FNPC_SocialRelationship NewRelationship;
        NewRelationship.TargetNPC = TargetNPC;
        KnownNPCs.Add(NewRelationship);
        Relationship = &KnownNPCs.Last();
    }
    
    Relationship->TrustLevel = FMath::Clamp(Relationship->TrustLevel + TrustChange, 0.0f, 100.0f);
    Relationship->FearLevel = FMath::Clamp(Relationship->FearLevel + FearChange, 0.0f, 100.0f);
    Relationship->InteractionCount++;
    Relationship->LastInteractionTime = GetWorld()->GetTimeSeconds();
}

FNPC_SocialRelationship* UNPC_SocialBehaviorSystem::GetRelationship(APawn* TargetNPC)
{
    if (!TargetNPC)
        return nullptr;
        
    for (FNPC_SocialRelationship& Relationship : KnownNPCs)
    {
        if (Relationship.TargetNPC == TargetNPC)
        {
            return &Relationship;
        }
    }
    
    return nullptr;
}

bool UNPC_SocialBehaviorSystem::IsInTribalTerritory(FVector Location)
{
    float Distance = FVector::Dist(Location, TribalTerritory);
    return Distance <= TerritoryRadius;
}

void UNPC_SocialBehaviorSystem::SetTribalLeader(APawn* Leader)
{
    TribalLeader = Leader;
    
    if (Leader && SocialRank != ENPC_SocialRank::Leader)
    {
        // Update tribal territory to leader's location
        TribalTerritory = Leader->GetActorLocation();
    }
}

void UNPC_SocialBehaviorSystem::RememberInteraction(APawn* TargetNPC, ENPC_SocialInteraction InteractionType, FVector Location)
{
    if (!TargetNPC)
        return;
        
    FNPC_SocialMemory Memory;
    Memory.LastInteractionType = InteractionType;
    Memory.LastSeenLocation = Location;
    Memory.MemoryStrength = 1.0f;
    Memory.TimeStamp = GetWorld()->GetTimeSeconds();
    
    SocialMemories.Add(TargetNPC, Memory);
}

FNPC_SocialMemory* UNPC_SocialBehaviorSystem::GetSocialMemory(APawn* TargetNPC)
{
    if (!TargetNPC)
        return nullptr;
        
    return SocialMemories.Find(TargetNPC);
}

void UNPC_SocialBehaviorSystem::ForgetOldMemories(float MaxAge)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    TArray<TWeakObjectPtr<APawn>> MemoriesToRemove;
    
    for (auto& MemoryPair : SocialMemories)
    {
        if (CurrentTime - MemoryPair.Value.TimeStamp > MaxAge)
        {
            MemoriesToRemove.Add(MemoryPair.Key);
        }
    }
    
    for (const TWeakObjectPtr<APawn>& NPCToForget : MemoriesToRemove)
    {
        SocialMemories.Remove(NPCToForget);
    }
}

ENPC_SocialInteraction UNPC_SocialBehaviorSystem::DecideInteractionType(APawn* TargetNPC)
{
    if (!TargetNPC)
        return ENPC_SocialInteraction::Greeting;
        
    FNPC_SocialRelationship* Relationship = GetRelationship(TargetNPC);
    
    if (!Relationship)
    {
        // First encounter - default to greeting
        return ENPC_SocialInteraction::Greeting;
    }
    
    // Decision based on trust and fear levels
    if (Relationship->FearLevel > 70.0f)
    {
        return ENPC_SocialInteraction::Warning;
    }
    else if (Relationship->TrustLevel > 80.0f)
    {
        return CooperationLevel > 0.7f ? ENPC_SocialInteraction::Cooperation : ENPC_SocialInteraction::Sharing;
    }
    else if (Relationship->TrustLevel < 20.0f && AggressionLevel > 0.6f)
    {
        return ENPC_SocialInteraction::Conflict;
    }
    else if (Relationship->TrustLevel > 60.0f)
    {
        return ENPC_SocialInteraction::Trading;
    }
    
    return ENPC_SocialInteraction::Greeting;
}

bool UNPC_SocialBehaviorSystem::ShouldApproach(APawn* TargetNPC)
{
    if (!TargetNPC)
        return false;
        
    FNPC_SocialRelationship* Relationship = GetRelationship(TargetNPC);
    
    if (!Relationship)
    {
        // Approach unknown NPCs based on social rank and personality
        return SocialRank >= ENPC_SocialRank::Hunter && CooperationLevel > 0.5f;
    }
    
    // Approach if trust is high and fear is low
    return Relationship->TrustLevel > 50.0f && Relationship->FearLevel < 30.0f;
}

bool UNPC_SocialBehaviorSystem::ShouldFlee(APawn* TargetNPC)
{
    if (!TargetNPC)
        return false;
        
    FNPC_SocialRelationship* Relationship = GetRelationship(TargetNPC);
    
    if (!Relationship)
    {
        // Flee from unknown NPCs if we're low rank and not aggressive
        return SocialRank <= ENPC_SocialRank::Gatherer && AggressionLevel < 0.3f;
    }
    
    // Flee if fear is very high or trust is very low
    return Relationship->FearLevel > 80.0f || (Relationship->TrustLevel < 10.0f && Relationship->FearLevel > 50.0f);
}

FVector UNPC_SocialBehaviorSystem::GetSafeLocation()
{
    APawn* Owner = Cast<APawn>(GetOwner());
    if (!Owner)
        return FVector::ZeroVector;
        
    FVector CurrentLocation = Owner->GetActorLocation();
    
    // If we're in tribal territory, stay here
    if (IsInTribalTerritory(CurrentLocation))
    {
        return TribalTerritory;
    }
    
    // Otherwise, head towards tribal territory
    return TribalTerritory;
}

float UNPC_SocialBehaviorSystem::CalculateRelationshipScore(APawn* TargetNPC)
{
    FNPC_SocialRelationship* Relationship = GetRelationship(TargetNPC);
    if (!Relationship)
        return 0.0f;
        
    // Calculate overall relationship score
    float Score = Relationship->TrustLevel - Relationship->FearLevel;
    
    // Bonus for frequent interactions
    if (Relationship->InteractionCount > 5)
    {
        Score += 10.0f;
    }
    
    return FMath::Clamp(Score, -100.0f, 100.0f);
}

bool UNPC_SocialBehaviorSystem::IsHostileTowards(APawn* TargetNPC)
{
    float RelationshipScore = CalculateRelationshipScore(TargetNPC);
    return RelationshipScore < -50.0f || (AggressionLevel > 0.8f && RelationshipScore < 20.0f);
}