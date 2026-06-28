// BiomeManager.h
// Engine Architect #02 — Transpersonal Game Studio
// Biome classification, transition, and environmental parameter system
// Priority: P1 — World Generation

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

// ============================================================
// Enums — must be at global scope (UE5 compilation rule)
// ============================================================

UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    Grassland   UMETA(DisplayName = "Cretaceous Grassland"),
    Forest      UMETA(DisplayName = "Cretaceous Forest"),
    Swamp       UMETA(DisplayName = "Cretaceous Swamp"),
    Desert      UMETA(DisplayName = "Cretaceous Desert"),
    Volcanic    UMETA(DisplayName = "Volcanic Badlands"),
    Riverbank   UMETA(DisplayName = "Cretaceous Riverbank"),
};

// ============================================================
// Structs — must be at global scope
// ============================================================

USTRUCT(BlueprintType)
struct FBiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FText DisplayName;

    // Environmental parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Environment", meta = (ClampMin = "-10.0", ClampMax = "60.0"))
    float BaseTemperature = 28.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Environment", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float BaseHumidity = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Environment", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float BaseWindSpeed = 15.0f;

    // Gameplay parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Gameplay", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FoliageDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Gameplay", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float DinosaurDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Gameplay", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float PlayerMovementSpeedModifier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Gameplay")
    bool bAllowsFireCrafting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Resources")
    TArray<FString> ResourceTypes;
};

// ============================================================
// Delegate declarations
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBiomeChanged, EBiomeType, NewBiome);

// ============================================================
// ABiomeManager
// ============================================================

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Biome Map ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|State", meta = (AllowPrivateAccess = "true"))
    TMap<EBiomeType, FBiomeData> BiomeDataMap;

    // ---- Current State ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|State")
    EBiomeType CurrentBiomeType;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|State")
    EBiomeType TargetBiomeType;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TransitionBlendAlpha;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|State")
    bool bBiomeTransitionActive;

    // ---- Environmental State ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|Environment")
    float CurrentTemperatureCelsius;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|Environment")
    float CurrentHumidityPercent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|Environment")
    float CurrentWindSpeedKmh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|Environment")
    float CurrentVisibilityMeters;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|Environment")
    bool bIsRaining;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RainIntensity;

public:
    // ---- Configuration ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float WorldSizeKm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    int32 BiomeSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float TemperatureHeatStressThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float TemperatureColdStressThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float HumidityDehydrationModifier;

    // ---- Delegates ----
    UPROPERTY(BlueprintAssignable, Category = "Biome|Events")
    FOnBiomeChanged OnBiomeChanged;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FBiomeData GetBiomeData(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetPlayerSurvivalModifier(const FVector& PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void TriggerBiomeTransition(EBiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    TArray<FString> GetAvailableResourcesAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool CanCraftFireAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintPure, Category = "Biome")
    EBiomeType GetCurrentBiomeType() const { return CurrentBiomeType; }

    UFUNCTION(BlueprintPure, Category = "Biome")
    float GetCurrentTemperature() const { return CurrentTemperatureCelsius; }

    UFUNCTION(BlueprintPure, Category = "Biome")
    float GetCurrentHumidity() const { return CurrentHumidityPercent; }

    UFUNCTION(BlueprintPure, Category = "Biome")
    bool GetIsRaining() const { return bIsRaining; }

private:
    void InitializeBiomeMap();
    void ApplyBiomeParameters(const FBiomeData& BiomeData);
    void UpdateEnvironmentalParameters(float DeltaTime);
    void OnBiomeTransitionComplete(EBiomeType NewBiome);
};
