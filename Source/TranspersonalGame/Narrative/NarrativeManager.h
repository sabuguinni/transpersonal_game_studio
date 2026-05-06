#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "NarrativeManager.generated.h"

// Forward declarations
class UNarr_DialogueComponent;
class ANarr_NarrativeTrigger;

UENUM(BlueprintType)
enum class ENarr_NarrativeEvent : uint8
{
    Discovery           UMETA(DisplayName = "Discovery"),
    Danger             UMETA(DisplayName = "Danger"),
    Weather            UMETA(DisplayName = "Weather"),
    DinosaurEncounter  UMETA(DisplayName = "Dinosaur Encounter"),
    BiomeTransition    UMETA(DisplayName = "Biome Transition"),
    QuestUpdate        UMETA(DisplayName = "Quest Update")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NarrativeEvent EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsUrgent;

    FNarr_NarrativeData()
    {
        EventType = ENarr_NarrativeEvent::Discovery;
        NarrativeText = "";
        AudioPath = "";
        Duration = 5.0f;
        bIsUrgent = false;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNarrativeEvent, const FNarr_NarrativeData&, NarrativeData);

/**
 * Sistema de gestão narrativa para o jogo de sobrevivência pré-histórico
 * Gere diálogos, narrações e eventos narrativos baseados em contexto
 */
UCLASS()
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Narrative event system
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeEvent(ENarr_NarrativeEvent EventType, const FString& Context = "");

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayNarration(const FNarr_NarrativeData& NarrativeData);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterNarrativeTrigger(ANarr_NarrativeTrigger* Trigger);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UnregisterNarrativeTrigger(ANarr_NarrativeTrigger* Trigger);

    // Context-based narrative
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateNarrativeContext(EEng_BiomeType CurrentBiome, EEng_WeatherType Weather, EEng_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_NarrativeData GetContextualNarrative(ENarr_NarrativeEvent EventType);

    // Dinosaur encounter narratives
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDinosaurEncounter(EEng_DinosaurSpecies Species, float Distance, bool bIsHostile);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerWeatherWarning(EEng_WeatherType IncomingWeather, float TimeToArrival);

    // Event delegates
    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnNarrativeEvent OnNarrativeEvent;

protected:
    // Current narrative context
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    EEng_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    EEng_WeatherType CurrentWeather;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    EEng_ThreatLevel CurrentThreatLevel;

    // Registered narrative triggers
    UPROPERTY()
    TArray<ANarr_NarrativeTrigger*> NarrativeTriggers;

    // Narrative data library
    UPROPERTY(EditDefaultsOnly, Category = "Narrative")
    TMap<ENarr_NarrativeEvent, TArray<FNarr_NarrativeData>> NarrativeLibrary;

    // Audio URLs for generated voice samples
    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    FString PaleontologistFieldNotesURL;

    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    FString EmergencyAlertURL;

    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    FString DiscoveryNarrationURL;

    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    FString WeatherWarningURL;

private:
    void InitializeNarrativeLibrary();
    FNarr_NarrativeData CreateNarrativeData(ENarr_NarrativeEvent EventType, const FString& Text, const FString& AudioURL, float Duration, bool bUrgent = false);
    void LogNarrativeEvent(const FNarr_NarrativeData& Data);
};