#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "MetasoundSource.h"
#include "Engine/TriggerVolume.h"
#include "SharedTypes.h"
#include "Audio_MetaSoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_AmbienceType : uint8
{
    Forest = 0,
    Danger,
    Campfire,
    TribalDrums,
    Wind,
    Footsteps,
    Narrative
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_AmbienceZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_AmbienceType AmbienceType = EAudio_AmbienceType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    TSoftObjectPtr<USoundBase> AudioAsset;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_NarrativeTrigger
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    FString TriggerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    FVector TriggerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    TSoftObjectPtr<USoundBase> NarrativeAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    float TriggerRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    bool bHasTriggered = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    bool bCanRepeat = false;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio System")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zones")
    TArray<FAudio_AmbienceZone> AmbienceZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    TArray<FAudio_NarrativeTrigger> NarrativeTriggers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TArray<class UAudioComponent*> AudioComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AmbienceVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float NarrativeVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float FootstepVolume = 0.8f;

public:
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioZones();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdatePlayerProximity(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerNarrativeAudio(const FString& TriggerName);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetAmbienceVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetNarrativeVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayFootstepAudio(const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void CreateAmbienceZone(EAudio_AmbienceType Type, const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RemoveAmbienceZone(int32 ZoneIndex);

private:
    void UpdateAmbienceAudio(float DeltaTime);
    void CheckNarrativeTriggers(const FVector& PlayerLocation);
    UAudioComponent* CreateAudioComponent();
    float CalculateDistanceAttenuation(const FVector& SourceLocation, const FVector& ListenerLocation, float MaxDistance);
};