#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameplayTags.h"
#include "GroupCombatComponent.generated.h"

class ACombatAIController;

UENUM(BlueprintType)
enum class EGroupRole : uint8
{
    None,
    Leader,
    Follower,
    Scout,
    Flanker,
    Support,
    Defender,
    Berserker
};

UENUM(BlueprintType)
enum class EGroupTactic : uint8
{
    None,
    SwarmAttack,
    CoordinatedAssault,
    PincerMovement,
    WaveAttack,
    AmbushFormation,
    DefensiveCircle,
    HuntingPack,
    Distraction
};

UENUM(BlueprintType)
enum class EGroupCommunication : uint8
{
    None,
    Alert,
    Attack,
    Retreat,
    Help,
    Regroup,
    FlankLeft,
    FlankRight,
    HoldPosition,
    Advance
};

USTRUCT(BlueprintType)
struct FGroupMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    ACombatAIController* Controller = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    EGroupRole Role = EGroupRole::Follower;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    float HealthPercentage = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    FVector AssignedPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    AActor* AssignedTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    float DistanceToLeader = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    float LastCommunicationTime = 0.0f;

    FGroupMember()
    {
        Controller = nullptr;
        Role = EGroupRole::Follower;
        HealthPercentage = 1.0f;
        AssignedPosition = FVector::ZeroVector;
        AssignedTarget = nullptr;
        DistanceToLeader = 0.0f;
        bIsActive = true;
        LastCommunicationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FGroupFormation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FString FormationName = TEXT("Default");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    TArray<FVector> RelativePositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    bool bMaintainDistance = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float OptimalSpacing = 200.0f;

    FGroupFormation()
    {
        FormationName = TEXT("Default");
        FormationRadius = 500.0f;
        bMaintainDistance = true;
        OptimalSpacing = 200.0f;
    }
};

