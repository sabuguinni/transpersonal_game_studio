#include "Combat_PackBehaviorManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"

UCombat_PackBehaviorManager::UCombat_PackBehaviorManager()
{
    PackUpdateInterval = 0.5f;
    FormationUpdateInterval = 1.0f;
    DefaultFormationRadius = 500.0f;
    FormationTolerance = 150.0f;
    PackCohesionRadius = 1000.0f;
    RetreatHealthThreshold = 0.3f;
    MaxPackSize = 8.0f;
    LastPackUpdate = 0.0f;
    LastFormationUpdate = 0.0f;
    NextPackID = 1;
}

void UCombat_PackBehaviorManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Combat Pack Behavior Manager initialized"));
    
    // Clear any existing packs
    ActivePacks.Empty();
    NextPackID = 1;
}

void UCombat_PackBehaviorManager::Deinitialize()
{
    // Clean up all packs
    ActivePacks.Empty();
    
    Super::Deinitialize();
}

void UCombat_PackBehaviorManager::Tick(float DeltaTime)
{
    if (!GetWorld())
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update pack states
    if (CurrentTime - LastPackUpdate >= PackUpdateInterval)
    {
        UpdatePackStates(DeltaTime);
        LastPackUpdate = CurrentTime;
    }
    
    // Update formations
    if (CurrentTime - LastFormationUpdate >= FormationUpdateInterval)
    {
        UpdatePackFormations(DeltaTime);
        LastFormationUpdate = CurrentTime;
    }
    
    // Clean up invalid packs
    CleanupInvalidPacks();
}

FString UCombat_PackBehaviorManager::CreatePack(const TArray<AActor*>& PackMembers, AActor* AlphaLeader)
{
    if (PackMembers.Num() == 0 || !AlphaLeader)
    {
        return FString();
    }
    
    FString PackID = FString::Printf(TEXT("Pack_%d"), NextPackID++);
    
    FCombat_PackData NewPack;
    NewPack.PackID = PackID;
    NewPack.AlphaLeader = AlphaLeader;
    NewPack.CurrentFormation = ECombat_PackFormation::Circle;
    NewPack.FormationRadius = DefaultFormationRadius;
    
    // Add alpha leader first
    FCombat_PackMember AlphaMember;
    AlphaMember.Actor = AlphaLeader;
    AlphaMember.Role = ECombat_PackRole::Alpha;
    AlphaMember.HealthPercentage = 1.0f;
    AlphaMember.StaminaPercentage = 1.0f;
    AlphaMember.LastKnownPosition = AlphaLeader->GetActorLocation();
    AlphaMember.LastUpdateTime = GetWorld()->GetTimeSeconds();
    NewPack.Members.Add(AlphaMember);
    
    // Add other members
    for (AActor* Member : PackMembers)
    {
        if (Member && Member != AlphaLeader && IsValidPackMember(Member))
        {
            FCombat_PackMember PackMember;
            PackMember.Actor = Member;
            PackMember.Role = ECombat_PackRole::Hunter;
            PackMember.HealthPercentage = 1.0f;
            PackMember.StaminaPercentage = 1.0f;
            PackMember.LastKnownPosition = Member->GetActorLocation();
            PackMember.LastUpdateTime = GetWorld()->GetTimeSeconds();
            NewPack.Members.Add(PackMember);
            
            if (NewPack.Members.Num() >= MaxPackSize)
            {
                break;
            }
        }
    }
    
    // Set formation center to alpha position
    if (AlphaLeader)
    {
        NewPack.FormationCenter = AlphaLeader->GetActorLocation();
    }
    
    ActivePacks.Add(PackID, NewPack);
    
    UE_LOG(LogTemp, Warning, TEXT("Created pack %s with %d members"), *PackID, NewPack.Members.Num());
    
    return PackID;
}

bool UCombat_PackBehaviorManager::AddMemberToPack(const FString& PackID, AActor* NewMember, ECombat_PackRole Role)
{
    if (!NewMember || !IsValidPackMember(NewMember))
    {
        return false;
    }
    
    FCombat_PackData* Pack = ActivePacks.Find(PackID);
    if (!Pack || Pack->Members.Num() >= MaxPackSize)
    {
        return false;
    }
    
    // Check if member is already in pack
    for (const FCombat_PackMember& Member : Pack->Members)
    {
        if (Member.Actor == NewMember)
        {
            return false;
        }
    }
    
    FCombat_PackMember PackMember;
    PackMember.Actor = NewMember;
    PackMember.Role = Role;
    PackMember.HealthPercentage = 1.0f;
    PackMember.StaminaPercentage = 1.0f;
    PackMember.LastKnownPosition = NewMember->GetActorLocation();
    PackMember.LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    Pack->Members.Add(PackMember);
    
    return true;
}

