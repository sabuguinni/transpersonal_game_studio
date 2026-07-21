#include "NPC_TribalSocialSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"

UNPC_TribalSocialSystem::UNPC_TribalSocialSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Tick twice per second for social updates
    
    // Initialize default tribal role
    CurrentRole.RoleName = TEXT("Gatherer");
    CurrentRole.RolePriority = 1;
    CurrentRole.Authority = 10.0f;
    CurrentRole.ResponsibleTasks.Add(TEXT("Collect berries"));
    CurrentRole.ResponsibleTasks.Add(TEXT("Find water"));
    CurrentRole.ResponsibleTasks.Add(TEXT("Gather wood"));
    
    // Default personality traits
    Sociability = 50.0f;
    Aggression = 20.0f;
    Empathy = 60.0f;
    Leadership = 30.0f;
    
    TribalGroupName = TEXT("MainTribe");
    TribalGroupID = 1;
}

void UNPC_TribalSocialSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize social system
    LastSocialUpdate = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("NPC Tribal Social System initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UNPC_TribalSocialSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update social relationships periodically
    if (CurrentTime - LastSocialUpdate >= SocialUpdateInterval)
    {
        UpdateRelationshipDecay(DeltaTime);
        ProcessSocialInteractions();
        ForgetOldMemories(300.0f); // Forget memories older than 5 minutes
        LastSocialUpdate = CurrentTime;
    }
}

void UNPC_TribalSocialSystem::AddRelationship(AActor* TargetActor, float InitialStrength)
{
    if (!TargetActor || TargetActor == GetOwner())
    {
        return;
    }
    
    FNPC_SocialRelationship NewRelationship;
    NewRelationship.TargetActor = TargetActor;
    NewRelationship.RelationshipStrength = FMath::Clamp(InitialStrength, -100.0f, 100.0f);
    NewRelationship.Trust = 50.0f;
    NewRelationship.Respect = 50.0f;
    NewRelationship.LastInteractionTime = GetWorld()->GetTimeSeconds();
    
    SocialRelationships.Add(TargetActor, NewRelationship);
    
    UE_LOG(LogTemp, Log, TEXT("%s established relationship with %s (Strength: %.1f)"), 
           *GetOwner()->GetName(), *TargetActor->GetName(), InitialStrength);
}

void UNPC_TribalSocialSystem::ModifyRelationship(AActor* TargetActor, float StrengthDelta, float TrustDelta, float RespectDelta)
{
    if (!TargetActor)
    {
        return;
    }
    
    // Find or create relationship
    FNPC_SocialRelationship* Relationship = SocialRelationships.Find(TargetActor);
    if (!Relationship)
    {
        AddRelationship(TargetActor, 0.0f);
        Relationship = SocialRelationships.Find(TargetActor);
    }
    
    if (Relationship)
    {
        Relationship->RelationshipStrength = FMath::Clamp(
            Relationship->RelationshipStrength + StrengthDelta, -100.0f, 100.0f);
        Relationship->Trust = FMath::Clamp(
            Relationship->Trust + TrustDelta, 0.0f, 100.0f);
        Relationship->Respect = FMath::Clamp(
            Relationship->Respect + RespectDelta, 0.0f, 100.0f);
        Relationship->LastInteractionTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, Log, TEXT("%s relationship with %s modified: Strength=%.1f, Trust=%.1f, Respect=%.1f"), 
               *GetOwner()->GetName(), *TargetActor->GetName(), 
               Relationship->RelationshipStrength, Relationship->Trust, Relationship->Respect);
    }
}

FNPC_SocialRelationship UNPC_TribalSocialSystem::GetRelationship(AActor* TargetActor) const
{
    if (const FNPC_SocialRelationship* Relationship = SocialRelationships.Find(TargetActor))
    {
        return *Relationship;
    }
    
    // Return default neutral relationship
    FNPC_SocialRelationship DefaultRelationship;
    return DefaultRelationship;
}

TArray<AActor*> UNPC_TribalSocialSystem::GetAllKnownActors() const
{
    TArray<AActor*> KnownActors;
    
    for (const auto& Pair : SocialRelationships)
    {
        if (Pair.Key.IsValid())
        {
            KnownActors.Add(Pair.Key.Get());
        }
    }
    
    return KnownActors;
}

