#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_BiomeArchitect.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<EDinosaurSpecies> NativeDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<EVegetationType> NativeVegetation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxActorsPerBiome = 4000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius = 5000.0f;

    FEng_BiomeDefinition()
    {
        BiomeType = EBiomeType::Forest;
        BiomeName = TEXT("Forest");
        Temperature = 22.0f;
        Humidity = 0.7f;
        Elevation = 100.0f;
        MaxActorsPerBiome = 4000;
        BiomeRadius = 5000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType FromBiome = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType ToBiome = EBiomeType::Desert;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionWidth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float BlendFactor = 0.5f;

    FEng_BiomeTransition()
    {
        FromBiome = EBiomeType::Forest;
        ToBiome = EBiomeType::Desert;
        TransitionWidth = 1000.0f;
        BlendFactor = 0.5f;
    }
};

/**
 * Engine Architect's Biome System - Core architecture for world biome management
 * Defines biome types, transitions, and architectural rules for world generation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_BiomeArchitect : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_BiomeArchitect();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Biome Definition Management
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void InitializeBiomeDefinitions();

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    FEng_BiomeDefinition GetBiomeDefinition(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void RegisterBiomeDefinition(const FEng_BiomeDefinition& BiomeDefinition);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    TArray<FEng_BiomeDefinition> GetAllBiomeDefinitions() const;

    // Biome Transition Management
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void RegisterBiomeTransition(const FEng_BiomeTransition& Transition);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    FEng_BiomeTransition GetBiomeTransition(EBiomeType FromBiome, EBiomeType ToBiome) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool HasBiomeTransition(EBiomeType FromBiome, EBiomeType ToBiome) const;

    // World Architecture Validation
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool ValidateBiomeLayout(const TArray<EBiomeType>& BiomeLayout) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    int32 GetMaxActorsForBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    float GetBiomeRadius(EBiomeType BiomeType) const;

    // Architecture Rules
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool CanDinosaurSpawnInBiome(EDinosaurSpecies Species, EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool CanVegetationSpawnInBiome(EVegetationType VegetationType, EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    TArray<EDinosaurSpecies> GetNativeDinosaursForBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    TArray<EVegetationType> GetNativeVegetationForBiome(EBiomeType BiomeType) const;

    // Performance Architecture
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool IsActorLimitReached(EBiomeType BiomeType, int32 CurrentActorCount) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    int32 GetRemainingActorBudget(EBiomeType BiomeType, int32 CurrentActorCount) const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome Data")
    TMap<EBiomeType, FEng_BiomeDefinition> BiomeDefinitions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome Data")
    TArray<FEng_BiomeTransition> BiomeTransitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rules")
    int32 GlobalMaxActors = 20000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rules")
    int32 MaxDinosaursTotal = 150;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rules")
    float DefaultBiomeRadius = 5000.0f;

private:
    void SetupDefaultBiomes();
    void SetupDefaultTransitions();
    bool ValidateBiomeDefinition(const FEng_BiomeDefinition& BiomeDefinition) const;
};