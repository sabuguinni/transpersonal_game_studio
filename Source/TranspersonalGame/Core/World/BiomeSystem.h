// BiomeSystem.h
// Agent #05 — Procedural World Generator
// Biome classification and environmental audio/visual configuration

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    CretaceousJungle    UMETA(DisplayName = "Cretaceous Jungle"),
    RockyHighlands      UMETA(DisplayName = "Rocky Highlands"),
    RiverValley         UMETA(DisplayName = "River Valley"),
    OpenPlains          UMETA(DisplayName = "Open Plains"),
    SwampWetlands       UMETA(DisplayName = "Swamp Wetlands"),
    VolcanicBadlands    UMETA(DisplayName = "Volcanic Badlands"),
};

USTRUCT(BlueprintType)
struct FWorld_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float AmbientTemperature = 28.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString AmbientSoundCue = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasWater = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurSpawnMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeZone")
    FWorld_BiomeConfig Config;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeZone")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeZone")
    float Radius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeZone")
    FString ZoneName = TEXT("UnnamedZone");
};

UCLASS(ClassGroup = (World), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBiomeDetectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBiomeDetectorComponent();

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetCurrentBiome() const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeConfig GetCurrentBiomeConfig() const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetDistanceToNearestWater() const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsInDangerousZone() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    EWorld_BiomeType CurrentBiome = EWorld_BiomeType::OpenPlains;

    UPROPERTY()
    float BiomeTransitionAlpha = 1.0f;

    UPROPERTY()
    float LastBiomeCheckTime = 0.0f;

    static constexpr float BiomeCheckInterval = 2.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetBiomeAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeConfig GetBiomeConfigAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterBiomeZone(const FWorld_BiomeZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    TArray<FWorld_BiomeZone> GetAllBiomeZones() const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    int32 GetBiomeZoneCount() const;

    UFUNCTION(CallInEditor, Category = "Biome")
    void InitializeDefaultBiomes();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FWorld_BiomeZone> BiomeZones;

private:
    FWorld_BiomeConfig DefaultConfig;
};
