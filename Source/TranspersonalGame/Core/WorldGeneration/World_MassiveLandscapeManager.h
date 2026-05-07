#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeEditorSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "World_MassiveLandscapeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> FeatureTypes;

    FWorld_BiomeZone()
    {
        BiomeName = TEXT("Unknown");
        CenterLocation = FVector::ZeroVector;
        Radius = 1000.0f;
        BiomeColor = FLinearColor::White;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_LandscapeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 ComponentCountX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 ComponentCountY;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 SectionsPerComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 QuadsPerSection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    FVector WorldScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    float TargetAreaKm2;

    FWorld_LandscapeSettings()
    {
        ComponentCountX = 32;
        ComponentCountY = 16;
        SectionsPerComponent = 2;
        QuadsPerSection = 127;
        WorldScale = FVector(25.0f, 50.0f, 1.0f);
        TargetAreaKm2 = 200.0f;
    }
};

/**
 * Manages massive landscape creation and biome zone setup for 200km2 world
 * Handles landscape scaling, biome placement, and environmental feature distribution
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_MassiveLandscapeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_MassiveLandscapeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Landscape Management
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    bool CreateMassiveLandscape();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    bool ScaleLandscapeToTarget();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    ALandscape* GetCurrentLandscape();

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void InitializeBiomeZones();

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void CreateBiomeMarkers();

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void PopulateBiomeFeatures();

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    FWorld_BiomeZone GetBiomeAtLocation(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    TArray<FWorld_BiomeZone> GetAllBiomes() const { return BiomeZones; }

    // Environmental Features
    UFUNCTION(BlueprintCallable, Category = "Environmental Features")
    void SpawnEnvironmentalFeatures(const FWorld_BiomeZone& Biome, int32 FeatureCount);

    UFUNCTION(BlueprintCallable, Category = "Environmental Features")
    AActor* CreateFeatureActor(const FString& FeatureType, FVector Location, FRotator Rotation);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    float CalculateWorldArea() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector ConvertKilometersToUnrealUnits(FVector KmCoordinates) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsLocationInBiome(FVector Location, const FWorld_BiomeZone& Biome) const;

protected:
    // Landscape Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Settings")
    FWorld_LandscapeSettings LandscapeConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landscape")
    ALandscape* ManagedLandscape;

    // Biome Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome Actors")
    TArray<AActor*> BiomeMarkerActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environmental Features")
    TArray<AActor*> EnvironmentalFeatureActors;

    // Generation Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    bool bAutoCreateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    int32 MaxFeaturesPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    float FeatureSpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    float MinFeatureDistance;

private:
    // Internal helper functions
    void SetupDefaultBiomes();
    FVector GetRandomLocationInBiome(const FWorld_BiomeZone& Biome) const;
    bool IsValidFeatureLocation(FVector Location) const;
    void ApplyFeatureMesh(AActor* FeatureActor, const FString& FeatureType);
};