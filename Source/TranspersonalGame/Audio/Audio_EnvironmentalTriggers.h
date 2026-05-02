#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Audio_EnvironmentalTriggers.generated.h"

// Forward declarations
class USoundBase;
class UAudioComponent;

UENUM(BlueprintType)
enum class EAudio_EnvironmentalTriggerType : uint8
{
    Discovery       UMETA(DisplayName = "Discovery"),
    Danger          UMETA(DisplayName = "Danger"),
    SafeZone        UMETA(DisplayName = "Safe Zone"),
    Water           UMETA(DisplayName = "Water Source"),
    AncientBones    UMETA(DisplayName = "Ancient Bones"),
    RaptorTerritory UMETA(DisplayName = "Raptor Territory"),
    TRexDomain      UMETA(DisplayName = "T-Rex Domain"),
    MigrationPath   UMETA(DisplayName = "Migration Path")
};

USTRUCT(BlueprintType)
struct FAudio_EnvironmentalTriggerData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_EnvironmentalTriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> NarrativeVoiceline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AudioIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FearImpact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLoopAmbientSound;

    FAudio_EnvironmentalTriggerData()
    {
        TriggerType = EAudio_EnvironmentalTriggerType::Discovery;
        AudioIntensity = 1.0f;
        FearImpact = 0.0f;
        bLoopAmbientSound = true;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_EnvironmentalTriggerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_EnvironmentalTriggerComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    FAudio_EnvironmentalTriggerData TriggerData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    bool bTriggerOnce;

    UPROPERTY(BlueprintReadOnly, Category = "Environmental Audio")
    bool bHasTriggered;

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void TriggerEnvironmentalAudio(AActor* TriggeringActor);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void StopEnvironmentalAudio();

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetTriggerData(const FAudio_EnvironmentalTriggerData& NewTriggerData);

protected:
    UPROPERTY()
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY()
    UAudioComponent* NarrativeAudioComponent;

    void PlayAmbientSound();
    void PlayNarrativeVoiceline();
    void UpdatePlayerFearLevel(float FearDelta);
};

UCLASS(Blueprintable)
class TRANSPERSONALGAME_API AAudio_EnvironmentalTriggerActor : public ATriggerBox
{
    GENERATED_BODY()

public:
    AAudio_EnvironmentalTriggerActor();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

    UFUNCTION()
    void OnActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudio_EnvironmentalTriggerComponent* EnvironmentalTriggerComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    bool bActivateOnEntry;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    bool bDeactivateOnExit;
};