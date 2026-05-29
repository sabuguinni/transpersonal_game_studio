#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "../SharedTypes.h"
#include "Anim_AnimationMontageManager.generated.h"

UENUM(BlueprintType)
enum class EAnim_MontageType : uint8
{
    None            UMETA(DisplayName = "None"),
    Combat          UMETA(DisplayName = "Combat"),
    Interaction     UMETA(DisplayName = "Interaction"),
    Emote           UMETA(DisplayName = "Emote"),
    Survival        UMETA(DisplayName = "Survival"),
    Death           UMETA(DisplayName = "Death"),
    Stunned         UMETA(DisplayName = "Stunned"),
    Climbing        UMETA(DisplayName = "Climbing"),
    Swimming        UMETA(DisplayName = "Swimming")
};

UENUM(BlueprintType)
enum class EAnim_MontagePriority : uint8
{
    Low             UMETA(DisplayName = "Low"),
    Normal          UMETA(DisplayName = "Normal"),
    High            UMETA(DisplayName = "High"),
    Critical        UMETA(DisplayName = "Critical"),
    Override        UMETA(DisplayName = "Override")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MontageData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    UAnimMontage* Montage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    EAnim_MontageType MontageType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    EAnim_MontagePriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    float PlayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    float BlendInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    float BlendOutTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    bool bCanBeInterrupted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    bool bLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    FString MontageID;

    FAnim_MontageData()
    {
        Montage = nullptr;
        MontageType = EAnim_MontageType::None;
        Priority = EAnim_MontagePriority::Normal;
        PlayRate = 1.0f;
        BlendInTime = 0.25f;
        BlendOutTime = 0.25f;
        bCanBeInterrupted = true;
        bLooping = false;
        MontageID = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MontageQueueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Queue")
    FAnim_MontageData MontageData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Queue")
    float QueueTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Queue")
    bool bAutoPlay;

    FAnim_MontageQueueEntry()
    {
        QueueTime = 0.0f;
        bAutoPlay = true;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAnim_OnMontageEvent, const FString&, MontageID, EAnim_MontageType, MontageType);

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_AnimationMontageManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_AnimationMontageManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core montage management
    UFUNCTION(BlueprintCallable, Category = "Animation|Montage")
    bool PlayMontage(const FAnim_MontageData& MontageData, bool bForcePlay = false);

    UFUNCTION(BlueprintCallable, Category = "Animation|Montage")
    bool PlayMontageByID(const FString& MontageID, bool bForcePlay = false);

    UFUNCTION(BlueprintCallable, Category = "Animation|Montage")
    void StopMontage(float BlendOutTime = 0.25f);

    UFUNCTION(BlueprintCallable, Category = "Animation|Montage")
    void StopMontageByType(EAnim_MontageType MontageType, float BlendOutTime = 0.25f);

    UFUNCTION(BlueprintCallable, Category = "Animation|Montage")
    void PauseMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation|Montage")
    void ResumeMontage();

    // Queue management
    UFUNCTION(BlueprintCallable, Category = "Animation|Queue")
    void QueueMontage(const FAnim_MontageData& MontageData);

    UFUNCTION(BlueprintCallable, Category = "Animation|Queue")
    void QueueMontageByID(const FString& MontageID);

    UFUNCTION(BlueprintCallable, Category = "Animation|Queue")
    void ClearMontageQueue();

    UFUNCTION(BlueprintCallable, Category = "Animation|Queue")
    void RemoveFromQueue(const FString& MontageID);

    // Montage registration
    UFUNCTION(BlueprintCallable, Category = "Animation|Registration")
    void RegisterMontage(const FAnim_MontageData& MontageData);

    UFUNCTION(BlueprintCallable, Category = "Animation|Registration")
    void UnregisterMontage(const FString& MontageID);

    UFUNCTION(BlueprintCallable, Category = "Animation|Registration")
    void ClearAllMontages();

    // Query functions
    UFUNCTION(BlueprintPure, Category = "Animation|Query")
    bool IsPlayingMontage() const;

    UFUNCTION(BlueprintPure, Category = "Animation|Query")
    bool IsPlayingMontageOfType(EAnim_MontageType MontageType) const;

    UFUNCTION(BlueprintPure, Category = "Animation|Query")
    FString GetCurrentMontageID() const;

    UFUNCTION(BlueprintPure, Category = "Animation|Query")
    EAnim_MontageType GetCurrentMontageType() const;

    UFUNCTION(BlueprintPure, Category = "Animation|Query")
    float GetMontagePosition() const;

    UFUNCTION(BlueprintPure, Category = "Animation|Query")
    float GetMontageLength() const;

    UFUNCTION(BlueprintPure, Category = "Animation|Query")
    bool CanPlayMontage(const FAnim_MontageData& MontageData) const;

    // Priority and interruption
    UFUNCTION(BlueprintCallable, Category = "Animation|Priority")
    bool CanInterruptCurrentMontage(EAnim_MontagePriority NewPriority) const;

    UFUNCTION(BlueprintCallable, Category = "Animation|Priority")
    void SetMontageInterruptible(bool bCanInterrupt);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Animation|Events")
    FAnim_OnMontageEvent OnMontageStarted;

    UPROPERTY(BlueprintAssignable, Category = "Animation|Events")
    FAnim_OnMontageEvent OnMontageEnded;

    UPROPERTY(BlueprintAssignable, Category = "Animation|Events")
    FAnim_OnMontageEvent OnMontageInterrupted;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montages", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FAnim_MontageData> RegisteredMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Queue", meta = (AllowPrivateAccess = "true"))
    TArray<FAnim_MontageQueueEntry> MontageQueue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current", meta = (AllowPrivateAccess = "true"))
    FAnim_MontageData CurrentMontageData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    float DefaultBlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    bool bAutoProcessQueue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    float QueueProcessInterval;

private:
    // Internal state
    bool bIsMontageActive;
    float LastQueueProcessTime;
    
    // References
    class UAnimInstance* CachedAnimInstance;
    class USkeletalMeshComponent* CachedMeshComponent;
    
    // Internal methods
    void CacheAnimationComponents();
    void ProcessMontageQueue();
    void PlayNextInQueue();
    bool InternalPlayMontage(const FAnim_MontageData& MontageData, bool bForcePlay);
    void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
    void OnMontageEnded_Internal(UAnimMontage* Montage, bool bInterrupted);
    void BroadcastMontageEvent(const FString& MontageID, EAnim_MontageType MontageType, FAnim_OnMontageEvent& Event);
};