#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Eng_BiomeSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSubclassOf<class AActor>> VegetationTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSubclassOf<class APawn>> DinosaurTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurSpawnRate = 0.1f;

    FEng_BiomeData()
    {
        BiomeType = EBiomeType::Plains;
        Temperature = 25.0f;
        Humidity = 0.5f;
        Elevation = 0.0f;
        VegetationDensity = 1.0f;
        DinosaurSpawnRate = 0.1f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_BiomeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_BiomeManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeData GetBiomeData(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void SetBiomeData(EBiomeType BiomeType, const FEng_BiomeData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<EBiomeType> GetAllBiomeTypes();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetTemperatureAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetHumidityAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void GenerateBiomeMap(int32 WorldSize = 10000);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void UpdateBiomeTransitions();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TMap<EBiomeType, FEng_BiomeData> BiomeDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FVector2D> BiomeRegions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float BiomeTransitionDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    int32 BiomeMapResolution = 512;

private:
    void InitializeBiomeDatabase();
    EBiomeType CalculateBiomeFromEnvironment(float Temperature, float Humidity, float Elevation);
    float GetNoiseValue(const FVector2D& Location, float Scale = 0.001f);
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

    UFUNCTION(BlueprintCallable, Category = "Biome Component")
    void UpdateCurrentBiome();

    UFUNCTION(BlueprintCallable, Category = "Biome Component")
    EBiomeType GetCurrentBiome() const { return CurrentBiome; }

    UFUNCTION(BlueprintCallable, Category = "Biome Component")
    FEng_BiomeData GetCurrentBiomeData() const;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBiomeChanged, EBiomeType, OldBiome, EBiomeType, NewBiome);
    UPROPERTY(BlueprintAssignable, Category = "Biome Component")
    FOnBiomeChanged OnBiomeChanged;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome Component")
    EBiomeType CurrentBiome = EBiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Component")
    float UpdateInterval = 1.0f;

private:
    float TimeSinceLastUpdate = 0.0f;
    UEng_BiomeManager* BiomeManager = nullptr;
};