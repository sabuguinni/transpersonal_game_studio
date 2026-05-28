#include "NPC_SocialSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"

UNPC_SocialSystem::UNPC_SocialSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update every 0.5 seconds
    
    CurrentRole = ENPC_SocialRole::None;
    SocialUpdateInterval = 1.0f;
    MaxSocialDistance = 5000.0f;
    MaxTrackedRelations = 20;
    PackFormationDistance = 1500.0f;
    LeaderFollowDistance = 800.0f;
    LastSocialUpdate = 0.0f;
}

void UNPC_SocialSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize pack data
    PackData = FNPC_PackData();
    
    // Set default role based on actor name or type
    if (GetOwner())
    {
        FString ActorName = GetOwner()->GetName();
        if (ActorName.Contains("Leader"))
        {
            SetSocialRole(ENPC_SocialRole::PackLeader);
        }
        else if (ActorName.Contains("Scout"))
        {
            SetSocialRole(ENPC_SocialRole::Scout);
        }
        else if (ActorName.Contains("Hunter"))
        {
            SetSocialRole(ENPC_SocialRole::Hunter);
        }
        else
        {
            SetSocialRole(ENPC_SocialRole::Gatherer);
        }
    }
}

void UNPC_SocialSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update social relations periodically
    if (CurrentTime - LastSocialUpdate >= SocialUpdateInterval)
    {
        UpdateSocialRelations();
        UpdatePackCohesion();
        CleanupOldRelations();
        LastSocialUpdate = CurrentTime;
    }
}

void UNPC_SocialSystem::SetSocialRole(ENPC_SocialRole NewRole)
{
    CurrentRole = NewRole;
    
    // Adjust behavior parameters based on role
    switch (CurrentRole)
    {
        case ENPC_SocialRole::PackLeader:
            MaxSocialDistance = 8000.0f;
            PackFormationDistance = 2000.0f;
            break;
        case ENPC_SocialRole::Scout:
            MaxSocialDistance = 12000.0f;
            PackFormationDistance = 3000.0f;
            break;
        case ENPC_SocialRole::Hunter:
            MaxSocialDistance = 6000.0f;
            PackFormationDistance = 1200.0f;
            break;
        case ENPC_SocialRole::Defender:
            MaxSocialDistance = 4000.0f;
            PackFormationDistance = 800.0f;
            break;
        default:
            MaxSocialDistance = 5000.0f;
            PackFormationDistance = 1500.0f;
            break;
    }
}

void UNPC_SocialSystem::UpdateRelationship(AActor* TargetActor, ENPC_SocialStatus NewStatus, float TrustChange)
{
    if (!TargetActor || TargetActor == GetOwner())
    {
        return;
    }
    
    FNPC_SocialRelation* Relation = FindOrCreateRelation(TargetActor);
    if (Relation)
    {
        Relation->Status = NewStatus;
        Relation->TrustLevel = FMath::Clamp(Relation->TrustLevel + TrustChange, -1.0f, 1.0f);
        Relation->LastInteractionTime = GetWorld()->GetTimeSeconds();
        Relation->InteractionCount++;
        
        // Update pack relationships if in pack
        if (PackData.bIsActive && NewStatus == ENPC_SocialStatus::Hostile)
        {
            BroadcastToPackMembers(FString::Printf(TEXT("Threat detected: %s"), *TargetActor->GetName()));
        }
    }
}

FNPC_SocialRelation UNPC_SocialSystem::GetRelationship(AActor* TargetActor) const
{
    for (const FNPC_SocialRelation& Relation : SocialRelations)
    {
        if (Relation.TargetActor.IsValid() && Relation.TargetActor.Get() == TargetActor)
        {
            return Relation;
        }
    }
    
    // Return default neutral relationship
    FNPC_SocialRelation DefaultRelation;
    DefaultRelation.TargetActor = TargetActor;
    DefaultRelation.Status = ENPC_SocialStatus::Neutral;
    return DefaultRelation;
}

TArray<AActor*> UNPC_SocialSystem::GetActorsWithStatus(ENPC_SocialStatus Status) const
{
    TArray<AActor*> Result;
    
    for (const FNPC_SocialRelation& Relation : SocialRelations)
    {
        if (Relation.TargetActor.IsValid() && Relation.Status == Status)
        {
            Result.Add(Relation.TargetActor.Get());
        }
    }
    
    return Result;
}

