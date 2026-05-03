#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "../SharedTypes.h"
#include "NarrativeManager.generated.h"

// Narrative Event Types
UENUM(BlueprintType)
enum class ENarr_EventType : uint8
{
    DinosaurSighting    UMETA(DisplayName = "Dinosaur Sighting"),
    FossilDiscovery     UMETA(DisplayName = "Fossil Discovery"),
    ThreatDetection     UMETA(DisplayName = "Threat Detection"),
    BiomeTransition     UMETA(DisplayName = "Biome Transition"),
    SafetyWarning       UMETA(DisplayName = "Safety Warning"),
    ResearchUpdate      UMETA(DisplayName = "Research Update")
};

UENUM(BlueprintType)
enum class ENarr_NarratorType : uint8
{
    TacticalNarrator        UMETA(DisplayName = "Tactical Narrator"),
    FieldResearcher         UMETA(DisplayName = "Field Researcher"),
    SafetyGuide            UMETA(DisplayName = "Safety Guide"),
    PaleontologyNarrator   UMETA(DisplayName = "Paleontology Narrator")
};

// Narrative Event Structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_EventType EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NarratorType NarratorType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString EventText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float Cooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    EEng_BiomeType TriggerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    EEng_DinosaurSpecies TriggerDinosaur;

    FNarr_NarrativeEvent()
    {
        EventType = ENarr_EventType::DinosaurSighting;
        NarratorType = ENarr_NarratorType::TacticalNarrator;
        EventText = "Default narrative event";
        AudioAssetPath = "";
        Priority = 1.0f;
        Cooldown = 30.0f;
        TriggerBiome = EEng_BiomeType::Savanna;
        TriggerDinosaur = EEng_DinosaurSpecies::TRex;
    }
};

// Point of Interest Structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_PointOfInterest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
    EEng_BiomeType Biome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
    float InteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
    bool bIsDiscovered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
    TArray<FNarr_NarrativeEvent> AssociatedEvents;

    FNarr_PointOfInterest()
    {
        Name = "Unknown POI";
        Location = FVector::ZeroVector;
        Biome = EEng_BiomeType::Savanna;
        InteractionRadius = 1000.0f;
        bIsDiscovered = false;
    }
};

/**
 * Narrative Manager - Handles dynamic storytelling and contextual narration
 * Triggers appropriate voice lines based on player actions, discoveries, and environmental context
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeManager();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Narrative Functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeEvent(ENarr_EventType EventType, EEng_DinosaurSpecies DinosaurSpecies = EEng_DinosaurSpecies::TRex);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayNarration(const FNarr_NarrativeEvent& Event);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterPointOfInterest(const FNarr_PointOfInterest& POI);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CheckProximityToPoints(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnBiomeTransition(EEng_BiomeType FromBiome, EEng_BiomeType ToBiome);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnDinosaurEncounter(EEng_DinosaurSpecies Species, float Distance, bool bIsHostile);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnFossilDiscovery(EEng_DinosaurSpecies Species, const FVector& Location);

    // Configuration Functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void LoadNarrativeEvents();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetNarrationVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsNarrationPlaying() const;

protected:
    // Narrative Event Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Database")
    TArray<FNarr_NarrativeEvent> NarrativeEvents;

    // Points of Interest
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI System")
    TArray<FNarr_PointOfInterest> PointsOfInterest;

    // Audio System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* NarrationAudioComponent;

    // Cooldown Management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cooldown")
    TMap<ENarr_EventType, float> EventCooldowns;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float NarrationVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float ProximityCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableContextualNarration;

private:
    // Internal Functions
    void InitializeNarrativeDatabase();
    void InitializePointsOfInterest();
    bool CanTriggerEvent(ENarr_EventType EventType) const;
    void UpdateEventCooldown(ENarr_EventType EventType, float CooldownTime);
    FNarr_NarrativeEvent* FindEventByType(ENarr_EventType EventType, ENarr_NarratorType PreferredNarrator = ENarr_NarratorType::TacticalNarrator);

    // Timer Handles
    FTimerHandle ProximityCheckTimer;
};