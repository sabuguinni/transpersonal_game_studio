#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/ActorComponent.h"
#include "AnimationSystemManager.generated.h"

UENUM(BlueprintType)
enum class ECharacterAnimationType : uint8
{
    Player          UMETA(DisplayName = "Player Character"),
    SmallHerbivore  UMETA(DisplayName = "Small Herbivore"),
    LargeHerbivore  UMETA(DisplayName = "Large Herbivore"),
    SmallCarnivore  UMETA(DisplayName = "Small Carnivore"),
    LargeCarnivore  UMETA(DisplayName = "Large Carnivore"),
    Apex            UMETA(DisplayName = "Apex Predator")
};

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm            UMETA(DisplayName = "Calm"),
    Alert           UMETA(DisplayName = "Alert"),
    Nervous         UMETA(DisplayName = "Nervous"),
    Fearful         UMETA(DisplayName = "Fearful"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Feeding         UMETA(DisplayName = "Feeding"),
    Resting         UMETA(DisplayName = "Resting")
};

USTRUCT(BlueprintType)
struct FCharacterPersonality
{
    GENERATED_BODY()

    // Personalidade única de cada personagem
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Nervousness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Confidence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.5f;

    // Variações físicas únicas
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float MovementSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float StepLength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.9", ClampMax = "1.1"))
    float BodySway = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float HeadMovement = 1.0f;
};

/**
 * Sistema central de gestão de animações
 * Coordena Motion Matching, IK, e personalidade de personagens
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnimationSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnimationSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Configuração do personagem
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Setup")
    ECharacterAnimationType CharacterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Setup")
    FCharacterPersonality Personality;

    // Estado emocional atual
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EEmotionalState CurrentEmotionalState;

    // Funções principais
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetEmotionalState(EEmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdatePersonalityInfluence(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetPersonalityModifier(const FString& ModifierName) const;

    // Eventos para outros sistemas
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmotionalStateChanged, EEmotionalState, OldState, EEmotionalState, NewState);
    UPROPERTY(BlueprintAssignable)
    FOnEmotionalStateChanged OnEmotionalStateChanged;

private:
    // Estado interno
    float StateTransitionTime;
    EEmotionalState PreviousEmotionalState;
    
    // Timers para variações naturais
    float PersonalityUpdateTimer;
    float NaturalVariationTimer;

    void ProcessEmotionalTransition(float DeltaTime);
    void ApplyNaturalVariations(float DeltaTime);
};