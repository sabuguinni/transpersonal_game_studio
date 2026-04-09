// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "CharacterExpressionSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCharacterExpression, Log, All);

class USkeletalMeshComponent;
class UAnimInstance;

/**
 * Primary emotional states for characters
 */
UENUM(BlueprintType)
enum class EPrimaryEmotion : uint8
{
    Neutral         UMETA(DisplayName = "Neutral"),
    Joy             UMETA(DisplayName = "Joy"),
    Sadness         UMETA(DisplayName = "Sadness"),
    Anger           UMETA(DisplayName = "Anger"),
    Fear            UMETA(DisplayName = "Fear"),
    Surprise        UMETA(DisplayName = "Surprise"),
    Disgust         UMETA(DisplayName = "Disgust"),
    Contempt        UMETA(DisplayName = "Contempt"),
    Pride           UMETA(DisplayName = "Pride"),
    Shame           UMETA(DisplayName = "Shame"),
    Curiosity       UMETA(DisplayName = "Curiosity"),
    Determination   UMETA(DisplayName = "Determination"),
    Confusion       UMETA(DisplayName = "Confusion"),
    Relief          UMETA(DisplayName = "Relief"),
    Anticipation    UMETA(DisplayName = "Anticipation")
};

/**
 * Secondary emotional modifiers
 */
UENUM(BlueprintType)
enum class EEmotionalModifier : uint8
{
    None            UMETA(DisplayName = "None"),
    Intense         UMETA(DisplayName = "Intense"),
    Subtle          UMETA(DisplayName = "Subtle"),
    Suppressed      UMETA(DisplayName = "Suppressed"),
    Exaggerated     UMETA(DisplayName = "Exaggerated"),
    Conflicted      UMETA(DisplayName = "Conflicted"),
    Tired           UMETA(DisplayName = "Tired"),
    Alert           UMETA(DisplayName = "Alert"),
    Relaxed         UMETA(DisplayName = "Relaxed"),
    Tense           UMETA(DisplayName = "Tense")
};

/**
 * Facial expression regions for detailed control
 */
UENUM(BlueprintType)
enum class EFacialRegion : uint8
{
    Eyes            UMETA(DisplayName = "Eyes"),
    Eyebrows        UMETA(DisplayName = "Eyebrows"),
    Mouth           UMETA(DisplayName = "Mouth"),
    Cheeks          UMETA(DisplayName = "Cheeks"),
    Nose            UMETA(DisplayName = "Nose"),
    Forehead        UMETA(DisplayName = "Forehead"),
    Jaw             UMETA(DisplayName = "Jaw"),
    Overall         UMETA(DisplayName = "Overall")
};

/**
 * Expression intensity levels
 */
UENUM(BlueprintType)
enum class EExpressionIntensity : uint8
{
    None            UMETA(DisplayName = "None"),
    VerySubtle      UMETA(DisplayName = "Very Subtle"),
    Subtle          UMETA(DisplayName = "Subtle"),
    Moderate        UMETA(DisplayName = "Moderate"),
    Strong          UMETA(DisplayName = "Strong"),
    Intense         UMETA(DisplayName = "Intense"),
    Extreme         UMETA(DisplayName = "Extreme")
};

