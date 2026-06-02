#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MetasoundSource.h"
#include "MetasoundParameterTransmitter.h"
#include "Components/AudioComponent.h"
#include "Audio_MetaSoundController.generated.h"

UENUM(BlueprintType)
enum class EAudio_MetaSoundTrigger : uint8
{
    None = 0,
    DinosaurApproach,
    DangerLevel,
    HeartbeatIntensity,
    EnvironmentTransition,
    CombatStart,
    CombatEnd,
    Crafting,
    Discovery,
    WeatherChange,
    DayNightTransition
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_MetaSoundParameter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ParameterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FloatValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 IntValue = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool BoolValue = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudio_MetaSoundTrigger TriggerType = EAudio_MetaSoundTrigger::None;

    FAudio_MetaSoundParameter()
    {
        ParameterName = NAME_None;
        FloatValue = 0.0f;
        IntValue = 0;
        BoolValue = false;
        TriggerType = EAudio_MetaSoundTrigger::None;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_MetaSoundController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_MetaSoundController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void PlayMetaSound(UMetaSoundSource* MetaSound, FVector Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void StopMetaSound();

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetFloatParameter(FName ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetIntParameter(FName ParameterName, int32 Value);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetBoolParameter(FName ParameterName, bool Value);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void TriggerEvent(EAudio_MetaSoundTrigger TriggerType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void UpdateDangerLevel(float DangerLevel);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void UpdateHeartbeatIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void UpdateEnvironmentAmbience(const FString& BiomeType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void RegisterMetaSoundAsset(EAudio_MetaSoundTrigger TriggerType, UMetaSoundSource* MetaSound);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    bool IsMetaSoundPlaying() const;

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetMasterVolume(float Volume);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* AudioComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MetaSound")
    TMap<EAudio_MetaSoundTrigger, UMetaSoundSource*> MetaSoundAssets;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MetaSound")
    UMetaSoundSource* CurrentMetaSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MetaSound")
    TArray<FAudio_MetaSoundParameter> ActiveParameters;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MetaSound")
    float MasterVolume = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MetaSound")
    float CurrentDangerLevel = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MetaSound")
    float CurrentHeartbeatIntensity = 0.0f;

    void InitializeAudioComponent();
    void ApplyParametersToMetaSound();
    void UpdateParameterValue(FName ParameterName, float Value);
    UMetaSoundSource* GetMetaSoundForTrigger(EAudio_MetaSoundTrigger TriggerType) const;
};