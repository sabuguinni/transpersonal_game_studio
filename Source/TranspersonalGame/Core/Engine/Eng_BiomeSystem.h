#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Eng_BiomeSystem.generated.h"

// Forward declarations
class UBiomeManager;
class ATerrainActor;

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Savanna         UMETA(DisplayName = "Savanna"),
    Swamp           UMETA(DisplayName = "Swamp"),
    Forest          UMETA(DisplayName = "Forest"),
    Desert          UMETA(DisplayName = "Desert"),
    Mountain        UMETA(DisplayName = "Mountain"),
    Transition      UMETA(DisplayName = "Transition Zone")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 30000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f; // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f; // 0-1 range

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Fertility = 0.7f; // Vegetation density multiplier

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSubclassOf<class ADinosaurBase>> NativeDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxActorsPerBiome = 4000;

    FEng_BiomeData()
    {
        BiomeType = EEng_BiomeType::Savanna;
        CenterLocation = FVector::ZeroVector;
        Radius = 30000.0f;
        Temperature = 25.0f;
        Humidity = 0.5f;
        Fertility = 0.7f;
        MaxActorsPerBiome = 4000;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_TerrainSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightScale = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float NoiseScale = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 Octaves = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Persistence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Lacunarity = 2.0f;

    FEng_TerrainSettings()
    {
        HeightScale = 1000.0f;
        NoiseScale = 0.001f;
        Octaves = 4;
        Persistence = 0.5f;
        Lacunarity = 2.0f;
    }
};

/**
 * Core biome management system for the prehistoric world
 * Handles biome distribution, terrain generation, and environmental parameters
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_BiomeSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_BiomeSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeData GetBiomeData(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<FEng_BiomeData> GetAllBiomes() const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool IsLocationInBiome(const FVector& WorldLocation, EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetDistanceToBiomeCenter(const FVector& WorldLocation, EEng_BiomeType BiomeType) const;

    // Environmental Parameters
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetFertilityAtLocation(const FVector& WorldLocation) const;

    // Terrain Generation
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    float GenerateHeightAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    FVector GetTerrainNormalAtLocation(const FVector& WorldLocation) const;

    // Actor Limits
    UFUNCTION(BlueprintCallable, Category = "Actor Management")
    bool CanSpawnActorInBiome(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Actor Management")
    int32 GetActorCountInBiome(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Actor Management")
    void RegisterActorInBiome(AActor* Actor, EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Actor Management")
    void UnregisterActorFromBiome(AActor* Actor, EEng_BiomeType BiomeType);

    // Validation
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateBiomeConfiguration() const;

    UFUNCTION(BlueprintCallable, Category = "Validation")
    void LogBiomeStatus() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    TMap<EEng_BiomeType, FEng_BiomeData> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Configuration")
    FEng_TerrainSettings TerrainSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
    TMap<EEng_BiomeType, TArray<TWeakObjectPtr<AActor>>> BiomeActors;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
    bool bIsInitialized = false;

private:
    void SetupDefaultBiomes();
    float CalculateBlendWeight(const FVector& Location, const FEng_BiomeData& BiomeData) const;
    float GenerateNoise(float X, float Y, int32 Octaves, float Persistence, float Scale) const;
};

/**
 * Component for actors that need to be aware of their biome
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_BiomeAwareComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_BiomeAwareComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Biome Awareness")
    EEng_BiomeType GetCurrentBiome() const { return CurrentBiome; }

    UFUNCTION(BlueprintCallable, Category = "Biome Awareness")
    FEng_BiomeData GetCurrentBiomeData() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Awareness")
    void UpdateBiomeAwareness();

    UPROPERTY(BlueprintAssignable, Category = "Events")
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBiomeChanged, EEng_BiomeType, OldBiome, EEng_BiomeType, NewBiome);

    FOnBiomeChanged OnBiomeChanged;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Biome State")
    EEng_BiomeType CurrentBiome = EEng_BiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float UpdateFrequency = 1.0f; // Updates per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoRegisterWithBiome = true;

private:
    float LastUpdateTime = 0.0f;
    UEng_BiomeSystem* BiomeSystem = nullptr;
};

#include "Eng_BiomeSystem.generated.h"