#include "Crowd_SocialDynamicsManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCrowd_SocialDynamicsManager::UCrowd_SocialDynamicsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    InteractionRadius = 300.0f;
    GroupFormationThreshold = 200.0f;
    SocialInfluenceRange = 500.0f;
    bEnableDynamicGrouping = true;
    bEnableConflictResolution = true;
}

void UCrowd_SocialDynamicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Social Dynamics Manager initialized"));
}

void UCrowd_SocialDynamicsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateGroupDynamics(DeltaTime);
    ProcessSocialInfluence(DeltaTime);
    UpdateInteractions(DeltaTime);
    
    if (bEnableDynamicGrouping)
    {
        ProcessDynamicGrouping();
    }
    
    CleanupExpiredInteractions();
}

void UCrowd_SocialDynamicsManager::CreateSocialGroup(const TArray<AActor*>& Members, const FString& GroupName)
{
    if (Members.Num() == 0)
    {
        return;
    }

    FCrowd_SocialGroup NewGroup;
    NewGroup.Members = Members;
    NewGroup.GroupName = GroupName;
    NewGroup.GroupStatus = ECrowd_SocialStatus::Neutral;
    NewGroup.CohesionRadius = GroupFormationThreshold;
    NewGroup.GroupMorale = 50.0f;
    
    AssignGroupLeader(NewGroup);
    SocialGroups.Add(NewGroup);

    UE_LOG(LogTemp, Log, TEXT("Created social group '%s' with %d members"), *GroupName, Members.Num());
}

void UCrowd_SocialDynamicsManager::DisbandGroup(int32 GroupIndex)
{
    if (SocialGroups.IsValidIndex(GroupIndex))
    {
        FString GroupName = SocialGroups[GroupIndex].GroupName;
        SocialGroups.RemoveAt(GroupIndex);
        UE_LOG(LogTemp, Log, TEXT("Disbanded social group '%s'"), *GroupName);
    }
}

void UCrowd_SocialDynamicsManager::InitiateSocialInteraction(AActor* Initiator, AActor* Target, ECrowd_SocialInteractionType InteractionType)
{
    if (!Initiator || !Target || Initiator == Target)
    {
        return;
    }

    FCrowd_SocialInteraction NewInteraction;
    NewInteraction.Initiator = Initiator;
    NewInteraction.Target = Target;
    NewInteraction.InteractionType = InteractionType;
    NewInteraction.InteractionStrength = 1.0f;
    NewInteraction.Duration = 5.0f;
    NewInteraction.bIsActive = true;

    ActiveInteractions.Add(NewInteraction);

    UE_LOG(LogTemp, Log, TEXT("Social interaction initiated between %s and %s"), 
           *Initiator->GetName(), *Target->GetName());
}

void UCrowd_SocialDynamicsManager::UpdateGroupDynamics(float DeltaTime)
{
    for (FCrowd_SocialGroup& Group : SocialGroups)
    {
        if (Group.Members.Num() == 0)
        {
            continue;
        }

        // Update group cohesion
        FVector GroupCenter = FVector::ZeroVector;
        int32 ValidMembers = 0;

        for (AActor* Member : Group.Members)
        {
            if (IsValid(Member))
            {
                GroupCenter += Member->GetActorLocation();
                ValidMembers++;
            }
        }

        if (ValidMembers > 0)
        {
            GroupCenter /= ValidMembers;

            // Apply cohesion forces to group members
            for (AActor* Member : Group.Members)
            {
                if (IsValid(Member))
                {
                    ApplySocialForces(Member, Group);
                }
            }
        }

        // Update group morale based on external factors
        float MoraleChange = FMath::RandRange(-1.0f, 1.0f) * DeltaTime;
        UpdateGroupMorale(Group, MoraleChange);
    }
}

