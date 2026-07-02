// BiomeManager.h — Engine Architect #02 — PROD_CYCLE_AUTO_20260702_005
// P1 World Generation: Biome system — 5 biome types for Cretaceous survival world
// Follows SharedTypes.h conventions, uses Eng_ prefix for all custom types

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BiomeManager.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 compilation rule)
// ============================================================

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Forest    UMETA(DisplayName = "Cretaceous Forest"),
    Plains    UMETA(DisplayName = "Open Plains"),
    Swamp     UMETA(DisplayName = "Primordial Swamp"),
    Volcanic  UMETA(DisplayName = "Volcanic Badlands"),
    River     UMETA(DisplayName = "River Delta"),
    Unknown   UMETA(DisplayName = "Unknown")
};

// ============================================================
// STRUCTS — must be at global scope (UE5 compilation rule)
// ============================================================

USTRUCT(BlueprintType)
struct FEng_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float TemperatureMin = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float TemperatureMax = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float HumidityMin = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float HumidityMax = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Gameplay")
    float DangerLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Gameplay")
    float ResourceMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FLinearColor BiomeColor = FLinearColor::White;
};

// ============================================================
// DELEGATE — biome transition event
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEng_OnBiomeChanged,
    EEng_BiomeType, OldBiome,
    EEng_BiomeType, NewBiome);

// ============================================================
// UCLASS
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent),
    DisplayName = "Biome Manager")
class TRANSPERSONALGAME_API UBiomeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UBiomeManager();

    // --- Lifecycle ---
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    // --- Biome Queries ---
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetDangerLevelAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetResourceMultiplierAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FText GetBiomeDisplayName(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetCurrentBiome() const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void ForceSetBiome(EEng_BiomeType NewBiome);

    // --- Definition Access ---
    UFUNCTION(BlueprintCallable, Category = "Biome")
    const FEng_BiomeDefinition* GetBiomeDefinition(EEng_BiomeType BiomeType) const;

    // --- Events ---
    UPROPERTY(BlueprintAssignable, Category = "Biome|Events")
    FEng_OnBiomeChanged OnBiomeChanged;

    // --- Data ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    TArray<FEng_BiomeDefinition> BiomeDefinitions;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|State",
        meta = (AllowPrivateAccess = "true"))
    EEng_BiomeType CurrentBiome = EEng_BiomeType::Forest;

    void InitializeDefaultBiomes();
};
