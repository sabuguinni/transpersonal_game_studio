#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "SharedTypes.h"
#include "Anim_MontageController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MontageData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* Montage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float PlayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float BlendInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float BlendOutTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FName StartSection;

    FAnim_MontageData()
    {
        Montage = nullptr;
        PlayRate = 1.0f;
        BlendInTime = 0.25f;
        BlendOutTime = 0.25f;
        bLooping = false;
        StartSection = NAME_None;
    }
};

UENUM(BlueprintType)
enum class EAnim_MontageType : uint8
{
    None = 0,
    Jump,
    Attack,
    Interact,
    Craft,
    Gather,
    Eat,
    Drink,
    Sleep,
    Fear,
    Pain,
    Death,
    Emote
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MontageController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MontageController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Montage Library
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Montages")
    TMap<EAnim_MontageType, FAnim_MontageData> MontageLibrary;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    EAnim_MontageType CurrentMontageType;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    UAnimMontage* CurrentMontage;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    bool bIsMontageActive;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    float MontagePosition;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    float MontageLength;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Settings")
    bool bAutoStopOnMovement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Settings")
    float MovementThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Settings")
    bool bQueueMontages;

    // Montage Control
    UFUNCTION(BlueprintCallable, Category = "Animation|Control")
    bool PlayMontage(EAnim_MontageType MontageType, float CustomPlayRate = -1.0f, FName StartSection = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Animation|Control")
    bool PlayMontageByAsset(UAnimMontage* Montage, float PlayRate = 1.0f, float BlendInTime = 0.25f, FName StartSection = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Animation|Control")
    void StopMontage(float BlendOutTime = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation|Control")
    void PauseMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation|Control")
    void ResumeMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation|Control")
    void SetMontagePosition(float Position);

    UFUNCTION(BlueprintCallable, Category = "Animation|Control")
    void JumpToSection(FName SectionName);

    // Montage Queries
    UFUNCTION(BlueprintPure, Category = "Animation|Query")
    bool IsMontageActive() const { return bIsMontageActive; }

    UFUNCTION(BlueprintPure, Category = "Animation|Query")
    float GetMontagePosition() const { return MontagePosition; }

    UFUNCTION(BlueprintPure, Category = "Animation|Query")
    float GetMontageLength() const { return MontageLength; }

    UFUNCTION(BlueprintPure, Category = "Animation|Query")
    float GetMontagePlayRate() const;

    UFUNCTION(BlueprintPure, Category = "Animation|Query")
    FName GetCurrentSection() const;

    UFUNCTION(BlueprintPure, Category = "Animation|Query")
    TArray<FName> GetMontageSections() const;

    // Survival Specific Montages
    UFUNCTION(BlueprintCallable, Category = "Animation|Survival")
    void PlayJumpMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation|Survival")
    void PlayInteractMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation|Survival")
    void PlayCraftMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation|Survival")
    void PlayGatherMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation|Survival")
    void PlayEatMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation|Survival")
    void PlayDrinkMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation|Survival")
    void PlayFearMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation|Survival")
    void PlayPainMontage();

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMontageStarted, EAnim_MontageType, MontageType, UAnimMontage*, Montage);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMontageEnded, EAnim_MontageType, MontageType, bool, bCompleted);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMontageNotify, EAnim_MontageType, MontageType, FName, NotifyName, float, Position);

    UPROPERTY(BlueprintAssignable, Category = "Animation|Events")
    FOnMontageStarted OnMontageStarted;

    UPROPERTY(BlueprintAssignable, Category = "Animation|Events")
    FOnMontageEnded OnMontageEnded;

    UPROPERTY(BlueprintAssignable, Category = "Animation|Events")
    FOnMontageNotify OnMontageNotify;

private:
    // Internal State
    TWeakObjectPtr<ACharacter> OwnerCharacter;
    TWeakObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;
    TWeakObjectPtr<UAnimInstance> AnimInstance;

    // Montage Queue
    TArray<EAnim_MontageType> MontageQueue;

    // Internal Methods
    void UpdateMontageState();
    void ProcessMontageQueue();
    void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
    void OnMontageEnded_Internal(UAnimMontage* Montage, bool bInterrupted);
    void CheckMovementInterrupt();

    // Cached Values
    FVector LastLocation;
    float LastMovementTime;
};