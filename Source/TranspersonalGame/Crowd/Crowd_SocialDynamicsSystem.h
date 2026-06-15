#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "MassEntityTypes.h"
#include "SharedTypes.h"
#include "Crowd_SocialDynamicsSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SocialRelationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    FMassEntityHandle EntityA;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    FMassEntityHandle EntityB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    ECrowd_RelationshipType RelationshipType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float RelationshipStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float LastInteractionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    int32 InteractionCount;

    FCrowd_SocialRelationship()
    {
        EntityA = FMassEntityHandle();
        EntityB = FMassEntityHandle();
        RelationshipType = ECrowd_RelationshipType::Neutral;
        RelationshipStrength = 0.5f;
        LastInteractionTime = 0.0f;
        InteractionCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SocialProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Profile")
    ECrowd_PersonalityType PersonalityType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Profile")
    float Extroversion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Profile")
    float Agreeableness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Profile")
    float Dominance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Profile")
    float SocialRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Profile")
    TArray<FMassEntityHandle> KnownEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Profile")
    TArray<FCrowd_SocialRelationship> Relationships;

    FCrowd_SocialProfile()
    {
        PersonalityType = ECrowd_PersonalityType::Balanced;
        Extroversion = 0.5f;
        Agreeableness = 0.5f;
        Dominance = 0.5f;
        SocialRadius = 500.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SocialGroup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Group")
    int32 GroupID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Group")
    TArray<FMassEntityHandle> Members;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Group")
    FMassEntityHandle Leader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Group")
    ECrowd_GroupType GroupType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Group")
    float GroupCohesion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Group")
    FVector GroupCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Group")
    float FormationTime;

    FCrowd_SocialGroup()
    {
        GroupID = -1;
        Leader = FMassEntityHandle();
        GroupType = ECrowd_GroupType::Casual;
        GroupCohesion = 0.5f;
        GroupCenter = FVector::ZeroVector;
        FormationTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_SocialDynamicsSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_SocialDynamicsSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Social Profile Management
    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    void CreateSocialProfile(FMassEntityHandle Entity, ECrowd_PersonalityType PersonalityType);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    FCrowd_SocialProfile GetSocialProfile(FMassEntityHandle Entity);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    void UpdatePersonalityTraits(FMassEntityHandle Entity, float Extroversion, float Agreeableness, float Dominance);

    // Relationship Management
    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    void EstablishRelationship(FMassEntityHandle EntityA, FMassEntityHandle EntityB, ECrowd_RelationshipType RelationType);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    void UpdateRelationshipStrength(FMassEntityHandle EntityA, FMassEntityHandle EntityB, float StrengthDelta);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    ECrowd_RelationshipType GetRelationshipType(FMassEntityHandle EntityA, FMassEntityHandle EntityB);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    float GetRelationshipStrength(FMassEntityHandle EntityA, FMassEntityHandle EntityB);

    // Social Interactions
    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    void ProcessSocialInteraction(FMassEntityHandle EntityA, FMassEntityHandle EntityB, ECrowd_InteractionType InteractionType);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    TArray<FMassEntityHandle> FindNearbyEntities(FMassEntityHandle Entity, float SearchRadius);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    bool ShouldEntitiesInteract(FMassEntityHandle EntityA, FMassEntityHandle EntityB);

    // Group Dynamics
    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    int32 CreateSocialGroup(ECrowd_GroupType GroupType, FMassEntityHandle Leader);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    bool AddEntityToGroup(int32 GroupID, FMassEntityHandle Entity);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    bool RemoveEntityFromGroup(int32 GroupID, FMassEntityHandle Entity);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    void UpdateGroupCohesion(int32 GroupID, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    TArray<FMassEntityHandle> GetGroupMembers(int32 GroupID);

    // Social Influence
    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    FVector CalculateSocialInfluence(FMassEntityHandle Entity, const TArray<FMassEntityHandle>& NearbyEntities);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    void PropagateEmotionalState(FMassEntityHandle SourceEntity, ECrowd_EmotionalState EmotionalState, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    ECrowd_EmotionalState DetermineEmotionalResponse(FMassEntityHandle Entity, ECrowd_InteractionType InteractionType);

    // Leadership and Hierarchy
    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    FMassEntityHandle SelectGroupLeader(const TArray<FMassEntityHandle>& Candidates);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    float CalculateLeadershipScore(FMassEntityHandle Entity);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    void UpdateHierarchy(int32 GroupID);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Settings")
    TMap<FMassEntityHandle, FCrowd_SocialProfile> SocialProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Settings")
    TArray<FCrowd_SocialRelationship> AllRelationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Settings")
    TMap<int32, FCrowd_SocialGroup> SocialGroups;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Settings")
    float InteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Settings")
    float RelationshipDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Settings")
    float EmotionalContagionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Settings")
    float GroupFormationThreshold;

private:
    int32 NextGroupID;
    float LastSocialUpdate;

    void DecayRelationships(float DeltaTime);
    void UpdateSocialProfiles(float DeltaTime);
    void ProcessGroupFormation();
    void ProcessGroupDissolution();
    FCrowd_SocialRelationship* FindRelationship(FMassEntityHandle EntityA, FMassEntityHandle EntityB);
    float CalculatePersonalityCompatibility(const FCrowd_SocialProfile& ProfileA, const FCrowd_SocialProfile& ProfileB);
};