// BiomeSystem.h — Agent #5 Procedural World Generator
// Defines 6 prehistoric biome zones for the survival game world

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BiomeSystem.generated.h"

// ============================================================
// ENUMS — must be at global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    RockyHighlands  UMETA(DisplayName = "Rocky Highlands"),
    RiverDelta      UMETA(DisplayName = "River Delta"),
    VolcanicZone    UMETA(DisplayName = "Volcanic Zone"),
    Swampland       UMETA(DisplayName = "Swampland")
};

// ============================================================
// STRUCTS — must be at global scope (RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FoliageDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown Biome");
};

// ============================================================
// DELEGATE
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBiomeChanged, EWorld_BiomeType, NewBiome, FWorld_BiomeData, BiomeData);

// ============================================================
// ACTOR CLASS
// ============================================================

UCLASS(BlueprintType, Blueprintable, ClassGroup = "World")
class TRANSPERSONALGAME_API ABiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    ABiomeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ---- Core Methods ----

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome")
    float GetBlendWeight(const FVector& WorldLocation, EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void OnBiomeChanged(EWorld_BiomeType NewBiome);

    UFUNCTION(CallInEditor, Category = "Biome|Debug")
    void DebugDrawBiomeBoundaries();

    // ---- Properties ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FWorld_BiomeData> Biomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeBlendRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Debug")
    bool bDebugDrawBiomes;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EWorld_BiomeType CurrentPlayerBiome;

    // ---- Delegate ----

    UPROPERTY(BlueprintAssignable, Category = "Biome|Events")
    FOnBiomeChanged OnBiomeChangedDelegate;
};
