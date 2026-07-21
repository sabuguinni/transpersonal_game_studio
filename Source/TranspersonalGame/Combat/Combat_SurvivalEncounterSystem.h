#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TriggerSphere.h"
#include "Engine/TargetPoint.h"
#include "SharedTypes.h"
#include "Combat_SurvivalEncounterSystem.generated.h"

UENUM(BlueprintType)
enum class ECombat_SurvivalThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Critical    UMETA(DisplayName = "Critical Threat"),
    Lethal      UMETA(DisplayName = "Lethal Threat")
};

UENUM(BlueprintType)
enum class ECombat_EnvironmentalHazard : uint8
{
    None            UMETA(DisplayName = "No Hazard"),
    CliffEdge       UMETA(DisplayName = "Cliff Edge"),
    DenseVegetation UMETA(DisplayName = "Dense Vegetation"),
    WaterHazard     UMETA(DisplayName = "Water Hazard"),
    RockyTerrain    UMETA(DisplayName = "Rocky Terrain"),
    PredatorDen     UMETA(DisplayName = "Predator Den"),
    DeadEnd         UMETA(DisplayName = "Dead End")
};

USTRUCT(BlueprintType)
struct FCombat_ThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    ECombat_SurvivalThreatLevel ThreatLevel = ECombat_SurvivalThreatLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float ThreatDistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    FVector ThreatDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    AActor* ThreatSource = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float ThreatIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    bool bIsActiveThreat = false;

    FCombat_ThreatAssessment()
    {
        ThreatLevel = ECombat_SurvivalThreatLevel::Safe;
        ThreatDistance = 0.0f;
        ThreatDirection = FVector::ZeroVector;
        ThreatSource = nullptr;
        ThreatIntensity = 0.0f;
        bIsActiveThreat = false;
    }
};

USTRUCT(BlueprintType)
struct FCombat_SurvivalWaypoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    FString WaypointType = TEXT("Generic");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    float SafetyRating = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    bool bIsOccupied = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    AActor* WaypointActor = nullptr;

    FCombat_SurvivalWaypoint()
    {
        Location = FVector::ZeroVector;
        WaypointType = TEXT("Generic");
        SafetyRating = 0.5f;
        bIsOccupied = false;
        WaypointActor = nullptr;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_SurvivalEncounterSystem : public AActor
{
    GENERATED_BODY()

public:
    ACombat_SurvivalEncounterSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* EncounterZone;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualizationMesh;

    // Threat Assessment System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float ThreatDetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float ThreatUpdateInterval = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Threat Assessment")
    TArray<FCombat_ThreatAssessment> ActiveThreats;

    UPROPERTY(BlueprintReadOnly, Category = "Threat Assessment")
    ECombat_SurvivalThreatLevel CurrentThreatLevel = ECombat_SurvivalThreatLevel::Safe;

    // Survival Waypoint System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoints")
    TArray<FCombat_SurvivalWaypoint> SurvivalWaypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoints")
    float WaypointSearchRadius = 3000.0f;

    // Environmental Hazards
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TArray<ECombat_EnvironmentalHazard> DetectedHazards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float HazardAvoidanceRadius = 500.0f;

    // Encounter Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    bool bEncounterActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float EncounterDuration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    int32 MaxSimultaneousThreats = 3;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Survival Combat")
    void InitializeEncounterSystem();

    UFUNCTION(BlueprintCallable, Category = "Survival Combat")
    FCombat_ThreatAssessment AssessThreat(AActor* PotentialThreat, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Survival Combat")
    FCombat_SurvivalWaypoint FindNearestSafeWaypoint(const FVector& FromLocation);

    UFUNCTION(BlueprintCallable, Category = "Survival Combat")
    TArray<FCombat_SurvivalWaypoint> GetEscapeRoutes(const FVector& FromLocation, const FVector& ThreatDirection);

    UFUNCTION(BlueprintCallable, Category = "Survival Combat")
    void UpdateThreatAssessment();

    UFUNCTION(BlueprintCallable, Category = "Survival Combat")
    void RegisterEnvironmentalHazard(ECombat_EnvironmentalHazard HazardType, const FVector& HazardLocation);

    UFUNCTION(BlueprintCallable, Category = "Survival Combat")
    bool IsLocationSafe(const FVector& Location, float SafetyRadius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Survival Combat")
    void StartSurvivalEncounter();

    UFUNCTION(BlueprintCallable, Category = "Survival Combat")
    void EndSurvivalEncounter();

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival Combat")
    void OnThreatLevelChanged(ECombat_SurvivalThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival Combat")
    void OnSafeWaypointFound(const FCombat_SurvivalWaypoint& Waypoint);

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival Combat")
    void OnEnvironmentalHazardDetected(ECombat_EnvironmentalHazard HazardType, const FVector& HazardLocation);

private:
    // Internal state
    float ThreatUpdateTimer = 0.0f;
    TArray<AActor*> CachedActorsInRange;
    
    // Helper functions
    void UpdateCachedActors();
    float CalculateThreatIntensity(AActor* ThreatActor, const FVector& PlayerLocation);
    ECombat_SurvivalThreatLevel DetermineThreatLevel(float ThreatIntensity, float Distance);
    void ProcessEnvironmentalFactors(const FVector& Location);
};