/**
 * Component that manages group combat coordination and pack behavior
 * Handles communication, formation, and coordinated attacks
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UGroupCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGroupCombatComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Group Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    float GroupRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    int32 MaxGroupSize = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    float CommunicationRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    float CoordinationUpdateInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    bool bAutoAssignRoles = true;

    // Group State
    UPROPERTY(BlueprintReadOnly, Category = "Group")
    TArray<FGroupMember> GroupMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Group")
    ACombatAIController* GroupLeader;

    UPROPERTY(BlueprintReadOnly, Category = "Group")
    EGroupTactic CurrentTactic;

    UPROPERTY(BlueprintReadOnly, Category = "Group")
    FGroupFormation CurrentFormation;

    UPROPERTY(BlueprintReadOnly, Category = "Group")
    AActor* GroupTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Group")
    FVector GroupCenterPosition;

    // Tactical Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float FlankingCoordination = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float AttackSynchronization = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bEnablePackHunting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bEnableCoordinatedRetreat = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float RetreatThreshold = 0.3f;

public:
    // Group Management
    UFUNCTION(BlueprintCallable, Category = "Group")
    void JoinGroup(ACombatAIController* NewMember);

    UFUNCTION(BlueprintCallable, Category = "Group")
    void LeaveGroup(ACombatAIController* Member);

    UFUNCTION(BlueprintCallable, Category = "Group")
    void SetGroupLeader(ACombatAIController* NewLeader);

    UFUNCTION(BlueprintPure, Category = "Group")
    bool IsGroupLeader() const;

    UFUNCTION(BlueprintPure, Category = "Group")
    int32 GetGroupSize() const { return GroupMembers.Num(); }

    UFUNCTION(BlueprintPure, Category = "Group")
    TArray<ACombatAIController*> GetActiveMembers() const;

    // Role Assignment
    UFUNCTION(BlueprintCallable, Category = "Group")
    void AssignRole(ACombatAIController* Member, EGroupRole Role);

    UFUNCTION(BlueprintCallable, Category = "Group")
    void AutoAssignRoles();

    UFUNCTION(BlueprintPure, Category = "Group")
    EGroupRole GetOptimalRole(ACombatAIController* Member) const;

    // Tactical Coordination
    UFUNCTION(BlueprintCallable, Category = "Tactics")
    void SetGroupTactic(EGroupTactic Tactic);

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    void ExecuteCoordinatedAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    void ExecuteFlankingManeuver(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    void ExecutePincerMovement(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    void ExecuteCoordinatedRetreat();

    // Formation Management
    UFUNCTION(BlueprintCallable, Category = "Formation")
    void SetFormation(const FGroupFormation& Formation);

    UFUNCTION(BlueprintCallable, Category = "Formation")
    void MaintainFormation();

    UFUNCTION(BlueprintCallable, Category = "Formation")
    FVector GetFormationPosition(ACombatAIController* Member);

    UFUNCTION(BlueprintPure, Category = "Formation")
    bool IsInFormation() const;

    // Communication
    UFUNCTION(BlueprintCallable, Category = "Communication")
    void SendGroupMessage(EGroupCommunication Message, FVector Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void ReceiveGroupMessage(EGroupCommunication Message, ACombatAIController* Sender, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void BroadcastThreatAlert(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void CallForGroupSupport(ACombatAIController* Caller);

    // Target Assignment
    UFUNCTION(BlueprintCallable, Category = "Targeting")
    void AssignGroupTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Targeting")
    void AssignIndividualTargets();

    UFUNCTION(BlueprintCallable, Category = "Targeting")
    AActor* GetAssignedTarget(ACombatAIController* Member) const;

    UFUNCTION(BlueprintPure, Category = "Targeting")
    bool ShouldFocusFireOnTarget(AActor* Target) const;

    // Pack Hunting Behavior
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void InitiatePackHunt(AActor* Prey);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void ExecuteHerdingBehavior(AActor* Prey);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void CoordinateAmbush(AActor* Prey);

    UFUNCTION(BlueprintPure, Category = "Pack Hunting")
    FVector CalculateHerdingPosition(AActor* Prey, ACombatAIController* Member) const;

    // Group Intelligence
    UFUNCTION(BlueprintCallable, Category = "Intelligence")
    void ShareIntelligence();

    UFUNCTION(BlueprintCallable, Category = "Intelligence")
    void UpdateGroupKnowledge(AActor* Actor, FVector LastKnownPosition);

    UFUNCTION(BlueprintPure, Category = "Intelligence")
    float GetGroupMorale() const;

    UFUNCTION(BlueprintCallable, Category = "Intelligence")
    void BoostGroupMorale(float Amount);

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "Group")
    FVector CalculateGroupCenter() const;

    UFUNCTION(BlueprintPure, Category = "Group")
    float GetAverageGroupHealth() const;

    UFUNCTION(BlueprintPure, Category = "Group")
    bool IsGroupIntact() const;

    UFUNCTION(BlueprintPure, Category = "Group")
    ACombatAIController* FindNearestMember(FVector Position) const;

    // Debugging
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawGroupInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawFormation() const;

protected:
    // Internal group logic
    void UpdateGroupState();
    void UpdateMemberPositions();
    void ProcessGroupCommunication();
    void EvaluateGroupTactics();
    void MonitorGroupHealth();
    
    // Formation calculations
    FVector CalculateFormationPosition(int32 MemberIndex, const FGroupFormation& Formation);
    void AdjustFormationForTerrain();
    bool ValidateFormationPosition(FVector Position);
    
    // Tactical decision making
    EGroupTactic SelectOptimalTactic(AActor* Target);
    void AdaptTacticsToSituation();
    bool ShouldChangeFormation();
    
    // Communication processing
    void ProcessCommunicationQueue();
    void HandleEmergencyCommunication(EGroupCommunication Message, ACombatAIController* Sender);
    void UpdateCommunicationNetwork();

    // Timers
    float CoordinationUpdateTimer = 0.0f;
    float FormationUpdateTimer = 0.0f;
    float CommunicationTimer = 0.0f;
    float TacticsEvaluationTimer = 0.0f;

    // Group memory and intelligence
    TMap<AActor*, FVector> SharedKnowledge;
    TArray<EGroupCommunication> CommunicationQueue;
    float GroupMorale = 1.0f;
    int32 SuccessfulCoordinatedAttacks = 0;

    // Cached references
    class ACombatAIController* OwnerController;
    class UWorld* CachedWorld;
};