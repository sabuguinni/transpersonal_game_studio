#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "BiomeSystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    float Radius = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    float DangerLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    TArray<FString> AllowedDinosaurTypes;

    FEng_BiomeConfiguration()
    {
        AllowedDinosaurTypes.Add(TEXT("TRex"));
        AllowedDinosaurTypes.Add(TEXT("Velociraptor"));
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBiomeSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBiomeSystemManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EBiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeConfiguration GetBiomeConfiguration(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool IsLocationInBiome(const FVector& Location, EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<FVector> GetSpawnPointsInBiome(EBiomeType BiomeType, int32 Count = 10) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetBiomeTemperature(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetBiomeHumidity(const FVector& Location) const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Biome System")
    TMap<EBiomeType, FEng_BiomeConfiguration> BiomeConfigurations;

    void SetupDefaultBiomes();
    FVector GetRandomPointInBiome(const FEng_BiomeConfiguration& BiomeConfig) const;
};

#include "BiomeSystemManager.generated.h"