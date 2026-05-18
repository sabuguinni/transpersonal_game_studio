#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Combat_BehaviorManager.generated.h"

UENUM(BlueprintType)
enum class ECombat_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Territorial UMETA(DisplayName = "Territorial")
};

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_ThreatData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* ThreatActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float LastSeenTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector LastKnownPosition;

    FCombat_ThreatData()
    {
        ThreatActor = nullptr;
        ThreatLevel = ECombat_ThreatLevel::None;
        Distance = 0.0f;
        LastSeenTime = 0.0f;
        LastKnownPosition = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector PackCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float PackRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsHunting;

    FCombat_PackData()
    {
        PackLeader = nullptr;
        PackCenter = FVector::ZeroVector;
        PackRadius = 1000.0f;
        bIsHunting = false;
    }
};

/**
 * Combat Behavior Manager - Orchestrates tactical combat AI for dinosaurs and enemies
 * Handles threat assessment, pack coordination, and combat state management
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_BehaviorManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_BehaviorManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core behavior state management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetBehaviorState(ECombat_BehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombat_BehaviorState GetBehaviorState() const { return CurrentBehaviorState; }

    // Threat assessment and tracking
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RegisterThreat(AActor* ThreatActor, ECombat_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateThreatAssessment(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Combat")
    FCombat_ThreatData GetHighestThreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ClearThreats();

    // Pack behavior coordination
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void JoinPack(AActor* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdatePackCoordination(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsInPack() const { return PackData.PackLeader != nullptr; }

    // Combat decision making
    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector CalculateOptimalAttackPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldEngageTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldFleeFromThreat(const FCombat_ThreatData& ThreatData);

    // Tactical positioning
    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetFlankingPosition(AActor* Target, bool bLeftFlank = true);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetAmbushPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteTacticalManeuver(const FString& ManeuverType);

protected:
    // Current behavior state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    ECombat_BehaviorState CurrentBehaviorState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    ECombat_BehaviorState PreviousBehaviorState;

    // Threat tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    TArray<FCombat_ThreatData> KnownThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float ThreatDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float ThreatForgetTime;

    // Pack coordination
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    FCombat_PackData PackData;

    // Combat parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float FleeThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    bool bCanFormPacks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    bool bIsTerritorial;

private:
    // Internal state tracking
    float StateChangeTime;
    float LastThreatUpdateTime;
    float LastPackUpdateTime;

    // Helper functions
    void ProcessBehaviorTransitions(float DeltaTime);
    void UpdateThreatPositions();
    void CoordinatePackMovement();
    ECombat_ThreatLevel AssessThreatLevel(AActor* ThreatActor);
    bool IsValidThreat(AActor* Actor);
};