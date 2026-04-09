#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "CharacterEmotionalSystem.generated.h"

class USkeletalMeshComponent;
class UMaterialInstanceDynamic;

/**
 * Emotional states for character expression
 */
UENUM(BlueprintType)
enum class ECharacterEmotion : uint8
{
    Neutral         UMETA(DisplayName = "Neutral"),
    Fear            UMETA(DisplayName = "Fear"),
    Anxiety         UMETA(DisplayName = "Anxiety"),
    Determination   UMETA(DisplayName = "Determination"),
    Exhaustion      UMETA(DisplayName = "Exhaustion"),
    Hope            UMETA(DisplayName = "Hope"),
    Despair         UMETA(DisplayName = "Despair"),
    Anger           UMETA(DisplayName = "Anger"),
    Curiosity       UMETA(DisplayName = "Curiosity"),
    Relief          UMETA(DisplayName = "Relief"),
    
    MAX             UMETA(Hidden)
};

/**
 * Survival stress levels affecting character appearance
 */
UENUM(BlueprintType)
enum class ESurvivalStressLevel : uint8
{
    Calm            UMETA(DisplayName = "Calm"),
    Mild            UMETA(DisplayName = "Mild Stress"),
    Moderate        UMETA(DisplayName = "Moderate Stress"),
    High            UMETA(DisplayName = "High Stress"),
    Extreme         UMETA(DisplayName = "Extreme Stress"),
    
    MAX             UMETA(Hidden)
};

/**
 * Structure for facial expression parameters
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FFacialExpressionData
{
    GENERATED_BODY()

    FFacialExpressionData()
        : EyebrowPosition(0.0f)
        , EyelidTension(0.0f)
        , MouthCurvature(0.0f)
        , JawTension(0.0f)
        , NostrilFlare(0.0f)
        , EyeWideness(0.0f)
        , LipCompression(0.0f)
        , CheekTension(0.0f)
        , ForeheadWrinkles(0.0f)
        , ExpressionIntensity(1.0f)
    {
    }

    // Facial muscle controls (-1.0 to 1.0 range)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float EyebrowPosition; // -1 = lowered (angry), +1 = raised (surprised)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float EyelidTension; // -1 = relaxed, +1 = tense/squinting

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float MouthCurvature; // -1 = frown, +1 = smile

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float JawTension; // -1 = relaxed, +1 = clenched

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NostrilFlare; // 0 = normal, 1 = flared

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EyeWideness; // 0 = normal, 1 = wide open

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float LipCompression; // 0 = normal, 1 = compressed/tight

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CheekTension; // 0 = relaxed, 1 = tense

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ForeheadWrinkles; // 0 = smooth, 1 = deeply wrinkled

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float ExpressionIntensity; // Overall intensity multiplier
};

/**
 * Data table row for emotion definitions
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEmotionDefinition : public FTableRowBase
{
    GENERATED_BODY()

    FEmotionDefinition()
        : EmotionName(TEXT("Neutral"))
        , Description(TEXT(""))
        , Duration(5.0f)
        , TransitionTime(1.0f)
        , bCanInterrupt(true)
        , Priority(1)
    {
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    FString EmotionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression")
    FFacialExpressionData ExpressionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float Duration; // How long this emotion lasts

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float TransitionTime; // Time to blend to this emotion

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bCanInterrupt; // Can this emotion be interrupted by others

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    int32 Priority; // Higher priority emotions override lower ones
};

/**
 * Character Emotional System Component
 * 
 * Manages the emotional state and facial expressions of characters.
 * Reflects their psychological state through visual cues - fear in their eyes,
 * determination in their jaw, exhaustion in their posture.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCharacterEmotionalSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCharacterEmotionalSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Emotion control
    UFUNCTION(BlueprintCallable, Category = "Emotions")
    void SetEmotion(ECharacterEmotion NewEmotion, float Intensity = 1.0f, float Duration = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Emotions")
    void BlendToEmotion(ECharacterEmotion NewEmotion, float BlendTime = 1.0f, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Emotions")
    void AddEmotionalLayer(ECharacterEmotion LayerEmotion, float LayerIntensity = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Emotions")
    void RemoveEmotionalLayer(ECharacterEmotion LayerEmotion);

    UFUNCTION(BlueprintCallable, Category = "Emotions")
    void ClearAllEmotions();

    // Survival stress system
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetSurvivalStress(ESurvivalStressLevel StressLevel);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddStressEvent(float StressAmount, float Duration = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyFatigueEffects(float FatigueLevel);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyHungerEffects(float HungerLevel);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyInjuryEffects(float InjuryLevel, const FString& InjuryType);

    // Environmental reactions
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ReactToDinosaur(float ThreatLevel, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ReactToWeather(const FString& WeatherType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ReactToTimeOfDay(float TimeOfDay);

    // Getters
    UFUNCTION(BlueprintCallable, Category = "Emotions")
    ECharacterEmotion GetCurrentEmotion() const { return CurrentEmotion; }

    UFUNCTION(BlueprintCallable, Category = "Emotions")
    float GetEmotionalIntensity() const { return CurrentIntensity; }

    UFUNCTION(BlueprintCallable, Category = "Emotions")
    FFacialExpressionData GetCurrentExpression() const { return CurrentExpression; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    ESurvivalStressLevel GetStressLevel() const { return CurrentStressLevel; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetOverallStress() const { return OverallStressLevel; }

protected:
    // Current emotional state
    UPROPERTY(BlueprintReadOnly, Category = "State")
    ECharacterEmotion CurrentEmotion;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FFacialExpressionData CurrentExpression;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FFacialExpressionData TargetExpression;

    // Survival state
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    ESurvivalStressLevel CurrentStressLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float OverallStressLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FatigueLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HungerLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float InjuryLevel;

    // Emotion blending
    UPROPERTY()
    TMap<ECharacterEmotion, float> EmotionalLayers;

    UPROPERTY()
    float BlendTimer;

    UPROPERTY()
    float BlendDuration;

    UPROPERTY()
    bool bIsBlending;

    // Configuration
    UPROPERTY(EditAnywhere, Category = "Configuration")
    UDataTable* EmotionDataTable;

    UPROPERTY(EditAnywhere, Category = "Configuration")
    float DefaultEmotionDuration = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Configuration")
    float DefaultBlendTime = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Configuration")
    float StressDecayRate = 0.1f; // Stress reduction per second

    UPROPERTY(EditAnywhere, Category = "Configuration")
    float MaxStressLevel = 100.0f;

    // Material references for facial expression
    UPROPERTY()
    UMaterialInstanceDynamic* FaceMaterialInstance;

    UPROPERTY()
    USkeletalMeshComponent* FaceMeshComponent;

    // Emotion timing
    UPROPERTY()
    float EmotionTimer;

    UPROPERTY()
    float CurrentEmotionDuration;

private:
    void InitializeFacialMaterials();
    void UpdateFacialExpression(float DeltaTime);
    void ApplyExpressionToMaterial(const FFacialExpressionData& Expression);
    
    FFacialExpressionData GetEmotionExpression(ECharacterEmotion Emotion) const;
    FFacialExpressionData BlendExpressions(const FFacialExpressionData& A, const FFacialExpressionData& B, float Alpha) const;
    
    void UpdateStressLevel(float DeltaTime);
    void ApplyStressToExpression();
    
    FEmotionDefinition* GetEmotionDefinition(ECharacterEmotion Emotion) const;
    
    void HandleEmotionTimeout();
    void ProcessEmotionalLayers();
};