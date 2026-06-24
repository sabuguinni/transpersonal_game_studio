#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CrowdSimulationManager.generated.h"

// ============================================================
// ENUMS — global scope (RULE 1: USTRUCT/UENUM at global scope)
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Working     UMETA(DisplayName = "Working")
};

UENUM(BlueprintType)
enum class ECrowd_AgentLOD : uint8
{
    Full        UMETA(DisplayName = "Full (< 2000uu)"),
    Mid         UMETA(DisplayName = "Mid (2000-5000uu)"),
    Distant     UMETA(DisplayName = "Distant (> 5000uu)")
};

UENUM(BlueprintType)
enum class ECrowd_ZoneType : uint8
{
    Village     UMETA(DisplayName = "Village"),
    Waterhole   UMETA(DisplayName = "Waterhole"),
    Hunting     UMETA(DisplayName = "Hunting Ground"),
    Migration   UMETA(DisplayName = "Migration Path")
};

// ============================================================
// STRUCTS — global scope
// ============================================================

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    AActor* AgentActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector AgentLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentState AgentState = ECrowd_AgentState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentLOD LODLevel = ECrowd_AgentLOD::Full;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float MoveSpeed = 150.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float FearLevel = 0.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_ZoneData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float ZoneRadius = 800.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_ZoneType ZoneType = ECrowd_ZoneType::Village;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 MaxAgentsInZone = 20;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 CurrentAgentCount = 0;
};

// ============================================================
// UCLASS
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // --- Configuration ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxCrowdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FleeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float CrowdGatherRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LODDistanceFar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LODDistanceMid;

    // --- Runtime State ---

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    bool bCrowdFleeActive;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    FVector FleeOrigin;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_AgentData> ActiveAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_ZoneData> RegisteredZones;

    // --- Public API ---

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerCrowdFlee(FVector DinosaurLocation, float DinosaurThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void ResetCrowdFlee();

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveCrowdCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    TArray<FCrowd_AgentData> GetAgentsInRadius(FVector Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    ECrowd_AgentLOD GetAgentLODLevel(FVector AgentLocation, FVector PlayerLocation);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void InitializeCrowdZones();
    void UpdateFleeingAgents(float DeltaTime);
    void UpdateIdleAgents(float DeltaTime);
};
