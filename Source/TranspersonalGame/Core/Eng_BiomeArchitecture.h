#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "../SharedTypes.h"
#include "Eng_BiomeArchitecture.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBiomeTransition, EEng_BiomeType, FromBiome, EEng_BiomeType, ToBiome);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBiomeLoaded, EEng_BiomeType, BiomeType);

/**
 * Core biome data structure for the world generation system
 * Defines environmental parameters, spawn rules, and atmospheric settings
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Core")
    EEng_BiomeType BiomeType = EEng_BiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Core")
    FString BiomeName = TEXT("Unnamed Biome");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float Temperature = 25.0f; // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float Humidity = 0.5f; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float Precipitation = 0.3f; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FLinearColor SkyColor = FLinearColor(0.4f, 0.7f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<TSoftObjectPtr<UStaticMesh>> VegetationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wildlife")
    TArray<TSoftClassPtr<APawn>> DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wildlife")
    float DinosaurSpawnRate = 0.1f;

    FEng_BiomeDefinition()
    {
        BiomeType = EEng_BiomeType::Savanna;
        BiomeName = TEXT("Default Savanna");
        Temperature = 25.0f;
        Humidity = 0.5f;
        Precipitation = 0.3f;
    }
};

/**
 * Biome zone component - attached to actors that define biome boundaries
 * Handles biome detection and transition triggers
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_BiomeZoneComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_BiomeZoneComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    EEng_BiomeType BiomeType = EEng_BiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    float ZoneRadius = 5000.0f; // 5km radius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    int32 ZonePriority = 0; // Higher priority overrides lower

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnBiomeTransition OnBiomeEntered;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnBiomeTransition OnBiomeExited;

    UFUNCTION(BlueprintCallable, Category = "Biome Zone")
    bool IsLocationInZone(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Zone")
    float GetDistanceToZoneCenter(const FVector& Location) const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    TArray<AActor*> ActorsInZone;

    void CheckActorProximity();
};

/**
 * Global biome management subsystem
 * Handles biome registration, queries, and environmental state
 */
UCLASS()
class TRANSPERSONALGAME_API UEng_BiomeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Biome Manager")
    void RegisterBiome(const FEng_BiomeDefinition& BiomeDefinition);

    UFUNCTION(BlueprintCallable, Category = "Biome Manager")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Manager")
    FEng_BiomeDefinition GetBiomeDefinition(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Manager")
    TArray<EEng_BiomeType> GetAllRegisteredBiomes() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Manager")
    void SetCurrentBiome(EEng_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Biome Manager")
    EEng_BiomeType GetCurrentBiome() const { return CurrentBiome; }

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnBiomeTransition OnBiomeChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnBiomeLoaded OnBiomeSystemReady;

    // Environmental queries
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetTemperatureAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetHumidityAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    FLinearColor GetSkyColorForBiome(EEng_BiomeType BiomeType) const;

protected:
    UPROPERTY()
    TMap<EEng_BiomeType, FEng_BiomeDefinition> RegisteredBiomes;

    UPROPERTY()
    EEng_BiomeType CurrentBiome = EEng_BiomeType::Savanna;

    UPROPERTY()
    TArray<UEng_BiomeZoneComponent*> ActiveZones;

private:
    void InitializeDefaultBiomes();
    void RegisterBiomeZone(UEng_BiomeZoneComponent* ZoneComponent);
    void UnregisterBiomeZone(UEng_BiomeZoneComponent* ZoneComponent);

    friend class UEng_BiomeZoneComponent;
};

/**
 * Biome marker actor - placed in the world to define biome boundaries
 * Automatically registers with the biome manager on spawn
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_BiomeMarker : public AActor
{
    GENERATED_BODY()

public:
    AEng_BiomeMarker();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MarkerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UEng_BiomeZoneComponent* BiomeZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Marker")
    EEng_BiomeType BiomeType = EEng_BiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Marker")
    float BiomeRadius = 5000.0f;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};