#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "CrowdSimulationManager.generated.h"

// ── Crowd species enum ──────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ECrowd_DinoSpecies : uint8
{
    None            UMETA(DisplayName = "None"),
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Pachycephalo    UMETA(DisplayName = "Pachycephalo"),
    Protoceratops   UMETA(DisplayName = "Protoceratops"),
    Tsintaosaurus   UMETA(DisplayName = "Tsintaosaurus"),
};

// ── Herd behaviour state ────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ECrowd_HerdState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
};

// ── Per-agent data ──────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_DinoSpecies Species = ECrowd_DinoSpecies::None;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_HerdState State = ECrowd_HerdState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float Health = 100.f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float Fear = 0.f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 HerdID = -1;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    TObjectPtr<AActor> ActorRef = nullptr;
};

// ── Herd group data ─────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FCrowd_HerdData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 HerdID = -1;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_DinoSpecies Species = ECrowd_DinoSpecies::None;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_HerdState State = ECrowd_HerdState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector CentreOfMass = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    TArray<int32> AgentIndices;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector MigrationTarget = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float AlertRadius = 3000.f;
};

// ── Crowd Simulation Manager ────────────────────────────────────────────────
UCLASS(ClassGroup = "Crowd", meta = (DisplayName = "Crowd Simulation Manager"))
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

    // ── Configuration ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgents = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float TickInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SeparationRadius = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float CohesionRadius = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FleeSpeed = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float GrazeSpeed = 120.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float MigrateSpeed = 400.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float PredatorAlertRadius = 4000.f;

    // ── Runtime state ──────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_AgentData> Agents;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_HerdData> Herds;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    int32 ActiveAgentCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    int32 ActiveHerdCount = 0;

    // ── Blueprint-callable API ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterAgent(AActor* Actor, ECrowd_DinoSpecies Species, int32 HerdID);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UnregisterAgent(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerFleeResponse(FVector ThreatLocation, float ThreatRadius, ECrowd_DinoSpecies AffectedSpecies);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerMigration(int32 HerdID, FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    FCrowd_HerdData GetHerdData(int32 HerdID) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetAgentCountBySpecies(ECrowd_DinoSpecies Species) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetHerdState(int32 HerdID, ECrowd_HerdState NewState);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    FVector GetHerdCentreOfMass(int32 HerdID) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    bool IsPlayerNearHerd(int32 HerdID, float CheckRadius = 2000.f) const;

    // ── AActor overrides ───────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

protected:
    void SimulateCrowdTick(float DeltaTime);
    void UpdateHerdCentresOfMass();
    void ApplySeparation(FCrowd_AgentData& Agent, const TArray<FCrowd_AgentData>& Neighbours);
    void ApplyCohesion(FCrowd_AgentData& Agent, const FCrowd_HerdData& Herd);
    void ApplyFleeVelocity(FCrowd_AgentData& Agent, FVector ThreatLocation);
    int32 FindOrCreateHerd(ECrowd_DinoSpecies Species, int32 HerdID);

private:
    float AccumulatedTime = 0.f;
    int32 NextHerdID = 0;
};
