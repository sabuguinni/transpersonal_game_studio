#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Narr_CinematicDirector.generated.h"

UENUM(BlueprintType)
enum class ENarr_CinematicType : uint8
{
    GameIntro       UMETA(DisplayName = "Game Introduction"),
    ThreatWarning   UMETA(DisplayName = "Threat Warning"),
    Discovery       UMETA(DisplayName = "Discovery Scene"),
    Survival        UMETA(DisplayName = "Survival Moment"),
    Death           UMETA(DisplayName = "Death Scene")
};

USTRUCT(BlueprintType)
struct FNarr_CinematicSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    FString SequenceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    FVector CameraStartLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    FVector CameraEndLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    FRotator CameraStartRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    FRotator CameraEndRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    FString VoicelineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    FString VoicelineAudioURL;

    FNarr_CinematicSequence()
    {
        SequenceName = TEXT("Default");
        Duration = 5.0f;
        CameraStartLocation = FVector::ZeroVector;
        CameraEndLocation = FVector::ZeroVector;
        CameraStartRotation = FRotator::ZeroRotator;
        CameraEndRotation = FRotator::ZeroRotator;
        VoicelineText = TEXT("");
        VoicelineAudioURL = TEXT("");
    }
};

/**
 * Cinematic Director - Manages narrative cinematics and intro sequences
 * Handles camera movement, voicelines, and atmospheric storytelling
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_CinematicDirector : public AActor
{
    GENERATED_BODY()

public:
    ANarr_CinematicDirector();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCameraComponent* CinematicCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* VoicelineAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Settings")
    ENarr_CinematicType CurrentCinematicType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Settings")
    TArray<FNarr_CinematicSequence> CinematicSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Settings")
    bool bAutoPlayOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Settings")
    float FadeInDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Settings")
    float FadeOutDuration;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    bool bIsPlayingCinematic;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    int32 CurrentSequenceIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    float CurrentSequenceTime;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Cinematic")
    void PlayCinematic(ENarr_CinematicType CinematicType);

    UFUNCTION(BlueprintCallable, Category = "Cinematic")
    void StopCinematic();

    UFUNCTION(BlueprintCallable, Category = "Cinematic")
    void SkipToNextSequence();

    UFUNCTION(BlueprintCallable, Category = "Cinematic")
    void SetupGameIntroSequence();

    UFUNCTION(BlueprintCallable, Category = "Cinematic")
    void SetupThreatWarningSequence();

    UFUNCTION(BlueprintCallable, Category = "Cinematic")
    void SetupDiscoverySequence();

    UFUNCTION(BlueprintCallable, Category = "Cinematic")
    bool IsPlayingCinematic() const { return bIsPlayingCinematic; }

    UFUNCTION(BlueprintCallable, Category = "Cinematic")
    float GetCurrentSequenceProgress() const;

protected:
    void UpdateCinematicSequence(float DeltaTime);
    void StartNextSequence();
    void EndCinematic();
    FVector LerpCameraPosition(const FNarr_CinematicSequence& Sequence, float Alpha) const;
    FRotator LerpCameraRotation(const FNarr_CinematicSequence& Sequence, float Alpha) const;
    void PlayVoiceline(const FString& VoicelineURL);
};