#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

/**
 * FBiomeZone — defines a single biome region in the world.
 * Placed at global scope per RULE 1 (USTRUCT at global scope only).
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TemperatureBase = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityBase = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DangerLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<EDinosaurSpecies> InhabitingSpecies;

    FBiomeZone()
        : BiomeType(EBiomeType::Forest)
        , CenterLocation(FVector::ZeroVector)
        , Radius(3000.0f)
        , TemperatureBase(25.0f)
        , HumidityBase(0.5f)
        , DangerLevel(0.5f)
    {}
};

/**
 * FBiomeTransition — blending data between two adjacent biomes.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeA = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeB = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BlendAlpha = 0.0f;

    FBiomeTransition()
        : BiomeA(EBiomeType::Forest)
        , BiomeB(EBiomeType::Savanna)
        , BlendAlpha(0.0f)
    {}
};

/**
 * ABiomeManager — World Actor that manages all biome zones.
 * P1 Priority system. Placed once in the level; queried by all other systems.
 *
 * Usage:
 *   ABiomeManager* BM = ABiomeManager::GetInstance(GetWorld());
 *   EBiomeType Biome = BM->GetBiomeAtLocation(PlayerLocation);
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|World")
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // --- Static accessor ---
    UFUNCTION(BlueprintCallable, Category = "Biome", meta = (WorldContext = "WorldContextObject"))
    static ABiomeManager* GetInstance(UObject* WorldContextObject);

    // --- Biome query ---
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FBiomeZone GetBiomeZoneAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FBiomeTransition GetTransitionAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetDangerLevelAtLocation(const FVector& WorldLocation) const;

    // --- Zone management ---
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterBiomeZone(const FBiomeZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void ClearAllZones();

    UFUNCTION(BlueprintCallable, Category = "Biome")
    int32 GetZoneCount() const;

    // --- Default world setup ---
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome")
    void InitializeDefaultBiomes();

    // --- Debug ---
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Debug")
    void DrawBiomeDebugSpheres();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY(EditAnywhere, Category = "Biome", meta = (AllowPrivateAccess = "true"))
    TArray<FBiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, Category = "Biome", meta = (AllowPrivateAccess = "true"))
    float TransitionBlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Biome", meta = (AllowPrivateAccess = "true"))
    bool bDrawDebugVisuals = false;

    // Internal helpers
    const FBiomeZone* FindNearestZone(const FVector& Location) const;
    float GetBlendWeight(const FVector& Location, const FBiomeZone& Zone) const;
};
