#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Sound/SoundCue.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "Arch_WaterStructureManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_WaterStructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Structure")
    FString StructureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Structure")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Structure")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Structure")
    float WaterLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Structure")
    float StructuralIntegrity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Structure")
    bool bIsSubmerged;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Structure")
    float FlowRate;

    FArch_WaterStructureData()
    {
        StructureName = TEXT("DefaultWaterStructure");
        Location = FVector::ZeroVector;
        BiomeType = EBiomeType::Forest;
        WaterLevel = 0.0f;
        StructuralIntegrity = 100.0f;
        bIsSubmerged = false;
        FlowRate = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_BridgeConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bridge")
    float SpanLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bridge")
    float Height;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bridge")
    int32 SupportPillars;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bridge")
    FString MaterialType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bridge")
    float LoadCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bridge")
    bool bHasRailings;

    FArch_BridgeConfiguration()
    {
        SpanLength = 1000.0f;
        Height = 200.0f;
        SupportPillars = 2;
        MaterialType = TEXT("Stone");
        LoadCapacity = 500.0f;
        bHasRailings = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_WalkwayData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walkway")
    TArray<FVector> PathPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walkway")
    float WalkwayWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walkway")
    float ElevationHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walkway")
    bool bIsElevated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walkway")
    FString SurfaceMaterial;

    FArch_WalkwayData()
    {
        WalkwayWidth = 200.0f;
        ElevationHeight = 150.0f;
        bIsElevated = true;
        SurfaceMaterial = TEXT("Wood");
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_WaterStructureManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_WaterStructureManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core water structure management
    UFUNCTION(BlueprintCallable, Category = "Water Structures")
    void InitializeWaterStructures();

    UFUNCTION(BlueprintCallable, Category = "Water Structures")
    void CreateBridgeAtLocation(const FVector& StartLocation, const FVector& EndLocation, const FArch_BridgeConfiguration& Config);

    UFUNCTION(BlueprintCallable, Category = "Water Structures")
    void CreateElevatedWalkway(const FArch_WalkwayData& WalkwayData);

    UFUNCTION(BlueprintCallable, Category = "Water Structures")
    void CreateWaterCollectionSystem(const FVector& Location, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Water Structures")
    void UpdateStructuralIntegrity(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Water Structures")
    void CheckWaterLevelChanges();

    // Bridge-specific methods
    UFUNCTION(BlueprintCallable, Category = "Bridges")
    AActor* SpawnBridgeSegment(const FVector& Location, const FRotator& Rotation, const FString& MaterialType);

    UFUNCTION(BlueprintCallable, Category = "Bridges")
    void CalculateBridgeStress(AActor* BridgeActor, float LoadWeight);

    UFUNCTION(BlueprintCallable, Category = "Bridges")
    bool ValidateBridgePlacement(const FVector& StartPoint, const FVector& EndPoint);

    // Walkway methods
    UFUNCTION(BlueprintCallable, Category = "Walkways")
    void GenerateWalkwayPath(const TArray<FVector>& ControlPoints);

    UFUNCTION(BlueprintCallable, Category = "Walkways")
    void AddWalkwaySupports(const FVector& Location, float Height);

    UFUNCTION(BlueprintCallable, Category = "Walkways")
    void UpdateWalkwaySafety(AActor* WalkwayActor);

    // Water collection systems
    UFUNCTION(BlueprintCallable, Category = "Water Collection")
    void CreateAqueductSystem(const FVector& SourceLocation, const FVector& DestinationLocation);

    UFUNCTION(BlueprintCallable, Category = "Water Collection")
    void SetupRainwaterCollection(const FVector& Location, float CollectionArea);

    UFUNCTION(BlueprintCallable, Category = "Water Collection")
    float CalculateWaterFlow(const FVector& StartPoint, const FVector& EndPoint, float Elevation);

    // Environmental interaction
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateStructuresForWeather(float RainIntensity, float WindSpeed);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ApplyWaterDamage(AActor* StructureActor, float WaterExposure);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void HandleFloodConditions(float FloodLevel);

    // Audio and visual effects
    UFUNCTION(BlueprintCallable, Category = "Effects")
    void PlayWaterStructureAudio(const FVector& Location, const FString& SoundType);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void UpdateVisualEffects(AActor* StructureActor, float WaterLevel);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void CreateSplashEffects(const FVector& Location, float Intensity);

protected:
    // Water structure data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Structures")
    TArray<FArch_WaterStructureData> WaterStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Structures")
    TArray<AActor*> ActiveBridges;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Structures")
    TArray<AActor*> ActiveWalkways;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Structures")
    TArray<AActor*> WaterCollectionSystems;

    // Configuration parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MaxBridgeSpan;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MinWalkwayWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float WaterLevelThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float StructuralDecayRate;

    // Audio components
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TObjectPtr<UAudioComponent> WaterFlowAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TObjectPtr<UAudioComponent> StructuralStressAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TObjectPtr<USoundCue> BridgeCreakSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TObjectPtr<USoundCue> WaterSplashSound;

    // Material references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TObjectPtr<UMaterialInterface> StoneBridgeMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TObjectPtr<UMaterialInterface> WoodWalkwayMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TObjectPtr<UMaterialInterface> WetStoneMaterial;

private:
    // Internal management
    void UpdateWaterStructureStates();
    void CheckStructuralLimits();
    void OptimizeWaterStructures();
    bool IsLocationNearWater(const FVector& Location, float Radius = 500.0f);
    float CalculateWaterDistance(const FVector& Location);
    void CleanupInvalidStructures();
};