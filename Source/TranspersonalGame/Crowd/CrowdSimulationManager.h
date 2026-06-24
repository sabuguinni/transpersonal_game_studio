// CrowdSimulationManager.h
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric herd simulation subsystem

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdSimulationManager.generated.h"

// ── Shared herd species enum ──
UENUM(BlueprintType)
enum class ECrowd_HerdSpecies : uint8
{
    Parasaurolophus     UMETA(DisplayName = "Parasaurolophus"),
    Protoceratops       UMETA(DisplayName = "Protoceratops"),
    Pachycephalosaurus  UMETA(DisplayName = "Pachycephalosaurus"),
    Tsintaosaurus       UMETA(DisplayName = "Tsintaosaurus"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
};

// ── Herd data struct ──
USTRUCT(BlueprintType)
struct FCrowd_HerdData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FName HerdName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_HerdSpecies Species = ECrowd_HerdSpecies::Parasaurolophus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 MemberCount = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector HerdCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector CurrentWaypoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MoveSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float FlockingRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsFleeing = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float TimeSinceLastUpdate = 0.0f;
};

// ── Crowd Simulation Manager — World Subsystem ──
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Register a new herd group
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterHerd(const FCrowd_HerdData& HerdData);

    // Tick herd movement (called from GameMode or tick)
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UpdateHerdBehavior(float DeltaTime);

    // Trigger flee response for all herds within radius of threat
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerFleeResponse(FVector ThreatLocation, float ThreatRadius);

    // Query
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd")
    int32 GetActiveCrowdCount() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd")
    FCrowd_HerdData GetHerdByName(FName HerdName) const;

    // Config
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxCrowdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float HerdUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FlockingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SeparationRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State",
        meta = (AllowPrivateAccess = "true"))
    bool bCrowdSimulationActive;

private:
    UPROPERTY()
    TArray<FCrowd_HerdData> RegisteredHerds;
};