void UNPC_SocialSystem::JoinPack(const TArray<AActor*>& PackMembers, AActor* Leader)
{
    PackData.bIsActive = true;
    PackData.PackMembers.Empty();
    
    for (AActor* Member : PackMembers)
    {
        if (Member && Member != GetOwner())
        {
            PackData.PackMembers.Add(Member);
            
            // Establish friendly relationship with pack members
            UpdateRelationship(Member, ENPC_SocialStatus::Friendly, 0.3f);
        }
    }
    
    PackData.PackLeader = Leader;
    if (Leader)
    {
        UpdateRelationship(Leader, ENPC_SocialStatus::Respected, 0.5f);
    }
    
    // Calculate initial pack center
    if (PackData.PackMembers.Num() > 0)
    {
        FVector CenterSum = FVector::ZeroVector;
        int32 ValidMembers = 0;
        
        for (const TWeakObjectPtr<AActor>& Member : PackData.PackMembers)
        {
            if (Member.IsValid())
            {
                CenterSum += Member->GetActorLocation();
                ValidMembers++;
            }
        }
        
        if (ValidMembers > 0)
        {
            PackData.PackCenterLocation = CenterSum / ValidMembers;
        }
    }
    
    PackData.PackCohesion = 1.0f;
}

void UNPC_SocialSystem::LeavePack()
{
    PackData.bIsActive = false;
    PackData.PackMembers.Empty();
    PackData.PackLeader = nullptr;
    PackData.PackCohesion = 0.0f;
    
    // Become outcast
    SetSocialRole(ENPC_SocialRole::Outcast);
}

bool UNPC_SocialSystem::ShouldFollowLeader() const
{
    if (!PackData.bIsActive || !PackData.PackLeader.IsValid())
    {
        return false;
    }
    
    // Only non-leaders should follow
    if (CurrentRole == ENPC_SocialRole::PackLeader)
    {
        return false;
    }
    
    // Check distance to leader
    if (GetOwner())
    {
        float DistanceToLeader = FVector::Dist(GetOwner()->GetActorLocation(), PackData.PackLeader->GetActorLocation());
        return DistanceToLeader > LeaderFollowDistance;
    }
    
    return false;
}

bool UNPC_SocialSystem::ShouldDefendPackMember(AActor* Member) const
{
    if (!PackData.bIsActive || !Member)
    {
        return false;
    }
    
    // Check if member is in our pack
    for (const TWeakObjectPtr<AActor>& PackMember : PackData.PackMembers)
    {
        if (PackMember.IsValid() && PackMember.Get() == Member)
        {
            return true;
        }
    }
    
    return PackData.PackLeader.IsValid() && PackData.PackLeader.Get() == Member;
}

FVector UNPC_SocialSystem::GetPreferredPosition() const
{
    if (!GetOwner())
    {
        return FVector::ZeroVector;
    }
    
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    
    if (!PackData.bIsActive)
    {
        return CurrentLocation;
    }
    
    // Calculate preferred position based on role and pack dynamics
    FVector PreferredPosition = PackData.PackCenterLocation;
    
    switch (CurrentRole)
    {
        case ENPC_SocialRole::PackLeader:
            // Leaders stay at pack center
            PreferredPosition = PackData.PackCenterLocation;
            break;
        case ENPC_SocialRole::Scout:
            // Scouts stay on pack perimeter
            {
                FVector ToCenter = PackData.PackCenterLocation - CurrentLocation;
                ToCenter.Normalize();
                PreferredPosition = PackData.PackCenterLocation - (ToCenter * PackData.PackRadius * 0.8f);
            }
            break;
        case ENPC_SocialRole::Defender:
            // Defenders form outer ring
            {
                FVector ToCenter = PackData.PackCenterLocation - CurrentLocation;
                ToCenter.Normalize();
                PreferredPosition = PackData.PackCenterLocation - (ToCenter * PackData.PackRadius * 0.6f);
            }
            break;
        default:
            // Others stay close to center
            PreferredPosition = PackData.PackCenterLocation + FVector(
                FMath::RandRange(-PackData.PackRadius * 0.3f, PackData.PackRadius * 0.3f),
                FMath::RandRange(-PackData.PackRadius * 0.3f, PackData.PackRadius * 0.3f),
                0.0f
            );
            break;
    }
    
    return PreferredPosition;
}

void UNPC_SocialSystem::SendSocialSignal(const FString& SignalType, AActor* TargetActor)
{
    if (PackData.bIsActive)
    {
        BroadcastToPackMembers(SignalType);
    }
    
    // Trigger Blueprint event
    OnSocialSignalReceived(SignalType, GetOwner());
}