bool UCombat_PackBehaviorManager::RemoveMemberFromPack(const FString& PackID, AActor* Member)
{
    FCombat_PackData* Pack = ActivePacks.Find(PackID);
    if (!Pack || !Member)
    {
        return false;
    }
    
    for (int32 i = Pack->Members.Num() - 1; i >= 0; i--)
    {
        if (Pack->Members[i].Actor == Member)
        {
            Pack->Members.RemoveAt(i);
            
            // If this was the alpha, promote a new one
            if (Pack->AlphaLeader == Member && Pack->Members.Num() > 0)
            {
                Pack->AlphaLeader = Pack->Members[0].Actor;
                Pack->Members[0].Role = ECombat_PackRole::Alpha;
            }
            
            return true;
        }
    }
    
    return false;
}

void UCombat_PackBehaviorManager::DisbandPack(const FString& PackID)
{
    ActivePacks.Remove(PackID);
}

bool UCombat_PackBehaviorManager::SetPackFormation(const FString& PackID, ECombat_PackFormation Formation, const FVector& Center)
{
    FCombat_PackData* Pack = ActivePacks.Find(PackID);
    if (!Pack)
    {
        return false;
    }
    
    Pack->CurrentFormation = Formation;
    Pack->FormationCenter = Center;
    Pack->LastFormationUpdate = GetWorld()->GetTimeSeconds();
    
    return true;
}

FVector UCombat_PackBehaviorManager::GetFormationPositionForMember(const FString& PackID, AActor* Member)
{
    FCombat_PackData* Pack = ActivePacks.Find(PackID);
    if (!Pack || !Member)
    {
        return FVector::ZeroVector;
    }
    
    // Find member index
    int32 MemberIndex = -1;
    for (int32 i = 0; i < Pack->Members.Num(); i++)
    {
        if (Pack->Members[i].Actor == Member)
        {
            MemberIndex = i;
            break;
        }
    }
    
    if (MemberIndex == -1)
    {
        return FVector::ZeroVector;
    }
    
    // Calculate position based on formation type
    switch (Pack->CurrentFormation)
    {
        case ECombat_PackFormation::Circle:
            return CalculateCircleFormationPosition(*Pack, MemberIndex);
        case ECombat_PackFormation::Line:
            return CalculateLineFormationPosition(*Pack, MemberIndex);
        case ECombat_PackFormation::Wedge:
            return CalculateWedgeFormationPosition(*Pack, MemberIndex);
        case ECombat_PackFormation::Ambush:
            return CalculateAmbushFormationPosition(*Pack, MemberIndex);
        case ECombat_PackFormation::Scatter:
            // Random position around center
            {
                FVector RandomOffset = FVector(
                    FMath::RandRange(-Pack->FormationRadius, Pack->FormationRadius),
                    FMath::RandRange(-Pack->FormationRadius, Pack->FormationRadius),
                    0.0f
                );
                return Pack->FormationCenter + RandomOffset;
            }
        default:
            return Pack->FormationCenter;
    }
}

bool UCombat_PackBehaviorManager::IsPackInFormation(const FString& PackID, float ToleranceRadius)
{
    FCombat_PackData* Pack = ActivePacks.Find(PackID);
    if (!Pack)
    {
        return false;
    }
    
    int32 MembersInPosition = 0;
    
    for (int32 i = 0; i < Pack->Members.Num(); i++)
    {
        if (Pack->Members[i].Actor.IsValid())
        {
            FVector TargetPosition = GetFormationPositionForMember(PackID, Pack->Members[i].Actor.Get());
            FVector ActualPosition = Pack->Members[i].Actor->GetActorLocation();
            
            float Distance = FVector::Dist(TargetPosition, ActualPosition);
            if (Distance <= ToleranceRadius)
            {
                MembersInPosition++;
            }
        }
    }
    
    // Consider pack in formation if 75% of members are in position
    return MembersInPosition >= (Pack->Members.Num() * 0.75f);
}

