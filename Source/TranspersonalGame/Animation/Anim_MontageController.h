#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimMontage.h"
#include "Engine/DataAsset.h"
#include "SharedTypes.h"
#include "Anim_MontageController.generated.h"

UENUM(BlueprintType)
enum class EAnim_MontageType : uint8
{
    Combat         UMETA(DisplayName = "Combat"),
    Interaction    UMETA(DisplayName = "Interaction"),
    Emote         UMETA(DisplayName = "Emote"),
    Death         UMETA(DisplayName = "Death"),
    Crafting      UMETA(DisplayName = "Crafting"),
    Gathering     UMETA(DisplayName = "Gathering")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MontageData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> Montage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    EAnim_MontageType MontageType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FName MontageName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float PlayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bCanBeInterrupted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    int32 Priority;

    FAnim_MontageData()
    {
        MontageType = EAnim_MontageType::Interaction;
        MontageName = NAME_None;
        PlayRate = 1.0f;
        bCanBeInterrupted = true;
        Priority = 0;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMontageComplete, FName, MontageName, bool, bInterrupted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMontageStarted, FName, MontageName);

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MontageController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MontageController();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TArray<FAnim_MontageData> MontageDatabase;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    FName CurrentMontage;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool bIsPlayingMontage;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    int32 CurrentMontagePriority;

public:
    UPROPERTY(BlueprintAssignable, Category = "Animation")
    FOnMontageComplete OnMontageComplete;

    UPROPERTY(BlueprintAssignable, Category = "Animation")
    FOnMontageStarted OnMontageStarted;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool PlayMontage(FName MontageName, float PlayRate = 1.0f, bool bForcePlay = false);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool PlayMontageByType(EAnim_MontageType MontageType, float PlayRate = 1.0f, bool bForcePlay = false);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopCurrentMontage(float BlendOutTime = 0.25f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopAllMontages(float BlendOutTime = 0.25f);

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsPlayingMontage() const { return bIsPlayingMontage; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    FName GetCurrentMontageName() const { return CurrentMontage; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetMontagePosition() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMontagePosition(float Position);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void AddMontageToDatabase(const FAnim_MontageData& MontageData);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void RemoveMontageFromDatabase(FName MontageName);

private:
    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

    FAnim_MontageData* FindMontageData(FName MontageName);
    FAnim_MontageData* FindMontageDataByType(EAnim_MontageType MontageType);
    bool CanPlayMontage(const FAnim_MontageData& MontageData, bool bForcePlay) const;

    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UAnimInstance* AnimInstance;
};