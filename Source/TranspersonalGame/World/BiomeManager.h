// BiomeManager.h
// Transpersonal Game Studio — Core Systems Programmer (#03)
// Prehistoric world biome management — UWorldSubsystem
// 6 biomes: Jungle, Plains, Swamp, Volcanic, Coastal, Mountain

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "BiomeManager.generated.h"

// ─── Enums (global scope, unique prefixed names) ─────────────────────────────

UENUM(BlueprintType)
enum class ECore_BiomeType : uint8
{
    Jungle      UMETA(DisplayName = "Jungle"),
    Plains      UMETA(DisplayName = "Plains"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Coastal     UMETA(DisplayName = "Coastal"),
    Mountain    UMETA(DisplayName = "Mountain"),
    COUNT       UMETA(Hidden)
};

// ─── Structs (global scope) ───────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FCore_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    ECore_BiomeType BiomeType = ECore_BiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector WorldCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;      // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;          // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> NativeDinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown Biome");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasVolcanicActivity = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasWaterBodies = false;
};

USTRUCT(BlueprintType)
struct FCore_BiomeBlendResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    ECore_BiomeType DominantBiome = ECore_BiomeType::Plains;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float BlendedTemperature = 25.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float BlendedHumidity = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    FLinearColor BlendedFogColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float BlendedFogDensity = 0.02f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float DominantBiomeWeight = 1.0f;
};

// ─── BiomeManager Class ───────────────────────────────────────────────────────

UCLASS(BlueprintType, ClassGroup = "World")
class TRANSPERSONALGAME_API UBiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UBiomeManager();

    // ── UWorldSubsystem overrides ──────────────────────────────────────────
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ── Registered Biome Zones ─────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Zones")
    TArray<FCore_BiomeZone> BiomeZones;

    // ── Query Methods ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    ECore_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    FCore_BiomeBlendResult GetBlendedBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    FCore_BiomeZone GetBiomeZoneData(ECore_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    TArray<FString> GetDinosaursForBiome(ECore_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    // ── Registration ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Biome|Registration")
    void RegisterBiomeZone(const FCore_BiomeZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Biome|Registration")
    void ClearAllBiomeZones();

    // ── Static accessor ────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Biome", meta = (WorldContext = "WorldContextObject"))
    static UBiomeManager* GetBiomeManager(UObject* WorldContextObject);

private:
    void InitializeDefaultBiomes();
    float CalculateBiomeWeight(const FCore_BiomeZone& Zone, const FVector& Location) const;
};
