#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "../../SharedTypes.h"
#include "Eng_SurvivalArchitecture.generated.h"

/**
 * Core survival mechanics architecture defining the foundational systems
 * for prehistoric survival gameplay. Establishes the technical framework
 * that all survival-related agents must follow.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float MaxHunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float MaxThirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float Temperature = 37.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float MaxFear = 100.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_EnvironmentalFactors
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float AmbientTemperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float Humidity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float WindSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bIsRaining = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float TimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float DangerLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TArray<FString> NearbyThreats;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SurvivalArchitectureRules
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rules")
    float HealthDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rules")
    float HungerDecayRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rules")
    float ThirstDecayRate = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rules")
    float StaminaRegenRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rules")
    float TemperatureChangeRate = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rules")
    float FearDecayRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rules")
    bool bEnableRealisticSurvival = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rules")
    bool bEnableEnvironmentalEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rules")
    bool bEnableThreatSystem = true;
};

/**
 * Engine Architect's survival mechanics architecture system.
 * Defines the core technical framework for all survival gameplay.
 * This class establishes the rules that all other agents must follow.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_SurvivalArchitecture : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_SurvivalArchitecture();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === SURVIVAL STATS MANAGEMENT ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Architecture")
    FEng_SurvivalStats CurrentStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Architecture")
    FEng_EnvironmentalFactors EnvironmentalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Architecture")
    FEng_SurvivalArchitectureRules ArchitectureRules;

    // === ARCHITECTURE VALIDATION ===
    UFUNCTION(BlueprintCallable, Category = "Survival Architecture")
    bool ValidateSurvivalArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Survival Architecture")
    void InitializeSurvivalSystems();

    UFUNCTION(BlueprintCallable, Category = "Survival Architecture")
    void UpdateSurvivalStats(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Survival Architecture")
    void ProcessEnvironmentalEffects(float DeltaTime);

    // === SURVIVAL MECHANICS CORE ===
    UFUNCTION(BlueprintCallable, Category = "Survival Mechanics")
    void ModifyHealth(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival Mechanics")
    void ModifyHunger(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival Mechanics")
    void ModifyThirst(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival Mechanics")
    void ModifyStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival Mechanics")
    void ModifyTemperature(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival Mechanics")
    void ModifyFear(float Amount);

    // === ENVIRONMENTAL INTERACTION ===
    UFUNCTION(BlueprintCallable, Category = "Environmental Systems")
    void UpdateEnvironmentalFactors(const FEng_EnvironmentalFactors& NewFactors);

    UFUNCTION(BlueprintCallable, Category = "Environmental Systems")
    float CalculateTemperatureEffect() const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Systems")
    float CalculateWeatherEffect() const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Systems")
    void AddThreat(const FString& ThreatName);

    UFUNCTION(BlueprintCallable, Category = "Environmental Systems")
    void RemoveThreat(const FString& ThreatName);

    // === ARCHITECTURE ENFORCEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Architecture Enforcement")
    bool EnforceArchitectureRules();

    UFUNCTION(BlueprintCallable, Category = "Architecture Enforcement")
    TArray<FString> GetArchitectureViolations() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture Enforcement")
    void LogArchitectureStatus() const;

private:
    // Internal architecture validation
    bool bIsInitialized = false;
    float LastUpdateTime = 0.0f;
    TArray<FString> ArchitectureViolations;

    void ValidateStatRanges();
    void ApplyEnvironmentalEffects(float DeltaTime);
    void ProcessSurvivalDecay(float DeltaTime);
    void UpdateThreatAssessment();
};