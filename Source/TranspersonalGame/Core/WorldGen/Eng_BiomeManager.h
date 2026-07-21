#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Eng_BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Savana UMETA(DisplayName = "Savana"),
    Forest UMETA(DisplayName = "Forest"),
    Swamp UMETA(DisplayName = "Swamp"),
    Mountain UMETA(DisplayName = "Mountain"),
    Desert UMETA(DisplayName = "Desert"),
    River UMETA(DisplayName = "River")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> AllowedVegetation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> AllowedDinosaurs;

    FEng_BiomeData()
    {
        BiomeType = EEng_BiomeType::Savana;
        Temperature = 25.0f;
        Humidity = 0.5f;
        Elevation = 100.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AEng_BiomeManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TMap<EEng_BiomeType, FEng_BiomeData> BiomeDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float BiomeTransitionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    int32 MaxBiomeZones;

public:
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EEng_BiomeType GetBiomeAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeData GetBiomeData(EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomeDatabase();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool CanSpawnDinosaurInBiome(const FString& DinosaurType, EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<FVector> GetValidSpawnLocationsInBiome(EEng_BiomeType BiomeType, int32 Count);
};