void UCrowd_SocialDynamicsManager::ProcessSocialInfluence(float DeltaTime)
{
    for (int32 i = 0; i < SocialGroups.Num(); i++)
    {
        for (int32 j = i + 1; j < SocialGroups.Num(); j++)
        {
            FCrowd_SocialGroup& Group1 = SocialGroups[i];
            FCrowd_SocialGroup& Group2 = SocialGroups[j];

            if (Group1.Members.Num() == 0 || Group2.Members.Num() == 0)
            {
                continue;
            }

            // Check if groups are close enough to influence each other
            FVector Group1Center = FVector::ZeroVector;
            FVector Group2Center = FVector::ZeroVector;

            for (AActor* Member : Group1.Members)
            {
                if (IsValid(Member))
                {
                    Group1Center += Member->GetActorLocation();
                }
            }
            Group1Center /= FMath::Max(1, Group1.Members.Num());

            for (AActor* Member : Group2.Members)
            {
                if (IsValid(Member))
                {
                    Group2Center += Member->GetActorLocation();
                }
            }
            Group2Center /= FMath::Max(1, Group2.Members.Num());

            float Distance = FVector::Dist(Group1Center, Group2Center);
            if (Distance < SocialInfluenceRange)
            {
                // Groups influence each other
                if (bEnableConflictResolution && 
                    Group1.GroupStatus != Group2.GroupStatus)
                {
                    HandleGroupConflict(Group1, Group2);
                }
            }
        }
    }
}

FCrowd_SocialGroup* UCrowd_SocialDynamicsManager::FindGroupForActor(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return nullptr;
    }

    for (FCrowd_SocialGroup& Group : SocialGroups)
    {
        if (Group.Members.Contains(Actor))
        {
            return &Group;
        }
    }

    return nullptr;
}

TArray<AActor*> UCrowd_SocialDynamicsManager::GetNearbyActors(AActor* CenterActor, float Radius)
{
    TArray<AActor*> NearbyActors;
    
    if (!IsValid(CenterActor))
    {
        return NearbyActors;
    }

    UWorld* World = CenterActor->GetWorld();
    if (!World)
    {
        return NearbyActors;
    }

    FVector CenterLocation = CenterActor->GetActorLocation();

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (IsValid(Actor) && Actor != CenterActor)
        {
            float Distance = FVector::Dist(CenterLocation, Actor->GetActorLocation());
            if (Distance <= Radius)
            {
                NearbyActors.Add(Actor);
            }
        }
    }

    return NearbyActors;
}

void UCrowd_SocialDynamicsManager::HandleGroupConflict(FCrowd_SocialGroup& Group1, FCrowd_SocialGroup& Group2)
{
    // Simple conflict resolution based on group size and morale
    float Group1Strength = Group1.Members.Num() * Group1.GroupMorale;
    float Group2Strength = Group2.Members.Num() * Group2.GroupMorale;

    if (Group1Strength > Group2Strength)
    {
        UpdateGroupMorale(Group1, 5.0f);
        UpdateGroupMorale(Group2, -3.0f);
        Group2.GroupStatus = ECrowd_SocialStatus::Fleeing;
    }
    else if (Group2Strength > Group1Strength)
    {
        UpdateGroupMorale(Group2, 5.0f);
        UpdateGroupMorale(Group1, -3.0f);
        Group1.GroupStatus = ECrowd_SocialStatus::Fleeing;
    }
    else
    {
        // Stalemate
        UpdateGroupMorale(Group1, -1.0f);
        UpdateGroupMorale(Group2, -1.0f);
    }

    UE_LOG(LogTemp, Log, TEXT("Group conflict resolved between '%s' and '%s'"), 
           *Group1.GroupName, *Group2.GroupName);
}

void UCrowd_SocialDynamicsManager::UpdateGroupMorale(FCrowd_SocialGroup& Group, float MoraleChange)
{
    Group.GroupMorale = FMath::Clamp(Group.GroupMorale + MoraleChange, 0.0f, 100.0f);
    
    // Adjust group status based on morale
    if (Group.GroupMorale < 20.0f)
    {
        Group.GroupStatus = ECrowd_SocialStatus::Fleeing;
    }
    else if (Group.GroupMorale > 80.0f)
    {
        Group.GroupStatus = ECrowd_SocialStatus::Aggressive;
    }
    else
    {
        Group.GroupStatus = ECrowd_SocialStatus::Neutral;
    }
}

