#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "World_BiomeStreamingManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float StreamingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherType WeatherPattern;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float VegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor BiomeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 LODLevel;

    FWorld_BiomeConfig()
    {
        BiomeName = TEXT("DefaultBiome");
        Location = FVector::ZeroVector;
        StreamingRadius = 1000.0f;
        BiomeType = EBiomeType::Temperate;
        WeatherPattern = EWeatherType::Clear;
        FogDensity = 0.5f;
        VegetationDensity = 0.5f;
        BiomeColor = FLinearColor::White;
        LODLevel = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    FString FromBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    FString ToBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    FVector TransitionLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float BlendFactor;

    FWorld_WeatherTransition()
    {
        FromBiome = TEXT("None");
        ToBiome = TEXT("None");
        TransitionLocation = FVector::ZeroVector;
        TransitionRadius = 500.0f;
        BlendFactor = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeStreamingManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeStreamingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* StreamingDetectionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeConfig> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    TArray<FWorld_WeatherTransition> WeatherTransitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float StreamingUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float MaxStreamingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveStreamingZones;

private:
    UPROPERTY()
    TArray<AActor*> ActiveBiomeActors;

    UPROPERTY()
    TArray<AActor*> StreamingVolumeActors;

    UPROPERTY()
    AActor* WeatherControllerActor;

    float LastStreamingUpdate;
    FString CurrentActiveBiome;

public:
    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void InitializeBiomeSystem();

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void CreateBiomeZone(const FWorld_BiomeConfig& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void UpdateStreamingZones(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    FString GetCurrentBiome(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void UpdateWeatherSystem(const FString& BiomeName, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void CreateWeatherTransition(const FWorld_WeatherTransition& Transition);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeBiomePerformance(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetBiomeLOD(const FString& BiomeName, int32 LODLevel);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void GenerateDefaultBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void ValidateBiomeConfiguration();

protected:
    UFUNCTION()
    void OnStreamingVolumeEntered(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnStreamingVolumeExited(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

private:
    void CreateVegetationCluster(const FVector& Location, float Density, const FString& BiomeName);
    void UpdateBiomeLighting(const FString& BiomeName, const FLinearColor& BiomeColor);
    void CleanupInactiveBiomes();
    float CalculateStreamingPriority(const FVector& BiomeLocation, const FVector& PlayerLocation) const;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UWorld_BiomeStreamingSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    AWorld_BiomeStreamingManager* GetBiomeStreamingManager() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void RegisterBiomeManager(AWorld_BiomeStreamingManager* Manager);

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    FString GetPlayerCurrentBiome() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateGlobalStreamingSettings(float MaxDistance, int32 MaxZones);

private:
    UPROPERTY()
    AWorld_BiomeStreamingManager* BiomeManager;

    UPROPERTY()
    FString CachedPlayerBiome;

    float LastBiomeCheck;
};