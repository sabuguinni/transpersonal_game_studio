#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdSimulationManager.generated.h"

// ============================================================
// ENUMS — global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_HerdState : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Resting     UMETA(DisplayName = "Resting"),
    Drinking    UMETA(DisplayName = "Drinking")
};

UENUM(BlueprintType)
enum class ECrowd_PackHuntMode : uint8
{
    Patrolling          UMETA(DisplayName = "Patrolling"),
    Stalking            UMETA(DisplayName = "Stalking"),
    CoordinatedAttack   UMETA(DisplayName = "CoordinatedAttack"),
    Retreating          UMETA(DisplayName = "Retreating")
};

// ============================================================
// STRUCTS — global scope (RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct FCrowd_HerdDescriptor
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FName HerdID;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FString SpeciesName;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    int32 AgentCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FVector HerdCenterLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerdState CurrentState = ECrowd_HerdState::Grazing;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    float MigrationProgress = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FVector MigrationDestination = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FCrowd_PackDescriptor
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Pack")
    FName PackID;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Pack")
    FString SpeciesName;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Pack")
    int32 AgentCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Pack")
    FVector PackCenterLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Pack")
    ECrowd_PackHuntMode HuntingMode = ECrowd_PackHuntMode::Patrolling;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Pack")
    FName TargetHerdID;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Pack")
    float HuntCooldown = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Pack")
    TArray<FVector> PatrolWaypoints;
};

// ============================================================
// UCLASS
// ============================================================

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Registration
    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void RegisterHerd(const FCrowd_HerdDescriptor& HerdDesc);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Pack")
    void RegisterPack(const FCrowd_PackDescriptor& PackDesc);

    // Simulation tick
    UFUNCTION(BlueprintCallable, Category = "Crowd|Simulation")
    void UpdateHerdBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Simulation")
    void UpdatePackBehavior(float DeltaTime);

    // Threat system
    UFUNCTION(BlueprintCallable, Category = "Crowd|Threat")
    void NotifyThreatAtLocation(FVector ThreatLocation, float ThreatRadius, float ThreatIntensity);

    // Queries
    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    int32 GetTotalActiveAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    TArray<FCrowd_HerdDescriptor> GetHerdsInRadius(FVector Center, float Radius) const;

    // Config
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxHerdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxPackAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float HerdUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float PackUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float AgentSeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float HerdCohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float PackHuntingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FleeResponseRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    bool bSimulationActive;

private:
    UPROPERTY()
    TArray<FCrowd_HerdDescriptor> RegisteredHerds;

    UPROPERTY()
    TArray<FCrowd_PackDescriptor> RegisteredPacks;
};