void UCrowd_SocialDynamicsManager::AssignGroupLeader(FCrowd_SocialGroup& Group)
{
    if (Group.Members.Num() == 0)
    {
        Group.GroupLeader = nullptr;
        return;
    }

    // Simple leader selection - first valid member
    for (AActor* Member : Group.Members)
    {
        if (IsValid(Member))
        {
            Group.GroupLeader = Member;
            break;
        }
    }
}

void UCrowd_SocialDynamicsManager::ProcessDynamicGrouping()
{
    // Find isolated actors and try to group them
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    TArray<AActor*> UngroupedActors;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (IsValid(Actor) && !FindGroupForActor(Actor))
        {
            UngroupedActors.Add(Actor);
        }
    }

    // Try to form new groups from nearby ungrouped actors
    for (int32 i = 0; i < UngroupedActors.Num(); i++)
    {
        AActor* Actor = UngroupedActors[i];
        if (!IsValid(Actor))
        {
            continue;
        }

        TArray<AActor*> NearbyUngrouped = GetNearbyActors(Actor, GroupFormationThreshold);
        NearbyUngrouped.RemoveAll([this](AActor* A) { return FindGroupForActor(A) != nullptr; });

        if (NearbyUngrouped.Num() >= 2)
        {
            NearbyUngrouped.Add(Actor);
            CreateSocialGroup(NearbyUngrouped, FString::Printf(TEXT("DynamicGroup_%d"), SocialGroups.Num()));
        }
    }
}

void UCrowd_SocialDynamicsManager::UpdateInteractions(float DeltaTime)
{
    for (FCrowd_SocialInteraction& Interaction : ActiveInteractions)
    {
        if (Interaction.bIsActive)
        {
            Interaction.Duration -= DeltaTime;
            
            if (Interaction.Duration <= 0.0f)
            {
                Interaction.bIsActive = false;
            }
        }
    }
}

void UCrowd_SocialDynamicsManager::CleanupExpiredInteractions()
{
    ActiveInteractions.RemoveAll([](const FCrowd_SocialInteraction& Interaction) {
        return !Interaction.bIsActive;
    });
}

float UCrowd_SocialDynamicsManager::CalculateSocialCompatibility(AActor* Actor1, AActor* Actor2)
{
    if (!IsValid(Actor1) || !IsValid(Actor2))
    {
        return 0.0f;
    }

    // Simple compatibility based on distance and random factor
    float Distance = FVector::Dist(Actor1->GetActorLocation(), Actor2->GetActorLocation());
    float DistanceFactor = FMath::Clamp(1.0f - (Distance / SocialInfluenceRange), 0.0f, 1.0f);
    float RandomFactor = FMath::RandRange(0.5f, 1.0f);
    
    return DistanceFactor * RandomFactor;
}

void UCrowd_SocialDynamicsManager::ApplySocialForces(AActor* Actor, const FCrowd_SocialGroup& Group)
{
    if (!IsValid(Actor))
    {
        return;
    }

    // Calculate cohesion force towards group center
    FVector GroupCenter = FVector::ZeroVector;
    int32 ValidMembers = 0;

    for (AActor* Member : Group.Members)
    {
        if (IsValid(Member) && Member != Actor)
        {
            GroupCenter += Member->GetActorLocation();
            ValidMembers++;
        }
    }

    if (ValidMembers > 0)
    {
        GroupCenter /= ValidMembers;
        FVector CohesionForce = (GroupCenter - Actor->GetActorLocation()).GetSafeNormal();
        
        // Apply subtle movement influence (this would typically integrate with movement components)
        // For now, just log the social influence
        UE_LOG(LogTemp, VeryVerbose, TEXT("Applying social cohesion force to %s"), *Actor->GetName());
    }
}