#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "../Core/SharedTypes.h"
#include "Anim_EnvironmentalAnimationController.generated.h"

class ACharacter;
class USkeletalMeshComponent;
class UAnimInstance;

UENUM(BlueprintType)
enum class EAnim_EnvironmentalState : uint8
{
    Normal          UMETA(DisplayName = "Normal"),
    InWater         UMETA(DisplayName = "In Water"),
    OnSteepSlope    UMETA(DisplayName = "On Steep Slope"),
    InDenseVegetation UMETA(DisplayName = "In Dense Vegetation"),
    OnIce           UMETA(DisplayName = "On Ice"),
    InMud           UMETA(DisplayName = "In Mud"),
    InSand          UMETA(DisplayName = "In Sand"),
    InSnow          UMETA(DisplayName = "In Snow"),
    OnRocks         UMETA(DisplayName = "On Rocks"),
    InCave          UMETA(DisplayName = "In Cave")
};

UENUM(BlueprintType)
enum class EAnim_WeatherState : uint8
{
    Clear           UMETA(DisplayName = "Clear"),
    Rain            UMETA(DisplayName = "Rain"),
    Storm           UMETA(DisplayName = "Storm"),
    Snow            UMETA(DisplayName = "Snow"),
    Fog             UMETA(DisplayName = "Fog"),
    Wind            UMETA(DisplayName = "Wind"),
    Heat            UMETA(DisplayName = "Heat"),
    Cold            UMETA(DisplayName = "Cold")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_EnvironmentalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    EAnim_EnvironmentalState EnvironmentalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    EAnim_WeatherState WeatherState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float WaterDepth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float SlopeAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float VegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float WindStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float Visibility;

    FAnim_EnvironmentalData()
    {
        EnvironmentalState = EAnim_EnvironmentalState::Normal;
        WeatherState = EAnim_WeatherState::Clear;
        WaterDepth = 0.0f;
        SlopeAngle = 0.0f;
        VegetationDensity = 0.0f;
        WindStrength = 0.0f;
        Temperature = 20.0f;
        Visibility = 1.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnvironmentalStateChanged, EAnim_EnvironmentalState, OldState, EAnim_EnvironmentalState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeatherStateChanged, EAnim_WeatherState, OldState, EAnim_WeatherState, NewState);

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_EnvironmentalAnimationController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_EnvironmentalAnimationController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Environmental state management
    UFUNCTION(BlueprintCallable, Category = "Environmental Animation")
    void UpdateEnvironmentalState(const FAnim_EnvironmentalData& NewEnvironmentalData);

    UFUNCTION(BlueprintCallable, Category = "Environmental Animation")
    void SetEnvironmentalState(EAnim_EnvironmentalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Environmental Animation")
    void SetWeatherState(EAnim_WeatherState NewState);

    UFUNCTION(BlueprintPure, Category = "Environmental Animation")
    EAnim_EnvironmentalState GetCurrentEnvironmentalState() const { return CurrentEnvironmentalData.EnvironmentalState; }

    UFUNCTION(BlueprintPure, Category = "Environmental Animation")
    EAnim_WeatherState GetCurrentWeatherState() const { return CurrentEnvironmentalData.WeatherState; }

    UFUNCTION(BlueprintPure, Category = "Environmental Animation")
    FAnim_EnvironmentalData GetCurrentEnvironmentalData() const { return CurrentEnvironmentalData; }

    // Animation blending
    UFUNCTION(BlueprintCallable, Category = "Environmental Animation")
    void UpdateAnimationBlending();

    UFUNCTION(BlueprintPure, Category = "Environmental Animation")
    float GetEnvironmentalBlendWeight(EAnim_EnvironmentalState State) const;

    UFUNCTION(BlueprintPure, Category = "Environmental Animation")
    float GetWeatherBlendWeight(EAnim_WeatherState State) const;

    // Surface detection
    UFUNCTION(BlueprintCallable, Category = "Environmental Animation")
    void DetectSurfaceType();

    UFUNCTION(BlueprintCallable, Category = "Environmental Animation")
    void DetectWeatherConditions();

    // Animation montages for environmental reactions
    UFUNCTION(BlueprintCallable, Category = "Environmental Animation")
    bool PlayEnvironmentalReaction(EAnim_EnvironmentalState State);

    UFUNCTION(BlueprintCallable, Category = "Environmental Animation")
    bool PlayWeatherReaction(EAnim_WeatherState State);

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Environmental Animation")
    void SetEnvironmentalMontage(EAnim_EnvironmentalState State, UAnimMontage* Montage);

    UFUNCTION(BlueprintCallable, Category = "Environmental Animation")
    void SetWeatherMontage(EAnim_WeatherState State, UAnimMontage* Montage);

    UFUNCTION(BlueprintCallable, Category = "Environmental Animation")
    void SetEnvironmentalBlendSpace(EAnim_EnvironmentalState State, UBlendSpace* BlendSpace);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Environmental Animation")
    FOnEnvironmentalStateChanged OnEnvironmentalStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Environmental Animation")
    FOnWeatherStateChanged OnWeatherStateChanged;

protected:
    // Core references
    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComp;

    UPROPERTY()
    UAnimInstance* AnimInstance;

    // Current environmental data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Animation", meta = (AllowPrivateAccess = "true"))
    FAnim_EnvironmentalData CurrentEnvironmentalData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Animation", meta = (AllowPrivateAccess = "true"))
    FAnim_EnvironmentalData TargetEnvironmentalData;

    // Blend weights for smooth transitions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Animation", meta = (AllowPrivateAccess = "true"))
    TMap&lt;EAnim_EnvironmentalState, float&gt; EnvironmentalBlendWeights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Animation", meta = (AllowPrivateAccess = "true"))
    TMap&lt;EAnim_WeatherState, float&gt; WeatherBlendWeights;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets", meta = (AllowPrivateAccess = "true"))
    TMap&lt;EAnim_EnvironmentalState, UAnimMontage*&gt; EnvironmentalMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets", meta = (AllowPrivateAccess = "true"))
    TMap&lt;EAnim_WeatherState, UAnimMontage*&gt; WeatherMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets", meta = (AllowPrivateAccess = "true"))
    TMap&lt;EAnim_EnvironmentalState, UBlendSpace*&gt; EnvironmentalBlendSpaces;

    // Detection settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (AllowPrivateAccess = "true"))
    float SurfaceDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (AllowPrivateAccess = "true"))
    float EnvironmentalUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (AllowPrivateAccess = "true"))
    float BlendTransitionSpeed;

    // Timers
    float EnvironmentalUpdateTimer;

private:
    // Helper functions
    void UpdateBlendWeights(float DeltaTime);
    void PerformSurfaceTrace();
    void AnalyzeEnvironmentalConditions();
    UAnimMontage* GetMontageForEnvironmentalState(EAnim_EnvironmentalState State) const;
    UAnimMontage* GetMontageForWeatherState(EAnim_WeatherState State) const;
    void InitializeBlendWeights();
};