bool UCombat_PackBehaviorManager::SetPackTarget(const FString& PackID, AActor* Target)
{
    FCombat_PackData* Pack = ActivePacks.Find(PackID);
    if (!Pack)
    {
        return false;
    }
    
    Pack->PrimaryTarget = Target;
    Pack->bIsHunting = (Target != nullptr);
    
    return true;
}

AActor* UCombat_PackBehaviorManager::GetPackTarget(const FString& PackID)
{
    FCombat_PackData* Pack = ActivePacks.Find(PackID);
    if (!Pack)
    {
        return nullptr;
    }
    
    return Pack->PrimaryTarget.Get();
}

TArray<AActor*> UCombat_PackBehaviorManager::GetPackMembers(const FString& PackID)
{
    TArray<AActor*> Members;
    
    FCombat_PackData* Pack = ActivePacks.Find(PackID);
    if (Pack)
    {
        for (const FCombat_PackMember& Member : Pack->Members)
        {
            if (Member.Actor.IsValid())
            {
                Members.Add(Member.Actor.Get());
            }
        }
    }
    
    return Members;
}

AActor* UCombat_PackBehaviorManager::GetPackAlpha(const FString& PackID)
{
    FCombat_PackData* Pack = ActivePacks.Find(PackID);
    if (!Pack)
    {
        return nullptr;
    }
    
    return Pack->AlphaLeader.Get();
}

FString UCombat_PackBehaviorManager::FindPackForActor(AActor* Actor)
{
    if (!Actor)
    {
        return FString();
    }
    
    for (const auto& PackPair : ActivePacks)
    {
        const FCombat_PackData& Pack = PackPair.Value;
        for (const FCombat_PackMember& Member : Pack.Members)
        {
            if (Member.Actor == Actor)
            {
                return PackPair.Key;
            }
        }
    }
    
    return FString();
}

ECombat_PackRole UCombat_PackBehaviorManager::GetActorPackRole(AActor* Actor)
{
    if (!Actor)
    {
        return ECombat_PackRole::Hunter;
    }
    
    for (const auto& PackPair : ActivePacks)
    {
        const FCombat_PackData& Pack = PackPair.Value;
        for (const FCombat_PackMember& Member : Pack.Members)
        {
            if (Member.Actor == Actor)
            {
                return Member.Role;
            }
        }
    }
    
    return ECombat_PackRole::Hunter;
}

float UCombat_PackBehaviorManager::GetPackCohesion(const FString& PackID)
{
    FCombat_PackData* Pack = ActivePacks.Find(PackID);
    if (!Pack || Pack->Members.Num() < 2)
    {
        return 1.0f;
    }
    
    FVector CenterOfMass = FVector::ZeroVector;
    int32 ValidMembers = 0;
    
    // Calculate center of mass
    for (const FCombat_PackMember& Member : Pack->Members)
    {
        if (Member.Actor.IsValid())
        {
            CenterOfMass += Member.Actor->GetActorLocation();
            ValidMembers++;
        }
    }
    
    if (ValidMembers == 0)
    {
        return 0.0f;
    }
    
    CenterOfMass /= ValidMembers;
    
    // Calculate average distance from center
    float TotalDistance = 0.0f;
    for (const FCombat_PackMember& Member : Pack->Members)
    {
        if (Member.Actor.IsValid())
        {
            TotalDistance += FVector::Dist(Member.Actor->GetActorLocation(), CenterOfMass);
        }
    }
    
    float AverageDistance = TotalDistance / ValidMembers;
    
    // Convert to cohesion value (1.0 = tight formation, 0.0 = scattered)
    return FMath::Clamp(1.0f - (AverageDistance / PackCohesionRadius), 0.0f, 1.0f);
}

bool UCombat_PackBehaviorManager::IsPackHealthy(const FString& PackID, float HealthThreshold)
{
    FCombat_PackData* Pack = ActivePacks.Find(PackID);
    if (!Pack)
    {
        return false;
    }
    
    float TotalHealth = 0.0f;
    int32 ValidMembers = 0;
    
    for (const FCombat_PackMember& Member : Pack->Members)
    {
        if (Member.Actor.IsValid())
        {
            TotalHealth += Member.HealthPercentage;
            ValidMembers++;
        }
    }
    
    if (ValidMembers == 0)
    {
        return false;
    }
    
    float AverageHealth = TotalHealth / ValidMembers;
    return AverageHealth >= HealthThreshold;
}

