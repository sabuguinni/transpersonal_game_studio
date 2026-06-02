#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "../SharedTypes.h"
#include "Eng_WorldArchitect.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_WorldBounds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Bounds")
    FVector MinBounds = FVector(-10000, -10000, -1000);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Bounds")
    FVector MaxBounds = FVector(10000, 10000, 2000);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Bounds")
    float BiomeTransitionZone = 500.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_TerrainSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightScale = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float NoiseScale = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 TerrainResolution = 1024;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bEnableWorldPartition = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_WorldArchitect : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_WorldArchitect();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void InitializeWorldBounds(const FEng_WorldBounds& Bounds);

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void SetupBiomeZones();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void ValidateWorldPartition();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    FVector GetWorldCenter() const;

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool IsLocationInBounds(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    EBiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void RegisterBiomeManager(class UBiomeManager* Manager);

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void SetTerrainSettings(const FEng_TerrainSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "World Architecture", CallInEditor = true)
    void ValidateWorldConfiguration();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FEng_WorldBounds WorldBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FEng_TerrainSettings TerrainSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<FVector> BiomeZoneCenters;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    bool bWorldInitialized = false;

private:
    class UBiomeManager* BiomeManagerRef = nullptr;
    
    void CalculateBiomeZones();
    void ValidateActorCounts();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_WorldAnchor : public AActor
{
    GENERATED_BODY()

public:
    AEng_WorldAnchor();

    UFUNCTION(BlueprintCallable, Category = "World Anchor")
    void SetAnchorType(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "World Anchor")
    EBiomeType GetAnchorType() const { return AnchorBiome; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    EBiomeType AnchorBiome = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float InfluenceRadius = 1000.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* AnchorMesh;
};