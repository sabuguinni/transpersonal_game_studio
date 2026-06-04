#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Char_FacialExpressionSystem.generated.h"

// Facial expression types for Cretaceous tribal humans
UENUM(BlueprintType)
enum class EChar_FacialExpression : uint8
{
    Neutral         UMETA(DisplayName = "Neutral"),
    Alert           UMETA(DisplayName = "Alert"),
    Fearful         UMETA(DisplayName = "Fearful"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Curious         UMETA(DisplayName = "Curious"),
    Tired           UMETA(DisplayName = "Tired"),
    Hungry          UMETA(DisplayName = "Hungry"),
    Pain            UMETA(DisplayName = "Pain"),
    Focused         UMETA(DisplayName = "Focused"),
    Surprised       UMETA(DisplayName = "Surprised")
};

// Expression intensity levels
UENUM(BlueprintType)
enum class EChar_ExpressionIntensity : uint8
{
    Subtle      UMETA(DisplayName = "Subtle"),
    Moderate    UMETA(DisplayName = "Moderate"),
    Strong      UMETA(DisplayName = "Strong"),
    Extreme     UMETA(DisplayName = "Extreme")
};

// Facial expression configuration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_ExpressionData
{
    GENERATED_BODY()

    // Expression type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression")
    EChar_FacialExpression ExpressionType = EChar_FacialExpression::Neutral;

    // Intensity of the expression
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression")
    EChar_ExpressionIntensity Intensity = EChar_ExpressionIntensity::Moderate;

    // Duration to hold expression (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression")
    float Duration = 2.0f;

    // Transition time to reach expression
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression")
    float TransitionTime = 0.5f;

    // Morph target weights for this expression
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Expression")
    TMap<FString, float> MorphTargetWeights;

    FChar_ExpressionData()
    {
        ExpressionType = EChar_FacialExpression::Neutral;
        Intensity = EChar_ExpressionIntensity::Moderate;
        Duration = 2.0f;
        TransitionTime = 0.5f;
    }
};

// Expression trigger conditions
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_ExpressionTrigger
{
    GENERATED_BODY()

    // Health threshold to trigger expression
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    float HealthThreshold = 0.5f;

    // Fear level to trigger expression
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    float FearThreshold = 0.7f;

    // Hunger level to trigger expression
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    float HungerThreshold = 0.8f;

    // Distance to threat to trigger expression
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    float ThreatDistance = 1000.0f;

    // Expression to trigger
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    FChar_ExpressionData TriggeredExpression;

    FChar_ExpressionTrigger()
    {
        HealthThreshold = 0.5f;
        FearThreshold = 0.7f;
        HungerThreshold = 0.8f;
        ThreatDistance = 1000.0f;
    }
};

/**
 * Manages facial expressions for Cretaceous tribal characters
 * Handles morph target blending, expression triggers, and emotional responses
 */
UCLASS(ClassGroup=(Character), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_FacialExpressionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_FacialExpressionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Set facial expression directly
    UFUNCTION(BlueprintCallable, Category = "Facial Expression")
    void SetExpression(EChar_FacialExpression Expression, EChar_ExpressionIntensity Intensity = EChar_ExpressionIntensity::Moderate);

    // Play expression with custom duration
    UFUNCTION(BlueprintCallable, Category = "Facial Expression")
    void PlayExpression(const FChar_ExpressionData& ExpressionData);

    // Return to neutral expression
    UFUNCTION(BlueprintCallable, Category = "Facial Expression")
    void ReturnToNeutral();

    // Check for automatic expression triggers
    UFUNCTION(BlueprintCallable, Category = "Facial Expression")
    void UpdateExpressionTriggers();

    // Get current expression
    UFUNCTION(BlueprintPure, Category = "Facial Expression")
    EChar_FacialExpression GetCurrentExpression() const { return CurrentExpression; }

    // Check if transitioning between expressions
    UFUNCTION(BlueprintPure, Category = "Facial Expression")
    bool IsTransitioning() const { return bIsTransitioning; }

protected:
    // Current facial expression
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expression State")
    EChar_FacialExpression CurrentExpression = EChar_FacialExpression::Neutral;

    // Current expression intensity
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expression State")
    EChar_ExpressionIntensity CurrentIntensity = EChar_ExpressionIntensity::Moderate;

    // Whether currently transitioning between expressions
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expression State")
    bool bIsTransitioning = false;

    // Time remaining for current expression
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expression State")
    float ExpressionTimeRemaining = 0.0f;

    // Transition progress (0-1)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expression State")
    float TransitionProgress = 0.0f;

    // Expression configuration data table
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    class UDataTable* ExpressionDataTable;

    // Automatic expression triggers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FChar_ExpressionTrigger> ExpressionTriggers;

    // Reference to character's skeletal mesh
    UPROPERTY()
    class USkeletalMeshComponent* CharacterMesh;

    // Reference to character's game state for survival stats
    UPROPERTY()
    class ATranspersonalGameState* GameState;

private:
    // Apply morph target weights to mesh
    void ApplyMorphTargets(const TMap<FString, float>& MorphTargets, float BlendWeight = 1.0f);

    // Get expression data for given type and intensity
    FChar_ExpressionData GetExpressionData(EChar_FacialExpression Expression, EChar_ExpressionIntensity Intensity);

    // Initialize default expression configurations
    void InitializeDefaultExpressions();

    // Update transition between expressions
    void UpdateTransition(float DeltaTime);

    // Check survival stat triggers
    void CheckSurvivalStatTriggers();

    // Check environmental triggers
    void CheckEnvironmentalTriggers();
};