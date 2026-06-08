#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Eng_GameplayArchitect.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_GameplayRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Rule")
    FString RuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Rule")
    FString RuleDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Rule")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Rule")
    int32 Priority;

    FEng_GameplayRule()
    {
        RuleName = TEXT("DefaultRule");
        RuleDescription = TEXT("Default gameplay rule");
        bIsActive = true;
        Priority = 1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_GameplayMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float PlayerSurvivalTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 DinosaurEncounters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CraftedItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float DistanceTraveled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 ResourcesGathered;

    FEng_GameplayMetrics()
    {
        PlayerSurvivalTime = 0.0f;
        DinosaurEncounters = 0;
        CraftedItems = 0;
        DistanceTraveled = 0.0f;
        ResourcesGathered = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_GameplayArchitect : public AActor
{
    GENERATED_BODY()

public:
    AEng_GameplayArchitect();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // GAMEPLAY ARCHITECTURE CORE
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Architecture")
    TArray<FEng_GameplayRule> GameplayRules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Architecture")
    FEng_GameplayMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Architecture")
    bool bEnableGameplayValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Architecture")
    float ValidationInterval;

    // SURVIVAL MECHANICS ARCHITECTURE
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Architecture")
    float BaseHealthDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Architecture")
    float BaseHungerDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Architecture")
    float BaseThirstDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Architecture")
    float BaseStaminaRegenRate;

    // DINOSAUR INTERACTION ARCHITECTURE
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Architecture")
    float DinosaurDetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Architecture")
    float DinosaurAggressionMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Architecture")
    bool bEnableDinosaurPackBehavior;

    // CRAFTING SYSTEM ARCHITECTURE
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting Architecture")
    int32 MaxCraftingSlots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting Architecture")
    float CraftingSpeedMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting Architecture")
    bool bEnableAdvancedCrafting;

    // GAMEPLAY VALIDATION FUNCTIONS
    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    bool ValidateGameplayRules();

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    void UpdateGameplayMetrics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    void RegisterGameplayRule(const FEng_GameplayRule& NewRule);

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    void RemoveGameplayRule(const FString& RuleName);

    // SURVIVAL MECHANICS FUNCTIONS
    UFUNCTION(BlueprintCallable, Category = "Survival Architecture")
    void InitializeSurvivalSystems();

    UFUNCTION(BlueprintCallable, Category = "Survival Architecture")
    void ValidateSurvivalBalance();

    UFUNCTION(BlueprintCallable, Category = "Survival Architecture")
    float CalculateOptimalDecayRate(const FString& StatType);

    // DINOSAUR SYSTEM FUNCTIONS
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Architecture")
    void InitializeDinosaurSystems();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Architecture")
    void ValidateDinosaurBehavior();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Architecture")
    int32 GetOptimalDinosaurCount();

    // CRAFTING SYSTEM FUNCTIONS
    UFUNCTION(BlueprintCallable, Category = "Crafting Architecture")
    void InitializeCraftingSystems();

    UFUNCTION(BlueprintCallable, Category = "Crafting Architecture")
    void ValidateCraftingBalance();

    UFUNCTION(BlueprintCallable, Category = "Crafting Architecture")
    bool CanCraftItem(const FString& ItemName);

    // ARCHITECTURE REPORTING
    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    FString GenerateGameplayReport();

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    void ExportArchitectureData();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Gameplay Architecture")
    void ValidateAllGameplaySystems();

private:
    float LastValidationTime;
    bool bSystemsInitialized;
    
    void InternalValidateGameplay();
    void InternalUpdateMetrics(float DeltaTime);
    void InternalInitializeSystems();
};