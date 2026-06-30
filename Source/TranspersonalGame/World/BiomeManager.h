// BiomeManager.h
// Engine Architect #02 — Transpersonal Game Studio
// Classifies world positions into biomes; drives foliage density, weather, ambient audio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BiomeManager.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// EEng_BiomeType — all biome types in the Cretaceous world
// RULE: declared at global scope, Eng_ prefix, unique name
// ─────────────────────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Jungle      UMETA(DisplayName = "Jungle"),
    Savanna     UMETA(DisplayName = "Savanna"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Riverbank   UMETA(DisplayName = "Riverbank"),
    OpenPlains  UMETA(DisplayName = "Open Plains"),
    Forest      UMETA(DisplayName = "Forest"),
    Desert      UMETA(DisplayName = "Desert"),
};

// ─────────────────────────────────────────────────────────────────────────────
// FEng_BiomeZone — one biome region in world space
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FEng_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Centre = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Jungle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 30.f;   // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;      // 0..1
};

// ─────────────────────────────────────────────────────────────────────────────
// FEng_BiomeBlendData — blended data returned for a query position
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FEng_BiomeBlendData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EEng_BiomeType DominantBiome = EEng_BiomeType::Jungle;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    TArray<EEng_BiomeType> NearbyBiomes;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float BlendedTemperature = 30.f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float BlendedHumidity = 0.5f;
};

// ─────────────────────────────────────────────────────────────────────────────
// Delegate — broadcast when player crosses a biome boundary
// ─────────────────────────────────────────────────────────────────────────────
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEng_OnBiomeChanged, EEng_BiomeType, NewBiome);

// ─────────────────────────────────────────────────────────────────────────────
// ABiomeManager — placed once in the level; manages all biome zones
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|World")
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // ── Lifecycle ──────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Query API (callable from Blueprint + other C++ systems) ────────────
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeBlendData GetBlendedBiomeData(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetCurrentPlayerBiome() const { return CurrentBiome; }

    // ── Configuration ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    int32 WorldSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float BiomeBlendRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Debug")
    bool bDebugDrawBiomes;

    // ── Runtime state ──────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|State")
    EEng_BiomeType CurrentBiome;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|State")
    TArray<FEng_BiomeZone> BiomeZones;

    // ── Delegate ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Biome|Events")
    FEng_OnBiomeChanged OnBiomeChangedDelegate;

private:
    void InitializeBiomeMap();
    void OnBiomeChanged(EEng_BiomeType NewBiome);
    void DrawBiomeDebug();
    float GetDefaultTemperature(EEng_BiomeType Biome) const;
    float GetDefaultHumidity(EEng_BiomeType Biome) const;
};