/**
 * Emotional state data structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEmotionalState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    EPrimaryEmotion PrimaryEmotion = EPrimaryEmotion::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    EEmotionalModifier Modifier = EEmotionalModifier::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float DecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    bool bIsTransient = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    FString EmotionTrigger;

    FEmotionalState()
    {
        PrimaryEmotion = EPrimaryEmotion::Neutral;
        Intensity = 0.5f;
        Modifier = EEmotionalModifier::None;
        Duration = 5.0f;
        DecayRate = 0.1f;
        bIsTransient = true;
    }
};

/**
 * Facial expression configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FFacialExpressionConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression")
    FString ExpressionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression")
    TMap<FString, float> MorphTargetValues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression")
    float BlendTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression")
    bool bLooping = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression")
    float LoopDuration = 0.0f;

    FFacialExpressionConfig()
    {
        BlendTime = 1.0f;
        bLooping = false;
        LoopDuration = 0.0f;
    }
};

/**
 * Expression blend data for smooth transitions
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FExpressionBlendData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend")
    FFacialExpressionConfig SourceExpression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend")
    FFacialExpressionConfig TargetExpression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend")
    float BlendAlpha = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend")
    float BlendSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend")
    bool bIsBlending = false;

    FExpressionBlendData()
    {
        BlendAlpha = 0.0f;
        BlendSpeed = 1.0f;
        bIsBlending = false;
    }
};

/**
 * Character Expression System
 * Manages facial expressions, emotional states, and dynamic expression blending
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCharacterExpressionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCharacterExpressionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Expression Functions
    UFUNCTION(BlueprintCallable, Category = "Expression")
    void SetPrimaryEmotion(EPrimaryEmotion Emotion, float Intensity = 0.7f, float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Expression")
    void SetEmotionalState(const FEmotionalState& EmotionalState);

    UFUNCTION(BlueprintCallable, Category = "Expression")
    void BlendToExpression(const FFacialExpressionConfig& Expression, float BlendTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Expression")
    void SetFacialExpression(const FString& ExpressionName, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Expression")
    void ResetToNeutral(float BlendTime = 1.0f);

    // Regional Expression Control
    UFUNCTION(BlueprintCallable, Category = "Expression|Regional")
    void SetRegionalExpression(EFacialRegion Region, const TMap<FString, float>& MorphTargets, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Expression|Regional")
    void ModifyEyeExpression(float EyeOpenness, float EyeDirection, float BlinkRate);

    UFUNCTION(BlueprintCallable, Category = "Expression|Regional")
    void ModifyMouthExpression(float MouthOpenness, float SmileIntensity, float LipTension);

    UFUNCTION(BlueprintCallable, Category = "Expression|Regional")
    void ModifyBrowExpression(float BrowHeight, float BrowFurrow, float BrowAsymmetry);

    // Emotional State Management
    UFUNCTION(BlueprintCallable, Category = "Expression|Emotion")
    FEmotionalState GetCurrentEmotionalState() const;

    UFUNCTION(BlueprintCallable, Category = "Expression|Emotion")
    void AddEmotionalModifier(EEmotionalModifier Modifier, float Duration = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Expression|Emotion")
    void TriggerEmotionalResponse(const FString& Trigger, EPrimaryEmotion ResponseEmotion, float Intensity = 0.7f);

    UFUNCTION(BlueprintCallable, Category = "Expression|Emotion")
    void UpdateEmotionalState(float DeltaTime);

    // Expression Library Management
    UFUNCTION(BlueprintCallable, Category = "Expression|Library")
    void LoadExpressionLibrary();

    UFUNCTION(BlueprintCallable, Category = "Expression|Library")
    void RegisterCustomExpression(const FString& Name, const FFacialExpressionConfig& Expression);

    UFUNCTION(BlueprintCallable, Category = "Expression|Library")
    FFacialExpressionConfig GetExpressionByName(const FString& Name);

    UFUNCTION(BlueprintCallable, Category = "Expression|Library")
    TArray<FString> GetAvailableExpressions() const;

    // Dynamic Expression Generation
    UFUNCTION(BlueprintCallable, Category = "Expression|Dynamic")
    FFacialExpressionConfig GenerateExpressionFromEmotion(EPrimaryEmotion Emotion, EEmotionalModifier Modifier = EEmotionalModifier::None);

    UFUNCTION(BlueprintCallable, Category = "Expression|Dynamic")
    void CreateMicroExpression(EPrimaryEmotion Emotion, float Duration = 0.5f, float Intensity = 0.3f);

    UFUNCTION(BlueprintCallable, Category = "Expression|Dynamic")
    void SimulateNaturalBlink(float BlinkRate = 0.2f);

    // Contextual Expressions
    UFUNCTION(BlueprintCallable, Category = "Expression|Context")
    void ReactToEnvironment(float Temperature, float DangerLevel, float ComfortLevel);

    UFUNCTION(BlueprintCallable, Category = "Expression|Context")
    void ReactToCharacter(AActor* OtherCharacter, float Relationship, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Expression|Context")
    void ReactToEvent(const FString& EventType, float EventIntensity);

    // Animation Integration
    UFUNCTION(BlueprintCallable, Category = "Expression|Animation")
    void SynchronizeWithBodyAnimation(UAnimInstance* AnimInstance);

    UFUNCTION(BlueprintCallable, Category = "Expression|Animation")
    void SetExpressionAnimationSpeed(float Speed);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Expression|Utility")
    bool IsExpressionActive() const;

    UFUNCTION(BlueprintCallable, Category = "Expression|Utility")
    float GetExpressionIntensity() const;

    UFUNCTION(BlueprintCallable, Category = "Expression|Utility")
    void SetMeshComponent(USkeletalMeshComponent* MeshComp);

protected:
    // Target Mesh Component
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression|Setup")
    TObjectPtr<USkeletalMeshComponent> TargetMeshComponent;

    // Current Emotional State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression|State")
    FEmotionalState CurrentEmotionalState;

    // Expression Library
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression|Library")
    TMap<FString, FFacialExpressionConfig> ExpressionLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression|Library")
    TSoftObjectPtr<UDataTable> ExpressionDataTable;

    // Blending System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression|Blending")
    FExpressionBlendData CurrentBlend;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression|Blending")
    float DefaultBlendSpeed = 2.0f;

    // Natural Animation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression|Natural")
    float BlinkTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression|Natural")
    float BlinkInterval = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression|Natural")
    float BlinkDuration = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression|Natural")
    bool bEnableNaturalBlink = true;

    // Micro Expressions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression|Micro")
    TArray<FEmotionalState> MicroExpressionQueue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression|Micro")
    float MicroExpressionTimer = 0.0f;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression|Config")
    bool bEnableEmotionalDecay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression|Config")
    bool bEnableMicroExpressions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression|Config")
    bool bEnableContextualReactions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression|Config")
    float ExpressionSensitivity = 1.0f;

private:
    // Internal Functions
    void InitializeExpressionLibrary();
    void ProcessExpressionBlending(float DeltaTime);
    void ProcessNaturalAnimations(float DeltaTime);
    void ProcessMicroExpressions(float DeltaTime);
    void ApplyMorphTargetsToMesh(const TMap<FString, float>& MorphTargets, float Intensity = 1.0f);
    void BlendMorphTargets(const TMap<FString, float>& SourceMorphs, const TMap<FString, float>& TargetMorphs, float Alpha, TMap<FString, float>& OutBlendedMorphs);
    
    FFacialExpressionConfig CreateExpressionFromEmotion(EPrimaryEmotion Emotion);
    void UpdateEmotionalDecay(float DeltaTime);
    
    // Cached Data
    TMap<FString, float> CurrentMorphTargets;
    TMap<FString, float> PreviousMorphTargets;
    
    bool bIsInitialized = false;
    float LastBlinkTime = 0.0f;
};