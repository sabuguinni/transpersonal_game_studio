#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "Engine/TargetPoint.h"
#include "SharedTypes.h"
#include "NPCBehaviorTypes.h"
#include "NPC_SocialInteractionSystem.generated.h"

UENUM(BlueprintType)
enum class ENPC_SocialInteractionType : uint8
{
    None = 0,
    Greeting,
    Grooming,
    PlayFighting,
    FoodSharing,
    TerritoryNegotiation,
    MateSelection,
    ParentOffspring,
    PackBonding,
    ConflictResolution,
    AllianceFormation
};

UENUM(BlueprintType)
enum class ENPC_SocialZoneType : uint8
{
    None = 0,
    Feeding,
    Water,
    Resting,
    Nesting,
    Social,
    Danger,
    Territory,
    Neutral
};

USTRUCT(BlueprintType)
struct FNPC_SocialRelationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TWeakObjectPtr<AActor> TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float RelationshipStrength = 0.0f; // -1.0 (hostile) to 1.0 (friendly)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float TrustLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float Familiarity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float LastInteractionTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    int32 InteractionCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    ENPC_SocialInteractionType LastInteractionType = ENPC_SocialInteractionType::None;

    FNPC_SocialRelationship()
    {
        RelationshipStrength = 0.0f;
        TrustLevel = 0.0f;
        Familiarity = 0.0f;
        LastInteractionTime = 0.0f;
        InteractionCount = 0;
        LastInteractionType = ENPC_SocialInteractionType::None;
    }
};

USTRUCT(BlueprintType)
struct FNPC_SocialZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    TWeakObjectPtr<ATriggerBox> ZoneTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    ENPC_SocialZoneType ZoneType = ENPC_SocialZoneType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float ZoneInfluence = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    int32 MaxOccupants = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    int32 CurrentOccupants = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    TArray<TWeakObjectPtr<AActor>> OccupantActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    bool bIsActive = true;

    FNPC_SocialZone()
    {
        ZoneType = ENPC_SocialZoneType::None;
        ZoneInfluence = 1.0f;
        MaxOccupants = 10;
        CurrentOccupants = 0;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct FNPC_GroupDynamics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    TArray<TWeakObjectPtr<AActor>> GroupMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    TWeakObjectPtr<AActor> GroupLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    float GroupCohesion = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    float GroupMorale = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    FVector GroupCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    float GroupRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    bool bIsFormationActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    ENPC_BehaviorState GroupState = ENPC_BehaviorState::Idle;

    FNPC_GroupDynamics()
    {
        GroupCohesion = 0.5f;
        GroupMorale = 0.5f;
        GroupCenter = FVector::ZeroVector;
        GroupRadius = 1000.0f;
        bIsFormationActive = false;
        GroupState = ENPC_BehaviorState::Idle;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_SocialInteractionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_SocialInteractionSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Social Relationship Management
    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    void InitializeSocialSystem();

    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    void UpdateSocialRelationships(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    FNPC_SocialRelationship* GetRelationship(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    void AddOrUpdateRelationship(AActor* TargetActor, float StrengthDelta, float TrustDelta);

    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    void ProcessSocialInteraction(AActor* TargetActor, ENPC_SocialInteractionType InteractionType);

    // Zone Management
    UFUNCTION(BlueprintCallable, Category = "Social Zones")
    void RegisterSocialZone(ATriggerBox* ZoneTrigger, ENPC_SocialZoneType ZoneType, int32 MaxOccupants = 10);

    UFUNCTION(BlueprintCallable, Category = "Social Zones")
    void UpdateZoneOccupancy();

    UFUNCTION(BlueprintCallable, Category = "Social Zones")
    FNPC_SocialZone* GetNearestZoneOfType(ENPC_SocialZoneType ZoneType);

    UFUNCTION(BlueprintCallable, Category = "Social Zones")
    bool IsInSocialZone(ENPC_SocialZoneType ZoneType);

    // Group Dynamics
    UFUNCTION(BlueprintCallable, Category = "Group Dynamics")
    void JoinGroup(AActor* GroupLeader);

    UFUNCTION(BlueprintCallable, Category = "Group Dynamics")
    void LeaveGroup();

    UFUNCTION(BlueprintCallable, Category = "Group Dynamics")
    void UpdateGroupDynamics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Group Dynamics")
    void SetGroupFormation(bool bActivate);

    UFUNCTION(BlueprintCallable, Category = "Group Dynamics")
    FVector GetFormationPosition();

    // Communication System
    UFUNCTION(BlueprintCallable, Category = "Communication")
    void SendSocialSignal(ENPC_SocialInteractionType SignalType, float Range = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void ReceiveSocialSignal(AActor* Sender, ENPC_SocialInteractionType SignalType);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void BroadcastGroupAlert(ENPC_BehaviorState AlertState);

    // Behavioral Influence
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    float CalculateSocialInfluence(ENPC_BehaviorState DesiredState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ApplySocialPressure(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool ShouldFollowGroupBehavior();

protected:
    // Social Relationships
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social System")
    TMap<TWeakObjectPtr<AActor>, FNPC_SocialRelationship> SocialRelationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social System")
    float SocialUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social System")
    float RelationshipDecayRate = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social System")
    float MaxSocialRange = 2000.0f;

    // Social Zones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Zones")
    TArray<FNPC_SocialZone> RegisteredZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Zones")
    FNPC_SocialZone* CurrentZone;

    // Group Dynamics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group Dynamics")
    FNPC_GroupDynamics GroupData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group Dynamics")
    bool bIsGroupMember = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group Dynamics")
    bool bIsGroupLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group Dynamics")
    int32 GroupFormationIndex = 0;

    // Communication
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float CommunicationRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float LastCommunicationTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float CommunicationCooldown = 2.0f;

    // Behavioral Influence
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SocialInfluenceStrength = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float IndependenceLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float GroupConformityThreshold = 0.7f;

private:
    float LastSocialUpdate = 0.0f;
    float LastGroupUpdate = 0.0f;
    
    void CleanupInvalidRelationships();
    void UpdateRelationshipDecay(float DeltaTime);
    void ProcessNearbyActors();
    void UpdateGroupCohesion();
    void CalculateGroupCenter();
};

#include "NPC_SocialInteractionSystem.generated.h"