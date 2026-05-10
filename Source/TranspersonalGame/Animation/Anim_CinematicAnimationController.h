#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Anim_CinematicAnimationController.generated.h"

UENUM(BlueprintType)
enum class EAnim_CinematicType : uint8
{
    None UMETA(DisplayName = "None"),
    Discovery UMETA(DisplayName = "Discovery"),
    DinosaurEncounter UMETA(DisplayName = "Dinosaur Encounter"),
    SurvivalAction UMETA(DisplayName = "Survival Action"),
    Environmental UMETA(DisplayName = "Environmental"),
    Combat UMETA(DisplayName = "Combat"),
    Crafting UMETA(DisplayName = "Crafting"),
    Exploration UMETA(DisplayName = "Exploration")
};

UENUM(BlueprintType)
enum class EAnim_CinematicState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Preparing UMETA(DisplayName = "Preparing"),
    Playing UMETA(DisplayName = "Playing"),
    Blending UMETA(DisplayName = "Blending"),
    Completed UMETA(DisplayName = "Completed"),
    Interrupted UMETA(DisplayName = "Interrupted")
};

USTRUCT(BlueprintType)
struct FAnim_CinematicSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    FName SequenceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    EAnim_CinematicType CinematicType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    TSoftObjectPtr<UAnimMontage> AnimationMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    float BlendInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    float BlendOutTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    bool bLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    bool bInterruptible;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    int32 Priority;

    FAnim_CinematicSequence()
    {
        SequenceName = NAME_None;
        CinematicType = EAnim_CinematicType::None;
        Duration = 1.0f;
        BlendInTime = 0.25f;
        BlendOutTime = 0.25f;
        bLooping = false;
        bInterruptible = true;
        Priority = 0;
    }
};

USTRUCT(BlueprintType)
struct FAnim_CinematicCamera
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FVector CameraLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FRotator CameraRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float FieldOfView;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float TransitionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    bool bSmoothTransition;

    FAnim_CinematicCamera()
    {
        CameraLocation = FVector::ZeroVector;
        CameraRotation = FRotator::ZeroRotator;
        FieldOfView = 90.0f;
        TransitionTime = 1.0f;
        bSmoothTransition = true;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCinematicStateChanged, EAnim_CinematicState, OldState, EAnim_CinematicState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCinematicCompleted, FName, SequenceName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCinematicInterrupted, FName, SequenceName);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_CinematicAnimationController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_CinematicAnimationController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core Cinematic Functions
    UFUNCTION(BlueprintCallable, Category = "Cinematic Animation")
    bool PlayCinematicSequence(FName SequenceName, bool bForcePlay = false);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Animation")
    void StopCurrentCinematic(bool bBlendOut = true);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Animation")
    void PauseCinematic();

    UFUNCTION(BlueprintCallable, Category = "Cinematic Animation")
    void ResumeCinematic();

    UFUNCTION(BlueprintCallable, Category = "Cinematic Animation")
    bool RegisterCinematicSequence(const FAnim_CinematicSequence& NewSequence);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Animation")
    void UnregisterCinematicSequence(FName SequenceName);

    // Camera Control
    UFUNCTION(BlueprintCallable, Category = "Cinematic Camera")
    void SetCinematicCamera(const FAnim_CinematicCamera& CameraSettings);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Camera")
    void RestorePlayerCamera();

    // State Queries
    UFUNCTION(BlueprintPure, Category = "Cinematic Animation")
    EAnim_CinematicState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Cinematic Animation")
    FName GetCurrentSequenceName() const { return CurrentSequenceName; }

    UFUNCTION(BlueprintPure, Category = "Cinematic Animation")
    float GetCurrentPlayTime() const { return CurrentPlayTime; }

    UFUNCTION(BlueprintPure, Category = "Cinematic Animation")
    float GetCurrentSequenceDuration() const;

    UFUNCTION(BlueprintPure, Category = "Cinematic Animation")
    bool IsCinematicPlaying() const { return CurrentState == EAnim_CinematicState::Playing; }

    // Sequence Management
    UFUNCTION(BlueprintCallable, Category = "Cinematic Animation")
    TArray<FName> GetAvailableSequences() const;

    UFUNCTION(BlueprintCallable, Category = "Cinematic Animation")
    FAnim_CinematicSequence GetSequenceData(FName SequenceName) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Cinematic Events")
    FOnCinematicStateChanged OnCinematicStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Cinematic Events")
    FOnCinematicCompleted OnCinematicCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Cinematic Events")
    FOnCinematicInterrupted OnCinematicInterrupted;

protected:
    // Internal Functions
    void UpdateCinematicPlayback(float DeltaTime);
    void SetCinematicState(EAnim_CinematicState NewState);
    void OnCinematicMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    void BlendToCinematicCamera(const FAnim_CinematicCamera& CameraSettings);
    void ValidateSequenceData();

    // Component References
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    class APlayerController* PlayerController;

    UPROPERTY()
    class ACameraActor* CinematicCamera;

    // Cinematic Data
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic Sequences", meta = (AllowPrivateAccess = "true"))
    TMap<FName, FAnim_CinematicSequence> CinematicSequences;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cinematic State", meta = (AllowPrivateAccess = "true"))
    EAnim_CinematicState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cinematic State", meta = (AllowPrivateAccess = "true"))
    FName CurrentSequenceName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cinematic State", meta = (AllowPrivateAccess = "true"))
    float CurrentPlayTime;

    UPROPERTY()
    UAnimMontage* CurrentMontage;

    // Camera Data
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Settings", meta = (AllowPrivateAccess = "true"))
    FAnim_CinematicCamera DefaultCameraSettings;

    UPROPERTY()
    FAnim_CinematicCamera OriginalCameraSettings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Settings", meta = (AllowPrivateAccess = "true"))
    bool bUseCinematicCamera;

    // Timeline Component for smooth camera transitions
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UTimelineComponent* CameraTimeline;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Settings", meta = (AllowPrivateAccess = "true"))
    UCurveFloat* CameraTransitionCurve;

    // Timers
    FTimerHandle CinematicTimerHandle;
    FTimerHandle BlendOutTimerHandle;
};