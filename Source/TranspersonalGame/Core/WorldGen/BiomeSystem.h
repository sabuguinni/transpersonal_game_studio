// BiomeSystem.h
// Transpersonal Game Studio — Agent #05 Procedural World Generator
// Biome detection, parameters, and environmental audio/visual configuration
// Cycle: PROD_CYCLE_AUTO_20260701_001

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/DataTable.h"
#include "BiomeSystem.generated.h"

// ── BIOME TYPE ENUM ──────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    JungleForest     UMETA(DisplayName = "Jungle Forest"),
    VolcanicPlains   UMETA(DisplayName = "Volcanic Plains"),
    RiverWetlands    UMETA(DisplayName = "River Wetlands"),
    RockyHighlands   UMETA(DisplayName = "Rocky Highlands"),
    OpenSavanna      UMETA(DisplayName = "Open Savanna"),
    Unknown          UMETA(DisplayName = "Unknown")
};

// ── BIOME PARAMETERS STRUCT ──────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float Temperature = 25.0f;          // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float Humidity = 0.6f;              // 0.0 - 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Vegetation")
    float VegetationDensity = 0.5f;     // 0.0 - 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Danger")
    float DangerLevel = 0.5f;           // 0.0 - 1.0 (affects predator spawn rate)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FLinearColor FogColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FLinearColor AmbientColor = FLinearColor(0.3f, 0.4f, 0.3f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Audio")
    TSoftObjectPtr<USoundBase> AmbientSoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Audio")
    float AmbientSoundVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float WaterAvailability = 0.5f;     // affects thirst drain rate

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float FoodAvailability = 0.5f;      // affects hunger drain rate

    FWorld_BiomeParameters() {}
};

// ── BIOME ZONE STRUCT ────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeZone")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::OpenSavanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeZone")
    FVector2D CenterXY = FVector2D(0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeZone")
    float RadiusUnits = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeZone")
    FWorld_BiomeParameters Parameters;

    FWorld_BiomeZone() {}
};

// ── BIOME DETECTOR COMPONENT ─────────────────────────────────────────────────
UCLASS(ClassGroup = (WorldGen), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBiomeDetectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBiomeDetectorComponent();

    // Current biome the owning actor is in
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome")
    EWorld_BiomeType CurrentBiome = EWorld_BiomeType::Unknown;

    // Current biome parameters
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome")
    FWorld_BiomeParameters CurrentBiomeParams;

    // How often to check biome (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DetectionInterval = 2.0f;

    // Detect biome at given world location
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType DetectBiomeAtLocation(FVector WorldLocation);

    // Get parameters for a biome type
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeParameters GetBiomeParameters(EWorld_BiomeType BiomeType) const;

    // Blueprint event when biome changes
    UFUNCTION(BlueprintImplementableEvent, Category = "Biome")
    void OnBiomeChanged(EWorld_BiomeType OldBiome, EWorld_BiomeType NewBiome);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    float TimeSinceLastDetection = 0.0f;
    EWorld_BiomeType LastDetectedBiome = EWorld_BiomeType::Unknown;

    // Registered biome zones
    TArray<FWorld_BiomeZone> BiomeZones;

    void InitializeDefaultBiomeZones();
    void InitializeBiomeParameters(TMap<EWorld_BiomeType, FWorld_BiomeParameters>& OutParams) const;
};

// ── BIOME MANAGER ACTOR ───────────────────────────────────────────────────────
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // All biome zones in the world
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeManager")
    TArray<FWorld_BiomeZone> BiomeZones;

    // Ambient audio component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BiomeManager")
    UAudioComponent* AmbientAudioComponent;

    // Get biome type at world location
    UFUNCTION(BlueprintCallable, Category = "BiomeManager")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    // Get biome parameters at world location
    UFUNCTION(BlueprintCallable, Category = "BiomeManager")
    FWorld_BiomeParameters GetBiomeParametersAtLocation(FVector WorldLocation) const;

    // Update ambient audio for biome
    UFUNCTION(BlueprintCallable, Category = "BiomeManager")
    void UpdateAmbientAudioForBiome(EWorld_BiomeType BiomeType);

    // Register a new biome zone at runtime
    UFUNCTION(BlueprintCallable, Category = "BiomeManager")
    void RegisterBiomeZone(FWorld_BiomeZone NewZone);

    // Get all zones of a given biome type
    UFUNCTION(BlueprintCallable, Category = "BiomeManager")
    TArray<FWorld_BiomeZone> GetZonesOfType(EWorld_BiomeType BiomeType) const;

    // Debug: draw biome zone boundaries in viewport
    UFUNCTION(CallInEditor, Category = "BiomeManager|Debug")
    void DrawBiomeZoneBoundaries();

protected:
    virtual void BeginPlay() override;

private:
    void SetupDefaultBiomeZones();
};