ECombat_PackFormation UCombat_PackBehaviorManager::GetOptimalFormation(const FString& PackID, AActor* Target)
{
    FCombat_PackData* Pack = ActivePacks.Find(PackID);
    if (!Pack || !Target)
    {
        return ECombat_PackFormation::Circle;
    }
    
    float DistanceToTarget = FVector::Dist(Pack->FormationCenter, Target->GetActorLocation());
    int32 PackSize = Pack->Members.Num();
    
    // Close range - circle for surrounding
    if (DistanceToTarget < 500.0f)
    {
        return ECombat_PackFormation::Circle;
    }
    // Medium range - wedge for approach
    else if (DistanceToTarget < 1500.0f)
    {
        return ECombat_PackFormation::Wedge;
    }
    // Long range - line for coordinated advance
    else
    {
        return ECombat_PackFormation::Line;
    }
}

bool UCombat_PackBehaviorManager::ShouldPackRetreat(const FString& PackID)
{
    FCombat_PackData* Pack = ActivePacks.Find(PackID);
    if (!Pack)
    {
        return false;
    }
    
    // Check pack health
    if (!IsPackHealthy(PackID, RetreatHealthThreshold))
    {
        return true;
    }
    
    // Check if alpha is down
    if (!Pack->AlphaLeader.IsValid())
    {
        return true;
    }
    
    // Check pack size
    int32 ValidMembers = 0;
    for (const FCombat_PackMember& Member : Pack->Members)
    {
        if (Member.Actor.IsValid())
        {
            ValidMembers++;
        }
    }
    
    // Retreat if pack is too small
    if (ValidMembers < 2)
    {
        return true;
    }
    
    return false;
}

AActor* UCombat_PackBehaviorManager::GetBestPackTarget(const FString& PackID, const TArray<AActor*>& PotentialTargets)
{
    FCombat_PackData* Pack = ActivePacks.Find(PackID);
    if (!Pack || PotentialTargets.Num() == 0)
    {
        return nullptr;
    }
    
    AActor* BestTarget = nullptr;
    float BestScore = -1.0f;
    
    for (AActor* Target : PotentialTargets)
    {
        if (!Target)
        {
            continue;
        }
        
        float DistanceToTarget = FVector::Dist(Pack->FormationCenter, Target->GetActorLocation());
        float ThreatLevel = CalculatePackThreatLevel(*Pack, Target);
        
        // Score based on distance (closer is better) and threat level (lower threat is better)
        float Score = (1.0f / (DistanceToTarget + 1.0f)) * (1.0f / (ThreatLevel + 0.1f));
        
        if (Score > BestScore)
        {
            BestScore = Score;
            BestTarget = Target;
        }
    }
    
    return BestTarget;
}

void UCombat_PackBehaviorManager::UpdatePackStates(float DeltaTime)
{
    for (auto& PackPair : ActivePacks)
    {
        FCombat_PackData& Pack = PackPair.Value;
        
        // Update member status
        for (FCombat_PackMember& Member : Pack.Members)
        {
            UpdateMemberStatus(Member);
        }
        
        // Update formation center to alpha position
        if (Pack.AlphaLeader.IsValid())
        {
            Pack.FormationCenter = Pack.AlphaLeader->GetActorLocation();
        }
    }
}

void UCombat_PackBehaviorManager::UpdatePackFormations(float DeltaTime)
{
    // Formation updates are handled by individual AI when they query formation positions
}

void UCombat_PackBehaviorManager::UpdatePackCombat(float DeltaTime)
{
    for (auto& PackPair : ActivePacks)
    {
        FCombat_PackData& Pack = PackPair.Value;
        
        // Check if pack should retreat
        if (ShouldPackRetreat(PackPair.Key))
        {
            Pack.bIsDefending = true;
            Pack.bIsHunting = false;
            Pack.PrimaryTarget = nullptr;
        }
    }
}

void UCombat_PackBehaviorManager::CleanupInvalidPacks()
{
    TArray<FString> PacksToRemove;
    
    for (auto& PackPair : ActivePacks)
    {
        FCombat_PackData& Pack = PackPair.Value;
        
        // Remove invalid members
        for (int32 i = Pack.Members.Num() - 1; i >= 0; i--)
        {
            if (!Pack.Members[i].Actor.IsValid())
            {
                Pack.Members.RemoveAt(i);
            }
        }
        
        // Mark pack for removal if no valid members
        if (Pack.Members.Num() == 0)
        {
            PacksToRemove.Add(PackPair.Key);
        }
    }
    
    // Remove empty packs
    for (const FString& PackID : PacksToRemove)
    {
        ActivePacks.Remove(PackID);
    }
}