void UNPC_TribalSocialSystem::SetTribalRole(const FNPC_TribalRole& NewRole)
{
    CurrentRole = NewRole;
    
    UE_LOG(LogTemp, Log, TEXT("%s assigned new tribal role: %s (Authority: %.1f)"), 
           *GetOwner()->GetName(), *NewRole.RoleName, NewRole.Authority);
    
    // Broadcast role change to nearby tribal members
    BroadcastSocialEvent(TEXT("RoleChange"));
}

void UNPC_TribalSocialSystem::AddTaskToRole(const FString& TaskName)
{
    if (!CurrentRole.ResponsibleTasks.Contains(TaskName))
    {
        CurrentRole.ResponsibleTasks.Add(TaskName);
        
        UE_LOG(LogTemp, Log, TEXT("%s learned new task: %s"), 
               *GetOwner()->GetName(), *TaskName);
    }
}

bool UNPC_TribalSocialSystem::CanPerformTask(const FString& TaskName) const
{
    return CurrentRole.ResponsibleTasks.Contains(TaskName);
}

void UNPC_TribalSocialSystem::RecordSocialInteraction(AActor* TargetActor, const FString& Action, bool bWasThreatening, bool bWasHelpful)
{
    if (!TargetActor)
    {
        return;
    }
    
    FNPC_SocialMemory Memory;
    Memory.LastSeenLocation = TargetActor->GetActorLocation();
    Memory.LastSeenTime = GetWorld()->GetTimeSeconds();
    Memory.LastAction = Action;
    Memory.bWasThreatening = bWasThreatening;
    Memory.bWasHelpful = bWasHelpful;
    
    SocialMemories.Add(TargetActor, Memory);
    
    // Modify relationship based on interaction
    float StrengthDelta = 0.0f;
    float TrustDelta = 0.0f;
    float RespectDelta = 0.0f;
    
    if (bWasHelpful)
    {
        StrengthDelta += 5.0f;
        TrustDelta += 3.0f;
        RespectDelta += 2.0f;
    }
    
    if (bWasThreatening)
    {
        StrengthDelta -= 10.0f;
        TrustDelta -= 5.0f;
        RespectDelta -= 3.0f;
    }
    
    ModifyRelationship(TargetActor, StrengthDelta, TrustDelta, RespectDelta);
    
    UE_LOG(LogTemp, Log, TEXT("%s recorded interaction with %s: %s (Threatening: %s, Helpful: %s)"), 
           *GetOwner()->GetName(), *TargetActor->GetName(), *Action,
           bWasThreatening ? TEXT("Yes") : TEXT("No"),
           bWasHelpful ? TEXT("Yes") : TEXT("No"));
}

FNPC_SocialMemory UNPC_TribalSocialSystem::GetSocialMemory(AActor* TargetActor) const
{
    if (const FNPC_SocialMemory* Memory = SocialMemories.Find(TargetActor))
    {
        return *Memory;
    }
    
    return FNPC_SocialMemory();
}

void UNPC_TribalSocialSystem::ForgetOldMemories(float MaxAge)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    TArray<TWeakObjectPtr<AActor>> ToRemove;
    
    for (auto& Pair : SocialMemories)
    {
        if (CurrentTime - Pair.Value.LastSeenTime > MaxAge)
        {
            ToRemove.Add(Pair.Key);
        }
    }
    
    for (const auto& ActorPtr : ToRemove)
    {
        SocialMemories.Remove(ActorPtr);
        // Also decay the relationship
        if (FNPC_SocialRelationship* Relationship = SocialRelationships.Find(ActorPtr))
        {
            Relationship->RelationshipStrength *= 0.8f; // Decay by 20%
        }
    }
    
    if (ToRemove.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("%s forgot %d old memories"), 
               *GetOwner()->GetName(), ToRemove.Num());
    }
}

