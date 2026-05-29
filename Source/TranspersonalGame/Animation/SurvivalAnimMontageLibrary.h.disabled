#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SurvivalAnimMontageLibrary.generated.h"

UENUM(BlueprintType)
enum class EAnim_SurvivalMontageType : uint8
{
    None            UMETA(DisplayName = "None"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Eating          UMETA(DisplayName = "Eating"),
    Hiding          UMETA(DisplayName = "Hiding"),
    Climbing        UMETA(DisplayName = "Climbing"),
    Swimming        UMETA(DisplayName = "Swimming"),
    Injured         UMETA(DisplayName = "Injured"),
    Exhausted       UMETA(DisplayName = "Exhausted"),
    Fearful         UMETA(DisplayName = "Fearful")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MontageConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> MontageSoft;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float PlayRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float BlendInTime = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float BlendOutTime = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bLooping = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bInterruptible = true;

    FAnim_MontageConfig()
    {
        MontageSoft = nullptr;
        PlayRate = 1.0f;
        BlendInTime = 0.25f;
        BlendOutTime = 0.25f;
        bLooping = false;
        bInterruptible = true;
    }
};

/**
 * Animation montage library for survival actions
 * Handles playing contextual animations for survival mechanics
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API USurvivalAnimMontageLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // Play survival action montage
    UFUNCTION(BlueprintCallable, Category = "Animation|Survival")
    static bool PlaySurvivalMontage(UAnimInstance* AnimInstance, EAnim_SurvivalMontageType MontageType, float PlayRate = 1.0f);

    // Stop current survival montage
    UFUNCTION(BlueprintCallable, Category = "Animation|Survival")
    static void StopSurvivalMontage(UAnimInstance* AnimInstance, float BlendOutTime = 0.25f);

    // Check if survival montage is playing
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Animation|Survival")
    static bool IsSurvivalMontageActive(UAnimInstance* AnimInstance);

    // Get current survival montage type
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Animation|Survival")
    static EAnim_SurvivalMontageType GetCurrentSurvivalMontageType(UAnimInstance* AnimInstance);

    // Configure montage settings
    UFUNCTION(BlueprintCallable, Category = "Animation|Survival")
    static void ConfigureMontageSettings(EAnim_SurvivalMontageType MontageType, const FAnim_MontageConfig& Config);

    // Get montage configuration
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Animation|Survival")
    static FAnim_MontageConfig GetMontageConfig(EAnim_SurvivalMontageType MontageType);

    // Play contextual survival animation based on character state
    UFUNCTION(BlueprintCallable, Category = "Animation|Survival")
    static bool PlayContextualSurvivalAnimation(UAnimInstance* AnimInstance, float Health, float Hunger, float Thirst, float Fear);

protected:
    // Static montage configurations
    static TMap<EAnim_SurvivalMontageType, FAnim_MontageConfig> MontageConfigs;

    // Initialize default montage configurations
    static void InitializeMontageConfigs();

    // Get montage asset path for type
    static FString GetMontageAssetPath(EAnim_SurvivalMontageType MontageType);
};