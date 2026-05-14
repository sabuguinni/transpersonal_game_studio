#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/TriggerVolume.h"
#include "../SharedTypes.h"
#include "ArchitecturalStructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    CaveDwelling        UMETA(DisplayName = "Cave Dwelling"),
    WoodenShelter       UMETA(DisplayName = "Wooden Shelter"),
    StoneMonument       UMETA(DisplayName = "Stone Monument"),
    UndergroundTunnel   UMETA(DisplayName = "Underground Tunnel"),
    DefensiveBarrier    UMETA(DisplayName = "Defensive Barrier"),
    StorageCache        UMETA(DisplayName = "Storage Cache"),
    RitualSite          UMETA(DisplayName = "Ritual Site"),
    WaterSource         UMETA(DisplayName = "Water Source")
};

UENUM(BlueprintType)
enum class EArch_StructureCondition : uint8
{
    Pristine            UMETA(DisplayName = "Pristine"),
    WellMaintained      UMETA(DisplayName = "Well Maintained"),
    Weathered           UMETA(DisplayName = "Weathered"),
    Damaged             UMETA(DisplayName = "Damaged"),
    Ruined              UMETA(DisplayName = "Ruined"),
    Collapsed           UMETA(DisplayName = "Collapsed")
};

USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::CaveDwelling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureCondition Condition = EArch_StructureCondition::Weathered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float AgeInYears = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 MaxOccupants = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsHabitable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasFirePit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasWaterAccess = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsDefensive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity = 100.0f;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::CaveDwelling;
        Condition = EArch_StructureCondition::Weathered;
        AgeInYears = 50.0f;
        MaxOccupants = 4;
        bIsHabitable = true;
        bHasFirePit = false;
        bHasWaterAccess = false;
        bIsDefensive = false;
        StructuralIntegrity = 100.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitecturalStructureManager : public AActor
{
    GENERATED_BODY()

public:
    AArchitecturalStructureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MainStructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* InteriorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* PropsMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Data")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> InteriorSpawnPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FString> InteriorPropNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bHasNaturalLighting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bHasArtificialLighting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AmbientLightIntensity = 0.3f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void InitializeStructure(EArch_StructureType Type, EArch_StructureCondition InitialCondition);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void UpdateStructuralIntegrity(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    bool CanAccommodateOccupants(int32 RequestedOccupants) const;

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void SpawnInteriorProps();

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void ClearInteriorProps();

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    FVector GetRandomInteriorSpawnPoint() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateLightingConditions(bool bDayTime, float WeatherIntensity);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void ApplyWeatherDamage(float WeatherSeverity, float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Structure Management")
    FString GetStructureDescription() const;

    UFUNCTION(BlueprintPure, Category = "Structure Management")
    bool IsStructureUsable() const;

protected:
    UFUNCTION()
    void OnStructureEntered(AActor* OverlappedActor, AActor* OtherActor);

    UFUNCTION()
    void OnStructureExited(AActor* OverlappedActor, AActor* OtherActor);

private:
    void SetupStructureMeshes();
    void ConfigureInteriorLayout();
    void UpdateVisualCondition();

    UPROPERTY()
    TArray<AActor*> SpawnedProps;

    float LastWeatherDamageTime = 0.0f;
    float WeatherDamageInterval = 60.0f; // Apply weather damage every minute
};