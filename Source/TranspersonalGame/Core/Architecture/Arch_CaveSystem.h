#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/TriggerVolume.h"
#include "../SharedTypes.h"
#include "Arch_CaveSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_CaveType : uint8
{
    None = 0,
    ShallowCave = 1,
    DeepCave = 2,
    CaveSystem = 3,
    UndergroundChamber = 4,
    LavaTube = 5,
    WaterCave = 6
};

UENUM(BlueprintType)
enum class EArch_CaveCondition : uint8
{
    Stable = 0,
    MinorRockfall = 1,
    Unstable = 2,
    PartialCollapse = 3,
    Dangerous = 4,
    Sealed = 5
};

USTRUCT(BlueprintType)
struct FArch_CaveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    EArch_CaveType CaveType = EArch_CaveType::ShallowCave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    EArch_CaveCondition Condition = EArch_CaveCondition::Stable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    float Depth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    float Width = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    float Height = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    float Temperature = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    float Humidity = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    float AirQuality = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    bool bHasWater = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    bool bHasBats = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    bool bHasPaintings = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    int32 MaxOccupants = 5;

    FArch_CaveData()
    {
        CaveType = EArch_CaveType::ShallowCave;
        Condition = EArch_CaveCondition::Stable;
        Depth = 100.0f;
        Width = 200.0f;
        Height = 150.0f;
        Temperature = 15.0f;
        Humidity = 80.0f;
        AirQuality = 90.0f;
        bHasWater = false;
        bHasBats = false;
        bHasPaintings = false;
        MaxOccupants = 5;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_CaveSystem : public AActor
{
    GENERATED_BODY()

public:
    AArch_CaveSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CaveMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* EntranceMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteriorVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* EntranceVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* AmbientLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Data")
    FArch_CaveData CaveData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float StabilityDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float TemperatureVariation = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float HumidityVariation = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    TArray<UStaticMesh*> CaveMeshVariants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    TArray<UStaticMesh*> EntranceMeshVariants;

public:
    UFUNCTION(BlueprintCallable, Category = "Cave")
    void InitializeCave(EArch_CaveType Type, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Cave")
    void UpdateEnvironmentalConditions(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Cave")
    bool IsAccessible() const;

    UFUNCTION(BlueprintCallable, Category = "Cave")
    bool IsSafeForOccupancy() const;

    UFUNCTION(BlueprintCallable, Category = "Cave")
    float GetProtectionValue() const;

    UFUNCTION(BlueprintCallable, Category = "Cave")
    void AddOccupant(AActor* Occupant);

    UFUNCTION(BlueprintCallable, Category = "Cave")
    void RemoveOccupant(AActor* Occupant);

    UFUNCTION(BlueprintCallable, Category = "Cave")
    void TriggerRockfall(float Severity);

    UFUNCTION(BlueprintCallable, Category = "Cave")
    void CreateCavePaintings();

    UFUNCTION(BlueprintCallable, Category = "Cave")
    void SpawnBatColony();

    UFUNCTION(BlueprintCallable, Category = "Cave")
    void AddWaterSource();

    UFUNCTION(BlueprintPure, Category = "Cave")
    EArch_CaveType GetCaveType() const { return CaveData.CaveType; }

    UFUNCTION(BlueprintPure, Category = "Cave")
    EArch_CaveCondition GetCondition() const { return CaveData.Condition; }

    UFUNCTION(BlueprintPure, Category = "Cave")
    float GetTemperature() const { return CaveData.Temperature; }

    UFUNCTION(BlueprintPure, Category = "Cave")
    float GetHumidity() const { return CaveData.Humidity; }

    UFUNCTION(BlueprintPure, Category = "Cave")
    bool HasWater() const { return CaveData.bHasWater; }

    UFUNCTION(BlueprintPure, Category = "Cave")
    bool HasBats() const { return CaveData.bHasBats; }

    UFUNCTION(BlueprintPure, Category = "Cave")
    bool HasPaintings() const { return CaveData.bHasPaintings; }

private:
    void UpdateCaveMesh();
    void UpdateLighting();
    void CheckStructuralStability();
    void UpdateAirQuality();

    UPROPERTY()
    TArray<AActor*> CurrentOccupants;

    float LastEnvironmentUpdate = 0.0f;
    float EnvironmentUpdateInterval = 10.0f;
    float BaseTemperature = 15.0f;
    float BaseHumidity = 80.0f;
};