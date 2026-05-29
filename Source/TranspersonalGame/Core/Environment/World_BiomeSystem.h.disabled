#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "../SharedTypes.h"
#include "World_BiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"), 
    Rocky       UMETA(DisplayName = "Rocky"),
    Wetlands    UMETA(DisplayName = "Wetlands"),
    Desert      UMETA(DisplayName = "Desert"),
    Tundra      UMETA(DisplayName = "Tundra"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Coastal     UMETA(DisplayName = "Coastal")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor AmbientColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WindStrength = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AudioFadeDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<TSoftObjectPtr<UStaticMesh>> VegetationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float TerrainRoughness = 0.5f;

    FWorld_BiomeProperties()
    {
        Temperature = 20.0f;
        Humidity = 0.5f;
        WindStrength = 0.3f;
        AudioFadeDistance = 2000.0f;
        TerrainRoughness = 0.5f;
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
    float TransitionDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float BlendFactor = 0.5f;
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FWorld_BiomeProperties GetBiomeProperties(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FWorld_BiomeProperties GetBlendedBiomeProperties(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void RegisterBiomeZone(EWorld_BiomeType BiomeType, const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void UpdateEnvironmentalAudio(const FVector& ListenerLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<FVector> GetBiomeCenters(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Development")
    void GenerateDefaultBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Development")
    void ValidateBiomeSystem();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    TMap<EWorld_BiomeType, FWorld_BiomeProperties> BiomePropertiesMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    TArray<FWorld_BiomeTransition> BiomeTransitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    TMap<EWorld_BiomeType, TArray<FVector>> BiomeZoneCenters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    TMap<EWorld_BiomeType, float> BiomeZoneRadii;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<EWorld_BiomeType, class UAudioComponent*> ActiveAudioComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float BiomeUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MaxAudioDistance = 5000.0f;

private:
    float LastUpdateTime = 0.0f;
    EWorld_BiomeType LastPlayerBiome = EWorld_BiomeType::Forest;

    void InitializeDefaultBiomes();
    float CalculateDistanceToBiome(const FVector& Location, EWorld_BiomeType BiomeType) const;
    FWorld_BiomeProperties BlendBiomeProperties(const FWorld_BiomeProperties& BiomeA, const FWorld_BiomeProperties& BiomeB, float BlendFactor) const;
    void UpdateAudioForBiome(EWorld_BiomeType BiomeType, const FVector& ListenerLocation, float Volume);
};