FVector UCombat_PackBehaviorManager::CalculateCircleFormationPosition(const FCombat_PackData& Pack, int32 MemberIndex)
{
    if (Pack.Members.Num() <= 1)
    {
        return Pack.FormationCenter;
    }
    
    float AngleStep = 360.0f / Pack.Members.Num();
    float Angle = AngleStep * MemberIndex;
    float RadianAngle = FMath::DegreesToRadians(Angle);
    
    FVector Offset = FVector(
        FMath::Cos(RadianAngle) * Pack.FormationRadius,
        FMath::Sin(RadianAngle) * Pack.FormationRadius,
        0.0f
    );
    
    return Pack.FormationCenter + Offset;
}

FVector UCombat_PackBehaviorManager::CalculateLineFormationPosition(const FCombat_PackData& Pack, int32 MemberIndex)
{
    if (Pack.Members.Num() <= 1)
    {
        return Pack.FormationCenter;
    }
    
    float Spacing = Pack.FormationRadius / Pack.Members.Num();
    float OffsetX = (MemberIndex - (Pack.Members.Num() / 2.0f)) * Spacing;
    
    return Pack.FormationCenter + FVector(OffsetX, 0.0f, 0.0f);
}

FVector UCombat_PackBehaviorManager::CalculateWedgeFormationPosition(const FCombat_PackData& Pack, int32 MemberIndex)
{
    if (Pack.Members.Num() <= 1)
    {
        return Pack.FormationCenter;
    }
    
    if (MemberIndex == 0)
    {
        // Alpha at the point
        return Pack.FormationCenter;
    }
    
    int32 Row = (MemberIndex - 1) / 2 + 1;
    int32 Side = (MemberIndex - 1) % 2 == 0 ? -1 : 1;
    
    float RowSpacing = Pack.FormationRadius * 0.3f;
    float SideSpacing = Pack.FormationRadius * 0.4f;
    
    FVector Offset = FVector(
        -Row * RowSpacing,
        Side * SideSpacing * Row,
        0.0f
    );
    
    return Pack.FormationCenter + Offset;
}

FVector UCombat_PackBehaviorManager::CalculateAmbushFormationPosition(const FCombat_PackData& Pack, int32 MemberIndex)
{
    // Spread members in a wide arc for ambush
    if (Pack.Members.Num() <= 1)
    {
        return Pack.FormationCenter;
    }
    
    float ArcAngle = 180.0f; // Half circle
    float AngleStep = ArcAngle / (Pack.Members.Num() - 1);
    float Angle = -90.0f + (AngleStep * MemberIndex);
    float RadianAngle = FMath::DegreesToRadians(Angle);
    
    float AmbushRadius = Pack.FormationRadius * 1.5f;
    
    FVector Offset = FVector(
        FMath::Cos(RadianAngle) * AmbushRadius,
        FMath::Sin(RadianAngle) * AmbushRadius,
        0.0f
    );
    
    return Pack.FormationCenter + Offset;
}

bool UCombat_PackBehaviorManager::IsValidPackMember(AActor* Actor)
{
    if (!Actor)
    {
        return false;
    }
    
    // Check if actor has required components or is a valid pawn
    return Actor->IsA<APawn>() || Actor->FindComponentByClass<UPrimitiveComponent>() != nullptr;
}

void UCombat_PackBehaviorManager::UpdateMemberStatus(FCombat_PackMember& Member)
{
    if (!Member.Actor.IsValid())
    {
        return;
    }
    
    Member.LastKnownPosition = Member.Actor->GetActorLocation();
    Member.LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    // Update health and stamina if possible
    // This would typically interface with health/stamina components
    Member.HealthPercentage = 1.0f; // Placeholder
    Member.StaminaPercentage = 1.0f; // Placeholder
}

float UCombat_PackBehaviorManager::CalculatePackThreatLevel(const FCombat_PackData& Pack, AActor* Target)
{
    if (!Target)
    {
        return 0.0f;
    }
    
    // Simple threat calculation based on distance and target type
    float DistanceToTarget = FVector::Dist(Pack.FormationCenter, Target->GetActorLocation());
    float ThreatLevel = 1.0f;
    
    // Closer targets are more threatening
    if (DistanceToTarget < 500.0f)
    {
        ThreatLevel += 2.0f;
    }
    else if (DistanceToTarget < 1000.0f)
    {
        ThreatLevel += 1.0f;
    }
    
    return ThreatLevel;
}