bool UNPC_TribalSocialSystem::ShouldObeyActor(AActor* TargetActor) const
{
    if (!TargetActor)
    {
        return false;
    }
    
    // Check if target has higher authority
    if (UNPC_TribalSocialSystem* TargetSocial = TargetActor->FindComponentByClass<UNPC_TribalSocialSystem>())
    {
        if (TargetSocial->GetAuthorityLevel() > GetAuthorityLevel())
        {
            // Also consider relationship - we obey those we respect
            FNPC_SocialRelationship Relationship = GetRelationship(TargetActor);
            return Relationship.Respect > 60.0f;
        }
    }
    
    return false;
}

AActor* UNPC_TribalSocialSystem::FindTribalLeader() const
{
    AActor* Leader = nullptr;
    float HighestAuthority = 0.0f;
    
    TArray<AActor*> NearbyMembers = GetNearbyTribalMembers(2000.0f);
    
    for (AActor* Member : NearbyMembers)
    {
        if (UNPC_TribalSocialSystem* MemberSocial = Member->FindComponentByClass<UNPC_TribalSocialSystem>())
        {
            if (MemberSocial->GetAuthorityLevel() > HighestAuthority)
            {
                HighestAuthority = MemberSocial->GetAuthorityLevel();
                Leader = Member;
            }
        }
    }
    
    return Leader;
}

bool UNPC_TribalSocialSystem::ShouldApproachActor(AActor* TargetActor) const
{
    if (!TargetActor)
    {
        return false;
    }
    
    FNPC_SocialRelationship Relationship = GetRelationship(TargetActor);
    
    // Approach if we like them and they're not threatening
    bool bPositiveRelationship = Relationship.RelationshipStrength > 20.0f;
    bool bTrustworthy = Relationship.Trust > 40.0f;
    bool bHighSociability = Sociability > 60.0f;
    
    FNPC_SocialMemory Memory = GetSocialMemory(TargetActor);
    bool bNotThreatening = !Memory.bWasThreatening;
    
    return (bPositiveRelationship && bTrustworthy) || (bHighSociability && bNotThreatening);
}

bool UNPC_TribalSocialSystem::ShouldAvoidActor(AActor* TargetActor) const
{
    if (!TargetActor)
    {
        return false;
    }
    
    FNPC_SocialRelationship Relationship = GetRelationship(TargetActor);
    FNPC_SocialMemory Memory = GetSocialMemory(TargetActor);
    
    // Avoid if relationship is very negative or they were threatening
    bool bNegativeRelationship = Relationship.RelationshipStrength < -30.0f;
    bool bUntrustworthy = Relationship.Trust < 20.0f;
    bool bWasThreatening = Memory.bWasThreatening;
    
    return bNegativeRelationship || bUntrustworthy || bWasThreatening;
}

float UNPC_TribalSocialSystem::GetSocialComfort(AActor* TargetActor) const
{
    if (!TargetActor)
    {
        return 0.0f;
    }
    
    FNPC_SocialRelationship Relationship = GetRelationship(TargetActor);
    
    // Comfort is based on relationship strength, trust, and personality
    float Comfort = (Relationship.RelationshipStrength + 100.0f) / 2.0f; // Convert -100/100 to 0/100
    Comfort = (Comfort + Relationship.Trust) / 2.0f;
    
    // Modify by personality traits
    if (IsActorInSameTribe(TargetActor))
    {
        Comfort += 20.0f; // Bonus for same tribe
    }
    
    return FMath::Clamp(Comfort, 0.0f, 100.0f);
}

TArray<AActor*> UNPC_TribalSocialSystem::GetNearbyTribalMembers(float Radius) const
{
    TArray<AActor*> NearbyMembers;
    
    if (!GetOwner())
    {
        return NearbyMembers;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return NearbyMembers;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Find all actors with tribal social systems
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor != GetOwner())
        {
            if (UNPC_TribalSocialSystem* TribalSystem = Actor->FindComponentByClass<UNPC_TribalSocialSystem>())
            {
                float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
                if (Distance <= Radius && IsActorInSameTribe(Actor))
                {
                    NearbyMembers.Add(Actor);
                }
            }
        }
    }
    
    return NearbyMembers;
}

