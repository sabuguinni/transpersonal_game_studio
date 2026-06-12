#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Anim_MontageController.generated.h"

UENUM(BlueprintType)
enum class EAnim_MontageCategory : uint8
{
    Combat          UMETA(DisplayName = "Combat"),
    Interaction     UMETA(DisplayName = "Interaction"),
    Emotion         UMETA(DisplayName = "Emotion"),
    Death           UMETA(DisplayName = "Death"),
    Special         UMETA(DisplayName = "Special")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MontageData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    UAnimMontage* Montage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    float PlayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    float BlendInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    float BlendOutTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    bool bLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    int32 Priority;

    FAnim_MontageData()
    {
        Montage = nullptr;
        PlayRate = 1.0f;
        BlendInTime = 0.25f;
        BlendOutTime = 0.25f;
        bLooping = false;
        Priority = 0;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMontageCompleted, EAnim_MontageCategory, Category, FName, MontageName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMontageBlendedOut, EAnim_MontageCategory, Category, FName, MontageName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMontageNotify, EAnim_MontageCategory, Category, FName, MontageName, FName, NotifyName);

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MontageController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MontageController();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montages")
    TMap<EAnim_MontageCategory, TMap<FName, FAnim_MontageData>> MontageLibrary;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    TMap<EAnim_MontageCategory, FName> CurrentlyPlayingMontages;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    TArray<EAnim_MontageCategory> MontageQueue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAllowMontageInterruption;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float GlobalPlayRateMultiplier;

public:
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnMontageCompleted OnMontageCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnMontageBlendedOut OnMontageBlendedOut;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnMontageNotify OnMontageNotify;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void RegisterMontage(EAnim_MontageCategory Category, FName MontageName, const FAnim_MontageData& MontageData);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool PlayMontage(EAnim_MontageCategory Category, FName MontageName, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopMontage(EAnim_MontageCategory Category, float BlendOutTime = 0.25f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopAllMontages(float BlendOutTime = 0.25f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void QueueMontage(EAnim_MontageCategory Category, FName MontageName);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayQueuedMontages();

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsMontagePlayingInCategory(EAnim_MontageCategory Category) const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    FName GetCurrentMontageInCategory(EAnim_MontageCategory Category) const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetMontagePosition(EAnim_MontageCategory Category) const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMontagePosition(EAnim_MontageCategory Category, float Position);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMontagePlayRate(EAnim_MontageCategory Category, float PlayRate);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool PlayCombatMontage(FName MontageName, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool PlayInteractionMontage(FName MontageName, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool PlayDeathMontage(FName MontageName, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void JumpToMontageSection(EAnim_MontageCategory Category, FName SectionName);

private:
    UAnimInstance* GetAnimInstance() const;
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    void ProcessMontageQueue();
    bool CanPlayMontage(EAnim_MontageCategory Category, int32 Priority) const;
};