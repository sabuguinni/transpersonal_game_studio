#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "BiomeSystem.generated.h"

// ============================================================
// BIOME SYSTEM — Agent #5 Procedural World Generator
// 5 prehistoric biomes: Jungle, Volcanic, Wetlands, Rocky, Savanna
// ============================================================

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    None            UMETA(DisplayName = "None"),
    JungleForest    UMETA(DisplayName = "Jungle Forest"),
    VolcanicPlains  UMETA(DisplayName = "Volcanic Plains"),
    RiverWetlands   UMETA(DisplayName = "River Wetlands"),
    RockyHighlands  UMETA(DisplayName = "Rocky Highlands"),
    OpenSavanna     UMETA(DisplayName = "Open Savanna")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeParameters : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float Temperature = 25.0f;  // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float Humidity = 0.5f;  // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Vegetation")
    float VegetationDensity = 0.5f;  // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Danger")
    float DangerLevel = 0.5f;  // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FLinearColor FogColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FLinearColor AmbientColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float PlayerStaminaDrainMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float PlayerHungerDrainMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float PlayerThirstDrainMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float DinosaurSpawnWeight = 1.0f;
};

USTRUCT(BlueprintType)
struct FWorld_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EWorld_BiomeType FromBiome = EWorld_BiomeType::None;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EWorld_BiomeType ToBiome = EWorld_BiomeType::None;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float BlendAlpha = 0.0f;  // 0 = fully FromBiome, 1 = fully ToBiome
};

// ============================================================
// UBiomeDetectorComponent — attaches to player/NPC to track current biome
// ============================================================
UCLASS(ClassGroup = "WorldGen", meta = (BlueprintSpawnableComponent), DisplayName = "Biome Detector")
class TRANSPERSONALGAME_API UBiomeDetectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBiomeDetectorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Current biome the owning actor is in
    UPROPERTY(BlueprintReadOnly, Category = "Biome", meta = (AllowPrivateAccess = "true"))
    EWorld_BiomeType CurrentBiome = EWorld_BiomeType::None;

    // Previous biome (for transition detection)
    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EWorld_BiomeType PreviousBiome = EWorld_BiomeType::None;

    // Active transition data
    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    FWorld_BiomeTransition ActiveTransition;

    // How often to check biome (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Settings")
    float BiomeCheckInterval = 1.0f;

    // Radius for biome overlap check
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Settings")
    float DetectionRadius = 100.0f;

    // Get current biome parameters
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeParameters GetCurrentBiomeParameters() const;

    // Get biome parameters for a specific type
    UFUNCTION(BlueprintCallable, Category = "Biome")
    static FWorld_BiomeParameters GetBiomeParametersForType(EWorld_BiomeType BiomeType);

    // Manually set biome (for testing/scripting)
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void SetCurrentBiome(EWorld_BiomeType NewBiome);

    // Delegate for biome change events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBiomeChanged, EWorld_BiomeType, OldBiome, EWorld_BiomeType, NewBiome);

    UPROPERTY(BlueprintAssignable, Category = "Biome|Events")
    FOnBiomeChanged OnBiomeChanged;

private:
    float TimeSinceLastCheck = 0.0f;

    void DetectCurrentBiome();
    EWorld_BiomeType SampleBiomeAtLocation(const FVector& Location) const;
};

// ============================================================
// ABiomeWorldActor — placed in level to define biome regions
// ============================================================
UCLASS(BlueprintType, Blueprintable, DisplayName = "Biome Region Actor")
class TRANSPERSONALGAME_API ABiomeWorldActor : public AActor
{
    GENERATED_BODY()

public:
    ABiomeWorldActor();

    // Which biome this actor represents
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::JungleForest;

    // Radius of this biome region (meters)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius = 3000.0f;

    // Transition blend distance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TransitionBlendDistance = 500.0f;

    // Biome parameters for this region
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FWorld_BiomeParameters BiomeParameters;

    // Is this biome currently active (has player inside)
    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    bool bIsPlayerInside = false;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsLocationInBiome(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetBlendAlphaAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Debug")
    void ApplyBiomeDefaults();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<class USphereComponent> BiomeBoundsComponent;
};
