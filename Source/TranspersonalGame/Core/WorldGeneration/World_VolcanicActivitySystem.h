#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/PointLight.h"
#include "SharedTypes.h"
#include "World_VolcanicActivitySystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_VolcanicState : uint8
{
    Dormant     UMETA(DisplayName = "Dormant"),
    Active      UMETA(DisplayName = "Active"),
    Erupting    UMETA(DisplayName = "Erupting"),
    Cooling     UMETA(DisplayName = "Cooling")
};

UENUM(BlueprintType)
enum class EWorld_LavaFlowType : uint8
{
    Pahoehoe    UMETA(DisplayName = "Pahoehoe"),
    AALava      UMETA(DisplayName = "A'a Lava"),
    Pillow      UMETA(DisplayName = "Pillow Lava"),
    Block       UMETA(DisplayName = "Block Lava")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_VolcanicData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic")
    EWorld_VolcanicState VolcanicState = EWorld_VolcanicState::Dormant;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic")
    float EruptionIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic")
    float LavaTemperature = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic")
    float AshCloudRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic")
    float ThermalRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic")
    bool bHasLavaFlow = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic")
    EWorld_LavaFlowType LavaFlowType = EWorld_LavaFlowType::Pahoehoe;

    FWorld_VolcanicData()
    {
        VolcanicState = EWorld_VolcanicState::Dormant;
        EruptionIntensity = 0.0f;
        LavaTemperature = 1200.0f;
        AshCloudRadius = 5000.0f;
        ThermalRadius = 2000.0f;
        bHasLavaFlow = false;
        LavaFlowType = EWorld_LavaFlowType::Pahoehoe;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_ThermalSpringData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal")
    float Temperature = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal")
    float HealingRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal")
    float SteamIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal")
    bool bIsActive = true;

    FWorld_ThermalSpringData()
    {
        Temperature = 60.0f;
        HealingRate = 5.0f;
        SteamIntensity = 1.0f;
        bIsActive = true;
    }
};

/**
 * Manages volcanic activity system including eruptions, lava flows, thermal springs, and volcanic terrain
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_VolcanicActivitySystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_VolcanicActivitySystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core volcanic system properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    FWorld_VolcanicData VolcanicData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    TArray<FVector> VolcanicLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    TArray<FVector> LavaFlowPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    TArray<FWorld_ThermalSpringData> ThermalSprings;

    // Volcanic activity management
    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void InitializeVolcanicSystem();

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void CreateVolcanicTerrain();

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void SpawnLavaFlows();

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void CreateThermalSprings();

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void UpdateVolcanicActivity(float DeltaTime);

    // Eruption system
    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void TriggerEruption(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void StopEruption();

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    bool IsVolcanoActive() const;

    // Lava flow system
    UFUNCTION(BlueprintCallable, Category = "Lava System")
    void CreateLavaFlow(const FVector& StartLocation, const FVector& EndLocation, EWorld_LavaFlowType FlowType);

    UFUNCTION(BlueprintCallable, Category = "Lava System")
    void UpdateLavaFlows(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Lava System")
    float GetLavaTemperatureAtLocation(const FVector& Location) const;

    // Thermal spring system
    UFUNCTION(BlueprintCallable, Category = "Thermal System")
    void SpawnThermalSpring(const FVector& Location, const FWorld_ThermalSpringData& SpringData);

    UFUNCTION(BlueprintCallable, Category = "Thermal System")
    bool IsLocationNearThermalSpring(const FVector& Location, float Radius = 500.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Thermal System")
    float GetHealingRateAtLocation(const FVector& Location) const;

    // Volcanic hazard system
    UFUNCTION(BlueprintCallable, Category = "Hazard System")
    bool IsLocationInAshCloud(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Hazard System")
    float GetVolcanicHazardLevel(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Hazard System")
    void ApplyVolcanicDamage(AActor* Actor, float DamageAmount);

    // Resource system
    UFUNCTION(BlueprintCallable, Category = "Resource System")
    void CreateObsidianFields();

    UFUNCTION(BlueprintCallable, Category = "Resource System")
    void CreateVolcanicAshDeposits();

    UFUNCTION(BlueprintCallable, Category = "Resource System")
    bool CanHarvestObsidian(const FVector& Location) const;

protected:
    // Internal volcanic state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Internal")
    float EruptionTimer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Internal")
    float LavaFlowTimer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Internal")
    bool bSystemInitialized;

    // Spawned actors
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawned Actors")
    TArray<AActor*> VolcanicActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawned Actors")
    TArray<AActor*> LavaFlowActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawned Actors")
    TArray<AActor*> ThermalSpringActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawned Actors")
    TArray<AActor*> VolcanicLights;

    // Helper functions
    void SpawnVolcanicCone(const FVector& Location, float Scale, EWorld_VolcanicState State);
    void SpawnVolcanicVent(const FVector& Location);
    void SpawnVolcanicRocks(const FVector& CenterLocation, int32 Count, float Radius);
    void UpdateVolcanicLighting();
    float CalculateDistanceToNearestVolcano(const FVector& Location) const;
};