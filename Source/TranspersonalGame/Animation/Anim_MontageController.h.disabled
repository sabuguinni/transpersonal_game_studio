#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Anim_MontageController.generated.h"

UENUM(BlueprintType)
enum class EAnim_MontageType : uint8
{
    Attack          UMETA(DisplayName = "Attack"),
    Dodge           UMETA(DisplayName = "Dodge"),
    Block           UMETA(DisplayName = "Block"),
    Interact        UMETA(DisplayName = "Interact"),
    Craft           UMETA(DisplayName = "Craft"),
    Gather          UMETA(DisplayName = "Gather"),
    Climb           UMETA(DisplayName = "Climb"),
    Swim            UMETA(DisplayName = "Swim"),
    Injured         UMETA(DisplayName = "Injured"),
    Death           UMETA(DisplayName = "Death"),
    Emote           UMETA(DisplayName = "Emote"),
    Special         UMETA(DisplayName = "Special")
};

UENUM(BlueprintType)
enum class EAnim_MontagePriority : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Normal      UMETA(DisplayName = "Normal"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MontageData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    class UAnimMontage* Montage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    EAnim_MontageType Type = EAnim_MontageType::Special;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    EAnim_MontagePriority Priority = EAnim_MontagePriority::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    float PlayRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    float BlendInTime = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    float BlendOutTime = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    bool bCanBeInterrupted = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    FString Description;

    FAnim_MontageData()
    {
        Montage = nullptr;
        Type = EAnim_MontageType::Special;
        Priority = EAnim_MontagePriority::Normal;
        PlayRate = 1.0f;
        BlendInTime = 0.25f;
        BlendOutTime = 0.25f;
        bCanBeInterrupted = true;
        Description = TEXT("Default Montage");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_PlayingMontage
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Playing Montage")
    class UAnimMontage* Montage = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Playing Montage")
    EAnim_MontageType Type = EAnim_MontageType::Special;

    UPROPERTY(BlueprintReadOnly, Category = "Playing Montage")
    EAnim_MontagePriority Priority = EAnim_MontagePriority::Normal;

    UPROPERTY(BlueprintReadOnly, Category = "Playing Montage")
    float StartTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Playing Montage")
    float Duration = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Playing Montage")
    bool bCanBeInterrupted = true;

    FAnim_PlayingMontage()
    {
        Montage = nullptr;
        Type = EAnim_MontageType::Special;
        Priority = EAnim_MontagePriority::Normal;
        StartTime = 0.0f;
        Duration = 0.0f;
        bCanBeInterrupted = true;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMontageStarted, UAnimMontage*, Montage, EAnim_MontageType, Type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMontageEnded, UAnimMontage*, Montage, bool, bInterrupted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMontageNotify, UAnimMontage*, Montage, FName, NotifyName, const FBranchingPointNotifyPayload&, Payload);

/**
 * Component for managing animation montages on prehistoric characters
 * Handles combat animations, survival actions, and contextual interactions
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MontageController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MontageController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Montage library
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage Library")
    TMap<EAnim_MontageType, FAnim_MontageData> MontageLibrary;

    // Currently playing montages
    UPROPERTY(BlueprintReadOnly, Category = "Playing Montages")
    TArray<FAnim_PlayingMontage> PlayingMontages;

    // Component references
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ACharacter* OwnerCharacter = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class UAnimInstance* AnimInstance = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class USkeletalMeshComponent* MeshComponent = nullptr;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxConcurrentMontages = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoStopLowPriorityMontages = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DefaultPlayRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DefaultBlendTime = 0.25f;

public:
    // Montage playback functions
    UFUNCTION(BlueprintCallable, Category = "Animation Montage")
    bool PlayMontage(EAnim_MontageType MontageType, float PlayRate = -1.0f, float BlendInTime = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation Montage")
    bool PlayMontageByReference(UAnimMontage* Montage, EAnim_MontagePriority Priority = EAnim_MontagePriority::Normal, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation Montage")
    void StopMontage(EAnim_MontageType MontageType, float BlendOutTime = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation Montage")
    void StopAllMontages(float BlendOutTime = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation Montage")
    void StopMontagesOfPriority(EAnim_MontagePriority Priority, float BlendOutTime = -1.0f);

    // Montage query functions
    UFUNCTION(BlueprintPure, Category = "Animation Montage")
    bool IsMontageTypePlaying(EAnim_MontageType MontageType) const;

    UFUNCTION(BlueprintPure, Category = "Animation Montage")
    bool IsAnyMontagePlaying() const;

    UFUNCTION(BlueprintPure, Category = "Animation Montage")
    int32 GetPlayingMontageCount() const;

    UFUNCTION(BlueprintPure, Category = "Animation Montage")
    UAnimMontage* GetCurrentMontageOfType(EAnim_MontageType MontageType) const;

    UFUNCTION(BlueprintPure, Category = "Animation Montage")
    TArray<UAnimMontage*> GetAllPlayingMontages() const;

    // Montage library management
    UFUNCTION(BlueprintCallable, Category = "Animation Montage")
    void RegisterMontage(EAnim_MontageType MontageType, UAnimMontage* Montage, EAnim_MontagePriority Priority = EAnim_MontagePriority::Normal);

    UFUNCTION(BlueprintCallable, Category = "Animation Montage")
    void UnregisterMontage(EAnim_MontageType MontageType);

    UFUNCTION(BlueprintCallable, Category = "Animation Montage")
    void LoadDefaultMontages();

    // Survival-specific montage functions
    UFUNCTION(BlueprintCallable, Category = "Survival Animations")
    void PlayAttackMontage(int32 AttackVariant = 0);

    UFUNCTION(BlueprintCallable, Category = "Survival Animations")
    void PlayDodgeMontage(FVector DodgeDirection);

    UFUNCTION(BlueprintCallable, Category = "Survival Animations")
    void PlayInteractionMontage(const FString& InteractionType);

    UFUNCTION(BlueprintCallable, Category = "Survival Animations")
    void PlayCraftingMontage(const FString& CraftingType);

    UFUNCTION(BlueprintCallable, Category = "Survival Animations")
    void PlayGatheringMontage(const FString& ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Survival Animations")
    void PlayInjuryMontage(float InjurySeverity);

    UFUNCTION(BlueprintCallable, Category = "Survival Animations")
    void PlayDeathMontage();

    // Event delegates
    UPROPERTY(BlueprintAssignable, Category = "Animation Events")
    FOnMontageStarted OnMontageStarted;

    UPROPERTY(BlueprintAssignable, Category = "Animation Events")
    FOnMontageEnded OnMontageEnded;

    UPROPERTY(BlueprintAssignable, Category = "Animation Events")
    FOnMontageNotify OnMontageNotify;

protected:
    // Internal functions
    void UpdatePlayingMontages();
    void CleanupFinishedMontages();
    bool CanPlayMontage(const FAnim_MontageData& MontageData) const;
    void HandleMontageInterruption(EAnim_MontagePriority NewPriority);
    
    // Montage event handlers
    UFUNCTION()
    void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

    UFUNCTION()
    void OnMontageNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

private:
    // Internal state
    float LastUpdateTime = 0.0f;
    bool bIsInitialized = false;
    
    // Helper functions
    FAnim_MontageData* FindMontageData(EAnim_MontageType MontageType);
    const FAnim_MontageData* FindMontageData(EAnim_MontageType MontageType) const;
    void RemovePlayingMontage(UAnimMontage* Montage);
    void AddPlayingMontage(const FAnim_MontageData& MontageData);
};