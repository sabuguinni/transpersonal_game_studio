#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "AnimationSystemManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterEmotionalStateChanged, class ACharacter*, Character, float, FearLevel);

/**
 * Sistema central de gerenciamento de animações
 * Responsável por coordenar Motion Matching, IK, e estados emocionais
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAnimationSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAnimationSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Sistema de Motion Matching
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* PlayerLocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* DinosaurBehaviorDatabase;

    // Sistema de IK Adaptativo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float TerrainAdaptationRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float FootPlantingThreshold = 0.1f;

    // Estados Emocionais Dinâmicos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional States")
    TMap<FString, float> EmotionalStateWeights;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCharacterEmotionalStateChanged OnCharacterEmotionalStateChanged;

public:
    // Interface pública para outros sistemas
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void RegisterCharacterForAnimation(class ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void UpdateCharacterFearLevel(class ACharacter* Character, float NewFearLevel);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void TriggerEmergencyAnimation(class ACharacter* Character, const FString& TriggerType);

    // Sistema de Variações Procedurais
    UFUNCTION(BlueprintCallable, Category = "Procedural Animation")
    void GenerateUniqueMovementVariation(class ACharacter* Character);

private:
    // Personagens registados no sistema
    UPROPERTY()
    TArray<class ACharacter*> RegisteredCharacters;

    // Cache de dados de animação
    UPROPERTY()
    TMap<class ACharacter*, struct FCharacterAnimationData> AnimationDataCache;

    void UpdateMotionMatchingQueries(float DeltaTime);
    void ProcessTerrainAdaptation(float DeltaTime);
    void UpdateEmotionalStates(float DeltaTime);
};

// Estrutura para dados de animação por personagem
USTRUCT(BlueprintType)
struct FCharacterAnimationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentFearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownThreatDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeSinceLastThreat = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsInDanger = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> ActiveEmotionalTags;
};