void UNPC_TribalSocialSystem::CallForHelp(const FVector& DangerLocation)
{
    TArray<AActor*> NearbyMembers = GetNearbyTribalMembers(1500.0f);
    
    for (AActor* Member : NearbyMembers)
    {
        if (UNPC_TribalSocialSystem* MemberSocial = Member->FindComponentByClass<UNPC_TribalSocialSystem>())
        {
            // Record that we called for help
            MemberSocial->RecordSocialInteraction(GetOwner(), TEXT("Called for help"), false, true);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("%s called for help! Danger at location: %s"), 
           *GetOwner()->GetName(), *DangerLocation.ToString());
    
    BroadcastSocialEvent(TEXT("CallForHelp"));
}

void UNPC_TribalSocialSystem::ShareInformation(const FString& Information, float Radius)
{
    TArray<AActor*> NearbyMembers = GetNearbyTribalMembers(Radius);
    
    for (AActor* Member : NearbyMembers)
    {
        if (UNPC_TribalSocialSystem* MemberSocial = Member->FindComponentByClass<UNPC_TribalSocialSystem>())
        {
            MemberSocial->RecordSocialInteraction(GetOwner(), 
                FString::Printf(TEXT("Shared info: %s"), *Information), false, true);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s shared information with %d nearby members: %s"), 
           *GetOwner()->GetName(), NearbyMembers.Num(), *Information);
}

void UNPC_TribalSocialSystem::UpdateRelationshipDecay(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (auto& Pair : SocialRelationships)
    {
        FNPC_SocialRelationship& Relationship = Pair.Value;
        
        // Decay relationships over time if no recent interaction
        float TimeSinceInteraction = CurrentTime - Relationship.LastInteractionTime;
        if (TimeSinceInteraction > 60.0f) // 1 minute
        {
            float DecayRate = 0.1f * DeltaTime; // Small decay per second
            
            // Positive relationships decay towards neutral
            if (Relationship.RelationshipStrength > 0.0f)
            {
                Relationship.RelationshipStrength = FMath::Max(0.0f, 
                    Relationship.RelationshipStrength - DecayRate);
            }
            // Negative relationships also decay towards neutral (forgiveness)
            else if (Relationship.RelationshipStrength < 0.0f)
            {
                Relationship.RelationshipStrength = FMath::Min(0.0f, 
                    Relationship.RelationshipStrength + DecayRate);
            }
        }
    }
}

void UNPC_TribalSocialSystem::ProcessSocialInteractions()
{
    if (!GetOwner())
    {
        return;
    }
    
    // Look for nearby actors to potentially interact with
    TArray<AActor*> NearbyMembers = GetNearbyTribalMembers(500.0f);
    
    for (AActor* Member : NearbyMembers)
    {
        if (ShouldApproachActor(Member))
        {
            // Positive social interaction
            RecordSocialInteraction(Member, TEXT("Friendly approach"), false, true);
        }
        else if (ShouldAvoidActor(Member))
        {
            // Record avoidance
            RecordSocialInteraction(Member, TEXT("Avoided"), false, false);
        }
    }
}

bool UNPC_TribalSocialSystem::IsActorInSameTribe(AActor* TargetActor) const
{
    if (!TargetActor)
    {
        return false;
    }
    
    if (UNPC_TribalSocialSystem* TargetSocial = TargetActor->FindComponentByClass<UNPC_TribalSocialSystem>())
    {
        return TargetSocial->TribalGroupID == TribalGroupID;
    }
    
    return false;
}

float UNPC_TribalSocialSystem::CalculateSocialDistance(AActor* TargetActor) const
{
    if (!TargetActor || !GetOwner())
    {
        return 10000.0f; // Very far
    }
    
    FNPC_SocialRelationship Relationship = GetRelationship(TargetActor);
    
    // Social distance is inverse of relationship strength
    float SocialDistance = 100.0f - ((Relationship.RelationshipStrength + 100.0f) / 2.0f);
    
    return SocialDistance;
}

void UNPC_TribalSocialSystem::BroadcastSocialEvent(const FString& EventType, AActor* TargetActor)
{
    // This could be extended to use a proper event system
    UE_LOG(LogTemp, Log, TEXT("%s broadcast social event: %s"), 
           *GetOwner()->GetName(), *EventType);
    
    // For now, just log the event
    // In a full implementation, this would notify other systems
}