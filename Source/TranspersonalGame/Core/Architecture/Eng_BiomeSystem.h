#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Eng_BiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Grassland       UMETA(DisplayName = "Grassland"),
    Forest          UMETA(DisplayName = "Forest"),
    Desert          UMETA(DisplayName = "Desert"),
    Swamp           UMETA(DisplayName = "Swamp"),
    Mountain        UMETA(DisplayName = "Mountain"),
    River           UMETA(DisplayName = "River"),
    Lake            UMETA(DisplayName = "Lake"),
    Volcanic        UMETA(DisplayName = "Volcanic")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Fertility = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> VegetationTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float SpawnDensity = 1.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_BiomeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_BiomeComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FEng_BiomeData BiomeData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius = 5000.0f;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetBiomeType() const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTemperature() const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetHumidity() const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsLocationInBiome(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void UpdateBiomeInfluence(float DeltaTime);
};

UCLASS()
class TRANSPERSONALGAME_API AEng_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AEng_BiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Manager")
    TArray<UEng_BiomeComponent*> ActiveBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Manager")
    float BiomeUpdateInterval = 5.0f;

    UFUNCTION(BlueprintCallable, Category = "Biome Manager")
    EEng_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Manager")
    FEng_BiomeData GetBiomeDataAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Manager")
    void RegisterBiome(UEng_BiomeComponent* BiomeComponent);

    UFUNCTION(BlueprintCallable, Category = "Biome Manager")
    void UnregisterBiome(UEng_BiomeComponent* BiomeComponent);

    UFUNCTION(BlueprintCallable, Category = "Biome Manager")
    TArray<UEng_BiomeComponent*> GetBiomesInRange(const FVector& Location, float Range) const;

private:
    float LastUpdateTime = 0.0f;

    void UpdateAllBiomes(float DeltaTime);
    UEng_BiomeComponent* FindClosestBiome(const FVector& Location) const;
};