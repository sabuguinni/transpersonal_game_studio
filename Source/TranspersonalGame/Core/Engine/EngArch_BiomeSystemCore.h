#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "EngArch_BiomeSystemCore.generated.h"

UENUM(BlueprintType)
enum class EEngArch_BiomeType : uint8
{
    Savanna         UMETA(DisplayName = "Savanna"),
    Forest          UMETA(DisplayName = "Forest"),
    Desert          UMETA(DisplayName = "Desert"),
    Mountain        UMETA(DisplayName = "Mountain"),
    Wetland         UMETA(DisplayName = "Wetland"),
    Volcanic        UMETA(DisplayName = "Volcanic"),
    Coastal         UMETA(DisplayName = "Coastal"),
    None            UMETA(DisplayName = "None")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEngArch_BiomeProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEngArch_BiomeType BiomeType = EEngArch_BiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterAvailability = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius = 25000.0f;

    FEngArch_BiomeProperties()
    {
        BiomeType = EEngArch_BiomeType::Savanna;
        Temperature = 25.0f;
        Humidity = 0.5f;
        Elevation = 0.0f;
        VegetationDensity = 0.5f;
        WaterAvailability = 0.5f;
        BiomeCenter = FVector::ZeroVector;
        BiomeRadius = 25000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEngArch_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Transition")
    EEngArch_BiomeType FromBiome = EEngArch_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Transition")
    EEngArch_BiomeType ToBiome = EEngArch_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Transition")
    float TransitionDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Transition")
    float BlendFactor = 0.5f;

    FEngArch_BiomeTransition()
    {
        FromBiome = EEngArch_BiomeType::None;
        ToBiome = EEngArch_BiomeType::None;
        TransitionDistance = 5000.0f;
        BlendFactor = 0.5f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEngArch_BiomeSystemCore : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEngArch_BiomeSystemCore();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomeSystem();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void RegisterBiome(const FEngArch_BiomeProperties& BiomeProperties);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void UnregisterBiome(EEngArch_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome System")
    EEngArch_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome System")
    FEngArch_BiomeProperties GetBiomeProperties(EEngArch_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome System")
    TArray<EEngArch_BiomeType> GetAllRegisteredBiomes() const;

    // Biome Transitions
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void AddBiomeTransition(const FEngArch_BiomeTransition& Transition);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome System")
    FEngArch_BiomeTransition GetBiomeTransition(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome System")
    bool IsInTransitionZone(const FVector& WorldLocation) const;

    // Environmental Queries
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome System")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome System")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome System")
    float GetVegetationDensityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome System")
    TArray<FString> GetDinosaurSpeciesAtLocation(const FVector& WorldLocation) const;

    // System Status
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome System")
    bool IsBiomeSystemInitialized() const { return bIsInitialized; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome System")
    int32 GetRegisteredBiomeCount() const;

private:
    UPROPERTY()
    TMap<EEngArch_BiomeType, FEngArch_BiomeProperties> RegisteredBiomes;

    UPROPERTY()
    TArray<FEngArch_BiomeTransition> BiomeTransitions;

    UPROPERTY()
    bool bIsInitialized = false;

    // Internal methods
    void SetupDefaultBiomes();
    void SetupDefaultTransitions();
    float CalculateDistanceToBiome(const FVector& WorldLocation, const FEngArch_BiomeProperties& BiomeProps) const;
    EEngArch_BiomeType FindClosestBiome(const FVector& WorldLocation) const;
};

#include "EngArch_BiomeSystemCore.generated.h"