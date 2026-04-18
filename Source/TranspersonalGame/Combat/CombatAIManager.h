#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "CombatAIManager.generated.h"

class ADinosaurPawn;
class ATranspersonalCharacter;
class UCombat_TacticalAIComponent;

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Idle,
    Hunting,
    Stalking,
    Coordinating,
    Attacking,
    Retreating,
    Regrouping
};

UENUM(BlueprintType)
enum class ECombat_PackRole : uint8
{
    Alpha,
    Flanker,
    Ambusher,
    Distractor,
    Support
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalGroup
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    TArray<TWeakObjectPtr<ADinosaurPawn>> Members;

    UPROPERTY(BlueprintReadWrite)
    TWeakObjectPtr<ADinosaurPawn> Alpha;

    UPROPERTY(BlueprintReadWrite)
    ECombat_TacticalState CurrentState;

    UPROPERTY(BlueprintReadWrite)
    FVector TargetLocation;

    UPROPERTY(BlueprintReadWrite)
    TWeakObjectPtr<ATranspersonalCharacter> CurrentTarget;

    UPROPERTY(BlueprintReadWrite)
    float FormationRadius;

    UPROPERTY(BlueprintReadWrite)
    float LastCoordinationTime;

    FCombat_TacticalGroup()
    {
        CurrentState = ECombat_TacticalState::Idle;
        TargetLocation = FVector::ZeroVector;
        FormationRadius = 500.0f;
        LastCoordinationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalCommand
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    ECombat_TacticalState CommandType;

    UPROPERTY(BlueprintReadWrite)
    FVector TargetPosition;

    UPROPERTY(BlueprintReadWrite)
    ECombat_PackRole AssignedRole;

    UPROPERTY(BlueprintReadWrite)
    float Priority;

    UPROPERTY(BlueprintReadWrite)
    float ExecutionTime;

    FCombat_TacticalCommand()
    {
        CommandType = ECombat_TacticalState::Idle;
        TargetPosition = FVector::ZeroVector;
        AssignedRole = ECombat_PackRole::Support;
        Priority = 1.0f;
        ExecutionTime = 0.0f;
    }
};

/**
 * Combat AI Manager - Orchestrates tactical combat AI for dinosaur packs
 * Manages pack coordination, tactical positioning, and combat strategies
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCombatAIManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCombatAIManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Group management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    int32 CreateTacticalGroup(const TArray<ADinosaurPawn*>& Members, ADinosaurPawn* Alpha);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void DisbandTacticalGroup(int32 GroupID);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AddMemberToGroup(int32 GroupID, ADinosaurPawn* NewMember);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RemoveMemberFromGroup(int32 GroupID, ADinosaurPawn* Member);

    // Tactical coordination
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetGroupTarget(int32 GroupID, ATranspersonalCharacter* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetGroupState(int32 GroupID, ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinateGroupAttack(int32 GroupID);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteTacticalRetreat(int32 GroupID);

    // Combat analysis
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldEngageTarget(const FCombat_TacticalGroup& Group, ATranspersonalCharacter* PotentialTarget) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateFlankingPosition(const FCombat_TacticalGroup& Group, ATranspersonalCharacter* Target, ECombat_PackRole Role) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_PackRole AssignOptimalRole(ADinosaurPawn* Member, const FCombat_TacticalGroup& Group) const;

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FCombat_TacticalGroup* GetTacticalGroup(int32 GroupID);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<int32> GetActiveGroupIDs() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateAllGroups(float DeltaTime);

protected:
    // Group storage
    UPROPERTY()
    TMap<int32, FCombat_TacticalGroup> TacticalGroups;

    UPROPERTY()
    int32 NextGroupID;

    // Combat parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float MaxEngagementRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float MinGroupCoordinationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float FlankingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AmbushPositionRadius;

    // Internal methods
    void UpdateGroupCoordination(FCombat_TacticalGroup& Group, float DeltaTime);
    void ExecuteGroupCommand(FCombat_TacticalGroup& Group, const FCombat_TacticalCommand& Command);
    bool ValidateGroupIntegrity(FCombat_TacticalGroup& Group);
    void CleanupInvalidGroups();

    // Combat calculations
    float CalculateGroupThreatLevel(const FCombat_TacticalGroup& Group) const;
    float CalculateTargetThreatLevel(ATranspersonalCharacter* Target) const;
    bool IsPositionStrategic(const FVector& Position, const FCombat_TacticalGroup& Group, ATranspersonalCharacter* Target) const;
};