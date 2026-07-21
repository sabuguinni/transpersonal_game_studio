#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/TriggerBox.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Narr_BiomeNarrativeSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_BiomeType : uint8
{
    Swamp       UMETA(DisplayName = "Swamp"),
    Forest      UMETA(DisplayName = "Forest"),
    Savana      UMETA(DisplayName = "Savana"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain")
};

USTRUCT(BlueprintType)
struct FNarr_BiomeNarrativeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_BiomeType BiomeType = ENarr_BiomeType::Swamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerCooldown = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsEmergencyAlert = false;

    FNarr_BiomeNarrativeData()
    {
        NarrativeText = TEXT("Default narrative text");
        AudioURL = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_BiomeNarrativeSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_BiomeNarrativeSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerBiomeNarrative(ENarr_BiomeType BiomeType, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterBiomeTrigger(ATriggerBox* TriggerBox, ENarr_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayEmergencyAlert(const FString& AlertMessage, const FVector& DangerLocation);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsBiomeNarrativeOnCooldown(ENarr_BiomeType BiomeType) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<ENarr_BiomeType, FNarr_BiomeNarrativeData> BiomeNarratives;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TMap<ENarr_BiomeType, float> LastTriggerTimes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    UAudioComponent* NarrativeAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* EmergencyAlertSound;

private:
    void InitializeBiomeNarratives();
    void PlayNarrativeAudio(const FString& AudioURL);
    float GetCurrentTime() const;
};