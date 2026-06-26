#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/SkeletalMesh.h"
#include "CrowdHerdManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_HerdBehavior : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Resting     UMETA(DisplayName = "Resting"),
    Drinking    UMETA(DisplayName = "Drinking"),
};

UENUM(BlueprintType)
enum class ECrowd_SpeciesRole : uint8
{
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Predator    UMETA(DisplayName = "Predator"),
    Apex        UMETA(DisplayName = "Apex Predator"),
    Scavenger   UMETA(DisplayName = "Scavenger"),
};

USTRUCT(BlueprintType)
struct FCrowd_HerdAgent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_HerdBehavior CurrentBehavior = ECrowd_HerdBehavior::Grazing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float AlertLevel = 0.0f;  // 0=calm, 1=panicked

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector FlockCenter = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdGroup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FString HerdID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_SpeciesRole Role = ECrowd_SpeciesRole::Herbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    TArray<FCrowd_HerdAgent> Agents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float TerritoryRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_HerdBehavior GroupBehavior = ECrowd_HerdBehavior::Grazing;
};

/**
 * ACrowd_HerdManager
 * Manages prehistoric animal herds and crowd simulation groups.
 * Handles flocking, grazing, migration, and threat response.
 * Agent #13 — Crowd & Traffic Simulation
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Crowd Herd Manager"))
class TRANSPERSONALGAME_API ACrowd_HerdManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_HerdManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Herd Registration ---
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterHerd(const FCrowd_HerdGroup& NewHerd);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UnregisterHerd(const FString& HerdID);

    // --- Behavior Control ---
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetHerdBehavior(const FString& HerdID, ECrowd_HerdBehavior NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerFleeResponse(FVector ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerMigration(const FString& HerdID, FVector Destination);

    // --- Flocking Algorithm ---
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    FVector CalculateFlockingVector(const FCrowd_HerdAgent& Agent, const FCrowd_HerdGroup& Herd);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    FVector CalculateSeparation(const FCrowd_HerdAgent& Agent, const TArray<FCrowd_HerdAgent>& Neighbors, float MinDist);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    FVector CalculateCohesion(const FCrowd_HerdAgent& Agent, const TArray<FCrowd_HerdAgent>& Neighbors);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    FVector CalculateAlignment(const FCrowd_HerdAgent& Agent, const TArray<FCrowd_HerdAgent>& Neighbors);

    // --- Query ---
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd")
    int32 GetTotalAgentCount() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd")
    TArray<FString> GetActiveHerdIDs() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd")
    FCrowd_HerdGroup GetHerdByID(const FString& HerdID) const;

    // --- LOD ---
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UpdateCrowdLOD(FVector PlayerLocation);

    // --- Config ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FlockingUpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SeparationDistance = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float CohesionWeight = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SeparationWeight = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float AlignmentWeight = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgentsPerHerd = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LODDistanceClose = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LODDistanceFar = 8000.0f;

private:
    UPROPERTY()
    TArray<FCrowd_HerdGroup> ActiveHerds;

    float FlockingTimer = 0.0f;

    void UpdateHerdBehaviors(float DeltaTime);
    void UpdateFlocking(FCrowd_HerdGroup& Herd, float DeltaTime);
    FVector GetHerdCenter(const FCrowd_HerdGroup& Herd) const;
};
