#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Narr_DinosaurEncounterNarrative.generated.h"

// Dinosaur encounter types for narrative context
UENUM(BlueprintType)
enum class ENarr_DinosaurEncounterType : uint8
{
    FirstSighting,
    PredatorHunt,
    HerbivoreGrazing,
    PackBehavior,
    TerritorialDispute,
    MatingRitual,
    Feeding,
    Migration,
    Nesting,
    DeathScene
};

// Narrative tension levels during dinosaur encounters
UENUM(BlueprintType)
enum class ENarr_EncounterTension : uint8
{
    Peaceful,
    Cautious,
    Tense,
    Dangerous,
    Terrifying,
    LifeOrDeath
};

// Dinosaur behavior context for storytelling
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DinosaurBehaviorContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    FString DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    ENarr_DinosaurEncounterType EncounterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    ENarr_EncounterTension TensionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    float DistanceToPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    bool bIsAggressive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    bool bIsInPack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    int32 PackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    FString BiomeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    FString TimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    FString WeatherConditions;

    FNarr_DinosaurBehaviorContext()
    {
        DinosaurSpecies = TEXT("Unknown");
        EncounterType = ENarr_DinosaurEncounterType::FirstSighting;
        TensionLevel = ENarr_EncounterTension::Cautious;
        DistanceToPlayer = 100.0f;
        bIsAggressive = false;
        bIsInPack = false;
        PackSize = 1;
        BiomeLocation = TEXT("Forest");
        TimeOfDay = TEXT("Day");
        WeatherConditions = TEXT("Clear");
    }
};

// Encounter narrative data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_EncounterNarrative
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Narrative")
    FString NarrativeTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Narrative")
    FString OpeningDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Narrative")
    FString TensionBuildup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Narrative")
    FString ClimaxMoment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Narrative")
    FString Resolution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Narrative")
    TArray<FString> PlayerThoughts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Narrative")
    TArray<FString> EnvironmentalDetails;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Narrative")
    float NarrativeDuration;

    FNarr_EncounterNarrative()
    {
        NarrativeTitle = TEXT("Unknown Encounter");
        OpeningDescription = TEXT("");
        TensionBuildup = TEXT("");
        ClimaxMoment = TEXT("");
        Resolution = TEXT("");
        NarrativeDuration = 30.0f;
    }
};

/**
 * Dinosaur Encounter Narrative System
 * Generates dynamic narrative content for dinosaur encounters based on behavior context
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DinosaurEncounterNarrative : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DinosaurEncounterNarrative();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Generate narrative for dinosaur encounter
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Narrative")
    FNarr_EncounterNarrative GenerateEncounterNarrative(const FNarr_DinosaurBehaviorContext& BehaviorContext);

    // Trigger encounter narrative sequence
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Narrative")
    void TriggerEncounterNarrative(const FNarr_DinosaurBehaviorContext& BehaviorContext);

    // Update narrative based on changing encounter dynamics
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Narrative")
    void UpdateEncounterNarrative(const FNarr_DinosaurBehaviorContext& UpdatedContext);

    // Get appropriate tension level based on encounter context
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Narrative")
    ENarr_EncounterTension CalculateTensionLevel(const FNarr_DinosaurBehaviorContext& BehaviorContext);

    // Generate species-specific narrative elements
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Narrative")
    TArray<FString> GenerateSpeciesNarrative(const FString& DinosaurSpecies, ENarr_DinosaurEncounterType EncounterType);

    // Generate environmental narrative context
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Narrative")
    TArray<FString> GenerateEnvironmentalNarrative(const FString& BiomeLocation, const FString& WeatherConditions);

    // Generate player psychological response
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Narrative")
    TArray<FString> GeneratePlayerPsychologicalResponse(ENarr_EncounterTension TensionLevel, bool bIsFirstEncounter);

protected:
    // Current encounter narrative
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current Narrative")
    FNarr_EncounterNarrative CurrentEncounter;

    // Active behavior context
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current Narrative")
    FNarr_DinosaurBehaviorContext ActiveContext;

    // Narrative templates for different species
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Templates")
    TMap<FString, TArray<FString>> SpeciesNarrativeTemplates;

    // Environmental narrative templates
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Templates")
    TMap<FString, TArray<FString>> BiomeNarrativeTemplates;

    // Tension-based narrative modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Templates")
    TMap<ENarr_EncounterTension, TArray<FString>> TensionNarrativeModifiers;

    // Track encounter history for narrative continuity
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Encounter History")
    TArray<FNarr_EncounterNarrative> EncounterHistory;

    // Maximum history entries to maintain
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxHistoryEntries;

    // Narrative update frequency
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float NarrativeUpdateInterval;

    // Timer for narrative updates
    UPROPERTY()
    float NarrativeUpdateTimer;

private:
    // Initialize narrative templates
    void InitializeNarrativeTemplates();

    // Build species-specific templates
    void BuildSpeciesTemplates();

    // Build biome-specific templates
    void BuildBiomeTemplates();

    // Build tension-based modifiers
    void BuildTensionModifiers();

    // Select appropriate narrative template
    FString SelectNarrativeTemplate(const TArray<FString>& Templates) const;

    // Apply narrative modifiers based on context
    FString ApplyNarrativeModifiers(const FString& BaseNarrative, const FNarr_DinosaurBehaviorContext& Context) const;

    // Calculate narrative complexity based on encounter factors
    float CalculateNarrativeComplexity(const FNarr_DinosaurBehaviorContext& BehaviorContext) const;
};