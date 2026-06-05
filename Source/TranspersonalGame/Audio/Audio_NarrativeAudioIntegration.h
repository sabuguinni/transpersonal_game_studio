#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/TriggerBox.h"
#include "Audio_NarrativeAudioIntegration.generated.h"

UENUM(BlueprintType)
enum class EAudio_NarrativeAudioType : uint8
{
    SurvivalWarning,
    CampfireStory,
    HuntAftermath,
    AmbientWind,
    DinosaurProximity,
    WeatherChange
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_NarrativeAudioEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_NarrativeAudioType AudioType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bIs3DPositional;

    FAudio_NarrativeAudioEvent()
    {
        AudioType = EAudio_NarrativeAudioType::SurvivalWarning;
        TriggerLocation = FVector::ZeroVector;
        TriggerRadius = 500.0f;
        Volume = 1.0f;
        bIs3DPositional = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_NarrativeAudioIntegration : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_NarrativeAudioIntegration();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    TArray<FAudio_NarrativeAudioEvent> NarrativeAudioEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    UAudioComponent* CampfireAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    UAudioComponent* AmbientWindComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    float MaxAudibleDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    float VolumeMultiplier;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void TriggerNarrativeAudio(EAudio_NarrativeAudioType AudioType, FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void SetCampfireAudioActive(bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void UpdateAmbientWindIntensity(float WindStrength);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void PlaySurvivalVoiceLine(const FString& VoiceLineID);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void SetNarrativeAudioVolume(float NewVolume);

private:
    void InitializeAudioComponents();
    void UpdateAudioPositions();
    bool IsPlayerInRange(FVector AudioLocation, float Range);
    
    UPROPERTY()
    TMap<EAudio_NarrativeAudioType, UAudioComponent*> AudioComponentMap;

    float CurrentWindIntensity;
    bool bCampfireActive;
};