#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "World_BiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    Swampland   UMETA(DisplayName = "Swampland"),
    Canyon      UMETA(DisplayName = "Canyon"),
    Mountains   UMETA(DisplayName = "Mountains"),
    Desert      UMETA(DisplayName = "Desert"),
    Coastal     UMETA(DisplayName = "Coastal"),
    Volcanic    UMETA(DisplayName = "Volcanic")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Default Biome");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> VegetationTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> WildlifeTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float SpawnDensity = 1.0f;

    FWorld_BiomeData()
    {
        VegetationTypes.Add(TEXT("Generic_Tree"));
        VegetationTypes.Add(TEXT("Generic_Grass"));
        WildlifeTypes.Add(TEXT("Small_Herbivore"));
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EWorld_BiomeType FromBiome = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EWorld_BiomeType ToBiome = EWorld_BiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionWidth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float BlendStrength = 0.5f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core biome functions
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetBiomeInfluence(FVector WorldLocation, EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<EWorld_BiomeType> GetNearbyBiomes(FVector WorldLocation, float Radius) const;

    // Biome management
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void RegisterBiomeZone(FVector Center, float Radius, EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void UpdateBiomeTransitions();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void GenerateDefaultBiomes();

    // Environmental effects
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetTemperatureAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetHumidityAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FLinearColor GetBiomeColorAtLocation(FVector WorldLocation) const;

    // Debug functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugDrawBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ClearBiomeZones();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TMap<EWorld_BiomeType, FWorld_BiomeData> BiomeDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FWorld_BiomeTransition> BiomeTransitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float BiomeNoiseScale = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float BiomeBlendDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    int32 BiomeResolution = 128;

private:
    struct FBiomeZone
    {
        FVector Center;
        float Radius;
        EWorld_BiomeType BiomeType;
        float Strength;
    };

    TArray<FBiomeZone> BiomeZones;
    
    void InitializeBiomeDatabase();
    float CalculateNoiseValue(FVector WorldLocation) const;
    EWorld_BiomeType DetermineBiomeFromNoise(float NoiseValue, FVector WorldLocation) const;
    float CalculateDistanceInfluence(FVector Location, const FBiomeZone& Zone) const;
};