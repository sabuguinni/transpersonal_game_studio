#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_BiomeArchitecture.generated.h"

// Biome coordinate definitions (mandatory for all spawning)
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector MinBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector MaxBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity;

    FEng_BiomeZone()
    {
        BiomeName = TEXT("Unknown");
        CenterLocation = FVector::ZeroVector;
        MinBounds = FVector::ZeroVector;
        MaxBounds = FVector::ZeroVector;
        Temperature = 20.0f;
        Humidity = 0.5f;
    }
};

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Swamp       UMETA(DisplayName = "Pantano"),
    Forest      UMETA(DisplayName = "Floresta"),
    Savanna     UMETA(DisplayName = "Savana"),
    Desert      UMETA(DisplayName = "Deserto"),
    Mountain    UMETA(DisplayName = "Montanha Nevada")
};

/**
 * Engine Architecture - Biome System
 * Defines the 5 mandatory biome zones and enforces spawn coordinate rules
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_BiomeArchitecture : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_BiomeArchitecture();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // Core biome functions
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_BiomeZone GetBiomeZone(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    EEng_BiomeType GetBiomeTypeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FVector GetRandomLocationInBiome(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsLocationInBiome(const FVector& Location, EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_BiomeZone> GetAllBiomeZones() const;

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSpawnLocation(const FVector& Location, const FString& ActorType) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    void ValidateAllActorsInMap();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Configuration")
    TMap<EEng_BiomeType, FEng_BiomeZone> BiomeZones;

    void InitializeBiomeZones();
    void LogBiomeConfiguration() const;
};