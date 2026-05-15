#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "Sound/SoundCue.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Audio_MetaSoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_NarrativeType : uint8
{
    None = 0,
    AncientWisdom,
    DangerWarning,
    HuntInstruction,
    SurvivalTip,
    QuestNarration,
    EnvironmentalAlert
};

UENUM(BlueprintType)
enum class EAudio_SpatialMode : uint8
{
    TwoD = 0,
    ThreeD_Positional,
    ThreeD_Ambient,
    ThreeD_Directional
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    EAudio_NarrativeType NarrativeType = EAudio_NarrativeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    TSoftObjectPtr<USoundWave> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    float Duration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    int32 Priority = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    bool bIs3D = true;

    FAudio_VoiceLine()
    {
        NarrativeType = EAudio_NarrativeType::None;
        AudioURL = "";
        Duration = 0.0f;
        Priority = 1;
        bIs3D = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_MetaSoundParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound")
    float LowPassFilter = 22000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound")
    float HighPassFilter = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound")
    float ReverbAmount = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound")
    bool bUseDistanceAttenuation = true;

    FAudio_MetaSoundParams()
    {
        Volume = 1.0f;
        Pitch = 1.0f;
        LowPassFilter = 22000.0f;
        HighPassFilter = 20.0f;
        ReverbAmount = 0.0f;
        bUseDistanceAttenuation = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_MetaSoundManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_MetaSoundManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* PrimaryAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* SecondaryAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Lines")
    TArray<FAudio_VoiceLine> NarrativeVoiceLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound")
    TSoftObjectPtr<UMetaSoundSource> NarrativeMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound")
    TSoftObjectPtr<UMetaSoundSource> AmbientMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FAudio_MetaSoundParams DefaultParams;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsPlayingNarrative = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAudio_NarrativeType CurrentNarrativeType = EAudio_NarrativeType::None;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentPlaybackTime = 0.0f;

public:
    // Narrative voice line playback
    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool PlayNarrativeLine(EAudio_NarrativeType NarrativeType, const FVector& WorldLocation = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopNarrativeLine();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsNarrativePlaying() const { return bIsPlayingNarrative; }

    // MetaSound parameter control
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMetaSoundParameter(const FString& ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetAudioParams(const FAudio_MetaSoundParams& Params);

    // Voice line management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterVoiceLine(const FAudio_VoiceLine& VoiceLine);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    FAudio_VoiceLine GetVoiceLineByType(EAudio_NarrativeType NarrativeType) const;

    // 3D spatial audio
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetSpatialMode(EAudio_SpatialMode SpatialMode);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateListenerPosition(const FVector& ListenerLocation, const FVector& ListenerForward);

private:
    void InitializeDefaultVoiceLines();
    void LoadMetaSoundAssets();
    UAudioComponent* GetAvailableAudioComponent();
    void OnNarrativeFinished();

    UPROPERTY()
    TArray<UAudioComponent*> AudioComponentPool;

    float NarrativeStartTime = 0.0f;
    float CurrentNarrativeDuration = 0.0f;
};