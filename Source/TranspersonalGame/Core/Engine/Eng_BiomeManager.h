#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "../../SharedTypes.h"
#include "Eng_BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> AllowedVegetation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> AllowedDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxActors = 4000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 CurrentActorCount = 0;

    FEng_BiomeConfiguration()
    {
        AllowedVegetation.Add("Tree");
        AllowedVegetation.Add("Bush");
        AllowedDinosaurs.Add("TRex");
        AllowedDinosaurs.Add("Velociraptor");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_BiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_BiomeManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EBiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeConfiguration GetBiomeConfiguration(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool CanSpawnActorInBiome(EBiomeType BiomeType, const FString& ActorType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterActorInBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void UnregisterActorFromBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    TArray<FVector> GetSpawnLocationsInBiome(EBiomeType BiomeType, int32 Count) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetBiomeTemperature(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetBiomeHumidity(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void UpdateBiomeActorCounts();

    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsBiomeOvercrowded(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void CleanupOvercrowdedBiomes();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TMap<EBiomeType, FEng_BiomeConfiguration> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 GlobalMaxActors = 20000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 GlobalCurrentActors = 0;

private:
    void SetupDefaultBiomes();
    float CalculateDistanceFromBiomeCenter(const FVector& Location, const FEng_BiomeConfiguration& Biome) const;
    EBiomeType FindClosestBiome(const FVector& Location) const;
};