#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Audio_VoicelineIntegration.generated.h"

UENUM(BlueprintType)
enum class EAudio_VoicelineType : uint8
{
    AtmosphericNarrator    UMETA(DisplayName = "Atmospheric Narrator"),
    SurvivalExpert        UMETA(DisplayName = "Survival Expert"),
    PlayerThought         UMETA(DisplayName = "Player Thought"),
    TribalElder          UMETA(DisplayName = "Tribal Elder"),
    WildernessGuide      UMETA(DisplayName = "Wilderness Guide"),
    DangerWarning        UMETA(DisplayName = "Danger Warning")
};

UENUM(BlueprintType)
enum class EAudio_VoicelineTrigger : uint8
{
    OnPlayerSpawn         UMETA(DisplayName = "On Player Spawn"),
    OnDinosaurSighting    UMETA(DisplayName = "On Dinosaur Sighting"),
    OnLowHealth          UMETA(DisplayName = "On Low Health"),
    OnCrafting           UMETA(DisplayName = "On Crafting"),
    OnNightfall          UMETA(DisplayName = "On Nightfall"),
    OnDanger             UMETA(DisplayName = "On Danger"),
    OnDiscovery          UMETA(DisplayName = "On Discovery"),
    OnSurvival           UMETA(DisplayName = "On Survival")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_VoicelineData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    FString VoicelineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    EAudio_VoicelineType VoicelineType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    EAudio_VoicelineTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    float CooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voiceline")
    bool bIsContextual;

    FAudio_VoicelineData()
    {
        VoicelineText = TEXT("");
        AudioURL = TEXT("");
        VoicelineType = EAudio_VoicelineType::AtmosphericNarrator;
        TriggerType = EAudio_VoicelineTrigger::OnPlayerSpawn;
        Duration = 10.0f;
        CooldownTime = 30.0f;
        bIsContextual = true;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_VoicelineIntegration : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_VoicelineIntegration();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    TArray<FAudio_VoicelineData> VoicelineDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    class UAudioComponent* VoicelineAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    float MasterVoicelineVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    bool bVoicelinesEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    float MinTimeBetweenVoicelines;

    // Cooldown tracking
    UPROPERTY()
    TMap<EAudio_VoicelineTrigger, float> TriggerCooldowns;

    UPROPERTY()
    float LastVoicelineTime;

    UPROPERTY()
    FTimerHandle VoicelineTimerHandle;

public:
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeVoicelineSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerVoiceline(EAudio_VoicelineTrigger TriggerType, const FVector& Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayVoicelineByType(EAudio_VoicelineType VoicelineType, const FVector& Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopCurrentVoiceline();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    bool CanTriggerVoiceline(EAudio_VoicelineTrigger TriggerType) const;

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetVoicelineVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void EnableVoicelines(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    FAudio_VoicelineData GetRandomVoicelineByTrigger(EAudio_VoicelineTrigger TriggerType) const;

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void AddVoicelineToDatabase(const FAudio_VoicelineData& NewVoiceline);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void LoadVoicelinesFromNarrativeSystem();

protected:
    UFUNCTION()
    void OnVoicelineFinished();

    void UpdateCooldowns();
    void PlayVoicelineInternal(const FAudio_VoicelineData& VoicelineData, const FVector& Location);
    void LoadPresetVoicelines();
};