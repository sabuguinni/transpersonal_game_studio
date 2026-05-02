#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Audio_AdaptiveMusicManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_MusicState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Tension     UMETA(DisplayName = "Tension"),
    Danger      UMETA(DisplayName = "Danger"),
    Combat      UMETA(DisplayName = "Combat")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_AdaptiveMusicManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AdaptiveMusicManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Music components for different states
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* CalmMusicComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* TensionMusicComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* DangerMusicComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* CombatMusicComponent;

    // Current music state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music State")
    EAudio_MusicState CurrentMusicState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music State")
    EAudio_MusicState TargetMusicState;

    // Transition settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float TransitionDuration;

    UPROPERTY(BlueprintReadOnly, Category = "Transition")
    float TransitionTimer;

    UPROPERTY(BlueprintReadOnly, Category = "Transition")
    bool bIsTransitioning;

    // Volume settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CalmVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TensionVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DangerVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CombatVolume;

public:
    // Music state control functions
    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void SetMusicState(EAudio_MusicState NewState);

    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void TransitionToMusicState(EAudio_MusicState NewState, float Duration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void StopAllMusic();

    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void SetMasterVolume(float Volume);

    // Utility functions
    UFUNCTION(BlueprintPure, Category = "Music State")
    UAudioComponent* GetCurrentMusicComponent() const;

    UFUNCTION(BlueprintPure, Category = "Music State")
    UAudioComponent* GetMusicComponentForState(EAudio_MusicState State) const;

    UFUNCTION(BlueprintPure, Category = "Music State")
    bool IsTransitioning() const { return bIsTransitioning; }

private:
    void UpdateMusicTransition(float DeltaTime);
    void StartMusicComponent(UAudioComponent* Component);
    void StopMusicComponent(UAudioComponent* Component);
    void SetComponentVolume(UAudioComponent* Component, float Volume);
};