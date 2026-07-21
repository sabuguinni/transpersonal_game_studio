#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Audio_DynamicAudioTrigger.generated.h"

UENUM(BlueprintType)
enum class EAudio_TriggerType : uint8
{
    Ambient,
    Danger,
    Narrative,
    Combat,
    Discovery
};

UCLASS()
class TRANSPERSONALGAME_API AAudio_DynamicAudioTrigger : public ATriggerBox
{
    GENERATED_BODY()

public:
    AAudio_DynamicAudioTrigger();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    class UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    EAudio_TriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    USoundCue* AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    USoundCue* NarrativeSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    float FadeInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    float FadeOutTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    float VolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    bool bLoopAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    bool bPlayOnce;

private:
    bool bHasTriggered;
    void PlayAudioForTriggerType();
    void StopAudioWithFade();
};