void UNPC_SocialSystem::UpdateSocialRelations()
{
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    // Find nearby actors and update relationships
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    for (AActor* Actor : NearbyActors)
    {
        if (!Actor || Actor == GetOwner())
        {
            continue;
        }
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        if (Distance <= MaxSocialDistance)
        {
            // Create or update relationship
            FNPC_SocialRelation* Relation = FindOrCreateRelation(Actor);
            if (Relation)
            {
                // Decay trust over time if no recent interactions
                float TimeSinceInteraction = GetWorld()->GetTimeSeconds() - Relation->LastInteractionTime;
                if (TimeSinceInteraction > 60.0f) // 1 minute
                {
                    Relation->TrustLevel *= 0.99f; // Slow decay
                }
            }
        }
    }
}

void UNPC_SocialSystem::UpdatePackCohesion()
{
    if (!PackData.bIsActive)
    {
        return;
    }
    
    // Calculate new pack center
    FVector CenterSum = FVector::ZeroVector;
    int32 ValidMembers = 0;
    
    for (const TWeakObjectPtr<AActor>& Member : PackData.PackMembers)
    {
        if (Member.IsValid())
        {
            CenterSum += Member->GetActorLocation();
            ValidMembers++;
        }
    }
    
    if (ValidMembers > 0)
    {
        PackData.PackCenterLocation = CenterSum / ValidMembers;
        
        // Calculate cohesion based on member distances
        float TotalDistance = 0.0f;
        for (const TWeakObjectPtr<AActor>& Member : PackData.PackMembers)
        {
            if (Member.IsValid())
            {
                TotalDistance += FVector::Dist(Member->GetActorLocation(), PackData.PackCenterLocation);
            }
        }
        
        float AverageDistance = TotalDistance / ValidMembers;
        PackData.PackCohesion = FMath::Clamp(1.0f - (AverageDistance / PackData.PackRadius), 0.0f, 1.0f);
    }
    else
    {
        // No valid members, disband pack
        LeavePack();
    }
}

void UNPC_SocialSystem::CleanupOldRelations()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove relations that are too old or invalid
    SocialRelations.RemoveAll([CurrentTime](const FNPC_SocialRelation& Relation)
    {
        if (!Relation.TargetActor.IsValid())
        {
            return true;
        }
        
        // Remove relations older than 5 minutes with no interactions
        float TimeSinceInteraction = CurrentTime - Relation.LastInteractionTime;
        return TimeSinceInteraction > 300.0f && Relation.InteractionCount == 0;
    });
    
    // Limit number of tracked relations
    if (SocialRelations.Num() > MaxTrackedRelations)
    {
        // Sort by interaction count and remove least interacted
        SocialRelations.Sort([](const FNPC_SocialRelation& A, const FNPC_SocialRelation& B)
        {
            return A.InteractionCount > B.InteractionCount;
        });
        
        SocialRelations.SetNum(MaxTrackedRelations);
    }
}

FNPC_SocialRelation* UNPC_SocialSystem::FindOrCreateRelation(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return nullptr;
    }
    
    // Find existing relation
    for (FNPC_SocialRelation& Relation : SocialRelations)
    {
        if (Relation.TargetActor.IsValid() && Relation.TargetActor.Get() == TargetActor)
        {
            return &Relation;
        }
    }
    
    // Create new relation if we have space
    if (SocialRelations.Num() < MaxTrackedRelations)
    {
        FNPC_SocialRelation NewRelation;
        NewRelation.TargetActor = TargetActor;
        NewRelation.Status = ENPC_SocialStatus::Neutral;
        NewRelation.TrustLevel = 0.0f;
        NewRelation.LastInteractionTime = GetWorld()->GetTimeSeconds();
        NewRelation.InteractionCount = 0;
        
        SocialRelations.Add(NewRelation);
        return &SocialRelations.Last();
    }
    
    return nullptr;
}

void UNPC_SocialSystem::BroadcastToPackMembers(const FString& Message)
{
    if (!PackData.bIsActive)
    {
        return;
    }
    
    for (const TWeakObjectPtr<AActor>& Member : PackData.PackMembers)
    {
        if (Member.IsValid())
        {
            // Try to find social system on pack member
            if (UNPC_SocialSystem* MemberSocial = Member->FindComponentByClass<UNPC_SocialSystem>())
            {
                MemberSocial->OnSocialSignalReceived(Message, GetOwner());
            }
        }
    }
}